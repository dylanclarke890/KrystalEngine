/*
 * Copyright (C) 2007-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/FontLoadRequest.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Font;
  class FontCreationContext;
  class FontDescription;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::css
{
  class CSSFontFace;
  class CSSFontSelector;

  /// @brief One source of a font face: an installed font (`local()`) or a web font file (`url()`).
  /// @note Owned by its font face, which it shares its reference count with.
  // FIXME: SVG fonts (`<font-face>` elements), once there are those. There are no fonts from array buffers,
  // as there's no `FontFace` interface (see DIFFERENCES.md).
  class CSSFontFaceSource final : public FontLoadRequestClient
  {
    KRYS_NON_COPY_MOVABLE(CSSFontFaceSource);

  public:
    // clang-format off
    //                      => Success
    //                    //
    // Pending => Loading
    //                    \\.
    //                      => Failure
    // clang-format on
    enum class Status : uint8
    {
      Pending,
      Loading,
      Success,
      Failure
    };

  private:
    StringAtom _fontFaceName {StringAtom::Null()}; // Font name for local fonts
    CSSFontFace &_owningCSSFontFace;
    WeakPtr<CSSFontSelector> _fontSelector;     // For remote fonts, to orchestrate loading.
    const RefPtr<FontLoadRequest> _fontRequest; // Also for remote fonts, a pointer to the resource request.

    Status _status {Status::Pending};

  public:
    CSSFontFaceSource(CSSFontFace &owner, StringAtom fontFaceName) noexcept;
    CSSFontFaceSource(CSSFontFace &owner, CSSFontSelector &fontSelector,
                      Ref<FontLoadRequest> &&request) noexcept;
    ~CSSFontFaceSource() noexcept override;

    // FontLoadRequestClient.
    void AddRef() const noexcept final;
    void SubRef() const noexcept final;
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final;

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }

    void OpportunisticallyStartFontDataURLLoading() noexcept;

    void Load(dom::Document *document = nullptr) noexcept;
    KRYS_NODISCARD RefPtr<Font> GetFont(const FontDescription &fontDescription, bool syntheticBold,
                                        bool syntheticItalic,
                                        const FontCreationContext &fontCreationContext) noexcept;

    KRYS_NODISCARD FontLoadRequest *GetFontLoadRequest() const noexcept
    {
      return _fontRequest.get();
    }
    KRYS_NODISCARD bool RequiresExternalResource() const noexcept
    {
      return _fontRequest.get();
    }

  private:
    KRYS_NODISCARD bool ShouldIgnoreFontLoadCompletions() const noexcept;

    void FontLoaded(FontLoadRequest &fontRequest) noexcept override;

    void SetStatus(Status newStatus) noexcept;
  };
}
