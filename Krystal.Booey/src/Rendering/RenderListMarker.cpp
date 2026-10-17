#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD style::Difference AdjustedStyleDifference(style::Difference diff,
                                                             const RenderStyle &oldStyle,
                                                             const RenderStyle &newStyle) noexcept
    {
      if (diff >= style::DifferenceResult::Layout)
      {
        return diff;
      }
      // FIXME: Preferably we do this at RenderStyle::changeRequiresLayout but checking against
      // pseudo(::marker) is not sufficient.
      auto needsLayout = oldStyle.GetListStylePosition() != newStyle.GetListStylePosition()
                         || oldStyle.GetListStyleType() != newStyle.GetListStyleType()
                         || oldStyle.IsDisplayInlineType() != newStyle.IsDisplayInlineType();
      return needsLayout ? style::Difference {style::DifferenceResult::Layout} : diff;
    }

    KRYS_NODISCARD TextDirection ContentTextDirection(utf8_stringview content) noexcept
    {
      if (content.empty())
      {
        return TextDirection::LTR;
      }
      // FIXME: Depending on the string value, we may need the real bidi algorithm. (rdar://106139180)
      // Also we may need to start checking for the entire content for directionality (and whether we need to
      // check for additional directionality characters like U_RIGHT_TO_LEFT_EMBEDDING).
      auto bidiCategory = u_charDirection(static_cast<UChar32>(*text::CodePointIterator(content)));
      if (bidiCategory != U_RIGHT_TO_LEFT && bidiCategory != U_RIGHT_TO_LEFT_ARABIC)
      {
        return TextDirection::LTR;
      }
      return TextDirection::RTL;
    }

    // This is temporary and will be removed when subpixel inline layout is enabled.
    // FIXME: Stop snapping once inline layout is subpixel.
    enum class SnapDirection : uint8
    {
      Floor,
      Ceil,
      Round
    };
    KRYS_NODISCARD float Snap(float value, SnapDirection direction = SnapDirection::Round) noexcept
    {
      switch (direction)
      {
        case SnapDirection::Floor: return std::floor(value);
        case SnapDirection::Ceil:  return std::ceil(value);
        case SnapDirection::Round: return std::round(value);
      }
      krys_debug_assert(false);
      return value;
    }

    KRYS_NODISCARD utf8_string Reversed(utf8_stringview text) noexcept
    {
      List<char32> codePoints;
      for (auto it = text::CodePointIterator(text); !it.AtEnd(); ++it)
      {
        codePoints.push_back(*it);
      }
      StringBuilder builder;
      for (auto codePoint = codePoints.rbegin(); codePoint != codePoints.rend(); ++codePoint)
      {
        builder.Append(*codePoint);
      }
      return krys::move(builder).ToString();
    }

    KRYS_NODISCARD utf8_string TextForRun(const ListMarkerTextContent &textContent,
                                          const RenderStyle &style) noexcept
    {
      krys_debug_assert(!textContent.IsEmpty());

      // Since the bidi algorithm doesn't run on this text, we instead reorder the characters here.
      // We use u_charDirection to figure out if the marker text is RTL and assume the suffix matches the
      // surrounding direction.
      StringBuilder textForRun;
      if (textContent.TextDirection == TextDirection::LTR)
      {
        if (style.GetWritingMode().IsBidiLTR())
        {
          return textContent.TextWithSuffix;
        }
        textForRun.Append(Reversed(textContent.Suffix()), textContent.TextWithoutSuffix());
      }
      else
      {
        if (!style.GetWritingMode().IsBidiLTR())
        {
          return Reversed(textContent.TextWithSuffix);
        }
        textForRun.Append(Reversed(textContent.TextWithoutSuffix()), textContent.Suffix());
      }
      return krys::move(textForRun).ToString();
    }
  }

  RenderListMarker::RenderListMarker(RenderListItem &listItem, RenderStyle &&style) noexcept
      : RenderBox(Type::ListMarker, listItem.GetDocument(), krys::move(style)),
        _listItem(CreateWeakPtr(&listItem))
  {
    SetInline(true);
    SetBlockLevelReplacedOrAtomicInline(true); // pretend to be replaced
    krys_debug_assert(IsRenderListMarker());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderListMarker::~RenderListMarker() noexcept = default;

  void RenderListMarker::WillBeDestroyed() noexcept
  {
    // FIXME: Stop being the image's client, once there's image loading.
    RenderBox::WillBeDestroyed();
  }

  void RenderListMarker::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    RenderBox::StyleWillChange(AdjustedStyleDifference(diff, Style(), newStyle), newStyle);
  }

  void RenderListMarker::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    if (oldStyle)
    {
      diff = AdjustedStyleDifference(diff, *oldStyle, Style());
    }
    RenderBox::StyleDidChange(diff, oldStyle);

    if (auto newImage = Style().ListStyleImage().TryStyleImage(); _image != newImage)
    {
      // FIXME: Stop being the old image's client and become the new one's, once there's image loading.
      _image = krys::move(newImage);
    }
  }

  bool RenderListMarker::IsImage() const noexcept
  {
    return _image && !_image->ErrorOccurred();
  }

  RenderBox *RenderListMarker::ParentBox(RenderBox &box) noexcept
  {
    krys_debug_assert(_listItem);
    // FIXME: The column spanner placeholder's parent inside a multi-column flow, once there's multi-column
    // layout.
    return box.ParentBox();
  }

  void RenderListMarker::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    LayoutUnit blockOffset;
    for (auto *ancestor = ParentBox(*this); ancestor && ancestor != _listItem.get();
         ancestor = ParentBox(*ancestor))
    {
      blockOffset += ancestor->LogicalTop();
    }

    _lineLogicalOffsetForListItem = _listItem->LogicalLeftOffsetForLine(blockOffset);

    if (IsImage())
    {
      UpdateInlineMarginsAndContent();
      // FIXME: The image's size, once there's image loading.
      SetWidth(0_lu);
      SetHeight(0_lu);
      _layoutBounds = {Height().ToFloat(), 0.f};
    }
    else
    {
      SetLogicalWidth(MinPreferredLogicalWidth());
      SetLogicalHeight(LayoutUnit(Style().MetricsOfPrimaryFont().IntHeight()));
      _layoutBounds = LayoutBoundForTextContent(_textContent.TextWithSuffix);
    }

    SetMarginStart(0_lu);
    SetMarginEnd(0_lu);

    if (auto fixedStartMargin = Style().MarginStart().TryFixed())
    {
      SetMarginStart(LayoutUnit(fixedStartMargin->ResolveZoom(Style().UsedZoomForLength())));
    }
    if (auto fixedEndMargin = Style().MarginEnd().TryFixed())
    {
      SetMarginEnd(LayoutUnit(fixedEndMargin->ResolveZoom(Style().UsedZoomForLength())));
    }

    ClearNeedsLayout();
  }

  void RenderListMarker::UpdateInlineMarginsAndContent() noexcept
  {
    // FIXME: It's messy to use the preferredLogicalWidths dirty bit for this optimization, also unclear if
    // this is premature optimization.
    if (NeedsPreferredLogicalWidthsUpdate())
    {
      UpdateContent();
    }
    UpdateInlineMargins();
  }

  void RenderListMarker::UpdateContent() noexcept
  {
    if (IsImage())
    {
      // FIXME: The image's container size (a default bullet of half the ascent, or the image's intrinsic
      // size), once there's image loading.
      _textContent = {
        .TextWithSuffix = {},
        .TextWithoutSuffixLength = 0,
        .TextDirection = TextDirection::LTR,
      };
      return;
    }

    Style().GetListStyleType().SwitchOn(
      [&](const css::keywords::None &)
      {
        _textContent = {
          .TextWithSuffix = u8" ",
          .TextWithoutSuffixLength = 0,
          .TextDirection = TextDirection::LTR,
        };
      },
      [&](const css::CSSOMStringAtom &identifier)
      {
        _textContent = {
          .TextWithSuffix = utf8_string(identifier.View()),
          .TextWithoutSuffixLength = identifier.View().size(),
          .TextDirection = ContentTextDirection(identifier.View()),
        };
      },
      [&](const style::CounterStyle &)
      {
        auto counter = GetCounterStyle();
        krys_debug_assert(counter);

        StringBuilder text;
        text.Append(counter->Prefix().Text, counter->Text(_listItem->Value(), Style().GetWritingMode()));
        auto textWithoutSuffix = krys::move(text).ToString();
        StringBuilder textWithSuffix;
        textWithSuffix.Append(textWithoutSuffix, counter->Suffix().Text);
        _textContent = {
          .TextWithSuffix = krys::move(textWithSuffix).ToString(),
          .TextWithoutSuffixLength = textWithoutSuffix.size(),
          .TextDirection = ContentTextDirection(textWithoutSuffix),
        };
      });
  }

  void RenderListMarker::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());
    UpdateContent();

    if (IsImage())
    {
      // FIXME: The image's size, once there's image loading.
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = 0_lu;
      ClearNeedsPreferredWidthsUpdate();
      UpdateInlineMargins();
      return;
    }

    const FontCascade &font = Style().GetFontCascade();

    LayoutUnit logicalWidth;
    if (WidthUsesMetricsOfPrimaryFont())
    {
      logicalWidth = LayoutUnit((font.MetricsOfPrimaryFont().IntAscent() * 2 / 3 + 1) / 2 + 2);
    }
    else if (!_textContent.IsEmpty())
    {
      auto text = TextForRun(_textContent, Style());
      logicalWidth = LayoutUnit(font.Width(TextRun(text)));
    }

    _minPreferredLogicalWidth = logicalWidth;
    _maxPreferredLogicalWidth = logicalWidth;

    ClearNeedsPreferredWidthsUpdate();

    UpdateInlineMargins();
  }

  void RenderListMarker::UpdateInlineMargins() noexcept
  {
    constexpr int32 markerPadding = 7;
    const FontMetrics &fontMetrics = Style().MetricsOfPrimaryFont();

    auto marginsForInsideMarker = [&]() -> Pair<LayoutUnit, LayoutUnit>
    {
      if (IsImage())
      {
        return {0_lu, LayoutUnit(markerPadding)};
      }

      if (WidthUsesMetricsOfPrimaryFont())
      {
        return {LayoutUnit(-1), LayoutUnit(fontMetrics.IntAscent()) - MinPreferredLogicalWidth() + 1};
      }

      return {0_lu, 0_lu};
    };

    auto marginsForOutsideMarker = [&]() -> Pair<LayoutUnit, LayoutUnit>
    {
      if (IsImage())
      {
        return {-MinPreferredLogicalWidth() - markerPadding, LayoutUnit(markerPadding)};
      }

      int32 offset = fontMetrics.IntAscent() * 2 / 3;
      if (WidthUsesMetricsOfPrimaryFont())
      {
        return {LayoutUnit(-offset - markerPadding - 1),
                LayoutUnit(offset + markerPadding + 1) - MinPreferredLogicalWidth()};
      }

      if (_textContent.IsEmpty())
      {
        return {0_lu, 0_lu};
      }

      if (Style().GetListStyleType().IsString())
      {
        return {-MinPreferredLogicalWidth(), 0_lu};
      }

      return {-MinPreferredLogicalWidth() - offset / 2, LayoutUnit(offset / 2)};
    };

    auto [marginStart, marginEnd] = IsInside() ? marginsForInsideMarker() : marginsForOutsideMarker();
    MutableStyle().SetMarginStart(style::MarginEdge::Fixed {marginStart.ToFloat()});
    MutableStyle().SetMarginEnd(style::MarginEdge::Fixed {marginEnd.ToFloat()});
  }

  bool RenderListMarker::IsInside() const noexcept
  {
    return Style().GetListStylePosition() == ListStylePosition::Inside;
  }

  bool RenderListMarker::IsDisclosureMarker() const noexcept
  {
    const auto &listStyleType = Style().GetListStyleType();
    if (auto counterStyle = listStyleType.TryCounterStyle())
    {
      const auto &identifier = counterStyle->Identifier.Value;
      return identifier == u8"disclosure-open" || identifier == u8"disclosure-closed";
    }
    return false;
  }

  const RenderListItem *RenderListMarker::ListItem() const noexcept
  {
    return _listItem.get();
  }

  RefPtr<css::CounterStyle> RenderListMarker::GetCounterStyle() const noexcept
  {
    auto counterStyle = Style().GetListStyleType().TryCounterStyle();
    if (!counterStyle)
    {
      return nullptr;
    }
    return GetDocument().StyleScope().GetCounterStyleRegistry().ResolvedCounterStyle(*counterStyle);
  }

  bool RenderListMarker::WidthUsesMetricsOfPrimaryFont() const noexcept
  {
    const auto &listType = Style().GetListStyleType();
    return listType.IsCircle() || listType.IsDisc() || listType.IsSquare();
  }

  Pair<float, float> RenderListMarker::LayoutBoundForTextContent(utf8_stringview text) const noexcept
  {
    // FIXME: This should be part of the inline box builder.
    // This is essentially what we do in LineBoxBuilder::enclosingAscentDescentWithFallbackFonts.
    auto ascentAndDescent = [&](const FontMetrics &fontMetrics)
    {
      auto ascent = Snap(fontMetrics.Ascent());
      auto descent = Snap(fontMetrics.Descent());
      auto halfLeading = (Snap(fontMetrics.LineSpacing()) - (ascent + descent)) / 2.f;
      return Pair<float, float> {Snap(ascent + halfLeading, SnapDirection::Floor),
                                 Snap(descent + halfLeading, SnapDirection::Ceil)};
    };
    const auto &style = Style();
    const auto &metricsOfPrimaryFont = style.MetricsOfPrimaryFont();
    auto primaryFontHeight = Snap(metricsOfPrimaryFont.Height());

    if (style.GetLineHeight().IsNormal())
    {
      auto maxAscentAndDescent = ascentAndDescent(metricsOfPrimaryFont);

      HashSet<RefPtr<const Font>> fallbackFonts;
      (void)style.GetFontCascade().Width(
        TextRun(text, 0, 0, ExpansionBehavior::DefaultBehavior(), style.GetWritingMode().BidiDirection()),
        &fallbackFonts);
      for (const auto &fallbackFont : fallbackFonts)
      {
        const auto &fontMetrics = fallbackFont->GetFontMetrics();
        if (primaryFontHeight >= Snap(fontMetrics.Height(), SnapDirection::Floor))
        {
          // FIXME: Figure out why certain symbols (e.g. disclosure-open) would initiate fallback fonts with
          // just slightly different (subpixel) metrics. This is mainly about preserving legacy behavior.
          continue;
        }
        auto ascentDescent = ascentAndDescent(fontMetrics);
        maxAscentAndDescent.first = std::max(maxAscentAndDescent.first, ascentDescent.first);
        maxAscentAndDescent.second = std::max(maxAscentAndDescent.second, ascentDescent.second);
      }
      return maxAscentAndDescent;
    }

    auto primaryFontAscentAndDescent = ascentAndDescent(metricsOfPrimaryFont);
    auto halfLeading = (Snap(style.ComputedLineHeight(), SnapDirection::Floor)
                        - (primaryFontAscentAndDescent.first + primaryFontAscentAndDescent.second))
                       / 2.f;
    return {Snap(primaryFontAscentAndDescent.first + halfLeading, SnapDirection::Floor),
            Snap(primaryFontAscentAndDescent.second + halfLeading, SnapDirection::Ceil)};
  }
}
