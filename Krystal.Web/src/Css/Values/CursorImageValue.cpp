#include "Krystal.Web/Css/Values/CursorImageValue.hpp"
#include "Krystal.Core/Memory/PointerComparison.hpp"
#include "Krystal.Web/Css/Values/ImageValue.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/ValuePair.hpp"
#include "Krystal.Web/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Web/Style/Builder/BuilderState.hpp"

namespace krys::web::css
{
  CursorImageValue::CursorImageValue(Ref<Value> image, RefPtr<ValuePair> hotSpot,
                                     css::Url originalUrl) noexcept
      : Value(ValueType::CursorImage), _image(krys::move(image)), _hotSpot(krys::move(hotSpot)),
        _originalUrl(krys::move(originalUrl))
  {
  }

  CursorImageValue::~CursorImageValue() noexcept = default;

  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> image, RefPtr<ValuePair> hotSpot) noexcept
  {
    auto *imageValue = DynamicDowncast<ImageValue>(*image);
    auto originalUrl = imageValue ? imageValue->Url() : css::Url::None();
    return Create(krys::move(image), krys::move(hotSpot), krys::move(originalUrl));
  }

  Ref<CursorImageValue> CursorImageValue::Create(Ref<Value> image, RefPtr<ValuePair> hotSpot,
                                                 css::Url originalUrl) noexcept
  {
    return AdoptRef(*new CursorImageValue(krys::move(image), krys::move(hotSpot), krys::move(originalUrl)));
  }

  RefPtr<StyleCursorImage> CursorImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto styleImage = state.CreateStyleImage(*_image);
    if (!styleImage)
    {
      return nullptr;
    }

    Maybe<PointI> hotSpot;
    if (_hotSpot)
    {
      auto resolve = [&](const Value &coordinate)
      {
        // FIXME: The spec doesn't say how to make a pixel of a fractional or out-of-range coordinate;
        // truncating may not match other engines.
        return static_cast<int>(
          Downcast<PrimitiveValue>(coordinate).ResolveAsNumber(state.CssToLengthConversionData()));
      };
      hotSpot = PointI {resolve(_hotSpot->First()), resolve(_hotSpot->Second())};
    }
    return StyleCursorImage::Create(krys::move(styleImage), hotSpot,
                                    style::ConvertToStyle(_originalUrl, state));
  }

  CSSOMString CursorImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto text = _image->CssText(context);
    if (_hotSpot)
    {
      // Both coordinates, even when they're equal.
      text.push_back(u8' ');
      text.append(_hotSpot->First().CssText(context));
      text.push_back(u8' ');
      text.append(_hotSpot->Second().CssText(context));
    }
    return text;
  }

  bool CursorImageValue::Equals(const CursorImageValue &other) const noexcept
  {
    return *_image == *other._image && ArePointingToEqualData(_hotSpot, other._hotSpot);
  }

  IterationStatus CursorImageValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    if (func(*_image) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return _hotSpot ? func(*_hotSpot) : IterationStatus::Continue;
  }
}
