#include "Krystal.Booey/CSS/Properties/Consumers/Grid.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineValue.hpp"
#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD bool IsGridBreadthIdent(ValueId id) noexcept
  {
    return IdentMatches<ValueId::MinContent, ValueId::WebkitMinContent, ValueId::MaxContent,
                        ValueId::WebkitMaxContent, ValueId::Auto>(id);
  }

  KRYS_NODISCARD static RefPtr<PrimitiveValue> ConsumeCustomIdentForGridLine(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().ValueId() == ValueId::Auto || tokens.Peek().ValueId() == ValueId::Span)
    {
      return nullptr;
    }

    return ConsumeCustomIdent(tokens);
  }

  Maybe<GridNamedAreaMapRow> ConsumeUnresolvedGridTemplateAreasRow(TokenRange &tokens,
                                                                   PropertyParserState &) noexcept
  {
    // Utilize the NRVO by having all paths return this one `row` instance to avoid unnecessary copies.
    Maybe<GridNamedAreaMapRow> row;

    if (tokens.Peek().Type() != TokenType::String)
    {
      return row;
    }

    auto decodeResult =
      krys::text::Decode<krys::text::UTF8>(tokens.ConsumeIncludingWhitespace().IdentCodePoints());
    krys_debug_assert(decodeResult.Error != krys::text::DecodeError::None);
    auto &rowString = decodeResult.Output;
    if (std::ranges::all_of(rowString, IsCSSSpace<char32>))
    {
      return row;
    }

    // Once initial checks are completed, the value can be `emplaced` into the `Maybe` to initialize
    // it in-place.
    row.emplace();

    CSSOMString areaName;
    for (auto character : rowString)
    {
      if (IsCSSSpace(character))
      {
        if (!areaName.empty())
        {
          row->push_back(areaName);
          areaName.clear();
        }
        continue;
      }
      if (character == '.')
      {
        if (areaName == u8".")
        {
          continue;
        }

        if (!areaName.empty())
        {
          row->push_back(areaName);
          areaName.clear();
        }
      }
      else
      {
        if (!IsNameCodePoint(character))
        {
          // In this error case, we simply destroy the row in-place, and return it its now `std::nullopt`
          // state.
          row = {};
          return row;
        }
        if (areaName == u8".")
        {
          row->push_back(u8".");
          areaName.clear();
        }
      }

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<const char32>(&character, 1));
      krys_debug_assert(encodeResult.Error != krys::text::EncodeError::None);
      areaName.append_range(encodeResult.Output);
    }

    if (!areaName.empty())
    {
      row->push_back(areaName);
    }

    return row;
  }

  RefPtr<Value> ConsumeGridLine(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <grid-line> = auto
    //             | <custom-ident>
    //             | [ [ <integer [-∞,-1]> | <integer [1,∞]> ] && <custom-ident>? ]
    //             | [ span && [ <integer [1,∞]> || <custom-ident> ] ]
    //
    // https://drafts.csswg.org/css-grid/#typedef-grid-row-start-grid-line

    if (tokens.Peek().ValueId() == ValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    RefPtr<PrimitiveValue> spanValue;
    RefPtr<PrimitiveValue> gridLineName;
    RefPtr<PrimitiveValue> numericValue = PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);

    if (numericValue)
    {
      gridLineName = ConsumeCustomIdentForGridLine(tokens);
      spanValue = ConsumeIdent<ValueId::Span>(tokens);
    }
    else
    {
      spanValue = ConsumeIdent<ValueId::Span>(tokens);
      if (spanValue)
      {
        numericValue = PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
        gridLineName = ConsumeCustomIdentForGridLine(tokens);
        if (!numericValue)
        {
          numericValue = PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
        }
      }
      else
      {
        gridLineName = ConsumeCustomIdentForGridLine(tokens);
        if (gridLineName)
        {
          numericValue = PrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
          spanValue = ConsumeIdent<ValueId::Span>(tokens);
          if (!spanValue && !numericValue)
          {
            return gridLineName;
          }
        }
        else
        {
          return nullptr;
        }
      }
    }

    if (spanValue && !numericValue && !gridLineName)
    {
      return nullptr; // "span" keyword alone is invalid.
    }
    if (spanValue && numericValue && numericValue->IsNegative().value_or(false))
    {
      return nullptr; // Negative numbers are not allowed for span.
    }
    if (numericValue && numericValue->IsZero().value_or(false))
    {
      return nullptr; // An <integer> value of zero makes the declaration invalid.
    }

    return GridLineValue::Create(krys::move(spanValue), krys::move(numericValue), krys::move(gridLineName));
  }

  KRYS_NODISCARD static bool IsGridTrackFixedSized(const PrimitiveValue &primitiveValue) noexcept
  {
    switch (primitiveValue.ValueId())
    {
      case ValueId::MinContent:
      case ValueId::WebkitMinContent:
      case ValueId::MaxContent:
      case ValueId::WebkitMaxContent:
      case ValueId::Auto:
      {
        return false;
      }
      default:
      {
        return !primitiveValue.IsFlex();
      }
    }
  }

  KRYS_NODISCARD static bool IsGridTrackFixedSized(const Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value))
    {
      return IsGridTrackFixedSized(*primitiveValue);
    }

    auto &function = Downcast<FunctionValue>(value);
    if (function.Name() == ValueId::FitContent || function.Length() < 2)
    {
      return false;
    }

    return IsGridTrackFixedSized(Downcast<PrimitiveValue>(function.Get(0uz)))
           || IsGridTrackFixedSized(Downcast<PrimitiveValue>(function.Get(1uz)));
  }

  KRYS_NODISCARD static RefPtr<PrimitiveValue> ConsumeGridBreadth(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept
  {
    // <track-breadth>       = <length-percentage [0,∞]> | <flex [0,∞]> | min-content | max-content | auto
    // https://drafts.csswg.org/css-grid/#typedef-track-breadth

    const Token &token = tokens.Peek();
    if (IsGridBreadthIdent(token.ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    // TODO: we should use the UnitType enum here instead of a string comparison
    if (token.Type() == TokenType::Dimension && token.Unit() == u8"fr")
    {
      auto numericValue = tokens.Peek().NumericValue();
      if (numericValue < 0)
      {
        return nullptr;
      }

      tokens.Discard();
      tokens.DiscardWhitespace();

      return PrimitiveValue::Create(numericValue, UnitType::fr);
    }

    return PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
  }

  KRYS_NODISCARD static RefPtr<Value> ConsumeFitContent(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept
  {
    TokenRange rangeCopy = tokens;
    TokenRange args = ConsumeFunction(rangeCopy);

    auto length = PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(args, state);
    if (!length || !args.IsAtEnd())
    {
      return nullptr;
    }

    tokens = rangeCopy;
    return FunctionValue::Create(ValueId::FitContent, krys::move(length));
  }

  RefPtr<Value> ConsumeGridTrackSize(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <track-size>          = <track-breadth> | minmax( <inflexible-breadth> , <track-breadth> ) |
    // fit-content( <length-percentage [0,∞]> ) <track-breadth>       = <length-percentage [0,∞]> | <flex
    // [0,∞]> | min-content | max-content | auto <inflexible-breadth>  = <length-percentage [0,∞]> |
    // min-content | max-content | auto
    //
    // https://drafts.csswg.org/css-grid/#typedef-track-size

    const Token &token = tokens.Peek();
    if (IdentMatches<ValueId::Auto>(token.ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    if (token.FunctionId() == ValueId::Minmax)
    {
      TokenRange rangeCopy = tokens;
      TokenRange args = ConsumeFunction(rangeCopy);

      auto minTrackBreadth = ConsumeGridBreadth(args, state);
      if (!minTrackBreadth || minTrackBreadth->IsFlex() || !ConsumeComma(args))
      {
        return nullptr;
      }

      auto maxTrackBreadth = ConsumeGridBreadth(args, state);
      if (!maxTrackBreadth || !args.IsAtEnd())
      {
        return nullptr;
      }

      tokens = rangeCopy;

      return FunctionValue::Create(ValueId::Minmax, krys::move(minTrackBreadth), krys::move(maxTrackBreadth));
    }

    if (token.FunctionId() == ValueId::FitContent)
    {
      return ConsumeFitContent(tokens, state);
    }

    return ConsumeGridBreadth(tokens, state);
  }

  RefPtr<GridLineNamesValue> ConsumeGridLineNames(TokenRange &tokens, PropertyParserState &,
                                                  AllowEmpty allowEmpty) noexcept
  {
    TokenRange rangeCopy = tokens;
    if (rangeCopy.ConsumeIncludingWhitespace().Type() != TokenType::OpenSquare)
    {
      return nullptr;
    }

    SmallList<CSSOMString, 4> lineNames;
    while (auto lineName = ConsumeCustomIdentForGridLine(rangeCopy))
    {
      lineNames.push_back(lineName->CustomIdent());
    }

    if (rangeCopy.ConsumeIncludingWhitespace().Type() != TokenType::CloseSquare)
    {
      return nullptr;
    }

    tokens = rangeCopy;
    if (allowEmpty == AllowEmpty::No && lineNames.empty())
    {
      return nullptr;
    }

    return GridLineNamesValue::Create(lineNames);
  }

  KRYS_NODISCARD static bool ConsumeGridTrackRepeatFunction(TokenRange &tokens, PropertyParserState &state,
                                                            ValueListBuilder &list, bool &isAutoRepeat,
                                                            bool &allTracksAreFixedSized) noexcept
  {
    TokenRange args = ConsumeFunction(tokens);
    ValueListBuilder repeatedValues;

    RefPtr<PrimitiveValue> repetitions;
    auto autoRepeatType = ConsumeIdentRaw<ValueId::AutoFill, ValueId::AutoFit>(args);
    isAutoRepeat = autoRepeatType.has_value();
    if (!isAutoRepeat)
    {
      repetitions =
        PrimitiveValueResolver<Integer<Range {1, Range::Inf}, size_t>>::ConsumeAndResolve(args, state);
      if (!repetitions)
      {
        return false;
      }
    }

    if (!ConsumeComma(args))
    {
      return false;
    }

    if (auto lineNames = ConsumeGridLineNames(args, state))
    {
      repeatedValues.push_back(lineNames);
    }

    size_t numberOfTracks = 0;
    while (!args.IsAtEnd())
    {
      auto trackSize = ConsumeGridTrackSize(args, state);
      if (!trackSize)
      {
        return false;
      }

      if (allTracksAreFixedSized)
      {
        allTracksAreFixedSized = IsGridTrackFixedSized(*trackSize);
      }

      repeatedValues.push_back(krys::move(trackSize));
      ++numberOfTracks;
      if (auto lineNames = ConsumeGridLineNames(args, state))
      {
        repeatedValues.push_back(krys::move(lineNames));
      }
    }
    // We should have found at least one <track-size> or else it is not a valid <track-list>.
    if (!numberOfTracks)
    {
      return false;
    }

    if (isAutoRepeat)
    {
      list.push_back(GridAutoRepeatValue::Create(*autoRepeatType, krys::move(repeatedValues)));
    }
    else
    {
      auto maxRepetitions = static_cast<int64>(GridPosition::max() / numberOfTracks);
      if (auto repetitionsInteger = repetitions->ResolveAsIntegerIfNotCalculated();
          repetitionsInteger && repetitionsInteger > maxRepetitions)
      {
        repetitions = PrimitiveValue::CreateInteger(static_cast<double>(maxRepetitions));
      }

      list.push_back(GridIntegerRepeatValue::Create(krys::move(repetitions), krys::move(repeatedValues)));
    }

    return true;
  }

  KRYS_NODISCARD static bool ConsumeSubgridNameRepeatFunction(TokenRange &tokens, PropertyParserState &state,
                                                              ValueListBuilder &list,
                                                              bool &isAutoRepeat) noexcept
  {
    TokenRange args = ConsumeFunction(tokens);
    RefPtr<PrimitiveValue> repetitions;

    isAutoRepeat = ConsumeIdentRaw<ValueId::AutoFill>(args).has_value();
    if (!isAutoRepeat)
    {
      repetitions =
        PrimitiveValueResolver<Integer<Range {1, Range::Inf}, size_t>>::ConsumeAndResolve(args, state);
      if (!repetitions)
      {
        return false;
      }
      if (auto repetitionsInteger = repetitions->ResolveAsIntegerIfNotCalculated();
          repetitionsInteger && repetitionsInteger > GridPosition::max())
      {
        repetitions = PrimitiveValue::CreateInteger(GridPosition::max());
      }
    }

    if (!ConsumeComma(args))
    {
      return false;
    }

    ValueListBuilder repeatedValues;
    do
    {
      auto lineNames = ConsumeGridLineNames(args, state, AllowEmpty::Yes);
      if (!lineNames)
      {
        return false;
      }

      repeatedValues.push_back(krys::move(lineNames));
    } while (!args.IsAtEnd());

    if (isAutoRepeat)
    {
      list.push_back(GridAutoRepeatValue::Create(ValueId::AutoFill, krys::move(repeatedValues)));
    }
    else
    {
      list.push_back(GridIntegerRepeatValue::Create(krys::move(repetitions), krys::move(repeatedValues)));
    }

    return true;
  }

  RefPtr<Value> ConsumeGridTrackList(TokenRange &tokens, PropertyParserState &state,
                                     TrackListType trackListType) noexcept
  {
    bool seenAutoRepeat = false;
    if (trackListType == GridTemplate && tokens.Peek().ValueId() == ValueId::Subgrid)
    {
      DiscardIdent(tokens);
      ValueListBuilder values;
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Delim)
      {
        if (tokens.Peek().FunctionId() == ValueId::Repeat)
        {
          bool isAutoRepeat;
          if (!ConsumeSubgridNameRepeatFunction(tokens, state, values, isAutoRepeat))
          {
            return nullptr;
          }

          if (isAutoRepeat && seenAutoRepeat)
          {
            return nullptr;
          }
          seenAutoRepeat = seenAutoRepeat || isAutoRepeat;
        }
        else if (auto value = ConsumeGridLineNames(tokens, state, AllowEmpty::Yes))
        {
          values.push_back(krys::move(value));
        }
        else
        {
          return nullptr;
        }
      }

      return SubgridValue::Create(krys::move(values));
    }

    bool allowGridLineNames = trackListType != GridAuto;
    if (!allowGridLineNames && tokens.Peek().Type() == TokenType::OpenSquare)
    {
      return nullptr;
    }

    ValueListBuilder values;
    bool allowRepeat = trackListType == GridTemplate;
    bool allTracksAreFixedSized = true;
    if (auto lineNames = ConsumeGridLineNames(tokens, state))
    {
      values.push_back(krys::move(lineNames));
    }
    do
    {
      bool isAutoRepeat;
      if (tokens.Peek().FunctionId() == ValueId::Repeat)
      {
        if (!allowRepeat)
        {
          return nullptr;
        }

        if (!ConsumeGridTrackRepeatFunction(tokens, state, values, isAutoRepeat, allTracksAreFixedSized))
        {
          return nullptr;
        }

        if (isAutoRepeat && seenAutoRepeat)
        {
          return nullptr;
        }

        seenAutoRepeat = seenAutoRepeat || isAutoRepeat;
      }
      else if (RefPtr<Value> value = ConsumeGridTrackSize(tokens, state))
      {
        if (allTracksAreFixedSized)
        {
          allTracksAreFixedSized = IsGridTrackFixedSized(*value);
        }

        values.push_back(krys::move(value));
      }
      else
      {
        return nullptr;
      }

      if (seenAutoRepeat && !allTracksAreFixedSized)
      {
        return nullptr;
      }

      if (!allowGridLineNames && tokens.Peek().Type() == TokenType::OpenSquare)
      {
        return nullptr;
      }

      if (auto lineNames = ConsumeGridLineNames(tokens, state))
      {
        values.push_back(krys::move(lineNames));
      }
    } while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Delim);

    return ValueList::CreateSpaceSeparated(krys::move(values));
  }

  RefPtr<Value> ConsumeGridTemplatesRowsOrColumns(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // none | <track-list> | <auto-track-list> | subgrid <line-name-list>?
    // https://drafts.csswg.org/css-grid/#track-sizing

    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    return ConsumeGridTrackList(tokens, state, GridTemplate);
  }

  RefPtr<Value> ConsumeGridTemplateAreas(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    GridNamedAreaMap map;
    do
    {
      auto row = ConsumeUnresolvedGridTemplateAreasRow(tokens, state);
      if (!row || !AddRow(map, *row))
      {
        return nullptr;
      }
    } while (tokens.Peek().Type() == TokenType::String);

    if (!map.rowCount)
    {
      return nullptr;
    }

    return GridTemplateAreasValue::Create({krys::move(map)});
  }

  RefPtr<Value> ConsumeGridAutoFlow(TokenRange &tokens, PropertyParserState &) noexcept
  {
    auto rowOrColumnValue = ConsumeIdent<ValueId::Row, ValueId::Column, ValueId::Normal>(tokens);
    auto denseAlgorithm = ConsumeIdent<ValueId::Dense>(tokens);

    if (!rowOrColumnValue)
    {
      rowOrColumnValue = ConsumeIdent<ValueId::Row, ValueId::Column>(tokens);
      if (!rowOrColumnValue && !denseAlgorithm)
      {
        return nullptr;
      }
    }

    ValueListBuilder parsedValues;
    if (rowOrColumnValue)
    {
      parsedValues.push_back(krys::move(rowOrColumnValue));
    }

    if (denseAlgorithm)
    {
      parsedValues.push_back(krys::move(denseAlgorithm));
    }

    return ValueList::CreateSpaceSeparated(krys::move(parsedValues));
  }
}