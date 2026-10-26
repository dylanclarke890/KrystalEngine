#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGLength
  class SVGLength : public SVGValueProperty<SVGLengthValue>
  {
    using Base = SVGValueProperty<SVGLengthValue>;
    using Base::_value;
    using Base::Base;

  public:
    enum : uint16
    {
      SVG_LENGTHTYPE_UNKNOWN = static_cast<uint16>(SVGLengthType::Unknown),
      SVG_LENGTHTYPE_NUMBER = static_cast<uint16>(SVGLengthType::Number),
      SVG_LENGTHTYPE_PERCENTAGE = static_cast<uint16>(SVGLengthType::Percentage),
      SVG_LENGTHTYPE_EMS = static_cast<uint16>(SVGLengthType::Ems),
      SVG_LENGTHTYPE_EXS = static_cast<uint16>(SVGLengthType::Exs),
      SVG_LENGTHTYPE_PX = static_cast<uint16>(SVGLengthType::Pixels),
      SVG_LENGTHTYPE_CM = static_cast<uint16>(SVGLengthType::Centimeters),
      SVG_LENGTHTYPE_MM = static_cast<uint16>(SVGLengthType::Millimeters),
      SVG_LENGTHTYPE_IN = static_cast<uint16>(SVGLengthType::Inches),
      SVG_LENGTHTYPE_PT = static_cast<uint16>(SVGLengthType::Points),
      SVG_LENGTHTYPE_PC = static_cast<uint16>(SVGLengthType::Picas)
    };

    KRYS_NODISCARD static Ref<SVGLength> Create() noexcept
    {
      return AdoptRef(*new SVGLength());
    }

    KRYS_NODISCARD static Ref<SVGLength> Create(const SVGLengthValue &value) noexcept
    {
      return AdoptRef(*new SVGLength(value));
    }

    KRYS_NODISCARD static Ref<SVGLength> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                                const SVGLengthValue &value = SVGLengthValue {}) noexcept
    {
      return AdoptRef(*new SVGLength(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGLength>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGLength(value.ReleaseValue()));
    }

    KRYS_NODISCARD Ref<SVGLength> Clone() const noexcept
    {
      return SVGLength::Create(_value);
    }

    KRYS_NODISCARD uint16 UnitType() const noexcept
    {
      // Per spec: https://svgwg.org/svg2-draft/types.html#__svg__SVGLength__SVG_LENGTHTYPE_UNKNOWN
      if (_value.LengthType() > SVGLengthType::Picas)
      {
        return 0;
      }
      return static_cast<uint16>(_value.LengthType());
    }

    KRYS_NODISCARD dom::ExceptionOr<float> ValueForBindings() noexcept;
    dom::ExceptionOr<void> SetValueForBindings(float) noexcept;

    KRYS_NODISCARD float ValueInSpecifiedUnits() noexcept
    {
      return _value.ValueInSpecifiedUnits();
    }

    dom::ExceptionOr<void> SetValueInSpecifiedUnits(float valueInSpecifiedUnits) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetValueInSpecifiedUnits(valueInSpecifiedUnits);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetValueAsString(dom::DOMStringView value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetValueAsString(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    dom::ExceptionOr<void> NewValueSpecifiedUnits(uint16 unitType, float valueInSpecifiedUnits) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      if (unitType == SVG_LENGTHTYPE_UNKNOWN || unitType > SVG_LENGTHTYPE_PC)
      {
        return dom::Exception {dom::ExceptionCode::NotSupportedError};
      }

      _value = {valueInSpecifiedUnits, static_cast<SVGLengthType>(unitType), _value.LengthMode()};
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> ConvertToSpecifiedUnits(uint16 unitType) noexcept;

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return _value.ValueAsString();
    }
  };
}
