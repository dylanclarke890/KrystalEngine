/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 *
 */

#include "Krystal.Booey/Accessibility/AccessibilityProgressIndicator.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo::ax
{
  Ref<AccessibilityProgressIndicator> AccessibilityProgressIndicator::Create(AXID id, RenderObject &renderer,
                                                                             AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityProgressIndicator(id, &renderer, renderer.GetNode(), cache));
  }

  Ref<AccessibilityProgressIndicator> AccessibilityProgressIndicator::Create(AXID id, dom::Element &element,
                                                                             AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityProgressIndicator(id, nullptr, &element, cache));
  }

  AccessibilityProgressIndicator::AccessibilityProgressIndicator(AXID id, RenderObject *renderer,
                                                                 dom::Node *node,
                                                                 AXObjectCache &cache) noexcept
      : AccessibilityRenderObject(id, renderer, node, cache)
  {
  }

  AccessibilityRole AccessibilityProgressIndicator::DetermineAccessibilityRole() noexcept
  {
    return MeterElement() ? AccessibilityRole::Meter : AccessibilityRole::ProgressIndicator;
  }

  bool AccessibilityProgressIndicator::IsIndeterminate() const noexcept
  {
    // A progress element with no value is indeterminate.
    if (auto *progress = ProgressElement())
    {
      return !progress->HasAttribute(u8"value");
    }
    return false;
  }

  float AccessibilityProgressIndicator::ValueForRange() const noexcept
  {
    if (auto *progress = ProgressElement(); progress && progress->Position() >= 0)
    {
      return static_cast<float>(progress->Value());
    }
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Value());
    }
    return 0;
  }

  float AccessibilityProgressIndicator::MaxValueForRange() const noexcept
  {
    if (auto *progress = ProgressElement())
    {
      return static_cast<float>(progress->Max());
    }
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Max());
    }
    return 0;
  }

  float AccessibilityProgressIndicator::MinValueForRange() const noexcept
  {
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Min());
    }
    return 0;
  }

  dom::DOMString AccessibilityProgressIndicator::ValueDescription() const noexcept
  {
    auto description = AccessibilityRenderObject::ValueDescription();
    if (!description.empty())
    {
      return description;
    }
    // A meter's value text is its content.
    // FIXME: The gauge region (optimum, suboptimal or worse), once there are localised strings for it.
    if (auto *meter = MeterElement())
    {
      return meter->TextContent().value_or(dom::DOMString {});
    }
    return description;
  }

  html::HTMLProgressElement *AccessibilityProgressIndicator::ProgressElement() const noexcept
  {
    return DynamicDowncast<html::HTMLProgressElement>(GetNode());
  }

  html::HTMLMeterElement *AccessibilityProgressIndicator::MeterElement() const noexcept
  {
    return DynamicDowncast<html::HTMLMeterElement>(GetNode());
  }
}
