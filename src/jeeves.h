
#pragma once

#include <jni.h>
#include <jvmti.h>
#include <vector>
#include <functional>
#include <set>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <functional>
using namespace std::placeholders;

namespace jiim {

class jeeves {
protected:
	JavaVM *vm;
	jvmtiEnv *env;
	JNIEnv *jni;
	jvmtiCapabilities caps;
	jvmtiEventCallbacks callbacks;
	std::set<jvmtiEvent> notifs;

public:

	inline JNIEnv *get_jni() const 
	{
		return jni;
	}

	inline jvmtiEnv *get_env() const
	{
		return env;
	}

	jeeves(JavaVM *_vm)
		: vm(_vm)
	{
		(void)memset(&caps, 0, sizeof(jvmtiCapabilities));
		(void)memset(&callbacks, 0, sizeof(callbacks));

		jint error = vm->GetEnv((void**) &env, JVMTI_VERSION_1_1);
		if(error != JNI_OK)
		{
			std::cout << "Cannot get starting JVMTI environment." << std::endl;
			exit(-1);
		}
	}

	jvmtiError boot()
	{
		jvmtiError error = env->AddCapabilities(&caps);
		if(error != JVMTI_ERROR_NONE)
			return error;

		error = env->SetEventCallbacks(&callbacks,(jint)sizeof(callbacks));
		if(error != JVMTI_ERROR_NONE)
			return error;

		for(auto notif: notifs)
		{
			error = env->SetEventNotificationMode(JVMTI_ENABLE, notif, nullptr);
			if(error != JVMTI_ERROR_NONE)
				return error;
		}

		return error;
	}

	~jeeves()
	{}

	inline jeeves* prepare() {
		return this;
	}

	jeeves* accesses_timers();
	jeeves* accesses_threads();
	jeeves* accesses_stack_frames();
	jeeves* accesses_early_returns();
	jeeves* accesses_heap();
	jeeves* accesses_local_variables();
	jeeves* accesses_breakpoints();
	jeeves* accesses_watched_fields();
	jeeves* accesses_classes();
	jeeves* accesses_fields();
	jeeves* accesses_methods();

	jeeves* listens_to_field_modifications();
	jeeves* listens_to_field_access();
	jeeves* listens_to_single_steps();
	jeeves* listens_to_breakpoints();
	jeeves* listens_to_frame_pops();
	jeeves* listens_to_method_entries();
	jeeves* listens_to_method_exits();
	jeeves* listens_to_native_binds();
	jeeves* listens_to_exceptions();
	jeeves* listens_to_exceptions_caught();
	jeeves* listens_to_threads_start();
	jeeves* listens_to_threads_end();
	jeeves* listens_to_classes_load();
	jeeves* listens_to_classes_prepare();
	jeeves* listens_to_classes_hook();
	jeeves* listens_to_vm_start();
	jeeves* listens_to_vm_init();
	jeeves* listens_to_vm_die();
	jeeves* listens_to_compiled_method_loads();
	jeeves* listens_to_compiled_method_unloads();
	jeeves* listens_to_dynamic_code_generation();
	jeeves* listens_to_data_dumps();
	jeeves* listens_to_garbage_collection();

	jeeves* sends_vm_init_to(jvmtiEventVMInit);
    jeeves* sends_vm_death_to(jvmtiEventVMDeath);
    jeeves* sends_thread_start_to(jvmtiEventThreadStart);
    jeeves* sends_thread_end_to(jvmtiEventThreadEnd);
    jeeves* sends_class_file_load_hook_to(jvmtiEventClassFileLoadHook);
    jeeves* sends_class_load_to(jvmtiEventClassLoad);
    jeeves* sends_class_prepare_to(jvmtiEventClassPrepare);
    jeeves* sends_vm_start_to(jvmtiEventVMStart);
    jeeves* sends_exception_to(jvmtiEventException);
    jeeves* sends_exception_catch_to(jvmtiEventExceptionCatch);
    jeeves* sends_single_step_to(jvmtiEventSingleStep);
    jeeves* sends_frame_pop_to(jvmtiEventFramePop);
    jeeves* sends_breakpoint_to(jvmtiEventBreakpoint);
    jeeves* sends_field_access_to(jvmtiEventFieldAccess);
    jeeves* sends_field_modification_to(jvmtiEventFieldModification);
    jeeves* sends_method_entry_to(jvmtiEventMethodEntry);
    jeeves* sends_method_exit_to(jvmtiEventMethodExit);
    jeeves* sends_native_method_bind_to(jvmtiEventNativeMethodBind);
    jeeves* sends_compiled_method_load_to(jvmtiEventCompiledMethodLoad);
    jeeves* sends_compiled_method_unload_to(jvmtiEventCompiledMethodUnload);
    jeeves* sends_dynamic_code_generated_to(jvmtiEventDynamicCodeGenerated);
    jeeves* sends_data_dump_request_to(jvmtiEventDataDumpRequest);
    jeeves* sends_garbage_collection_start_to(jvmtiEventGarbageCollectionStart);
    jeeves* sends_garbage_collection_finish_to(jvmtiEventGarbageCollectionFinish);
};

}