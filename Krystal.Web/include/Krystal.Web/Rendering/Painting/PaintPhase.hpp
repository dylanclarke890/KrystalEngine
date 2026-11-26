/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::web
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
    BlockBackground,
    ChildBlockBackground,
    ChildBlockBackgrounds,
    Float,
    Foreground,
    Outline,
    ChildOutlines,
    SelfOutline,
    Selection,
    CollapsedTableBorders,
    TextClip,
    Mask,
    ClippingMask,
    EventRegion,
  };

  consteval PaintPhase MaxEnumValue(PaintPhase) noexcept
  {
    return PaintPhase::EventRegion;
  }

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
    ExcludeReplacedContentExceptForIFrames,
    ExcludeText,
    FixedAndStickyLayersOnly,
    DrawsHDRContent,
    DraggableSnapshot,
  };

  consteval PaintBehavior MaxEnumValue(PaintBehavior) noexcept
  {
    return PaintBehavior::DraggableSnapshot;
  }
}
