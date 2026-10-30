#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  /// @brief A copy-on-write reference to shared data: copies share the data until one of them calls `Access()`.
  /// @note `T` must be ref-counted and have a `Copy()` returning a `Ref<T>`.
  template <typename T>
  class DataRef
  {
  private:
    Ref<T> _data;

  public:
    DataRef(Ref<T> &&data) noexcept : _data(krys::move(data))
    {
    }

    DataRef(const DataRef &other) noexcept : _data(other._data)
    {
    }

    DataRef &operator=(const DataRef &other) noexcept
    {
      _data = other._data;
      return *this;
    }

    DataRef(DataRef &&) noexcept = default;
    DataRef &operator=(DataRef &&) noexcept = default;

    KRYS_NODISCARD operator const T &() const noexcept KRYS_LIFETIME_BOUND
    {
      return *_data;
    }
    KRYS_NODISCARD const T *Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.get();
    }
    KRYS_NODISCARD const T &Get() const noexcept KRYS_LIFETIME_BOUND
    {
      return *_data;
    }
    KRYS_NODISCARD const T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      return *_data;
    }
    KRYS_NODISCARD const T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.get();
    }

    /// @brief The data for writing, copied first if it's shared.
    KRYS_NODISCARD T &Access() noexcept KRYS_LIFETIME_BOUND
    {
      if (!_data->HasOneRef())
      {
        _data = _data->Copy();
      }
      return *_data;
    }

    KRYS_NODISCARD bool operator==(const DataRef &other) const noexcept
    {
      return _data.get() == other._data.get() || *_data == *other._data;
    }
  };
}
