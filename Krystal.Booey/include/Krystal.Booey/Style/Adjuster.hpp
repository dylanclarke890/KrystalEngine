/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
  class EventTarget;
}

namespace krys::boo::svg
{
  class SVGElement;
}

namespace krys::boo::style
{
  class Update;

  /// @brief Adjusts a computed style for its element and parent: blockification and inlinification, used
  /// z-index, overflow, text decorations in effect, and the element-specific rules.
  class Adjuster
  {
    CheckedRef<const dom::Document> _document;
    const RenderStyle &_parentStyle;
    const RenderStyle &_parentBoxStyle;
    RefPtr<dom::Element> _element;

  public:
    Adjuster(const dom::Document &document, const RenderStyle &parentStyle, const RenderStyle *parentBoxStyle,
             dom::Element *element) noexcept;

    static void AdjustFromBuilder(RenderStyle &style) noexcept;
    void Adjust(RenderStyle &style) const noexcept;
    void AdjustAnimatedStyle(RenderStyle &style, EnumSet<AnimationImpact> impact) const noexcept;

    static void AdjustFirstLetterStyle(RenderStyle &style) noexcept;
    static void AdjustFirstLineStyle(RenderStyle &style) noexcept;
    static void AdjustSVGElementStyle(RenderStyle &style, const svg::SVGElement &svgElement) noexcept;
    static bool AdjustEventListenerRegionTypesForRootStyle(RenderStyle &rootStyle,
                                                           const dom::Document &document) noexcept;
    /// @brief Propagates the writing mode and direction of the document element (or `<body>`) to the initial
    /// containing block and the document element's used style.
    static void PropagateToDocumentElementAndInitialContainingBlock(Update &update,
                                                                    const dom::Document &document) noexcept;
    KRYS_NODISCARD static UniquePtr<RenderStyle>
      RestoreUsedDocumentElementStyleToComputed(const RenderStyle &style) noexcept;

  private:
    void AdjustInternal(RenderStyle &style) const noexcept;
    void AdjustDisplayContentsStyle(RenderStyle &style) const noexcept;

    void AdjustThemeStyle(RenderStyle &style, const RenderStyle &parentStyle) const noexcept;

    KRYS_NODISCARD static EnumSet<EventListenerRegionType>
      ComputeEventListenerRegionTypes(const dom::Document &document, const RenderStyle &style,
                                      const dom::EventTarget &eventTarget,
                                      EnumSet<EventListenerRegionType> parentTypes) noexcept;
  };
}
