/*
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Page/FrameSnapshotting.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo
{
  namespace
  {
    struct ScopedFramePaintingState
    {
      KRYS_NON_COPY_MOVABLE(ScopedFramePaintingState);

      const LocalFrame &Frame;
      const dom::Node *Node;
      const EnumSet<boo::PaintBehavior> PaintBehavior;
      const boo::Color BackgroundColor;

      ScopedFramePaintingState(LocalFrame &frame, dom::Node *node) noexcept
          : Frame(frame), Node(node), PaintBehavior(frame.View()->GetPaintBehavior()),
            BackgroundColor(frame.View()->BaseBackgroundColor())
      {
        krys_debug_assert(!node || node->Renderer());
      }

      ~ScopedFramePaintingState() noexcept
      {
        Frame.View()->SetPaintBehavior(PaintBehavior);
        Frame.View()->SetBaseBackgroundColor(BackgroundColor);
        Frame.View()->SetNodeToDraw(nullptr);
      }
    };
  }

  RefPtr<ImageBuffer> SnapshotFrameRect(LocalFrame &frame, const IntRect &imageRect,
                                        SnapshotOptions &&options) noexcept
  {
    List<FloatRect> clipRects;
    return SnapshotFrameRectWithClip(frame, imageRect, clipRects, krys::move(options));
  }

  // FIXME: The page's scale when it delegates scaling to the host, once pages can.
  RefPtr<ImageBuffer> SnapshotFrameRectWithClip(LocalFrame &frame, const IntRect &imageRect,
                                                const List<FloatRect> &clipRects,
                                                SnapshotOptions &&options) noexcept
  {
    if (!frame.GetPage())
    {
      return nullptr;
    }

    Ref<dom::Document> document = ShareRef(*frame.Document());
    (void)document->UpdateLayout();

    LocalFrameView::SelectionInSnapshot shouldIncludeSelection = LocalFrameView::IncludeSelection;
    if (options.Flags.Contains(SnapshotFlags::ExcludeSelectionHighlighting))
    {
      shouldIncludeSelection = LocalFrameView::ExcludeSelection;
    }

    LocalFrameView::CoordinateSpaceForSnapshot coordinateSpace = LocalFrameView::DocumentCoordinates;
    if (options.Flags.Contains(SnapshotFlags::InViewCoordinates))
    {
      coordinateSpace = LocalFrameView::ViewCoordinates;
    }

    ScopedFramePaintingState state(frame, nullptr);

    auto paintBehavior = state.PaintBehavior;
    if (options.Flags.Contains(SnapshotFlags::ForceBlackText))
    {
      paintBehavior.Add(PaintBehavior::ForceBlackText);
    }
    if (options.Flags.Contains(SnapshotFlags::PaintSelectionOnly))
    {
      paintBehavior.Add(PaintBehavior::SelectionOnly);
    }
    if (options.Flags.Contains(SnapshotFlags::PaintSelectionAndBackgroundsOnly))
    {
      paintBehavior.Add(PaintBehavior::SelectionAndBackgroundsOnly);
    }
    if (options.Flags.Contains(SnapshotFlags::PaintEverythingExcludingSelection))
    {
      paintBehavior.Add(PaintBehavior::ExcludeSelection);
    }
    if (options.Flags.Contains(SnapshotFlags::ExcludeReplacedContentExceptForIFrames))
    {
      paintBehavior.Add(PaintBehavior::ExcludeReplacedContentExceptForIFrames);
    }
    if (options.Flags.Contains(SnapshotFlags::ExcludeText))
    {
      paintBehavior.Add(PaintBehavior::ExcludeText);
    }
    if (options.Flags.Contains(SnapshotFlags::FixedAndStickyLayersOnly))
    {
      paintBehavior.Add(PaintBehavior::FixedAndStickyLayersOnly);
    }
    if (options.Flags.Contains(SnapshotFlags::DraggableElement))
    {
      paintBehavior.Add(PaintBehavior::DraggableSnapshot);
    }

    // Other paint behaviors are set by paintContentsForSnapshot.
    frame.View()->SetPaintBehavior(paintBehavior);

    float scaleFactor = document->DeviceScaleFactor();
    if (options.Flags.Contains(SnapshotFlags::PaintWith3xBaseScale))
    {
      scaleFactor = 3;
    }

    if (options.Flags.Contains(SnapshotFlags::PaintWithIntegralScaleFactor))
    {
      scaleFactor = std::ceil(scaleFactor);
    }

    auto buffer = ImageBuffer::Create(FloatSize(imageRect.Size()), scaleFactor);
    if (!buffer)
    {
      return nullptr;
    }

    buffer->Context().Translate(-FloatPoint(imageRect.Location()));

    if (!clipRects.empty())
    {
      Path clipPath;
      for (const auto &rect : clipRects)
      {
        clipPath.AddRect(EncloseRectToDevicePixels(rect, scaleFactor));
      }
      buffer->Context().ClipPath(clipPath);
    }

    frame.View()->PaintContentsForSnapshot(buffer->Context(), imageRect, shouldIncludeSelection,
                                           coordinateSpace);
    return buffer;
  }

  RefPtr<ImageBuffer> SnapshotSelection(LocalFrame &frame, SnapshotOptions &&options) noexcept
  {
    auto &selection = frame.Document()->Selection();

    if (!selection.IsRange())
    {
      return nullptr;
    }

    FloatRect selectionBounds = selection.SelectionBounds();

    // It is possible for the selection bounds to be empty; see https://bugs.webkit.org/show_bug.cgi?id=56645.
    if (selectionBounds.IsEmpty())
    {
      return nullptr;
    }

    options.Flags.Add(SnapshotFlags::PaintSelectionOnly);
    return SnapshotFrameRect(frame, EnclosingIntRect(selectionBounds), krys::move(options));
  }

  RefPtr<ImageBuffer> SnapshotNode(LocalFrame &frame, dom::Node &node, SnapshotOptions &&options) noexcept
  {
    if (!node.Renderer())
    {
      return nullptr;
    }

    ScopedFramePaintingState state(frame, &node);

    frame.View()->SetBaseBackgroundColor(Color::transparentBlack);
    frame.View()->SetNodeToDraw(&node);

    LayoutRect topLevelRect;
    return SnapshotFrameRect(frame, SnappedIntRect(node.Renderer()->PaintingRootRect(topLevelRect)),
                             krys::move(options));
  }
}
