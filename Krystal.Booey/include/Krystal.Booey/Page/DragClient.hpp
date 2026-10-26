#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/Page/DragItem.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  class DataTransfer;
}

namespace krys::boo
{
  class DragData;
  class LocalFrame;

  /// @brief What a page's drag controller asks of its host: which drags may start, and running a drag that
  /// starts in the page, calling the drag controller back as the cursor moves and the drag ends.
  class DragClient
  {
    KRYS_NON_COPY_MOVABLE(DragClient);

  public:
    DragClient() noexcept = default;
    virtual ~DragClient() noexcept = default;

    virtual void WillPerformDragDestinationAction(DragDestinationAction action,
                                                  const DragData &dragData) noexcept = 0;
    virtual void WillPerformDragSourceAction(DragSourceAction action, const IntPoint &point,
                                             dom::DataTransfer &dataTransfer) noexcept = 0;
    virtual void DidConcludeEditDrag() noexcept
    {
    }
    /// @brief The drags that may start at @p rootViewPoint.
    KRYS_NODISCARD virtual EnumSet<DragSourceAction>
      DragSourceActionMaskForPoint(const IntPoint &rootViewPoint) noexcept = 0;

    /// @brief Runs a drag of @p dataTransfer's data from @p frame, as @p dragItem describes.
    virtual void StartDrag(DragItem dragItem, dom::DataTransfer &dataTransfer,
                           LocalFrame &frame) noexcept = 0;
    virtual void DragEnded() noexcept
    {
    }
  };

  /// @brief The drag client of a page that has no host: no drag starts in the page.
  class EmptyDragClient final : public DragClient
  {
  public:
    void WillPerformDragDestinationAction(DragDestinationAction, const DragData &) noexcept final
    {
    }
    void WillPerformDragSourceAction(DragSourceAction, const IntPoint &, dom::DataTransfer &) noexcept final
    {
    }
    KRYS_NODISCARD EnumSet<DragSourceAction> DragSourceActionMaskForPoint(const IntPoint &) noexcept final
    {
      return {};
    }
    void StartDrag(DragItem, dom::DataTransfer &, LocalFrame &) noexcept final
    {
    }
  };
}
