#include "Krystal.Booey/Rendering/StyledMarkedText.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  static void ComputeStyleForPseudoElementStyle(StyledMarkedText::Style &style,
                                                const RenderStyle *pseudoElementStyle,
                                                const PaintInfo &paintInfo) noexcept
  {
    if (!pseudoElementStyle)
    {
      return;
    }

    style.BackgroundColor = pseudoElementStyle->VisitedDependentColorWithColorFilter(
      css::PropertyId::BackgroundColor, paintInfo.PaintBehavior);
    style.TextStyles.FillColor = pseudoElementStyle->UsedStrokeColor();
    style.TextStyles.StrokeColor = pseudoElementStyle->UsedStrokeColor();
    style.TextStyles.HasExplicitlySetFillColor = pseudoElementStyle->HasExplicitlySetColor();

    auto color = TextDecorationPainter::DecorationColor(*pseudoElementStyle, paintInfo.PaintBehavior);
    auto decorationStyle = pseudoElementStyle->GetTextDecorationStyle();
    auto decorations = pseudoElementStyle->TextDecorationLineInEffect();

    if (decorations.HasUnderline())
    {
      style.TextDecorationStyles.Underline.DecorationColor = color;
      style.TextDecorationStyles.Underline.DecorationStyle = decorationStyle;
    }
    if (decorations.HasOverline())
    {
      style.TextDecorationStyles.Overline.DecorationColor = color;
      style.TextDecorationStyles.Overline.DecorationStyle = decorationStyle;
    }
    if (decorations.HasLineThrough())
    {
      style.TextDecorationStyles.Linethrough.DecorationColor = color;
      style.TextDecorationStyles.Linethrough.DecorationStyle = decorationStyle;
    }
  }

  KRYS_NODISCARD static StyledMarkedText ResolveStyleForMarkedText(const MarkedText &markedText,
                                                                   const StyledMarkedText::Style &baseStyle,
                                                                   const RenderText &renderer,
                                                                   const RenderStyle &lineStyle,
                                                                   const PaintInfo &paintInfo) noexcept
  {
    auto style = baseStyle;
    switch (markedText.MarkedType)
    {
      case MarkedText::Type::Correction:
      case MarkedText::Type::DictationAlternatives:
      case MarkedText::Type::Unmarked:              break;
      case MarkedText::Type::GrammarError:
      case MarkedText::Type::SpellingError:
        // FIXME: ::grammar-error and ::spelling-error, once there are document markers.
        break;
      case MarkedText::Type::Highlight:
      {
        auto renderStyle = renderer.Parent()->GetUncachedPseudoStyle(
          style::PseudoElementRequest(PseudoElementType::Highlight, markedText.HighlightName),
          &renderer.Style());
        ComputeStyleForPseudoElementStyle(style, renderStyle.get(), paintInfo);
        break;
      }
      case MarkedText::Type::FragmentHighlight:
      {
        if (auto *renderStyle = renderer.TargetTextPseudoStyle())
        {
          ComputeStyleForPseudoElementStyle(style, renderStyle, paintInfo);
          break;
        }

        style.BackgroundColor = RenderTheme::Singleton().AnnotationHighlightBackgroundColor(
          style::ColorOptions::UseSystemAppearance);
        style.TextStyles.FillColor = RenderTheme::Singleton().AnnotationHighlightForegroundColor(
          style::ColorOptions::UseSystemAppearance);
        break;
      }
      case MarkedText::Type::TextExtractionHighlight:
        // FIXME: This just uses the same color as annotation highlights for now, but we may eventually
        // require a different appearance.
        style.BackgroundColor = RenderTheme::Singleton().AnnotationHighlightBackgroundColor(
          style::ColorOptions::UseSystemAppearance);
        style.TextStyles.FillColor = RenderTheme::Singleton().AnnotationHighlightForegroundColor(
          style::ColorOptions::UseSystemAppearance);
        break;
      case MarkedText::Type::TextMatch:
        // Text matches always use the light system appearance.
        style.BackgroundColor =
          RenderTheme::Singleton().TextSearchHighlightColor(style::ColorOptions::UseSystemAppearance);
        break;
      case MarkedText::Type::DraggedContent:     style.Alpha = 0.25; break;
      case MarkedText::Type::TransparentContent: style.Alpha = 0.0; break;
      case MarkedText::Type::Selection:
      {
        style.TextStyles =
          ComputeTextSelectionPaintStyle(style.TextStyles, renderer, lineStyle, paintInfo, style.TextShadow);

        Color selectionBackgroundColor = renderer.SelectionBackgroundColor();
        style.BackgroundColor = selectionBackgroundColor;
        if (selectionBackgroundColor.IsValid() && selectionBackgroundColor.IsVisible()
            && style.TextStyles.FillColor == selectionBackgroundColor)
        {
          style.BackgroundColor = selectionBackgroundColor.InvertedColorWithAlpha(1.0);
        }
        break;
      }
    }
    StyledMarkedText styledMarkedText = markedText;
    styledMarkedText.MarkedStyle = krys::move(style);
    return styledMarkedText;
  }

  StyledMarkedText::Style
    StyledMarkedText::ComputeStyleForUnmarkedMarkedText(const RenderText &renderer,
                                                        const RenderStyle &lineStyle, bool isFirstLine,
                                                        const PaintInfo &paintInfo) noexcept
  {
    StyledMarkedText::Style markedStyle;
    markedStyle.TextDecorationStyles = TextDecorationPainter::StylesForRenderer(
      renderer, lineStyle.TextDecorationLineInEffect(), isFirstLine, paintInfo.PaintBehavior);
    markedStyle.TextStyles = ComputeTextPaintStyle(renderer, lineStyle, paintInfo);
    markedStyle.TextShadow =
      paintInfo.ForceTextColor() ? style::TextShadows {css::keywords::None {}} : lineStyle.GetTextShadow();
    return markedStyle;
  }

  KRYS_NODISCARD static TextDecorationPainter::Styles
    ComputeStylesForTextDecorations(const TextDecorationPainter::Styles &previousTextDecorationStyles,
                                    const TextDecorationPainter::Styles &currentTextDecorationStyles) noexcept
  {
    auto textDecorations =
      TextDecorationPainter::TextDecorationsInEffectForStyle(currentTextDecorationStyles);

    if (textDecorations.IsNone())
    {
      return previousTextDecorationStyles;
    }

    auto textDecorationStyles = previousTextDecorationStyles;

    if (textDecorations.HasUnderline())
    {
      textDecorationStyles.Underline = currentTextDecorationStyles.Underline;
    }
    if (textDecorations.HasOverline())
    {
      textDecorationStyles.Overline = currentTextDecorationStyles.Overline;
    }
    if (textDecorations.HasLineThrough())
    {
      textDecorationStyles.Linethrough = currentTextDecorationStyles.Linethrough;
    }
    return textDecorationStyles;
  }

  KRYS_NODISCARD static List<StyledMarkedText>
    CoalesceAdjacentWithSameRanges(List<StyledMarkedText> &&styledTexts)
  {
    krys_debug_assert(!styledTexts.empty());
    List<StyledMarkedText> frontmostMarkedTexts;
    frontmostMarkedTexts.push_back(styledTexts[0]);
    for (size_t i = 1; i < styledTexts.size(); ++i)
    {
      auto &text = styledTexts[i];
      auto &previousStyledMarkedText = frontmostMarkedTexts.back();
      // StyledMarkedTexts completely cover each other.
      if (previousStyledMarkedText.StartOffset == text.StartOffset
          && previousStyledMarkedText.EndOffset == text.EndOffset)
      {
        // If either background for two different custom highlight StyledMarkedTexts are not opaque, blend
        // colors together.
        if (previousStyledMarkedText.HighlightName != text.HighlightName
            && (!previousStyledMarkedText.MarkedStyle.BackgroundColor.IsOpaque()
                || !text.MarkedStyle.BackgroundColor.IsOpaque()
                || (text.HighlightName == StringAtom::Null()
                    && text.MarkedStyle.BackgroundColor.IsVisible())))
        {
          previousStyledMarkedText.MarkedStyle.BackgroundColor = BlendSourceOver(
            previousStyledMarkedText.MarkedStyle.BackgroundColor, text.MarkedStyle.BackgroundColor);
        }
        // Take text color of StyledMarkedText, maintaining insertion and priority order.
        if (text.MarkedType != MarkedText::Type::Unmarked
            && text.MarkedStyle.TextStyles.HasExplicitlySetFillColor)
        {
          previousStyledMarkedText.MarkedStyle.TextStyles.FillColor = text.MarkedStyle.TextStyles.FillColor;
        }
        // Take the highlightName of the latest StyledMarkedText, regardless of priority.
        if (text.HighlightName != StringAtom::Null())
        {
          previousStyledMarkedText.HighlightName = text.HighlightName;
        }

        if (previousStyledMarkedText.Priority <= text.Priority)
        {
          previousStyledMarkedText.Priority = text.Priority;
          // If highlight, combine textDecorationStyles accordingly.
          // FIXME: Check for taking textDecorationStyles needs to accommodate other MarkedText type.
          if (text.HighlightName != StringAtom::Null())
          {
            previousStyledMarkedText.MarkedStyle.TextDecorationStyles =
              ComputeStylesForTextDecorations(previousStyledMarkedText.MarkedStyle.TextDecorationStyles,
                                              text.MarkedStyle.TextDecorationStyles);
          }
          // If higher or same priority and opaque, override background color.
          if (text.MarkedStyle.BackgroundColor.IsOpaque())
          {
            previousStyledMarkedText.MarkedStyle.BackgroundColor = text.MarkedStyle.BackgroundColor;
          }
        }
        continue;
      }
      frontmostMarkedTexts.push_back(krys::move(text));
    }
    return frontmostMarkedTexts;
  }

  static void OrderHighlights(const ListHashSet<StringAtom> &markedTextsNames,
                              List<MarkedText> &markedTexts) noexcept
  {
    if (markedTexts.empty())
    {
      return;
    }

    HashMap<StringAtom, int32> markedTextsNamesPriority;
    int32 index = 0;
    for (auto &highlightName : markedTextsNames)
    {
      markedTextsNamesPriority.emplace(highlightName, index);
      index++;
    }

    auto priorityOf = [&](const StringAtom &name)
    {
      auto found = markedTextsNamesPriority.find(name);
      return found == markedTextsNamesPriority.end() ? 0 : found->second;
    };

    index = 0;
    while (index < static_cast<int32>(markedTexts.size() - 1))
    {
      // If two adjacent highlights with same ranges are not in correct priority order, swap them and move on.
      if (markedTexts[index].HighlightName != StringAtom::Null()
          && markedTexts[index + 1].HighlightName != StringAtom::Null()
          && priorityOf(markedTexts[index].HighlightName) > priorityOf(markedTexts[index + 1].HighlightName)
          && markedTexts[index].StartOffset == markedTexts[index + 1].StartOffset
          && markedTexts[index].EndOffset == markedTexts[index + 1].EndOffset)
      {
        std::swap(markedTexts[index], markedTexts[index + 1]);
      }
      ++index;
    }
  }

  List<StyledMarkedText> StyledMarkedText::SubdivideAndResolve(const List<MarkedText> &textsToSubdivide,
                                                               const RenderText &renderer, bool isFirstLine,
                                                               const PaintInfo &paintInfo)
  {
    if (textsToSubdivide.empty())
    {
      return {};
    }

    // Keep track of original order of highlights.
    ListHashSet<StringAtom> markedTextsNames;
    for (auto &markedText : textsToSubdivide)
    {
      if (markedText.HighlightName != StringAtom::Null())
      {
        markedTextsNames.add(markedText.HighlightName);
      }
    }

    auto &lineStyle = isFirstLine ? renderer.FirstLineStyle() : renderer.Style();
    auto baseStyle = ComputeStyleForUnmarkedMarkedText(renderer, lineStyle, isFirstLine, paintInfo);

    if (textsToSubdivide.size() == 1 && textsToSubdivide[0].MarkedType == MarkedText::Type::Unmarked)
    {
      StyledMarkedText styledMarkedText = textsToSubdivide[0];
      styledMarkedText.MarkedStyle = krys::move(baseStyle);
      return {styledMarkedText};
    }

    auto markedTexts = MarkedText::Subdivide(textsToSubdivide, OverlapStrategy::None);
    krys_debug_assert(!markedTexts.empty());
    if (markedTexts.empty()) KRYS_UNLIKELY
    {
      return {};
    }

    // Check if vector contains custom highlights.
    bool containsHighlights = std::ranges::any_of(markedTexts, [](const auto &item)
                                                  { return item.MarkedType == MarkedText::Type::Highlight; });

    // Sort custom highlights to follow correct priority/insertion order.
    if (containsHighlights)
    {
      OrderHighlights(markedTextsNames, markedTexts);

      List<StyledMarkedText> frontmostMarkedTexts;
      frontmostMarkedTexts.reserve(markedTexts.size());
      for (auto &markedText : markedTexts)
      {
        frontmostMarkedTexts.push_back(
          ResolveStyleForMarkedText(markedText, baseStyle, renderer, lineStyle, paintInfo));
      }

      return CoalesceAdjacentWithSameRanges(krys::move(frontmostMarkedTexts));
    }

    // Compute frontmost overlapping styled marked texts.
    List<StyledMarkedText> frontmostMarkedTexts;
    frontmostMarkedTexts.reserve(markedTexts.size());
    frontmostMarkedTexts.push_back(
      ResolveStyleForMarkedText(markedTexts[0], baseStyle, renderer, lineStyle, paintInfo));
    for (size_t i = 1; i < markedTexts.size(); ++i)
    {
      auto &text = markedTexts[i];
      auto &previousStyledMarkedText = frontmostMarkedTexts.back();
      // Marked texts completely cover each other.
      if (previousStyledMarkedText.StartOffset == text.StartOffset
          && previousStyledMarkedText.EndOffset == text.EndOffset)
      {
        previousStyledMarkedText = ResolveStyleForMarkedText(text, previousStyledMarkedText.MarkedStyle,
                                                             renderer, lineStyle, paintInfo);
        continue;
      }
      frontmostMarkedTexts.push_back(
        ResolveStyleForMarkedText(text, baseStyle, renderer, lineStyle, paintInfo));
    }

    return frontmostMarkedTexts;
  }

  template <typename TEqualityFunction>
  KRYS_NODISCARD static List<StyledMarkedText> CoalesceAdjacent(const List<StyledMarkedText> &textsToCoalesce,
                                                                TEqualityFunction &&equalityFunction)
  {
    if (textsToCoalesce.size() <= 1)
    {
      return textsToCoalesce;
    }

    auto areAdjacentMarkedTextsWithSameStyle = [&](const StyledMarkedText &a, const StyledMarkedText &b)
    {
      return a.EndOffset == b.StartOffset && equalityFunction(a.MarkedStyle, b.MarkedStyle);
    };

    List<StyledMarkedText> styledMarkedTexts;
    styledMarkedTexts.reserve(textsToCoalesce.size());
    styledMarkedTexts.push_back(textsToCoalesce[0]);
    for (size_t i = 1; i < textsToCoalesce.size(); ++i)
    {
      auto &text = textsToCoalesce[i];
      auto &previousStyledMarkedText = styledMarkedTexts.back();
      if (areAdjacentMarkedTextsWithSameStyle(previousStyledMarkedText, text))
      {
        previousStyledMarkedText.EndOffset = text.EndOffset;
        continue;
      }
      styledMarkedTexts.push_back(text);
    }

    return styledMarkedTexts;
  }

  List<StyledMarkedText>
    StyledMarkedText::CoalesceAdjacentWithEqualBackground(const List<StyledMarkedText> &markedTexts)
  {
    return CoalesceAdjacent(markedTexts,
                            [&](auto &a, auto &b) { return a.BackgroundColor == b.BackgroundColor; });
  }

  List<StyledMarkedText>
    StyledMarkedText::CoalesceAdjacentWithEqualForeground(const List<StyledMarkedText> &markedTexts)
  {
    return CoalesceAdjacent(
      markedTexts, [&](auto &a, auto &b)
      { return a.TextStyles == b.TextStyles && a.TextShadow == b.TextShadow && a.Alpha == b.Alpha; });
  }

  List<StyledMarkedText>
    StyledMarkedText::CoalesceAdjacentWithEqualDecorations(const List<StyledMarkedText> &markedTexts)
  {
    return CoalesceAdjacent(markedTexts,
                            [&](auto &a, auto &b)
                            {
                              return a.TextDecorationStyles == b.TextDecorationStyles
                                     && a.TextStyles == b.TextStyles && a.TextShadow == b.TextShadow
                                     && a.Alpha == b.Alpha;
                            });
  }
}
