#include "Krystal.Log/ILogger.hpp"
#define SPDLOG_HEADER_ONLY
#include <cassert>
#include <chrono>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace
{
  spdlog::level::level_enum ToSpdLogLevel(Krys::Log::Level level) noexcept
  {
    switch (level)
    {
      case Krys::Log::Level::Trace:    return spdlog::level::trace;
      case Krys::Log::Level::Debug:    return spdlog::level::debug;
      case Krys::Log::Level::Info:     return spdlog::level::info;
      case Krys::Log::Level::Warn:     return spdlog::level::warn;
      case Krys::Log::Level::Error:    return spdlog::level::err;
      case Krys::Log::Level::Critical: return spdlog::level::critical;
      default:
      case Krys::Log::Level::Off:      return spdlog::level::off;
    }
  }
}

namespace Krys::Log
{
  using namespace spdlog;

  class SpdlogLogger : public ILogger
  {
    LoggerSettings _settings;
    logger _logger;

  public:
    SpdlogLogger(const LoggerSettings &settings) : _settings(settings), _logger(settings.Name)
    {
      if (_settings.OutputToConsole)
        AddConsoleSink();

      if (!_settings.FilePath.empty())
        AddFileSink(_settings.FilePath);

      _logger.set_pattern("[%H:%M:%S.%e] [%l] %v", pattern_time_type::local);
      _logger.set_level(ToSpdLogLevel(_settings.Level));
    }

    void SetLevel(Level level) noexcept override
    {
      _logger.set_level(ToSpdLogLevel(level));
      _settings.Level = level;
    }

    KRYS_NODISCARD Level GetLevel() const noexcept
    {
      return _settings.Level;
    }

    void Flush() noexcept
    {
      _logger.flush();
    }

    void AddConsoleSink() noexcept
    {
      auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      consoleSink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
      _logger.sinks().push_back(consoleSink);
    }

    void AddFileSink(const string &path) noexcept
    {
      auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, true);
      fileSink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
      _logger.sinks().push_back(fileSink);
    }

  protected:
    void LogImpl(Level level, const string &message) noexcept override
    {
      if (level < _settings.Level)
        return;

      // source_loc spdlogLoc(loc.file_name(), loc.line(), loc.function_name());
      _logger.log(std::chrono::system_clock::now(), source_loc {}, ToSpdLogLevel(level), message);
    }
  };

  Expected<UniquePtr<ILogger>> CreateLogger(const LoggerSettings &settings) noexcept
  {
    if (settings.Name.empty())
      return Unexpected("Logger name cannot be empty.");

    if (settings.FilePath.empty() && !settings.OutputToConsole)
      return Unexpected("At least one sink (console or file) must be enabled.");

    try
    {
      return Expected<UniquePtr<ILogger>>(CreateUnique<SpdlogLogger>(settings));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }

  static ILogger *GlobalLogger = nullptr;

  void SetGlobalLogger(const UniquePtr<ILogger> &logger) noexcept
  {
    GlobalLogger = logger.get();
  }

  ILogger *GetGlobalLogger() noexcept
  {
    assert(GlobalLogger != nullptr && "Global logger is not set!");
    return GlobalLogger;
  }
}