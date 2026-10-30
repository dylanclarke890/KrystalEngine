#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Why a load failed.
  class ResourceError
  {
  public:
    enum class Type : uint8
    {
      Null,
      General,
      AccessControl,
      Cancellation,
      Timeout
    };

  private:
    utf8_string _domain;
    int32 _errorCode {0};
    url::URL _failingURL;
    utf8_string _localizedDescription;
    Type _type {Type::Null};

  public:
    ResourceError() noexcept = default;
    ResourceError(utf8_string domain, int32 errorCode, url::URL failingURL, utf8_string localizedDescription,
                  Type type = Type::General) noexcept
        : _domain(krys::move(domain)), _errorCode(errorCode), _failingURL(krys::move(failingURL)),
          _localizedDescription(krys::move(localizedDescription)), _type(type)
    {
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _type == Type::Null;
    }
    KRYS_NODISCARD bool IsGeneral() const noexcept
    {
      return _type == Type::General;
    }
    KRYS_NODISCARD bool IsAccessControl() const noexcept
    {
      return _type == Type::AccessControl;
    }
    KRYS_NODISCARD bool IsCancellation() const noexcept
    {
      return _type == Type::Cancellation;
    }
    KRYS_NODISCARD bool IsTimeout() const noexcept
    {
      return _type == Type::Timeout;
    }

    KRYS_NODISCARD const utf8_string &Domain() const noexcept
    {
      return _domain;
    }
    KRYS_NODISCARD int32 ErrorCode() const noexcept
    {
      return _errorCode;
    }
    KRYS_NODISCARD const url::URL &FailingURL() const noexcept
    {
      return _failingURL;
    }
    KRYS_NODISCARD const utf8_string &LocalizedDescription() const noexcept
    {
      return _localizedDescription;
    }
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
  };

  /// @brief The domain of errors Booey raises itself, rather than the host's loader.
  inline constexpr utf8_stringview ErrorDomainBooeyInternal = u8"BooeyInternal";
}
