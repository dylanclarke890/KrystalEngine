#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class GraphicsContext;
  class RenderElement;
  class RenderStyle;

  /// @brief An opaque pointer identifying the image data behind a style image.
  using WrappedImagePtr = const void *;

  /// @brief The computed value of an `<image>`.
  /// FIXME: Loading (`load`) and clients (`addClient`, `removeClient`, `hasClient`) come with the resource
  /// loader. Sizing against a renderer and drawing are pure upstream; here they default to an image of no
  /// size that draws nothing, until loaded images implement them.
  class StyleImage : public RefCounted<StyleImage>, public CanMakeWeakPtr<StyleImage>
  {
  protected:
    enum class Type : uint8
    {
      CachedImage,
      CursorImage,
      ImageSet,
      FilterImage,
      CanvasImage,
      CrossfadeImage,
      GradientImage,
      NamedImage,
      InvalidImage,
      PaintImage,
    };

  private:
    Type _type;

  public:
    virtual ~StyleImage() noexcept = default;

    KRYS_NODISCARD bool operator==(const StyleImage &other) const noexcept
    {
      return Equals(other);
    }

    KRYS_NODISCARD virtual bool Equals(const StyleImage &other) const noexcept = 0;

    // Computed Style representation.
    KRYS_NODISCARD virtual Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept = 0;

    // Opaque representation.
    KRYS_NODISCARD virtual WrappedImagePtr Data() const noexcept = 0;

    // Loading.
    KRYS_NODISCARD virtual bool IsPending() const noexcept = 0;
    KRYS_NODISCARD virtual bool ErrorOccurred() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool UsesDataProtocol() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool HasImage() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual style::URL Url() const noexcept
    {
      return {};
    }

    // Size / scale.
    KRYS_NODISCARD virtual bool UsesImageContainerSize() const noexcept = 0;
    KRYS_NODISCARD virtual bool ImageHasRelativeWidth() const noexcept = 0;
    KRYS_NODISCARD virtual bool ImageHasRelativeHeight() const noexcept = 0;
    KRYS_NODISCARD virtual float ImageScaleFactor() const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual bool ImageHasNaturalDimensions() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool IsLoaded(const RenderElement *) const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual FloatSize ImageSize(const RenderElement *, float /* multiplier */) const noexcept
    {
      return {};
    }
    virtual void ComputeIntrinsicDimensions(const RenderElement *, float &intrinsicWidth,
                                            float &intrinsicHeight, FloatSize &intrinsicRatio) noexcept
    {
      intrinsicWidth = 0;
      intrinsicHeight = 0;
      intrinsicRatio = {};
    }

    // Image.
    KRYS_NODISCARD virtual RefPtr<Image> GetImage(const RenderElement *, const FloatSize &,
                                                  const GraphicsContext & /* destinationContext */,
                                                  bool /* isForFirstLine */ = false) const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual StyleImage *SelectedImage() noexcept
    {
      return this;
    }
    KRYS_NODISCARD virtual const StyleImage *SelectedImage() const noexcept
    {
      return this;
    }
    KRYS_NODISCARD virtual CachedImage *GetCachedImage() const noexcept
    {
      return nullptr;
    }

    // Rendering.
    KRYS_NODISCARD virtual bool CanRender(const RenderElement *, float /* multiplier */) const noexcept
    {
      return true;
    }
    virtual void SetContainerContextForRenderer(const RenderElement &, const FloatSize &, float) noexcept
    {
    }
    KRYS_NODISCARD virtual bool KnownToBeOpaque(const RenderElement &) const noexcept
    {
      return false;
    }

    // Derived type.
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsCachedImage() const noexcept
    {
      return _type == Type::CachedImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsCursorImage() const noexcept
    {
      return _type == Type::CursorImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsImageSet() const noexcept
    {
      return _type == Type::ImageSet;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsGeneratedImage() const noexcept
    {
      return IsFilterImage() || IsCanvasImage() || IsCrossfadeImage() || IsGradientImage() || IsNamedImage()
             || IsPaintImage() || IsInvalidImage();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsFilterImage() const noexcept
    {
      return _type == Type::FilterImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsCanvasImage() const noexcept
    {
      return _type == Type::CanvasImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsCrossfadeImage() const noexcept
    {
      return _type == Type::CrossfadeImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsGradientImage() const noexcept
    {
      return _type == Type::GradientImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsNamedImage() const noexcept
    {
      return _type == Type::NamedImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsPaintImage() const noexcept
    {
      return _type == Type::PaintImage;
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsInvalidImage() const noexcept
    {
      return _type == Type::InvalidImage;
    }

    KRYS_NODISCARD bool HasCachedImage() const noexcept
    {
      return _type == Type::CachedImage || SelectedImage()->IsCachedImage();
    }

  protected:
    explicit StyleImage(Type type) noexcept : _type {type}
    {
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(ToClassName, predicate)                                      \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToClassName)                                             \
    KRYS_NODISCARD static bool IsType(const krys::boo::StyleImage &image) noexcept                           \
    {                                                                                                        \
      return image.predicate();                                                                              \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
