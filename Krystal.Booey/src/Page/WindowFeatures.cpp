#include "Krystal.Booey/Page/WindowFeatures.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <cstdint>

namespace krys::boo
{
  namespace
  {
    /// @see https://html.spec.whatwg.org/#feature-separator
    KRYS_NODISCARD bool IsSeparator(char8 character) noexcept
    {
      return text::IsASCIIWhitespace(character) || character == u8'=' || character == u8',';
    }

    KRYS_NODISCARD bool IsKey(dom::DOMStringView key, utf8_stringview name) noexcept
    {
      return text::IsASCIICaselessEqual(key, name);
    }

    void SetWindowFeature(WindowFeatures &features, dom::DOMStringView key, dom::DOMStringView value) noexcept
    {
      // Listing a key with no value is shorthand for key=yes.
      int32 numericValue;
      if (value.empty() || text::IsASCIICaselessEqual(value, utf8_stringview(u8"yes"))
          || text::IsASCIICaselessEqual(value, utf8_stringview(u8"true")))
      {
        numericValue = 1;
      }
      else
      {
        auto parsed = html::Numbers::ParseInteger(value);
        numericValue =
          parsed.Success() ? static_cast<int32>(std::clamp<int64>(parsed.Value, INT32_MIN, INT32_MAX)) : 0;
      }

      // We treat key of "resizable" here as an additional feature rather than setting resizeable to true.
      // This is consistent with Firefox, but could also be handled at another level.
      if (IsKey(key, u8"left") || IsKey(key, u8"screenx"))
      {
        features.X = static_cast<float>(numericValue);
      }
      else if (IsKey(key, u8"top") || IsKey(key, u8"screeny"))
      {
        features.Y = static_cast<float>(numericValue);
      }
      else if (IsKey(key, u8"width") || IsKey(key, u8"innerwidth"))
      {
        features.Width = static_cast<float>(numericValue);
      }
      else if (IsKey(key, u8"height") || IsKey(key, u8"innerheight"))
      {
        features.Height = static_cast<float>(numericValue);
      }
      else if (IsKey(key, u8"popup"))
      {
        features.Popup = numericValue != 0;
      }
      else if (IsKey(key, u8"menubar"))
      {
        features.MenuBarVisible = numericValue != 0;
      }
      else if (IsKey(key, u8"toolbar"))
      {
        features.ToolBarVisible = numericValue != 0;
      }
      else if (IsKey(key, u8"location"))
      {
        features.LocationBarVisible = numericValue != 0;
      }
      else if (IsKey(key, u8"status"))
      {
        features.StatusBarVisible = numericValue != 0;
      }
      else if (IsKey(key, u8"fullscreen"))
      {
        features.Fullscreen = numericValue != 0;
      }
      else if (IsKey(key, u8"scrollbars"))
      {
        features.ScrollbarsVisible = numericValue != 0;
      }
      else if (IsKey(key, u8"resizable"))
      {
        features.Resizable = numericValue != 0;
      }
      else if (IsKey(key, u8"noopener"))
      {
        features.NoOpener = numericValue != 0;
      }
      else if (IsKey(key, u8"noreferrer"))
      {
        features.NoReferrer = numericValue != 0;
      }
      else if (!key.empty() || !value.empty())
      {
        features.HasAdditionalFeatures = true;
        if (numericValue == 1)
        {
          features.AdditionalFeatures.emplace_back(key);
        }
      }
    }
  }

  bool WindowFeatures::WantsPopup() const noexcept
  {
    // If the WindowFeatures string contains nothing more than noopener and noreferrer we consider the string
    // to be empty and thus return false based on the algorithm above.
    if (!HasAdditionalFeatures && !X && !Y && !Width && !Height && !Popup && !MenuBarVisible
        && !StatusBarVisible && !ToolBarVisible && !LocationBarVisible && !ScrollbarsVisible && !Resizable)
    {
      return false;
    }

    // If popup is defined, return its value as a boolean.
    if (Popup)
    {
      return *Popup;
    }

    // If location (default to false) and toolbar (default to false) are false return true.
    if ((!LocationBarVisible || !*LocationBarVisible) && (!ToolBarVisible || !*ToolBarVisible))
    {
      return true;
    }

    // If menubar (default to false) is false return true.
    if (!MenuBarVisible || !*MenuBarVisible)
    {
      return true;
    }

    // If resizable (default to true) is false return true.
    if (Resizable && !*Resizable)
    {
      return true;
    }

    // If scrollbars (default to false) is false return false.
    if (!ScrollbarsVisible || !*ScrollbarsVisible)
    {
      return true;
    }

    // If status (default to false) is false return true.
    if (!StatusBarVisible || !*StatusBarVisible)
    {
      return true;
    }

    return false;
  }

  WindowFeatures ParseWindowFeatures(dom::DOMStringView features) noexcept
  {
    WindowFeatures windowFeatures;
    size_t length = features.size();
    for (size_t i = 0; i < length;)
    {
      // Skip to first non-separator.
      while (i < length && IsSeparator(features[i]))
      {
        ++i;
      }
      size_t keyBegin = i;

      // Skip to first separator.
      while (i < length && !IsSeparator(features[i]))
      {
        ++i;
      }
      size_t keyEnd = i;

      // Skip to first '=', but don't skip past a ',' or a non-separator.
      while (i < length && features[i] != u8'=' && features[i] != u8',' && IsSeparator(features[i]))
      {
        ++i;
      }

      // Skip to first non-separator, but don't skip past a ','.
      if (i < length && IsSeparator(features[i]))
      {
        while (i < length && IsSeparator(features[i]) && features[i] != u8',')
        {
          ++i;
        }
        size_t valueBegin = i;

        // Skip to first separator.
        while (i < length && !IsSeparator(features[i]))
        {
          ++i;
        }
        size_t valueEnd = i;
        SetWindowFeature(windowFeatures, features.substr(keyBegin, keyEnd - keyBegin),
                         features.substr(valueBegin, valueEnd - valueBegin));
      }
      else
      {
        SetWindowFeature(windowFeatures, features.substr(keyBegin, keyEnd - keyBegin), {});
      }
    }
    return windowFeatures;
  }
}
