#include "Krystal.Booey/SVG/SVGAngleValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  float SVGAngleValue::Value() const noexcept
  {
    switch (_unitType)
    {
      case SVG_ANGLETYPE_GRAD:
      {
        return grad2deg(_valueInSpecifiedUnits);
      }
      case SVG_ANGLETYPE_RAD:
      {
        return rad2deg(_valueInSpecifiedUnits);
      }
      case SVG_ANGLETYPE_TURN:
      {
        return turn2deg(_valueInSpecifiedUnits);
      }
      case SVG_ANGLETYPE_UNSPECIFIED:
      case SVG_ANGLETYPE_UNKNOWN:
      case SVG_ANGLETYPE_DEG:
      {
        return _valueInSpecifiedUnits;
      }
    }
    krys_assert_msg(false, "Not reached");
    return 0;
  }

  void SVGAngleValue::SetValue(float value) noexcept
  {
    switch (_unitType)
    {
      case SVG_ANGLETYPE_GRAD:
      {
        _valueInSpecifiedUnits = deg2grad(value);
        return;
      }
      case SVG_ANGLETYPE_RAD:
      {
        _valueInSpecifiedUnits = deg2rad(value);
        return;
      }
      case SVG_ANGLETYPE_TURN:
      {
        _valueInSpecifiedUnits = deg2turn(value);
        return;
      }
      case SVG_ANGLETYPE_UNSPECIFIED:
      case SVG_ANGLETYPE_UNKNOWN:
      case SVG_ANGLETYPE_DEG:
      {
        _valueInSpecifiedUnits = value;
        return;
      }
    }
    krys_assert_msg(false, "Not reached");
  }

  dom::DOMString SVGAngleValue::ValueAsString() const noexcept
  {
    StringBuilder builder;
    switch (_unitType)
    {
      case SVG_ANGLETYPE_DEG:
      {
        builder.Append(_valueInSpecifiedUnits, u8"deg");
        break;
      }
      case SVG_ANGLETYPE_RAD:
      {
        builder.Append(_valueInSpecifiedUnits, u8"rad");
        break;
      }
      case SVG_ANGLETYPE_TURN:
      {
        builder.Append(_valueInSpecifiedUnits, u8"turn");
        break;
      }
      case SVG_ANGLETYPE_GRAD:
      {
        builder.Append(_valueInSpecifiedUnits, u8"grad");
        break;
      }
      case SVG_ANGLETYPE_UNSPECIFIED:
      case SVG_ANGLETYPE_UNKNOWN:
      {
        builder.Append(_valueInSpecifiedUnits);
        break;
      }
    }
    return krys::move(builder).ToString();
  }

  namespace
  {
    KRYS_NODISCARD SVGAngleValue::Type ParseAngleType(dom::DOMStringView buffer) noexcept
    {
      switch (buffer.size())
      {
        case 0:
        {
          return SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED;
        }
        case 3:
        {
          if (buffer == u8"deg")
          {
            return SVGAngleValue::SVG_ANGLETYPE_DEG;
          }
          if (buffer == u8"rad")
          {
            return SVGAngleValue::SVG_ANGLETYPE_RAD;
          }
          break;
        }
        case 4:
        {
          if (buffer == u8"grad")
          {
            return SVGAngleValue::SVG_ANGLETYPE_GRAD;
          }
          if (buffer == u8"turn")
          {
            return SVGAngleValue::SVG_ANGLETYPE_TURN;
          }
          break;
        }
      }
      return SVGAngleValue::SVG_ANGLETYPE_UNKNOWN;
    }
  }

  dom::ExceptionOr<void> SVGAngleValue::SetValueAsString(dom::DOMStringView value) noexcept
  {
    if (value.empty())
    {
      _unitType = SVG_ANGLETYPE_UNSPECIFIED;
      return {};
    }

    auto buffer = value;
    auto valueInSpecifiedUnits = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);
    if (!valueInSpecifiedUnits)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    auto unitType = ParseAngleType(buffer);
    if (unitType == SVGAngleValue::SVG_ANGLETYPE_UNKNOWN)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    _unitType = unitType;
    _valueInSpecifiedUnits = *valueInSpecifiedUnits;
    return {};
  }

  dom::ExceptionOr<void> SVGAngleValue::NewValueSpecifiedUnits(uint16 unitType,
                                                               float valueInSpecifiedUnits) noexcept
  {
    if (unitType == SVG_ANGLETYPE_UNKNOWN || unitType > SVG_ANGLETYPE_GRAD)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    _unitType = static_cast<Type>(unitType);
    _valueInSpecifiedUnits = valueInSpecifiedUnits;
    return {};
  }

  dom::ExceptionOr<void> SVGAngleValue::ConvertToSpecifiedUnits(uint16 unitType) noexcept
  {
    if (unitType == SVG_ANGLETYPE_UNKNOWN || _unitType == SVG_ANGLETYPE_UNKNOWN
        || unitType > SVG_ANGLETYPE_GRAD)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (unitType == _unitType)
    {
      return {};
    }

    switch (_unitType)
    {
      case SVG_ANGLETYPE_TURN:
      {
        switch (unitType)
        {
          case SVG_ANGLETYPE_GRAD:
          {
            _valueInSpecifiedUnits = turn2grad(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_UNSPECIFIED:
          case SVG_ANGLETYPE_DEG:
          {
            _valueInSpecifiedUnits = turn2deg(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_RAD:
          {
            _valueInSpecifiedUnits = deg2rad(turn2deg(_valueInSpecifiedUnits));
            break;
          }
          case SVG_ANGLETYPE_TURN:
          case SVG_ANGLETYPE_UNKNOWN:
          {
            krys_assert_msg(false, "Not reached");
            break;
          }
        }
        break;
      }
      case SVG_ANGLETYPE_RAD:
      {
        switch (unitType)
        {
          case SVG_ANGLETYPE_GRAD:
          {
            _valueInSpecifiedUnits = rad2grad(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_UNSPECIFIED:
          case SVG_ANGLETYPE_DEG:
          {
            _valueInSpecifiedUnits = rad2deg(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_TURN:
          {
            _valueInSpecifiedUnits = deg2turn(rad2deg(_valueInSpecifiedUnits));
            break;
          }
          case SVG_ANGLETYPE_RAD:
          case SVG_ANGLETYPE_UNKNOWN:
          {
            krys_assert_msg(false, "Not reached");
            break;
          }
        }
        break;
      }
      case SVG_ANGLETYPE_GRAD:
      {
        switch (unitType)
        {
          case SVG_ANGLETYPE_RAD:
          {
            _valueInSpecifiedUnits = grad2rad(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_UNSPECIFIED:
          case SVG_ANGLETYPE_DEG:
          {
            _valueInSpecifiedUnits = grad2deg(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_TURN:
          {
            _valueInSpecifiedUnits = grad2turn(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_GRAD:
          case SVG_ANGLETYPE_UNKNOWN:
          {
            krys_assert_msg(false, "Not reached");
            break;
          }
        }
        break;
      }
      case SVG_ANGLETYPE_UNSPECIFIED:
        // Spec: For angles, a unitless value is treated the same as if degrees were specified.
      case SVG_ANGLETYPE_DEG:
      {
        switch (unitType)
        {
          case SVG_ANGLETYPE_RAD:
          {
            _valueInSpecifiedUnits = deg2rad(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_GRAD:
          {
            _valueInSpecifiedUnits = deg2grad(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_TURN:
          {
            _valueInSpecifiedUnits = deg2turn(_valueInSpecifiedUnits);
            break;
          }
          case SVG_ANGLETYPE_UNSPECIFIED:
          case SVG_ANGLETYPE_DEG:
          {
            break;
          }
          case SVG_ANGLETYPE_UNKNOWN:
          {
            krys_assert_msg(false, "Not reached");
            break;
          }
        }
        break;
      }
      case SVG_ANGLETYPE_UNKNOWN:
      {
        krys_assert_msg(false, "Not reached");
        break;
      }
    }

    _unitType = static_cast<Type>(unitType);

    return {};
  }
}
