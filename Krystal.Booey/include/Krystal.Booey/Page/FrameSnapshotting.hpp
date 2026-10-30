#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FloatRect;
  class IntRect;
  class ImageBuffer;
  class LocalFrame;

  namespace dom
  {
    class Node;
  }

  /// @brief What a snapshot paints, and how.
  enum class SnapshotFlags : uint16
  {
    ExcludeSelectionHighlighting,
    PaintSelectionOnly,
    InViewCoordinates,
    ForceBlackText,
    PaintSelectionAndBackgroundsOnly,
    PaintEverythingExcludingSelection,
    PaintWithIntegralScaleFactor,
    Shareable,
    Accelerated,
    ExcludeReplacedContentExceptForIFrames,
    PaintWith3xBaseScale,
    ExcludeText,
    FixedAndStickyLayersOnly,
    DraggableElement,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::SnapshotFlags, 14uz)

namespace krys::boo
{
  /// @brief How a snapshot is taken.
  /// FIXME: The pixel format and colour space, with bitmap image buffers.
  struct SnapshotOptions
  {
    EnumSet<SnapshotFlags> Flags;
  };

  /// @brief A snapshot of @p imageRect of @p frame (in its document's coordinates, unless the options ask
  /// for the view's), at the device scale factor; null if the frame has no page or the rect is empty.
  /// @note Shareable and accelerated snapshots are the same as others: every image buffer records a display
  /// list.
  KRYS_NODISCARD RefPtr<ImageBuffer> SnapshotFrameRect(LocalFrame &, const IntRect &,
                                                       SnapshotOptions &&) noexcept;
  /// @brief A snapshot of @p imageRect, clipped to @p clipRects if there are any.
  KRYS_NODISCARD RefPtr<ImageBuffer> SnapshotFrameRectWithClip(LocalFrame &, const IntRect &,
                                                               const List<FloatRect> &clipRects,
                                                               SnapshotOptions &&) noexcept;
  /// @brief A snapshot of @p node and its descendants alone, on a transparent background; null if it isn't
  /// rendered.
  KRYS_NODISCARD RefPtr<ImageBuffer> SnapshotNode(LocalFrame &, dom::Node &, SnapshotOptions &&) noexcept;
  /// @brief A snapshot of the frame's selection alone; null if it isn't a range or its bounds are empty.
  KRYS_NODISCARD RefPtr<ImageBuffer> SnapshotSelection(LocalFrame &, SnapshotOptions &&) noexcept;
}
