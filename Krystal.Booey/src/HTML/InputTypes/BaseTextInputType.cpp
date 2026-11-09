#include "Krystal.Booey/HTML/InputTypes/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <unicode/uregex.h>
#include <unicode/utext.h>

namespace krys::boo::html
{
  namespace
  {
    /// @brief A pattern attribute compiled as an ICU regular expression, matched against whole values.
    class CompiledPattern
    {
      KRYS_NON_COPY_MOVABLE(CompiledPattern);

    private:
      URegularExpression *_regex {nullptr};

    public:
      explicit CompiledPattern(dom::DOMStringView rawPattern) noexcept
      {
        UErrorCode status = U_ZERO_ERROR;
        UText pattern = UTEXT_INITIALIZER;
        utext_openUTF8(&pattern, reinterpret_cast<const char *>(rawPattern.data()),
                       static_cast<int64_t>(rawPattern.size()), &status);
        // Like the `v` flag, unknown escapes are errors.
        _regex = uregex_openUText(&pattern, UREGEX_ERROR_ON_UNKNOWN_ESCAPES, nullptr, &status);
        utext_close(&pattern);
        if (U_FAILURE(status) && _regex)
        {
          uregex_close(_regex);
          _regex = nullptr;
        }
      }

      ~CompiledPattern() noexcept
      {
        if (_regex)
        {
          uregex_close(_regex);
        }
      }

      /// @brief Whether the pattern compiled.
      KRYS_NODISCARD bool IsValid() const noexcept
      {
        return _regex;
      }

      /// @brief Whether the pattern matches the whole of @p value (as if anchored with `^(?:` and `)$`).
      KRYS_NODISCARD bool Matches(dom::DOMStringView value) const noexcept
      {
        krys_debug_assert(_regex);
        UErrorCode status = U_ZERO_ERROR;
        UText text = UTEXT_INITIALIZER;
        utext_openUTF8(&text, reinterpret_cast<const char *>(value.data()),
                       static_cast<int64_t>(value.size()), &status);
        uregex_setUText(_regex, &text, &status);
        bool matches = uregex_matches64(_regex, -1, &status);
        utext_close(&text);
        return U_SUCCESS(status) && matches;
      }
    };
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

    CompiledPattern pattern(*rawPattern);
    if (!pattern.IsValid())
    {
      return false;
    }

    auto valuePatternMismatch = [&](dom::DOMStringView value)
    {
      return !pattern.Matches(value);
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
