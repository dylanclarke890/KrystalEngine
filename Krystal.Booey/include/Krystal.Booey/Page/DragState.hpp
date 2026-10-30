#pragma once

#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The drag a mouse gesture may start, or has started, in a page.
  // FIXME: The origin an image's data is restricted to (restrictedOriginForImageData), once there are
  // security origins.
  struct DragState
  {
    /// @brief The element that may be a drag source, for the current mouse gesture.
    RefPtr<dom::Element> Source;
    bool ShouldDispatchEvents {false};
    // Should be Maybe<>.
    EnumSet<DragSourceAction> Type;
    /// @brief Used on only the source side of dragging.
    RefPtr<dom::DataTransfer> DataTransfer;
  };
}
