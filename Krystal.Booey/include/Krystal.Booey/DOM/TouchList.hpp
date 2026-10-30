#pragma once

#include "Krystal.Booey/DOM/Touch.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A list of touches, as a touch event's `touches`, `targetTouches` and `changedTouches`.
  /// @see https://w3c.github.io/touch-events/#touchlist-interface
  class TouchList : public RefCounted<TouchList>
  {
  private:
    List<Ref<Touch>> _values;

  public:
    KRYS_NODISCARD static Ref<TouchList> Create() noexcept
    {
      return AdoptRef(*new TouchList);
    }

    KRYS_NODISCARD static Ref<TouchList> Create(List<Ref<Touch>> touches) noexcept
    {
      return AdoptRef(*new TouchList(krys::move(touches)));
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchlist-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_values.size());
    }

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchlist-item
    KRYS_NODISCARD Touch *Item(uint32 index) const noexcept;

    void Append(Ref<Touch> &&touch) noexcept
    {
      _values.push_back(krys::move(touch));
    }

  private:
    TouchList() noexcept = default;

    explicit TouchList(List<Ref<Touch>> touches) noexcept : _values(krys::move(touches))
    {
    }
  };
}
