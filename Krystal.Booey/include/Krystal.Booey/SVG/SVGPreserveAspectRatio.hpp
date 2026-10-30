#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/coords.html#InterfaceSVGPreserveAspectRatio
  class SVGPreserveAspectRatio : public SVGValueProperty<SVGPreserveAspectRatioValue>
  {
    using Base = SVGValueProperty<SVGPreserveAspectRatioValue>;
    using Base::_value;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGPreserveAspectRatio>
      Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
             const SVGPreserveAspectRatioValue &value = {}) noexcept
    {
      return AdoptRef(*new SVGPreserveAspectRatio(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGPreserveAspectRatio>>
      Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return AdoptRef(*new SVGPreserveAspectRatio(value.ReleaseValue()));
    }

    KRYS_NODISCARD uint16 Align() const noexcept
    {
      return _value.Align();
    }

    dom::ExceptionOr<void> SetAlign(uint16 value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetAlign(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    KRYS_NODISCARD uint16 MeetOrSlice() const noexcept
    {
      return _value.MeetOrSlice();
    }

    dom::ExceptionOr<void> SetMeetOrSlice(uint16 value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      auto result = _value.SetMeetOrSlice(value);
      if (result.HasException())
      {
        return result;
      }

      CommitChange();
      return result;
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return _value.ValueAsString();
    }
  };
}
