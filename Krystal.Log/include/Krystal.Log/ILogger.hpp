#pragma once

#include <Krystal.Core/Core.hpp>

#include <format>

namespace Krys::Log
{
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

    virtual void AddConsoleSink() noexcept = 0;

    virtual void AddFileSink(const string &path) noexcept = 0;

  protected:
    ILogger() noexcept = default;

    virtual void LogImpl(Level level, const string &message) noexcept = 0;
  };

  NO_DISCARD Unique<ILogger> CreateLogger(const string &name) noexcept;

  void SetGlobalLogger(const Unique<ILogger> &logger) noexcept;

  NO_DISCARD ILogger *GetGlobalLogger() noexcept;
}