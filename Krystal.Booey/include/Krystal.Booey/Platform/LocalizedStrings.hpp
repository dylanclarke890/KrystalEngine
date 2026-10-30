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

  /// @brief The text of a `details` element's default summary, shown when it has no `summary` child.
  KRYS_NODISCARD utf8_string DefaultDetailsSummaryText() noexcept;

  /// @brief The label of a file input's button.
  KRYS_NODISCARD utf8_string FileButtonChooseFileLabel() noexcept;
  /// @brief The label of a `multiple` file input's button.
  KRYS_NODISCARD utf8_string FileButtonChooseMultipleFilesLabel() noexcept;
  /// @brief What a file input shows without a file.
  KRYS_NODISCARD utf8_string FileButtonNoFileSelectedLabel() noexcept;
  /// @brief What a `multiple` file input shows without files.
  KRYS_NODISCARD utf8_string FileButtonNoFilesSelectedLabel() noexcept;
  /// @brief What a file input shows for more than one file.
  KRYS_NODISCARD utf8_string MultipleFileUploadText(uint32 numberOfFiles) noexcept;

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
