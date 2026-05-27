#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Element;

  /// @see https://dom.spec.whatwg.org/#interface-htmlcollection
  class HTMLCollection : public RefCounted<HTMLCollection>, public CanMakeWeakPtr<HTMLCollection>
  {
  public:
    virtual ~HTMLCollection() noexcept = default;

#pragma region HTMLCollection - https://dom.spec.whatwg.org/#interface-htmlcollection

    KRYS_NODISCARD virtual RefPtr<Element> Item(size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<const Element> Item(size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<Element> operator[](size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<const Element> operator[](size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<Element> NamedItem(DOMStringView name) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<const Element> NamedItem(DOMStringView name) const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<Element> operator[](DOMStringView name) noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<const Element> operator[](DOMStringView name) const noexcept = 0;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

  protected:
    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }
#pragma endregion
  };
}