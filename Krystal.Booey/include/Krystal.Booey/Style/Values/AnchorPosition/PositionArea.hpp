#pragma once

#include "Krystal.Booey/Style/Values/AnchorPosition/PositionAreaSpan.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  class WritingMode;
  enum class BoxAxis : uint8;
  enum class ItemPosition : uint8;
  enum class LogicalBoxAxis : uint8;

  namespace style
  {
    // <'position-area'> = none | <position-area>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-area

    // A non-`none` position-area is formed by two spans of opposite axes, that uniquely determine
    // the area of the containing block.
    struct PositionAreaValue
    {
      PositionAreaValue(PositionAreaSpan blockOrXAxis, PositionAreaSpan inlineOrYAxis) noexcept;

      KRYS_NODISCARD constexpr PositionAreaSpan BlockOrXAxis() const noexcept
      {
        return _blockOrXAxis;
      }
      KRYS_NODISCARD constexpr PositionAreaSpan InlineOrYAxis() const noexcept
      {
        return _inlineOrYAxis;
      }

      KRYS_NODISCARD PositionAreaSpan SpanForAxis(BoxAxis physicalAxis, WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept;
      KRYS_NODISCARD PositionAreaSpan SpanForAxis(LogicalBoxAxis logicalAxis,
                                                  WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept;

      // Start/end based on container's coordinate-increasing direction (RenderBox coordinates)
      KRYS_NODISCARD PositionAreaTrack CoordMatchedTrackForAxis(BoxAxis, WritingMode containerWritingMode,
                                                                WritingMode selfWritingMode) const noexcept;

      KRYS_NODISCARD ItemPosition DefaultAlignmentForAxis(BoxAxis, WritingMode containerWritingMode,
                                                          WritingMode selfWritingMode) const noexcept;

      KRYS_NODISCARD constexpr bool operator==(const PositionAreaValue &) const noexcept = default;

    private:
      PositionAreaSpan _blockOrXAxis;
      PositionAreaSpan _inlineOrYAxis;
    };

    struct PositionArea
    {
      PositionArea(css::keywords::None) noexcept : _value {}
      {
      }
      PositionArea(PositionAreaValue value) noexcept : _value {value}
      {
      }

      KRYS_NODISCARD bool IsNone() const noexcept
      {
        return !_value;
      }
      KRYS_NODISCARD bool IsValue() const noexcept
      {
        return !!_value;
      }
      KRYS_NODISCARD Maybe<PositionAreaValue> TryValue() const noexcept
      {
        return _value;
      }

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
        if (IsNone())
        {
          return visitor(css::keywords::None {});
        }
        return visitor(*_value);
      }

      KRYS_NODISCARD bool operator==(const PositionArea &) const noexcept = default;

    private:
      Maybe<PositionAreaValue> _value;
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<PositionArea>
    {
      KRYS_NODISCARD PositionArea operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<PositionAreaValue>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const PositionAreaValue &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<PositionAreaValue>
    {
      void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                      const PositionAreaValue &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionArea)
