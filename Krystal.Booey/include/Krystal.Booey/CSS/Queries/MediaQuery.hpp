/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
