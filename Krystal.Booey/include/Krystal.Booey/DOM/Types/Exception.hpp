#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::boo
{
  /// @brief What went wrong: a `DOMException`'s name, or one of the two simple exceptions.
  /// @see https://webidl.spec.whatwg.org/#idl-DOMException-error-names
  enum class ExceptionCode : uint8
  {
    /// @note Deprecated: `RangeError` replaces it.
    IndexSizeError,
    HierarchyRequestError,
    WrongDocumentError,
    InvalidCharacterError,
    NoModificationAllowedError,
    NotFoundError,
    NotSupportedError,
    InUseAttributeError,
    InvalidStateError,
    SyntaxError,
    InvalidModificationError,
    NamespaceError,
    /// @note Deprecated: `TypeError` replaces it for bad arguments, `NotSupportedError` for unsupported
    /// operations and `NotAllowedError` for refused requests.
    InvalidAccessError,
    /// @note Deprecated: `TypeError` replaces it.
    TypeMismatchError,
    SecurityError,
    NetworkError,
    AbortError,
    UrlMismatchError,
    /// @note Deprecated: the `QuotaExceededError` interface derived from `DOMException` replaces it.
    QuotaExceededError,
    TimeoutError,
    InvalidNodeTypeError,
    DataCloneError,
    EncodingError,
    NotReadableError,
    UnknownError,
    ConstraintError,
    DataError,
    TransactionInactiveError,
    ReadonlyError,
    VersionError,
    OperationError,
    NotAllowedError,
    OptOutError,

    /// @brief The simple exceptions Booey throws, which are ECMAScript errors rather than `DOMException`s.
    /// @see https://webidl.spec.whatwg.org/#dfn-simple-exception
    RangeError,
    TypeError,
  };

  consteval ExceptionCode MaxEnumValue(ExceptionCode) noexcept
  {
    return ExceptionCode::TypeError;
  }

  /// @brief An exception an algorithm throws, as the error of an `ExceptionOr`: its code and an optional
  /// message for whoever reads it.
  /// @see https://webidl.spec.whatwg.org/#idl-exceptions
  class Exception
  {
  private:
    ExceptionCode _code;
    utf8_string _message;

  public:
    explicit Exception(ExceptionCode code, utf8_string message = {}) noexcept
        : _code {code}, _message {krys::move(message)}
    {
    }

    KRYS_NODISCARD ExceptionCode Code() const noexcept
    {
      return _code;
    }

    KRYS_NODISCARD const utf8_string &Message() const noexcept
    {
      return _message;
    }
  };
}
