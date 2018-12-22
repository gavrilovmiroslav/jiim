#pragma once

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

#include "jvmti_utils.h"

using json = nlohmann::json;

namespace jiim 
{
	std::string get_class_signature(jvmtiEnv* jvmti_env, jclass klass) 
	{
	    char *signature = nullptr;
	    auto error = jvmti_env->GetClassSignature(klass, &signature, nullptr);
	    if (error == JVMTI_ERROR_INVALID_CLASS) 
	    {
	        return "<invalid class>";
	    }

	    std::string ret = std::string(signature);
	    if(signature != nullptr)
		   	jvmti_env->Deallocate((unsigned char*)(signature));

	    return ret;
	}

	namespace cookie_cutter
	{
		struct tagged_jvalue
		{
			jvalue value;
			unsigned char tag;

			tagged_jvalue(unsigned char tag, jvalue value)
				: value(value)
				, tag(tag)
			{}

			friend std::ostream& operator<<(std::ostream& stream, const tagged_jvalue& v);

			json to_json() const
			{
				json valuation;
				switch(tag)
				{
					case 'B': valuation = { { "tag", "B" }, { "value", value.b } }; break;
					case 'C': valuation = { { "tag", "C" }, { "value", value.c } }; break;
					case 'I': valuation = { { "tag", "I" }, { "value", value.i } }; break;
					case 'S': valuation = { { "tag", "S" }, { "value", value.s } }; break;
					case 'Z': valuation = { { "tag", "Z" }, { "value", value.z } }; break;
					case 'D': valuation = { { "tag", "D" }, { "value", value.d } }; break;
					case 'F': valuation = { { "tag", "F" }, { "value", value.f } }; break;
					case 'J': valuation = { { "tag", "J" }, { "value", value.j } }; break;
					case 'L': valuation = { { "tag", "L" }, { "value", value.j } }; break;
					default: valuation = { { "tag", tag }, "value", value.j }; break;
				}

				return valuation;
			}
		};

		std::ostream& operator<<(std::ostream& stream, const tagged_jvalue& v)
		{
			char output[100];
			switch(v.tag)
			{
				case 'B': sprintf(output, "%d", v.value.b); break;
				case 'C': sprintf(output, "%d", v.value.c); break;
				case 'I': sprintf(output, "%d", v.value.i); break;
				case 'S': sprintf(output, "%d", v.value.s); break;
				case 'Z': sprintf(output, "%d", v.value.z); break;
				case 'D': sprintf(output, "%f", v.value.d); break;
				case 'F': sprintf(output, "%f", v.value.f); break;
				case 'J': sprintf(output, "%lu", v.value.j); break;
				case 'L': sprintf(output, "%p", v.value.l); break;
				default: sprintf(output, "<unknown>"); break;
			}

			stream << output;
			return stream;
    	}    	

		tagged_jvalue get_value_by_signature(jvmtiEnv* jvmti_env, jthread thread, jmethodID method, jvmtiLocalVariableEntry entry)
		{
			jvalue value;
			jvmtiError error;
			switch(entry.signature[0])
			{
				case 'B': { jint v; error = jvmti_env->GetLocalInt(thread, 0, entry.slot, &v); value.b = v; } break;
				case 'C': { jint v; error = jvmti_env->GetLocalInt(thread, 0, entry.slot, &v); value.c = v; } break;
				case 'I': { jint v; error = jvmti_env->GetLocalInt(thread, 0, entry.slot, &v); value.i = v; } break;
				case 'S': { jint v; error = jvmti_env->GetLocalInt(thread, 0, entry.slot, &v); value.s = v; } break;
				case 'Z': { jint v; error = jvmti_env->GetLocalInt(thread, 0, entry.slot, &v); value.z = v; } break;
				case 'D': { jdouble v; error = jvmti_env->GetLocalDouble(thread, 0, entry.slot, &v); value.d = v; } break;
				case 'F': { jfloat v; error = jvmti_env->GetLocalFloat(thread, 0, entry.slot, &v); value.f = v; } break;
				case 'J': { jlong v; error = jvmti_env->GetLocalLong(thread, 0, entry.slot, &v); value.j = v; } break;
				case 'L': { jobject v; error = jvmti_env->GetLocalObject(thread, 0, entry.slot, &v); value.l = v; } break;
				default: return tagged_jvalue('N', value);
			}

			if(error == JVMTI_ERROR_INVALID_SLOT)
				return tagged_jvalue('N', value);

			// debug mode off:
			if(error != JVMTI_ERROR_NONE)
				return tagged_jvalue('N', value);

			// debug mode on:
			// {
			// 	char* error_name;
			// 	jvmti_env->GetErrorName(error, &error_name);

			// 	jclass declaringType;

			// 	check_jvmti_error(jvmti_env, 
			// 		jvmti_env->GetMethodDeclaringClass(method, &declaringType), 
			// 		"Unable to get method declaring class");

			// 	std::string klass = jiim::get_class_signature(jvmti_env, declaringType);

			// 	char *name;
			// 	char *sig;
			// 	char *gsig;
			// 	check_jvmti_error(jvmti_env, 
			// 		jvmti_env->GetMethodName(method, &name, &sig, &gsig), 
			// 		"Unable to get method name");

			// 	std::stringstream stream;
			// 	stream << klass << "#" << name << ((gsig == NULL) ? sig : gsig);

			// 	if(name) jvmti_env->Deallocate((unsigned char*)(name));
			// 		if(sig) jvmti_env->Deallocate((unsigned char*)(sig));
			// 		if(gsig) jvmti_env->Deallocate((unsigned char*)(gsig));

			// 	std::cerr << stream.str() << "::" << entry.name << " -- " << error_name << std::endl;

			// 	return tagged_jvalue('N', value);
			// }

			return tagged_jvalue(entry.signature[0], value);
		}

		tagged_jvalue get_field_by_signature(jvmtiEnv* jvmti_env, JNIEnv* jni_env, jclass cls, jfieldID field, char* sig, unsigned int mods)
		{
			bool is_static = mods & 0x0008;
			jvalue value;
			try 
			{
				switch(sig[0])
				{
					case 'B': { value.b = is_static ? jni_env->GetStaticByteField(cls, field) : jni_env->GetByteField(cls, field); } break;
					case 'C': { value.c = is_static ? jni_env->GetStaticCharField(cls, field) : jni_env->GetCharField(cls, field); } break;
					case 'I': { value.i = is_static ? jni_env->GetStaticIntField(cls, field) : jni_env->GetIntField(cls, field); } break;
					case 'S': { value.s = is_static ? jni_env->GetStaticShortField(cls, field) : jni_env->GetShortField(cls, field); } break;
					case 'Z': { value.z = is_static ? jni_env->GetStaticBooleanField(cls, field) : jni_env->GetBooleanField(cls, field); } break;
					case 'D': { value.d = is_static ? jni_env->GetStaticDoubleField(cls, field) : jni_env->GetDoubleField(cls, field); } break;
					case 'F': { value.f = is_static ? jni_env->GetStaticFloatField(cls, field) : jni_env->GetFloatField(cls, field); } break;
					case 'J': { value.j = is_static ? jni_env->GetStaticLongField(cls, field) : jni_env->GetLongField(cls, field); } break;
					case 'L': { value.l = is_static ? jni_env->GetStaticObjectField(cls, field) : jni_env->GetObjectField(cls, field); } break;
					default: ;
				}
	
				return tagged_jvalue(sig[0], value);
			}
			catch(...)
			{
				return tagged_jvalue('N', value);
			}
		}

		std::unordered_map<std::string, tagged_jvalue> get_variable_bindings(jvmtiEnv* jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, const std::vector<std::string>& targets)
		{
			std::unordered_map<std::string, tagged_jvalue> bindings;

			jint local_variable_entry_count;
			jvmtiLocalVariableEntry* local_variable_table;

			check_jvmti_error(jvmti_env, 
				jvmti_env->GetLocalVariableTable(method, &local_variable_entry_count, &local_variable_table),
				"Unable to get variable table");

			for(int i = 0; i < local_variable_entry_count; i++)
			{
				jint field_entry_count;
				jfieldID* field_table;

				auto entry = local_variable_table[i];
				auto v = get_value_by_signature(jvmti_env, thread, method, local_variable_table[i]);
				if(v.tag == 'N') continue;

				bindings.emplace(entry.name, v);

				if(entry.signature[0] == 'L')
				{
					bool target_found = false;

					for(auto target: targets)
					{
						if(boost::starts_with(entry.signature, target))
						{
							target_found = true;
							break;
						}
					}

					if(target_found)
					{
						jclass cls;

						if(v.value.l != nullptr)
						{
							cls = jni_env->GetObjectClass(v.value.l);

							jvmti_env->GetClassFields(cls, &field_entry_count, &field_table);

							for(int j = 0; j < field_entry_count; j++)
							{
								char* name;
								char* sig;
								jint mods;
								jvmti_env->GetFieldName(cls, field_table[j], &name, &sig, nullptr);
								jvmti_env->GetFieldModifiers(cls, field_table[j], &mods);

								auto field_value = get_field_by_signature(jvmti_env, jni_env, cls, field_table[j], sig, mods);
								if(field_value.tag == 'N') continue;

								std::stringstream ss;
								ss << entry.name << "." << name;
								bindings.emplace(ss.str(), field_value);

								jvmti_env->Deallocate((unsigned char*)name);

								if(sig != nullptr)
									jvmti_env->Deallocate((unsigned char*)sig);
							}
						}
					}
				}
			}

			if(local_variable_table != nullptr)
			   	jvmti_env->Deallocate((unsigned char*)(local_variable_table));

			return bindings;
		}		
	}
}
