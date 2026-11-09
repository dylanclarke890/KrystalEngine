/*
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 * Copyright (C) 2013, 2014 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/HTML/Canvas/TextMetrics.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/Painting/CSSFilterRenderer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include "Krystal.Booey/Text/UnicodeBidi.hpp"

namespace krys::boo::html
{
  UniquePtr<CanvasRenderingContext2D>
    CanvasRenderingContext2D::Create(CanvasBase &canvas, CanvasRenderingContext2DSettings &&settings,
                                     bool usesCSSCompatibilityParseMode) noexcept
  {
    return CreateUnique<CanvasRenderingContext2D>(canvas, krys::move(settings),
                                                  usesCSSCompatibilityParseMode);
  }

  CanvasRenderingContext2D::CanvasRenderingContext2D(CanvasBase &canvas,
                                                     CanvasRenderingContext2DSettings &&settings,
                                                     bool usesCSSCompatibilityParseMode) noexcept
      : CanvasRenderingContext2DBase(canvas, Type::CanvasElement2D, krys::move(settings),
                                     usesCSSCompatibilityParseMode)
  {
  }

  CanvasRenderingContext2D::~CanvasRenderingContext2D() noexcept = default;

  HTMLCanvasElement &CanvasRenderingContext2D::Canvas() const noexcept
  {
    return Downcast<HTMLCanvasElement>(GetCanvasBase());
  }

  Maybe<style::Filter>
    CanvasRenderingContext2D::SetFilterStringWithoutUpdatingStyle(dom::DOMStringView filterString) noexcept
  {
    auto &canvas = Canvas();
    auto &document = canvas.NodeDocument();
    document.UpdateStyleIfNeeded();

    const auto *style = canvas.ComputedStyle();
    if (!style)
    {
      return null;
    }

    auto parserContext = css::ParserContext(css::StrictToParserMode(!UsesCSSCompatibilityParseMode()));
    return css::PropertyParserHelpers::ParseFilterValueListOrNoneRaw(
      css::CSSOMString(filterString), parserContext, document, const_cast<RenderStyle &>(*style));
  }

  RefPtr<Filter> CanvasRenderingContext2D::CreateFilter(const FloatRect &bounds) const noexcept
  {
    if (bounds.IsEmpty())
    {
      return nullptr;
    }

    auto *context = EffectiveDrawingContext();
    if (!context)
    {
      return nullptr;
    }

    auto *renderer = Canvas().Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    auto *page = Canvas().NodeDocument().GetPage();
    if (!page)
    {
      return nullptr;
    }

    auto outsets = CalculateFilterOutsets(bounds);
    auto filterRegion = bounds + ToFloatBoxExtent(outsets);

    auto preferredFilterRenderingModes = page->PreferredFilterRenderingModes(*context);
    auto filter = CSSFilterRenderer::Create(*renderer, GetState().Filter,
                                            {
                                              .ReferenceBox = bounds,
                                              .FilterRegion = filterRegion,
                                              .Scale = {1, 1},
                                            },
                                            preferredFilterRenderingModes, *context);
    if (!filter)
    {
      return nullptr;
    }

    return filter;
  }

  IntOutsets CanvasRenderingContext2D::CalculateFilterOutsets(const FloatRect &bounds) const noexcept
  {
    if (GetState().Filter.IsNone())
    {
      return {};
    }

    auto *renderer = Canvas().Renderer();
    if (!renderer)
    {
      return {};
    }

    return CSSFilterRenderer::CalculateOutsets(*renderer, GetState().Filter, bounds);
  }

  void CanvasRenderingContext2D::DrawFocusIfNeeded(dom::Element &element) noexcept
  {
    DrawFocusIfNeededInternal(_path, element);
  }

  void CanvasRenderingContext2D::DrawFocusIfNeeded(Path2D &path, dom::Element &element) noexcept
  {
    DrawFocusIfNeededInternal(path.GetPath(), element);
  }

  // https://html.spec.whatwg.org/#draw-focus-if-needed
  void CanvasRenderingContext2D::DrawFocusIfNeededInternal(const Path &path, dom::Element &element) noexcept
  {
    // 1. If element is not focused or is not a descendant of context's canvas element, then return.
    // (Nor is an empty path drawn.)
    auto *context = EffectiveDrawingContext();
    auto &canvas = Canvas();
    if (!element.Focused() || !HasInvertibleTransform() || path.IsEmpty() || !element.IsDescendantOf(canvas)
        || !context)
    {
      return;
    }

    // 2. Draw a focus ring of the appropriate style along path, following platform conventions.
    // 3. Inform the user that the focus is at the location given by the intended path. (Skipped: there's no
    //    accessibility.)
    context->DrawFocusRing(path, 1,
                           RenderTheme::Singleton().FocusRingColor(
                             element.NodeDocument().StyleColorOptions(canvas.ComputedStyle())));
    DidDrawEntireCanvas();
  }

  // https://html.spec.whatwg.org/#dom-context-2d-font
  void CanvasRenderingContext2D::SetFont(dom::DOMStringView newFont) noexcept
  {
    Canvas().NodeDocument().UpdateStyleIfNeeded();

    SetFontWithoutUpdatingStyle(newFont);
  }

  void CanvasRenderingContext2D::SetFontWithoutUpdatingStyle(dom::DOMStringView newFont) noexcept
  {
    // Style isn't updated here: the canvas entry points update it once when they begin, and this is called in
    // the middle of their work. The style may be stale here, but that's better than changing it mid-call.

    if (newFont.empty())
    {
      return;
    }

    if (newFont == GetState().UnparsedFont && GetState().Font.Realized())
    {
      return;
    }

    auto &canvas = Canvas();
    auto &document = canvas.NodeDocument();

    // According to http://lists.w3.org/Archives/Public/public-html/2009Jul/0947.html,
    // the "inherit" and "initial" values must be ignored. ParseUnresolvedFont ignores these.
    auto unresolvedFont = css::PropertyParserHelpers::ParseUnresolvedFont(
      css::CSSOMString(newFont), document, css::StrictToParserMode(!UsesCSSCompatibilityParseMode()));
    if (!unresolvedFont)
    {
      return;
    }

    FontCascadeDescription fontDescription;
    if (const auto *computedStyle = canvas.ComputedStyle())
    {
      fontDescription = FontCascadeDescription {computedStyle->GetFontDescription()};
    }
    else
    {
      fontDescription.SetOneFamily(StringAtom(DefaultFontFamily));
      fontDescription.SetSpecifiedSize(DefaultFontSize);
      fontDescription.SetComputedSize(DefaultFontSize);
    }
    // The font's own longhands come from the new font; the other text drawing styles apply over it.
    ApplyTextDrawingStyles(fontDescription, false);

    // Map the <canvas> font into the text style. If the font uses keywords like larger/smaller, these will
    // work relative to the canvas.
    auto fontCascade =
      style::ResolveForUnresolvedFont(*unresolvedFont, krys::move(fontDescription), document);
    if (!fontCascade)
    {
      return;
    }

    auto newFontSafeCopy =
      dom::DOMString(newFont); // A copy, since newFont can be deleted inside RealizeSaves.
    RealizeSaves();
    ModifiableState().UnparsedFont = krys::move(newFontSafeCopy);

    ModifiableState().Font.Initialize(document.GetFontSelector(), *fontCascade);
    krys_debug_assert(GetState().Font.Realized());
    krys_debug_assert(GetState().Font.IsPopulated());
    TakeFontLonghands(GetState().Font.GetFontDescription());

    // Recompute the word and the letter spacing for the new font.
    dom::DOMString letterSpacing;
    SetLetterSpacing(std::exchange(ModifiableState().LetterSpacing, letterSpacing));
    dom::DOMString wordSpacing;
    SetWordSpacing(std::exchange(ModifiableState().WordSpacing, wordSpacing));
  }

  TextDirection CanvasRenderingContext2D::ToTextDirection(Direction direction,
                                                          const RenderStyle **computedStyle) const noexcept
  {
    const auto *style =
      computedStyle || direction == Direction::Inherit ? Canvas().ExistingComputedStyle() : nullptr;
    if (computedStyle)
    {
      *computedStyle = style;
    }
    switch (direction)
    {
      case Direction::Inherit:
        return style ? style->GetWritingMode().ComputedTextDirection() : TextDirection::LTR;
      case Direction::Rtl: return TextDirection::RTL;
      case Direction::Ltr: return TextDirection::LTR;
    }
    krys_debug_assert(false);
    return TextDirection::LTR;
  }

  CanvasDirection CanvasRenderingContext2D::GetDirection() const noexcept
  {
    if (GetState().Direction == Direction::Inherit)
    {
      Canvas().NodeDocument().UpdateStyleIfNeeded();
    }
    return ToTextDirection(GetState().Direction) == TextDirection::RTL ? CanvasDirection::Rtl
                                                                       : CanvasDirection::Ltr;
  }

  // https://html.spec.whatwg.org/#dom-context-2d-filltext
  void CanvasRenderingContext2D::FillText(dom::DOMStringView text, double x, double y,
                                          Maybe<double> maxWidth) noexcept
  {
    DrawTextInternal(text, x, y, true, maxWidth);
  }

  void CanvasRenderingContext2D::StrokeText(dom::DOMStringView text, double x, double y,
                                            Maybe<double> maxWidth) noexcept
  {
    DrawTextInternal(text, x, y, false, maxWidth);
  }

  // https://html.spec.whatwg.org/#dom-context-2d-measuretext
  // (Runs the text preparation algorithm and returns a TextMetrics object with the glyphs' measurements.)
  Ref<TextMetrics> CanvasRenderingContext2D::MeasureText(dom::DOMStringView text) noexcept
  {
    Canvas().NodeDocument().UpdateStyleIfNeeded();

    auto normalizedText = NormalizeSpaces(text);
    const RenderStyle *computedStyle;
    auto direction = ToTextDirection(GetState().Direction, &computedStyle);
    bool override = computedStyle && IsOverride(computedStyle->GetUnicodeBidi());
    TextRun textRun(normalizedText, 0, 0, ExpansionBehavior::AllowRightOnly(), direction, override, true);
    return MeasureTextInternal(textRun);
  }

  auto CanvasRenderingContext2D::GetFontProxy() noexcept -> const FontProxy *
  {
    // Style isn't updated here, for the same reason as in SetFontWithoutUpdatingStyle.
    if (!GetState().Font.Realized())
    {
      SetFontWithoutUpdatingStyle(GetState().UnparsedFont);
    }
    return &GetState().Font;
  }

  StringAtom CanvasRenderingContext2D::InheritedLanguage() const noexcept
  {
    return StringAtom(css::EffectiveLang(Canvas()));
  }

  void CanvasRenderingContext2D::DrawTextInternal(dom::DOMStringView text, double x, double y, bool fill,
                                                  Maybe<double> maxWidth) noexcept
  {
    Canvas().NodeDocument().UpdateStyleIfNeeded();

    // 1. If any of the arguments are infinite or NaN, then return.
    // (Nor with a maximum width that isn't positive, which the text preparation algorithm makes no glyphs
    // for.)
    if (!CanDrawText(x, y, fill, maxWidth))
    {
      return;
    }

    // 2. Run the text preparation algorithm, passing it text, the object implementing the CanvasText
    //    interface, and, if the maxWidth argument was provided, that argument. Let glyphs be the result.
    // 3. Move all the shapes in glyphs to the right by x CSS pixels and down by y CSS pixels.
    // 4. Paint the shapes given in glyphs, as transformed by the current transformation matrix [...]
    auto normalizedText = NormalizeSpaces(text);
    const RenderStyle *computedStyle;
    auto direction = ToTextDirection(GetState().Direction, &computedStyle);
    bool override = computedStyle && IsOverride(computedStyle->GetUnicodeBidi());
    TextRun textRun(normalizedText, 0, 0, ExpansionBehavior::AllowRightOnly(), direction, override, true);
    DrawTextUnchecked(textRun, x, y, fill, maxWidth);
  }
}
