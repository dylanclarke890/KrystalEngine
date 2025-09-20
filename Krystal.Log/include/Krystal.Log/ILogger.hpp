#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
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
    string Name {"Default"};
    bool OutputToConsole {true};
    string FilePath {"logs/log.txt"};
    Level Level {Level::Info};
  };

  class ILogger
  {
  public:
    NO_COPY_MOVE(ILogger)

    virtual ~ILogger() noexcept = default;

    template <typename... Args>
    void Log(Level level, stringview fmt, Args &&...args)
    {
      auto message = std::vformat(fmt, std::make_format_args(args...));
      LogImpl(level, message);
    }

#define LOG_METHOD(level)                                                                                    \
  template <typename... Args>                                                                                \
  void level(stringview fmt, Args &&...args)                                                                 \
  {                                                                                                          \
    Log(Level::##level, fmt, std::forward<Args>(args)...);                                                   \
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