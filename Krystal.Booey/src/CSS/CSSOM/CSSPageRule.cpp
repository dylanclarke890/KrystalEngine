#include "Krystal.Booey/CSS/CSSOM/CSSPageRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSPageDescriptors.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSPageRule::CSSPageRule(PageRule &pageRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _pageRule(ShareRef(pageRule))
  {
  }

  CSSPageRule::~CSSPageRule() noexcept
  {
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->ClearParentRule();
    }
  }

  Ref<CSSPageRule> CSSPageRule::Create(PageRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSPageRule(rule, sheet));
  }

  CSSPageDescriptors &CSSPageRule::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = CSSPageDescriptors::Create(_pageRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSPageRule::SelectorText() const noexcept
  {
    if (const auto *selector = _pageRule->Selector())
    {
      if (auto text = selector->SelectorText(); !text.empty())
      {
        return text;
      }
    }
    return {};
  }

  void CSSPageRule::SetSelectorText(const CSSOMString &selectorText) noexcept
  {
    auto *sheet = ParentStyleSheet();
    auto *sheetContents = sheet ? &sheet->Contents() : nullptr;
    auto selectorList = SelectorParser::ParseSelectorList(
      selectorText, SelectorParserContext(GetParserContext()), sheetContents);
    if (!selectorList)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    _pageRule->WrapperAdoptSelectorList(krys::move(*selectorList));
  }

  CSSOMString CSSPageRule::CssText() const noexcept
  {
    auto selector = SelectorText();
    CSSOMString result;
    result.append(u8"@page");
    if (!selector.empty())
    {
      result.push_back(u8' ');
    }
    result.append(selector);

    if (auto declarations = _pageRule->Properties().AsText(DefaultSerialisationContext());
        !declarations.empty())
    {
      result.append(u8" { ").append(declarations).append(u8" }");
      return result;
    }

    result.append(u8" { }");
    return result;
  }

  void CSSPageRule::Reattach(Rule &rule) noexcept
  {
    _pageRule = ShareRef(Downcast<PageRule>(rule));
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_pageRule->MutableProperties());
    }
  }
}
