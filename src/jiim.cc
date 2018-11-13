
#include <jni.h>
#include <jvmti.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "jeeves.h"

void JNICALL
MethodEntry(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread,
            jmethodID method) {

}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
	jiim::jeeves jeeves(vm);

	auto resp = jeeves.prepare()
		->listens_to_method_entries()
		->listens_to_method_exits()
		->listens_to_breakpoints()
		->listens_to_single_steps()
		->sends_method_entry_to(&MethodEntry)
		->boot();

	assert(resp == JVMTI_ERROR_NONE);
	return resp;
}
