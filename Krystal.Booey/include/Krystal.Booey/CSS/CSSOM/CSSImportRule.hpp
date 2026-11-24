#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class ImportRule;
}

namespace krys::boo
{
  class MediaList;

  /// @brief An `@import` rule, as scripts see it.
  /// @see https://drafts.csswg.org/cssom/#the-cssimportrule-interface
  class CSSImportRule final : public CSSRule
  {
    friend class MediaList;

  private:
    const Ref<css::ImportRule> _rule;
    // The media list, which the imported style sheet shares, and the imported style sheet, made when first
    // asked for.
    mutable RefPtr<MediaList> _media;
    mutable RefPtr<CSSStyleSheet> _styleSheet;

    CSSImportRule(css::ImportRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSImportRule() noexcept override;

    KRYS_NODISCARD static Ref<CSSImportRule> Create(css::ImportRule &rule, CSSStyleSheet *parent) noexcept;

#pragma region CSSImportRule - https://drafts.csswg.org/cssom/#the-cssimportrule-interface

    /// @brief The URL the rule gives, as written.
    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-href
    KRYS_NODISCARD CSSOMString Href() const noexcept;

    /// @brief The media queries the imported style sheet applies to, which are its style sheet's.
    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-media
    KRYS_NODISCARD MediaList &Media() const noexcept;

    /// @brief The imported style sheet, or null until it has loaded.
    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-stylesheet
    KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept;

    /// @brief The cascade layer the style sheet is imported into: empty for an anonymous layer, null for
    /// none.
    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-layername
    KRYS_NODISCARD Maybe<CSSOMString> LayerName() const noexcept;

    /// @brief The condition of the rule's `supports()`, or null if it has none.
    /// @see https://drafts.csswg.org/cssom/#dom-cssimportrule-supportstext
    KRYS_NODISCARD Maybe<CSSOMString> SupportsText() const noexcept;

#pragma endregion

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::Import;
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;

    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept final;

    KRYS_NODISCARD const css::mq::MediaQueryList &MediaQueries() const noexcept;

    void SetMediaQueries(css::mq::MediaQueryList &&queries) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSImportRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Import;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
