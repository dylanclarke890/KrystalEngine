#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys::boo
{
  enum class ImageDrawResult : uint8
  {
    DidNothing,
    DidRequestDecoding,
    DidRecord,
    DidDraw
  };

  /// @brief How an image is drawn: its compositing, blend mode, orientation and interpolation, set from any
  /// of those in any order.
  /// FIXME: The decoding mode, subsampling, debug background and HDR options come with bitmap images.
  struct ImagePaintingOptions
  {
    template <typename Type>
    static constexpr bool IsOptionType =
      std::is_same_v<Type, CompositeOperator> || std::is_same_v<Type, boo::BlendMode>
      || std::is_same_v<Type, ImageOrientation> || std::is_same_v<Type, ImageOrientation::Orientation>
      || std::is_same_v<Type, boo::InterpolationQuality>;

  private:
    CompositeOperator _compositeOperator {CompositeOperator::SourceOver};
    boo::BlendMode _blendMode {boo::BlendMode::Normal};
    ImageOrientation _orientation {ImageOrientation::Orientation::None};
    boo::InterpolationQuality _interpolationQuality {boo::InterpolationQuality::Default};

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
    KRYS_NODISCARD boo::BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept
    {
      return _orientation;
    }
    KRYS_NODISCARD boo::InterpolationQuality GetInterpolationQuality() const noexcept
    {
      return _interpolationQuality;
    }

  private:
    void SetOption(CompositeOperator compositeOperator) noexcept
    {
      _compositeOperator = compositeOperator;
    }
    void SetOption(boo::BlendMode blendMode) noexcept
    {
      _blendMode = blendMode;
    }
    void SetOption(ImageOrientation orientation) noexcept
    {
      _orientation = orientation;
    }
    void SetOption(ImageOrientation::Orientation orientation) noexcept
    {
      _orientation = orientation;
    }
    void SetOption(boo::InterpolationQuality interpolationQuality) noexcept
    {
      _interpolationQuality = interpolationQuality;
    }
  };
}
