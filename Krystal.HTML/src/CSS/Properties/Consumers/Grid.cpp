#include "Krystal.HTML/CSS/Properties/Consumers/Grid.hpp"
#include "Krystal.HTML/CSS/Parser/CSSParserIdioms.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSFunctionValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridAutoRepeatValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridIntegerRepeatValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridLineNamesValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridLineValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridNamedAreaMap.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSGridTemplateAreasValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/CSSSubgridValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridPosition.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD bool IsGridBreadthIdent(CSSValueId id) noexcept
  {
    return IdentMatches<CSSValueId::MinContent, CSSValueId::WebkitMinContent, CSSValueId::MaxContent,
                        CSSValueId::WebkitMaxContent, CSSValueId::Auto>(id);
  }

  KRYS_NODISCARD static RefPtr<CSSPrimitiveValue>
    ConsumeCustomIdentForGridLine(CSSTokenRange &tokens) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::Auto || tokens.Peek().ValueId() == CSSValueId::Span)
    {
      return nullptr;
    }

    return ConsumeCustomIdent(tokens);
  }

  Maybe<GridNamedAreaMapRow> ConsumeUnresolvedGridTemplateAreasRow(CSSTokenRange &tokens,
                                                                   CSSPropertyParserState &) noexcept
  {
    // Utilize the NRVO by having all paths return this one `row` instance to avoid unnecessary copies.
    Maybe<GridNamedAreaMapRow> row;

    if (tokens.Peek().Type() != CSSTokenType::String)
    {
      return row;
    }

    auto rowString = Krys::Text::ConvertToUTF32(tokens.ConsumeIncludingWhitespace().IdentCodePoints());
    if (std::ranges::all_of(rowString, CSSParserIdioms::IsCSSSpace<char32>))
    {
      return row;
    }

    // Once initial checks are completed, the value can be `emplaced` into the `Maybe` to initialize
    // it in-place.
    row.emplace();

    CSSOMString areaName;
    for (auto character : rowString)
    {
      if (CSSParserIdioms::IsCSSSpace(character))
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
        if (!CSSParserIdioms::IsNameCodePoint(character))
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

      areaName.append_range(Krys::Text::ConvertToUTF8(Span<const char32>(&character, 1)));
    }
    if (!areaName.empty())
    {
      row->push_back(areaName);
    }

    return row;
  }

  RefPtr<CSSValue> ConsumeGridLine(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <grid-line> = auto
    //             | <custom-ident>
    //             | [ [ <integer [-∞,-1]> | <integer [1,∞]> ] && <custom-ident>? ]
    //             | [ span && [ <integer [1,∞]> || <custom-ident> ] ]
    //
    // https://drafts.csswg.org/css-grid/#typedef-grid-row-start-grid-line

    if (tokens.Peek().ValueId() == CSSValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    RefPtr<CSSPrimitiveValue> spanValue;
    RefPtr<CSSPrimitiveValue> gridLineName;
    RefPtr<CSSPrimitiveValue> numericValue =
      CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);

    if (numericValue)
    {
      gridLineName = ConsumeCustomIdentForGridLine(tokens);
      spanValue = ConsumeIdent<CSSValueId::Span>(tokens);
    }
    else
    {
      spanValue = ConsumeIdent<CSSValueId::Span>(tokens);
      if (spanValue)
      {
        numericValue = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
        gridLineName = ConsumeCustomIdentForGridLine(tokens);
        if (!numericValue)
        {
          numericValue = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
        }
      }
      else
      {
        gridLineName = ConsumeCustomIdentForGridLine(tokens);
        if (gridLineName)
        {
          numericValue = CSSPrimitiveValueResolver<Integer<>>::ConsumeAndResolve(tokens, state);
          spanValue = ConsumeIdent<CSSValueId::Span>(tokens);
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

    return CSSGridLineValue::Create(Krys::Move(spanValue), Krys::Move(numericValue),
                                    Krys::Move(gridLineName));
  }

  KRYS_NODISCARD static bool IsGridTrackFixedSized(const CSSPrimitiveValue &primitiveValue) noexcept
  {
    switch (primitiveValue.ValueId())
    {
      case CSSValueId::MinContent:
      case CSSValueId::WebkitMinContent:
      case CSSValueId::MaxContent:
      case CSSValueId::WebkitMaxContent:
      case CSSValueId::Auto:
      {
        return false;
      }
      default:
      {
        return !primitiveValue.IsFlex();
      }
    }
  }

  KRYS_NODISCARD static bool IsGridTrackFixedSized(const CSSValue &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value))
    {
      return IsGridTrackFixedSized(*primitiveValue);
    }

    auto &function = Downcast<CSSFunctionValue>(value);
    if (function.Name() == CSSValueId::FitContent || function.Length() < 2)
    {
      return false;
    }

    return IsGridTrackFixedSized(Downcast<CSSPrimitiveValue>(function.Get(0uz)))
           || IsGridTrackFixedSized(Downcast<CSSPrimitiveValue>(function.Get(1uz)));
  }

  KRYS_NODISCARD static RefPtr<CSSPrimitiveValue> ConsumeGridBreadth(CSSTokenRange &tokens,
                                                                     CSSPropertyParserState &state) noexcept
  {
    // <track-breadth>       = <length-percentage [0,∞]> | <flex [0,∞]> | min-content | max-content | auto
    // https://drafts.csswg.org/css-grid/#typedef-track-breadth

    const CSSToken &token = tokens.Peek();
    if (IsGridBreadthIdent(token.ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    // TODO: we should use the CSSUnitType enum here instead of a string comparison
    if (token.Type() == CSSTokenType::Dimension && token.Unit() == u8"fr")
    {
      auto numericValue = tokens.Peek().NumericValue();
      if (numericValue < 0)
      {
        return nullptr;
      }

      tokens.Discard();
      tokens.DiscardWhitespace();

      return CSSPrimitiveValue::Create(numericValue, CSSUnitType::fr);
    }

    return CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
  }

  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeFitContent(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state) noexcept
  {
    CSSTokenRange rangeCopy = tokens;
    CSSTokenRange args = ConsumeFunction(rangeCopy);

    auto length = CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(args, state);
    if (!length || !args.IsAtEnd())
    {
      return nullptr;
    }

    tokens = rangeCopy;
    return CSSFunctionValue::Create(CSSValueId::FitContent, Krys::Move(length));
  }

  RefPtr<CSSValue> ConsumeGridTrackSize(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <track-size>          = <track-breadth> | minmax( <inflexible-breadth> , <track-breadth> ) |
    // fit-content( <length-percentage [0,∞]> ) <track-breadth>       = <length-percentage [0,∞]> | <flex
    // [0,∞]> | min-content | max-content | auto <inflexible-breadth>  = <length-percentage [0,∞]> |
    // min-content | max-content | auto
    //
    // https://drafts.csswg.org/css-grid/#typedef-track-size

    const CSSToken &token = tokens.Peek();
    if (IdentMatches<CSSValueId::Auto>(token.ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    if (token.FunctionId() == CSSValueId::Minmax)
    {
      CSSTokenRange rangeCopy = tokens;
      CSSTokenRange args = ConsumeFunction(rangeCopy);

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

      return CSSFunctionValue::Create(CSSValueId::Minmax, Krys::Move(minTrackBreadth),
                                      Krys::Move(maxTrackBreadth));
    }

    if (token.FunctionId() == CSSValueId::FitContent)
    {
      return ConsumeFitContent(tokens, state);
    }

    return ConsumeGridBreadth(tokens, state);
  }

  RefPtr<CSSGridLineNamesValue> ConsumeGridLineNames(CSSTokenRange &tokens, CSSPropertyParserState &,
                                                     AllowEmpty allowEmpty) noexcept
  {
    CSSTokenRange rangeCopy = tokens;
    if (rangeCopy.ConsumeIncludingWhitespace().Type() != CSSTokenType::OpenSquare)
    {
      return nullptr;
    }

    SmallList<CSSOMString, 4> lineNames;
    while (auto lineName = ConsumeCustomIdentForGridLine(rangeCopy))
    {
      lineNames.push_back(lineName->CustomIdent());
    }

    if (rangeCopy.ConsumeIncludingWhitespace().Type() != CSSTokenType::CloseSquare)
    {
      return nullptr;
    }

    tokens = rangeCopy;
    if (allowEmpty == AllowEmpty::No && lineNames.empty())
    {
      return nullptr;
    }

    return CSSGridLineNamesValue::Create(lineNames);
  }

  KRYS_NODISCARD static bool ConsumeGridTrackRepeatFunction(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            CSSValueListBuilder &list, bool &isAutoRepeat,
                                                            bool &allTracksAreFixedSized) noexcept
  {
    CSSTokenRange args = ConsumeFunction(tokens);
    CSSValueListBuilder repeatedValues;

    RefPtr<CSSPrimitiveValue> repetitions;
    auto autoRepeatType = ConsumeIdentRaw<CSSValueId::AutoFill, CSSValueId::AutoFit>(args);
    isAutoRepeat = autoRepeatType.has_value();
    if (!isAutoRepeat)
    {
      repetitions =
        CSSPrimitiveValueResolver<Integer<CSSRange {1, CSSRange::Inf}, size_t>>::ConsumeAndResolve(args,
                                                                                                   state);
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

      repeatedValues.push_back(Krys::Move(trackSize));
      ++numberOfTracks;
      if (auto lineNames = ConsumeGridLineNames(args, state))
      {
        repeatedValues.push_back(Krys::Move(lineNames));
      }
    }
    // We should have found at least one <track-size> or else it is not a valid <track-list>.
    if (!numberOfTracks)
    {
      return false;
    }

    if (isAutoRepeat)
    {
      list.push_back(CSSGridAutoRepeatValue::Create(*autoRepeatType, Krys::Move(repeatedValues)));
    }
    else
    {
      auto maxRepetitions = static_cast<int64>(GridPosition::max() / numberOfTracks);
      if (auto repetitionsInteger = repetitions->ResolveAsIntegerIfNotCalculated();
          repetitionsInteger && repetitionsInteger > maxRepetitions)
      {
        repetitions = CSSPrimitiveValue::CreateInteger(static_cast<double>(maxRepetitions));
      }

      list.push_back(CSSGridIntegerRepeatValue::Create(Krys::Move(repetitions), Krys::Move(repeatedValues)));
    }

    return true;
  }

  KRYS_NODISCARD static bool ConsumeSubgridNameRepeatFunction(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              CSSValueListBuilder &list,
                                                              bool &isAutoRepeat) noexcept
  {
    CSSTokenRange args = ConsumeFunction(tokens);
    RefPtr<CSSPrimitiveValue> repetitions;

    isAutoRepeat = ConsumeIdentRaw<CSSValueId::AutoFill>(args).has_value();
    if (!isAutoRepeat)
    {
      repetitions =
        CSSPrimitiveValueResolver<Integer<CSSRange {1, CSSRange::Inf}, size_t>>::ConsumeAndResolve(args,
                                                                                                   state);
      if (!repetitions)
      {
        return false;
      }
      if (auto repetitionsInteger = repetitions->ResolveAsIntegerIfNotCalculated();
          repetitionsInteger && repetitionsInteger > GridPosition::max())
      {
        repetitions = CSSPrimitiveValue::CreateInteger(GridPosition::max());
      }
    }

    if (!ConsumeComma(args))
    {
      return false;
    }

    CSSValueListBuilder repeatedValues;
    do
    {
      auto lineNames = ConsumeGridLineNames(args, state, AllowEmpty::Yes);
      if (!lineNames)
      {
        return false;
      }

      repeatedValues.push_back(Krys::Move(lineNames));
    } while (!args.IsAtEnd());

    if (isAutoRepeat)
    {
      list.push_back(CSSGridAutoRepeatValue::Create(CSSValueId::AutoFill, Krys::Move(repeatedValues)));
    }
    else
    {
      list.push_back(CSSGridIntegerRepeatValue::Create(Krys::Move(repetitions), Krys::Move(repeatedValues)));
    }

    return true;
  }

  RefPtr<CSSValue> ConsumeGridTrackList(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                        TrackListType trackListType) noexcept
  {
    bool seenAutoRepeat = false;
    if (trackListType == GridTemplate && tokens.Peek().ValueId() == CSSValueId::Subgrid)
    {
      DiscardIdent(tokens);
      CSSValueListBuilder values;
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Delim)
      {
        if (tokens.Peek().FunctionId() == CSSValueId::Repeat)
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
          values.push_back(Krys::Move(value));
        }
        else
        {
          return nullptr;
        }
      }

      return CSSSubgridValue::Create(Krys::Move(values));
    }

    bool allowGridLineNames = trackListType != GridAuto;
    if (!allowGridLineNames && tokens.Peek().Type() == CSSTokenType::OpenSquare)
    {
      return nullptr;
    }

    CSSValueListBuilder values;
    bool allowRepeat = trackListType == GridTemplate;
    bool allTracksAreFixedSized = true;
    if (auto lineNames = ConsumeGridLineNames(tokens, state))
    {
      values.push_back(Krys::Move(lineNames));
    }
    do
    {
      bool isAutoRepeat;
      if (tokens.Peek().FunctionId() == CSSValueId::Repeat)
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
      else if (RefPtr<CSSValue> value = ConsumeGridTrackSize(tokens, state))
      {
        if (allTracksAreFixedSized)
        {
          allTracksAreFixedSized = IsGridTrackFixedSized(*value);
        }

        values.push_back(Krys::Move(value));
      }
      else
      {
        return nullptr;
      }

      if (seenAutoRepeat && !allTracksAreFixedSized)
      {
        return nullptr;
      }

      if (!allowGridLineNames && tokens.Peek().Type() == CSSTokenType::OpenSquare)
      {
        return nullptr;
      }

      if (auto lineNames = ConsumeGridLineNames(tokens, state))
      {
        values.push_back(Krys::Move(lineNames));
      }
    } while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Delim);

    return CSSValueList::CreateSpaceSeparated(Krys::Move(values));
  }

  RefPtr<CSSValue> ConsumeGridTemplatesRowsOrColumns(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept
  {
    // none | <track-list> | <auto-track-list> | subgrid <line-name-list>?
    // https://drafts.csswg.org/css-grid/#track-sizing

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    return ConsumeGridTrackList(tokens, state, GridTemplate);
  }

  RefPtr<CSSValue> ConsumeGridTemplateAreas(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
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
    } while (tokens.Peek().Type() == CSSTokenType::String);

    if (!map.rowCount)
    {
      return nullptr;
    }

    return CSSGridTemplateAreasValue::Create({Krys::Move(map)});
  }

  RefPtr<CSSValue> ConsumeGridAutoFlow(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    auto rowOrColumnValue = ConsumeIdent<CSSValueId::Row, CSSValueId::Column, CSSValueId::Normal>(tokens);
    auto denseAlgorithm = ConsumeIdent<CSSValueId::Dense>(tokens);

    if (!rowOrColumnValue)
    {
      rowOrColumnValue = ConsumeIdent<CSSValueId::Row, CSSValueId::Column>(tokens);
      if (!rowOrColumnValue && !denseAlgorithm)
      {
        return nullptr;
      }
    }

    CSSValueListBuilder parsedValues;
    if (rowOrColumnValue)
    {
      parsedValues.push_back(Krys::Move(rowOrColumnValue));
    }

    if (denseAlgorithm)
    {
      parsedValues.push_back(Krys::Move(denseAlgorithm));
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(parsedValues));
  }
}