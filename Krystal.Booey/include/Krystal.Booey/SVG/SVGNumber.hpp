#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGNumber
  class SVGNumber : public SVGValueProperty<float>
  {
    using Base = SVGValueProperty<float>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGNumber> Create(float value = 0) noexcept
    {
      return AdoptRef(*new SVGNumber(value));
    }

    KRYS_NODISCARD static Ref<SVGNumber> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                                float value = 0) noexcept
    {
      return AdoptRef(*new SVGNumber(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGNumber>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGNumber(value.ReleaseValue()));
    }

    KRYS_NODISCARD Ref<SVGNumber> Clone() const noexcept
    {
      return SVGNumber::Create(_value);
    }

    KRYS_NODISCARD float ValueForBindings() noexcept
    {
      return _value;
    }

    dom::ExceptionOr<void> SetValueForBindings(float value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value = value;
      CommitChange();
      return {};
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return SVGPropertyTraits<float>::ToString(_value);
    }
  };
}
