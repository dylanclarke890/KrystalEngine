#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPrimitiveList.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGStringList
  class SVGStringList final : public SVGPrimitiveList<dom::DOMString>
  {
    using Base = SVGPrimitiveList<dom::DOMString>;
    using Base::_items;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGStringList> Create(SVGPropertyOwner *owner) noexcept
    {
      return AdoptRef(*new SVGStringList(owner));
    }

    void Reset(dom::DOMStringView string) noexcept
    {
      Parse(string, u8' ');

      // Add empty string, if list is empty.
      if (_items.empty())
      {
        _items.push_back(dom::DOMString {});
      }
    }

    bool Parse(dom::DOMStringView, char8 delimiter) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;
  };
}
