#include "Krystal.Booey/CSS/CSSOM/CSSViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSViewTransitionRule::CSSViewTransitionRule(css::ViewTransitionRule &viewTransitionRule,
                                               CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _viewTransitionRule(ShareRef(viewTransitionRule))
  {
  }

  CSSViewTransitionRule::~CSSViewTransitionRule() noexcept = default;

  Ref<CSSViewTransitionRule> CSSViewTransitionRule::Create(css::ViewTransitionRule &rule,
                                                           CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSViewTransitionRule(rule, sheet));
  }

  CSSOMString CSSViewTransitionRule::CssText() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8"@view-transition { ");

    if (auto navigation = _viewTransitionRule->Navigation())
    {
      builder.Append(u8"navigation: ");
      builder.Append(*navigation == ViewTransitionNavigation::Auto ? u8"auto" : u8"none");
      builder.Append(u8"; ");
    }

    auto types = Types();
    if (!types.empty())
    {
      builder.Append(u8"types:");
      for (const auto &type : types)
      {
        builder.Append(u8' ');
        builder.Append(type.View());
      }
      builder.Append(u8"; ");
    }

    builder.Append(u8'}');
    return krys::move(builder).ToString();
  }

  CSSOMStringAtom CSSViewTransitionRule::Navigation() const noexcept
  {
    auto navigation = _viewTransitionRule->Navigation();
    if (!navigation)
    {
      return CSSOMStringAtom::Empty();
    }
    if (*navigation == ViewTransitionNavigation::Auto)
    {
      return u8"auto";
    }
    return u8"none";
  }

  List<CSSOMStringAtom> CSSViewTransitionRule::Types() const noexcept
  {
    return _viewTransitionRule->Types();
  }

  void CSSViewTransitionRule::Reattach(Rule &rule) noexcept
  {
    _viewTransitionRule = ShareRef(Downcast<css::ViewTransitionRule>(rule));
  }
}
