/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/Paths/WindRule.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Booey/Platform/ThemeTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/StyleAppearance.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Booey/Text/TextSpacing.hpp"
#include "Krystal.Booey/Text/UnicodeBidi.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"

// FIXME: Add specializations of ValueRepresentation for all enums listed in this file.

// Helper to define an enum that still needs a value representation conformance.
#define KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(T)                                           \
  template <>                                                                                                \
  inline constexpr bool EnumHasValueRepresentation<boo::T> = false;

namespace krys::boo::css
{
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(BorderStyle)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(BreakBetween)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(CSSBoxType)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(CursorType)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(CompositeOperator)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(DisplayType)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FillAttachment)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FillBox)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FillRepeat)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FlexWrap)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FontSizeAdjust::Metric)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FontVariantCaps)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(FontVariantPosition)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(ImageRendering)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(Kerning)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(NinePieceImageRule)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(Overflow)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(PositionType)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(RubyPosition)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(ScrollSnapAxis)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(ScrollSnapAxisAlignType)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(ShapeRendering)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(StyleWritingMode)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(StyleAppearance)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TextCombine)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TextDirection)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TextEmphasisMark)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TextJustify)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TextWrapMode)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(TransformStyle3D)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(UnicodeBidi)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(UserSelect)
  KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE(WindRule)
}

#undef KRYS_DEFINE_ENUM_NEEDS_VALUE_REPRESENTATION_CONFORMANCE
