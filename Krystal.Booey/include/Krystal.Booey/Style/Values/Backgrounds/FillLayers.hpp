#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundSize.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/RepeatStyle.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueList.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include <ranges>

namespace krys::boo
{
  class RenderElement;

  namespace style
  {
    // Utilities for working with the BackgroundLayers and MaskLayers types.
    // FIXME: `imagesAreLoaded` and `hasHDRContent` come with the resource loader.

    template <typename T>
    concept FillLayer = CoordinatedValueListValue<T> && requires(T value) {
      { value.GetImage() } -> std::same_as<const ImageOrNone &>;
      { value.HasImage() } -> std::same_as<bool>;
      { value.GetSize() } -> std::same_as<const BackgroundSize &>;
      { value.GetAttachment() } -> std::same_as<FillAttachment>;
      { value.GetClip() } -> std::same_as<FillBox>;
      { value.SetClipMax(std::declval<FillBox>()) };
    };

    template <FillLayer T>
    void ComputeClipMax(const CoordinatedValueList<T> &list) noexcept
    {
      auto computedClipMax = FillBox::NoClip;
      for (auto &layer : list.UsedValues() | std::views::reverse)
      {
        computedClipMax = ClipMax(computedClipMax, layer.GetClip());
        layer.SetClipMax(computedClipMax);
      }
    }

    template <FillLayer T>
    KRYS_NODISCARD bool HasImageInAnyLayer(const CoordinatedValueList<T> &list) noexcept
    {
      return std::ranges::any_of(list.UsedValues(), [](auto &layer) { return layer.HasImage(); });
    }

    template <FillLayer T>
    KRYS_NODISCARD bool HasImageWithAttachment(const CoordinatedValueList<T> &list,
                                               FillAttachment attachment) noexcept
    {
      return std::ranges::any_of(list.UsedValues(), [&attachment](auto &layer)
                                 { return layer.HasImage() && layer.GetAttachment() == attachment; });
    }

    template <FillLayer T>
    KRYS_NODISCARD bool HasEntirelyFixedBackground(const CoordinatedValueList<T> &list) noexcept
    {
      return std::ranges::all_of(
        list.UsedValues(), [](auto &layer)
        { return layer.HasImage() && layer.GetAttachment() == FillAttachment::FixedBackground; });
    }

    template <FillLayer T>
    KRYS_NODISCARD bool HasAnyBackgroundClipText(const CoordinatedValueList<T> &list) noexcept
    {
      return std::ranges::any_of(list.UsedValues(),
                                 [](auto &layer) { return layer.GetClip() == FillBox::Text; });
    }

    template <FillLayer T>
    KRYS_NODISCARD RefPtr<StyleImage> FindLayerUsedImage(const CoordinatedValueList<T> &list,
                                                         WrappedImagePtr image, bool &isNonEmpty) noexcept
    {
      for (auto &layer : list.UsedValues())
      {
        auto layerImage = layer.GetImage().TryStyleImage();
        if (!layerImage || layerImage->Data() != image)
        {
          continue;
        }

        // FIXME: This really needs to compute the tile rect with BackgroundPainter::calculateFillTileSize().
        isNonEmpty = krys::SwitchOn(
          layer.GetSize(), [&](const css::keywords::Cover &) { return false; },
          [&](const css::keywords::Contain &) { return false; },
          [&](const style::BackgroundSize::LengthSize &size)
          {
            auto isAutoOrKnownNonZero = [](auto &length)
            {
              return krys::SwitchOn(
                length, [](const style::BackgroundSizeLength::Fixed &fixed) { return !fixed.IsZero(); },
                [](const style::BackgroundSizeLength::Percentage &percentage)
                { return !percentage.IsZero(); }, [](const style::BackgroundSizeLength::Calc &)
                { return false; }, [](const css::keywords::Auto &) { return true; });
            };
            return isAutoOrKnownNonZero(size.Width()) && isAutoOrKnownNonZero(size.Height());
          });
        return layerImage;
      }

      isNonEmpty = false;
      return nullptr;
    }

  }
}
