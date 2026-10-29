#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  FilterImageValue::FilterImageValue(Ref<Value> &&imageValueOrNone, boo::css::Filter &&filter) noexcept
      : Value(ValueType::FilterImage), _imageValueOrNone(krys::move(imageValueOrNone)),
        _filter(krys::move(filter))
  {
  }

  Ref<FilterImageValue> FilterImageValue::Create(Ref<Value> &&imageValueOrNone,
                                                 boo::css::Filter &&filter) noexcept
  {
    return AdoptRef(*new FilterImageValue(krys::move(imageValueOrNone), krys::move(filter)));
  }

  bool FilterImageValue::Equals(const FilterImageValue &other) const noexcept
  {
    return EqualInputImages(other) && _filter == other._filter;
  }

  bool FilterImageValue::EqualInputImages(const FilterImageValue &other) const noexcept
  {
    return CompareValue(_imageValueOrNone, other._imageValueOrNone);
  }

  CSSOMString FilterImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    result.Append(u8"filter(");
    result.Append(_imageValueOrNone->CssText(context));
    result.Append(u8", ");
    Serialise(result, context, _filter);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  IterationStatus FilterImageValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    if (func(*_imageValueOrNone) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, _filter) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return IterationStatus::Continue;
  }

  RefPtr<StyleImage> FilterImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    return StyleFilterImage::Create(state.CreateStyleImage(*_imageValueOrNone),
                                    style::ConvertToStyle(_filter, state));
  }
}
