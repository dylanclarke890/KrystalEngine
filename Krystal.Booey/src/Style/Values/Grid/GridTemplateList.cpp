#include "Krystal.Booey/Style/Values/Grid/GridTemplateList.hpp"

#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::style
{
  GridTemplateList::GridTemplateList(GridTrackList &&entries) noexcept : Tracks {krys::move(entries)}
  {
    if (Tracks.empty())
    {
      return;
    }

    auto createGridLineNamesList = [](const auto &names, uint32 currentNamedGridLine,
                                      GridNamedLinesMap &namedGridLines,
                                      GridOrderedNamedLinesMap &orderedNamedGridLines)
    {
      auto &orderedResult = orderedNamedGridLines.Map.try_emplace(currentNamedGridLine).first->second;
      for (const auto &name : names)
      {
        namedGridLines.Map.try_emplace(name).first->second.push_back(currentNamedGridLine);
        orderedResult.push_back(name);
      }
    };

    uint32 currentNamedGridLine = 0;
    uint32 autoRepeatIndex = 0;
    for (const auto &entry : Tracks)
    {
      krys::SwitchOn(
        entry,
        [&](const GridTrackSize &size)
        {
          ++currentNamedGridLine;
          Sizes.push_back(size);
        },
        [&](const List<css::CSSOMString> &names)
        {
          createGridLineNamesList(names, currentNamedGridLine, NamedLines, OrderedNamedLines);
          // Subgrids only have line names defined, not track sizes, so we want our count
          // to be the number of lines named rather than number of sized tracks.
          if (Subgrid)
          {
            ++currentNamedGridLine;
          }
        },
        [&](const GridTrackEntryRepeat &repeat)
        {
          for (size_t i = 0; i < repeat.Repeats; ++i)
          {
            for (const auto &repeatEntry : repeat.Tracks)
            {
              if (const auto *names = std::get_if<List<css::CSSOMString>>(&repeatEntry))
              {
                createGridLineNamesList(*names, currentNamedGridLine, NamedLines, OrderedNamedLines);
                // Subgrids only have line names defined, not track sizes, so we want our count
                // to be the number of lines named rather than number of sized tracks.
                if (Subgrid)
                {
                  ++currentNamedGridLine;
                }
              }
              else
              {
                ++currentNamedGridLine;
                Sizes.push_back(std::get<GridTrackSize>(repeatEntry));
              }
            }
          }
        },
        [&](const GridTrackEntryAutoRepeat &repeat)
        {
          krys_debug_assert(!autoRepeatIndex);
          autoRepeatIndex = 0;
          AutoRepeatType = repeat.Type;
          for (const auto &autoRepeatEntry : repeat.Tracks)
          {
            if (const auto *names = std::get_if<List<css::CSSOMString>>(&autoRepeatEntry))
            {
              createGridLineNamesList(*names, autoRepeatIndex, AutoRepeatNamedLines,
                                      AutoRepeatOrderedNamedLines);
              if (Subgrid)
              {
                ++autoRepeatIndex;
              }
              continue;
            }
            ++autoRepeatIndex;
            AutoRepeatSizes.push_back(std::get<GridTrackSize>(autoRepeatEntry));
          }
          AutoRepeatInsertionPoint = currentNamedGridLine;
          if (!Subgrid)
          {
            ++currentNamedGridLine;
          }
        },
        [&](const GridTrackEntrySubgrid &) { Subgrid = true; });
    }

    // The parser should have rejected any <track-list> without any <track-size> as
    // this is not conformant to the syntax.
    krys_debug_assert(!Sizes.empty() || !AutoRepeatSizes.empty() || Subgrid);
  }

  // MARK: - Conversion

  GridTemplateList CSSValueConversion<GridTemplateList>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    const css::ValueContainingList *valueList = nullptr;
    GridTrackList trackList;

    const auto *subgridValue = DynamicDowncast<css::SubgridValue>(value);
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }
    }
    else if (subgridValue)
    {
      valueList = subgridValue;
      trackList.emplace_back(GridTrackEntrySubgrid {});
    }
    else if (const auto *list = DynamicDowncast<css::ValueList>(value))
    {
      valueList = list;
    }
    else
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    // https://drafts.csswg.org/css-grid-2/#computed-tracks
    // The computed track list of a non-subgrid axis is a list alternating between line name sets
    // and track sections, with the first and last items being line name sets.
    auto ensureLineNames = [&](auto &list)
    {
      if (subgridValue)
      {
        return;
      }
      if (list.empty() || !std::holds_alternative<List<css::CSSOMString>>(list.back()))
      {
        list.emplace_back(List<css::CSSOMString> {});
      }
    };

    auto buildRepeatList = [&](const css::Value &repeatValue, RepeatTrackList &repeatList)
    {
      const auto *vectorValue = RequiredDowncast<css::ValueContainingList>(state, repeatValue);
      if (!vectorValue)
      {
        return;
      }

      for (const auto &currentValue : *vectorValue)
      {
        if (const auto *namesValue = DynamicDowncast<css::GridLineNamesValue>(currentValue))
        {
          auto names = namesValue->Names();
          repeatList.emplace_back(List<css::CSSOMString>(names.begin(), names.end()));
        }
        else
        {
          ensureLineNames(repeatList);
          repeatList.emplace_back(ConvertCSSValueToStyle<GridTrackSize>(state, currentValue));
        }
      }

      if (!repeatList.empty())
      {
        ensureLineNames(repeatList);
      }
    };

    auto addOne = [&](const css::Value &currentValue)
    {
      if (const auto *namesValue = DynamicDowncast<css::GridLineNamesValue>(currentValue))
      {
        auto names = namesValue->Names();
        trackList.emplace_back(List<css::CSSOMString>(names.begin(), names.end()));
        return;
      }

      ensureLineNames(trackList);

      if (const auto *autoRepeatValue = DynamicDowncast<css::GridAutoRepeatValue>(currentValue))
      {
        auto autoRepeatId = autoRepeatValue->AutoRepeatID();
        krys_debug_assert(autoRepeatId == css::ValueId::AutoFill || autoRepeatId == css::ValueId::AutoFit);

        GridTrackEntryAutoRepeat repeat;
        repeat.Type = autoRepeatId == css::ValueId::AutoFill ? AutoRepeatType::Fill : AutoRepeatType::Fit;
        buildRepeatList(currentValue, repeat.Tracks);
        trackList.emplace_back(krys::move(repeat));
      }
      else if (const auto *integerRepeatValue = DynamicDowncast<css::GridIntegerRepeatValue>(currentValue))
      {
        auto repetitions =
          ClampTo(integerRepeatValue->Repetitions().ResolveAsInteger(state.CssToLengthConversionData()), 1,
                  GridPosition::Max());

        GridTrackEntryRepeat repeat;
        repeat.Repeats = static_cast<uint32>(repetitions);
        buildRepeatList(currentValue, repeat.Tracks);
        trackList.emplace_back(krys::move(repeat));
      }
      else
      {
        trackList.emplace_back(ConvertCSSValueToStyle<GridTrackSize>(state, currentValue));
      }
    };

    if (!valueList)
    {
      addOne(value);
    }
    else
    {
      for (const auto &item : *valueList)
      {
        addOne(item);
      }
    }

    if (!trackList.empty())
    {
      ensureLineNames(trackList);
    }

    return {krys::move(trackList)};
  }

  // MARK: - Blending

  KRYS_NODISCARD static RepeatTrackList BlendRepeatList(const RepeatTrackList &from,
                                                        const RepeatTrackList &to,
                                                        const BlendingContext &context) noexcept
  {
    RepeatTrackList result;

    for (size_t i = 0; i < from.size(); ++i)
    {
      krys::SwitchOn(
        from[i], [&](const GridTrackSize &size)
        { result.emplace_back(style::Blend(size, std::get<GridTrackSize>(to[i]), context)); },
        [&](const List<css::CSSOMString> &names)
        {
          if (context.Progress < 0.5)
          {
            result.emplace_back(names);
          }
          else
          {
            result.emplace_back(std::get<List<css::CSSOMString>>(to[i]));
          }
        });
    }

    return result;
  }

  bool Blending<GridTemplateList>::CanBlend(const GridTemplateList &from, const GridTemplateList &to) noexcept
  {
    if (from.Tracks.size() != to.Tracks.size())
    {
      return false;
    }

    for (size_t i = 0; i < from.Tracks.size(); ++i)
    {
      const auto &toEntry = to.Tracks[i];
      bool canBlend = krys::SwitchOn(
        from.Tracks[i], [&](const GridTrackSize &) { return std::holds_alternative<GridTrackSize>(toEntry); },
        [&](const List<css::CSSOMString> &)
        { return std::holds_alternative<List<css::CSSOMString>>(toEntry); },
        [&](const GridTrackEntryRepeat &repeat)
        {
          const auto *toRepeat = std::get_if<GridTrackEntryRepeat>(&toEntry);
          return toRepeat && repeat.Repeats == toRepeat->Repeats
                 && repeat.Tracks.size() == toRepeat->Tracks.size();
        },
        [](const GridTrackEntryAutoRepeat &) { return false; },
        [](const GridTrackEntrySubgrid &) { return false; });
      if (!canBlend)
      {
        return false;
      }
    }

    return true;
  }

  GridTemplateList Blending<GridTemplateList>::Blend(const GridTemplateList &from, const GridTemplateList &to,
                                                     const BlendingContext &context) noexcept
  {
    if (!CanBlend(from, to))
    {
      return context.Progress < 0.5 ? from : to;
    }

    GridTrackList result;

    for (size_t i = 0; i < from.Tracks.size(); ++i)
    {
      const auto &toEntry = to.Tracks[i];
      krys::SwitchOn(
        from.Tracks[i], [&](const GridTrackSize &size)
        { result.emplace_back(style::Blend(size, std::get<GridTrackSize>(toEntry), context)); },
        [&](const List<css::CSSOMString> &names)
        {
          if (context.Progress < 0.5)
          {
            result.emplace_back(names);
          }
          else
          {
            result.emplace_back(std::get<List<css::CSSOMString>>(toEntry));
          }
        },
        [&](const GridTrackEntryRepeat &repeatFrom)
        {
          const auto &repeatTo = std::get<GridTrackEntryRepeat>(toEntry);
          GridTrackEntryRepeat repeatResult;
          repeatResult.Repeats = repeatFrom.Repeats;
          repeatResult.Tracks = BlendRepeatList(repeatFrom.Tracks, repeatTo.Tracks, context);
          result.emplace_back(krys::move(repeatResult));
        },
        [&](const GridTrackEntryAutoRepeat &repeatFrom)
        {
          const auto &repeatTo = std::get<GridTrackEntryAutoRepeat>(toEntry);
          GridTrackEntryAutoRepeat repeatResult;
          repeatResult.Type = repeatFrom.Type;
          repeatResult.Tracks = BlendRepeatList(repeatFrom.Tracks, repeatTo.Tracks, context);
          result.emplace_back(krys::move(repeatResult));
        },
        [](const GridTrackEntrySubgrid &) {});
    }

    return GridTemplateList {krys::move(result)};
  }
}
