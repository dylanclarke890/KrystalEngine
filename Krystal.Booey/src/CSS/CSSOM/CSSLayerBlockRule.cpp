#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSLayerBlockRule::CSSLayerBlockRule(LayerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSGroupingRule(rule, parent)
  {
    krys_debug_assert(!rule.IsStatement());
  }

  Ref<CSSLayerBlockRule> CSSLayerBlockRule::Create(LayerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSLayerBlockRule(rule, parent));
  }

  CSSOMString CSSLayerBlockRule::CssText() const noexcept
  {
    CSSOMString builder;
    builder.append(u8"@layer");
    if (auto name = Name(); !name.empty())
    {
      builder.append(u8" ").append(name);
    }
    AppendCSSTextForItems(builder);
    return builder;
  }

  CSSOMString CSSLayerBlockRule::Name() const noexcept
  {
    const auto &layer = Downcast<LayerRule>(GetGroupRule());

    if (layer.Name().empty())
    {
      return {};
    }

    return StringFromCascadeLayerName(layer.Name());
  }

  CSSOMString StringFromCascadeLayerName(const CascadeLayerName &name) noexcept
  {
    CSSOMString result;
    for (const auto &segment : name)
    {
      SerialiseIdentifier(CSSOMString(segment.View()), result);
      if (&segment != &name.back())
      {
        result.push_back(u8'.');
      }
    }
    return result;
  }
}
