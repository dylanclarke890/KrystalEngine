#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  /// @brief A copy-on-write reference to shared data: copies share the data until one of them calls
  /// `Access()`.
  /// @note `T` must be ref-counted and have a `Copy()` returning a `Ref<T>`.
  template <typename T>
  class DataRef
  {
  private:
    Ref<T> _data;

  public:
    constexpr DataRef(Ref<T> &&data) noexcept : _data(krys::move(data))
    {
    }

    constexpr DataRef(const DataRef &other) noexcept : _data(other._data)
    {
    }

    constexpr DataRef &operator=(const DataRef &other) noexcept = default;

    constexpr DataRef(DataRef &&) noexcept = default;

    constexpr DataRef &operator=(DataRef &&) noexcept = default;

    KRYS_NODISCARD constexpr operator const T &() const noexcept KRYS_LIFETIME_BOUND
    {
      return *_data;
    }

    KRYS_NODISCARD constexpr const T *get() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.get();
    }

    KRYS_NODISCARD constexpr const T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      return *_data;
    }

    KRYS_NODISCARD constexpr const T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.get();
    }

    /// @brief The data for writing, copied first if it's shared.
    KRYS_NODISCARD constexpr T &Access() noexcept KRYS_LIFETIME_BOUND
    {
      if (!_data->HasOneRef())
      {
        _data = _data->Copy();
      }
      return *_data;
    }

    KRYS_NODISCARD constexpr bool operator==(const DataRef &other) const noexcept
    {
      return _data.get() == other._data.get() || *_data == *other._data;
    }
  };
}
