#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <limits>

namespace Krys::HTML
{
  class CSSTokenRange;

  namespace CSSPropertyParserHelpers
  {
    enum class ListOptimization : bool
    {
      None,
      SingleValue
    };

    struct ListBounds
    {
      size_t min;
      size_t max;

      KRYS_NODISCARD constexpr static ListBounds MinimumOf(size_t min) noexcept
      {
        return ListBounds {min, std::numeric_limits<size_t>::max()};
      }

      KRYS_NODISCARD constexpr static ListBounds Exactly(size_t value) noexcept
      {
        return ListBounds {value, value};
      }
    };

    constexpr inline auto ZeroOrMore = ListBounds::MinimumOf(0);
    constexpr inline auto OneOrMore = ListBounds::MinimumOf(1);

    template <char Separator, ListBounds Bounds, typename SubConsumer, typename... Args>
    auto ConsumeListSeparatedByIntoBuilder(CSSTokenRange &tokens, SubConsumer &&subConsumer,
                                           Args &&...args) noexcept -> Maybe<CSSValueListBuilder>
    {
      auto consumeSeparator = [](auto &tokens)
      {
        if constexpr (Separator == ',')
        {
          return ConsumeComma(tokens);
        }
        else if constexpr (Separator == '/')
        {
          return ConsumeSlash(tokens);
        }
        else if constexpr (Separator == ' ')
        {
          return !tokens.IsAtEnd();
        }
      };

      CSSValueListBuilder list;
      do
      {
        auto value = std::invoke(subConsumer, tokens, args...);
        if (!value)
        {
          if constexpr (Separator == ',')
          {
            return {};
          }
          else if constexpr (Separator == '/')
          {
            return {};
          }
          else if constexpr (Separator == ' ')
          {
            break;
          }
        }

        list.push_back(Krys::Move(value));
      } while (consumeSeparator(tokens));

      if constexpr (Bounds.min > 0)
      {
        if (list.size() < Bounds.min)
        {
          return {};
        }
      }
      if constexpr (Bounds.max < std::numeric_limits<size_t>::max())
      {
        if (list.size() > Bounds.max)
        {
          return {};
        }
      }

      return {Krys::Move(list)};
    }

    template <char Separator, ListBounds Bounds, ListOptimization Optimization = ListOptimization::None,
              typename ListType = CSSValueList, typename SubConsumer, typename... Args>
    auto ConsumeListSeparatedBy(CSSTokenRange &tokens, SubConsumer &&subConsumer, Args &&...args)
      -> std::conditional_t<Optimization == ListOptimization::None, RefPtr<ListType>, RefPtr<CSSValue>>
    {
      auto list = ConsumeListSeparatedByIntoBuilder<Separator, Bounds>(
        tokens, std::forward<SubConsumer>(subConsumer), std::forward<Args>(args)...);
      if (!list)
        return nullptr;

      if constexpr (Optimization == ListOptimization::SingleValue)
      {
        if (list->size() == 1uz)
        {
          return Krys::Move((*list)[0]);
        }
      }

      if constexpr (SameType<ListType, CSSValueList>)
      {
        if constexpr (Separator == ',')
        {
          return CSSValueList::CreateCommaSeparated(Krys::Move(*list));
        }
        else if constexpr (Separator == '/')
        {
          return CSSValueList::CreateSlashSeparated(Krys::Move(*list));
        }
        else if constexpr (Separator == ' ')
        {
          return CSSValueList::CreateSpaceSeparated(Krys::Move(*list));
        }
      }
      else
      {
        return ListType::Create(Krys::Move(*list));
      }
    }
  }
}