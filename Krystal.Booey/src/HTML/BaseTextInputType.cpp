#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <regex>
#include <string>

namespace krys::boo::html
{
  namespace
  {
    /// @brief @p text as UTF-16, as script regular expressions match it.
    KRYS_NODISCARD std::wstring ToUTF16(dom::DOMStringView text) noexcept
    {
      std::wstring result;
      for (size_t i = 0; i < text.size();)
      {
        auto byte = static_cast<uint8>(text[i]);
        char32_t codePoint = byte;
        size_t length = 1;
        if (byte >= 0xF0 && i + 3 < text.size() + 0)
        {
          codePoint = ((byte & 0x07) << 18) | ((text[i + 1] & 0x3F) << 12) | ((text[i + 2] & 0x3F) << 6)
                      | (text[i + 3] & 0x3F);
          length = 4;
        }
        else if (byte >= 0xE0 && i + 2 < text.size())
        {
          codePoint = ((byte & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
          length = 3;
        }
        else if (byte >= 0xC0 && i + 1 < text.size())
        {
          codePoint = ((byte & 0x1F) << 6) | (text[i + 1] & 0x3F);
          length = 2;
        }
        if (codePoint >= 0x10000)
        {
          codePoint -= 0x10000;
          result += static_cast<wchar_t>(0xD800 + (codePoint >> 10));
          result += static_cast<wchar_t>(0xDC00 + (codePoint & 0x3FF));
        }
        else
        {
          result += static_cast<wchar_t>(codePoint);
        }
        i += length;
      }
      return result;
    }

    /// @brief The pattern as a whole-value regular expression, or null if it isn't valid.
    KRYS_NODISCARD Maybe<std::wregex> CompilePattern(dom::DOMStringView rawPattern) noexcept
    {
      try
      {
        return std::wregex(L"^(?:" + ToUTF16(rawPattern) + L")$", std::regex_constants::ECMAScript);
      }
      catch (const std::regex_error &)
      {
        return std::nullopt;
      }
    }
  }

  bool BaseTextInputType::PatternMismatch(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto rawPattern = element.GetAttribute(u8"pattern");
    if (!rawPattern || value.empty())
    {
      return false;
    }

    auto regex = CompilePattern(*rawPattern);
    if (!regex)
    {
      return false;
    }

    auto valuePatternMismatch = [&](dom::DOMStringView value)
    {
      return !std::regex_match(ToUTF16(value), *regex);
    };

    if (IsEmailField() && element.Multiple())
    {
      for (auto address : infra::StringAlgorithms::StrictlySplit(value, u8','))
      {
        if (valuePatternMismatch(address))
        {
          return true;
        }
      }
      return false;
    }
    return valuePatternMismatch(value);
  }

  bool BaseTextInputType::SupportsPlaceholder() const noexcept
  {
    return true;
  }

  bool BaseTextInputType::SupportsSelectionAPI() const noexcept
  {
    return true;
  }

  bool BaseTextInputType::DirAutoUsesValue() const noexcept
  {
    return true;
  }
}
