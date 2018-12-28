
#include <time.h>
#include <stdlib.h>
#include <jni.h>
#include <jvmti.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <sstream>
#include <set>
#include <stack>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <utility>
#include "jvmti_utils.h"
#include "jeeves.h"
#include "cookie_cutter.h"

using json = nlohmann::json;

static bool mainReached = false;

static jiim::jeeves* jeeves;

static unsigned long long message_counter; 
static std::vector<std::string> start_targets;
static std::vector<std::string> follow_targets;

static boost::asio::io_context io_context;
static boost::asio::io_service io_service;
static boost::asio::ip::udp::socket sock(io_service);
static boost::asio::ip::udp::endpoint remote_endpoint;
static jrawMonitorID lock;

// caching

static std::unordered_map<jmethodID, std::string> method_names_cache;
static std::unordered_map<jmethodID, std::vector<unsigned char>*> method_bytecodes;
static std::unordered_map<jmethodID, jclass> declaring_class;
static std::unordered_map<jthread, int> waiting_for_single_steps;

static std::unordered_map<std::size_t, long long> over_the_top;

static std::set<unsigned char> snapshot_instructions { 

	// ifs and cmps:
	0x98, 0x97, 0x96, 0x95, 0xa5, 0xa6, 0x9f, 0xa2, 0xa3, 0xa4, 0xa1, 0xa0, 0x94, 0x99, 0x9c, 0x9d, 0x9e, 0x9b, 0x9a, 0xc7, 0xc6,

	// stores:
	0x3a, 0x4b, 0x4c, 0x4d, 0x4e, 0x39, 0x47, 0x48, 0x49, 0x4a, 0x38, 0x43, 0x44, 0x45, 0x46, 0x36, 0x3b, 0x3c, 0x3d, 0x3e, 0x37, 0x3f, 0x40, 0x41, 0x42,

	// putfield and putstatic:
	0xb5, 0xb3,

	// returns:
	0xb0, 0xaf, 0xae, 0xac, 0xad, 0xb1,

	// loads:
	// 0x19, 0x2a, 0x2b, 0x2c, 0x18, 0x26, 0x27, 0x28, 0x29, 0x17, 0x22, 0x23, 0x24, 0x25, 0x15, 0x1a, 0x1b, 0x1c, 0x1d, 0x16, 0x1e, 0x1f, 0x20, 0x21	
};

static std::set<unsigned char> arming_instructions {

	// stores:
	0x3a, 0x4b, 0x4c, 0x4d, 0x4e, 0x39, 0x47, 0x48, 0x49, 0x4a, 0x38, 0x43, 0x44, 0x45, 0x46, 0x36, 0x3b, 0x3c, 0x3d, 0x3e, 0x37, 0x3f, 0x40, 0x41, 0x42,

	// putfield and putstatic:
	0xb5, 0xb3,
};

std::string get_method_name(jvmtiEnv* jvmti_env, jmethodID method) 
{
    jclass declaringType;

    check_jvmti_error(jvmti_env, 
    	jvmti_env->GetMethodDeclaringClass(method, &declaringType), 
    	"Unable to get method declaring class");

    declaring_class.emplace(method, declaringType);

    std::string klass = jiim::get_class_signature(jvmti_env, declaringType);

    char *name;
    char *sig;
    char *gsig;
    check_jvmti_error(jvmti_env, 
    	jvmti_env->GetMethodName(method, &name, &sig, &gsig), 
    	"Unable to get method name");

    std::stringstream stream;
    stream << klass << "#" << name << ((gsig == NULL) ? sig : gsig);

    if(name) jvmti_env->Deallocate((unsigned char*)(name));
   	if(sig) jvmti_env->Deallocate((unsigned char*)(sig));
   	if(gsig) jvmti_env->Deallocate((unsigned char*)(gsig));

    return stream.str();
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
	srand(time(NULL));   // Initialization, should only be called once.
	message_counter = 1;
	mainReached = false;
	jeeves = new jiim::jeeves(vm);

	std::string unparsed_targets(options);

	if(unparsed_targets.length() == 0)
	{
		std::cerr << "No analysis targets given. Quitting." << std::endl;
		exit(1);
	}

	std::vector<std::string> targets;
	boost::split(targets, options, boost::is_any_of(","));

	for(auto target : targets)
	{
		char meaning = target.at(0);
		target = target.substr(1);
		std::stringstream ss;
		boost::replace_all(target, ".", "/");
		ss << "L" << target;
		target = ss.str();

		if(meaning == 'S')
		{
			std::cout << "Waiting for (S)tarting target " << target << std::endl;
			start_targets.push_back(target);
		}
		else if(meaning == 'F')
		{
			std::cout << "Waiting for (F)ollow target " << target << std::endl;
			follow_targets.push_back(target);
		}
		else
		{
			std::cerr << "Target should be prefixed exclusively with S (starting) or F (following)." << std::endl;
			exit(1);
		}
	}

	try
	{		
		sock.open(boost::asio::ip::udp::v4());
		remote_endpoint = boost::asio::ip::udp::endpoint(
			boost::asio::ip::address::from_string("0.0.0.0"), 5555);
	}
	catch(...)
	{
		std::cout << "Cannot open UDP port." << std::endl;
		exit(-1);
	}

	jeeves->prepare()
		->create_lock(lock)
		->accesses_local_variables()
		->accesses_fields()
		->accesses_methods()

		->sends_vm_init_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) 
		{
			auto class_object = jni_env->FindClass("sun/launcher/LauncherHelper");
			if(class_object == nullptr)
			{
				std::cout << "Cannot get starting sun.launcher.LauncherHelper class." << std::endl;
				exit(-1);
			}

			auto method_id = jni_env->GetStaticMethodID(class_object, "getApplicationClass",  "()Ljava/lang/Class;");

			auto error = jvmti_env->SetBreakpoint(method_id, 3 /* magic number, got this through trial and error */);
			if(error != JNI_OK)
			{
				std::cout << "Cannot set breakpoint (" << error << ")." << std::endl;
				exit(-1);
			}
		})

		->sends_method_entry_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method)
		{

			if(mainReached)
			{
				std::string name = "";
				bool following = false;

				if(method_names_cache.count(method) == 0)
				{
					name = get_method_name(jvmti_env, method);
					boost::trim(name);

					bool target_found = false;

					for(auto target: follow_targets)
					{
						if(boost::starts_with(name, target))
						{
							method_names_cache.emplace(method, name);
							following = true;
							break;
						}
					}

					for(auto target: start_targets)
					{
						if(boost::starts_with(name, target))
						{
							target_found = true;
							jeeves->single_step_on();

							method_names_cache.emplace(method, name);
							break;
						}
					}					

					if(!target_found) return;
				}
				else
				{
					name = method_names_cache[method];
				}

				// json j = { { "method_entry", { { "name", name }, } } };

				// std::string msg = j.dump();

				// boost::system::error_code err;
				// sock.send_to(boost::asio::buffer(msg.c_str(), msg.size()), 
				// 	remote_endpoint, 0, err);

				jint bytecode_count; 
				unsigned char* bytecode_array;

				auto result = jvmti_env->GetBytecodes(method, &bytecode_count, &bytecode_array);
				if(result == JNI_OK)
				{
					method_bytecodes.emplace(method, new std::vector<unsigned char>());
					for(int i = 0; i < bytecode_count; i++)
					{
						method_bytecodes[method]->push_back(bytecode_array[i]);
					}

					jvmti_env->Deallocate(bytecode_array);
				}

				if(following)
				{
					json bindings;

					auto variables = jiim::cookie_cutter::get_variable_bindings(jvmti_env, jni_env, thread, method, follow_targets);
					if(variables.size() > 0)
					{
						for(auto bound : variables)
						{
							bindings[bound.first] = bound.second.to_json();
						}

						json j = {
							{ "report", { 
								{ "index", message_counter++ },
								{ "method", method_names_cache[method] }, 
								{ "location", 1 }, 
								{ "bindings", bindings },
							} }
						};

						std::string msg = j.dump();

						boost::system::error_code err;
						sock.send_to(boost::asio::buffer(msg.c_str(), msg.size()), 
							remote_endpoint, 0, err);
					}					
				}
			}

		})

		->sends_method_exit_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jboolean was_popped_by_exception, jvalue return_value)
		{

			if(mainReached) 
			{
				std::string name = "";
				if(method_names_cache.count(method) > 0)
				{
					name = method_names_cache[method];
				}
				else
				{
					return;
				}

				jint bytecode_count;
				unsigned char* bytecode_array;

				auto result = jvmti_env->GetBytecodes(method, &bytecode_count, &bytecode_array);

				if(result == JNI_OK)
				{
					char *mname;
				    char *sig;
				    char *gsig;
				    check_jvmti_error(jvmti_env, 
				    	jvmti_env->GetMethodName(method, &mname, &sig, &gsig), 
				    	"Unable to get method name");
			
					auto sign = ((gsig == NULL) ? sig : gsig);
					int i = 0;
					for(; sign[i] != ')'; i++); 
					i++;

					char return_type = sign[i];

					if(mname) jvmti_env->Deallocate((unsigned char*)(mname));
					if(sig) jvmti_env->Deallocate((unsigned char*)(sig));
					if(gsig) jvmti_env->Deallocate((unsigned char*)(gsig));

					json bindings;

					auto variables = jiim::cookie_cutter::get_variable_bindings(jvmti_env, jni_env, thread, method, follow_targets);
					for(auto bound : variables)
					{
						bindings[bound.first] = bound.second.to_json();
					}

					if(return_type != 'V' && return_type != '[')
					{
						bindings[".return"] = jiim::cookie_cutter::tagged_jvalue(return_type, return_value).to_json();
					}

					if(bindings.size() > 0)
					{
						json j = { 
							{ "report", { 
								{ "index", message_counter++ }, 
								{ "method", name }, 
								{ "location", bytecode_count },
								{ "bindings", bindings }
							} } 
						};

						std::string msg = j.dump();
						method_names_cache.emplace(method, msg);

						boost::system::error_code err;
						sock.send_to(boost::asio::buffer(msg.c_str(), msg.size()), 
							remote_endpoint, 0, err);
					}
				}

				if(bytecode_array) jvmti_env->Deallocate(bytecode_array);
			}
		})

		->sends_single_step_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jlocation location) 
		{
			if(mainReached)
			{
				// this is inserted at the same time as method_names_cache, but is safer to use
				//  because it tells us whether the method in question is native or not!
				if(method_bytecodes.count(method) > 0)
				{
					std::size_t pair = std::hash<long>{}((long) method) ^ std::hash<long>{}((long)location);

					// if(over_the_top.count(pair) == 0)
					// {
					// 	over_the_top[pair] = 0;
					// }
					over_the_top[pair]++;

					if((float)over_the_top[pair] / (float)message_counter > 0.5f) return;

					std::vector<unsigned char> instructions = *(method_bytecodes[method]);
					auto inst = instructions[location];

					auto execute_single_step = waiting_for_single_steps[thread] >= 0 || snapshot_instructions.count(inst) > 0;

					if(execute_single_step)
					{
						if(waiting_for_single_steps[thread] >= 0)
						{
							waiting_for_single_steps[thread] = -1;
						}

						json bindings;

						auto variables = jiim::cookie_cutter::get_variable_bindings(jvmti_env, jni_env, thread, method, follow_targets);
						
						if(variables.size() > 0)
						{
							for(auto bound : variables)
							{
								bindings[bound.first] = bound.second.to_json();
							}

							json j = {
								{ "variable_bindings", {
									{ "index", message_counter++ },
									{ "method", method_names_cache[method] },
									{ "location", location },
									{ "bindings", bindings },
								} }
							};

							std::string msg = j.dump();

							boost::system::error_code err;
							sock.send_to(boost::asio::buffer(msg.c_str(), msg.size()), 
								remote_endpoint, 0, err);
						}

						if(arming_instructions.count(inst) > 0)
							waiting_for_single_steps[thread] = location;
					}
				}
			}

		})

		->sends_breakpoint_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jlocation location) 
		{
			mainReached = true;
		})

		->boot();

	return 0;
}
