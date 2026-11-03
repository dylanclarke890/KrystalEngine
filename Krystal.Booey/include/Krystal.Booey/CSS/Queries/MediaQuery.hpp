#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"

namespace krys::boo::css::mq
{
  enum class Prefix : bool
  {
    Not,
    Only
  };

  /// @brief The `all` media type.
  KRYS_NODISCARD inline const CSSOMStringAtom &AllAtom() noexcept
  {
    static const CSSOMStringAtom all {u8"all"};
    return all;
  }

  /// @brief The `print` media type.
  KRYS_NODISCARD inline const CSSOMStringAtom &PrintAtom() noexcept
  {
    static const CSSOMStringAtom print {u8"print"};
    return print;
  }

  /// @brief The `screen` media type.
  KRYS_NODISCARD inline const CSSOMStringAtom &ScreenAtom() noexcept
  {
    static const CSSOMStringAtom screen {u8"screen"};
    return screen;
  }

  /// @brief A media query: an optional `not`/`only` prefix, a media type and an optional condition.
  struct MediaQuery
  {
    Maybe<mq::Prefix> Prefix;
    CSSOMStringAtom MediaType {CSSOMStringAtom::Null()};
    Maybe<mq::Condition> Condition {};
  };

  using MediaQueryList = List<MediaQuery>;

  struct MediaQueryResult
  {
    mq::MediaQueryList MediaQueryList;
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
