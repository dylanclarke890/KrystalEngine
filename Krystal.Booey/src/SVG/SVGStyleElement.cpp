#include "Krystal.Booey/SVG/SVGStyleElement.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGStyleElement::SVGStyleElement(dom::Document &document) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Style)
  {
  }

  SVGStyleElement::~SVGStyleElement() noexcept
  {
    _styleSheetOwner.ClearDocumentData(*this);
  }

  bool SVGStyleElement::Disabled() const noexcept
  {
    auto *sheet = Sheet();
    return sheet && sheet->Disabled();
  }

  void SVGStyleElement::Disabled(bool value) noexcept
  {
    if (auto *sheet = Sheet())
    {
      sheet->SetDisabled(value);
    }
  }

  dom::DOMString SVGStyleElement::Type() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::TypeAttr().LocalName());
  }

  void SVGStyleElement::Type(dom::DOMString &&value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::TypeAttr().LocalName(), krys::move(value));
  }

  dom::DOMString SVGStyleElement::Media() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::MediaAttr().LocalName());
  }

  void SVGStyleElement::Media(dom::DOMString &&value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::MediaAttr().LocalName(), krys::move(value));
  }

  dom::DOMString SVGStyleElement::Title() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, SVGNames::TitleAttr().LocalName());
  }

  void SVGStyleElement::Title(dom::DOMString &&value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, SVGNames::TitleAttr().LocalName(), krys::move(value));
  }

  void SVGStyleElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                         Maybe<dom::DOMStringView> value) noexcept
  {
    auto newValue = value ? dom::DOMStringAtom(*value) : dom::DOMStringAtom::Null();
    if (name == SVGNames::TitleAttr())
    {
      if (auto *sheet = Sheet(); sheet && !dom::TreeQueries::IsInShadowTree(*this))
      {
        sheet->SetTitle(css::CSSOMString(newValue.View()));
      }
    }
    else if (name == SVGNames::TypeAttr())
    {
      _styleSheetOwner.SetContentType(newValue);
      _styleSheetOwner.ChildrenChanged(*this);
      if (auto *scope = _styleSheetOwner.StyleScope())
      {
        scope->DidChangeStyleSheetContents();
      }
    }
    else if (name == SVGNames::MediaAttr())
    {
      _styleSheetOwner.SetMedia(newValue);
      if (auto *sheet = Sheet())
      {
        sheet->SetMediaQueries(
          css::mq::MediaQueryParser::Parse(newValue.View(), NodeDocument().CssParserContext()));
        if (auto *scope = _styleSheetOwner.StyleScope())
        {
          scope->DidChangeStyleSheetContents();
        }
      }
      else
      {
        _styleSheetOwner.ChildrenChanged(*this);
      }
    }

    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGStyleElement::FinishParsingChildren() noexcept
  {
    _styleSheetOwner.FinishParsingChildren(*this);
    SVGElement::FinishParsingChildren();
  }

  void SVGStyleElement::OnInsert() noexcept
  {
    SVGElement::OnInsert();
    if (IsConnected())
    {
      _styleSheetOwner.InsertedIntoDocument(*this);
    }
  }

  void SVGStyleElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    SVGElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      _styleSheetOwner.RemovedFromDocument(*this);
    }
  }

  void SVGStyleElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();
    _styleSheetOwner.ChildrenChanged(*this);
  }
}
