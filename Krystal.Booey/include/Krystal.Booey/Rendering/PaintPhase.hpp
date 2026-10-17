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
    SelectionOnly,
    SkipSelectionHighlight,
    ForceBlackText,
    ForceWhiteText,
    ForceBlackBorder,
    RenderingSVGClipOrMask,
    SkipRootBackground,
    RootBackgroundOnly,
    SelectionAndBackgroundsOnly,
    ExcludeSelection,
    FlattenCompositingLayers, // Paint doesn't stop at compositing layer boundaries.
    // Paint should always complete image decoding of painted images.
    ForceSynchronousImageDecode,
    // Paint should always start asynchronous image decode of painted images, unless otherwise specified.
    DefaultAsynchronousImageDecode,
    CompositedOverflowScrollContent,
    AnnotateLinks,                // Collect all renderers with links to annotate their URLs (e.g. PDFs)
    EventRegionIncludeForeground, // FIXME: Event region painting should use paint phases.
    EventRegionIncludeBackground,
    // Paint is updating external backing store and visits all content, including composited content and
    // always completes image decoding of painted images. FIXME: Will be removed.
    Snapshotting,
    DontShowVisitedLinks,
    ExcludeReplacedContentExceptForIFrames,
    ExcludeText,
    FixedAndStickyLayersOnly,
    DrawsHDRContent,
    DraggableSnapshot,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::PaintPhase, 14uz)
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::PaintBehavior, 23uz)
