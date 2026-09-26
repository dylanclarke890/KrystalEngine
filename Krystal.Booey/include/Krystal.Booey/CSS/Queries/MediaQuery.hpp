#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"

namespace krys::boo::css::mq
{
  enum class Prefix : bool
  {
    Not,
    Only
  };

  struct MediaQuery
  {
    Maybe<Prefix> Prefix;
    CSSOMStringAtom MediaType;
    Maybe<Condition> Condition {};
  };

  using MediaQueryList = SmallList<MediaQuery>;

  struct MediaQueryResult
  {
    MediaQueryList MediaQueries;
    bool Result;
  };

  template <typename TraverseFunction>
  void TraverseFeatures(const MediaQuery &query, TraverseFunction &&function) noexcept
  {
    if (query.Condition)
    {
      TraverseFeatures(*query.Condition, function);
    }
  }

  template <typename TraverseFunction>
  void TraverseFeatures(const MediaQueryList &list, TraverseFunction &&function) noexcept
  {
    for (auto &query : list)
    {
      TraverseFeatures(query, function);
    }
  }
}