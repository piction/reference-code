#include "log.h"


//Loglevels: trace,debug,info,warn,err,critical,off
std::shared_ptr<spdlog::logger> Log::_logger;


void Log::Init( std::string logFolder)
{
    std::string fileName = "systemController.log";
    
    std::string consolePattern = "[system-Ctrl-%t][%H:%M:%S:%e / %o] [%^%l%$] %v";

    std::string filePattern = "[%t][%H:%M:%S:%e / %o] [%^%l%$] %v";
    auto max_size = 1048576 * 2; // 2MB size
    auto max_files = 3; // 3 rolling files

        
    std::string mkdirCmd = std::string("mkdir -p ") + logFolder;
    const int dir_err = system( mkdirCmd.c_str());
    
    std::string filePath = logFolder + "/" + fileName;



    spdlog::init_thread_pool(8192, 1);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
    stdout_sink->set_level(spdlog::level::trace);
    stdout_sink->set_pattern(consolePattern);
    

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath,max_size, max_files);
    rotating_sink->set_level(spdlog::level::trace);
    rotating_sink->set_pattern(filePattern);
    

    std::vector<spdlog::sink_ptr> sinks {stdout_sink, rotating_sink};
    _logger = std::make_shared<spdlog::async_logger>("system-controller", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    _logger->set_level(spdlog::level::trace);
    _logger->flush_on(spdlog::level::warn);

   // spdlog::flush_every(std::chrono::seconds(3));

    
}
void Log::Init() {
    Init("log");
}
