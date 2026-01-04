#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include "Krystal.Lib/Utils/Hijack.hpp"
#include <functional>
#include <iterator>

namespace Krys
{
  template <typename TIter>
  concept CanInvokeToMutableIter = requires { ToMutableIter(std::declval<TIter>()); };

  namespace Impl
  {
    enum class IterStart
    {
      Begin,
      End
    };

    template <typename T>
    concept HasPairIteratorInsert = requires {
      std::declval<T &>().insert(Krys::Ranges::cbegin(std::declval<T &>()),
                                 Krys::Ranges::cbegin(std::declval<T &>()),
                                 Krys::Ranges::cend(std::declval<T &>()));
    };

    template <IterStart from>
    struct ToMutableIterFn : public Krys::Hijack::Token<ToMutableIterFn<from>>,
                             public KrysHijackGlobalToken<ToMutableIterFn<from>>
    {
      template <typename TFromIt, typename TRange>
      constexpr auto operator()(TFromIt &&fromIt, TRange &source) const noexcept
      {
        using TURange = remove_cvref_t<TRange>;
        using TToIt = typename TURange::iterator;
        if constexpr (SameType<remove_cv_t<remove_ref_t<TToIt>>, TFromIt>)
        {
          (void)source;
          return TToIt(std::forward<TFromIt>(fromIt));
        }
        else if constexpr (CanInvokeToMutableIter<TFromIt>)
        {
          (void)source;
          return ToMutableIter(std::forward<TFromIt>(fromIt));
        }
        else if constexpr (HasPairIteratorInsert<TRange>)
        {
          // http://eel.is/c++draft/container.requirements#sequence.reqmts-8
          // "The iterator returned from a.insert(p, i, j)
          // points to the copy of the first element inserted into a,
          // or p if i == j."
          // in other words, this is our cheat code to avoid
          // hitting the worst-case-scenario here
          return source.insert(std::forward<TFromIt>(fromIt), Krys::Ranges::cend(source),
                               Krys::Ranges::cend(source));
        }
        else if constexpr (std::is_invocable_r_v<bool, std::not_equal_to<>, TToIt, TFromIt> // cf
                           && (Krys::Ranges::ForwardIterator<TFromIt>                       // cf
                               || Krys::Ranges::BidirectionalIterator<TFromIt>))
        {
          // we can avoid 2N walk of iterators
          // by just moving up by them if they're
          // comparable to one another
          if constexpr (from == IterStart::Begin)
          {
            auto beginIt = Krys::Ranges::begin(source);
            while (beginIt != fromIt)
            {
              ++beginIt;
            }
            return beginIt;
          }
          else
          {
            auto endIt = Krys::Ranges::end(source);
            while (endIt != fromIt)
            {
              --endIt;
            }
            return endIt;
          }
        }
        else
        {
          if constexpr (from == IterStart::Begin)
          {
            // either this is random access and O(1),
            // or this is some other weird iterator and it's O(2N)
            auto beginIt = Krys::Ranges::begin(source);
            auto itDist = std::distance(TFromIt(beginIt), std::forward<TFromIt>(fromIt));
            std::advance(beginIt, itDist);
            return beginIt;
          }
          else
          {
            // either this is random access and O(1),
            // or this is some other weird iterator and it's O(2N)
            auto endIt = Krys::Ranges::end(source);
            auto itDist = std::distance(std::forward<TFromIt>(fromIt), TFromIt(endIt));
            std::advance(endIt, -itDist);
            return endIt;
          }
        }
      }
    };
  }

  /// @brief Uses various methods to attempt to provide a mutable iterator from a non-mutable iterator.
  inline constexpr const Impl::ToMutableIterFn<Impl::IterStart::Begin> ToMutableIterFromBegin = {};

  /// @brief Uses various methods to attempt to provide a mutable iterator from a non-mutable iterator.
  inline constexpr const Impl::ToMutableIterFn<Impl::IterStart::End> ToMutableIterFromEnd = {};

  /// @brief An alias for Krys::ToMutableIterFromBegin.
  inline constexpr const auto &ToMutableIter = ToMutableIterFromBegin;
}