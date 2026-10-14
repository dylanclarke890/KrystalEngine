#include "Krystal.Booey/CSS/CSSOM/CSSLayerStatementRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSLayerStatementRule::CSSLayerStatementRule(LayerRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _layerRule(ShareRef(rule))
  {
    krys_debug_assert(rule.IsStatement());
  }

  CSSLayerStatementRule::~CSSLayerStatementRule() noexcept = default;

  Ref<CSSLayerStatementRule> CSSLayerStatementRule::Create(LayerRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSLayerStatementRule(rule, parent));
  }

  CSSOMString CSSLayerStatementRule::CssText() const noexcept
  {
    CSSOMString result;
    result.append(u8"@layer ");

    auto nameList = NameList();
    for (const auto &name : nameList)
    {
      result.append(name);
      if (&name != &nameList.back())
      {
        result.append(u8", ");
      }
    }
    result.push_back(u8';');

    return result;
  }

  List<CSSOMString> CSSLayerStatementRule::NameList() const noexcept
  {
    List<CSSOMString> result;

    for (const auto &name : _layerRule->NameList())
    {
      result.push_back(StringFromCascadeLayerName(name));
    }

    return result;
  }

  void CSSLayerStatementRule::Reattach(Rule &rule) noexcept
  {
    _layerRule = ShareRef(Downcast<LayerRule>(rule));
  }
}
