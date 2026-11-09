/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Properties/Consumers/Display.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // Keep in sync with the single keyword value fast path of CSSParserFastPaths's parseDisplay.
  RefPtr<Value> ConsumeDisplay(TokenRange &range, PropertyParserState &state) noexcept
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
      if (range.Peek().ValueId() == ValueId::InlineGridLanes)
      {
        return ConsumeIdent(range);
      }

      return ConsumeIdent<
        // <display-box>
        ValueId::Contents, ValueId::None,
        // <display-internal>
        ValueId::TableCaption, ValueId::TableCell, ValueId::TableColumnGroup, ValueId::TableColumn,
        ValueId::TableHeaderGroup, ValueId::TableFooterGroup, ValueId::TableRow, ValueId::TableRowGroup,
        ValueId::RubyBase, ValueId::RubyText,
        // <display-legacy>
        ValueId::InlineBlock, ValueId::InlineFlex, ValueId::InlineGrid, ValueId::InlineTable,
        // No layout support for the full <display-listitem> syntax, so treat it as <display-legacy>
        ValueId::ListItem>(range);
    }();

    auto AllowsValue = [&](ValueId value)
    {
      bool isRuby = value == ValueId::RubyBase || value == ValueId::RubyText || value == ValueId::BlockRuby
                    || value == ValueId::Ruby;
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

    ValueId nextValueId = range.Peek().ValueId();

    // Parse [ <display-outside> || <display-inside> ]
    Maybe<ValueId> parsedDisplayOutside;
    Maybe<ValueId> parsedDisplayInside;
    while (!range.IsAtEnd())
    {
      nextValueId = range.Peek().ValueId();
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
          krys_debug_assert(false);
          return ValueId::Inline;
        }
      }
    };

    auto shortValue = SelectShortValue();
    if (!AllowsValue(shortValue))
    {
      return nullptr;
    }

    return PrimitiveValue::Create(shortValue);
  }
}