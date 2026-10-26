#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief A value that the DOM sees as a `TDecoration` (an enumeration's `unsigned short`) and that is kept
  /// as another type.
  template <typename TDecoration>
  class SVGDecoratedProperty : public RefCounted<SVGDecoratedProperty<TDecoration>>
  {
  public:
    SVGDecoratedProperty() noexcept = default;
    virtual ~SVGDecoratedProperty() noexcept = default;

    virtual void SetValueInternal(const TDecoration &) noexcept = 0;
    virtual bool SetValue(const TDecoration &value) noexcept
    {
      SetValueInternal(value);
      return true;
    }

    // Used internally. It doesn't check for HighestExposedEnumValue for example.
    KRYS_NODISCARD virtual TDecoration ValueInternal() const noexcept = 0;

    // Used by the DOM APIs.
    KRYS_NODISCARD virtual TDecoration Value() const noexcept
    {
      return ValueInternal();
    }

    KRYS_NODISCARD virtual dom::DOMString ValueAsString() const noexcept = 0;
    KRYS_NODISCARD virtual Ref<SVGDecoratedProperty<TDecoration>> Clone() noexcept = 0;
  };
}
