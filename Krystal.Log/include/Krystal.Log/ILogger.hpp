#pragma once

#include <Krystal.Core/Core.hpp>

#include <format>

namespace Krys::Log
{
  struct LoggerSettings;
  class ILogger;

  NO_DISCARD Expected<Unique<ILogger>> CreateLogger(const LoggerSettings &settings) noexcept;

  void SetGlobalLogger(const Unique<ILogger> &logger) noexcept;

  NO_DISCARD ILogger *GetGlobalLogger() noexcept;

  enum class Level : uint8
  {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
  };

  struct LoggerSettings
  {
    string Name;
    bool OutputToConsole;
    string FilePath;
    Level Level;

    static constexpr LoggerSettings Default()
    {
      LoggerSettings settings {};
      settings.Name = "Default";
      settings.OutputToConsole = true;
      settings.FilePath = "logs/log.txt";
      settings.Level = Level::Info;
      return settings;
    }
  };

  class ILogger
  {
  public:
    NO_COPY_MOVE(ILogger)

    virtual ~ILogger() noexcept = default;

#define LOG_METHOD(level)                                                                                    \
  template <typename... Args>                                                                                \
  void level(stringview fmt, Args &&...args)                                                                 \
  {                                                                                                          \
    Log(Level::##level, fmt, std::forward<Args>(args)...);                                                   \
  }

    template <typename... Args>
    void Log(Level level, stringview fmt, Args &&...args)
    {
      auto message = std::vformat(fmt, std::make_format_args(args...));
      LogImpl(level, message);
    }

    LOG_METHOD(Trace)
    LOG_METHOD(Debug)
    LOG_METHOD(Info)
    LOG_METHOD(Warn)
    LOG_METHOD(Error)
    LOG_METHOD(Critical)

#undef LOG_METHOD

    virtual void SetLevel(Level level) noexcept = 0;

    NO_DISCARD virtual Level GetLevel() const noexcept = 0;

    virtual void Flush() noexcept = 0;

  protected:
    ILogger() noexcept = default;

    virtual void LogImpl(Level level, const string &message) noexcept = 0;
  };
}