#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <limits>

namespace krys::boo::css
{
  class TokenRange;

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
    auto ConsumeListSeparatedByIntoBuilder(TokenRange &tokens, SubConsumer &&subConsumer,
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

        list.push_back(krys::move(value));
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

      return {krys::move(list)};
    }

    template <char Separator, ListBounds Bounds, ListOptimization Optimization = ListOptimization::None,
              typename ListType = CSSValueList, typename SubConsumer, typename... Args>
    auto ConsumeListSeparatedBy(TokenRange &tokens, SubConsumer &&subConsumer, Args &&...args)
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
          return krys::move((*list)[0]);
        }
      }

      if constexpr (SameType<ListType, CSSValueList>)
      {
        if constexpr (Separator == ',')
        {
          return CSSValueList::CreateCommaSeparated(krys::move(*list));
        }
        else if constexpr (Separator == '/')
        {
          return CSSValueList::CreateSlashSeparated(krys::move(*list));
        }
        else if constexpr (Separator == ' ')
        {
          return CSSValueList::CreateSpaceSeparated(krys::move(*list));
        }
      }
      else
      {
        return ListType::Create(krys::move(*list));
      }
    }
  }
}