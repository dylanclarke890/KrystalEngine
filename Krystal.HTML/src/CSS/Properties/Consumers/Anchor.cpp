#include "Krystal.HTML/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // <position-area> syntax, rewritten for expository purpose:
  //
  // <position-area> =
  //   [ <physical-area>
  //   | <logical-area> | <self-logical-area>
  //   | <ambiguous-logical-area> | <self-ambiguous-logical-area> ]
  //
  // // Keywords that doesn't depend on an axis. That is, the result is the same,
  // // no matter which axis is used.
  // <axisless-keyword> = [ center | span-all ]
  //
  // // Keywords that refer to a physical axis.
  // <physical-area> = [ <physical-area-x> || <physical-area-y> ]
  // <physical-area-x> =
  //   [ left | right | span-left | span-right
  //   | x-start | x-end | span-x-start | span-x-end
  //   | x-self-start | x-self-end | span-x-self-start | span-x-self-end ] | <axisless-keyword>
  // <physical-area-y> =
  //   [ top | bottom | span-top | span-bottom
  //   | y-start | y-end | span-y-start | span-y-end
  //   | y-self-start | y-self-end | span-y-self-start | span-y-self-end ] | <axisless-keyword>
  //
  // // Keywords that refer to an explicit logical (block/inline) axis. The axis is
  // // resolved using the containing block's writing mode.
  // <logical-area> = [ <logical-area-block> || <logical-area-inline> ]
  // <logical-area-block> =
  //   [ block-start | block-end | span-block-start | span-block-end ] | <axisless-keyword>
  // <logical-area-inline> =
  //   [ inline-start | inline-end | span-inline-start | span-inline-end ] | <axisless-keyword>
  //
  // // Keywords that refer to an explicit logical (block/inline) axis. The axis is
  // // resolved using the element's own writing mode.
  // <self-logical-area> = [ <self-logical-area-block> || <self-logical-area-inline> ]
  // <self-logical-area-block> =
  //   [ self-block-start | self-block-end | span-self-block-start | span-self-block-end ] |
  //   <axisless-keyword>
  // <self-logical-area-inline> =
  //   [ self-inline-start | self-inline-end | span-self-inline-start | span-self-inline-end ] |
  //   <axisless-keyword>
  //
  // // Similar to <logical-area>, but the axis is ambiguous (can be block or inline)
  // // and must be resolved.
  // <ambiguous-logical-area> = <ambiguous-logical-area-keyword>{1, 2}
  // <ambiguous-logical-area-keyword> = [ start | end | span-start | span-end ] | <axisless-keyword>
  //
  // // Similar to <self-logical-area> but the axis is ambiguous.
  // <self-ambiguous-logical-area> = <self-ambiguous-logical-area-keyword>{1, 2}
  // <self-ambiguous-logical-area-keyword> = [ self-start | self-end | span-self-start | span-self-end ] |
  // <axisless-keyword>

  namespace
  {
    enum class KeywordType : uint8
    {
      // <physical-area-x>
      PhysicalX,
      // <physical-area-y>
      PhysicalY,

      // <logical-area-block>
      LogicalBlock,
      // <self-logical-area-block>
      SelfLogicalBlock,

      // <logical-area-inline>
      LogicalInline,
      // <self-logical-area-inline>
      SelfLogicalInline,

      // <ambiguous-position-area>
      Ambiguous,
      // <self-ambiguous-position-area>
      SelfAmbiguous,

      // <axisless-keyword>
      Axisless
    };

    KRYS_NODISCARD Maybe<KeywordType> GetKeywordType(CSSValueId id) noexcept
    {
      switch (id)
      {
        case CSSValueId::Left:
        case CSSValueId::Right:
        case CSSValueId::SpanLeft:
        case CSSValueId::SpanRight:
        case CSSValueId::XStart:
        case CSSValueId::XEnd:
        case CSSValueId::SpanXStart:
        case CSSValueId::SpanXEnd:
        case CSSValueId::SelfXStart:
        case CSSValueId::SelfXEnd:
        case CSSValueId::SpanSelfXStart:
        case CSSValueId::SpanSelfXEnd:
        {
          return KeywordType::PhysicalX;
        }
        case CSSValueId::Top:
        case CSSValueId::Bottom:
        case CSSValueId::SpanTop:
        case CSSValueId::SpanBottom:
        case CSSValueId::YStart:
        case CSSValueId::YEnd:
        case CSSValueId::SpanYStart:
        case CSSValueId::SpanYEnd:
        case CSSValueId::SelfYStart:
        case CSSValueId::SelfYEnd:
        case CSSValueId::SpanSelfYStart:
        case CSSValueId::SpanSelfYEnd:
        {
          return KeywordType::PhysicalY;
        }
        case CSSValueId::BlockStart:
        case CSSValueId::BlockEnd:
        case CSSValueId::SpanBlockStart:
        case CSSValueId::SpanBlockEnd:
        {
          return KeywordType::LogicalBlock;
        }
        case CSSValueId::InlineStart:
        case CSSValueId::InlineEnd:
        case CSSValueId::SpanInlineStart:
        case CSSValueId::SpanInlineEnd:
        {
          return KeywordType::LogicalInline;
        }
        case CSSValueId::SelfBlockStart:
        case CSSValueId::SelfBlockEnd:
        case CSSValueId::SpanSelfBlockStart:
        case CSSValueId::SpanSelfBlockEnd:
        {
          return KeywordType::SelfLogicalBlock;
        }
        case CSSValueId::SelfInlineStart:
        case CSSValueId::SelfInlineEnd:
        case CSSValueId::SpanSelfInlineStart:
        case CSSValueId::SpanSelfInlineEnd:
        {
          return KeywordType::SelfLogicalInline;
        }
        case CSSValueId::Start:
        case CSSValueId::End:
        case CSSValueId::SpanStart:
        case CSSValueId::SpanEnd:
        {
          return KeywordType::Ambiguous;
        }
        case CSSValueId::SelfStart:
        case CSSValueId::SelfEnd:
        case CSSValueId::SpanSelfStart:
        case CSSValueId::SpanSelfEnd:
        {
          return KeywordType::SelfAmbiguous;
        }
        case CSSValueId::Center:
        case CSSValueId::SpanAll:
        {
          return KeywordType::Axisless;
        }
        default:
        {
          return {};
        }
      }
    }

    // Check if the two keyword types are compatible with each other. For example,
    // <physical-area-x> must go with <physical-area-y> or <axisless-keyword>
    KRYS_NODISCARD bool AreTypesCompatible(KeywordType dim1Type, KeywordType dim2Type) noexcept
    {
      switch (dim1Type)
      {
        case KeywordType::PhysicalX:
        {
          return (dim2Type == KeywordType::PhysicalY || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::PhysicalY:
        {
          return (dim2Type == KeywordType::PhysicalX || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::LogicalBlock:
        {
          return (dim2Type == KeywordType::LogicalInline || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::LogicalInline:
        {
          return (dim2Type == KeywordType::LogicalBlock || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::SelfLogicalBlock:
        {
          return (dim2Type == KeywordType::SelfLogicalInline || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::SelfLogicalInline:
        {
          return (dim2Type == KeywordType::SelfLogicalBlock || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::Ambiguous:
        {
          return (dim2Type == KeywordType::Ambiguous || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::SelfAmbiguous:
        {
          return (dim2Type == KeywordType::SelfAmbiguous || dim2Type == KeywordType::Axisless);
        }
        case KeywordType::Axisless:
        {
          return true;
        }
      }

      assert(false);
      return false;
    }

    // Check if a keyword type is explicit about its axis.
    KRYS_NODISCARD bool IsTypeAxisExplicit(KeywordType type) noexcept
    {
      switch (type)
      {
        case KeywordType::PhysicalX:
        case KeywordType::PhysicalY:
        case KeywordType::LogicalBlock:
        case KeywordType::SelfLogicalBlock:
        case KeywordType::LogicalInline:
        case KeywordType::SelfLogicalInline:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    // Check if a keyword type refers to the X or block axis.
    KRYS_NODISCARD bool IsTypeBlockOrXAxis(KeywordType type)
    {
      switch (type)
      {
        case KeywordType::PhysicalX:
        case KeywordType::LogicalBlock:
        case KeywordType::SelfLogicalBlock:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    // Check if a keyword type refers to the Y or inline axis.
    KRYS_NODISCARD bool IsTypeInlineOrYAxis(KeywordType type) noexcept
    {
      switch (type)
      {
        case KeywordType::PhysicalY:
        case KeywordType::LogicalInline:
        case KeywordType::SelfLogicalInline:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD CSSValueId MakeAmbiguous(CSSValueId dim) noexcept
    {
      switch (dim)
      {
        case CSSValueId::BlockStart:          return CSSValueId::Start;
        case CSSValueId::SpanBlockStart:      return CSSValueId::SpanStart;
        case CSSValueId::SelfBlockStart:      return CSSValueId::SelfStart;
        case CSSValueId::SpanSelfBlockStart:  return CSSValueId::SpanSelfStart;

        case CSSValueId::BlockEnd:            return CSSValueId::End;
        case CSSValueId::SpanBlockEnd:        return CSSValueId::SpanEnd;
        case CSSValueId::SelfBlockEnd:        return CSSValueId::SelfEnd;
        case CSSValueId::SpanSelfBlockEnd:    return CSSValueId::SpanSelfEnd;

        case CSSValueId::InlineStart:         return CSSValueId::Start;
        case CSSValueId::SpanInlineStart:     return CSSValueId::SpanStart;
        case CSSValueId::SelfInlineStart:     return CSSValueId::SelfStart;
        case CSSValueId::SpanSelfInlineStart: return CSSValueId::SpanSelfStart;

        case CSSValueId::InlineEnd:           return CSSValueId::End;
        case CSSValueId::SpanInlineEnd:       return CSSValueId::SpanEnd;
        case CSSValueId::SelfInlineEnd:       return CSSValueId::SelfEnd;
        case CSSValueId::SpanSelfInlineEnd:   return CSSValueId::SpanSelfEnd;

        case CSSValueId::Center:              return CSSValueId::Center;

        default:
        {
          assert(false);
          return dim;
        }
      }
    }
  }

  RefPtr<CSSValue> ValueForPositionArea(CSSValueId dim1, CSSValueId dim2, ValueType context) noexcept
  {
    auto maybeDim1Type = GetKeywordType(dim1);
    if (!maybeDim1Type)
    {
      return nullptr;
    }

    auto maybeDim2Type = GetKeywordType(dim2);
    if (!maybeDim2Type)
    {
      return nullptr;
    }

    auto dim1Type = *maybeDim1Type;
    auto dim2Type = *maybeDim2Type;
    if (!AreTypesCompatible(dim1Type, dim2Type))
    {
      return nullptr;
    }

    if (dim1 == CSSValueId::SpanAll && IsTypeAxisExplicit(dim2Type))
    {
      return CSSPrimitiveValue::Create(dim2);
    }

    if (IsTypeAxisExplicit(dim1Type) && dim2 == CSSValueId::SpanAll)
    {
      return CSSPrimitiveValue::Create(dim1);
    }

    // Ensure the X/block axis keyword goes first in the pair.
    if (IsTypeInlineOrYAxis(dim1Type) || IsTypeBlockOrXAxis(dim2Type))
    {
      std::swap(dim1, dim2);
      std::swap(dim1Type, dim2Type);
    }

    if (context == ValueType::Computed)
    {
      // If one keyword is on the block axis and the other keyword is on the inline axis,
      // strip the block-/inline- prefix on the keywords.
      // e.g "block-start inline-end" is equivalent to "start end".
      // See https://drafts.csswg.org/css-anchor-position-1/#position-area-computed
      if ((dim1Type == KeywordType::LogicalBlock
           && (dim2Type == KeywordType::LogicalInline || dim2Type == KeywordType::Axisless))
          || (dim1Type == KeywordType::SelfLogicalBlock
              && (dim2Type == KeywordType::SelfLogicalInline || dim2Type == KeywordType::Axisless))
          || (dim1Type == KeywordType::Axisless
              && (dim2Type == KeywordType::LogicalInline || dim2Type == KeywordType::SelfLogicalInline)))
      {
        dim1 = MakeAmbiguous(dim1);
        dim2 = MakeAmbiguous(dim2);
      }
    }

    return CSSValuePair::Create(CSSPrimitiveValue::Create(dim1), CSSPrimitiveValue::Create(dim2));
  }

  RefPtr<CSSValue> ConsumePositionArea(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <'position-area'> = none | <position-area>
    // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-area

    auto maybeDim1 = ConsumeIdentRaw(tokens);
    if (!maybeDim1)
    {
      return nullptr;
    }

    auto dim1 = *maybeDim1;
    if (dim1 == CSSValueId::None)
    {
      return CSSPrimitiveValue::Create(CSSValueId::None);
    }

    auto maybeDim2 = ConsumeIdentRaw(tokens);
    if (!maybeDim2)
    {
      if (!GetKeywordType(dim1))
      {
        return nullptr;
      }

      return CSSPrimitiveValue::Create(dim1);
    }

    auto dim2 = *maybeDim2;
    return ValueForPositionArea(dim1, dim2, ValueType::Specified);
  }
}