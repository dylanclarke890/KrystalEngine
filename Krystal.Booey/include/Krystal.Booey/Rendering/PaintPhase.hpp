#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /*
   *  The painting of a layer occurs in three distinct phases.  Each phase involves
   *  a recursive descent into the layer's render objects. The first phase is the background phase.
   *  The backgrounds and borders of all blocks are painted.  Inlines are not painted at all.
   *  Floats must paint above block backgrounds but entirely below inline content that can overlap them.
   *  In the foreground phase, all inlines are fully painted.  Inline replaced elements will get all
   *  three phases invoked on them during this phase.
   */

  enum class PaintPhase : uint16
  {
    BlockBackground = 0,
    ChildBlockBackground = 1 << 0,
    ChildBlockBackgrounds = 1 << 1,
    Float = 1 << 2,
    Foreground = 1 << 3,
    Outline = 1 << 4,
    ChildOutlines = 1 << 5,
    SelfOutline = 1 << 6,
    Selection = 1 << 7,
    CollapsedTableBorders = 1 << 8,
    TextClip = 1 << 9,
    Mask = 1 << 10,
    ClippingMask = 1 << 11,
    EventRegion = 1 << 12,
    Accessibility = 1 << 13,
  };

  enum class PaintBehavior : uint32
  {
    Normal = 0,
    SelectionOnly = 1 << 0,
    SkipSelectionHighlight = 1 << 1,
    ForceBlackText = 1 << 2,
    ForceWhiteText = 1 << 3,
    ForceBlackBorder = 1 << 4,
    RenderingSVGClipOrMask = 1 << 5,
    SkipRootBackground = 1 << 6,
    RootBackgroundOnly = 1 << 7,
    SelectionAndBackgroundsOnly = 1 << 8,
    ExcludeSelection = 1 << 9,
    FlattenCompositingLayers = 1 << 10, // Paint doesn't stop at compositing layer boundaries.
    // Paint should always complete image decoding of painted images.
    ForceSynchronousImageDecode = 1 << 11,
    // Paint should always start asynchronous image decode of painted images, unless otherwise specified.
    DefaultAsynchronousImageDecode = 1 << 12,
    CompositedOverflowScrollContent = 1 << 13,
    AnnotateLinks = 1 << 14, // Collect all renderers with links to annotate their URLs (e.g. PDFs)
    EventRegionIncludeForeground = 1 << 15, // FIXME: Event region painting should use paint phases.
    EventRegionIncludeBackground = 1 << 16,
    // Paint is updating external backing store and visits all content, including composited content and
    // always completes image decoding of painted images. FIXME: Will be removed.
    Snapshotting = 1 << 17,
    DontShowVisitedLinks = 1 << 18,
    ExcludeReplacedContentExceptForIFrames = 1 << 19,
    ExcludeText = 1 << 20,
    FixedAndStickyLayersOnly = 1 << 21,
    DrawsHDRContent = 1 << 22,
    DraggableSnapshot = 1 << 23,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::PaintPhase, 14uz)
KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::PaintBehavior, 24uz)
