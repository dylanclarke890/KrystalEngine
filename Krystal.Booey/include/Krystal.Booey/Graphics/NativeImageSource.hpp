#pragma once

#include "Krystal.Booey/Graphics/ImageSource.hpp"

namespace krys::boo
{
  /// @brief The source of an image made from a native image: one frame, already decoded.
  class NativeImageSource : public ImageSource
  {
  private:
    ImageFrame _frame;

  public:
    KRYS_NODISCARD static Ref<NativeImageSource> Create(Ref<NativeImage> &&) noexcept;

  private:
    explicit NativeImageSource(Ref<NativeImage> &&) noexcept;

    KRYS_NODISCARD IntSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return _frame.Size();
    }
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept final
    {
      return _frame.GetNativeImage()->SinglePixelSolidColor();
    }

    const ImageFrame &PrimaryImageFrame(const Maybe<SubsamplingLevel> & = null) noexcept final
    {
      return _frame;
    }
    KRYS_NODISCARD RefPtr<NativeImage> PrimaryNativeImage() noexcept final
    {
      return _frame.GetNativeImage();
    }

    void Dump(TextStream &) const noexcept final;
  };
}
