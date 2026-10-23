#pragma once

#include "Krystal.Booey/DOM/FragmentDirectiveUtilities.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  /// @brief Parses a fragment directive (what follows `:~:` in a URL's fragment) into its text directives.
  /// @see https://wicg.github.io/scroll-to-text-fragment/#parse-a-text-directive
  class FragmentDirectiveParser
  {
  private:
    List<ParsedTextDirective> _parsedTextDirectives;
    utf8_stringview _remainingURLFragment;
    utf8_stringview _fragmentDirective;
    bool _isValid {false};

  public:
    explicit FragmentDirectiveParser(utf8_stringview fragmentDirective) noexcept;

    /// @brief The text directives (`text=`) that parsed; the rest are dropped.
    KRYS_NODISCARD const List<ParsedTextDirective> &ParsedTextDirectives() const noexcept
    {
      return _parsedTextDirectives;
    }

    KRYS_NODISCARD utf8_stringview FragmentDirective() const noexcept
    {
      return _fragmentDirective;
    }

    KRYS_NODISCARD utf8_stringview RemainingURLFragment() const noexcept
    {
      return _remainingURLFragment;
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _isValid;
    }

  private:
    void ParseFragmentDirective(utf8_stringview fragmentDirective) noexcept;
  };
}
