/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<ImageWrapper>::operator()(css::ValuePool &, const RenderStyle &style,
                                                             const ImageWrapper &value) noexcept
  {
    return value.Value->ComputedStyleValue(style);
  }

  // MARK: - Serialization

  void Serialiser<ImageWrapper>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                            const RenderStyle &style, const ImageWrapper &value) noexcept
  {
    builder.Append(value.Value->ComputedStyleValue(style)->CssText(context));
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
      return ImageWrapper {
        StyleCrossfadeImage::Create(krys::move(fromStyleImage), krys::move(toStyleImage), context.Progress)};
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
