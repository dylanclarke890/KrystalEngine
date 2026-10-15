#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<ImageWrapper>::operator()(css::ValuePool &, const RenderStyle &style,
                                                             const ImageWrapper &value) noexcept
  {
    return value.Value->ComputedStyleValue(style);
  }

  // MARK: - Serialization

  void Serialiser<ImageWrapper>::operator()(css::CSSOMString &builder,
                                            const css::SerialisationContext &context,
                                            const RenderStyle &style, const ImageWrapper &value) noexcept
  {
    builder.append(value.Value->ComputedStyleValue(style)->CssText(context));
  }

  // MARK: - Blending

  namespace
  {
    KRYS_NODISCARD ImageWrapper CrossfadeBlend(Ref<StyleCachedImage> &&fromStyleImage,
                                               Ref<StyleCachedImage> &&toStyleImage,
                                               const BlendingContext &context) noexcept
    {
      // If progress is at one of the extremes, we want getComputedStyle to show the image,
      // not a completed cross-fade, so we hand back one of the existing images.
      if (!context.Progress)
      {
        return ImageWrapper {krys::move(fromStyleImage)};
      }
      if (context.Progress == 1)
      {
        return ImageWrapper {krys::move(toStyleImage)};
      }
      if (!fromStyleImage->GetCachedImage() || !toStyleImage->GetCachedImage())
      {
        return ImageWrapper {krys::move(toStyleImage)};
      }
      return ImageWrapper {StyleCrossfadeImage::Create(krys::move(fromStyleImage), krys::move(toStyleImage),
                                                       context.Progress, false)};
    }

    KRYS_NODISCARD ImageWrapper FilterBlend(RefPtr<StyleImage> inputImage, const style::Filter &from,
                                            const style::Filter &to, const BlendingContext &context) noexcept
    {
      return ImageWrapper {StyleFilterImage::Create(krys::move(inputImage),
                                                    Blending<style::Filter> {}.Blend(from, to, context))};
    }
  }

  ImageWrapper Blending<ImageWrapper>::Blend(const ImageWrapper &a, const ImageWrapper &b,
                                             const BlendingContext &context) noexcept
  {
    if (!context.Progress)
    {
      return a;
    }

    if (context.Progress == 1.0)
    {
      return b;
    }

    RefPtr<StyleImage> aSelectedUnchecked = ShareRefPtr(a.Value->SelectedImage());
    RefPtr<StyleImage> bSelectedUnchecked = ShareRefPtr(b.Value->SelectedImage());
    if (!aSelectedUnchecked || !bSelectedUnchecked)
    {
      if (aSelectedUnchecked)
      {
        return ImageWrapper {Ref<StyleImage> {krys::move(aSelectedUnchecked)}};
      }
      if (bSelectedUnchecked)
      {
        return ImageWrapper {Ref<StyleImage> {krys::move(bSelectedUnchecked)}};
      }
      return context.Progress > 0.5 ? b : a;
    }

    Ref<StyleImage> aSelected {krys::move(aSelectedUnchecked)};
    Ref<StyleImage> bSelected {krys::move(bSelectedUnchecked)};

    // Interpolation between two generated images. Cross fade for all other cases.
    const auto *aFilter = DynamicDowncast<StyleFilterImage>(*aSelected);
    const auto *bFilter = DynamicDowncast<StyleFilterImage>(*bSelected);
    const auto *aCrossfade = DynamicDowncast<StyleCrossfadeImage>(*aSelected);
    const auto *bCrossfade = DynamicDowncast<StyleCrossfadeImage>(*bSelected);
    auto *aCachedImage = DynamicDowncast<StyleCachedImage>(*aSelected);
    auto *bCachedImage = DynamicDowncast<StyleCachedImage>(*bSelected);

    if (aFilter && bFilter)
    {
      // Interpolation of generated images is only possible if the input images are equal.
      // Otherwise fall back to cross fade animation.
      if (aFilter->EqualInputImages(*bFilter) && aFilter->InputImage()
          && Is<StyleCachedImage>(*aFilter->InputImage()))
      {
        return FilterBlend(aFilter->InputImage(), aFilter->GetFilter(), bFilter->GetFilter(), context);
      }
    }
    else if (aCrossfade && bCrossfade)
    {
      if (aCrossfade->EqualInputImages(*bCrossfade))
      {
        if (auto crossfadeBlend = bCrossfade->Blend(*aCrossfade, context))
        {
          return ImageWrapper {Ref<StyleImage> {krys::move(crossfadeBlend)}};
        }
      }
    }
    else if (aFilter && bCachedImage)
    {
      auto aFilterInputImage = aFilter->InputImage();
      const auto *aFilterCachedInputImage =
        aFilterInputImage ? DynamicDowncast<StyleCachedImage>(*aFilterInputImage) : nullptr;
      if (aFilterCachedInputImage && bCachedImage->Equals(*aFilterCachedInputImage))
      {
        return FilterBlend(krys::move(aFilterInputImage), aFilter->GetFilter(),
                           style::Filter {css::keywords::None {}}, context);
      }
    }
    else if (aCachedImage && bFilter)
    {
      auto bFilterInputImage = bFilter->InputImage();
      const auto *bFilterCachedInputImage =
        bFilterInputImage ? DynamicDowncast<StyleCachedImage>(*bFilterInputImage) : nullptr;
      if (bFilterCachedInputImage && aCachedImage->Equals(*bFilterCachedInputImage))
      {
        return FilterBlend(krys::move(bFilterInputImage), style::Filter {css::keywords::None {}},
                           bFilter->GetFilter(), context);
      }
    }

    if (aCachedImage && bCachedImage)
    {
      return CrossfadeBlend(ShareRef(*aCachedImage), ShareRef(*bCachedImage), context);
    }

    // FIXME: Add support for interpolation between two *gradient() functions.
    // https://bugs.webkit.org/show_bug.cgi?id=119956
    // FIXME: Add support cross fade between cached and generated images.
    // https://bugs.webkit.org/show_bug.cgi?id=78293

    return ImageWrapper {krys::move(bSelected)};
  }
}
