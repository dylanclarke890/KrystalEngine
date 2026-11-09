/*
 * Copyright (C) 2004, 2005, 2009 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo
{
  class LegacyRenderSVGContainer;
  class LegacyRenderSVGImage;
  class LegacyRenderSVGResourceContainer;
  class LegacyRenderSVGRoot;
  class LegacyRenderSVGShape;
  class RenderElement;
  class RenderObject;
  class RenderSVGGradientStop;
  class RenderSVGInlineText;
  class RenderSVGText;

  namespace svg
  {
    class SVGGraphicsElement;
  }

  // functions used by the main RenderTreeAsText code
  void Write(TextStream &, const LegacyRenderSVGShape &, EnumSet<RenderAsTextFlag>) noexcept;
  void Write(TextStream &, const LegacyRenderSVGRoot &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGContainer(TextStream &, const LegacyRenderSVGContainer &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGImage(TextStream &, const LegacyRenderSVGImage &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGPaintingFeatures(TextStream &, const RenderElement &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteResources(TextStream &, const RenderObject &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGResourceContainer(TextStream &, const LegacyRenderSVGResourceContainer &,
                                 EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGGradientStop(TextStream &, const RenderSVGGradientStop &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGText(TextStream &, const RenderSVGText &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGInlineText(TextStream &, const RenderSVGInlineText &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGGraphicsElement(TextStream &, const svg::SVGGraphicsElement &) noexcept;
}
