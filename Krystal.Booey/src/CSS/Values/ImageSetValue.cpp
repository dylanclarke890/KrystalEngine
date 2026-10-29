#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include <numeric>

namespace krys::boo::css
{
  Ref<ImageSetValue> ImageSetValue::Create(ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new ImageSetValue(krys::move(builder)));
  }

  ImageSetValue::ImageSetValue(ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::ImageSet, ValueSeparator::CommaSeparator, krys::move(builder))
  {
  }

  CSSOMString ImageSetValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {u8"image-set("};
    for (uint32 i = 0u; i < Length(); ++i)
    {
      if (i > 0u)
      {
        result.append(u8", ");
      }
      krys_debug_assert(Is<ImageSetOptionValue>(Item(i)));
      result.append(Item(i)->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> ImageSetValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto length = this->Length();

    List<ImageWithScale> images;
    images.reserve(length);
    for (uint32 i = 0; i < length; ++i)
    {
      const auto &option = Downcast<ImageSetOptionValue>(*Item(i));
      images.push_back(ImageWithScale {
        state.CreateStyleImage(option.Image()),
        option.Resolution().ResolveAsResolution<float>(state.CssToLengthConversionData()), option.Type()});
    }

    // Sort the images so that they are stored in order from lowest resolution to highest.
    // We want to maintain the authored order for serialization so we create a sorted indexing vector.
    List<size_t> sortedIndices(length);
    std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

    std::stable_sort(sortedIndices.begin(), sortedIndices.end(), [&images](size_t lhs, size_t rhs)
                     { return images[lhs].ScaleFactor < images[rhs].ScaleFactor; });

    return StyleImageSet::Create(krys::move(images), krys::move(sortedIndices));
  }
}
