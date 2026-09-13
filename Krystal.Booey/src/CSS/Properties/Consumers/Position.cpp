#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPositionValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPosition.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css::PropertyParserHelpers
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
    keywords::Left, keywords::Right, keywords::XStart, keywords::XEnd,

    // Vertical
    keywords::Top, keywords::Bottom, keywords::YStart, keywords::YEnd,

    // Any Axis
    keywords::Center, LengthPercentage<>>;

  // MARK: Predicate matching concepts

  template <typename T>
  concept IsHorizontalOnlyComponent = SameType<T, keywords::Left> || SameType<T, keywords::Right>
                                      || SameType<T, keywords::XStart> || SameType<T, keywords::XEnd>;

  template <typename T>
  concept IsHorizontalSecondComponent = IsHorizontalOnlyComponent<T> || SameType<T, keywords::Center>;

  template <typename T>
  concept IsVerticalOnlyComponent = SameType<T, keywords::Top> || SameType<T, keywords::Bottom>
                                    || SameType<T, keywords::YStart> || SameType<T, keywords::YEnd>;

  template <typename T>
  concept IsVerticalSecondComponent =
    IsVerticalOnlyComponent<T> || SameType<T, keywords::Center> || SameType<T, LengthPercentage<>>;

  KRYS_NODISCARD static Maybe<PositionUnresolvedComponent>
    ConsumePositionUnresolvedComponent(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::Left {}};
        }
        case ValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::Right {}};
        }
        case ValueId::XStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::XStart {}};
        }
        case ValueId::XEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::XEnd {}};
        }
        case ValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::Bottom {}};
        }
        case ValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::Top {}};
        }
        case ValueId::YStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::YStart {}};
        }
        case ValueId::YEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::YEnd {}};
        }
        case ValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return PositionUnresolvedComponent {keywords::Center {}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
      return PositionUnresolvedComponent {krys::move(*lengthPercentage)};
    return {};
  }

  KRYS_NODISCARD static Maybe<CSSPosition>
    PositionUnresolvedFromOneComponent(PositionUnresolvedComponent &&component) noexcept
  {
    // <position-one> = [ left | center | right | top | bottom | x-start | x-end | y-start | y-end |
    // <length-percentage> ]

    return krys::SwitchOn(
      krys::move(component), []<IsHorizontalOnlyComponent C>(C &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{krys::move(component)}, {keywords::Center {}}}; },
      []<IsVerticalOnlyComponent C>(C &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{keywords::Center {}}, {krys::move(component)}}; },
      [](keywords::Center &&) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{keywords::Center {}}, {keywords::Center {}}}; },
      [](LengthPercentage<> &&component) -> Maybe<CSSPosition>
      { return TwoComponentPositionHorizontalVertical {{krys::move(component)}, {keywords::Center {}}}; });
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

    return krys::SwitchOn(
      krys::move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | center | bottom | y-start | y-end | <length-percentage> ]
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsVerticalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component1)},
                                                           {krys::move(component2)}};
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ left | center | right | x-start | x-end ] (NOTE:
        // <length-percentage> is NOT allowed).
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsHorizontalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component2)},
                                                           {krys::move(component1)}};
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](keywords::Center &&component1) -> Maybe<CSSPosition>
      {
        // `component2` can be anything.
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component2)},
                                                           {krys::move(component1)}};
          },
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component1)},
                                                           {krys::move(component2)}};
          },
          [&](keywords::Center &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component1)},
                                                           {krys::move(component2)}};
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component1)},
                                                           {krys::move(component2)}};
          });
      },
      [&](LengthPercentage<> &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | center | bottom | y-start | y-end | <length-percentage> ]
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsVerticalSecondComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return TwoComponentPositionHorizontalVertical {{krys::move(component1)},
                                                           {krys::move(component2)}};
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

    return krys::SwitchOn(
      krys::move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ top | bottom | y-start | y-end | <length-percentage> ]
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be <length-percentage>
            if (!std::holds_alternative<LengthPercentage<>>(component3))
              return {};
            return ThreeComponentPositionHorizontalVerticalLengthSecond {
              {{krys::move(component1)}},
              {{krys::move(component2), std::get<LengthPercentage<>>(component3)}},
            };
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be in the set [ center | top | bottom | y-start | y-end ]
            return krys::SwitchOn(
              krys::move(component3),
              [&]<IsVerticalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthFirst {
                  {{krys::move(component1), krys::move(component2)}},
                  {{krys::move(component3)}},
                };
              },
              [&](keywords::Center &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthFirst {
                  {{krys::move(component1), krys::move(component2)}},
                  {{krys::move(component3)}},
                };
              },
              [](auto &&) -> Maybe<CSSPosition> { return {}; });
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component2` must be in the set [ left | right | x-start | x-end | <length-percentage> ]
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be <length-percentage>
            if (!std::holds_alternative<LengthPercentage<>>(component3))
              return {};
            return ThreeComponentPositionHorizontalVerticalLengthFirst {
              {{krys::move(component2), std::get<LengthPercentage<>>(component3)}},
              {{krys::move(component1)}},
            };
          },
          [&](LengthPercentage<> &&component2) -> Maybe<CSSPosition>
          {
            // `component3` must be in the set [ center | left | right | x-start | x-end ]
            return krys::SwitchOn(
              krys::move(component3),
              [&]<IsHorizontalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthSecond {
                  {{krys::move(component3)}},
                  {{krys::move(component1), krys::move(component2)}},
                };
              },
              [&](keywords::Center &&component3) -> Maybe<CSSPosition>
              {
                return ThreeComponentPositionHorizontalVerticalLengthSecond {
                  {{krys::move(component3)}},
                  {{krys::move(component1), krys::move(component2)}},
                };
              },
              [](auto &&) -> Maybe<CSSPosition> { return {}; });
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](keywords::Center &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be <length-percentage>
        if (!std::holds_alternative<LengthPercentage<>>(component3))
        {
          return {};
        }

        // `component2` must be in the set [ left | right | x-start | x-end | top | bottom | y-start | y-end ]
        return krys::SwitchOn(
          krys::move(component2),
          [&]<IsHorizontalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return ThreeComponentPositionHorizontalVerticalLengthFirst {
              {{krys::move(component2), std::get<LengthPercentage<>>(component3)}},
              {{krys::move(component1)}},
            };
          },
          [&]<IsVerticalOnlyComponent C2>(C2 &&component2) -> Maybe<CSSPosition>
          {
            return ThreeComponentPositionHorizontalVerticalLengthSecond {
              {{krys::move(component1)}},
              {{krys::move(component2), std::get<LengthPercentage<>>(component3)}},
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

    return krys::SwitchOn(
      krys::move(component1),
      [&]<IsHorizontalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be in the set [ top | bottom | y-start | y-end ]
        return krys::SwitchOn(
          krys::move(component3),
          [&]<IsVerticalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
          {
            return FourComponentPositionHorizontalVertical {
              {{krys::move(component1), std::get<LengthPercentage<>>(component2)}},
              {{krys::move(component3), std::get<LengthPercentage<>>(component4)}},
            };
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&]<IsVerticalOnlyComponent C1>(C1 &&component1) -> Maybe<CSSPosition>
      {
        // `component3` must be in the set [ left | right | x-start | x-end ]
        return krys::SwitchOn(
          krys::move(component3),
          [&]<IsHorizontalOnlyComponent C3>(C3 &&component3) -> Maybe<CSSPosition>
          {
            return FourComponentPositionHorizontalVertical {
              {{krys::move(component3), std::get<LengthPercentage<>>(component4)}},
              {{krys::move(component1), std::get<LengthPercentage<>>(component2)}},
            };
          },
          [](auto &&) -> Maybe<CSSPosition> { return {}; });
      },
      [&](keywords::Center &&) -> Maybe<CSSPosition>
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

  Maybe<CSSPosition> ConsumePositionUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
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
      auto position = PositionUnresolvedFromOneComponent(krys::move(*component1));
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
      auto position = PositionUnresolvedFromTwoComponents(krys::move(*component1), krys::move(*component2));
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

    auto position = PositionUnresolvedFromFourComponents(krys::move(*component1), krys::move(*component2),
                                                         krys::move(*component3), krys::move(*component4));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<CSSPosition> ConsumeBackgroundPositionUnresolved(TokenRange &tokens,
                                                         PropertyParserState &state) noexcept
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
      auto position = PositionUnresolvedFromOneComponent(krys::move(*component1));
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
      auto position = PositionUnresolvedFromTwoComponents(krys::move(*component1), krys::move(*component2));
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
      auto position = PositionUnresolvedFromThreeComponents(krys::move(*component1), krys::move(*component2),
                                                            krys::move(*component3));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto position = PositionUnresolvedFromFourComponents(krys::move(*component1), krys::move(*component2),
                                                         krys::move(*component3), krys::move(*component4));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<CSSPositionX> ConsumePositionXUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{keywords::Left {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {keywords::Left {}}};
        }
        case ValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionX {
              FourComponentPositionHorizontal {{keywords::Right {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {keywords::Right {}}};
        }
        case ValueId::XStart:
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
              FourComponentPositionHorizontal {{keywords::XStart {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {keywords::XStart {}}};
        }
        case ValueId::XEnd:
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
              FourComponentPositionHorizontal {{keywords::XEnd {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionX {TwoComponentPositionHorizontal {keywords::XEnd {}}};
        }
        case ValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return CSSPositionX {TwoComponentPositionHorizontal {keywords::Center {}}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return CSSPositionX {TwoComponentPositionHorizontal {krys::move(*lengthPercentage)}};
    }

    return {};
  }

  Maybe<CSSPositionY> ConsumePositionYUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{keywords::Top {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {keywords::Top {}}};
        }
        case ValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
          {
            return CSSPositionY {
              FourComponentPositionVertical {{keywords::Bottom {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {keywords::Bottom {}}};
        }
        case ValueId::YStart:
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
              FourComponentPositionVertical {{keywords::YStart {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {keywords::YStart {}}};
        }
        case ValueId::YEnd:
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
              FourComponentPositionVertical {{keywords::YEnd {}, krys::move(*lengthPercentage)}}};
          }

          return CSSPositionY {TwoComponentPositionVertical {keywords::YEnd {}}};
        }
        case ValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return CSSPositionY {TwoComponentPositionVertical {keywords::Center {}}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return CSSPositionY {TwoComponentPositionVertical {krys::move(*lengthPercentage)}};
    }

    return {};
  }

  Maybe<CSSPosition> ConsumeOneOrTwoComponentPositionUnresolved(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept
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
      auto position = PositionUnresolvedFromOneComponent(krys::move(*component1));
      if (!position)
      {
        return {};
      }

      tokens = rangeCopy;
      return position;
    }

    auto position = PositionUnresolvedFromTwoComponents(krys::move(*component1), krys::move(*component2));
    if (!position)
    {
      return {};
    }

    tokens = rangeCopy;
    return position;
  }

  Maybe<TwoComponentPositionHorizontal>
    consumeTwoComponentPositionHorizontalUnresolved(TokenRange &tokens, PropertyParserState &state)
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Left:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {keywords::Left {}};
        }
        case ValueId::Right:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {keywords::Right {}};
        }
        case ValueId::XStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {keywords::XStart {}};
        }
        case ValueId::XEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {keywords::XEnd {}};
        }
        case ValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionHorizontal {keywords::Center {}};
        }
        default: return {};
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return TwoComponentPositionHorizontal {krys::move(*lengthPercentage)};
    }

    return {};
  }

  Maybe<TwoComponentPositionVertical>
    consumeTwoComponentPositionVerticalUnresolved(TokenRange &tokens, PropertyParserState &state)
  {
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      switch (tokens.Peek().ValueId())
      {
        case ValueId::Bottom:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {keywords::Bottom {}};
        }
        case ValueId::Top:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {keywords::Top {}};
        }
        case ValueId::YStart:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {keywords::YStart {}};
        }
        case ValueId::YEnd:
        {
          if (!state.Context.cssAxisRelativePositionKeywordsEnabled)
          {
            return {};
          }

          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {keywords::YEnd {}};
        }
        case ValueId::Center:
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          return TwoComponentPositionVertical {keywords::Center {}};
        }
        default:
        {
          return {};
        }
      }
    }

    if (auto lengthPercentage = MetaConsumer<LengthPercentage<>>::Consume(tokens, state))
    {
      return TwoComponentPositionVertical {krys::move(*lengthPercentage)};
    }

    return {};
  }

  // MARK: CSSValue

  RefPtr<CSSValue> ConsumePosition(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (auto position = ConsumePositionUnresolved(tokens, state))
    {
      return CSSPositionValue::Create(krys::move(*position));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumePositionX(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (auto positionX = ConsumePositionXUnresolved(tokens, state))
    {
      return CSSPositionXValue::Create(krys::move(*positionX));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumePositionY(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (auto positionY = ConsumePositionYUnresolved(tokens, state))
    {
      return CSSPositionYValue::Create(krys::move(*positionY));
    }

    return nullptr;
  }
}