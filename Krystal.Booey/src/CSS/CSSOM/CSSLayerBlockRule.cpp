#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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
    StringBuilder builder;
    builder.Append(u8"@layer");
    if (auto name = Name(); !name.empty())
    {
      builder.Append(u8" ", name);
    }
    AppendCSSTextForItems(builder);
    return krys::move(builder).ToString();
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
    StringBuilder result;
    for (const auto &segment : name)
    {
      SerialiseIdentifier(CSSOMString(segment.View()), result);
      if (&segment != &name.back())
      {
        result.Append(u8'.');
      }
    }
    return krys::move(result).ToString();
  }
}
