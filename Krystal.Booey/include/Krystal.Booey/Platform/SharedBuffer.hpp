#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief Immutable, shared bytes: a resource's data, or what of it has arrived.
  // FIXME: The fragmented buffer (FragmentedSharedBuffer, its data segments and builder), file and memory
  // mapped data, once loading streams data.
  class SharedBuffer final : public RefCounted<SharedBuffer>
  {
    KRYS_NON_COPY_MOVABLE(SharedBuffer);

  private:
    List<uint8> _data;

    explicit SharedBuffer(List<uint8> &&data) noexcept : _data(krys::move(data))
    {
    }

  public:
    KRYS_NODISCARD static Ref<SharedBuffer> Create() noexcept
    {
      return AdoptRef(*new SharedBuffer({}));
    }
    KRYS_NODISCARD static Ref<SharedBuffer> Create(List<uint8> &&data) noexcept
    {
      return AdoptRef(*new SharedBuffer(krys::move(data)));
    }
    KRYS_NODISCARD static Ref<SharedBuffer> Create(Span<const uint8> data) noexcept
    {
      return AdoptRef(*new SharedBuffer(List<uint8>(data.begin(), data.end())));
    }

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept
    {
      return {_data.data(), _data.size()};
    }
    KRYS_NODISCARD const uint8 *Data() const noexcept
    {
      return _data.data();
    }
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _data.size();
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _data.empty();
    }

    /// @brief The byte at @p position.
    KRYS_NODISCARD uint8 operator[](size_t position) const noexcept
    {
      return _data[position];
    }
  };
}
