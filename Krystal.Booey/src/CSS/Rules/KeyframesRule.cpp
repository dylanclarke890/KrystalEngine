/*
 * Copyright (C) 2007-2024 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"

namespace krys::boo::css
{
  KeyframesRule::KeyframesRule(const CSSOMStringAtom &name) noexcept : Rule(RuleType::Keyframes), _name(name)
  {
  }

  KeyframesRule::KeyframesRule(const KeyframesRule &other) noexcept
      : Rule(other), _keyframes(other.Keyframes()), _name(other._name)
  {
  }

  KeyframesRule::~KeyframesRule() noexcept = default;

  Ref<KeyframesRule> KeyframesRule::Create(const CSSOMStringAtom &name) noexcept
  {
    return AdoptRef(*new KeyframesRule(name));
  }

  Ref<KeyframesRule> KeyframesRule::Copy() const noexcept
  {
    return AdoptRef(*new KeyframesRule(*this));
  }

  void KeyframesRule::ParserAppendKeyframe(RefPtr<KeyframeRule> &&keyframe) noexcept
  {
    if (!keyframe)
    {
      return;
    }
    _keyframes.push_back(krys::move(keyframe));
  }

  void KeyframesRule::WrapperAppendKeyframe(Ref<KeyframeRule> &&keyframe) noexcept
  {
    _keyframes.push_back(krys::move(keyframe));
  }

  void KeyframesRule::WrapperRemoveKeyframe(uint32 index) noexcept
  {
    _keyframes.erase(_keyframes.begin() + index);
  }

  Maybe<size_t> KeyframesRule::FindKeyframeIndex(const CSSOMString &key) const noexcept
  {
    auto keys = PropertyParserHelpers::ParseKeyframeKeyList(key, StrictParserContext());
    if (keys.empty())
    {
      return {};
    }

    List<KeyframeRule::Key> convertedKeys;
    convertedKeys.reserve(keys.size());
    for (const auto &[rangeName, offset] : keys)
    {
      convertedKeys.push_back({rangeName, offset});
    }

    for (auto i = _keyframes.size(); i--;)
    {
      if (_keyframes[i]->Keys() == convertedKeys)
      {
        return i;
      }
    }
    return {};
  }

  void KeyframesRule::ShrinkToFit() noexcept
  {
    _keyframes.shrink_to_fit();
  }
}
