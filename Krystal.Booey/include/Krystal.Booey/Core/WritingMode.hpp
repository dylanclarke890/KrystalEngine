#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  enum class WritingModeFlags : uint8
  {
    None = 0,
    IsVerticalText = 1 << 0, // Vertical writing modes.
    IsFlippedBlock = 1 << 1, // RL or BT block flow directions.
    IsVerticalType = 1 << 2, // Vertical typographic mode.
    IsBidiRTL = 1 << 3,      // Bidi directionality.
    IsUprightType = 1 << 4,  // Upright text orientation.
    IsSidewaysType = 1 << 5, // Sideways text orientation.

    BlockFlowMask = IsVerticalText | IsFlippedBlock,
    WritingModeMask = BlockFlowMask | IsVerticalType,
    OrientationMask = IsSidewaysType | IsUprightType, // Both is an error.
    OrientationShift = 4,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::WritingModeFlags, 7uz);

namespace krys::boo
{
  enum class StyleWritingMode : uint8
  {
    HorizontalTb = 0,
    SidewaysLr = WritingModeFlags::IsVerticalText,
    HorizontalBt = WritingModeFlags::IsFlippedBlock, // Non-standard.
    SidewaysRl = WritingModeFlags::IsVerticalText | WritingModeFlags::IsFlippedBlock,
    VerticalLr = WritingModeFlags::IsVerticalText | WritingModeFlags::IsVerticalType,
    VerticalRl =
      WritingModeFlags::IsVerticalText | WritingModeFlags::IsVerticalType | WritingModeFlags::IsFlippedBlock,
  };

  enum class TextDirection : bool
  {
    LTR = 0,
    RTL = 1
  };

  enum class TextOrientation : uint8
  {
    Mixed = 0,
    Upright = 1,
    Sideways = 2,
  };

  enum class FlowDirection : uint8
  {
    TopToBottom = 0,
    LeftToRight = WritingModeFlags::IsVerticalText,
    BottomToTop = WritingModeFlags::IsFlippedBlock,
    RightToLeft = WritingModeFlags::IsVerticalText | WritingModeFlags::IsFlippedBlock,
  };

  enum class BoxAxis : uint8;
  enum class LogicalBoxAxis : uint8;

  /// WritingMode efficiently stores a writing mode and can rapidly compute interesting things about it for
  /// use in layout. It fits in 1 byte, and can be cheaply copied or passed around by value.
  ///
  /// Writing modes are computed from the CSS 'writing-mode', 'direction', and 'text-orientation' properties.
  /// See CSS Writing Modes for more information. http://www.w3.org/TR/css-writing-modes/
  ///
  /// Be careful with the distinctions among these:
  ///   - IsBidiLTR/RTL (for typesetting, i.e. within a line box)
  ///   - IsInlineLeftToRight/TopToBottom (absolute physical directions)
  ///   - IsLogicalLeftInlineStart (flow-relative cmp coordinate-relative)
  ///   - IsLogicalLeftLineLeft (line-relative cmp coordinate-relative)
  ///   - ComputedTextDirection (style computation / element directionality)
  ///
  class WritingMode final
  {
  public:
    using Data = uint8;

  private:
    Data _bits {0u};

  public:
    constexpr WritingMode(StyleWritingMode writingMode, TextDirection bidiDirection,
                          TextOrientation verticalOrientation) noexcept
        : _bits(static_cast<Data>(writingMode)
                | (bidiDirection == TextDirection::RTL ? WritingModeFlags::IsBidiRTL : WritingModeFlags::None)
                | (static_cast<Data>(verticalOrientation)
                   << static_cast<Data>(WritingModeFlags::OrientationShift)))
    {
    }

    constexpr WritingMode(Data bits = 0) noexcept : _bits(bits)
    {
    }

    friend constexpr bool operator==(WritingMode, WritingMode) noexcept = default;

    KRYS_NODISCARD constexpr Data data() const noexcept
    {
      return _bits;
    }

    constexpr void SetWritingMode(StyleWritingMode writingMode) noexcept
    {
      _bits &= ~WritingModeFlags::WritingModeMask;
      _bits |= static_cast<Data>(writingMode);
    }
    constexpr void SetTextOrientation(TextOrientation verticalOrientation) noexcept
    {
      _bits &= ~WritingModeFlags::OrientationShift;
      _bits |= static_cast<Data>(verticalOrientation)
               << static_cast<Data>(WritingModeFlags::OrientationShift);
    }

    constexpr void SetTextDirection(TextDirection bidiDirection) noexcept
    {
      if (bidiDirection == TextDirection::RTL)
      {
        _bits |= WritingModeFlags::IsBidiRTL;
      }
      else
      {
        _bits &= ~WritingModeFlags::IsBidiRTL;
      }
    }

#pragma region Writing Axis

    KRYS_NODISCARD constexpr bool IsHorizontal() const noexcept
    {
      return !(_bits & WritingModeFlags::IsVerticalText);
    }

    KRYS_NODISCARD constexpr bool IsVertical() const noexcept
    {
      return _bits & WritingModeFlags::IsVerticalText;
    }

#pragma endregion

#pragma region(Mis)Matching

    KRYS_NODISCARD constexpr bool IsOrthogonal(WritingMode writingMode) const noexcept
    {
      return IsVertical() != (writingMode.IsVertical());
    }

    KRYS_NODISCARD constexpr bool IsBlockOpposing(WritingMode writingMode) const noexcept
    {
      Data self = (_bits & (WritingModeFlags::BlockFlowMask));
      Data other = (writingMode._bits & (WritingModeFlags::BlockFlowMask));
      return static_cast<WritingModeFlags>(self ^ other) == WritingModeFlags::IsFlippedBlock;
    }

    KRYS_NODISCARD constexpr bool IsInlineOpposing(WritingMode writingMode) const noexcept
    {
      return IsHorizontal() ? IsBidiRTL() != writingMode.IsBidiRTL()
                            : IsInlineTopToBottom() != writingMode.IsInlineTopToBottom();
    }

    KRYS_NODISCARD constexpr bool IsBlockMatchingAny(WritingMode writingMode) const noexcept
    {
      return IsHorizontal() ? IsBlockTopToBottom() == writingMode.IsAnyTopToBottom()
                            : IsBlockLeftToRight() == writingMode.IsAnyLeftToRight();
    }

    KRYS_NODISCARD constexpr bool IsInlineMatchingAny(WritingMode writingMode) const noexcept
    {
      return IsHorizontal() ? IsBidiLTR() == writingMode.IsAnyLeftToRight()
                            : IsInlineTopToBottom() == writingMode.IsAnyTopToBottom();
    }

#pragma endregion

#pragma region Text directionality. (FOR TYPESETTING ONLY)

    /// @brief line-right to line-left
    KRYS_NODISCARD constexpr bool IsBidiRTL() const noexcept
    {
      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=164507
      return _bits & WritingModeFlags::IsBidiRTL;
    }

    /// @brief line-left to line-right
    KRYS_NODISCARD constexpr bool IsBidiLTR() const noexcept
    {
      return !IsBidiRTL();
    }

#pragma endregion

#pragma region Physical flow directions

    KRYS_NODISCARD constexpr bool IsBlockTopToBottom() const noexcept
    {
      return !(_bits & WritingModeFlags::BlockFlowMask);
    }

    KRYS_NODISCARD constexpr bool IsBlockLeftToRight() const noexcept
    {
      return static_cast<WritingModeFlags>(_bits & WritingModeFlags::BlockFlowMask)
             == WritingModeFlags::IsVerticalText;
    }

    KRYS_NODISCARD constexpr bool IsInlineTopToBottom() const noexcept
    {
      // Using bitwise operators to avoid conditionals due to short-circuiting...
      return static_cast<unsigned>(IsVertical())
             & (static_cast<unsigned>(IsBidiRTL())
                ^ (static_cast<unsigned>(IsVerticalTypographic()) | static_cast<unsigned>(IsBlockFlipped())));
    }

    KRYS_NODISCARD constexpr bool IsInlineLeftToRight() const noexcept
    {
      return !(_bits & (WritingModeFlags::IsVerticalText | WritingModeFlags::IsBidiRTL));
    }

    // Block OR inline flow is top-to-bottom.
    KRYS_NODISCARD constexpr bool IsAnyTopToBottom() const noexcept
    {
      return IsBlockTopToBottom() || IsInlineTopToBottom();
    }

    // Block OR inline flow is left-to-right.
    KRYS_NODISCARD constexpr bool IsAnyLeftToRight() const noexcept
    {
      return IsInlineLeftToRight() || IsBlockLeftToRight();
    }

#pragma endregion

#pragma region Typesetting modes

    KRYS_NODISCARD constexpr bool IsVerticalTypographic() const noexcept
    {
      return _bits & WritingModeFlags::IsVerticalType;
    }

    KRYS_NODISCARD constexpr bool PrefersCentralBaseline() const noexcept
    {
      return IsVerticalTypographic() && !IsSidewaysOrientation();
    }

    KRYS_NODISCARD constexpr bool IsMixedOrientation() const noexcept
    {
      return !(_bits & WritingModeFlags::OrientationMask);
    }

    KRYS_NODISCARD constexpr bool IsUprightOrientation() const noexcept
    {
      return _bits & WritingModeFlags::IsUprightType;
    }

    KRYS_NODISCARD constexpr bool IsSidewaysOrientation() const noexcept
    {
      return _bits & WritingModeFlags::IsSidewaysType;
    }

#pragma endregion

#pragma region Line orientation

    /// @brief line-over != block-start
    KRYS_NODISCARD constexpr bool IsLineInverted() const noexcept
    {
      auto bits = _bits & WritingModeFlags::WritingModeMask;
      bool isHorizontalBt = static_cast<uint8>(WritingModeFlags::IsFlippedBlock) == bits;
      bool isVerticalLr =
        static_cast<uint8>(WritingModeFlags::IsVerticalText | WritingModeFlags::IsVerticalType) == bits;
      return isHorizontalBt || isVerticalLr;
    }

    /// @brief line-over == right
    KRYS_NODISCARD constexpr bool IsLineOverRight() const noexcept
    {
      return IsVerticalTypographic()
             || static_cast<WritingModeFlags>(_bits & WritingModeFlags::WritingModeMask)
                  == (WritingModeFlags::IsVerticalText | WritingModeFlags::IsFlippedBlock); // sideways-rl
    }

    /// @brief line-over == left
    KRYS_NODISCARD constexpr bool IsLineOverLeft() const noexcept
    {
      return static_cast<WritingModeFlags>(_bits & WritingModeFlags::WritingModeMask)
             == WritingModeFlags::IsVerticalText; // sideways-lr
    }

#pragma endregion

#pragma region Coordinate Flow Queries

    /// @brief Not true for sideways-lr.
    KRYS_NODISCARD constexpr bool IsLogicalLeftLineLeft() const noexcept
    {
      return ComputedWritingMode() != StyleWritingMode::SidewaysLr;
    }

    /// @brief == !IsInlineFlipped()
    KRYS_NODISCARD constexpr bool IsLogicalLeftInlineStart() const noexcept
    {
      return !IsInlineFlipped();
    }

    /// @brief Inline direction is RTL or BTT.
    KRYS_NODISCARD constexpr bool IsInlineFlipped() const noexcept
    {
      if (IsHorizontal())
      {
        return IsBidiRTL();
      }

      return !IsInlineTopToBottom();
    }

    /// @brief == Block direction is RL or BT.
    KRYS_NODISCARD constexpr bool IsBlockFlipped() const noexcept
    {
      return _bits & WritingModeFlags::IsFlippedBlock;
    }

#pragma endregion

#pragma region Direction Queries

    KRYS_NODISCARD constexpr FlowDirection BlockDirection() const noexcept
    {
      return static_cast<FlowDirection>(_bits & WritingModeFlags::BlockFlowMask);
    }

    KRYS_NODISCARD constexpr TextDirection BidiDirection() const noexcept
    {
      return ComputedTextDirection();
    }

    KRYS_NODISCARD constexpr FlowDirection InlineDirection() const noexcept
    {
      if (IsHorizontal())
      {
        return IsInlineLeftToRight() ? FlowDirection::LeftToRight : FlowDirection::RightToLeft;
      }

      return IsInlineTopToBottom() ? FlowDirection::TopToBottom : FlowDirection::BottomToTop;
    }

#pragma endregion

#pragma region Axis Queries

    KRYS_NODISCARD constexpr BoxAxis BlockAxis() const noexcept;

    KRYS_NODISCARD constexpr BoxAxis InlineAxis() const noexcept;

    KRYS_NODISCARD constexpr LogicalBoxAxis HorizontalAxis() const noexcept;

    KRYS_NODISCARD constexpr LogicalBoxAxis VerticalAxis() const noexcept;

#pragma endregion

#pragma region Computed Values

    KRYS_NODISCARD constexpr StyleWritingMode ComputedWritingMode() const noexcept
    {
      return static_cast<StyleWritingMode>(_bits & WritingModeFlags::WritingModeMask);
    }

    KRYS_NODISCARD constexpr TextDirection ComputedTextDirection() const noexcept
    {
      return static_cast<TextDirection>(IsBidiRTL());
    }

    KRYS_NODISCARD constexpr TextOrientation ComputedTextOrientation() const noexcept
    {
      Data data =
        (_bits & WritingModeFlags::OrientationMask) >> static_cast<Data>(WritingModeFlags::OrientationShift);
      return static_cast<TextOrientation>(data);
    }

#pragma endregion
  };
}
