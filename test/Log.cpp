//
// Created by taganyer on 25-7-3.
//

#include <test/Log.hpp>
#include <tinyBackend/Base/GlobalObject.hpp>

using namespace test;

using namespace LogSystem;

SystemLog test::global_logger(Global_ScheduledThread, Global_BufferPool,
                              global_log_path, TRACE);
