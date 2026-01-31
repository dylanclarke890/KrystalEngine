#pragma once

#include "Krystal.HTML/Utils/Exception.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include <cassert>

namespace Krys::HTML
{
  template <typename T>
  class ExceptionOr
  {
  public:
    using ReturnType = T;

  private:
    Expected<ReturnType, Exception> _value;

#if KRYS_ENV(DEV)
    bool _wasReleased {false};
#endif

  public:
    ExceptionOr(ReturnType &&value) noexcept : _value {Krys::Move(value)}
    {
    }

    ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(Krys::Move(ex))}
    {
    }

    template <typename OtherType>
    requires(Scalar<OtherType> && ConvertibleTo<OtherType, ReturnType>)
    ExceptionOr(const OtherType &value) noexcept(NoThrowConvertibleTo<OtherType, ReturnType>)
        : _value(static_cast<ReturnType>(value))
    {
    }

    KRYS_NODISCARD bool HasValue() const noexcept
    {
      return _value.has_value();
    }

    KRYS_NODISCARD bool HasException() const noexcept
    {
      return !_value.has_value();
    }

    KRYS_NODISCARD const Exception &GetException() const noexcept
    {
      assert(!_wasReleased);
      return _value.error();
    }

    KRYS_NODISCARD Exception &&ReleaseException() noexcept
    {
      assert(!std::exchange(_wasReleased, true));
      return Krys::Move(_value.error());
    }

    KRYS_NODISCARD const ReturnType &Value() const noexcept
    {
      assert(!_wasReleased);
      return _value.value();
    }

    KRYS_NODISCARD ReturnType &&ReleaseValue() noexcept
    {
      assert(!std::exchange(_wasReleased, true));
      return Krys::Move(_value.value());
    }
  };

  template <typename T>
  class ExceptionOr<T &>
  {
  public:
    using ReturnType = T &;
    using ReturnReferenceType = T;

  private:
    ExceptionOr<ReturnReferenceType *> _value;

  public:
    ExceptionOr(ReturnReferenceType &value) noexcept : _value {&value}
    {
    }

    ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(Krys::Move(ex))}
    {
    }

    KRYS_NODISCARD bool HasValue() const noexcept
    {
      return _value.HasValue();
    }

    KRYS_NODISCARD bool HasException() const noexcept
    {
      return _value.HasException();
    }

    KRYS_NODISCARD const Exception &GetException() const noexcept
    {
      return _value.GetException();
    }

    KRYS_NODISCARD Exception &&ReleaseException() noexcept
    {
      return _value.ReleaseException();
    }

    KRYS_NODISCARD const ReturnReferenceType &Value() const noexcept
    {
      return *_value.Value();
    }

    KRYS_NODISCARD ReturnReferenceType &&ReleaseValue() noexcept
    {
      return *_value.ReleaseValue();
    }
  };

  template <>
  class ExceptionOr<void>
  {
  private:
    Expected<void, Exception> _value;

#if KRYS_ENV(DEV)
    bool _wasReleased {false};
#endif

  public:
    using ReturnType = void;

    ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(Krys::Move(ex))}
    {
    }

    ExceptionOr() = default;

    KRYS_NODISCARD bool HasException() const noexcept
    {
      return !_value.has_value();
    }

    KRYS_NODISCARD const Exception &GetException() const noexcept
    {
      assert(!_wasReleased);
      return _value.error();
    }

    KRYS_NODISCARD Exception &&ReleaseException() noexcept
    {
      assert(!std::exchange(_wasReleased, true));
      return Krys::Move(_value.error());
    }
  };

}