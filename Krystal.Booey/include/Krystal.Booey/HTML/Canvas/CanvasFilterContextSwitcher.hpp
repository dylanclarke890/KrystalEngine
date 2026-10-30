#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatRect;
}

namespace krys::boo::html
{
  class CanvasRenderingContext2DBase;

  /// @brief Applies a 2D context's filter to what's drawn while it's in place: the context's state is saved,
  /// and its drawing goes into a filtered layer until the state is restored.
  class CanvasFilterContextSwitcher
  {
    KRYS_NON_COPY_MOVABLE(CanvasFilterContextSwitcher);

  private:
    CanvasRenderingContext2DBase &_context;

  public:
    /// @brief A switcher for drawing within @p bounds; null if the context has no filter, or it can't be
    /// applied.
    KRYS_NODISCARD static UniquePtr<CanvasFilterContextSwitcher> Create(CanvasRenderingContext2DBase &,
                                                                        const FloatRect &bounds) noexcept;

    explicit CanvasFilterContextSwitcher(CanvasRenderingContext2DBase &) noexcept;
    ~CanvasFilterContextSwitcher() noexcept;

    /// @brief The bounds drawn within, with the filter's outsets.
    KRYS_NODISCARD FloatRect ExpandedBounds() const noexcept;
  };
}
