#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <format>

namespace krys::log
{
  struct LoggerSettings;
  class ILogger;

  KRYS_NODISCARD Expected<UniquePtr<ILogger>> CreateLogger(const LoggerSettings &settings) noexcept;

  void SetGlobalLogger(const UniquePtr<ILogger> &logger) noexcept;

  KRYS_NODISCARD ILogger *GetGlobalLogger() noexcept;

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
    KRYS_NON_COPYABLE(ILogger);

  public:
    virtual ~ILogger() noexcept = default;

    ILogger(ILogger &) = default;
    ILogger &operator=(ILogger &) = default;
    ILogger(ILogger &&) = default;
    ILogger &operator=(ILogger &&) = default;

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

    KRYS_NODISCARD virtual Level GetLevel() const noexcept = 0;

    virtual void Flush() noexcept = 0;

  protected:
    ILogger() noexcept = default;

    virtual void LogImpl(Level level, const string &message) noexcept = 0;
  };

}

#ifdef KRYS_LOGGING_DISABLED

  #define KRYS_TRACE(...)
  #define KRYS_DEBUG(...)
  #define KRYS_LOG_INFO(...)
  #define KRYS_WARN(...)
  #define KRYS_ERROR(...)
  #define KRYS_CRITICAL(...)

#else

  #define KRYS_LOG_TRACE(...)                                                                                    \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Trace(__VA_ARGS__);                                                                          \
      }                                                                                                      \
    } while (0)

  #define KRYS_LOG_DEBUG(...)                                                                                    \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Debug(__VA_ARGS__);                                                                          \
      }                                                                                                      \
    } while (0)

  #define KRYS_LOG_INFO(...)                                                                                     \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Info(__VA_ARGS__);                                                                           \
      }                                                                                                      \
    } while (0)

  #define KRYS_LOG_WARN(...)                                                                                     \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Warn(__VA_ARGS__);                                                                           \
      }                                                                                                      \
    } while (0)

  #define KRYS_LOG_ERROR(...)                                                                                    \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Error(__VA_ARGS__);                                                                          \
      }                                                                                                      \
    } while (0)

  #define KRYS_LOG_CRITICAL(...)                                                                                 \
    do                                                                                                       \
    {                                                                                                        \
      if (auto logger = krys::log::GetGlobalLogger(); logger != nullptr)                                     \
      {                                                                                                      \
        logger->Critical(__VA_ARGS__);                                                                       \
      }                                                                                                      \
    } while (0)

#endif