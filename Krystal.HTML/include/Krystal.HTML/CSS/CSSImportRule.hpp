#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  class MediaList;
  class CSSStyleSheet;

  /// @see https://www.w3.org/TR/cssom-1/#cssimportrule
  class CSSImportRule : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSImportRule);

  public:
    CSSImportRule(RawPtr<CSSStyleSheet> stylesheet) noexcept;

    ~CSSImportRule() noexcept override = default;

#pragma region CSSImportRule - https://www.w3.org/TR/cssom-1/#cssimportrule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssimportrule-href
    KRYS_NODISCARD USVString Href() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssimportrule-media
    KRYS_NODISCARD RefPtr<MediaList> Media() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssimportrule-stylesheet
    KRYS_NODISCARD RefPtr<CSSStyleSheet> StyleSheet() const noexcept;

#pragma endregion
  };
}