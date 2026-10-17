#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief Block level information shared across child inline formatting contexts.
  class BlockLayoutState
  {
  public:
    struct LineClamp
    {
      size_t MaximumLines {0};
      bool ShouldDiscardOverflow {false};
      bool IsLegacy {true};
    };

    enum class TextBoxTrimSide : bool
    {
      Start,
      End
    };
    using TextBoxTrim = EnumSet<TextBoxTrimSide>;

    struct MarginState
    {
      void ResetBeforeSideOfBlock() noexcept
      {
        krys_debug_assert(AtBeforeSideOfBlock);
        AtBeforeSideOfBlock = {};
      }

      void ResetMarginValues() noexcept
      {
        PositiveMargin = 0_lu;
        NegativeMargin = 0_lu;
        ContentOffsetAfterSelfCollapsingBlock = 0_lu;
      }

      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return PositiveMargin - NegativeMargin;
      }

      // FIXME: This tracks RenderBlockFlow's MarginInfo for now.
      bool CanCollapseWithChildren : 1 {false};
      bool CanCollapseMarginBeforeWithChildren : 1 {false};
      bool CanCollapseMarginAfterWithChildren : 1 {false};
      bool QuirkContainer : 1 {false};
      bool AtBeforeSideOfBlock : 1 {true};
      bool AtAfterSideOfBlock : 1 {false};
      bool HasMarginBeforeQuirk : 1 {false};
      bool HasMarginAfterQuirk : 1 {false};
      bool DeterminedMarginBeforeQuirk : 1 {false};
      LayoutUnit PositiveMargin;
      LayoutUnit NegativeMargin;
      // This tracks the offset initiated by a self collapsing block when "next line top position" diverges
      // from "current line's bottom". See the last paragraph at
      // https://www.w3.org/TR/CSS22/box.html#collapsing-margins
      LayoutUnit ContentOffsetAfterSelfCollapsingBlock;
    };

  private:
    PlacedFloats &_placedFloats;
    Maybe<LineClamp> _lineClamp;
    TextBoxTrim _textBoxTrim;
    style::TextBoxEdge _textBoxEdge;
    Maybe<LayoutUnit> _intrusiveInitialLetterLogicalBottom;
    MarginState _marginState;

  public:
    BlockLayoutState(PlacedFloats &placedFloats, MarginState marginState, Maybe<LineClamp> lineClamp = null,
                     TextBoxTrim textBoxTrim = {}, style::TextBoxEdge textBoxEdge = css::keywords::Auto {},
                     Maybe<LayoutUnit> intrusiveInitialLetterLogicalBottom = null) noexcept
        : _placedFloats(placedFloats), _lineClamp(lineClamp), _textBoxTrim(textBoxTrim),
          _textBoxEdge(textBoxEdge),
          _intrusiveInitialLetterLogicalBottom(intrusiveInitialLetterLogicalBottom), _marginState(marginState)
    {
    }

    KRYS_NODISCARD PlacedFloats &GetPlacedFloats() noexcept
    {
      return _placedFloats;
    }
    KRYS_NODISCARD const PlacedFloats &GetPlacedFloats() const noexcept
    {
      return _placedFloats;
    }
    KRYS_NODISCARD Maybe<LineClamp> GetLineClamp() const noexcept
    {
      return _lineClamp;
    }
    KRYS_NODISCARD TextBoxTrim GetTextBoxTrim() const noexcept
    {
      return _textBoxTrim;
    }
    KRYS_NODISCARD style::TextBoxEdge GetTextBoxEdge() const noexcept
    {
      return _textBoxEdge;
    }
    KRYS_NODISCARD Maybe<LayoutUnit> IntrusiveInitialLetterLogicalBottom() const noexcept
    {
      return _intrusiveInitialLetterLogicalBottom;
    }

    KRYS_NODISCARD MarginState &GetMarginState() noexcept
    {
      return _marginState;
    }
    KRYS_NODISCARD const MarginState &GetMarginState() const noexcept
    {
      return _marginState;
    }
  };
}
