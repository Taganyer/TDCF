//
// Created by taganyer on 25-7-3.
//
#pragma once

#include <tinyBackend/Base/SystemLog.hpp>

namespace test {

    constexpr char global_log_path[] = "/home/taganyer/Code/Clion_project/TDCF/global_logs";

    extern LogSystem::SystemLog global_logger;


#define T_TRACE TRACE(global_logger)

#define T_DEBUG DEBUG(global_logger)

#define T_INFO INFO(global_logger)

#define T_WARN WARN(global_logger)

#define T_ERROR ERROR(global_logger)

#define T_FATAL FATAL(global_logger)

}
