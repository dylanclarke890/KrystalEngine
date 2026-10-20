#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/PlatformImage.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Color;

  /// @brief A decoded image frame the graphics context draws: the platform's image, identified for the
  /// renderer that caches it.
  // FIXME: Colour spaces other than sRGB, HDR headroom and rendering resource observers, once there are
  // any.
  class NativeImage : public RefCounted<NativeImage>, public CanMakeWeakPtr<NativeImage>
  {
    KRYS_NON_COPY_MOVABLE(NativeImage);

  protected:
    mutable PlatformImagePtr _platformImage;
    RenderingResourceIdentifier _renderingResourceIdentifier {RenderingResourceIdentifier::Generate()};

  public:
    KRYS_NODISCARD static RefPtr<NativeImage> Create(PlatformImagePtr &&platformImage) noexcept;
    /// @brief A native image that's drawn once or a few times.
    KRYS_NODISCARD static RefPtr<NativeImage> CreateTransient(PlatformImagePtr &&platformImage) noexcept;

    virtual ~NativeImage() noexcept;

    KRYS_NODISCARD virtual const PlatformImagePtr &GetPlatformImage() const noexcept;
    KRYS_NODISCARD virtual IntSize Size() const noexcept;
    KRYS_NODISCARD virtual bool HasAlpha() const noexcept;
    /// @brief The colour of a 1x1 image, which can be drawn as a fill.
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept;

    void ReplacePlatformImage(PlatformImagePtr &&platformImage) noexcept;

    KRYS_NODISCARD RenderingResourceIdentifier GetRenderingResourceIdentifier() const noexcept
    {
      return _renderingResourceIdentifier;
    }

  protected:
    explicit NativeImage(PlatformImagePtr &&platformImage) noexcept;
  };
}
