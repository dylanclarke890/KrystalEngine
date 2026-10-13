#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleCursorImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief One image of the `cursor` property, with an optional hot spot.
  class CursorImageValue final : public Value
  {
  private:
    CSSURL _originalURL;
    const Ref<Value> _imageValue;
    const RefPtr<Value> _hotSpot;

    CursorImageValue(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot, CSSURL &&originalURL) noexcept;

  public:
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> &&imageValue,
                                                       RefPtr<Value> &&hotSpot) noexcept;
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> &&imageValue, RefPtr<Value> &&hotSpot,
                                                       CSSURL &&originalURL) noexcept;

    KRYS_NODISCARD const CSSURL &OriginalURL() const noexcept
    {
      return _originalURL;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const CursorImageValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      return func(*_imageValue);
    }

    // FIXME: not defined until style::BuilderState and StyleCursorImage are ported.
    KRYS_NODISCARD RefPtr<StyleCursorImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CursorImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCursorImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
