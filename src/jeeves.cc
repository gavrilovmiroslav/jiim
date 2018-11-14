#include "jeeves.h"

namespace jiim {

jeeves* jeeves::accesses_timers()
{
	caps.can_get_current_thread_cpu_time = 1;
	caps.can_get_thread_cpu_time = 1;
	return this;
}

jeeves* jeeves::accesses_threads()
{
	caps.can_suspend = 1;
	caps.can_signal_thread = 1;
	caps.can_get_owned_monitor_info = 1;
	caps.can_get_owned_monitor_stack_depth_info = 1;
	caps.can_get_current_contended_monitor = 1;
	return this;
}

jeeves* jeeves::accesses_stack_frames()
{
	caps.can_pop_frame = 1;
	caps.can_generate_frame_pop_events = 1;
	return this;
}

jeeves* jeeves::accesses_early_returns()
{
	caps.can_force_early_return = 1;
	return this;
}

jeeves* jeeves::accesses_heap()
{
	caps.can_tag_objects = 1;
	return this;
}

jeeves* jeeves::accesses_local_variables()
{
	caps.can_access_local_variables = 1;
	return this;
}

jeeves* jeeves::accesses_breakpoints()
{
	caps.can_generate_breakpoint_events	= 1;
	return this;
}

jeeves* jeeves::accesses_watched_fields()
{
	caps.can_generate_field_access_events = 1;
	caps.can_generate_field_modification_events = 1;
	return this;
}

jeeves* jeeves::accesses_classes()
{
	caps.can_get_source_file_name = 1;
	caps.can_maintain_original_method_order = 1;
	caps.can_get_constant_pool = 1;
	caps.can_redefine_any_class = 1;
	caps.can_redefine_classes = 1;
	caps.can_retransform_classes = 1;
	caps.can_retransform_any_class = 1;
	caps.can_get_source_debug_extension	= 1;
	caps.can_get_monitor_info = 1;
	return this;
}

jeeves* jeeves::accesses_fields()
{
	caps.can_get_synthetic_attribute = 1;
	return this;
}

jeeves* jeeves::accesses_methods()
{
	caps.can_get_line_numbers = 1;
	caps.can_access_local_variables = 1;
	caps.can_get_bytecodes = 1;
	caps.can_get_synthetic_attribute = 1;
	caps.can_set_native_method_prefix = 1;
	return this;
}

// --------

jeeves* jeeves::listens_to_field_modifications()
{
	caps.can_generate_field_modification_events = 1;
	notifs.insert(JVMTI_EVENT_FIELD_MODIFICATION);
	return this;
}

jeeves* jeeves::listens_to_field_access()
{
	caps.can_generate_field_access_events = 1;
	notifs.insert(JVMTI_EVENT_FRAME_POP);
	return this;
}

jeeves* jeeves::listens_to_single_steps()
{
	caps.can_generate_single_step_events = 1;
	notifs.insert(JVMTI_EVENT_SINGLE_STEP);
	return this;
}

jeeves* jeeves::listens_to_breakpoints()
{
	caps.can_generate_breakpoint_events = 1;
	notifs.insert(JVMTI_EVENT_BREAKPOINT);
	return this;
}

jeeves* jeeves::listens_to_frame_pops()
{
	caps.can_generate_frame_pop_events = 1;
	notifs.insert(JVMTI_EVENT_FRAME_POP);
	return this;
}

jeeves* jeeves::listens_to_method_entries()
{
	caps.can_generate_method_entry_events = 1;
	notifs.insert(JVMTI_EVENT_METHOD_ENTRY);
	return this;
}

jeeves* jeeves::listens_to_method_exits()
{
	caps.can_generate_method_exit_events = 1;
	notifs.insert(JVMTI_EVENT_METHOD_EXIT);
	return this;
}

jeeves* jeeves::listens_to_native_binds()
{
	caps.can_generate_native_method_bind_events = 1;
	notifs.insert(JVMTI_EVENT_NATIVE_METHOD_BIND);
	return this;
}

jeeves* jeeves::listens_to_exceptions()
{
	caps.can_generate_exception_events = 1;
	notifs.insert(JVMTI_EVENT_EXCEPTION);
	return this;
}

jeeves* jeeves::listens_to_exceptions_caught()
{
	caps.can_generate_exception_events = 1;
	notifs.insert(JVMTI_EVENT_EXCEPTION_CATCH);
	return this;
}

jeeves* jeeves::listens_to_threads_start()
{
	notifs.insert(JVMTI_EVENT_THREAD_START);
	return this;
}

jeeves* jeeves::listens_to_threads_end()
{
	notifs.insert(JVMTI_EVENT_THREAD_END);
	return this;
}

jeeves* jeeves::listens_to_classes_load()
{
	notifs.insert(JVMTI_EVENT_CLASS_LOAD);
	return this;
}

jeeves* jeeves::listens_to_classes_prepare()
{
	notifs.insert(JVMTI_EVENT_CLASS_PREPARE);
	return this;
}

jeeves* jeeves::listens_to_classes_hook()
{
	caps.can_generate_all_class_hook_events = 1;
	notifs.insert(JVMTI_EVENT_CLASS_FILE_LOAD_HOOK);
	return this;
}

jeeves* jeeves::listens_to_vm_start()
{
	notifs.insert(JVMTI_EVENT_VM_START);
	return this;
}

jeeves* jeeves::listens_to_vm_init()
{
	notifs.insert(JVMTI_EVENT_VM_INIT);
	return this;
}

jeeves* jeeves::listens_to_vm_die()
{
	notifs.insert(JVMTI_EVENT_VM_DEATH);
	return this;
}

jeeves* jeeves::listens_to_compiled_method_loads()
{
	caps.can_generate_compiled_method_load_events = 1;
	notifs.insert(JVMTI_EVENT_COMPILED_METHOD_LOAD);
	return this;
}

jeeves* jeeves::listens_to_compiled_method_unloads()
{
	caps.can_generate_compiled_method_load_events = 1;
	notifs.insert(JVMTI_EVENT_COMPILED_METHOD_UNLOAD);
	return this;
}

jeeves* jeeves::listens_to_dynamic_code_generation()
{
	notifs.insert(JVMTI_EVENT_DYNAMIC_CODE_GENERATED);
	return this;
}

jeeves* jeeves::listens_to_data_dumps()
{
	notifs.insert(JVMTI_EVENT_DATA_DUMP_REQUEST);
	return this;
}

jeeves* jeeves::listens_to_garbage_collection()
{
	caps.can_generate_garbage_collection_events = 1;
	notifs.insert(JVMTI_EVENT_GARBAGE_COLLECTION_START);
	notifs.insert(JVMTI_EVENT_GARBAGE_COLLECTION_FINISH);
	return this;
}

// -------

jeeves* jeeves::sends_vm_init_to(jvmtiEventVMInit callback)
{
	listens_to_vm_init();
	callbacks.VMInit = *callback;
	return this;
}

jeeves* jeeves::sends_vm_death_to(jvmtiEventVMDeath callback)
{
	listens_to_vm_die();
	callbacks.VMDeath = *callback;
	return this;
}

jeeves* jeeves::sends_thread_start_to(jvmtiEventThreadStart callback)
{
	listens_to_threads_start();
	callbacks.ThreadStart = *callback;
	return this;
}

jeeves* jeeves::sends_thread_end_to(jvmtiEventThreadEnd callback)
{
	listens_to_threads_end();
	callbacks.ThreadEnd = *callback;
	return this;
}

jeeves* jeeves::sends_class_file_load_hook_to(jvmtiEventClassFileLoadHook callback)
{
	callbacks.ClassFileLoadHook = *callback;
	return this;
}

jeeves* jeeves::sends_class_load_to(jvmtiEventClassLoad callback)
{
	listens_to_classes_load();
	callbacks.ClassLoad = *callback;
	return this;
}

jeeves* jeeves::sends_class_prepare_to(jvmtiEventClassPrepare callback)
{
	listens_to_classes_prepare();
	callbacks.ClassPrepare = *callback;
	return this;
}

jeeves* jeeves::sends_vm_start_to(jvmtiEventVMStart callback)
{
	listens_to_vm_start();
	callbacks.VMStart = *callback;
	return this;
}

jeeves* jeeves::sends_exception_to(jvmtiEventException callback)
{
	listens_to_exceptions();
	callbacks.Exception = *callback;
	return this;
}

jeeves* jeeves::sends_exception_catch_to(jvmtiEventExceptionCatch callback)
{
	listens_to_exceptions_caught();
	callbacks.ExceptionCatch = *callback;
	return this;
}

jeeves* jeeves::sends_single_step_to(jvmtiEventSingleStep callback)
{
	listens_to_single_steps();
	callbacks.SingleStep = *callback;
	return this;
}

jeeves* jeeves::sends_frame_pop_to(jvmtiEventFramePop callback)
{
	listens_to_frame_pops();
	callbacks.FramePop = *callback;
	return this;
}

jeeves* jeeves::sends_breakpoint_to(jvmtiEventBreakpoint callback)
{
	listens_to_breakpoints();
	callbacks.Breakpoint = *callback;
	return this;
}

jeeves* jeeves::sends_field_access_to(jvmtiEventFieldAccess callback)
{
	listens_to_field_access();
	callbacks.FieldAccess = *callback;
	return this;
}

jeeves* jeeves::sends_field_modification_to(jvmtiEventFieldModification callback)
{
	listens_to_field_modifications();
	callbacks.FieldModification = *callback;
	return this;
}

jeeves* jeeves::sends_method_entry_to(jvmtiEventMethodEntry callback)
{
	listens_to_method_entries();
	callbacks.MethodEntry = *callback;
	return this;
}

jeeves* jeeves::sends_method_exit_to(jvmtiEventMethodExit callback)
{
	listens_to_method_exits();
	callbacks.MethodExit = *callback;
	return this;
}

jeeves* jeeves::sends_native_method_bind_to(jvmtiEventNativeMethodBind callback)
{
	listens_to_native_binds();
	callbacks.NativeMethodBind = *callback;
	return this;
}

jeeves* jeeves::sends_compiled_method_load_to(jvmtiEventCompiledMethodLoad callback)
{
	listens_to_compiled_method_loads();
	callbacks.CompiledMethodLoad = *callback;
	return this;
}

jeeves* jeeves::sends_compiled_method_unload_to(jvmtiEventCompiledMethodUnload callback)
{
	listens_to_compiled_method_unloads();
	callbacks.CompiledMethodUnload = *callback;
	return this;
}

jeeves* jeeves::sends_dynamic_code_generated_to(jvmtiEventDynamicCodeGenerated callback)
{
	listens_to_dynamic_code_generation();
	callbacks.DynamicCodeGenerated = *callback;
	return this;
}

jeeves* jeeves::sends_data_dump_request_to(jvmtiEventDataDumpRequest callback)
{
	listens_to_data_dumps();
	callbacks.DataDumpRequest = *callback;
	return this;
}

jeeves* jeeves::sends_garbage_collection_start_to(jvmtiEventGarbageCollectionStart callback)
{
	listens_to_garbage_collection();
	callbacks.GarbageCollectionStart = *callback;
	return this;
}

jeeves* jeeves::sends_garbage_collection_finish_to(jvmtiEventGarbageCollectionFinish callback)
{
	listens_to_garbage_collection();
	callbacks.GarbageCollectionFinish = *callback;
	return this;
}

}