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
    using value_type = T;

  private:
    Expected<value_type, Exception> _value;

#if KRYS_ENV(DEV)
    bool _wasReleased {false};
#endif

  public:
    ExceptionOr(value_type &&value) noexcept : _value {Krys::Move(value)}
    {
    }

    ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(Krys::Move(ex))}
    {
    }

    template <typename OtherType>
    requires(Scalar<OtherType> && ConvertibleTo<OtherType, value_type>)
    ExceptionOr(const OtherType &value) noexcept(NoThrowConvertibleTo<OtherType, value_type>)
        : _value(static_cast<value_type>(value))
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

    KRYS_NODISCARD const value_type &Value() const noexcept
    {
      assert(!_wasReleased);
      return _value.value();
    }

    KRYS_NODISCARD value_type &&ReleaseValue() noexcept
    {
      assert(!std::exchange(_wasReleased, true));
      return Krys::Move(_value.value());
    }
  };

  template <typename T>
  class ExceptionOr<T &>
  {
  public:
    using value_type = T &;
    using ReturnReferenceType = T;

  private:
    ExceptionOr<ReturnReferenceType *> _value;

  public:
    ExceptionOr(ReturnReferenceType &value) noexcept : _value {&value}
    {
    }

    ExceptionOr(Exception &&ex) noexcept : _value {Krys::Move(ex)}
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
      return _value.ReleaseValue();
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
    using value_type = void;

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