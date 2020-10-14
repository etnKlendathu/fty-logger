#include "fty/logger.h"
#include <fty/expected.h>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/helpers/pointer.h>
#include <log4cplus/logger.h>
#include <unistd.h>

namespace fty {

// =====================================================================================================================

class Logger::Instance::Impl
{
private:
    static constexpr const char* ENV_LOG_LEVEL   = "BIOS_LOG_LEVEL";
    static constexpr const char* ENV_LOG_PATTERN = "BIOS_LOG_PATTERN";

public:
    Impl(const std::string& compName, const std::string& configFile)
    {
        // initialize log4cplus
        log4cplus::initialize();

        // Create logger
        m_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(compName));

        // Set initial log level from environment
        if (auto env = envVar(ENV_LOG_LEVEL)) {
            if (auto level = levelFromString(*env)) {
                m_logger.setLogLevel(*level);
            } else {
                m_logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
            }
        }

        // Set initial log pattern from environment
        if (auto varEnv = envVar(ENV_LOG_PATTERN)) {
            m_layoutPattern = *varEnv;
        }

        // Read config file
        if (access(configFile.c_str(), R_OK) == 0) {
            // Load the file
            log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(configFile));
            // Start the thread watching the modification of the log config file
            m_watchConfigFile.reset(new log4cplus::ConfigureAndWatchThread(configFile.c_str(), 60000));
        } else {
            // Create console appender
            auto append = new log4cplus::ConsoleAppender(true, true);
            // Create and affect layout
            append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(m_layoutPattern)));
            append->setName(LOG4CPLUS_TEXT("Console" + m_agentName));

            // Add appender to logger
            m_logger.addAppender(log4cplus::helpers::SharedObjectPtr<log4cplus::Appender>(append));

            //log_error_log(*this,"File %s can't be accessed with read rights; this process will not monitor whether it becomes available later",_configFile.c_str());
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

    static Expected<std::string> envVar(const std::string& name)
    {
        const char* varEnv = std::getenv(name.c_str());
        if (!varEnv || varEnv[0] == '\0') {
            return unexpected("Variable {} is not set or empty", name);
        }
        return std::string(varEnv);
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
