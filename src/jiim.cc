
#include <jni.h>
#include <jvmti.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <set>

#include "jeeves.h"

static bool mainReached = false;
static std::set<std::string> names;

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
	mainReached = false;
	jiim::jeeves jeeves(vm);

	jeeves.prepare()

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
			if(mainReached) {
				char* name;
				jvmti_env->GetMethodName(method, &name, nullptr, nullptr);

				names.insert(name);
			    jvmti_env->Deallocate((unsigned char*)(name));
			}
		})

		->sends_breakpoint_to([](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jlocation location) 
		{
			mainReached = true;
		})

		->boot();

	return 0;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
}