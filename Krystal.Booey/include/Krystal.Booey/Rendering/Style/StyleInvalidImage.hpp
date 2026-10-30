#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo
{
  /// @brief The image an image set falls back to when none of its candidates is usable.
  class StyleInvalidImage final : public StyleGeneratedImage
  {
  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleInvalidImage> Create() noexcept;
    ~StyleInvalidImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool Equals(const StyleInvalidImage &) const noexcept
    {
      return false;
    }

  private:
    StyleInvalidImage() noexcept;

    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final
    {
    }
    KRYS_NODISCARD bool IsPending() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleInvalidImage, IsInvalidImage)
