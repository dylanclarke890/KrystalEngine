#pragma once

#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/Expected.hpp"

namespace krys::boo::dom
{
  template <typename T>
  class ExceptionOr
  {
  public:
    using value_type = T;

  private:
    Expected<value_type, Exception> _value;

#if KRYS_DEBUG
    bool _wasReleased {false};
#endif

  public:
    constexpr ExceptionOr(value_type &&value) noexcept : _value {krys::move(value)}
    {
    }

    constexpr ExceptionOr(const value_type &value) noexcept : _value {value}
    {
    }

    constexpr ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(krys::move(ex))}
    {
    }

    constexpr ExceptionOr(ExceptionCode code) noexcept : _value {Unexpected(Exception(code))}
    {
    }

    template <typename OtherType>
    requires(Scalar<OtherType> && ConvertibleTo<OtherType, value_type>)
    constexpr ExceptionOr(const OtherType &value) noexcept(NoThrowConvertibleTo<OtherType, value_type>)
        : _value(static_cast<value_type>(value))
    {
    }

    KRYS_NODISCARD constexpr bool HasValue() const noexcept
    {
      return _value.has_value();
    }

    KRYS_NODISCARD constexpr bool HasException() const noexcept
    {
      return !_value.has_value();
    }

    KRYS_NODISCARD constexpr const Exception &GetException() const noexcept
    {
      krys_debug_assert(!_wasReleased);
      return _value.error();
    }

    KRYS_NODISCARD constexpr Exception &&ReleaseException() noexcept
    {
      krys_debug_assert(!std::exchange(_wasReleased, true));
      return krys::move(_value.error());
    }

    KRYS_NODISCARD constexpr const value_type &Value() const noexcept
    {
      krys_debug_assert(!_wasReleased);
      return _value.value();
    }

    KRYS_NODISCARD constexpr value_type &Value() noexcept
    {
      krys_debug_assert(!_wasReleased);
      return _value.value();
    }

    KRYS_NODISCARD constexpr value_type &&ReleaseValue() noexcept
    {
      krys_debug_assert(!std::exchange(_wasReleased, true));
      return krys::move(_value.value());
    }

    KRYS_NODISCARD constexpr value_type &operator*() noexcept
    {
      return Value();
    }

    KRYS_NODISCARD constexpr value_type &operator->() noexcept
    {
      return Value();
    }

    KRYS_NODISCARD constexpr bool operator==(const ExceptionOr &other) const noexcept
    {
      if (HasValue() != other.HasValue())
      {
        return false;
      }

      if (HasValue())
      {
        return Value() == other.Value();
      }

      return GetException().Code() == other.GetException().Code()
             && GetException().Message() == other.GetException().Message();
    }

    KRYS_NODISCARD constexpr bool operator!=(const ExceptionOr &other) const noexcept
    {
      return !(*this == other);
    }

    KRYS_NODISCARD constexpr bool operator==(const value_type &value) const noexcept
    {
      return HasValue() && Value() == value;
    }

    KRYS_NODISCARD constexpr bool operator!=(const value_type &value) const noexcept
    {
      return !(*this == value);
    }

    KRYS_NODISCARD constexpr bool operator==(const Exception &exception) const noexcept
    {
      return HasException() && GetException().Code() == exception.Code()
             && GetException().Message() == exception.Message();
    }

    KRYS_NODISCARD constexpr bool operator!=(const Exception &exception) const noexcept
    {
      return !(*this == exception);
    }

    KRYS_NODISCARD constexpr bool operator==(ExceptionCode code) const noexcept
    {
      return HasException() && GetException().Code() == code;
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
    constexpr ExceptionOr(ReturnReferenceType &value) noexcept : _value {&value}
    {
    }

    constexpr ExceptionOr(Exception &&ex) noexcept : _value {krys::move(ex)}
    {
    }

    constexpr ExceptionOr(ExceptionCode code) noexcept : _value {Exception(code)}
    {
    }

    KRYS_NODISCARD constexpr bool HasValue() const noexcept
    {
      return _value.HasValue();
    }

    KRYS_NODISCARD constexpr bool HasException() const noexcept
    {
      return _value.HasException();
    }

    KRYS_NODISCARD constexpr const Exception &GetException() const noexcept
    {
      return _value.GetException();
    }

    KRYS_NODISCARD constexpr Exception &&ReleaseException() noexcept
    {
      return _value.ReleaseException();
    }

    KRYS_NODISCARD constexpr const ReturnReferenceType &Value() const noexcept
    {
      return *_value.Value();
    }

    KRYS_NODISCARD constexpr ReturnReferenceType &Value() noexcept
    {
      return *_value.Value();
    }

    KRYS_NODISCARD constexpr ReturnReferenceType &&ReleaseValue() noexcept
    {
      return _value.ReleaseValue();
    }

    KRYS_NODISCARD constexpr value_type &operator*() noexcept
    {
      return Value();
    }

    KRYS_NODISCARD constexpr value_type &operator->() noexcept
    {
      return Value();
    }

    KRYS_NODISCARD constexpr bool operator==(const ExceptionOr &other) const noexcept
    {
      if (HasValue() != other.HasValue())
      {
        return false;
      }

      if (HasValue())
      {
        return Value() == other.Value();
      }

      return GetException().Code() == other.GetException().Code()
             && GetException().Message() == other.GetException().Message();
    }

    KRYS_NODISCARD constexpr bool operator!=(const ExceptionOr &other) const noexcept
    {
      return !(*this == other);
    }

    KRYS_NODISCARD constexpr bool operator==(const value_type &value) const noexcept
    {
      return HasValue() && Value() == value;
    }

    KRYS_NODISCARD constexpr bool operator!=(const value_type &value) const noexcept
    {
      return !(*this == value);
    }

    KRYS_NODISCARD constexpr bool operator==(const Exception &exception) const noexcept
    {
      return HasException() && GetException().Code() == exception.Code()
             && GetException().Message() == exception.Message();
    }

    KRYS_NODISCARD constexpr bool operator!=(const Exception &exception) const noexcept
    {
      return !(*this == exception);
    }

    KRYS_NODISCARD constexpr bool operator==(ExceptionCode code) const noexcept
    {
      return HasException() && GetException().Code() == code;
    }
  };

  template <>
  class ExceptionOr<void>
  {
  private:
    Expected<void, Exception> _value;

#if KRYS_DEBUG
    bool _wasReleased {false};
#endif

  public:
    using value_type = void;

    constexpr ExceptionOr() = default;

    constexpr ExceptionOr(Exception &&ex) noexcept : _value {Unexpected(krys::move(ex))}
    {
    }

    constexpr ExceptionOr(ExceptionCode code) noexcept : _value {Unexpected(Exception(code))}
    {
    }

    KRYS_NODISCARD constexpr bool HasException() const noexcept
    {
      return !_value.has_value();
    }

    KRYS_NODISCARD constexpr const Exception &GetException() const noexcept
    {
      krys_debug_assert(!_wasReleased);
      return _value.error();
    }

    KRYS_NODISCARD constexpr Exception &&ReleaseException() noexcept
    {
      krys_debug_assert(!std::exchange(_wasReleased, true));
      return krys::move(_value.error());
    }

    KRYS_NODISCARD constexpr bool operator==(const Exception &exception) const noexcept
    {
      return HasException() && GetException().Code() == exception.Code()
             && GetException().Message() == exception.Message();
    }

    KRYS_NODISCARD constexpr bool operator!=(const Exception &exception) const noexcept
    {
      return !(*this == exception);
    }

    KRYS_NODISCARD constexpr bool operator==(ExceptionCode code) const noexcept
    {
      return HasException() && GetException().Code() == code;
    }
  };
}