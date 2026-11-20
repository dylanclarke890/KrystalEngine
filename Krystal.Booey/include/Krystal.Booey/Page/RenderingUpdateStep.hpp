/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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
 */

#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /// @brief The steps of the rendering update that something asked for.
  // FIXME: The steps for the features Booey doesn't have yet: revealing documents, fullscreen, paint and
  // event timing, video frame callbacks, validation messages and the scrolling tree update.
  enum class RenderingUpdateStep : uint8
  {
    FlushAutofocusCandidates,
    Resize,
    Scroll,
    MediaQueryEvaluation,
    Animations,
    AnimationFrameCallbacks,
    CaretAnimation,
    Images,
    ResizeObservations,
    FocusFixup,
    UpdateContentRelevancy,
    IntersectionObservations,
    SnapshottedScrollOffsets,
    CursorUpdate,
    PrepareCanvasesForDisplayOrFlush,
    LayerFlush,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderingUpdateStep, 16uz);
