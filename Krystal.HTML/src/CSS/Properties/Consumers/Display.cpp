#include "Krystal.HTML/CSS/Properties/Consumers/Display.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // Keep in sync with the single keyword value fast path of CSSParserFastPaths's parseDisplay.
  RefPtr<CSSValue> ConsumeDisplay(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'display'>        = [ <display-outside> || <display-inside> ] | <display-listitem> |
    // <display-internal> | <display-box> | <display-legacy> <display-outside>  = block | inline | run-in
    // <display-inside>   = flow | flow-root | table | flex | grid | grid-lanes | ruby
    // <display-listitem> = <display-outside>? && [ flow | flow-root ]? && list-item
    // <display-internal> = table-row-group | table-header-group |
    //                      table-footer-group | table-row | table-cell |
    //                      table-column-group | table-column | table-caption |
    //                      ruby-base | ruby-text | ruby-base-container |
    //                      ruby-text-container
    // <display-box>      = contents | none
    // <display-legacy>   = inline-block | inline-table | inline-flex | inline-grid | inline-grid-lanes
    // https://drafts.csswg.org/css-display/#propdef-display
    // FIXME: The grid-lanes keyword is a temporary placeholder for now, so that we can run WPT tests.

    // Parse single keyword values
    auto singleKeyword = [&]()
    {
      if (state.Context.gridLanesEnabled && range.Peek().ValueId() == CSSValueId::InlineGridLanes)
      {
        return ConsumeIdent(range);
      }

      return ConsumeIdent<
        // <display-box>
        CSSValueId::Contents, CSSValueId::None,
        // <display-internal>
        CSSValueId::TableCaption, CSSValueId::TableCell, CSSValueId::TableColumnGroup,
        CSSValueId::TableColumn, CSSValueId::TableHeaderGroup, CSSValueId::TableFooterGroup,
        CSSValueId::TableRow, CSSValueId::TableRowGroup, CSSValueId::RubyBase, CSSValueId::RubyText,
        // <display-legacy>
        CSSValueId::InlineBlock, CSSValueId::InlineFlex, CSSValueId::InlineGrid, CSSValueId::InlineTable,
        // Prefixed values
        CSSValueId::WebkitInlineBox, CSSValueId::WebkitBox,
        // No layout support for the full <display-listitem> syntax, so treat it as <display-legacy>
        CSSValueId::ListItem>(range);
    }();

    auto AllowsValue = [&](CSSValueId value)
    {
      bool isRuby = value == CSSValueId::RubyBase || value == CSSValueId::RubyText
                    || value == CSSValueId::BlockRuby || value == CSSValueId::Ruby;
      return !isRuby || IsUASheetBehavior(state.Context.Mode);
    };

    if (singleKeyword)
    {
      if (!AllowsValue(singleKeyword->ValueId()))
      {
        return nullptr;
      }

      return singleKeyword;
    }

    // Empty value, stop parsing
    if (range.IsAtEnd())
    {
      return nullptr;
    }

    // Convert -webkit-flex/-webkit-inline-flex to flex/inline-flex
    CSSValueId nextValueId = range.Peek().ValueId();
    if (nextValueId == CSSValueId::WebkitInlineFlex || nextValueId == CSSValueId::WebkitFlex)
    {
      DiscardIdent(range);
      return CSSPrimitiveValue::Create(nextValueId == CSSValueId::WebkitInlineFlex ? CSSValueId::InlineFlex
                                                                                   : CSSValueId::Flex);
    }

    // Parse [ <display-outside> || <display-inside> ]
    Maybe<CSSValueId> parsedDisplayOutside;
    Maybe<CSSValueId> parsedDisplayInside;
    while (!range.IsAtEnd())
    {
      auto nextValueId = range.Peek().ValueId();
      switch (nextValueId)
      {
        // <display-outside>
        case CSSValueId::Block:
        case CSSValueId::Inline:
        {
          if (parsedDisplayOutside)
          {
            return nullptr;
          }

          parsedDisplayOutside = nextValueId;
          break;
        }
        // <display-inside>
        case CSSValueId::GridLanes:
        {
          if (!state.Context.gridLanesEnabled)
          {
            return nullptr;
          }

          KRYS_FALLTHROUGH;
        }
        case CSSValueId::Flex:
        case CSSValueId::Flow:
        case CSSValueId::FlowRoot:
        case CSSValueId::Grid:
        case CSSValueId::Table:
        case CSSValueId::Ruby:
        {
          if (parsedDisplayInside)
          {
            return nullptr;
          }

          parsedDisplayInside = nextValueId;
          break;
        }
        default:
        {
          return nullptr;
        }
      }

      DiscardIdent(range);
    }

    // Set defaults when one of the two values are unspecified
    CSSValueId displayInside = parsedDisplayInside.value_or(CSSValueId::Flow);

    auto SelectShortValue = [&]() -> CSSValueId
    {
      if (!parsedDisplayOutside || *parsedDisplayOutside == CSSValueId::Inline)
      {
        if (displayInside == CSSValueId::Ruby)
        {
          return CSSValueId::Ruby;
        }
      }

      if (!parsedDisplayOutside || *parsedDisplayOutside == CSSValueId::Block)
      {
        // Alias display: flow to display: block
        if (displayInside == CSSValueId::Flow)
        {
          return CSSValueId::Block;
        }

        if (displayInside == CSSValueId::Ruby)
        {
          return CSSValueId::BlockRuby;
        }

        return displayInside;
      }

      // Convert `display: inline <display-inside>` to the equivalent short value
      switch (displayInside)
      {
        case CSSValueId::Flex:
        {
          return CSSValueId::InlineFlex;
        }
        case CSSValueId::Flow:
        {
          return CSSValueId::Inline;
        }
        case CSSValueId::FlowRoot:
        {
          return CSSValueId::InlineBlock;
        }
        case CSSValueId::Grid:
        {
          return CSSValueId::InlineGrid;
        }
        case CSSValueId::GridLanes:
        {
          return CSSValueId::InlineGridLanes;
        }
        case CSSValueId::Table:
        {
          return CSSValueId::InlineTable;
        }
        default:
        {
          assert(false);
          return CSSValueId::Inline;
        }
      }
    };

    auto shortValue = SelectShortValue();
    if (!AllowsValue(shortValue))
    {
      return nullptr;
    }

    return CSSPrimitiveValue::Create(shortValue);
  }
}