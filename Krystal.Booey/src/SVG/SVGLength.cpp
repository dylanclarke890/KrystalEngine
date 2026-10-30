#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo::svg
{
  dom::ExceptionOr<float> SVGLength::ValueForBindings() noexcept
  {
    RefPtr element = ShareRefPtr(ContextElement());

    // Ensure style is up-to-date for font-relative units (e.g., ch).
    // https://www.w3.org/TR/css-values-3/#absolute-lengths
    auto requiresStyleUpdate = [](SVGLengthType type)
    {
      switch (type)
      {
        case SVGLengthType::Number:
        case SVGLengthType::Pixels:
        case SVGLengthType::Centimeters:
        case SVGLengthType::Millimeters:
        case SVGLengthType::Inches:
        case SVGLengthType::Points:
        case SVGLengthType::Picas:       return false; // Absolute units don't need style update.
        default:                         return true;                          // Font-relative units need style update.
      }
    };

    if (requiresStyleUpdate(_value.LengthType()))
    {
      if (element)
      {
        element->NodeDocument().UpdateStyleIfNeeded();
      }
    }

    return _value.ValueForBindings(SVGLengthContext {element.get()});
  }

  dom::ExceptionOr<void> SVGLength::SetValueForBindings(float value) noexcept
  {
    if (IsReadOnly())
    {
      return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
    }

    auto result = _value.SetValue(SVGLengthContext {RefPtr {ShareRefPtr(ContextElement())}.get()}, value);
    if (result.HasException())
    {
      return result;
    }

    CommitChange();
    return result;
  }

  dom::ExceptionOr<void> SVGLength::ConvertToSpecifiedUnits(uint16 unitType) noexcept
  {
    if (IsReadOnly())
    {
      return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
    }

    if (unitType == SVG_LENGTHTYPE_UNKNOWN || unitType > SVG_LENGTHTYPE_PC)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    auto result = _value.ConvertToSpecifiedUnits(
      SVGLengthContext {RefPtr {ShareRefPtr(ContextElement())}.get()}, static_cast<SVGLengthType>(unitType));
    if (result.HasException())
    {
      return result;
    }

    CommitChange();
    return result;
  }
}
