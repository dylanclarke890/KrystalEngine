/*
 * Copyright (C) 2024 Igalia S.L.
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

/*
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"

namespace krys::boo
{
  namespace css
  {
    enum class FontTechnology : uint8;
  }

  class FontCreationContext;
  class FontDescription;
  class FontPlatformData;
  class SharedBuffer;

  /// @brief A web font loaded for the font backend: a FreeType face made from the font file's bytes.
  // FIXME: The faces of a font collection (itemInCollection), and font-face features and variations, which
  // the font backend doesn't apply yet.
  struct FontCustomPlatformData : public RefCounted<FontCustomPlatformData>
  {
    KRYS_NON_COPY_MOVABLE(FontCustomPlatformData);

    Ref<text::FreeTypeFace> Face;

    explicit FontCustomPlatformData(Ref<text::FreeTypeFace> &&face) noexcept : Face(krys::move(face))
    {
    }

    /// @brief The font in @p buffer (a TrueType or OpenType font; WOFF fonts are converted before), or null
    /// if it isn't one.
    KRYS_NODISCARD static RefPtr<FontCustomPlatformData>
      Create(SharedBuffer &buffer, const css::CSSOMString &itemInCollection) noexcept;

    /// @brief The platform font for @p description, made bold or italic synthetically if asked.
    KRYS_NODISCARD boo::FontPlatformData
      GetFontPlatformData(const FontDescription &description, bool bold, bool italic,
                          const FontCreationContext &fontCreationContext) const noexcept;

    /// @brief Whether fonts in @p format (a `format()` keyword or string, e.g. `woff2`) can be loaded.
    KRYS_NODISCARD static bool SupportsFormat(css::CSSOMStringView format) noexcept;

    /// @brief Whether the font technology @p technology is supported.
    KRYS_NODISCARD static bool SupportsTechnology(css::FontTechnology technology) noexcept;
  };
}
