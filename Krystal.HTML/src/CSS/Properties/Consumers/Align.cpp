#include "Krystal.HTML/CSS/Properties/Consumers/Align.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  namespace
  {
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignmentBaseline(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == CSSValueId::Baseline);

      // FIXME: The spec states that <baseline-position> is defined as `<baseline-position> = [ first | last
      // ]? && baseline`, allowing any ordering, but tests expect `[ first | last ]` to always be precede
      // `baseline`.

      tokens.Discard();
      tokens.DiscardWhitespace();

      return CSSPrimitiveValue::Create(CSSValueId::Baseline);
    }

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignmentFirstBaseline(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == CSSValueId::First);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (copy.Peek().ValueId() != CSSValueId::Baseline)
      {
        return nullptr;
      }

      tokens = copy;
      tokens.Discard();
      tokens.DiscardWhitespace();

      return CSSPrimitiveValue::Create(CSSValueId::Baseline);
    }

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeAlignmentLastBaseline(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state) noexcept
    {
      assert(tokens.Peek().ValueId() == CSSValueId::Last);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (copy.Peek().ValueId() != CSSValueId::Baseline)
      {
        return nullptr;
      }

      tokens = copy;
      tokens.Discard();
      tokens.DiscardWhitespace();

      return CSSValuePair::Create(CSSPrimitiveValue::Create(CSSValueId::Last),
                                  CSSPrimitiveValue::Create(CSSValueId::Baseline));
    }

    template <typename F>
    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeAlignmentOverflowPosition(CSSTokenRange &tokens, CSSPropertyParserState &,
                                       CSSValueId overflowSafety, F &&predicate) noexcept
    {
      assert(tokens.Peek().ValueId() == CSSValueId::Safe || tokens.Peek().ValueId() == CSSValueId::Unsafe);

      auto copy = tokens;
      copy.Discard();
      copy.DiscardWhitespace();

      if (auto position = copy.Peek().ValueId(); predicate(position))
      {
        tokens = copy;
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSValuePair::Create(CSSPrimitiveValue::Create(overflowSafety),
                                    CSSPrimitiveValue::Create(position));
      }

      return nullptr;
    }
  }

  RefPtr<CSSValue> ConsumeAlignContent(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'align-content'> = normal | <baseline-position> | <content-distribution> | <overflow-position>?
    // <content-position>
    // https://drafts.csswg.org/css-align/#propdef-align-content

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case CSSValueId::Normal:
      // <content-distribution>
      case CSSValueId::SpaceBetween:
      case CSSValueId::SpaceAround:
      case CSSValueId::SpaceEvenly:
      case CSSValueId::Stretch:
      // <content-position>
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::Center:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case CSSValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case CSSValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case CSSValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <content-position>
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(tokens, state, initial,
                                                [](auto second)
                                                {
                                                  switch (second)
                                                  {
                                                    case CSSValueId::Start:
                                                    case CSSValueId::End:
                                                    case CSSValueId::Center:
                                                    case CSSValueId::FlexStart:
                                                    case CSSValueId::FlexEnd:
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

  RefPtr<CSSValue> ConsumeJustifyContent(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'justify-content'> = normal | <content-distribution> | <overflow-position>? [ <content-position> |
    // left | right ]
    // https://drafts.csswg.org/css-align/#propdef-justify-content

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case CSSValueId::Normal:
      // <content-distribution>
      case CSSValueId::SpaceBetween:
      case CSSValueId::SpaceAround:
      case CSSValueId::SpaceEvenly:
      case CSSValueId::Stretch:
      // [ <content-position> | left | right ]
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::Center:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      case CSSValueId::Left:
      case CSSValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <overflow-position>? [ <content-position> | left | right ]
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(tokens, state, initial,
                                                [](auto second)
                                                {
                                                  switch (second)
                                                  {
                                                    case CSSValueId::Start:
                                                    case CSSValueId::End:
                                                    case CSSValueId::Center:
                                                    case CSSValueId::FlexStart:
                                                    case CSSValueId::FlexEnd:
                                                    case CSSValueId::Left:
                                                    case CSSValueId::Right:
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

  RefPtr<CSSValue> ConsumeAlignSelf(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'align-self'> = auto | normal | stretch | <baseline-position> | <overflow-position>? <self-position>
    // https://drafts.csswg.org/css-align/#propdef-align-self

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // auto
      case CSSValueId::Auto:
      // normal
      case CSSValueId::Normal:
      // stretch
      case CSSValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <self-position>
      case CSSValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::Center:
      case CSSValueId::SelfStart:
      case CSSValueId::SelfEnd:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case CSSValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case CSSValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case CSSValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <self-position>
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case CSSValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case CSSValueId::Start:
              case CSSValueId::End:
              case CSSValueId::Center:
              case CSSValueId::SelfStart:
              case CSSValueId::SelfEnd:
              case CSSValueId::FlexStart:
              case CSSValueId::FlexEnd:
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

  RefPtr<CSSValue> ConsumeJustifySelf(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'justify-self'> = auto | normal | stretch | <baseline-position> | <overflow-position>? [
    // <self-position> | left | right ]
    // https://drafts.csswg.org/css-align/#propdef-justify-self

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // auto
      case CSSValueId::Auto:
      // normal
      case CSSValueId::Normal:
      // stretch
      case CSSValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return CSSPrimitiveValue::Create(initial);
      }
      // [ <self-position> | left | right ]
      case CSSValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::Center:
      case CSSValueId::SelfStart:
      case CSSValueId::SelfEnd:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      case CSSValueId::Left:
      case CSSValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case CSSValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case CSSValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case CSSValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? [ <self-position> | left | right ]
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case CSSValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }
                KRYS_FALLTHROUGH;
              }
              case CSSValueId::Start:
              case CSSValueId::End:
              case CSSValueId::Center:
              case CSSValueId::SelfStart:
              case CSSValueId::SelfEnd:
              case CSSValueId::FlexStart:
              case CSSValueId::FlexEnd:
              case CSSValueId::Left:
              case CSSValueId::Right:
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

  RefPtr<CSSValue> ConsumeAlignItems(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'align-items'> = normal | stretch | <baseline-position> | <overflow-position>? <self-position>
    // https://drafts.csswg.org/css-align/#propdef-align-items

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case CSSValueId::Normal:
      // stretch
      case CSSValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <self-position>
      case CSSValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::Center:
      case CSSValueId::SelfStart:
      case CSSValueId::SelfEnd:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case CSSValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case CSSValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case CSSValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? <self-position>
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case CSSValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case CSSValueId::Start:
              case CSSValueId::End:
              case CSSValueId::Center:
              case CSSValueId::SelfStart:
              case CSSValueId::SelfEnd:
              case CSSValueId::FlexStart:
              case CSSValueId::FlexEnd:
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

  RefPtr<CSSValue> ConsumeJustifyItems(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'justify-items'> = normal | stretch | <baseline-position> | <overflow-position>? [ <self-position> |
    // left | right ] | legacy | legacy && [ left | right | center ]
    // https://drafts.csswg.org/css-align/#propdef-justify-items

    switch (auto initial = tokens.Peek().ValueId(); initial)
    {
      // normal
      case CSSValueId::Normal:
      // stretch
      case CSSValueId::Stretch:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(initial);
      }
      // [ <self-position> | left | right ] - NOTE: `left`, `right`, and `center` handled further below to
      // account for additional `legacy` keyword.
      case CSSValueId::AnchorCenter:
      {
        if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
        {
          return nullptr;
        }

        KRYS_FALLTHROUGH;
      }
      case CSSValueId::Start:
      case CSSValueId::End:
      case CSSValueId::SelfStart:
      case CSSValueId::SelfEnd:
      case CSSValueId::FlexStart:
      case CSSValueId::FlexEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return CSSPrimitiveValue::Create(initial);
      }
      // <baseline-position>
      case CSSValueId::First:
      {
        return ConsumeAlignmentFirstBaseline(tokens, state);
      }
      case CSSValueId::Last:
      {
        return ConsumeAlignmentLastBaseline(tokens, state);
      }
      case CSSValueId::Baseline:
      {
        return ConsumeAlignmentBaseline(tokens, state);
      }
      // <overflow-position>? [ <self-position> | left | right ]
      case CSSValueId::Unsafe:
      case CSSValueId::Safe:
      {
        return ConsumeAlignmentOverflowPosition(
          tokens, state, initial,
          [&](auto second)
          {
            switch (second)
            {
              case CSSValueId::AnchorCenter:
              {
                if (!state.Context.PropertySettings.cssAnchorPositioningEnabled)
                {
                  return false;
                }

                KRYS_FALLTHROUGH;
              }
              case CSSValueId::Start:
              case CSSValueId::End:
              case CSSValueId::Center:
              case CSSValueId::SelfStart:
              case CSSValueId::SelfEnd:
              case CSSValueId::FlexStart:
              case CSSValueId::FlexEnd:
              case CSSValueId::Left:
              case CSSValueId::Right:
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
      case CSSValueId::Legacy:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        switch (auto second = tokens.Peek().ValueId(); second)
        {
          case CSSValueId::Left:
          case CSSValueId::Right:
          case CSSValueId::Center:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();

            return CSSValuePair::Create(CSSPrimitiveValue::Create(initial),
                                        CSSPrimitiveValue::Create(second));
          }
          default:
          {
            return CSSPrimitiveValue::Create(initial);
          }
        }
      }
      case CSSValueId::Center:
      case CSSValueId::Left:
      case CSSValueId::Right:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        switch (auto second = tokens.Peek().ValueId(); second)
        {
          case CSSValueId::Legacy:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();

            // NOTE: Order is flipped to canonicalize to 'legacy *foo*' for serialization.
            return CSSValuePair::Create(CSSPrimitiveValue::Create(second),
                                        CSSPrimitiveValue::Create(initial));
          }
          default:
          {
            return CSSPrimitiveValue::Create(initial);
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