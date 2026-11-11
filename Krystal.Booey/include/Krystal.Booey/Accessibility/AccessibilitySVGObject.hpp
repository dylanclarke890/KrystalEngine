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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"

namespace krys::boo::ax
{
  /// @brief An object for an SVG element (or an SVG text renderer), with SVG-AAM's roles and inclusion rules.
  /// @see https://w3c.github.io/svg-aam/
  class AccessibilitySVGObject : public AccessibilityRenderObject
  {
  private:
    bool _isSVGRoot {false};

  public:
    KRYS_NODISCARD static Ref<AccessibilitySVGObject>
      Create(AXID id, RenderObject &renderer, AXObjectCache &cache, bool isSVGRoot = false) noexcept;
    KRYS_NODISCARD static Ref<AccessibilitySVGObject> Create(AXID id, dom::Element &element,
                                                             AXObjectCache &cache) noexcept;
    ~AccessibilitySVGObject() noexcept override;

    KRYS_NODISCARD bool IsAccessibilitySVGObject() const noexcept final
    {
      return true;
    }
    /// @brief Whether this is an `svg` root's object.
    KRYS_NODISCARD bool IsAccessibilitySVGRoot() const noexcept final
    {
      return _isSVGRoot;
    }

  protected:
    AccessibilitySVGObject(AXID id, RenderObject *renderer, dom::Node *node, AXObjectCache &cache,
                           bool isSVGRoot) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAriaRoleAttribute() const noexcept final;
    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept final;

  private:
    /// @brief Whether the element has a `title` or `desc` child, which SVG-AAM says includes it.
    KRYS_NODISCARD bool HasTitleOrDescriptionChild() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilitySVGObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsAccessibilitySVGObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
