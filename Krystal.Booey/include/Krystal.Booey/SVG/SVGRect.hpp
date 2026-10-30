#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"

namespace krys::boo::svg
{
  /// @brief The deprecated alias of `DOMRect` that SVG's interfaces still use.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGAnimatedRect
  class SVGRect : public SVGValueProperty<FloatRect>
  {
    using Base = SVGValueProperty<FloatRect>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGRect> Create(const FloatRect &value = {}) noexcept
    {
      return AdoptRef(*new SVGRect(value));
    }

    KRYS_NODISCARD static Ref<SVGRect> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                              const FloatRect &value = {}) noexcept
    {
      return AdoptRef(*new SVGRect(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGRect>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGRect(value.ReleaseValue()));
    }

    KRYS_NODISCARD float X() noexcept
    {
      return _value.X();
    }

    dom::ExceptionOr<void> SetX(float xValue) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetX(xValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Y() noexcept
    {
      return _value.Y();
    }

    dom::ExceptionOr<void> SetY(float yValue) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetY(yValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Width() noexcept
    {
      return _value.Width();
    }

    dom::ExceptionOr<void> SetWidth(float widthValue) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetWidth(widthValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD float Height() noexcept
    {
      return _value.Height();
    }

    dom::ExceptionOr<void> SetHeight(float heightValue) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetHeight(heightValue);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<FloatRect>::ToString(_value);
    }
  };
}
