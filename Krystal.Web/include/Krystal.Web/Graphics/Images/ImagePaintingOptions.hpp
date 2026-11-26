/*
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Image/ImageOrientation.hpp"
#include "Krystal.Web/Graphics/GraphicsTypes.hpp"
#include "Krystal.Web/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Web/Graphics/Images/ImageTypes.hpp"

namespace krys::web
{
  enum class ImageDrawResult : uint8
  {
    DidNothing,
    DidRequestDecoding,
    DidRecord,
    DidDraw,
  };

  /// @brief How an image is drawn: its compositing, blend mode, decoding, orientation, interpolation,
  /// subsampling and debug background, set from any of those in any order.
  /// FIXME: The HDR options, once there's HDR.
  struct ImagePaintingOptions
  {
    template <typename Type>
    constexpr static bool IsOptionType =
      std::is_same_v<Type, CompositeOperator> || std::is_same_v<Type, web::BlendMode>
      || std::is_same_v<Type, img::ImageOrientation>
      || std::is_same_v<Type, img::ImageOrientation::Orientation>
      || std::is_same_v<Type, web::InterpolationQuality> || std::is_same_v<Type, DecodingMode>
      || std::is_same_v<Type, AllowImageSubsampling> || std::is_same_v<Type, ShowDebugBackground>;

  private:
    CompositeOperator _compositeOperator {CompositeOperator::SourceOver};
    web::BlendMode _blendMode {web::BlendMode::Normal};
    web::DecodingMode _decodingMode {web::DecodingMode::Synchronous};
    img::ImageOrientation _orientation {img::ImageOrientation::Orientation::None};
    web::InterpolationQuality _interpolationQuality {web::InterpolationQuality::Default};
    web::AllowImageSubsampling _allowImageSubsampling {web::AllowImageSubsampling::No};
    web::ShowDebugBackground _showDebugBackground {web::ShowDebugBackground::No};

  public:
    ImagePaintingOptions() noexcept = default;

    template <typename T>
    requires IsOptionType<std::decay_t<T>>
    explicit ImagePaintingOptions(T option) noexcept
    {
      SetOption(option);
    }

    template <typename T, typename U, typename... Rest>
    requires IsOptionType<std::decay_t<T>>
    ImagePaintingOptions(T first, U second, Rest... rest) noexcept
    {
      SetOption(first);
      SetOption(second);
      (SetOption(rest), ...);
    }

    template <typename... Overrides>
    ImagePaintingOptions(const ImagePaintingOptions &other, Overrides... overrides) noexcept
        : ImagePaintingOptions(other)
    {
      (SetOption(overrides), ...);
    }

    ImagePaintingOptions(const ImagePaintingOptions &) noexcept = default;

    ImagePaintingOptions &operator=(const ImagePaintingOptions &) noexcept = default;

    KRYS_NODISCARD CompositeOperator GetCompositeOperator() const noexcept
    {
      return _compositeOperator;
    }

    KRYS_NODISCARD web::BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }

    KRYS_NODISCARD web::DecodingMode GetDecodingMode() const noexcept
    {
      return _decodingMode;
    }

    KRYS_NODISCARD img::ImageOrientation Orientation() const noexcept
    {
      return _orientation;
    }

    KRYS_NODISCARD web::InterpolationQuality GetInterpolationQuality() const noexcept
    {
      return _interpolationQuality;
    }

    KRYS_NODISCARD web::AllowImageSubsampling GetAllowImageSubsampling() const noexcept
    {
      return _allowImageSubsampling;
    }

    KRYS_NODISCARD web::ShowDebugBackground GetShowDebugBackground() const noexcept
    {
      return _showDebugBackground;
    }

  private:
    void SetOption(CompositeOperator compositeOperator) noexcept
    {
      _compositeOperator = compositeOperator;
    }

    void SetOption(web::BlendMode blendMode) noexcept
    {
      _blendMode = blendMode;
    }

    void SetOption(img::ImageOrientation orientation) noexcept
    {
      _orientation = orientation;
    }

    void SetOption(img::ImageOrientation::Orientation orientation) noexcept
    {
      _orientation = orientation;
    }

    void SetOption(web::InterpolationQuality interpolationQuality) noexcept
    {
      _interpolationQuality = interpolationQuality;
    }

    void SetOption(web::DecodingMode decodingMode) noexcept
    {
      _decodingMode = decodingMode;
    }

    void SetOption(web::AllowImageSubsampling allowImageSubsampling) noexcept
    {
      _allowImageSubsampling = allowImageSubsampling;
    }

    void SetOption(web::ShowDebugBackground showDebugBackground) noexcept
    {
      _showDebugBackground = showDebugBackground;
    }
  };
}
