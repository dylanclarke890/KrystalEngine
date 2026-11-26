/*
 * Copyright (C) 2016 Igalia, S.L.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Accessibility/AXSVGObject.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/RenderText.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGTextContentElement.hpp"

namespace krys::web
{
  namespace
  {
    KRYS_NODISCARD bool IsSVGElementNamed(const Node *node, DOMStringView localName) noexcept
    {
      auto *element = DynamicDowncast<Element>(node);
      return element && Is<SVGElement>(*element) && element->LocalName() == localName;
    }
  }

  AXSVGObject::AXSVGObject(AXId id, RenderObject *renderer, Node *node, AXObjectCache &cache,
                           bool isSVGRoot) noexcept
      : AXRenderObject(id, renderer, node, cache), _isSVGRoot(isSVGRoot)
  {
  }

  AXSVGObject::~AXSVGObject() noexcept = default;

  Ref<AXSVGObject> AXSVGObject::Create(AXId id, RenderObject &renderer, AXObjectCache &cache,
                                       bool isSVGRoot) noexcept
  {
    return AdoptRef(*new AXSVGObject(id, &renderer, renderer.GetNode(), cache, isSVGRoot));
  }

  Ref<AXSVGObject> AXSVGObject::Create(AXId id, Element &element, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXSVGObject(id, nullptr, &element, cache, false));
  }

  AXObject *AXSVGObject::ParentObject() const noexcept
  {
    // An SVG image's root is a child of the image showing it, in another document.
    if (auto *parent = _remoteParent.get())
    {
      return parent;
    }
    return AXRenderObject::ParentObject();
  }

  bool AXSVGObject::IsRootWithAccessibleContent() const noexcept
  {
    auto *root = _isSVGRoot ? GetElement() : nullptr;
    if (!root)
    {
      return false;
    }
    auto hasValue = [](const Element &element, DOMStringAtom name)
    {
      auto value = element.GetAttribute(krys::move(name));
      return value && !value->empty();
    };
    auto isAccessible = [&](const Node &node)
    {
      auto *element = DynamicDowncast<Element>(node);
      if (!element || !Is<SVGElement>(*element))
      {
        return false;
      }
      return IsSVGElementNamed(element, u8"title") || IsSVGElementNamed(element, u8"desc")
             || Is<SVGTextContentElement>(Downcast<SVGElement>(*element)) || hasValue(*element, u8"role")
             || hasValue(*element, u8"aria-label");
    };
    for (const Node *node = root; node; node = dom::TreeQueries::Next(*node, root))
    {
      if (isAccessible(*node))
      {
        return true;
      }
    }
    return false;
  }

  bool AXSVGObject::HasTitleOrDescriptionChild() const noexcept
  {
    auto *element = GetElement();
    if (!element)
    {
      return false;
    }
    for (auto *child = element->FirstElementChild().get(); child; child = child->NextElementSibling().get())
    {
      if (IsSVGElementNamed(child, u8"title") || IsSVGElementNamed(child, u8"desc"))
      {
        return true;
      }
    }
    return false;
  }

  bool AXSVGObject::ComputeIsIgnored() const noexcept
  {
    // SVG-AAM leaves out an element if (https://w3c.github.io/svg-aam/#exclude_elements):
    // * Core-AAM would leave it out;
    // * it's neither perceivable nor interactive;
    // * its first mappable role is presentational, unless it has a global ARIA attribute (Core-AAM's rule)
    //   or a title or desc child;
    // * an ancestor has presentational children (Core-AAM's rule).
    if (DefaultObjectInclusion() == AXObjectInclusion::IgnoreObject)
    {
      return true;
    }
    auto *renderer = Renderer();
    if (!renderer || renderer->IsLegacyRenderSVGHiddenContainer())
    {
      return true;
    }

    // "Elements with at least one title or desc child element MUST be included".
    if (HasTitleOrDescriptionChild())
    {
      return false;
    }
    if (_ariaRole == AXRole::Presentational)
    {
      return true;
    }
    if (_ariaRole != AXRole::Unknown)
    {
      return false;
    }

    // Text elements are included if they have content.
    if (renderer->IsRenderSVGText() || renderer->IsRenderSVGTextPath())
    {
      for (auto *child = Downcast<RenderElement>(*renderer).FirstChild(); child; child = child->NextSibling())
      {
        if (auto *text = DynamicDowncast<RenderText>(*child);
            text && !text->ContainsOnlyCollapsibleWhitespace())
        {
          return false;
        }
      }
    }

    // Shapes are left out unless there's a reason to include them: they're focusable or have event
    // listeners, or they're in a named group.
    if (renderer->IsLegacyRenderSVGShape())
    {
      auto *element = GetElement();
      if (CanSetFocusAttribute() || (element && !element->EventListeners().empty()))
      {
        return false;
      }
      for (const AXObject *object = this; object; object = object->ParentObject())
      {
        auto *nodeObject = DynamicDowncast<AXNodeObject>(*object);
        if (nodeObject && nodeObject->HasAttributesRequiredForInclusion())
        {
          return false;
        }
        if (object->IsAccessibilitySVGRoot())
        {
          return true;
        }
      }
      return true;
    }
    return AXRenderObject::ComputeIsIgnored();
  }

  AXRole AXSVGObject::DetermineAriaRoleAttribute() const noexcept
  {
    auto role = AXRenderObject::DetermineAriaRoleAttribute();
    if (role != AXRole::Presentational)
    {
      return role;
    }
    // A title or desc child trumps a presentational role
    // (https://lists.w3.org/Archives/Public/public-svg-a11y/2016Apr/0016.html).
    return HasTitleOrDescriptionChild() ? AXRole::Unknown : role;
  }

  AXRole AXSVGObject::DetermineAccessibilityRole() noexcept
  {
    if (_ariaRole != AXRole::Unknown)
    {
      return _ariaRole;
    }
    auto *renderer = Renderer();
    if (!renderer)
    {
      return AXRole::Unknown;
    }
    if (_isSVGRoot)
    {
      return AXRole::Generic;
    }

    auto *element = GetElement();
    if (renderer->IsLegacyRenderSVGShape() || renderer->IsLegacyRenderSVGPath()
        || renderer->IsLegacyRenderSVGImage() || IsSVGElementNamed(element, u8"use"))
    {
      return AXRole::Image;
    }
    if (renderer->IsLegacyRenderSVGForeignObject())
    {
      return AXRole::Generic;
    }
    if (IsSVGElementNamed(element, u8"g"))
    {
      // A g is generic (like a div) unless it has a name or is focusable
      // (https://w3c.github.io/svg-aam/#include_elements).
      return HasAccNameAttribute() || HasTitleOrDescriptionChild() || CanSetFocusAttribute()
               ? AXRole::Group
               : AXRole::Generic;
    }
    if (renderer->IsRenderSVGInlineText())
    {
      return AXRole::StaticText;
    }
    if (renderer->IsRenderSVGText())
    {
      return AXRole::SVGText;
    }
    if (renderer->IsRenderSVGTextPath())
    {
      return AXRole::SVGTextPath;
    }
    if (renderer->IsRenderSVGTSpan())
    {
      return AXRole::SVGTSpan;
    }
    if (IsSVGElementNamed(element, u8"a"))
    {
      return AXRole::Link;
    }
    return AXRenderObject::DetermineAccessibilityRole();
  }
}
