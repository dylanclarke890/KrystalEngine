#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/CSSOM/MediaList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
{
  CSSImportRule::CSSImportRule(css::ImportRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _rule(ShareRef(rule))
  {
  }

  CSSImportRule::~CSSImportRule() noexcept
  {
    // The style sheet and media list can outlive the rule, which is no longer theirs.
    if (_styleSheet)
    {
      _styleSheet->ClearOwnerRule();
    }
    if (_media)
    {
      _media->DetachFromParent();
    }
  }

  Ref<CSSImportRule> CSSImportRule::Create(css::ImportRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSImportRule(rule, parent));
  }

  CSSOMString CSSImportRule::Href() const noexcept
  {
    return _rule->Href();
  }

  MediaList &CSSImportRule::Media() const noexcept
  {
    if (!_media)
    {
      _media = MediaList::Create(const_cast<CSSImportRule *>(this));
    }
    return *_media;
  }

  CSSStyleSheet *CSSImportRule::GetStyleSheet() const noexcept
  {
    auto *contents = _rule->StyleSheet();
    if (!contents)
    {
      return nullptr;
    }
    if (!_styleSheet)
    {
      _styleSheet = CSSStyleSheet::Create(ShareRef(*contents), const_cast<CSSImportRule *>(this));
    }
    return _styleSheet.get();
  }

  RefPtr<CSSStyleSheet> CSSImportRule::ProtectedStyleSheet() const noexcept
  {
    return ShareRefPtr(GetStyleSheet());
  }

  Maybe<CSSOMString> CSSImportRule::LayerName() const noexcept
  {
    const auto &name = _rule->GetCascadeLayerName();
    if (!name)
    {
      return null;
    }
    return css::StringFromCascadeLayerName(*name);
  }

  Maybe<CSSOMString> CSSImportRule::SupportsText() const noexcept
  {
    const auto &text = _rule->SupportsText();
    if (text.empty())
    {
      return null;
    }
    return text;
  }

  CSSOMString CSSImportRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@import ", css::SerialiseUrl(Href()));
    if (auto layerName = LayerName())
    {
      if (layerName->empty())
      {
        builder.Append(u8" layer");
      }
      else
      {
        builder.Append(u8" layer(", *layerName, u8')');
      }
    }
    if (auto supports = SupportsText())
    {
      builder.Append(u8" supports(", *supports, u8')');
    }
    if (!MediaQueries().empty())
    {
      builder.Append(u8' ');
      css::mq::Serialise(builder, MediaQueries());
    }
    builder.Append(u8';');
    return krys::move(builder).ToString();
  }

  void CSSImportRule::Reattach(css::Rule &) noexcept
  {
    // Only a style sheet without `@import` rules has its contents shared and copied when changed, which is
    // when wrappers are reattached.
    krys_debug_assert(false);
  }

  void CSSImportRule::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    auto sheet = ProtectedStyleSheet();
    if (sheet && childStyleSheets.insert(sheet).second)
    {
      sheet->GetChildStyleSheets(childStyleSheets);
    }
  }

  const css::mq::MediaQueryList &CSSImportRule::MediaQueries() const noexcept
  {
    return _rule->MediaQueries();
  }

  void CSSImportRule::SetMediaQueries(css::mq::MediaQueryList &&queries) noexcept
  {
    _rule->SetMediaQueries(krys::move(queries));
  }
}
