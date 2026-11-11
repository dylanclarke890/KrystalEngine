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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"

namespace krys::boo::html
{
  class HTMLMeterElement;
  class HTMLProgressElement;
}

namespace krys::boo::ax
{
  /// @brief A `progress` or `meter` element: a progress indicator or meter whose range is the element's.
  class AccessibilityProgressIndicator final : public AccessibilityRenderObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilityProgressIndicator> Create(AXID id, RenderObject &renderer,
                                                                     AXObjectCache &cache) noexcept;
    KRYS_NODISCARD static Ref<AccessibilityProgressIndicator> Create(AXID id, dom::Element &element,
                                                                     AXObjectCache &cache) noexcept;

    KRYS_NODISCARD bool IsIndeterminate() const noexcept override;
    KRYS_NODISCARD float ValueForRange() const noexcept override;
    KRYS_NODISCARD float MaxValueForRange() const noexcept override;
    KRYS_NODISCARD float MinValueForRange() const noexcept override;
    KRYS_NODISCARD dom::DOMString ValueDescription() const noexcept override;

  private:
    AccessibilityProgressIndicator(AXID id, RenderObject *renderer, dom::Node *node,
                                   AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override
    {
      return IsIgnoredByDefault();
    }

    KRYS_NODISCARD html::HTMLProgressElement *ProgressElement() const noexcept;
    KRYS_NODISCARD html::HTMLMeterElement *MeterElement() const noexcept;
  };
}
