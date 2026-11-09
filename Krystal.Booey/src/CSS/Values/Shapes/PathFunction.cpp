/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Path>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                    const Path &value) noexcept
  {
    // <path()> = path( <'fill-rule'>? , <string> )

    if (value.FillRule && !std::holds_alternative<keywords::Nonzero>(*value.FillRule))
    {
      Serialise(builder, context, *value.FillRule);
      builder.Append(u8", ");
    }

    // FIXME: Add version of `buildStringFromByteStream` that takes a `StringBuilder`.
    CSSOMString pathString;
    (void)svg::BuildStringFromByteStream(value.Data.ByteStream, pathString,
                                         svg::PathParsingMode::UnalteredParsing);
    SerialiseString(pathString, builder);
  }

  void ComputedStyleDependenciesCollector<Path::PathData>::operator()(ComputedStyleDependencies &,
                                                                      const Path::PathData &) noexcept
  {
  }

  IterationStatus
    ValueChildrenVisitor<Path::PathData>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                                     const Path::PathData &) noexcept
  {
    return IterationStatus::Continue;
  }
}
