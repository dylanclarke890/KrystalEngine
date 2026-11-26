#include "Krystal.Web/Html/InputTypes/BaseTextInputType.hpp"
#include "Krystal.Text/Regex.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"

namespace krys::web::html
{
  bool BaseTextInputType::PatternMismatch(DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto rawPattern = element.GetAttribute(u8"pattern");
    if (!rawPattern || value.empty())
    {
      return false;
    }

    // Like the `v` flag, unknown escapes are errors.
    auto pattern = text::Regex::Compile(*rawPattern);
    if (!pattern)
    {
      return false;
    }

    auto valuePatternMismatch = [&](DOMStringView value)
    {
      return !pattern->MatchesWhole(value);
    };

    if (IsEmailField() && element.Multiple())
    {
      for (const auto &address : infra::StringAlgorithms::StrictlySplit(value, u8','))
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
