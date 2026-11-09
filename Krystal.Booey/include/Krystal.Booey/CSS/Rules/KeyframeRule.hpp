/*
 * Copyright (C) 2007-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  /// @brief A keyframe inside `@keyframes`: its keyframe selectors and declarations.
  class KeyframeRule final : public Rule
  {
  public:
    /// @brief A keyframe selector: an offset, optionally within a named timeline range.
    struct Key
    {
      /// @brief The timeline range (`contain`, `cover`, `entry`, ...), or `ValueId::Invalid` for none.
      ValueId RangeName;
      double Offset;

      void WriteToString(StringBuilder &builder) const noexcept;
      KRYS_NODISCARD bool operator==(const Key &) const noexcept = default;
    };

  private:
    Ref<StyleProperties> _properties;
    List<Key> _keys;

    explicit KeyframeRule(Ref<StyleProperties> &&properties) noexcept;
    KeyframeRule(List<Key> &&keys, Ref<StyleProperties> &&properties) noexcept;

  public:
    ~KeyframeRule() noexcept;

    KRYS_NODISCARD static Ref<KeyframeRule> Create(Ref<StyleProperties> &&properties) noexcept;
    KRYS_NODISCARD static Ref<KeyframeRule> Create(Span<const Pair<ValueId, double>> keys,
                                                   Ref<StyleProperties> &&properties) noexcept;

    /// @attention Keyframes are never copied.
    KRYS_NODISCARD Ref<KeyframeRule> Copy() const noexcept;

    KRYS_NODISCARD CSSOMString KeyText() const noexcept;

    /// @brief Replaces the keys with those parsed from @p keyText.
    /// @return False, leaving the keys unchanged, if @p keyText isn't a valid keyframe selector list.
    bool SetKeyText(const CSSOMString &keyText) noexcept;

    void SetKey(Key key) noexcept
    {
      krys_debug_assert(_keys.empty());
      _keys.clear();
      _keys.push_back(key);
    }

    KRYS_NODISCARD const List<Key> &Keys() const noexcept
    {
      return _keys;
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    KRYS_NODISCARD CSSOMString CssText() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::KeyframeRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsKeyframeRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
