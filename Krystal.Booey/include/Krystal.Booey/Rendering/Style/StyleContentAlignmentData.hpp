#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"

namespace krys::boo
{
  class StyleContentAlignmentData
  {
  public:
    constexpr StyleContentAlignmentData() noexcept = default;

    // Style data for Content-Distribution properties: align-content, justify-content.
    // <content-distribution> || [ <overflow-position>? && <content-position> ]
    constexpr StyleContentAlignmentData(ContentPosition position,
                                        ContentDistribution distribution = ContentDistribution::Default,
                                        OverflowAlignment overflow = OverflowAlignment::Default) noexcept
        : _position(ToUnderlying(position)), _distribution(ToUnderlying(distribution)),
          _overflow(ToUnderlying(overflow))
    {
    }

    void SetPosition(ContentPosition position) noexcept
    {
      _position = ToUnderlying(position);
    }
    void SetDistribution(ContentDistribution distribution) noexcept
    {
      _distribution = ToUnderlying(distribution);
    }
    void SetOverflow(OverflowAlignment overflow) noexcept
    {
      _overflow = ToUnderlying(overflow);
    }

    KRYS_NODISCARD ContentPosition Position() const noexcept
    {
      return static_cast<ContentPosition>(_position);
    }
    KRYS_NODISCARD ContentDistribution Distribution() const noexcept
    {
      return static_cast<ContentDistribution>(_distribution);
    }
    KRYS_NODISCARD OverflowAlignment Overflow() const noexcept
    {
      return static_cast<OverflowAlignment>(_overflow);
    }
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return ContentPosition::Normal == static_cast<ContentPosition>(_position)
             && ContentDistribution::Default == static_cast<ContentDistribution>(_distribution);
    }
    KRYS_NODISCARD bool IsStartward(Maybe<TextDirection> leftRightAxisDirection = null,
                                    bool isFlexReverse = false) const noexcept;
    KRYS_NODISCARD bool IsEndward(Maybe<TextDirection> leftRightAxisDirection = null,
                                  bool isFlexReverse = false) const noexcept;
    // leftRightAxisDirection is only needed for justify-content (invalid for align-content).
    // Pass null if neither the inline axis nor the physical left-right axis matches the justify-content axis
    // (e.g. in flexbox).
    KRYS_NODISCARD bool IsCentered() const noexcept;

    KRYS_NODISCARD bool operator==(const StyleContentAlignmentData &) const noexcept = default;

  private:
    uint16 _position : 4 {0};     // ContentPosition
    uint16 _distribution : 3 {0}; // ContentDistribution
    uint16 _overflow : 2 {0};     // OverflowAlignment
  };
}
