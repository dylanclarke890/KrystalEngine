#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRangeName.hpp"

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  SingleAnimationRangeName ConvertValueIdToSingleAnimationRangeName(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Normal:        return SingleAnimationRangeName::Normal;
      case css::ValueId::Cover:         return SingleAnimationRangeName::Cover;
      case css::ValueId::Contain:       return SingleAnimationRangeName::Contain;
      case css::ValueId::Entry:         return SingleAnimationRangeName::Entry;
      case css::ValueId::Exit:          return SingleAnimationRangeName::Exit;
      case css::ValueId::EntryCrossing: return SingleAnimationRangeName::EntryCrossing;
      case css::ValueId::ExitCrossing:  return SingleAnimationRangeName::ExitCrossing;
      default:                          krys_debug_assert_msg(false, "Unreachable"); return SingleAnimationRangeName::Normal;
    }
  }

  css::ValueId ConvertSingleAnimationRangeNameToValueId(SingleAnimationRangeName range) noexcept
  {
    switch (range)
    {
      case SingleAnimationRangeName::Normal:        return css::ValueId::Normal;
      case SingleAnimationRangeName::Cover:         return css::ValueId::Cover;
      case SingleAnimationRangeName::Contain:       return css::ValueId::Contain;
      case SingleAnimationRangeName::Entry:         return css::ValueId::Entry;
      case SingleAnimationRangeName::Exit:          return css::ValueId::Exit;
      case SingleAnimationRangeName::EntryCrossing: return css::ValueId::EntryCrossing;
      case SingleAnimationRangeName::ExitCrossing:  return css::ValueId::ExitCrossing;
      case SingleAnimationRangeName::Omitted:       return css::ValueId::Invalid;
    }
    krys_debug_assert_msg(false, "Unreachable");
    return css::ValueId::Normal;
  }

  css::CSSOMString ConvertSingleAnimationRangeNameToRangeString(SingleAnimationRangeName rangeName) noexcept
  {
    switch (rangeName)
    {
      case SingleAnimationRangeName::Normal:        return u8"normal";
      case SingleAnimationRangeName::Omitted:       return u8"omitted";
      case SingleAnimationRangeName::Cover:         return u8"cover";
      case SingleAnimationRangeName::Contain:       return u8"contain";
      case SingleAnimationRangeName::Entry:         return u8"entry";
      case SingleAnimationRangeName::Exit:          return u8"exit";
      case SingleAnimationRangeName::EntryCrossing: return u8"entry-crossing";
      case SingleAnimationRangeName::ExitCrossing:  return u8"exit-crossing";
    }
    krys_debug_assert_msg(false, "Unreachable");
    return u8"normal";
  }

  SingleAnimationRangeName
    ConvertRangeStringToSingleTimelineRangeName(const css::CSSOMString &rangeString) noexcept
  {
    if (rangeString == u8"cover")
    {
      return style::SingleAnimationRangeName::Cover;
    }
    if (rangeString == u8"contain")
    {
      return style::SingleAnimationRangeName::Contain;
    }
    if (rangeString == u8"entry")
    {
      return style::SingleAnimationRangeName::Entry;
    }
    if (rangeString == u8"exit")
    {
      return style::SingleAnimationRangeName::Exit;
    }
    if (rangeString == u8"entry-crossing")
    {
      return style::SingleAnimationRangeName::EntryCrossing;
    }
    if (rangeString == u8"exit-crossing")
    {
      return style::SingleAnimationRangeName::ExitCrossing;
    }
    return style::SingleAnimationRangeName::Normal;
  }
}
