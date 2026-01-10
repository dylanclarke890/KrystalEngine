#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/ToUnderlying.hpp"
#include <system_error>
#include <xhash>

namespace Krys::Text
{
  /// @brief Describes a failure to encode, decode, transcode, or count, for four core various reasons.
  /// @remarks This does not cover specific failures, like if a sequence was overlong (e.g., UTF-8) or if an
  /// encode operation produced an unpaired surrogate value (e.g. UTF-16).
  enum class EncodingError : uint8
  {
    /// @brief The okay status; everything is fine.
    /// @remarks This does not necessarily mean an error handler was not called. An error handler can set the
    /// error code to EncodingError::OK after performing corrective action: see
    /// Handlers::ReplacementHandler for an example.
    OK = 0x00,

    /// Input contains ill-formed sequences. This means there were available units of input to read, but what
    /// was read resulted in an error.
    InvalidSequence,

    /// Input contains incomplete sequences. This means that the input was exhausted, without finding an
    /// invalid sequence, and therefore more input may be required.
    /// @remarks Depending on context, this may or may not be an error in your use case (e.g., reading part of
    /// an incomplete network buffer and waiting for more). See Handlers::BasicIncompleteHandler as a way
    /// to aid with this use case.
    IncompleteSequence,

    /// Output cannot receive the successfully encoded or decoded sequence. This means that, while there were
    /// no invalid or incomplete sequences in the input, the output ran out of space to receive it.
    /// @remarks Provide a bigger storage area or guarantee that it meets the minimum required size for
    /// potential output. This can be queried for an encoding by using
    /// MaxCodePoints<the_encoding> for code points, and
    /// MaxCodeUnits<the_encoding> for code units.
    InsufficientOutputSpace,
  };

  /// @brief Converts an EncodingError to a string value.
  /// @returns A null-terminated string_view to the data.
  /// @remarks If a value outside of the allowed EncodingError is passed, then undefined behavior happens.
  KRYS_NODISCARD constexpr inline stringview ToString(EncodingError errorCode)
  {
    constexpr Array<stringview, 4> translation {
      {"OK", "InvalidSequence", "IncompleteSequence", "InsufficientOutputSpace"}};

    return translation[static_cast<size_t>(ToUnderlying(errorCode))];
  }

  namespace Impl
  {
    /// @brief The encoding category for system_error-style exceptions and errors.
    /// @remarks This type is generally paired with a std::error_condition.
    class EncodingCategory : public std::error_category
    {
      virtual const char *name() const noexcept override
      {
        return "EncodingError";
      }

      virtual string message(int untypedErrorCode) const override
      {
        EncodingError errorCode = static_cast<EncodingError>(untypedErrorCode);
        switch (errorCode)
        {
          case EncodingError::OK:
          case EncodingError::IncompleteSequence:
          case EncodingError::InsufficientOutputSpace:
          case EncodingError::InvalidSequence:
          {
            stringview name = ::Krys::Text::ToString(errorCode);
            return string(name.data(), name.size());
          }
        }
        return "unrecognized untyped error code";
      }

      virtual std::error_condition default_error_condition(int untypedErrorCode) const noexcept override
      {
        return std::error_condition(untypedErrorCode, *this);
      }
    };
  }

  /// @brief The system_error category type to be used with std::error_category-requiring errors.
  inline const std::error_category &EncodingCategory()
  {
    static const Impl::EncodingCategory category = {};
    return category;
  }
}

namespace std
{
  template <>
  struct is_error_condition_enum<::Krys::Text::EncodingError> : public std::true_type
  {
  };

  template <>
  class hash<::Krys::Text::EncodingError>
  {
  private:
    using UnderlyingErrorType = ::Krys::underlying_t<::Krys::Text::EncodingError>;

  public:
    std::size_t operator()(::Krys::Text::EncodingError errorCode) const
      noexcept(noexcept(std::hash<UnderlyingErrorType> {}(static_cast<UnderlyingErrorType>(errorCode))))
    {
      std::hash<UnderlyingErrorType> hasher {};
      return hasher(static_cast<UnderlyingErrorType>(errorCode));
    }
  };
}
