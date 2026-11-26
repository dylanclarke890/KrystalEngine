/*
 * Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Web/Svg/Elements/SVGTextContentElement.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Web/Css/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Editing/Position.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Web/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Web/Rendering/SVG/SVGTextQuery.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"
#include "Krystal.Web/Svg/SVGPoint.hpp"
#include "Krystal.Web/Svg/SVGRect.hpp"
#include <algorithm>

namespace krys::web
{
  SVGTextContentElement::SVGTextContentElement(Document &document,
                                               UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                               SVGElementInterface interface, SVGElementFlags flags) noexcept
      : SVGGraphicsElement(document, krys::move(propertyRegistry), interface,
                           flags | SVGElementFlag::IsSvgTextContentElement),
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
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    return SVGTextQuery(Renderer()).NumberOfCharacters();
  }

  float SVGTextContentElement::GetComputedTextLength() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    return SVGTextQuery(Renderer()).TextLength();
  }

  ExceptionOr<float> SVGTextContentElement::GetSubStringLength(uint32 charnum, uint32 nchars) noexcept
  {
    uint32 numberOfChars = GetNumberOfChars();
    if (charnum >= numberOfChars)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    nchars = std::min(nchars, numberOfChars - charnum);
    return SVGTextQuery(Renderer()).SubStringLength(charnum, nchars);
  }

  ExceptionOr<Ref<SVGPoint>> SVGTextContentElement::GetStartPositionOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    return SVGPoint::Create(SVGTextQuery(Renderer()).StartPositionOfCharacter(charnum));
  }

  ExceptionOr<Ref<SVGPoint>> SVGTextContentElement::GetEndPositionOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    return SVGPoint::Create(SVGTextQuery(Renderer()).EndPositionOfCharacter(charnum));
  }

  ExceptionOr<Ref<SVGRect>> SVGTextContentElement::GetExtentOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    return SVGRect::Create(SVGTextQuery(Renderer()).ExtentOfCharacter(charnum));
  }

  ExceptionOr<float> SVGTextContentElement::GetRotationOfChar(uint32 charnum) noexcept
  {
    if (charnum >= GetNumberOfChars())
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    return SVGTextQuery(Renderer()).RotationOfCharacter(charnum);
  }

  int32 SVGTextContentElement::GetCharNumAtPosition(DOMPointInit pointInit) noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    PointF transformPoint {static_cast<float>(pointInit.X), static_cast<float>(pointInit.Y)};
    return SVGTextQuery(Renderer()).CharacterNumberAtPosition(transformPoint);
  }

  ExceptionOr<void> SVGTextContentElement::SelectSubString(uint32 charnum, uint32 nchars) noexcept
  {
    uint32 numberOfChars = GetNumberOfChars();
    if (charnum >= numberOfChars)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    nchars = std::min(nchars, numberOfChars - charnum);

    auto &selection = NodeDocument().Selection();

    // Find selection start
    VisiblePosition start(FirstPositionInNode(this));
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
    KRYS_NODISCARD bool IsXMLSpaceAttribute(const DOMStringAtom &localName,
                                            const DOMStringAtom &namespaceURI) noexcept
    {
      return localName == u8"space" && namespaceURI == infra::namespaces::XML;
    }
  }

  bool SVGTextContentElement::HasPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI) const noexcept
  {
    if (IsXMLSpaceAttribute(localName, namespaceURI))
    {
      return true;
    }
    return SVGGraphicsElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGTextContentElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
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

  void SVGTextContentElement::AttributeChanged(const QualifiedName &name, Maybe<DOMStringView> oldValue,
                                               Maybe<DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(DOMStringView {});

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

  void SVGTextContentElement::SVGAttributeChanged(const QualifiedName &attributeName) noexcept
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
    if (_textLength->BaseVal()->Value() == *defaultTextLength)
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
