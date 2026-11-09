// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Parser/AtRuleId.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  using namespace std::string_view_literals;

  AtRuleId AtRuleIdFromName(CSSOMStringView name) noexcept
  {
    constexpr auto mappings = Array<Pair<CSSOMStringView, AtRuleId>, 25> {{
      {u8"annotation"sv, AtRuleId::Annotation},
      {u8"character-variant"sv, AtRuleId::CharacterVariant},
      {u8"charset"sv, AtRuleId::Charset},
      {u8"container"sv, AtRuleId::Container},
      {u8"counter-style"sv, AtRuleId::CounterStyle},
      {u8"font-face"sv, AtRuleId::FontFace},
      {u8"font-feature-values"sv, AtRuleId::FontFeatureValues},
      {u8"font-palette-values"sv, AtRuleId::FontPaletteValues},
      {u8"function"sv, AtRuleId::Function},
      {u8"import"sv, AtRuleId::Import},
      {u8"keyframes"sv, AtRuleId::Keyframes},
      {u8"layer"sv, AtRuleId::Layer},
      {u8"media"sv, AtRuleId::Media},
      {u8"namespace"sv, AtRuleId::Namespace},
      {u8"ornaments"sv, AtRuleId::Ornaments},
      {u8"position-try"sv, AtRuleId::PositionTry},
      {u8"property"sv, AtRuleId::Property},
      {u8"scope"sv, AtRuleId::Scope},
      {u8"starting-style"sv, AtRuleId::StartingStyle},
      {u8"styleset"sv, AtRuleId::Styleset},
      {u8"stylistic"sv, AtRuleId::Stylistic},
      {u8"supports"sv, AtRuleId::Supports},
      {u8"swash"sv, AtRuleId::Swash},
    }};

    for (const auto &[ruleName, id] : mappings)
    {
      if (text::IsASCIICaselessEqual(name, ruleName))
      {
        return id;
      }
    }
    return AtRuleId::Invalid;
  }
}
