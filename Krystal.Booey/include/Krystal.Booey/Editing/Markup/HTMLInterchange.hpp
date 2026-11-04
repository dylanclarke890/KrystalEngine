#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Text;
}

namespace krys::boo
{
  constexpr utf8_stringview AppleInterchangeNewline = u8"Apple-interchange-newline";
  constexpr utf8_stringview AppleConvertedSpace = u8"Apple-converted-space";
  constexpr utf8_stringview WebKitMSOListQuirksStyle = u8"WebKit-mso-list-quirks-style";
  constexpr utf8_stringview ApplePasteAsQuotation = u8"Apple-paste-as-quotation";
  constexpr utf8_stringview AppleStyleSpanClass = u8"Apple-style-span";
  constexpr utf8_stringview AppleTabSpanClass = u8"Apple-tab-span";

  enum class AnnotateForInterchange : bool
  {
    No,
    Yes
  };

  /// @brief @p in, from @p node, with runs of collapsible spaces kept as spans of no-break spaces, so they
  /// survive being pasted.
  KRYS_NODISCARD utf8_string ConvertHTMLTextToInterchangeFormat(utf8_stringview in,
                                                                const dom::Text *node) noexcept;
}
