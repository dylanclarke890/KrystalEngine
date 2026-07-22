#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  class MediaList;
  class CSSStyleSheet;

  /// @see https://drafts.csswg.org/cssom/#the-cssimportrule-interface
  class CSSImportRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSImportRule);

  public:
    CSSImportRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSImportRule() noexcept override = default;

#pragma region CSSImportRule - https://drafts.csswg.org/cssom/#cssimportrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-href
    KRYS_NODISCARD USVString Href() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-media
    KRYS_NODISCARD RefPtr<MediaList> Media() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-stylesheet
    KRYS_NODISCARD RefPtr<CSSStyleSheet> StyleSheet() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-layername
    KRYS_NODISCARD Maybe<CSSOMString> LayerName() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-supportstext
    KRYS_NODISCARD Maybe<CSSOMString> SupportsText() const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSImportRule)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSImportRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();