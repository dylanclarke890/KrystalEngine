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

#include "Krystal.Booey/Accessibility/AXProgressIndicator.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo
{
  Ref<AXProgressIndicator> AXProgressIndicator::Create(AXId id, RenderObject &renderer,
                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXProgressIndicator(id, &renderer, renderer.GetNode(), cache));
  }

  Ref<AXProgressIndicator> AXProgressIndicator::Create(AXId id, Element &element,
                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXProgressIndicator(id, nullptr, &element, cache));
  }

  AXProgressIndicator::AXProgressIndicator(AXId id, RenderObject *renderer, Node *node,
                                           AXObjectCache &cache) noexcept
      : AXRenderObject(id, renderer, node, cache)
  {
  }

  AXRole AXProgressIndicator::DetermineAccessibilityRole() noexcept
  {
    return MeterElement() ? AXRole::Meter : AXRole::ProgressIndicator;
  }

  bool AXProgressIndicator::IsIndeterminate() const noexcept
  {
    // A progress element with no value is indeterminate.
    if (auto *progress = ProgressElement())
    {
      return !progress->HasAttribute(u8"value");
    }
    return false;
  }

  float AXProgressIndicator::ValueForRange() const noexcept
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

  float AXProgressIndicator::MaxValueForRange() const noexcept
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

  float AXProgressIndicator::MinValueForRange() const noexcept
  {
    if (auto *meter = MeterElement())
    {
      return static_cast<float>(meter->Min());
    }
    return 0;
  }

  DOMString AXProgressIndicator::ValueDescription() const noexcept
  {
    auto description = AXRenderObject::ValueDescription();
    if (!description.empty())
    {
      return description;
    }
    // A meter's value text is its content (WebKit adds the gauge region on Cocoa only; Chrome doesn't).
    if (auto *meter = MeterElement())
    {
      return meter->TextContent().value_or(DOMString {});
    }
    return description;
  }

  HTMLProgressElement *AXProgressIndicator::ProgressElement() const noexcept
  {
    return DynamicDowncast<HTMLProgressElement>(GetNode());
  }

  HTMLMeterElement *AXProgressIndicator::MeterElement() const noexcept
  {
    return DynamicDowncast<HTMLMeterElement>(GetNode());
  }
}
