#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/Platform/DragImage.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What the drag client is handed to run a drag that starts in a page: what's dragged, where, and
  /// the image to show.
  struct DragItem
  {
    DragImage Image;

    // Where the image should be positioned relative to the cursor.
    FloatPoint ImageAnchorPoint;

    Maybe<DragSourceAction> SourceAction;
    IntPoint EventPositionInContentCoordinates;
    IntPoint DragLocationInContentCoordinates;
    IntPoint DragLocationInWindowCoordinates;
    utf8_string Title;
    url::URL Url;
    IntRect DragPreviewFrameInRootViewCoordinates;
    bool ContainsSelection {false};
  };
}
