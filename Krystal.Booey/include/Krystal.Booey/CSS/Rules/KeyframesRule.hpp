/*
 * Copyright (C) 2007, 2008, 2012, 2014 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@keyframes`.
  class KeyframesRule final : public Rule
  {
  private:
    mutable List<Ref<KeyframeRule>> _keyframes;
    CSSOMStringAtom _name;

    explicit KeyframesRule(const CSSOMStringAtom &name) noexcept;
    KeyframesRule(const KeyframesRule &other) noexcept;

  public:
    ~KeyframesRule() noexcept;

    KRYS_NODISCARD static Ref<KeyframesRule> Create(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD Ref<KeyframesRule> Copy() const noexcept;

    KRYS_NODISCARD const List<Ref<KeyframeRule>> &Keyframes() const noexcept
    {
      return _keyframes;
    }

    void ParserAppendKeyframe(RefPtr<KeyframeRule> &&keyframe) noexcept;
    void WrapperAppendKeyframe(Ref<KeyframeRule> &&keyframe) noexcept;
    void WrapperRemoveKeyframe(uint32 index) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    void SetName(const CSSOMStringAtom &name) noexcept
    {
      _name = name;
    }

    /// @return The index of the last keyframe whose keys are those parsed from @p key, or nothing if there
    /// is none or @p key isn't a valid keyframe selector list.
    KRYS_NODISCARD Maybe<size_t> FindKeyframeIndex(const CSSOMString &key) const noexcept;

    void ShrinkToFit() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::KeyframesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsKeyframesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
