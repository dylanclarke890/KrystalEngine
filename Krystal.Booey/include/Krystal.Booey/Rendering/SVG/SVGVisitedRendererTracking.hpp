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
