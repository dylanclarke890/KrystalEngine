#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief Converts decoded pixels from the colour space an image's embedded ICC profile describes to sRGB,
  /// as the image decoders write them (with Little CMS).
  /// @note A profile that describes sRGB, or one the transform can't be made from, gives no transform, so the
  /// pixels are left as they are.
  /// FIXME: Convert to the display's colour space rather than sRGB, once painting has wide gamut colours.
  class ICCProfileTransform
  {
    KRYS_NON_COPYABLE(ICCProfileTransform);

  public:
    /// @brief How a row of 8-bit pixels is laid out.
    enum class PixelLayout : uint8
    {
      RGB,
      RGBA,
      BGRA,
      /// @brief Adobe's inverted CMYK, as JPEGs store it.
      InvertedCMYK
    };

  private:
    void *_transform {nullptr};

  public:
    ICCProfileTransform() noexcept = default;
    ICCProfileTransform(ICCProfileTransform &&other) noexcept;
    ICCProfileTransform &operator=(ICCProfileTransform &&other) noexcept;
    ~ICCProfileTransform() noexcept;

    /// @brief The transform from the colour space @p profile describes, with pixels laid out as @p source, to
    /// sRGB laid out as @p destination. Empty if the profile can't be read, its colour space doesn't match
    /// @p source (RGB, or CMYK for `InvertedCMYK`), or it describes sRGB.
    /// @note Alpha, where both layouts have it, is copied unchanged.
    KRYS_NODISCARD static ICCProfileTransform Create(Span<const uint8> profile, PixelLayout source,
                                                     PixelLayout destination) noexcept;

    /// @brief Whether @p profile can be read and describes the colour space of pixels laid out as @p source
    /// (RGB, or CMYK for `InvertedCMYK`), whether or not it changes them.
    KRYS_NODISCARD static bool DescribesPixels(Span<const uint8> profile, PixelLayout source) noexcept;

    /// @brief Whether there's a transform to apply.
    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _transform;
    }

    /// @brief Converts @p pixelCount pixels from @p source to @p destination, which may be the same row when
    /// both layouts are the same size.
    void Transform(const uint8 *source, uint8 *destination, uint32 pixelCount) const noexcept;

  private:
    explicit ICCProfileTransform(void *transform) noexcept : _transform(transform)
    {
    }
  };
}
