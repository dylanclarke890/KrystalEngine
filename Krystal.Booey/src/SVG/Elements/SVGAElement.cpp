/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2016 Google Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/SVG/Animation/SVGSMILElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextContentElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::svg
{
  SVGAElement::SVGAElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::A),
        SVGURIReference(this), _target(SVGAnimatedString::Create(this))
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);

    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGAElement::_target>(SVGNames::TargetAttr());
    }
  }

  SVGAElement::~SVGAElement() noexcept = default;

  void SVGAElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                     Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::TargetAttr())
    {
      _target->SetBaseValInternal(dom::DOMString {value.value_or(dom::DOMStringView {})});
      return;
    }
    else if (name == SVGNames::RelAttr())
    {
      // Update SVGAElement::relList() if more rel attributes values are supported.
      auto hasToken = [&](dom::DOMStringView relation)
      {
        dom::DOMStringView remaining = value.value_or(dom::DOMStringView {});
        while (!remaining.empty())
        {
          auto start =
            std::ranges::find_if_not(remaining, [](char8_t c) { return text::IsASCIIWhitespace(c); });
          remaining = remaining.substr(static_cast<size_t>(start - remaining.begin()));
          auto end = std::ranges::find_if(remaining, [](char8_t c) { return text::IsASCIIWhitespace(c); });
          auto token = remaining.substr(0, static_cast<size_t>(end - remaining.begin()));
          if (!token.empty() && text::IsASCIICaselessEqual(token, relation))
          {
            return true;
          }
          remaining = remaining.substr(token.size());
        }
        return false;
      };
      if (hasToken(u8"noreferrer"))
      {
        _linkRelations.Add(SVGALinkRelation::NoReferrer);
      }
      if (_relList)
      {
        _relList->AssociatedAttributeValueChanged();
      }
    }

    SVGURIReference::ParseAttribute(name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGAElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      // Whether the element is a link follows from the attribute (css::IsLink).
      InstanceInvalidationGuard guard(*this);
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGAElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    auto *svgParent = DynamicDowncast<SVGElement>(ParentNode());
    if (svgParent && Is<SVGTextContentElement>(*svgParent))
    {
      return CreateRenderer<RenderSVGInline>(RenderObject::Type::SVGInline, *this, krys::move(style));
    }

    return CreateRenderer<LegacyRenderSVGTransformableContainer>(*this, krys::move(style));
  }

  void SVGAElement::ActivationBehavior(dom::Event &event) noexcept
  {
    if (!css::IsLink(*this))
    {
      return;
    }
    FollowTheHyperlink(event);
  }

  void SVGAElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (css::IsLink(*this))
    {
      if (Focused() && html::HyperlinkAlgorithms::IsEnterKeyKeydownEvent(event))
      {
        event.SetDefaultHandled();
        (void)DispatchSimulatedClick(&event);
        return;
      }

      // A click is followed as the activation behaviour; other buttons' clicks (the middle button opening the
      // link elsewhere) are followed here, and the host decides from the button.
      if (event.Type() == EventNames::AuxClick && dom::MouseEvent::CanTriggerActivationBehavior(event))
      {
        FollowTheHyperlink(event);
        return;
      }
    }

    SVGGraphicsElement::DefaultEventHandler(event);
  }

  void SVGAElement::FollowTheHyperlink(dom::Event &event) noexcept
  {
    auto url = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(Href());

    if (!url.empty() && url.front() == u8'#')
    {
      if (RefPtr targetElement = ShareRefPtr(DynamicDowncast<SVGSMILElement>(
            dom::mixins::NonElementParentNode::GetElementById(TreeScope().RootNode(), url.substr(1)).get())))
      {
        targetElement->BeginByLinkActivation();
        event.SetDefaultHandled();
        return;
      }
    }

    Ref<dom::Document> document = ShareRef(NodeDocument());
    auto completedURL = document->CompleteURL(url);

    auto target = Target();
    if (target.View().empty()
        && GetAttributeNS(XLinkNames::ShowAttr().NamespaceURI(), XLinkNames::ShowAttr().LocalName())
             == u8"new")
    {
      target = dom::DOMStringAtom(u8"_blank");
    }
    event.SetDefaultHandled();

    auto referrerPolicy =
      HasRel(SVGALinkRelation::NoReferrer) ? ReferrerPolicy::NoReferrer : ReferrerPolicy::EmptyString;

    if (RefPtr<LocalFrame> frame = ShareRefPtr(document->Frame()))
    {
      frame->Loader().ChangeLocation(completedURL, target, &event, referrerPolicy, dom::DOMStringAtom::Null(),
                                     this);
    }
  }

  bool SVGAElement::CanStartSelection() const noexcept
  {
    if (!css::IsLink(*this))
    {
      return SVGElement::CanStartSelection();
    }

    return HasEditableStyle();
  }

  bool SVGAElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // http://www.w3.org/2003/01/REC-SVG11-20030114-errata#linking-text-environment
    // The 'a' element may contain any element that its parent may contain, except itself.
    if (auto *svgChild = DynamicDowncast<SVGElement>(child); svgChild && Is<SVGAElement>(*svgChild))
    {
      return false;
    }

    if (auto *parent = ParentElement())
    {
      if (auto *svgParent = DynamicDowncast<SVGElement>(*parent))
      {
        return svgParent->ChildShouldCreateRenderer(child);
      }
    }

    return SVGElement::ChildShouldCreateRenderer(child);
  }

  dom::DOMTokenList &SVGAElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList =
        dom::ElementAlgorithms::CreateDOMTokenList(*this, SVGNames::RelAttr().LocalName(),
                                                   [](dom::Document &, dom::DOMStringView token)
                                                   {
                                                     return text::IsASCIICaselessEqual(token, u8"noreferrer")
                                                            || text::IsASCIICaselessEqual(token, u8"noopener")
                                                            || text::IsASCIICaselessEqual(token, u8"opener");
                                                   });
    }
    return *_relList;
  }

  bool SVGAElement::HasRel(SVGALinkRelation relation) const noexcept
  {
    return _linkRelations.Contains(relation);
  }
}
