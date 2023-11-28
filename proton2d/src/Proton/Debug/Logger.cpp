// 
// Wrapper for spdlog with logging macros
// From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.cpp
// 
#include "ptpch.h"
#include "Proton/Debug/Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace proton {

	Shared<spdlog::logger> Logger::s_CoreLogger;
	Shared<spdlog::logger> Logger::s_ClientLogger;

	void Logger::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(MakeShared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(MakeShared<spdlog::sinks::basic_file_sink_mt>("proton.log", true));

		logSinks[0]->set_pattern("%^[%T] [%l] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_CoreLogger = MakeShared<spdlog::logger>("PROTON", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = MakeShared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}
}
