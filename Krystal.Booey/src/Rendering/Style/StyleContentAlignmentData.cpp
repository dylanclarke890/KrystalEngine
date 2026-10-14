#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"

namespace krys::boo
{
  bool StyleContentAlignmentData::IsStartward(Maybe<TextDirection> leftRightAxisDirection,
                                              bool isFlexReverse) const noexcept
  {
    switch (static_cast<ContentPosition>(_position))
    {
      case ContentPosition::Normal:
        switch (static_cast<ContentDistribution>(_distribution))
        {
          case ContentDistribution::Default:
          case ContentDistribution::Stretch:
          case ContentDistribution::SpaceBetween: return !isFlexReverse;
          default:                                return false;
        }
      case ContentPosition::Start:
      case ContentPosition::Baseline:     return true;
      case ContentPosition::End:
      case ContentPosition::LastBaseline:
      case ContentPosition::Center:       return false;
      case ContentPosition::FlexStart:    return !isFlexReverse;
      case ContentPosition::FlexEnd:      return isFlexReverse;
      case ContentPosition::Left:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::LTR;
        }
        return true;
      case ContentPosition::Right:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::RTL;
        }
        return true;
      default: krys_debug_assert("Invalid ContentPosition"); return true;
    }
  }
  bool StyleContentAlignmentData::IsEndward(Maybe<TextDirection> leftRightAxisDirection,
                                            bool isFlexReverse) const noexcept
  {
    switch (static_cast<ContentPosition>(_position))
    {
      case ContentPosition::Normal:
        switch (static_cast<ContentDistribution>(_distribution))
        {
          case ContentDistribution::Default:
          case ContentDistribution::Stretch:
          case ContentDistribution::SpaceBetween: return isFlexReverse;
          default:                                return false;
        }
      case ContentPosition::Start:
      case ContentPosition::Baseline:
      case ContentPosition::Center:       return false;
      case ContentPosition::End:
      case ContentPosition::LastBaseline: return true;
      case ContentPosition::FlexStart:    return isFlexReverse;
      case ContentPosition::FlexEnd:      return !isFlexReverse;
      case ContentPosition::Left:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::RTL;
        }
        return false;
      case ContentPosition::Right:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::LTR;
        }
        return false;
      default: krys_debug_assert("Invalid ContentPosition"); return false;
    }
  }

  bool StyleContentAlignmentData::IsCentered() const noexcept
  {
    return static_cast<ContentPosition>(_position) == ContentPosition::Center
           || static_cast<ContentDistribution>(_distribution) == ContentDistribution::SpaceAround
           || static_cast<ContentDistribution>(_distribution) == ContentDistribution::SpaceEvenly;
  }
}
