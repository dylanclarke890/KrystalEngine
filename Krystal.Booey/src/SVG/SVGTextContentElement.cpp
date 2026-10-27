#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Booey/DOM/Dicts/DOMPointInit.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextQuery.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  SVGTextContentElement::SVGTextContentElement(dom::Document &document,
                                               UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                               SVGElementInterface interface, SVGElementFlags flags) noexcept
      : SVGGraphicsElement(document, krys::move(propertyRegistry), interface,
                           flags | SVGElementFlags::IsSvgTextContentElement),
        _textLength(SVGAnimatedLength::Create(this, SVGLengthMode::Other)),
        _lengthAdjust(SVGAnimatedEnumeration::Create(this, SVGLengthAdjustSpacing))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGTextContentElement::_textLength>(SVGNames::TextLengthAttr());
      PropertyRegistry::RegisterProperty<SVGLengthAdjustType, &SVGTextContentElement::_lengthAdjust>(
        SVGNames::LengthAdjustAttr());
    }
  }

  uint32 SVGTextContentElement::GetNumberOfChars() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    return SVGTextQuery(Renderer()).NumberOfCharacters();
  }

  float SVGTextContentElement::GetComputedTextLength() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    return SVGTextQuery(Renderer()).TextLength();
  }

  dom::ExceptionOr<float> SVGTextContentElement::GetSubStringLength(uint32 charnum, uint32 nchars) noexcept
  {
    uint32 numberOfChars = GetNumberOfChars();
    if (charnum >= numberOfChars)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    nchars = std::min(nchars, numberOfChars - charnum);
    return SVGTextQuery(Renderer()).SubStringLength(charnum, nchars);
  }

  dom::ExceptionOr<Ref<SVGPoint>> SVGTextContentElement::GetStartPositionOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return SVGPoint::Create(SVGTextQuery(Renderer()).StartPositionOfCharacter(charnum));
  }

  dom::ExceptionOr<Ref<SVGPoint>> SVGTextContentElement::GetEndPositionOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return SVGPoint::Create(SVGTextQuery(Renderer()).EndPositionOfCharacter(charnum));
  }

  dom::ExceptionOr<Ref<SVGRect>> SVGTextContentElement::GetExtentOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return SVGRect::Create(SVGTextQuery(Renderer()).ExtentOfCharacter(charnum));
  }

  dom::ExceptionOr<float> SVGTextContentElement::GetRotationOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    return SVGTextQuery(Renderer()).RotationOfCharacter(charnum);
  }

  int32 SVGTextContentElement::GetCharNumAtPosition(dom::DOMPointInit &&pointInit) noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    FloatPoint transformPoint {static_cast<float>(pointInit.X), static_cast<float>(pointInit.Y)};
    return SVGTextQuery(Renderer()).CharacterNumberAtPosition(transformPoint);
  }

  dom::ExceptionOr<void> SVGTextContentElement::SelectSubString(uint32 charnum, uint32 nchars) noexcept
  {
    uint32 numberOfChars = GetNumberOfChars();
    if (charnum >= numberOfChars)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    nchars = std::min(nchars, numberOfChars - charnum);

    auto &selection = NodeDocument().Selection();

    // Find selection start
    VisiblePosition start(dom::FirstPositionInNode(this));
    for (uint32 i = 0; i < charnum; ++i)
    {
      start = start.Next();
    }

    // Find selection end
    VisiblePosition end(start);
    for (uint32 i = 0; i < nchars; ++i)
    {
      end = end.Next();
    }

    selection.SetSelection(VisibleSelection(start, end));

    return {};
  }

  namespace
  {
    KRYS_NODISCARD bool IsXMLSpaceAttribute(dom::DOMStringAtom localName,
                                            dom::DOMStringAtom namespaceURI) noexcept
    {
      return localName == u8"space" && namespaceURI == infra::Namespaces::XML;
    }
  }

  bool
    SVGTextContentElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                              dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (IsXMLSpaceAttribute(localName, namespaceURI))
    {
      return true;
    }
    return SVGGraphicsElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGTextContentElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    if (IsXMLSpaceAttribute(localName, namespaceURI))
    {
      if (value == u8"preserve")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Preserve);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Collapse);
      }
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Nowrap);
      return;
    }

    SVGGraphicsElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

  void SVGTextContentElement::AttributeChanged(const dom::QualifiedName &name,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::LengthAdjustAttr())
    {
      auto propertyValue = SVGPropertyTraits<SVGLengthAdjustType>::FromString(*this, string);
      if (propertyValue > 0)
      {
        _lengthAdjust->SetBaseValInternal<SVGLengthAdjustType>(propertyValue);
      }
    }
    else if (name == SVGNames::TextLengthAttr())
    {
      _textLength->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Other, string, parseError,
                                                                SVGLengthNegativeValuesMode::Forbid));
    }

    ReportAttributeParsingError(parseError, name, string);

    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGTextContentElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      if (attributeName == SVGNames::TextLengthAttr())
      {
        _specifiedTextLength = _textLength->BaseVal()->Value();
      }

      InstanceInvalidationGuard guard(*this);
      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  SVGAnimatedLength &SVGTextContentElement::TextLengthAnimated() noexcept
  {
    static NeverDestroyed<SVGLengthValue> defaultTextLength(SVGLengthMode::Other);
    if (_textLength->BaseVal()->Value() == defaultTextLength.get())
    {
      _textLength->BaseVal()->Value() = SVGLengthValue {GetComputedTextLength(), SVGLengthType::Number};
    }
    return *_textLength;
  }

  bool SVGTextContentElement::SelfHasRelativeLengths() const noexcept
  {
    // Any element of the <text> subtree is advertized as using relative lengths.
    // On any window size change, we have to relayout the text subtree, as the
    // effective 'on-screen' font size may change.
    return true;
  }

  const SVGTextContentElement *
    SVGTextContentElement::ElementFromRenderer(const RenderObject *renderer) noexcept
  {
    if (!renderer)
    {
      return nullptr;
    }

    if (!renderer->IsRenderSVGText() && !renderer->IsRenderSVGInline())
    {
      return nullptr;
    }

    auto *element = Downcast<SVGElement>(renderer->GetNode());
    krys_debug_assert(element);
    return DynamicDowncast<SVGTextContentElement>(element);
  }
}
