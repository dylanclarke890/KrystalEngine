#include "Krystal.Booey/CSS/Properties/Consumers/Align.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  namespace
  {
    KRYS_NODISCARD RefPtr<Value> ConsumeAlignmentBaseline(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == ValueId::Baseline);

      // FIXME: The spec states that <baseline-position> is defined as `<baseline-position> = [ first | last
      // ]? && baseline`, allowing any ordering, but tests expect `[ first | last ]` to always be precede
      // `baseline`.

      tokens.Discard();
      tokens.DiscardWhitespace();

      return PrimitiveValue::Create(ValueId::Baseline);
    }

    KRYS_NODISCARD RefPtr<Value> ConsumeAlignmentFirstBaseline(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == ValueId::First);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (copy.Peek().ValueId() != ValueId::Baseline)
      {
        return nullptr;
      }

      tokens = copy;
      tokens.Discard();
      tokens.DiscardWhitespace();

      return PrimitiveValue::Create(ValueId::Baseline);
    }

    KRYS_NODISCARD RefPtr<Value> ConsumeAlignmentLastBaseline(TokenRange &tokens,
                                                                 PropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == ValueId::Last);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (copy.Peek().ValueId() != ValueId::Baseline)
      {
        return nullptr;
      }

      tokens = copy;
      tokens.Discard();
      tokens.DiscardWhitespace();

      return ValuePair::Create(PrimitiveValue::Create(ValueId::Last),
                                  PrimitiveValue::Create(ValueId::Baseline));
    }

    template <typename F>
    KRYS_NODISCARD RefPtr<Value>
      ConsumeAlignmentOverflowPosition(TokenRange &tokens, PropertyParserState &,
                                       ValueId overflowSafety, F &&predicate) noexcept
    {
      assert(tokens.Peek().ValueId() == ValueId::Safe || tokens.Peek().ValueId() == ValueId::Unsafe);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (auto position = copy.Peek().ValueId(); predicate(position))
      {
        tokens = copy;
        tokens.Discard();
        tokens.DiscardWhitespace();

        return ValuePair::Create(PrimitiveValue::Create(overflowSafety),
                                    PrimitiveValue::Create(position));
      }

      return nullptr;
    }
  }

  RefPtr<Value> ConsumeAlignContent(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'align-content'> = normal | <baseline-position> | <content-distribution> | <overflow-position>?
    // <content-position>
    // https://drafts.csswg.org/css-align/#propdef-align-content

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case ValueId::Normal:
      // <content-distribution>
      case ValueId::SpaceBetween:
      case ValueId::SpaceAround:
      case ValueId::SpaceEvenly:
      case ValueId::Stretch:
      // <content-position>
      case ValueId::Start:
      case ValueId::End:
      case ValueId::Center:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case ValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case ValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case ValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <content-position>
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(tokens, state, initial,
                                                [](auto second)
                                                {
                                                  switch (second)
                                                  {
                                                    case ValueId::Start:
                                                    case ValueId::End:
                                                    case ValueId::Center:
                                                    case ValueId::FlexStart:
                                                    case ValueId::FlexEnd:
                                                    {
                                                      return true;
                                                    }
                                                    default:
                                                    {
                                                      return false;
                                                    }
                                                  }
                                                });
      }
      default:
      {
        return nullptr;
      }
    }
  }

  RefPtr<Value> ConsumeJustifyContent(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'justify-content'> = normal | <content-distribution> | <overflow-position>? [ <content-position> |
    // left | right ]
    // https://drafts.csswg.org/css-align/#propdef-justify-content

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case ValueId::Normal:
      // <content-distribution>
      case ValueId::SpaceBetween:
      case ValueId::SpaceAround:
      case ValueId::SpaceEvenly:
      case ValueId::Stretch:
      // [ <content-position> | left | right ]
      case ValueId::Start:
      case ValueId::End:
      case ValueId::Center:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      case ValueId::Left:
      case ValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <overflow-position>? [ <content-position> | left | right ]
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(tokens, state, initial,
                                                [](auto second)
                                                {
                                                  switch (second)
                                                  {
                                                    case ValueId::Start:
                                                    case ValueId::End:
                                                    case ValueId::Center:
                                                    case ValueId::FlexStart:
                                                    case ValueId::FlexEnd:
                                                    case ValueId::Left:
                                                    case ValueId::Right:
                                                    {
                                                      return true;
                                                    }
                                                    default:
                                                    {
                                                      return false;
                                                    }
                                                  }
                                                });
      }
      default:
      {
        return nullptr;
      }
    }
  }

  RefPtr<Value> ConsumeAlignSelf(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'align-self'> = auto | normal | stretch | <baseline-position> | <overflow-position>? <self-position>
    // https://drafts.csswg.org/css-align/#propdef-align-self

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // auto
      case ValueId::Auto:
      // normal
      case ValueId::Normal:
      // stretch
      case ValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <self-position>
      case ValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case ValueId::Start:
      case ValueId::End:
      case ValueId::Center:
      case ValueId::SelfStart:
      case ValueId::SelfEnd:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case ValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case ValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case ValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <self-position>
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case ValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case ValueId::Start:
              case ValueId::End:
              case ValueId::Center:
              case ValueId::SelfStart:
              case ValueId::SelfEnd:
              case ValueId::FlexStart:
              case ValueId::FlexEnd:
              {
                return true;
              }
              default:
              {
                return false;
              }
            }
          });
      }
      default:
      {
        return nullptr;
      }
    }
  }

  RefPtr<Value> ConsumeJustifySelf(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'justify-self'> = auto | normal | stretch | <baseline-position> | <overflow-position>? [
    // <self-position> | left | right ]
    // https://drafts.csswg.org/css-align/#propdef-justify-self

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // auto
      case ValueId::Auto:
      // normal
      case ValueId::Normal:
      // stretch
      case ValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return PrimitiveValue::Create(initial);
      }
      // [ <self-position> | left | right ]
      case ValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case ValueId::Start:
      case ValueId::End:
      case ValueId::Center:
      case ValueId::SelfStart:
      case ValueId::SelfEnd:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      case ValueId::Left:
      case ValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case ValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case ValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case ValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? [ <self-position> | left | right ]
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case ValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }
                KRYS_FALLTHROUGH;
              }
              case ValueId::Start:
              case ValueId::End:
              case ValueId::Center:
              case ValueId::SelfStart:
              case ValueId::SelfEnd:
              case ValueId::FlexStart:
              case ValueId::FlexEnd:
              case ValueId::Left:
              case ValueId::Right:
              {
                return true;
              }
              default:
              {
                return false;
              }
            }
          });
      }
      default:
      {
        return nullptr;
      }
    }
  }

  RefPtr<Value> ConsumeAlignItems(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'align-items'> = normal | stretch | <baseline-position> | <overflow-position>? <self-position>
    // https://drafts.csswg.org/css-align/#propdef-align-items

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case ValueId::Normal:
      // stretch
      case ValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <self-position>
      case ValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case ValueId::Start:
      case ValueId::End:
      case ValueId::Center:
      case ValueId::SelfStart:
      case ValueId::SelfEnd:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case ValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case ValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case ValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <self-position>
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case ValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case ValueId::Start:
              case ValueId::End:
              case ValueId::Center:
              case ValueId::SelfStart:
              case ValueId::SelfEnd:
              case ValueId::FlexStart:
              case ValueId::FlexEnd:
              {
                return true;
              }
              default:
              {
                return false;
              }
            }
          });
      }

      default: return nullptr;
    }
  }

  RefPtr<Value> ConsumeJustifyItems(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'justify-items'> = normal | stretch | <baseline-position> | <overflow-position>? [ <self-position> |
    // left | right ] | legacy | legacy && [ left | right | center ]
    // https://drafts.csswg.org/css-align/#propdef-justify-items

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case ValueId::Normal:
      // stretch
      case ValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return PrimitiveValue::Create(initial);
      }
      // [ <self-position> | left | right ] - NOTE: `left`, `right`, and `center` handled further below to
      // account for additional `legacy` keyword.
      case ValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case ValueId::Start:
      case ValueId::End:
      case ValueId::SelfStart:
      case ValueId::SelfEnd:
      case ValueId::FlexStart:
      case ValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return PrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case ValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case ValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case ValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? [ <self-position> | left | right ]
      case ValueId::Unsafe:
      case ValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case ValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case ValueId::Start:
              case ValueId::End:
              case ValueId::Center:
              case ValueId::SelfStart:
              case ValueId::SelfEnd:
              case ValueId::FlexStart:
              case ValueId::FlexEnd:
              case ValueId::Left:
              case ValueId::Right:
              {
                return true;
              }
              default:
              {
                return false;
              }
            }
          });
      }

      // legacy | legacy && [ left | right | center ]
      case ValueId::Legacy:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        switch (auto second = tokens.Peek().ValueId(); second)
        {
          case ValueId::Left:
          case ValueId::Right:
          case ValueId::Center:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();

            return ValuePair::Create(PrimitiveValue::Create(initial),
                                        PrimitiveValue::Create(second));
          }
          default:
          {
            return PrimitiveValue::Create(initial);
          }
        }
      }
      case ValueId::Center:
      case ValueId::Left:
      case ValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        switch (auto second = tokens.Peek().ValueId(); second)
        {
          case ValueId::Legacy:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();

            // NOTE: Order is flipped to canonicalize to 'legacy *foo*' for serialization.
            return ValuePair::Create(PrimitiveValue::Create(second),
                                        PrimitiveValue::Create(initial));
          }
          default:
          {
            return PrimitiveValue::Create(initial);
          }
        }
      }
      default:
      {
        return nullptr;
      }
    }
  }
}