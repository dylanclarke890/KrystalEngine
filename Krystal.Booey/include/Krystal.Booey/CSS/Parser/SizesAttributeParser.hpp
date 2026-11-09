// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  struct ParserContext;
  enum class LengthUnit : uint8;

  /// @brief Evaluates a `sizes` attribute: the length of its first entry whose media condition matches, or
  /// `100vw`.
  /// @see https://html.spec.whatwg.org/#sizes-attributes
  /// @see https://html.spec.whatwg.org/#parse-a-sizes-attribute
  class SizesAttributeParser
  {
  private:
    WeakPtr<const dom::Document> _document;
    List<mq::MediaQueryResult> _dynamicMediaQueryResults;
    float _length {0};
    bool _lengthWasSet {false};
    bool _isValid {false};

  public:
    SizesAttributeParser(CSSOMStringView attribute, const dom::Document &document) noexcept;

    KRYS_NODISCARD float Length() const noexcept;

    KRYS_NODISCARD static float ComputeLength(double value, LengthUnit unit,
                                              const dom::Document &document) noexcept;

    /// @brief The media conditions whose result depends on the viewport or the user's preferences.
    KRYS_NODISCARD const List<mq::MediaQueryResult> &DynamicMediaQueryResults() const noexcept
    {
      return _dynamicMediaQueryResults;
    }

  private:
    KRYS_NODISCARD bool Parse(TokenRange range, const ParserContext &context) noexcept;
    KRYS_NODISCARD float EffectiveSize() const noexcept;
    KRYS_NODISCARD Maybe<float> CalculateLengthInPixels(TokenRange range) const noexcept;
    KRYS_NODISCARD bool MediaConditionMatches(const mq::MediaQuery &mediaCondition) const noexcept;
    KRYS_NODISCARD float EffectiveSizeDefaultValue() const noexcept;
  };
}
