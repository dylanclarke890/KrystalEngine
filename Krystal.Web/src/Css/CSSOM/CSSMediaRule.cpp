#include "Krystal.Web/Css/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Web/Css/CSSOM/MediaList.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryParser.hpp"
#include "Krystal.Web/Css/Rules/Rule.hpp"

namespace krys::web
{
  CSSMediaRule::CSSMediaRule(css::MediaRule &rule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(rule, parent)
  {
  }

  CSSMediaRule::~CSSMediaRule() noexcept
  {
    // The list can outlive the rule, so it keeps the queries itself.
    if (_media)
    {
      _media->DetachFromParent();
    }
  }

  Ref<CSSMediaRule> CSSMediaRule::Create(css::MediaRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSMediaRule(rule, parent));
  }

  MediaList *CSSMediaRule::Media() const noexcept
  {
    if (!_media)
    {
      _media = MediaList::Create(const_cast<CSSMediaRule *>(this));
    }
    return _media.get();
  }

  CSSOMString CSSMediaRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::CssText(const css::SerialisationContext &context) const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextWithReplacementUrlsForItems(builder, context);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    css::mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  const css::mq::MediaQueryList &CSSMediaRule::MediaQueries() const noexcept
  {
    return Downcast<css::MediaRule>(GetGroupRule()).MediaQueries();
  }

  void CSSMediaRule::SetMediaQueries(css::mq::MediaQueryList &&queries) noexcept
  {
    Downcast<css::MediaRule>(GetGroupRule()).SetMediaQueries(krys::move(queries));
  }
}
