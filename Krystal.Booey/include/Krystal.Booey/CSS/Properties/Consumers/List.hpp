/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <limits>

namespace krys::boo::css
{
  class TokenRange;

  namespace PropertyParserHelpers
  {
    enum class ListOptimization : bool
    {
      None,
      SingleValue
    };

    struct ListBounds
    {
      size_t Min;
      size_t Max;

      KRYS_NODISCARD constexpr static ListBounds MinimumOf(size_t min) noexcept
      {
        return ListBounds {min, std::numeric_limits<size_t>::max()};
      }

      KRYS_NODISCARD constexpr static ListBounds Exactly(size_t value) noexcept
      {
        return ListBounds {value, value};
      }
    };

    constexpr auto ZeroOrMore = ListBounds::MinimumOf(0);
    constexpr auto OneOrMore = ListBounds::MinimumOf(1);

    template <char Separator, ListBounds Bounds, typename SubConsumer, typename... Args>
    KRYS_NODISCARD auto ConsumeListSeparatedByIntoBuilder(TokenRange &tokens, SubConsumer &&subConsumer,
                                                          Args &&...args) noexcept -> Maybe<ValueListBuilder>
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

      ValueListBuilder list;
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

      if constexpr (Bounds.Min > 0)
      {
        if (list.size() < Bounds.Min)
        {
          return {};
        }
      }
      if constexpr (Bounds.Max < std::numeric_limits<size_t>::max())
      {
        if (list.size() > Bounds.Max)
        {
          return {};
        }
      }

      return {krys::move(list)};
    }

    template <char Separator, ListBounds Bounds, ListOptimization Optimization = ListOptimization::None,
              typename ListType = ValueList, typename SubConsumer, typename... Args>
    KRYS_NODISCARD auto ConsumeListSeparatedBy(TokenRange &tokens, SubConsumer &&subConsumer,
                                               Args &&...args) noexcept
      -> std::conditional_t<Optimization == ListOptimization::None, RefPtr<ListType>, RefPtr<Value>>
    {
      auto list = ConsumeListSeparatedByIntoBuilder<Separator, Bounds>(
        tokens, std::forward<SubConsumer>(subConsumer), std::forward<Args>(args)...);
      if (!list)
      {
        return nullptr;
      }

      if constexpr (Optimization == ListOptimization::SingleValue)
      {
        if (list->size() == 1uz)
        {
          return krys::move((*list)[0]);
        }
      }

      if constexpr (SameType<ListType, ValueList>)
      {
        if constexpr (Separator == ',')
        {
          return ValueList::CreateCommaSeparated(krys::move(*list));
        }
        else if constexpr (Separator == '/')
        {
          return ValueList::CreateSlashSeparated(krys::move(*list));
        }
        else if constexpr (Separator == ' ')
        {
          return ValueList::CreateSpaceSeparated(krys::move(*list));
        }
      }
      else
      {
        return ListType::Create(krys::move(*list));
      }
    }
  }
}