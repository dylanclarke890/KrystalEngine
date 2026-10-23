#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
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
}
