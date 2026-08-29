#include "Krystal.HTML/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPositionValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPosition.hpp"
#include "Krystal.Lib/Core/Visitor.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // MARK: <position>
  // https://drafts.csswg.org/css-values/#position

  // <position> = <position-one> | <position-two> | <position-four>

  // MARK: <bg-position>
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-background-position

  // background-position has special parsing rules, allowing a 3-value syntax:
  //
  // <bg-position> = <position-one> | <position-two> | <bg-position-three> | <position-four>

  // Sub-productions

  // <position-one> = [ left | center | right | top | bottom | x-start | x-end | y-start | y-end |
  // <length-percentage> ]
  //
  // <position-two> = [
  //   [ left | center | right | x-start | x-end ] &&
  //   [ top | center | bottom | y-start | y-end ]
  // |
  //   [ left | center | right | x-start | x-end | <length-percentage> ]
  //   [ top | center | bottom | y-start | y-end | <length-percentage> ]
  // ]
  //
  // <bg-position-three> = [
  //   [ [        left |  right | x-start | x-end ] <length-percentage> ] &&
  //   [ center |  top | bottom | y-start | y-end ]
  // |
  //   [ center | left |  right | x-start | x-end ] &&
  //   [ [         top | bottom | y-start | y-end ] <length-percentage> ]
  // ]
  //
  // <position-four> = [
  //   [ [ left | right | x-start | x-end ] <length-percentage> ] &&
  //   [ [ top | bottom | y-start | y-end ] <length-percentage> ]
  // ]

  // MARK: Unresolved CSSPosition

  using PositionUnresolvedComponent = Variant<
    // Horizontal
    Keywords::Left, Keywords::Right, Keywords::XStart, Keywords::XEnd,

    // Vertical
    Keywords::Top, Keywords::Bottom, Keywords::YStart, Keywords::YEnd,

    // Any Axis
    Keywords::Center, LengthPercentage<>>;

  // MARK: Predicate matching concepts

  template <typename T>
  concept IsHorizontalOnlyComponent = SameType<T, Keywords::Left> || SameType<T, Keywords::Right>
                                      || SameType<T, Keywords::XStart> || SameType<T, Keywords::XEnd>;

  template <typename T>
  concept IsHorizontalSecondComponent = IsHorizontalOnlyComponent<T> || SameType<T, Keywords::Center>;

  template <typename T>
  concept IsVerticalOnlyComponent = SameType<T, Keywords::Top> || SameType<T, Keywords::Bottom>
                                    || SameType<T, Keywords::YStart> || SameType<T, Keywords::YEnd>;

  template <typename T>
  concept IsVerticalSecondComponent =
    IsVerticalOnlyComponent<T> || SameType<T, Keywords::Center> || SameType<T, LengthPercentage<>>;

  KRYS_NODISCARD static Maybe<PositionUnresolvedComponent>
    ConsumePositionUnresolvedComponent(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::Left {}};
        }
        case CSSValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::Right {}};
        }
        case CSSValueId::XStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::XStart {}};
        }
        case CSSValueId::XEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::XEnd {}};
        }
        case CSSValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::Bottom {}};
        }
        case CSSValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::Top {}};
        }
        case CSSValueId::YStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::YStart {}};
        }
        case CSSValueId::YEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::YEnd {}};
        }
        case CSSValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {Keywords::Center {}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
      return PositionUnresolvedComponent {Krys::Move(*lengthPercentage)};
    return {};
  }

  KRYS_NODISCARD static Maybe<CSSPosition>
    PositionUnresolvedFromOneComponent(PositionUnresolvedComponent &&component) noexcept
  {
    // <position-one> = [ left | center | right | top | bottom | x-start | x-end | y-start | y-end |
    // <length-percentage> ]

    return Krys::SwitchOn(
      Krys::Move(component), []<IsHorizontalOnlyComponent C>(C &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{Krys::Move(component)}, {Keywords::Center {}}}; },
      []<IsVerticalOnlyComponent C>(C &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{Keywords::Center {}}, {Krys::Move(component)}}; },
      [](Keywords::Center &&) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{Keywords::Center {}}, {Keywords::Center {}}}; },
      [](LengthPercentage<> &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{Krys::Move(component)}, {Keywords::Center {}}}; });
  }

  KRYS_NODISCARD static Maybe<CSSPosition>
    PositionUnresolvedFromTwoComponents(PositionUnresolvedComponent &&component1,
                                        PositionUnresolvedComponent &&component2) noexcept
  {
    // <position-two> = [
    //   [ left | center | right | x-start | x-end ] &&
    //   [ top | center | bottom | y-start | y-end ]
    // |
    //   [ left | center | right | x-start | x-end | <length-percentage> ]
    //   [ top | center | bottom | y-start | y-end | <length-percentage> ]
    // ]

    return Krys::SwitchOn(
      Krys::Move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | center | bottom | y-start | y-end | <length-percentage> ]
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsVerticalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component1)},
                                                           {Krys::Move(component2)}};
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ left | center | right | x-start | x-end ] (NOTE:
        // <length-percentage> is NOT allowed).
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsHorizontalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component2)},
                                                           {Krys::Move(component1)}};
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](Keywords::Center &&component1) -> Maybe<CSSPosition>
      {
        // `component2` can be anything.
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component2)},
                                                           {Krys::Move(component1)}};
          },
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component1)},
                                                           {Krys::Move(component2)}};
          },
          [&](Keywords::Center &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component1)},
                                                           {Krys::Move(component2)}};
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component1)},
                                                           {Krys::Move(component2)}};
          });
      },
      [&](LengthPercentage<> &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | center | bottom | y-start | y-end | <length-percentage> ]
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsVerticalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{Krys::Move(component1)},
                                                           {Krys::Move(component2)}};
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      });
  }

  KRYS_NODISCARD static Maybe<CSSPosition>
    PositionUnresolvedFromThreeComponents(PositionUnresolvedComponent &&component1,
                                          PositionUnresolvedComponent &&component2,
                                          PositionUnresolvedComponent &&component3) noexcept
  {
    // Special case only for <bg-position> productions.

    // <bg-position-three> = [
    //   [ [        left |  right | x-start | x-end ] <length-percentage> ] &&
    //   [ center |  top | bottom | y-start | y-end ]
    // |
    //   [ center | left |  right | x-start | x-end ] &&
    //   [ [         top | bottom | y-start | y-end ] <length-percentage> ]
    // ]

    return Krys::SwitchOn(
      Krys::Move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | bottom | y-start | y-end | <length-percentage> ]
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be <length-percentage>
            if (!std::holds_alternative<LengthPercentage<>>(component3))
              return {};
            return ThreeComponentPositionHorizontalVerticalLengthSecond {
              {{Krys::Move(component1)}},
              {{Krys::Move(component2), std::get<LengthPercentage<>>(component3)}},
            };
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be in the set [ center | top | bottom | y-start | y-end ]
            return Krys::SwitchOn(
              Krys::Move(component3),
              [&]<IsVerticalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthFirst {
                  {{Krys::Move(component1), Krys::Move(component2)}},
                  {{Krys::Move(component3)}},
                };
              },
              [&](Keywords::Center &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthFirst {
                  {{Krys::Move(component1), Krys::Move(component2)}},
                  {{Krys::Move(component3)}},
                };
              },
              [](auto &&) -> Maybe<CSSPosition> { return {}; });
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ left | right | x-start | x-end | <length-percentage> ]
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be <length-percentage>
            if (!std::holds_alternative<LengthPercentage<>>(component3))
              return {};
            return ThreeComponentPositionHorizontalVerticalLengthFirst {
              {{Krys::Move(component2), std::get<LengthPercentage<>>(component3)}},
              {{Krys::Move(component1)}},
            };
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be in the set [ center | left | right | x-start | x-end ]
            return Krys::SwitchOn(
              Krys::Move(component3),
              [&]<IsHorizontalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthSecond {
                  {{Krys::Move(component3)}},
                  {{Krys::Move(component1), Krys::Move(component2)}},
                };
              },
              [&](Keywords::Center &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthSecond {
                  {{Krys::Move(component3)}},
                  {{Krys::Move(component1), Krys::Move(component2)}},
                };
              },
              [](auto &&) -> Maybe<CSSPosition> { return {}; });
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](Keywords::Center &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be <length-percentage>
        if (!std::holds_alternative<LengthPercentage<>>(component3))
        {
          return {};
        }

        // `component2` must be in the set [ left | right | x-start | x-end | top | bottom | y-start | y-end ]
        return Krys::SwitchOn(
          Krys::Move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return ThreeComponentPositionHorizontalVerticalLengthFirst {
              {{Krys::Move(component2), std::get<LengthPercentage<>>(component3)}},
              {{Krys::Move(component1)}},
            };
          },
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return ThreeComponentPositionHorizontalVerticalLengthSecond {
              {{Krys::Move(component1)}},
              {{Krys::Move(component2), std::get<LengthPercentage<>>(component3)}},
            };
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](LengthPercentage<> &&) -> Maybe<CSSPosition>
      {
        // `<length-percentage>` is invalid for the first component of three component position values.
        return {};
      });
  }

  KRYS_NODISCARD static Maybe<CSSPosition> PositionUnresolvedFromFourComponents(
    PositionUnresolvedComponent &&component1, PositionUnresolvedComponent &&component2,
    PositionUnresolvedComponent &&component3, PositionUnresolvedComponent &&component4) noexcept
  {
    // <position-four> = [
    //   [ [ left | right | x-start | x-end ] <length-percentage> ] &&
    //   [ [ top | bottom | y-start | y-end ] <length-percentage> ]
    // ]

    // `component2` and `component4` must be <length-percentage>
    if (!std::holds_alternative<LengthPercentage<>>(component2)
        || !std::holds_alternative<LengthPercentage<>>(component4))
      return {};

    return Krys::SwitchOn(
      Krys::Move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be in the set [ top | bottom | y-start | y-end ]
        return Krys::SwitchOn(
          Krys::Move(component3),
          [&]<IsVerticalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
          {
            return FourComponentPositionHorizontalVertical {
              {{Krys::Move(component1), std::get<LengthPercentage<>>(component2)}},
              {{Krys::Move(component3), std::get<LengthPercentage<>>(component4)}},
            };
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be in the set [ left | right | x-start | x-end ]
        return Krys::SwitchOn(
          Krys::Move(component3),
          [&]<IsHorizontalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
          {
            return FourComponentPositionHorizontalVertical {
              {{Krys::Move(component3), std::get<LengthPercentage<>>(component4)}},
              {{Krys::Move(component1), std::get<LengthPercentage<>>(component2)}},
            };
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](Keywords::Center &&) -> Maybe<CSSPosition>
      {
        // `center` is invalid for the first component of four component position values.
        return {};
      },
      [&](LengthPercentage<> &&) -> Maybe<CSSPosition>
      {
        // `<length-percentage>` is invalid for the first component of four component position values.
        return {};
      });
  }

  Maybe<CSSPosition> ConsumePositionUnresolved(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto rangeCopy = tokens;

    auto component1 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component1)
    {
      return {};
    }

    auto component2 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component2)
    {
      auto position = PositionUnresolvedFromOneComponent(Krys::Move(*component1));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto component3 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component3)
    {
      auto position = PositionUnresolvedFromTwoComponents(Krys::Move(*component1), Krys::Move(*component2));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto component4 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component4)
      return {};

    auto position = PositionUnresolvedFromFourComponents(Krys::Move(*component1), Krys::Move(*component2),
                                                         Krys::Move(*component3), Krys::Move(*component4));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<CSSPosition> ConsumeBackgroundPositionUnresolved(CSSTokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept
  {
    auto rangeCopy = tokens;

    auto component1 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component1)
    {
      return {};
    }

    auto component2 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component2)
    {
      auto position = PositionUnresolvedFromOneComponent(Krys::Move(*component1));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto component3 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component3)
    {
      auto position = PositionUnresolvedFromTwoComponents(Krys::Move(*component1), Krys::Move(*component2));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto component4 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component4)
    {
      auto position = PositionUnresolvedFromThreeComponents(Krys::Move(*component1), Krys::Move(*component2),
                                                            Krys::Move(*component3));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto position = PositionUnresolvedFromFourComponents(Krys::Move(*component1), Krys::Move(*component2),
                                                         Krys::Move(*component3), Krys::Move(*component4));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<CSSPositionX> ConsumePositionXUnresolved(CSSTokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{Keywords::Left {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {Keywords::Left {}}};
        }
        case CSSValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{Keywords::Right {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {Keywords::Right {}}};
        }
        case CSSValueId::XStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{Keywords::XStart {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {Keywords::XStart {}}};
        }
        case CSSValueId::XEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{Keywords::XEnd {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {Keywords::XEnd {}}};
        }
        case CSSValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return CSSPositionX {TwoComponentPositionHorizontal {Keywords::Center {}}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return CSSPositionX {TwoComponentPositionHorizontal {Krys::Move(*lengthPercentage)}};
    }

    return {};
  }

  Maybe<CSSPositionY> ConsumePositionYUnresolved(CSSTokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{Keywords::Top {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {Keywords::Top {}}};
        }
        case CSSValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{Keywords::Bottom {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {Keywords::Bottom {}}};
        }
        case CSSValueId::YStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{Keywords::YStart {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {Keywords::YStart {}}};
        }
        case CSSValueId::YEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{Keywords::YEnd {}, Krys::Move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {Keywords::YEnd {}}};
        }
        case CSSValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return CSSPositionY {TwoComponentPositionVertical {Keywords::Center {}}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return CSSPositionY {TwoComponentPositionVertical {Krys::Move(*lengthPercentage)}};
    }

    return {};
  }

  Maybe<CSSPosition> ConsumeOneOrTwoComponentPositionUnresolved(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept
  {
    auto rangeCopy = tokens;

    auto component1 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component1)
    {
      return {};
    }

    auto component2 = ConsumePositionUnresolvedComponent(rangeCopy, state);
    if (!component2)
    {
      auto position = PositionUnresolvedFromOneComponent(Krys::Move(*component1));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto position = PositionUnresolvedFromTwoComponents(Krys::Move(*component1), Krys::Move(*component2));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<TwoComponentPositionHorizontal>
    consumeTwoComponentPositionHorizontalUnresolved(CSSTokenRange &tokens, CSSPropertyParserState &state)
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {Keywords::Left {}};
        }
        case CSSValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {Keywords::Right {}};
        }
        case CSSValueId::XStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {Keywords::XStart {}};
        }
        case CSSValueId::XEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {Keywords::XEnd {}};
        }
        case CSSValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {Keywords::Center {}};
        }
        default: return {};
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return TwoComponentPositionHorizontal {Krys::Move(*lengthPercentage)};
    }

    return {};
  }

  Maybe<TwoComponentPositionVertical>
    consumeTwoComponentPositionVerticalUnresolved(CSSTokenRange &tokens, CSSPropertyParserState &state)
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {Keywords::Bottom {}};
        }
        case CSSValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {Keywords::Top {}};
        }
        case CSSValueId::YStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {Keywords::YStart {}};
        }
        case CSSValueId::YEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {Keywords::YEnd {}};
        }
        case CSSValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {Keywords::Center {}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return TwoComponentPositionVertical {Krys::Move(*lengthPercentage)};
    }

    return {};
  }

  // MARK: CSSValue

  RefPtr<CSSValue> ConsumePosition(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto position = ConsumePositionUnresolved(tokens, state))
    {
      return CSSPositionValue::Create(Krys::Move(*position));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumePositionX(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto positionX = ConsumePositionXUnresolved(tokens, state))
    {
      return CSSPositionXValue::Create(Krys::Move(*positionX));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumePositionY(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto positionY = ConsumePositionYUnresolved(tokens, state))
    {
      return CSSPositionYValue::Create(Krys::Move(*positionY));
    }

    return nullptr;
  }
}