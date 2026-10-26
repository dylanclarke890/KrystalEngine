#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What the drag client is handed to run a drag that starts in a page: what's dragged, and where.
  // FIXME: The drag image and where it sits relative to the cursor (image, imageAnchorPoint), with drag
  // images.
  struct DragItem
  {
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
