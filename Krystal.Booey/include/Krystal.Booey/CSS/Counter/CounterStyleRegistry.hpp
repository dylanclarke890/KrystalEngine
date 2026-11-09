/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  struct CounterStyle;
}

namespace krys::boo::css
{
  using CounterStyleMap = HashMap<CSSOMStringAtom, Ref<CounterStyle>>;

  /// @brief The counter styles of a document: its author `@counter-style` rules, over the predefined (user
  /// agent) counter styles shared by every document.
  class CounterStyleRegistry
  {
  private:
    CounterStyleMap _authorCounterStyles;
    bool _hasUnresolvedReferences {true};

  public:
    CounterStyleRegistry() noexcept = default;

    KRYS_NODISCARD static Ref<CounterStyle> DecimalCounter() noexcept;

    /// @brief The counter style named @p name, with its references resolved.
    KRYS_NODISCARD Ref<CounterStyle> ResolvedCounterStyle(const CSSOMStringAtom &name) noexcept;
    /// @brief The counter style @p style names, with its references resolved.
    KRYS_NODISCARD Ref<CounterStyle> ResolvedCounterStyle(const style::CounterStyle &style) noexcept;
    void ResolveReferencesIfNeeded() noexcept;

    void AddCounterStyle(const CounterStyleDescriptors &descriptors) noexcept;

    static void AddUserAgentCounterStyle(const CounterStyleDescriptors &descriptors) noexcept;
    static void ResolveUserAgentReferences() noexcept;

    KRYS_NODISCARD bool HasAuthorCounterStyles() const noexcept
    {
      return !_authorCounterStyles.empty();
    }

    void ClearAuthorCounterStyles() noexcept;

    KRYS_NODISCARD bool operator==(const CounterStyleRegistry &other) const noexcept;

  private:
    KRYS_NODISCARD static CounterStyleMap &UserAgentCounterStyles() noexcept;

    // If no map is passed on, user-agent counter styles map will be used
    static void ResolveFallbackReference(CounterStyle &counter, CounterStyleMap *map = nullptr) noexcept;
    static void ResolveExtendsReference(CounterStyle &counter, CounterStyleMap *map = nullptr) noexcept;
    static void ResolveExtendsReference(CounterStyle &counter, HashSet<CounterStyle *> &countersInChain,
                                        CounterStyleMap *map = nullptr) noexcept;

    KRYS_NODISCARD static Ref<CounterStyle> GetCounterStyle(const CSSOMStringAtom &name,
                                                            CounterStyleMap *map = nullptr) noexcept;

    void Invalidate() noexcept;
  };
}
