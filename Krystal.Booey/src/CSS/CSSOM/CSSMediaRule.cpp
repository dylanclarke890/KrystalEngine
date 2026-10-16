#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSMediaRule::CSSMediaRule(MediaRule &mediaRule, CSSStyleSheet *parent) noexcept
      : CSSConditionRule(mediaRule, parent)
  {
  }

  CSSMediaRule::~CSSMediaRule() noexcept
  {
    if (_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper->DetachFromParent();
    }
  }

  Ref<CSSMediaRule> CSSMediaRule::Create(MediaRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSMediaRule(rule, sheet));
  }

  const mq::MediaQueryList &CSSMediaRule::MediaQueries() const noexcept
  {
    return Downcast<MediaRule>(GetGroupRule()).MediaQueries();
  }

  void CSSMediaRule::SetMediaQueries(mq::MediaQueryList &&queries) noexcept
  {
    Downcast<MediaRule>(GetGroupRule()).SetMediaQueries(krys::move(queries));
  }

  CSSOMString CSSMediaRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::CssText(const SerialisationContext &context) const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@media ", ConditionText());
    AppendCSSTextWithReplacementURLsForItems(builder, context);
    return krys::move(builder).ToString();
  }

  CSSOMString CSSMediaRule::ConditionText() const noexcept
  {
    StringBuilder builder;
    mq::Serialise(builder, MediaQueries());
    return krys::move(builder).ToString();
  }

  MediaList *CSSMediaRule::Media() const noexcept
  {
    if (!_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper = MediaList::Create(const_cast<CSSMediaRule *>(this));
    }
    return _mediaCSSOMWrapper.get();
  }
}
