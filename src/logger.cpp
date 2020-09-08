#include "fty/logger.h"
#include <log4cplus/configurator.h>
#include <log4cplus/logger.h>

namespace fty {

// =====================================================================================================================

class Logger::Instance::Impl
{
public:
    Impl(const std::string& compName, const std::string& configFile)
    {
        // initialize log4cplus
        log4cplus::initialize();

        // Create logger
        m_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(compName));
        if (auto env = getenv("BIOS_LOG_LEVEL")) {
            if (auto level = levelFromString(env)) {
                m_logger.setLogLevel(*level);
            } else {
                m_logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
            }
        }

        const char* varEnv = getenv("BIOS_LOG_PATTERN");
        if (varEnv && !std::string(varEnv).empty()) {
            m_layoutPattern = varEnv;
        }
    }

private:
    static std::optional<log4cplus::LogLevel> levelFromString(const std::string& level)
    {
        if (level == "LOG_TRACE") {
            return log4cplus::TRACE_LOG_LEVEL;
        } else if (level == "LOG_DEBUG") {
            return log4cplus::DEBUG_LOG_LEVEL;
        } else if (level == "LOG_INFO") {
            return log4cplus::INFO_LOG_LEVEL;
        } else if (level == "LOG_WARNING") {
            return log4cplus::WARN_LOG_LEVEL;
        } else if (level == "LOG_ERR") {
            return log4cplus::ERROR_LOG_LEVEL;
        } else if (level == "LOG_CRIT") {
            return log4cplus::FATAL_LOG_LEVEL;
        } else if (level == "LOG_OFF") {
            return log4cplus::OFF_LOG_LEVEL;
        }
        return std::nullopt;
    }

private:
    using FileWatcher = std::unique_ptr<log4cplus::ConfigureAndWatchThread>;

    std::string       m_agentName;       // Name of the agent/component
    std::string       m_configFile;      // Path to the log configuration file if any
    std::string       m_layoutPattern;   // Layout pattern for logs
    log4cplus::Logger m_logger;          // log4cplus object to print logs
    FileWatcher       m_watchConfigFile; // Thread for watching modification of the log configuration file if any
};

// =====================================================================================================================

Logger::Instance::Instance(const std::string& compName, const std::string& configFile)
    : m_impl(new Impl(compName, configFile))
{
}

} // namespace fty
