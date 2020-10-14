#pragma once
#include "fty/convert.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <functional>
#include <memory>

// =====================================================================================================================

// clang-format off
#define logDbg(...)                  _log(fty::Logger::Level::Debug, true, __VA_ARGS__)
#define logInfo(...)                 _log(fty::Logger::Level::Info, true, __VA_ARGS__)
#define logFatal(...)                _log(fty::Logger::Level::Fatal, true, __VA_ARGS__)
#define logError(...)                _log(fty::Logger::Level::Error, true, __VA_ARGS__)
#define logWarn(...)                 _log(fty::Logger::Level::Warn, true, __VA_ARGS__)
#define logTrace(...)                _log(fty::Logger::Level::Trace, true, __VA_ARGS__)

#define logDbgIf(condition, ...)     _log(fty::Logger::Level::Debug, condition, __VA_ARGS__)
#define logInfoIf(condition, ...)    _log(fty::Logger::Level::Info, condition, __VA_ARGS__)
#define logFatalIf(condition, ...)   _log(fty::Logger::Level::Fatal, condition, __VA_ARGS__)
#define logErrorIf(condition, ...)   _log(fty::Logger::Level::Error, condition, __VA_ARGS__)
#define logWarnIf(condition, ...)    _log(fty::Logger::Level::Warn, condition, __VA_ARGS__)
#define logTraceIf(condition, ...)   _log(fty::Logger::Level::Trace, condition, __VA_ARGS__)
// clang-format on

// =====================================================================================================================

// old macroses support
// clang-format off
#define log_debug(...)      _logstr(fty::Logger::logInstance(), fty::Logger::Level::Debug, fty::details::sprintf(__VA_ARGS__))
#define log_info(...)       _logstr(fty::Logger::logInstance(), fty::Logger::Level::Info,  fty::details::sprintf(__VA_ARGS__))
#define log_fatal(...)      _logstr(fty::Logger::logInstance(), fty::Logger::Level::Fatal, fty::details::sprintf(__VA_ARGS__))
#define log_error(...)      _logstr(fty::Logger::logInstance(), fty::Logger::Level::Error, fty::details::sprintf(__VA_ARGS__))
#define log_warning(...)    _logstr(fty::Logger::logInstance(), fty::Logger::Level::Warn,  fty::details::sprintf(__VA_ARGS__))
#define log_trace(...)      _logstr(fty::Logger::logInstance(), fty::Logger::Level::Trace, fty::details::sprintf(__VA_ARGS__))

#define log_debug_log(inst, ...)      _logstr(inst, fty::Logger::Level::Debug, fty::details::sprintf(__VA_ARGS__))
#define log_info_log(inst, ...)       _logstr(inst, fty::Logger::Level::Info,  fty::details::sprintf(__VA_ARGS__))
#define log_fatal_log(inst, ...)      _logstr(inst, fty::Logger::Level::Fatal, fty::details::sprintf(__VA_ARGS__))
#define log_error_log(inst, ...)      _logstr(inst, fty::Logger::Level::Error, fty::details::sprintf(__VA_ARGS__))
#define log_warning_log(inst, ...)    _logstr(inst, fty::Logger::Level::Warn,  fty::details::sprintf(__VA_ARGS__))
#define log_trace_log(inst, ...)      _logstr(inst, fty::Logger::Level::Trace, fty::details::sprintf(__VA_ARGS__))
// clang-format on

// =====================================================================================================================

#define _log(level, condition, ...)                                                                                    \
    !fty::Logger::logInstance().isSupports(level) || !(condition)                                                      \
        ? void(0)                                                                                                      \
        : fty::Logger::Void() & fty::Logger(fty::Logger::logInstance(), level, __FILE__, __LINE__, __func__)           \
                                    << fmt::format("" __VA_ARGS__)

#define _logstr(inst, level, str)                                                                                      \
    !fty::Logger::logInstance().isSupports(level)                                                                      \
        ? void(0)                                                                                                      \
        : fty::Logger::Void() & fty::Logger(inst, level, __FILE__, __LINE__, __func__) << str

namespace fty {

// =====================================================================================================================

class Logger
{
public:
    enum class Level
    {
        Off,
        Fatal,
        Error,
        Warn,
        Info,
        Debug,
        Trace
    };

    struct Log
    {
        Level       level;
        std::string file;
        int         line;
        std::string func;
        std::string content;
    };

    class Instance
    {
    public:
        using Callback = std::function<void(const Log& log)>;

    public:
        Instance(const std::string& compName, const std::string& configFile = {});

    public:
        Callback& callBackFunction();
        void      setCallback(Callback&& callback);
        bool      isSupports(Level level);
        void      setLogLevel(Level lvl);
    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };

public:
    Logger(Instance& inst, Level level, const std::string& file, int line, const std::string& func,
        const std::string& cnt = {});
    ~Logger();

    Logger(const Logger&) = delete;
    Logger(Logger&&)      = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    template <typename T>
    Logger& operator<<(const T& val);

public:
    static void      setLogInstance(const std::string& instName, const std::string& config = {});
    static Instance& logInstance();
    static void      setLogLevel(Level level);

public:
    struct nowhitespace
    {
    };

public:
    struct Void
    {
        // This has to be an operator with a precedence lower than << but higher
        // than ?:
        void operator&(Logger&)
        {
        }
    };

private:
    static std::unique_ptr<Instance>& m_inst;
    bool                              m_inswhite = true;
};


// =====================================================================================================================

namespace details {

    template <typename... Args>
    inline std::string sprintf(const Args&... args)
    {
        std::string buf;
        auto        size = std::snprintf(nullptr, 0, args...) + 1;
        buf.resize(size_t(size));
        std::snprintf(buf.data(), buf.size(), args...);
        return buf;
    }

} // namespace details
// =====================================================================================================================

/*template <typename T>
Logger& Logger::operator<<(const T& val)
{
    auto addText = [&](const std::string& str) {
        if (m_inswhite && !m_log.content.empty()) {
            m_log.content += " ";
        }
        m_log.content += str;
    };

    addText(convert<std::string>(val));
    if constexpr (fty::is_map<T>::value) {
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& it : val) {
            ss << (!first ? ", " : "") << "{" << it.first << " : " << it.second << "}";
            first = false;
        }
        ss << "}";
        addText(ss.str());
    } else if constexpr (fty::is_list<T>::value) {
        std::stringstream ss;
        ss << "[";
        bool first = true;
        for (const auto& it : val) {
            ss << (!first ? ", " : "") << it;
            first = false;
        }
        ss << "]";
        addText(ss.str());
    } else if constexpr (std::is_same_v<T, nowhitespace>) {
        m_inswhite = false;
    } else {
        addText(fty::convert<std::string>(val));
    }
    return *this;
}*/

} // namespace fty

// =====================================================================================================================

/*template <typename T>
struct fmt::formatter<T, std::enable_if_t<fty::has_stream_op<T>::value, char>> : fmt::formatter<std::string>
{
    template <typename FormatCtx>
    auto format(const T& cls, FormatCtx& ctx)
    {
        fty::Logger::Log currentLog;
        {
            fty::Logger log;
            log.setCallback([&](const fty::Logger::Log& log) {
                currentLog = log;
            });
            cls.operator<<(log);
        }

        std::cerr << "!!!!! " << currentLog.content << "\n";
        return fmt::formatter<std::string>::format(currentLog.content, ctx);
    }
};*/
