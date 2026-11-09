/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class Font;
  class FontCascadeDescription;
  class FontDescription;
  class FontRanges;
  class FontSelector;

  /// @brief Told when the fonts a selector provides change.
  class FontSelectorClient
  {
  public:
    virtual ~FontSelectorClient() noexcept = default;

    virtual void FontsNeedUpdate(FontSelector &selector) noexcept = 0;
  };

  /// @brief Provides the fonts of the families a document names: its web fonts and the generic families its
  /// settings resolve.
  class FontSelector : public RefCounted<FontSelector>, public CanMakeWeakPtr<FontSelector>
  {
  public:
    virtual ~FontSelector() noexcept = default;

    KRYS_NODISCARD virtual FontRanges FontRangesForFamily(const FontDescription &description,
                                                          const StringAtom &family) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<Font> FallbackFontAt(const FontDescription &description,
                                                       size_t index) noexcept = 0;
    KRYS_NODISCARD virtual size_t FallbackFontCount() noexcept = 0;

    virtual void OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                          const StringAtom &family) noexcept = 0;

    virtual void FontCacheInvalidated() noexcept
    {
    }

    virtual void RegisterForInvalidationCallbacks(FontSelectorClient &client) noexcept = 0;
    virtual void UnregisterForInvalidationCallbacks(FontSelectorClient &client) noexcept = 0;

    KRYS_NODISCARD virtual uint32 UniqueId() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 Version() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsSimpleFontSelectorForDescription() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsCSSFontSelector() const noexcept
    {
      return false;
    }
  };
}
