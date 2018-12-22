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

#define ASSERT_MSG(condition, message) do { \
	if (!(condition)) { printf("%s\n", (message)); } \
	assert ((condition)); } while(false)

std::string get_error_name(jvmtiEnv* jvmti, jvmtiError error, const std::string message) 
{
    char *error_name = NULL;
    jvmtiError error_ = jvmti->GetErrorName(error, &error_name);
    ASSERT_MSG(error_ == JVMTI_ERROR_NONE, "Another JVMTI ERROR while getting an error name");

    const std::string messageSeparator = message.empty() ? "" : "; ";
    auto name = boost::format("%s%sJVMTI ERROR: '%d' (%s)") % message % messageSeparator % error %
                (error_name == NULL ? "Unknown" : error_name);
   	jvmti->Deallocate((unsigned char*)(error_name));
    return name.str();
}


void check_jvmti_error(jvmtiEnv* jvmti, jvmtiError error, const std::string message) 
{
    ASSERT_MSG(error == JVMTI_ERROR_NONE, 
    	get_error_name(jvmti, error, message).c_str());
}
