/*
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Igalia S.L
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Platform/Locale/LocalizedStrings.hpp"
#include <string>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD utf8_string Concatenate(utf8_stringview prefix, utf8_stringview value,
                                           utf8_stringview suffix = {}) noexcept
    {
      utf8_string result(prefix);
      result += value;
      result += suffix;
      return result;
    }

    KRYS_NODISCARD utf8_string Number(int value) noexcept
    {
      auto string = std::to_string(value);
      return utf8_string(reinterpret_cast<const char8 *>(string.data()), string.size());
    }
  }

  utf8_string FileButtonChooseFileLabel() noexcept
  {
    return u8"Choose File";
  }

  utf8_string FileButtonChooseMultipleFilesLabel() noexcept
  {
    return u8"Choose Files";
  }

  utf8_string FileButtonNoFileSelectedLabel() noexcept
  {
    return u8"no file selected";
  }

  utf8_string FileButtonNoFilesSelectedLabel() noexcept
  {
    return u8"no files selected";
  }

  utf8_string MultipleFileUploadText(uint32 numberOfFiles) noexcept
  {
    return Concatenate(Number(static_cast<int>(numberOfFiles)), u8" files");
  }

  utf8_string WeekFormatInLDML() noexcept
  {
    return u8"'Week' ww, yyyy";
  }

  utf8_string SubmitButtonDefaultLabel() noexcept
  {
    return u8"Submit";
  }

  utf8_string ResetButtonDefaultLabel() noexcept
  {
    return u8"Reset";
  }

  utf8_string DefaultDetailsSummaryText() noexcept
  {
    return u8"Details";
  }

  utf8_string ValidationMessageValueMissingText() noexcept
  {
    return u8"Fill out this field";
  }

  utf8_string ValidationMessageValueMissingForCheckboxText() noexcept
  {
    return u8"Select this checkbox";
  }

  utf8_string ValidationMessageValueMissingForFileText() noexcept
  {
    return u8"Select a file";
  }

  utf8_string ValidationMessageValueMissingForMultipleFileText() noexcept
  {
    return ValidationMessageValueMissingForFileText();
  }

  utf8_string ValidationMessageValueMissingForRadioText() noexcept
  {
    return u8"Select one of these options";
  }

  utf8_string ValidationMessageValueMissingForSelectText() noexcept
  {
    return u8"Select an item in the list";
  }

  utf8_string ValidationMessageTypeMismatchText() noexcept
  {
    return u8"Invalid value";
  }

  utf8_string ValidationMessageTypeMismatchForEmailText() noexcept
  {
    return u8"Enter an email address";
  }

  utf8_string ValidationMessageTypeMismatchForMultipleEmailText() noexcept
  {
    return ValidationMessageTypeMismatchForEmailText();
  }

  utf8_string ValidationMessageTypeMismatchForURLText() noexcept
  {
    return u8"Enter a URL";
  }

  utf8_string ValidationMessagePatternMismatchText() noexcept
  {
    return u8"Match the requested format";
  }

  utf8_string ValidationMessagePatternMismatchText(utf8_stringview title) noexcept
  {
    return Concatenate(u8"Match the requested format: ", title);
  }

  utf8_string ValidationMessageTooShortText(int, int minLength) noexcept
  {
    return Concatenate(u8"Use at least ", Number(minLength), u8" characters");
  }

  utf8_string ValidationMessageTooLongText(int, int maxLength) noexcept
  {
    return Concatenate(u8"Use no more than ", Number(maxLength), u8" characters");
  }

  utf8_string ValidationMessageRangeUnderflowText(utf8_stringview minimum) noexcept
  {
    return Concatenate(u8"Value must be greater than or equal to ", minimum);
  }

  utf8_string ValidationMessageRangeOverflowText(utf8_stringview maximum) noexcept
  {
    return Concatenate(u8"Value must be less than or equal to ", maximum);
  }

  utf8_string ValidationMessageStepMismatchText(utf8_stringview, utf8_stringview) noexcept
  {
    return u8"Enter a valid value";
  }

  utf8_string ValidationMessageBadInputForNumberText() noexcept
  {
    return u8"Enter a number";
  }

  utf8_string AXSearchFieldCancelButtonText() noexcept
  {
    return u8"cancel";
  }

  utf8_string AXDateFieldDayText() noexcept
  {
    return u8"day";
  }

  utf8_string AXDateFieldMonthText() noexcept
  {
    return u8"month";
  }

  utf8_string AXDateFieldWeekText() noexcept
  {
    return u8"week";
  }

  utf8_string AXDateFieldYearText() noexcept
  {
    return u8"year";
  }

  utf8_string AXTimeFieldHourText() noexcept
  {
    return u8"hour";
  }

  utf8_string AXTimeFieldMinuteText() noexcept
  {
    return u8"minutes";
  }

  utf8_string AXTimeFieldSecondText() noexcept
  {
    return u8"seconds";
  }

  utf8_string AXTimeFieldMillisecondText() noexcept
  {
    return u8"milliseconds";
  }
}
