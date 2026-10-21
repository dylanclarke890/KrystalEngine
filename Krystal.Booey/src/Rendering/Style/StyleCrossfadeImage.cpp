#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleCrossfadeImage::StyleCrossfadeImage(RefPtr<StyleImage> &&from, RefPtr<StyleImage> &&to,
                                           double percentage, bool isPrefixed) noexcept
      : StyleGeneratedImage {Type::CrossfadeImage, StyleCrossfadeImage::IsFixedSize},
        _from {krys::move(from)}, _to {krys::move(to)}, _percentage {percentage}, _isPrefixed {isPrefixed}
  {
  }

  StyleCrossfadeImage::~StyleCrossfadeImage() noexcept = default;

  bool StyleCrossfadeImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCrossfadeImage = DynamicDowncast<StyleCrossfadeImage>(other);
    return otherCrossfadeImage && Equals(*otherCrossfadeImage);
  }

  bool StyleCrossfadeImage::Equals(const StyleCrossfadeImage &other) const noexcept
  {
    return EqualInputImages(other) && _percentage == other._percentage;
  }

  bool StyleCrossfadeImage::EqualInputImages(const StyleCrossfadeImage &other) const noexcept
  {
    return ArePointingToEqualData(_from, other._from) && ArePointingToEqualData(_to, other._to);
  }

  RefPtr<StyleCrossfadeImage> StyleCrossfadeImage::Blend(KRYS_MAYBE_UNUSED const StyleCrossfadeImage &from,
                                                         const BlendingContext &) const noexcept
  {
    krys_debug_assert(EqualInputImages(from));

    // FIXME: Blend the percentages once both input images have loaded (their cached images), once the
    // resource loader is ported. Until then, they never have.
    return nullptr;
  }

  Ref<css::Value> StyleCrossfadeImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    auto fromComputedValue = _from ? _from->ComputedStyleValue(style)
                                   : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)};
    auto toComputedValue = _to ? _to->ComputedStyleValue(style)
                               : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)};
    return css::CrossFadeValue::Create(krys::move(fromComputedValue), krys::move(toComputedValue),
                                       css::PrimitiveValue::Create(_percentage), _isPrefixed);
  }

  void StyleCrossfadeImage::Load(CachedResourceLoader &loader, const ResourceLoaderOptions &options) noexcept
  {
    // FIXME: Follow the input images' cached images as their client (imageChanged), and blend once they're
    // ready.
    if (_from && _from->IsPending())
    {
      _from->Load(loader, options);
    }

    if (_to && _to->IsPending())
    {
      _to->Load(loader, options);
    }
  }

  bool StyleCrossfadeImage::IsPending() const noexcept
  {
    if (_from && _from->IsPending())
    {
      return true;
    }

    if (_to && _to->IsPending())
    {
      return true;
    }

    return false;
  }
}
