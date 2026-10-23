#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

// The user-visible strings forms show, in English.
// FIXME: Localisation, once the host says which language to use.
namespace krys::boo
{
  KRYS_NODISCARD utf8_string SubmitButtonDefaultLabel() noexcept;

  /// @brief The layout of week inputs' fields, in Unicode TR35 LDML: the week of the year (`ww`) and its
  /// year.
  KRYS_NODISCARD utf8_string WeekFormatInLDML() noexcept;
  KRYS_NODISCARD utf8_string ResetButtonDefaultLabel() noexcept;

  KRYS_NODISCARD utf8_string ValidationMessageValueMissingText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageValueMissingForCheckboxText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageValueMissingForFileText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageValueMissingForMultipleFileText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageValueMissingForRadioText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageValueMissingForSelectText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTypeMismatchText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTypeMismatchForEmailText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTypeMismatchForMultipleEmailText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTypeMismatchForURLText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessagePatternMismatchText() noexcept;
  KRYS_NODISCARD utf8_string ValidationMessagePatternMismatchText(utf8_stringview title) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTooShortText(int length, int minLength) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageTooLongText(int length, int maxLength) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageRangeUnderflowText(utf8_stringview minimum) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageRangeOverflowText(utf8_stringview maximum) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageStepMismatchText(utf8_stringview base,
                                                               utf8_stringview step) noexcept;
  KRYS_NODISCARD utf8_string ValidationMessageBadInputForNumberText() noexcept;
}
