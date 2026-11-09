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

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::cq
{
  Axis RequiredAxesForFeature(const mq::Feature &feature) noexcept
  {
    if (feature.Schema == &Features::Width())
    {
      return Axis::Width;
    }
    if (feature.Schema == &Features::Height())
    {
      return Axis::Height;
    }
    if (feature.Schema == &Features::InlineSize())
    {
      return Axis::Inline;
    }
    if (feature.Schema == &Features::BlockSize())
    {
      return Axis::Block;
    }
    if (feature.Schema == &Features::AspectRatio() || feature.Schema == &Features::Orientation())
    {
      return Axis::Inline | Axis::Block;
    }
    return Axis::None;
  }

  void Serialise(StringBuilder &builder, const ContainerQuery &query) noexcept
  {
    if (query.Name != CSSOMStringAtom::Null() && query.Name != CSSOMStringAtom::Empty())
    {
      SerialiseIdentifier(CSSOMString(query.Name.View()), builder);
      if (!query.Condition.Queries.empty())
      {
        builder.Append(u8' ');
      }
    }

    mq::Serialise(builder, query.Condition);
  }

  void Serialise(StringBuilder &builder, const ContainerQueryList &queries) noexcept
  {
    for (const auto &query : queries)
    {
      if (&query != &queries.front())
      {
        builder.Append(u8", ");
      }
      Serialise(builder, query);
    }
  }
}
