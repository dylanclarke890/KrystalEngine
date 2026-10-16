#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderListItem;

  /// @brief Creates, restyles, moves and destroys the markers of list items.
  class RenderTreeBuilder::List
  {
    KRYS_NON_COPY_MOVABLE(List);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit List(RenderTreeBuilder &builder) noexcept;

    void UpdateItemMarker(RenderListItem &listItemRenderer) noexcept;
  };
}
