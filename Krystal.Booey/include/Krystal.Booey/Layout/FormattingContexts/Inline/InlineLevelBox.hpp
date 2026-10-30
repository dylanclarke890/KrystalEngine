#pragma once

#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineFitEdge.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineHeight.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLineBoxContain.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::layout
{
  class LineBox;
  class LineBoxBuilder;
  class LineBoxVerticalAligner;

  /// @brief A box on a line box: the root inline box, an inline box (or the part of one on this line), an
  /// atomic inline box, a line break or a generic inline level box, with its logical rect and vertical
  /// metrics.
  class InlineLevelBox
  {
  public:
    enum class LineSpanningInlineBox : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD static InlineLevelBox
      CreateInlineBox(const Box &layoutBox, const RenderStyle &style, InlineLayoutUnit logicalLeft,
                      InlineLayoutUnit logicalWidth,
                      LineSpanningInlineBox isLineSpanning = LineSpanningInlineBox::No) noexcept;
    KRYS_NODISCARD static InlineLevelBox CreateRootInlineBox(const Box &layoutBox, const RenderStyle &style,
                                                             InlineLayoutUnit logicalLeft,
                                                             InlineLayoutUnit logicalWidth) noexcept;
    KRYS_NODISCARD static InlineLevelBox CreateAtomicInlineBox(const Box &layoutBox, const RenderStyle &style,
                                                               InlineLayoutUnit logicalLeft,
                                                               InlineLayoutUnit logicalWidth) noexcept;
    KRYS_NODISCARD static InlineLevelBox CreateLineBreakBox(const Box &layoutBox, const RenderStyle &style,
                                                            InlineLayoutUnit logicalLeft) noexcept;
    KRYS_NODISCARD static InlineLevelBox CreateGenericInlineLevelBox(const Box &layoutBox,
                                                                     const RenderStyle &style,
                                                                     InlineLayoutUnit logicalLeft) noexcept;

    struct AscentAndDescent
    {
      InlineLayoutUnit Ascent {0};
      InlineLayoutUnit Descent {0};

      KRYS_NODISCARD InlineLayoutUnit Height() const noexcept
      {
        return Ascent + Descent;
      }
      KRYS_NODISCARD friend bool operator==(const AscentAndDescent &,
                                            const AscentAndDescent &) noexcept = default;
    };

    using VerticalAlignment =
      Variant<css::keywords::Baseline, css::keywords::Sub, css::keywords::Super, css::keywords::Top,
              css::keywords::TextTop, css::keywords::Middle, css::keywords::Bottom, css::keywords::TextBottom,
              css::keywords::WebkitBaselineMiddle, InlineLayoutUnit>;

    enum class Type : uint8
    {
      InlineBox,
      LineSpanningInlineBox,
      RootInlineBox,
      AtomicInlineBox,
      LineBreakBox,
      GenericInlineLevelBox
    };

  private:
    friend class InlineDisplayLineBuilder;
    friend class LineBox;
    friend class LineBoxBuilder;
    friend class LineBoxVerticalAligner;
    friend class InlineFormattingUtils;
    friend class RubyFormattingContext;

    enum class PositionWithinLayoutBox : bool
    {
      First,
      Last
    };

    struct StyleData
    {
      const FontMetrics *PrimaryFontMetrics;
      const style::LineHeight *LineHeight;
      boo::TextBoxTrim TextBoxTrim;
      style::TextBoxEdge TextBoxEdge;
      style::LineFitEdge LineFitEdge;
      style::ZoomFactor ZoomFactor {1.0f, 1.0f};
      style::WebkitLineBoxContain LineBoxContain;
      InlineLayoutUnit PrimaryFontSize {0};
      VerticalAlignment VerticalAlignmentValue {};
    };

    struct TextEmphasis
    {
      InlineLayoutUnit Above;
      InlineLayoutUnit Below;
    };

    const Box *_layoutBox;
    // This is the combination of margin and border boxes. Inline level boxes are vertically aligned using
    // their margin boxes.
    InlineRect _logicalRect;
    AscentAndDescent _layoutBounds;
    AscentAndDescent _ascentAndDescent;
    InlineLayoutUnit _inlineBoxContentOffsetForTextBoxTrim {0.f};
    bool _hasContent {false};
    // These bits are about whether this inline level box is the first/last generated box of the associated
    // Layout::Box (e.g. always true for atomic inline level boxes, but inline boxes spanning over multiple
    // lines can produce separate first/last boxes).
    bool _isFirstWithinLayoutBox {false};
    bool _isLastWithinLayoutBox {false};
    Type _type {Type::InlineBox};
    StyleData _style;
    Maybe<TextEmphasis> _textEmphasis;

  public:
    KRYS_NODISCARD InlineLayoutUnit Ascent() const noexcept
    {
      return _ascentAndDescent.Ascent;
    }
    KRYS_NODISCARD InlineLayoutUnit Descent() const noexcept
    {
      return _ascentAndDescent.Descent;
    }
    // See https://www.w3.org/TR/css-inline-3/#layout-bounds
    KRYS_NODISCARD AscentAndDescent LayoutBounds() const noexcept
    {
      return _layoutBounds;
    }

    KRYS_NODISCARD bool HasContent() const noexcept
    {
      return _hasContent;
    }
    void SetHasContent() noexcept
    {
      krys_debug_assert(IsInlineBox());
      _hasContent = true;
    }

    KRYS_NODISCARD const VerticalAlignment &VerticalAlign() const noexcept
    {
      return _style.VerticalAlignmentValue;
    }
    KRYS_NODISCARD bool HasLineBoxRelativeAlignment() const noexcept
    {
      return HoldsAlternative<css::keywords::Top>(_style.VerticalAlignmentValue)
             || HoldsAlternative<css::keywords::Bottom>(_style.VerticalAlignmentValue);
    }

    KRYS_NODISCARD InlineLayoutUnit PreferredLineHeight() const noexcept;
    KRYS_NODISCARD bool IsPreferredLineHeightFontMetricsBased() const noexcept
    {
      return _style.LineHeight->IsNormal();
    }

    KRYS_NODISCARD bool MayStretchLineBox() const noexcept;

    KRYS_NODISCARD const FontMetrics &PrimarymetricsOfPrimaryFont() const noexcept
    {
      return *_style.PrimaryFontMetrics;
    }
    KRYS_NODISCARD InlineLayoutUnit FontSize() const noexcept
    {
      return _style.PrimaryFontSize;
    }

    KRYS_NODISCARD boo::TextBoxTrim GetTextBoxTrim() const noexcept
    {
      return _style.TextBoxTrim;
    }
    KRYS_NODISCARD style::TextBoxEdge GetTextBoxEdge() const noexcept
    {
      return _style.TextBoxEdge;
    }
    KRYS_NODISCARD style::LineFitEdge GetLineFitEdge() const noexcept
    {
      return _style.LineFitEdge;
    }
    KRYS_NODISCARD InlineLayoutUnit InlineBoxContentOffsetForTextBoxTrim() const noexcept
    {
      return _inlineBoxContentOffsetForTextBoxTrim;
    }

    KRYS_NODISCARD bool HasTextEmphasis() const noexcept
    {
      return (HasContent() || IsAtomicInlineBox()) && _textEmphasis.has_value();
    }
    KRYS_NODISCARD Maybe<InlineLayoutUnit> TextEmphasisAbove() const noexcept
    {
      return HasTextEmphasis() ? Maybe<InlineLayoutUnit> {_textEmphasis->Above} : null;
    }
    KRYS_NODISCARD Maybe<InlineLayoutUnit> TextEmphasisBelow() const noexcept
    {
      return HasTextEmphasis() ? Maybe<InlineLayoutUnit> {_textEmphasis->Below} : null;
    }

    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return _type == Type::InlineBox || IsRootInlineBox() || IsLineSpanningInlineBox();
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return _type == Type::RootInlineBox;
    }
    KRYS_NODISCARD bool IsLineSpanningInlineBox() const noexcept
    {
      return _type == Type::LineSpanningInlineBox;
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return _type == Type::AtomicInlineBox;
    }
    KRYS_NODISCARD bool IsListMarker() const noexcept
    {
      return IsAtomicInlineBox() && LayoutBox().IsListMarkerBox();
    }
    KRYS_NODISCARD bool IsLineBreakBox() const noexcept
    {
      return _type == Type::LineBreakBox;
    }

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD const Box &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }

    KRYS_NODISCARD bool IsFirstBox() const noexcept
    {
      return _isFirstWithinLayoutBox;
    }
    KRYS_NODISCARD bool IsLastBox() const noexcept
    {
      return _isLastWithinLayoutBox;
    }

  private:
    InlineLevelBox(const Box &layoutBox, const RenderStyle &style, InlineLayoutUnit logicalLeft,
                   InlineLayoutSize logicalSize, Type type,
                   EnumSet<PositionWithinLayoutBox> positionWithinLayoutBox = {
                     PositionWithinLayoutBox::First, PositionWithinLayoutBox::Last}) noexcept;

    KRYS_NODISCARD const InlineRect &LogicalRect() const noexcept
    {
      return _logicalRect;
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalTop() const noexcept
    {
      return _logicalRect.Top();
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalBottom() const noexcept
    {
      return _logicalRect.Bottom();
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalLeft() const noexcept
    {
      return _logicalRect.Left();
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalRight() const noexcept
    {
      return _logicalRect.Right();
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalWidth() const noexcept
    {
      return _logicalRect.Width();
    }
    KRYS_NODISCARD InlineLayoutUnit LogicalHeight() const noexcept
    {
      return _logicalRect.Height();
    }

    // FIXME: Remove legacy rounding.
    void SetLogicalWidth(InlineLayoutUnit logicalWidth) noexcept
    {
      _logicalRect.SetWidth(logicalWidth);
    }
    void SetLogicalHeight(InlineLayoutUnit logicalHeight) noexcept
    {
      _logicalRect.SetHeight(logicalHeight);
    }
    void SetLogicalTop(InlineLayoutUnit logicalTop) noexcept
    {
      _logicalRect.SetTop(logicalTop);
    }
    void SetLogicalLeft(InlineLayoutUnit logicalLeft) noexcept
    {
      _logicalRect.SetLeft(logicalLeft);
    }
    void SetAscentAndDescent(AscentAndDescent ascentAndDescent) noexcept
    {
      _ascentAndDescent = ascentAndDescent;
    }
    void SetLayoutBounds(const AscentAndDescent &layoutBounds) noexcept
    {
      _layoutBounds = layoutBounds;
    }
    void SetInlineBoxContentOffsetForTextBoxTrim(InlineLayoutUnit offset) noexcept
    {
      _inlineBoxContentOffsetForTextBoxTrim = offset;
    }

    void SetIsFirstBox() noexcept
    {
      _isFirstWithinLayoutBox = true;
    }
    void SetIsLastBox() noexcept
    {
      _isLastWithinLayoutBox = true;
    }

    void SetTextEmphasis(std::pair<InlineLayoutUnit, InlineLayoutUnit> textEmphasis) noexcept
    {
      if (!textEmphasis.first && !textEmphasis.second)
      {
        return;
      }
      if (textEmphasis.first)
      {
        _textEmphasis = TextEmphasis {textEmphasis.first, 0.f};
        return;
      }
      _textEmphasis = TextEmphasis {0.f, textEmphasis.second};
    }
  };
}
