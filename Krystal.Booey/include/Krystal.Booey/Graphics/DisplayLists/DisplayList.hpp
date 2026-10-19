#pragma once

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::display_list
{
  /// @brief Recorded drawing: the items a recorder appended, replayed into any graphics context.
  /// FIXME: The rendering resource observers (`addObserver`) come with the backend's resource caches.
  /// @note Not usable from multiple threads: the items' fonts aren't thread-safe.
  class DisplayList final : public ThreadSafeRefCounted<DisplayList>
  {
    KRYS_NON_COPYABLE(DisplayList);

  private:
    List<Item> _items;

  public:
    KRYS_NODISCARD static Ref<const DisplayList> Create(List<Item> &&items) noexcept
    {
      return AdoptRef(*new DisplayList(krys::move(items)));
    }

    ~DisplayList() noexcept;

    KRYS_NODISCARD Span<const Item> Items() const noexcept KRYS_LIFETIME_BOUND
    {
      return _items;
    }

    KRYS_NODISCARD utf8_string AsText(EnumSet<AsTextFlag>) const noexcept;
    void Dump(TextStream &) const noexcept;

  private:
    explicit DisplayList(List<Item> &&items) noexcept;
  };

  TextStream &operator<<(TextStream &, const DisplayList &) noexcept;
}
