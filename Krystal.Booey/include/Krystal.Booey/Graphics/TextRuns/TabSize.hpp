// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  enum TabSizeValueType
  {
    LengthValueType,
    SpaceValueType,
  };

  /// @brief A used `tab-size`: a number of spaces or a length in pixels.
  class TabSize
  {
  private:
    float _value {0};
    bool _isSpaces {false};

  public:
    constexpr TabSize() noexcept = default;

    constexpr TabSize(float numOrLength, TabSizeValueType isSpaces = SpaceValueType) noexcept
        : _value(numOrLength), _isSpaces(isSpaces)
    {
    }

    KRYS_NODISCARD constexpr bool IsSpaces() const noexcept
    {
      return _isSpaces;
    }

    KRYS_NODISCARD constexpr float WidthInPixels(float spaceWidth) const noexcept
    {
      return _isSpaces ? _value * spaceWidth : _value;
    }

    KRYS_NODISCARD constexpr float Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr explicit operator bool() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr bool operator==(const TabSize &) const noexcept = default;
  };
}
