#include "Krystal.Booey/CSS/Properties/Consumers/Display.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // Keep in sync with the single keyword value fast path of CSSParserFastPaths's parseDisplay.
  RefPtr<CSSValue> ConsumeDisplay(TokenRange &range, PropertyParserState &state) noexcept
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
      if (state.Context.gridLanesEnabled && range.Peek().ValueId() == ValueId::InlineGridLanes)
      {
        return ConsumeIdent(range);
      }

      return ConsumeIdent<
        // <display-box>
        ValueId::Contents, ValueId::None,
        // <display-internal>
        ValueId::TableCaption, ValueId::TableCell, ValueId::TableColumnGroup,
        ValueId::TableColumn, ValueId::TableHeaderGroup, ValueId::TableFooterGroup,
        ValueId::TableRow, ValueId::TableRowGroup, ValueId::RubyBase, ValueId::RubyText,
        // <display-legacy>
        ValueId::InlineBlock, ValueId::InlineFlex, ValueId::InlineGrid, ValueId::InlineTable,
        // Prefixed values
        ValueId::WebkitInlineBox, ValueId::WebkitBox,
        // No layout support for the full <display-listitem> syntax, so treat it as <display-legacy>
        ValueId::ListItem>(range);
    }();

    auto AllowsValue = [&](ValueId value)
    {
      bool isRuby = value == ValueId::RubyBase || value == ValueId::RubyText
                    || value == ValueId::BlockRuby || value == ValueId::Ruby;
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
    ValueId nextValueId = range.Peek().ValueId();
    if (nextValueId == ValueId::WebkitInlineFlex || nextValueId == ValueId::WebkitFlex)
    {
      DiscardIdent(range);
      return CSSPrimitiveValue::Create(nextValueId == ValueId::WebkitInlineFlex ? ValueId::InlineFlex
                                                                                   : ValueId::Flex);
    }

    // Parse [ <display-outside> || <display-inside> ]
    Maybe<ValueId> parsedDisplayOutside;
    Maybe<ValueId> parsedDisplayInside;
    while (!range.IsAtEnd())
    {
      auto nextValueId = range.Peek().ValueId();
      switch (nextValueId)
      {
        // <display-outside>
        case ValueId::Block:
        case ValueId::Inline:
        {
          if (parsedDisplayOutside)
          {
            return nullptr;
          }

          parsedDisplayOutside = nextValueId;
          break;
        }
        // <display-inside>
        case ValueId::GridLanes:
        {
          if (!state.Context.gridLanesEnabled)
          {
            return nullptr;
          }

          KRYS_FALLTHROUGH;
        }
        case ValueId::Flex:
        case ValueId::Flow:
        case ValueId::FlowRoot:
        case ValueId::Grid:
        case ValueId::Table:
        case ValueId::Ruby:
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
    ValueId displayInside = parsedDisplayInside.value_or(ValueId::Flow);

    auto SelectShortValue = [&]() -> ValueId
    {
      if (!parsedDisplayOutside || *parsedDisplayOutside == ValueId::Inline)
      {
        if (displayInside == ValueId::Ruby)
        {
          return ValueId::Ruby;
        }
      }

      if (!parsedDisplayOutside || *parsedDisplayOutside == ValueId::Block)
      {
        // Alias display: flow to display: block
        if (displayInside == ValueId::Flow)
        {
          return ValueId::Block;
        }

        if (displayInside == ValueId::Ruby)
        {
          return ValueId::BlockRuby;
        }

        return displayInside;
      }

      // Convert `display: inline <display-inside>` to the equivalent short value
      switch (displayInside)
      {
        case ValueId::Flex:
        {
          return ValueId::InlineFlex;
        }
        case ValueId::Flow:
        {
          return ValueId::Inline;
        }
        case ValueId::FlowRoot:
        {
          return ValueId::InlineBlock;
        }
        case ValueId::Grid:
        {
          return ValueId::InlineGrid;
        }
        case ValueId::GridLanes:
        {
          return ValueId::InlineGridLanes;
        }
        case ValueId::Table:
        {
          return ValueId::InlineTable;
        }
        default:
        {
          assert(false);
          return ValueId::Inline;
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