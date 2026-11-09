/*
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/Glyph.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  constexpr float GlyphSizeUnknown = -1;

  /// @brief Metrics of a font's glyphs, measured on demand and kept in pages of 16 glyphs.
  template <typename T>
  class GlyphMetricsMap
  {
  private:
    class GlyphMetricsPage
    {
    public:
      static constexpr size_t Size = 16;

    private:
      Array<T, Size> _metrics;

    public:
      GlyphMetricsPage() noexcept = default;

      explicit GlyphMetricsPage(const T &initialValue) noexcept
      {
        Fill(initialValue);
      }

      void Fill(const T &value) noexcept
      {
        _metrics.fill(value);
      }

      KRYS_NODISCARD T MetricsForGlyph(Glyph glyph) const noexcept
      {
        return _metrics[glyph % Size];
      }

      KRYS_NODISCARD const T &ExistingMetricsForGlyph(Glyph glyph) const noexcept
      {
        return _metrics[glyph % Size];
      }

      void SetMetricsForGlyph(Glyph glyph, const T &metrics) noexcept
      {
        _metrics[glyph % Size] = metrics;
      }
    };

    bool _filledPrimaryPage {false};
    // We optimize for the page that contains glyph indices 0-15.
    GlyphMetricsPage _primaryPage;
    HashMap<uint32, UniquePtr<GlyphMetricsPage>> _pages;

  public:
    KRYS_NODISCARD T MetricsForGlyph(Glyph glyph) noexcept
    {
      return LocatePage(glyph / GlyphMetricsPage::Size).MetricsForGlyph(glyph);
    }

    KRYS_NODISCARD const T &ExistingMetricsForGlyph(Glyph glyph) noexcept
    {
      return LocatePage(glyph / GlyphMetricsPage::Size).ExistingMetricsForGlyph(glyph);
    }

    void SetMetricsForGlyph(Glyph glyph, const T &metrics) noexcept
    {
      LocatePage(glyph / GlyphMetricsPage::Size).SetMetricsForGlyph(glyph, metrics);
    }

  private:
    KRYS_NODISCARD GlyphMetricsPage &LocatePage(uint32 pageNumber) noexcept
    {
      if (!pageNumber && _filledPrimaryPage)
      {
        return _primaryPage;
      }
      return LocatePageSlowCase(pageNumber);
    }

    KRYS_NODISCARD GlyphMetricsPage &LocatePageSlowCase(uint32 pageNumber) noexcept
    {
      if (!pageNumber)
      {
        krys_debug_assert(!_filledPrimaryPage);
        _primaryPage.Fill(UnknownMetrics());
        _filledPrimaryPage = true;
        return _primaryPage;
      }

      auto &page = _pages[pageNumber];
      if (!page)
      {
        page = CreateUnique<GlyphMetricsPage>(UnknownMetrics());
      }
      return *page;
    }

    KRYS_NODISCARD static T UnknownMetrics() noexcept;
  };

  template <>
  inline float GlyphMetricsMap<float>::UnknownMetrics() noexcept
  {
    return GlyphSizeUnknown;
  }

  template <>
  inline Maybe<Path> GlyphMetricsMap<Maybe<Path>>::UnknownMetrics() noexcept
  {
    return null;
  }

  template <>
  inline FloatRect GlyphMetricsMap<FloatRect>::UnknownMetrics() noexcept
  {
    return FloatRect(0, 0, GlyphSizeUnknown, GlyphSizeUnknown);
  }
}
