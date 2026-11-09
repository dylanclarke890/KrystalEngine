/*
 * Copyright (c) 2024 Igalia S.L.
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

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo
{
  class RenderElement;

  /// @brief The renderers a recursive walk is inside, so that it can stop at a cycle.
  class SVGVisitedRendererTracking
  {
    KRYS_NON_COPY_MOVABLE(SVGVisitedRendererTracking);

  public:
    using VisitedSet = HashSet<const RenderElement *>;

  private:
    VisitedSet &_visitedRenderers;

  public:
    explicit SVGVisitedRendererTracking(VisitedSet &visitedSet) noexcept : _visitedRenderers(visitedSet)
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _visitedRenderers.empty();
    }
    KRYS_NODISCARD bool IsVisiting(const RenderElement &renderer) const noexcept
    {
      return _visitedRenderers.contains(&renderer);
    }

    class Scope
    {
      KRYS_NON_COPY_MOVABLE(Scope);

    private:
      SVGVisitedRendererTracking &_tracking;
      const RenderElement &_renderer;

    public:
      Scope(SVGVisitedRendererTracking &tracking, const RenderElement &renderer) noexcept
          : _tracking(tracking), _renderer(renderer)
      {
        _tracking.AddUnique(renderer);
      }

      ~Scope() noexcept
      {
        _tracking.RemoveUnique(_renderer);
      }
    };

  private:
    void AddUnique(const RenderElement &renderer) noexcept
    {
      KRYS_MAYBE_UNUSED auto result = _visitedRenderers.insert(&renderer);
      krys_debug_assert(result.second);
    }

    void RemoveUnique(const RenderElement &renderer) noexcept
    {
      KRYS_MAYBE_UNUSED auto result = _visitedRenderers.erase(&renderer);
      krys_debug_assert(result);
    }
  };
}
