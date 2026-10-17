#pragma once

#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A list of rectangles, such as an element's client rects.
  /// @see https://drafts.fxtf.org/geometry/#domrectlist
  class DOMRectList : public RefCounted<DOMRectList>
  {
  private:
    List<Ref<DOMRect>> _items;

    DOMRectList() noexcept = default;
    explicit DOMRectList(const List<FloatQuad> &quads) noexcept;
    explicit DOMRectList(const List<FloatRect> &rects) noexcept;

  public:
    ~DOMRectList() noexcept;

    /// @brief A list of the bounding boxes of @p quads.
    KRYS_NODISCARD static Ref<DOMRectList> Create(const List<FloatQuad> &quads) noexcept
    {
      return AdoptRef(*new DOMRectList(quads));
    }
    KRYS_NODISCARD static Ref<DOMRectList> Create(const List<FloatRect> &rects) noexcept
    {
      return AdoptRef(*new DOMRectList(rects));
    }
    KRYS_NODISCARD static Ref<DOMRectList> Create() noexcept
    {
      return AdoptRef(*new DOMRectList());
    }

    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_items.size());
    }

    KRYS_NODISCARD DOMRect *Item(uint32 index) const noexcept
    {
      return index < _items.size() ? _items[index].get() : nullptr;
    }

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < _items.size();
    }
  };
}
