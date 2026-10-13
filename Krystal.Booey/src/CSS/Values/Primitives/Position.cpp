#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"

namespace krys::boo::css
{
  bool IsCenterPosition(const Position &position) noexcept
  {
    auto isCenter = [](const auto &component)
    {
      return krys::SwitchOn(
        component.Offset, [](auto) { return false; }, [](keywords::Center) { return true; },
        [](const LengthPercentage<> &value)
        {
          return krys::SwitchOn(
            value, [](const LengthPercentage<>::Raw &raw) { return raw == 50_css_percentage; },
            [](const LengthPercentage<>::Calc &) { return false; });
        });
    };

    return position.SwitchOn([&](const TwoComponentPositionHorizontalVertical &components)
                             { return isCenter(get<0>(components)) && isCenter(get<1>(components)); },
                             [&](const auto &) { return false; });
  }

  namespace
  {
    TwoComponentPositionHorizontal ToTwoComponent(const ThreeComponentPositionHorizontal &component) noexcept
    {
      return krys::SwitchOn(component.Offset,
                            [](auto keyword) -> TwoComponentPositionHorizontal { return {keyword}; });
    }

    TwoComponentPositionVertical ToTwoComponent(const ThreeComponentPositionVertical &component) noexcept
    {
      return krys::SwitchOn(component.Offset,
                            [](auto keyword) -> TwoComponentPositionVertical { return {keyword}; });
    }
  }

  Pair<PositionX, PositionY> Split(Position &&position) noexcept
  {
    // PositionX and PositionY don't utilize the three component variants, so the non-length containing item
    // must be converted to its two component variant.

    return krys::SwitchOn(
      krys::move(position.Value), [&](const auto &components) -> Pair<PositionX, PositionY>
      { return {PositionX(get<0>(components)), PositionY(get<1>(components))}; },
      [&](const ThreeComponentPositionHorizontalVerticalLengthFirst &components) -> Pair<PositionX, PositionY>
      { return {PositionX(get<0>(components)), PositionY(ToTwoComponent(get<1>(components)))}; },
      [&](
        const ThreeComponentPositionHorizontalVerticalLengthSecond &components) -> Pair<PositionX, PositionY>
      { return {PositionX(ToTwoComponent(get<0>(components))), PositionY(get<1>(components))}; });
  }
}
