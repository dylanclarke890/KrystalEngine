#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/TextMetrics.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/CSSFilterRenderer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ResolveForFont.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

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

  void CanvasRenderingContext2D::DrawFocusIfNeededInternal(const Path &path, dom::Element &element) noexcept
  {
    auto *context = EffectiveDrawingContext();
    auto &canvas = Canvas();
    if (!element.Focused() || !HasInvertibleTransform() || path.IsEmpty() || !element.IsDescendantOf(canvas)
        || !context)
    {
      return;
    }
    context->DrawFocusRing(path, 1,
                           RenderTheme::Singleton().FocusRingColor(
                             DocumentStyleColorOptions(element.NodeDocument(), canvas.ComputedStyle())));
    DidDrawEntireCanvas();
  }

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

    if (!CanDrawText(x, y, fill, maxWidth))
    {
      return;
    }

    auto normalizedText = NormalizeSpaces(text);
    const RenderStyle *computedStyle;
    auto direction = ToTextDirection(GetState().Direction, &computedStyle);
    bool override = computedStyle && IsOverride(computedStyle->GetUnicodeBidi());
    TextRun textRun(normalizedText, 0, 0, ExpansionBehavior::AllowRightOnly(), direction, override, true);
    DrawTextUnchecked(textRun, x, y, fill, maxWidth);
  }
}
