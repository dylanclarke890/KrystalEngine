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

#include "Krystal.Booey/Accessibility/AccessibilitySVGObject.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo::ax
{
  namespace
  {
    KRYS_NODISCARD bool IsSVGElementNamed(const dom::Node *node, dom::DOMStringView localName) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && Is<svg::SVGElement>(*element) && element->LocalName() == localName;
    }
  }

  AccessibilitySVGObject::AccessibilitySVGObject(AXID id, RenderObject *renderer, dom::Node *node,
                                                 AXObjectCache &cache, bool isSVGRoot) noexcept
      : AccessibilityRenderObject(id, renderer, node, cache), _isSVGRoot(isSVGRoot)
  {
  }

  AccessibilitySVGObject::~AccessibilitySVGObject() noexcept = default;

  Ref<AccessibilitySVGObject> AccessibilitySVGObject::Create(AXID id, RenderObject &renderer,
                                                             AXObjectCache &cache, bool isSVGRoot) noexcept
  {
    return AdoptRef(*new AccessibilitySVGObject(id, &renderer, renderer.GetNode(), cache, isSVGRoot));
  }

  Ref<AccessibilitySVGObject> AccessibilitySVGObject::Create(AXID id, dom::Element &element,
                                                             AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilitySVGObject(id, nullptr, &element, cache, false));
  }

  bool AccessibilitySVGObject::HasTitleOrDescriptionChild() const noexcept
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

  bool AccessibilitySVGObject::ComputeIsIgnored() const noexcept
  {
    // SVG-AAM leaves out an element if (https://w3c.github.io/svg-aam/#exclude_elements):
    // * Core-AAM would leave it out;
    // * it's neither perceivable nor interactive;
    // * its first mappable role is presentational, unless it has a global ARIA attribute (Core-AAM's rule)
    //   or a title or desc child;
    // * an ancestor has presentational children (Core-AAM's rule).
    if (DefaultObjectInclusion() == AccessibilityObjectInclusion::IgnoreObject)
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
    if (_ariaRole == AccessibilityRole::Presentational)
    {
      return true;
    }
    if (_ariaRole != AccessibilityRole::Unknown)
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
      for (const AccessibilityObject *object = this; object; object = object->ParentObject())
      {
        auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(*object);
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
    return AccessibilityRenderObject::ComputeIsIgnored();
  }

  AccessibilityRole AccessibilitySVGObject::DetermineAriaRoleAttribute() const noexcept
  {
    auto role = AccessibilityRenderObject::DetermineAriaRoleAttribute();
    if (role != AccessibilityRole::Presentational)
    {
      return role;
    }
    // A title or desc child trumps a presentational role
    // (https://lists.w3.org/Archives/Public/public-svg-a11y/2016Apr/0016.html).
    return HasTitleOrDescriptionChild() ? AccessibilityRole::Unknown : role;
  }

  AccessibilityRole AccessibilitySVGObject::DetermineAccessibilityRole() noexcept
  {
    if (_ariaRole != AccessibilityRole::Unknown)
    {
      return _ariaRole;
    }
    auto *renderer = Renderer();
    if (!renderer)
    {
      return AccessibilityRole::Unknown;
    }
    if (_isSVGRoot)
    {
      return AccessibilityRole::Generic;
    }

    auto *element = GetElement();
    if (renderer->IsLegacyRenderSVGShape() || renderer->IsLegacyRenderSVGPath()
        || renderer->IsLegacyRenderSVGImage() || IsSVGElementNamed(element, u8"use"))
    {
      return AccessibilityRole::Image;
    }
    if (renderer->IsLegacyRenderSVGForeignObject())
    {
      return AccessibilityRole::Generic;
    }
    if (IsSVGElementNamed(element, u8"g"))
    {
      // A g is generic (like a div) unless it has a name or is focusable
      // (https://w3c.github.io/svg-aam/#include_elements).
      return HasAccNameAttribute() || HasTitleOrDescriptionChild() || CanSetFocusAttribute()
               ? AccessibilityRole::Group
               : AccessibilityRole::Generic;
    }
    if (renderer->IsRenderSVGInlineText())
    {
      return AccessibilityRole::StaticText;
    }
    if (renderer->IsRenderSVGText())
    {
      return AccessibilityRole::SVGText;
    }
    if (renderer->IsRenderSVGTextPath())
    {
      return AccessibilityRole::SVGTextPath;
    }
    if (renderer->IsRenderSVGTSpan())
    {
      return AccessibilityRole::SVGTSpan;
    }
    if (IsSVGElementNamed(element, u8"a"))
    {
      return AccessibilityRole::Link;
    }
    return AccessibilityRenderObject::DetermineAccessibilityRole();
  }
}
