/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceContainer;
  class RenderElement;
  class RenderObject;
  class RenderStyle;
  class SVGResources;

  /// @brief The resources each SVG renderer uses, kept by the document.
  class SVGResourcesCache
  {
    KRYS_NON_COPY_MOVABLE(SVGResourcesCache);

  private:
    using CacheMap = HashMap<const RenderElement *, UniquePtr<SVGResources>>;
    CacheMap _cache;

  public:
    SVGResourcesCache() noexcept;
    ~SVGResourcesCache() noexcept;

    KRYS_NODISCARD static SVGResources *CachedResourcesForRenderer(const RenderElement &) noexcept;

    // Called from all SVG renderers addChild() methods.
    static void ClientWasAddedToTree(RenderObject &) noexcept;

    // Called from all SVG renderers removeChild() methods.
    static void ClientWillBeRemovedFromTree(RenderObject &) noexcept;

    // Called from all SVG renderers destroy() methods - except for LegacyRenderSVGResourceContainer.
    static void ClientDestroyed(RenderElement &) noexcept;

    // Called from all SVG renderers layout() methods.
    static void ClientLayoutChanged(RenderElement &) noexcept;

    // Called from all SVG renderers styleDidChange() methods.
    static void ClientStyleChanged(RenderElement &, style::Difference, const RenderStyle *oldStyle,
                                   const RenderStyle &newStyle) noexcept;

    // Called from LegacyRenderSVGResourceContainer::willBeDestroyed().
    static void ResourceDestroyed(LegacyRenderSVGResourceContainer &) noexcept;

    /// @brief Builds a renderer's resources from another style for a scope, and back from its own after.
    class SetStyleForScope
    {
      KRYS_NON_COPY_MOVABLE(SetStyleForScope);

    private:
      RenderElement &_renderer;
      const RenderStyle &_scopedStyle;
      bool _needsNewStyle {false};

    public:
      SetStyleForScope(RenderElement &, const RenderStyle &scopedStyle, const RenderStyle &newStyle) noexcept;
      ~SetStyleForScope() noexcept;

    private:
      void SetStyle(const RenderStyle &) noexcept;
    };

  private:
    void AddResourcesFromRenderer(RenderElement &, const RenderStyle &) noexcept;
    void RemoveResourcesFromRenderer(RenderElement &) noexcept;
  };
}
