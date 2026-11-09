/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2013 University of Washington.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
