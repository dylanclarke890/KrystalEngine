#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief One of the candidate images of an image set or cursor, with its resolution.
  struct ImageWithScale
  {
    RefPtr<StyleImage> Image {StyleInvalidImage::Create()};
    float ScaleFactor {1};
    Maybe<css::CSSOMString> MimeType {};

    KRYS_NODISCARD bool operator==(const ImageWithScale &other) const noexcept
    {
      return Image == other.Image && ScaleFactor == other.ScaleFactor;
    }
  };

  /// @brief An image that selects one of several candidates when it loads.
  /// FIXME: `load` (which selects the image) and the forwarding of the renderer-facing members to the
  /// selected image come with the resource loader and the renderer.
  class StyleMultiImage : public StyleImage
  {
  private:
    RefPtr<StyleImage> _selectedImage;
    bool _isPending {true};

  public:
    ~StyleMultiImage() noexcept override;

  protected:
    explicit StyleMultiImage(Type) noexcept;

    KRYS_NODISCARD bool Equals(const StyleMultiImage &other) const noexcept;

    KRYS_NODISCARD virtual ImageWithScale SelectBestFitImage(const dom::Document &) noexcept = 0;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept final;

  private:
    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    KRYS_NODISCARD bool ErrorOccurred() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final;
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final;
    KRYS_NODISCARD float ImageScaleFactor() const noexcept final;

    KRYS_NODISCARD const StyleImage *SelectedImage() const noexcept final
    {
      return _selectedImage.get();
    }
    KRYS_NODISCARD StyleImage *SelectedImage() noexcept final
    {
      return _selectedImage.get();
    }
  };
}
