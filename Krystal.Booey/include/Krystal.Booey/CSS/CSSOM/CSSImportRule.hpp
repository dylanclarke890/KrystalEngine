#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"

namespace krys::boo::css
{
  class MediaList;
  class ImportRule;

  //namespace MQ
  //{
  //  struct MediaQuery;
  //  using MediaQueryList = SmallList<MediaQuery>;
  //}

  class CSSImportRule final : public CSSRule
  {
    friend class MediaList;

  private:
    const Ref<ImportRule> _importRule;
    mutable RefPtr<MediaList> _mediaCSSOMWrapper;
    mutable RefPtr<CSSStyleSheet> _styleSheetCSSOMWrapper;

    CSSImportRule(ImportRule &rule, CSSStyleSheet *sheet) noexcept;

  public:
    KRYS_NODISCARD static Ref<CSSImportRule> Create(ImportRule &rule, CSSStyleSheet *sheet) noexcept
    {
      return AdoptRef(*new CSSImportRule(rule, sheet));
    }

    virtual ~CSSImportRule() noexcept;

    CSSOMString href() const noexcept;

    MediaList &media() const noexcept;

    CSSStyleSheet *styleSheet() const noexcept;

    CSSOMString layerName() const noexcept;

    CSSOMString supportsText() const noexcept;

    RuleType Type() const noexcept final
    {
      return RuleType::Import;
    }

    CSSOMString CssText() const noexcept final;

    CSSOMString CssText(const SerialisationContext &context) const noexcept final;

    void Reattach(RuleBase &rule) noexcept final;

    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &) noexcept final;

    CSSOMString CssTextInternal(const CSSOMString &urlString) const noexcept;

    //const MQ::MediaQueryList &mediaQueries() const noexcept;

    //void setMediaQueries(MQ::MediaQueryList &&) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_BEGIN(krys::boo::css::CSSImportRule)
  KRYS_NO_DISCARD static bool isType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.Type() == krys::boo::css::RuleType::Import;
  }
KRYS_SPECIALIZE_TYPE_TRAITS_END()
