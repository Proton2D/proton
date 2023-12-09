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
#define PT_CORE_TRACE(...)    ::proton::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define PT_CORE_INFO(...)     ::proton::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define PT_CORE_WARN(...)     ::proton::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define PT_CORE_ERROR(...)    ::proton::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define PT_CORE_CRITICAL(...) ::proton::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define PT_CORE_TRACE_FUNCSIG(...)    PT_CORE_TRACE(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_CORE_INFO_FUNCSIG(...)     PT_CORE_INFO(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_CORE_WARN_FUNCSIG(...)     PT_CORE_WARN(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_CORE_ERROR_FUNCSIG(...)    PT_CORE_ERROR(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_CORE_CRITICAL_FUNCSIG(...) PT_CORE_CRITICAL(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)

// Client log macros
#define PT_TRACE(...)         ::proton::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define PT_INFO(...)          ::proton::Logger::GetClientLogger()->info(__VA_ARGS__)
#define PT_WARN(...)          ::proton::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define PT_ERROR(...)         ::proton::Logger::GetClientLogger()->error(__VA_ARGS__)
#define PT_CRITICAL(...)      ::proton::Logger::GetClientLogger()->critical(__VA_ARGS__)

#define PT_TRACE_FUNCSIG(...)    PT_TRACE(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_INFO_FUNCSIG(...)     PT_INFO(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_WARN_FUNCSIG(...)     PT_WARN(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_ERROR_FUNCSIG(...)    PT_ERROR(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
#define PT_CRITICAL_FUNCSIG(...) PT_CRITICAL(proton::LoggerUtils::FormatFuncSignature(__FUNCSIG__) + __VA_ARGS__)
