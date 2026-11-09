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

#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  enum class AllowedImageType
  {
    None = 0,
    URLFunction = 1 << 0,
    RawStringAsURL = 1 << 1,
    ImageSet = 1 << 2,
    GeneratedImage = 1 << 3
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::PropertyParserHelpers::AllowedImageType, 5uz)

namespace krys::boo::css
{
  class Value;
  class TokenRange;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <image>
    // https://drafts.csswg.org/css-images-4/#image-values

    KRYS_NODISCARD RefPtr<Value>
      ConsumeImage(TokenRange &tokens, PropertyParserState &state,
                   AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                        | AllowedImageType::ImageSet
                                                        | AllowedImageType::GeneratedImage) noexcept;

    KRYS_NODISCARD RefPtr<Value>
      ConsumeImageOrNone(TokenRange &tokens, PropertyParserState &state,
                         AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                              | AllowedImageType::ImageSet
                                                              | AllowedImageType::GeneratedImage) noexcept;
  }
}