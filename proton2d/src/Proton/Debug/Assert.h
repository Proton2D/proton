// 
// Assert macros that trigger debugbreak
// From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Assert.h
// 
#pragma once

#include "Proton/Core/Base.h"
#include "Proton/Debug/Logger.h"
#include <filesystem>

#ifdef PT_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define PT_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { PT##type##ERROR(msg, __VA_ARGS__); PT_DEBUGBREAK(); } }
#define PT_INTERNAL_ASSERT_WITH_MSG(type, check, ...) PT_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define PT_INTERNAL_ASSERT_NO_MSG(type, check) PT_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PT_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define PT_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define PT_INTERNAL_ASSERT_GET_MACRO(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PT_INTERNAL_ASSERT_WITH_MSG, PT_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define PT_ASSERT(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define PT_CORE_ASSERT(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define PT_ASSERT(...)
#define PT_CORE_ASSERT(...)
#endif
