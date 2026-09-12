#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  /// @see https://webidl.spec.whatwg.org/#idl-DOMException-error-names.
  enum class ExceptionCode : uint8
  {
    IndexSizeError, // Deprecated. Use RangeError instead.
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
    InvalidAccessError, // Deprecated. use NotAllowedError instead.
    TypeMismatchError,  // Deprecated. Use TypeError instead.
    SecurityError,
    NetworkError,
    AbortError,
    URLMismatchError,
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

    // Simple exceptions (https://webidl.spec.whatwg.org/#idl-exceptions).
    RangeError,
    TypeError,
    JSSyntaxError, // Different from DOM SYNTAX_ERR.

    // Non-standard error.
    StackOverflowError,
    OutOfMemoryError,

    // Used to indicate to the bindings that a JS exception was thrown below and it should be propagated.
    ExistingExceptionError,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::ExceptionCode, 38u)

namespace krys::boo::dom
{
  class Exception
  {
  private:
    ExceptionCode _code;
    utf8_string _message;

  public:
    explicit Exception(ExceptionCode code, utf8_string &&message = {}) noexcept
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