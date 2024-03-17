//
// Wrapper for spdlog with logging macros
// From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h
// 
#pragma once
#include "Proton/Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside external headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#define PT_ENABLE_FUNC_SIGNATURE_LOGGING 1

namespace proton {

	class Logger
	{
	public:
		static void Init();

		static Shared<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Shared<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Shared<spdlog::logger> s_CoreLogger;
		static Shared<spdlog::logger> s_ClientLogger;
	};

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

namespace proton::LoggerUtils {

	std::string FormatFuncSignature(const std::string& funcsig);

}

// Core log macros
#define _PT_CORE_TRACE(...)    ::proton::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define _PT_CORE_INFO(...)     ::proton::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define _PT_CORE_WARN(...)     ::proton::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define _PT_CORE_ERROR(...)    ::proton::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define _PT_CORE_CRITICAL(...) ::proton::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#if PT_ENABLE_FUNC_SIGNATURE_LOGGING
	#define PT_CORE_TRACE(...)    _PT_CORE_TRACE(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_CORE_INFO(...)     _PT_CORE_INFO(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_CORE_WARN(...)     _PT_CORE_WARN(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_CORE_ERROR(...)    _PT_CORE_ERROR(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_CORE_CRITICAL(...) _PT_CORE_CRITICAL(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#else
	#define PT_CORE_TRACE(...)    _PT_CORE_TRACE(__VA_ARGS__)
	#define PT_CORE_INFO(...)     _PT_CORE_INFO(__VA_ARGS__)
	#define PT_CORE_WARN(...)     _PT_CORE_WARN(__VA_ARGS__)
	#define PT_CORE_ERROR(...)    _PT_CORE_ERROR(__VA_ARGS__)
	#define PT_CORE_CRITICAL(...) _PT_CORE_CRITICAL(__VA_ARGS__)
#endif

// Client log macros
#define _PT_TRACE(...)         ::proton::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define _PT_INFO(...)          ::proton::Logger::GetClientLogger()->info(__VA_ARGS__)
#define _PT_WARN(...)          ::proton::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define _PT_ERROR(...)         ::proton::Logger::GetClientLogger()->error(__VA_ARGS__)
#define _PT_CRITICAL(...)      ::proton::Logger::GetClientLogger()->critical(__VA_ARGS__)

#if PT_ENABLE_FUNC_SIGNATURE_LOGGING
	#define PT_TRACE(...)         _PT_TRACE(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_INFO(...)          _PT_INFO(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_WARN(...)          _PT_WARN(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_ERROR(...)         _PT_ERROR(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
	#define PT_CRITICAL(...)      _PT_CRITICAL(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#else
	#define PT_TRACE(...)         _PT_TRACE(__VA_ARGS__)
	#define PT_INFO(...)          _PT_INFO(__VA_ARGS__)
	#define PT_WARN(...)          _PT_WARN(__VA_ARGS__)
	#define PT_ERROR(...)         _PT_ERROR(__VA_ARGS__)
	#define PT_CRITICAL(...)      _PT_CRITICAL(__VA_ARGS__)
#endif
