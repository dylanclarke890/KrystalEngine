#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/Uninit.hpp"
#include <climits>
#include <cstddef>

namespace Krys::Text
{
  template <typename, std::size_t>
  class InlineBasicString;

  template <typename, std::size_t>
  class InlineVector;
}

namespace Krys::Text::detail
{
  template <std::size_t Capacity>
  struct OptimalStorageType
  {
    using type = conditional_t<
      Capacity <= UCHAR_MAX, uchar,
      conditional_t<Capacity <= USHRT_MAX, ushort,
                    conditional_t<Capacity <= UINT_MAX, uint,
                                  conditional_t<Capacity <= ULONG_MAX, ulong,
                                                conditional_t<Capacity <= ULLONG_MAX, ullong, // unsigned
                                                                                              // long
                                                                                              // long
                                                              std::size_t // just pick std::size_t if nothing
                                                                          // else matches
                                                              >           // end unsigned long long choice
                                                >                         // end unsigned long choice
                                  >                                       // end unsigned int choice
                    >                                                     // end unsigned short choice
      >;                                                                  // end unsigned char choice
  };

  template <std::size_t Capacity>
  using optimal_storage_type_t = typename OptimalStorageType<Capacity>::type;

  template <typename TLayoutFirst, typename TLayoutB>
  constexpr auto GetOptimalLayout() noexcept
  {
    // Cache is measured in 8-bit bytes, versus CHAR_BIT-sized structures
    constexpr std::size_t cacheBitSize = 64 * 8;
    constexpr std::size_t layoutFirstBitSize = (sizeof(TLayoutFirst) * CHAR_BIT);
    constexpr std::size_t layoutBBitSize = (sizeof(TLayoutB) * CHAR_BIT);
    constexpr bool layoutFirstCacheSmaller = layoutFirstBitSize <= cacheBitSize;
    constexpr bool layoutBCacheSmaller = layoutBBitSize <= cacheBitSize;
    if constexpr (layoutFirstCacheSmaller && layoutBCacheSmaller)
    {
      // if both layouts are smaller than cache
      // pick the one with the optimal size
      if constexpr (sizeof(TLayoutFirst) < sizeof(TLayoutB))
      {
        return TLayoutFirst {};
      }
      else
      {
        return TLayoutB {};
      }
    }
    else if constexpr (layoutFirstCacheSmaller)
    {
      // prefer the one that fits in cache (size first better)
      return TLayoutFirst {};
    }
    else if constexpr (layoutBCacheSmaller)
    {
      // prefer the one that fits in cache (size last if it gets us inside of cache)
      return TLayoutB {};
    }
    else
    {
      // There are 2 intepretations.
      // (1) is always prefer size first when we're beyond cache size.
      // That would mostly be to prevent cache hits, albeit due to alignment it CAN blow up size
      // of a structure considerable....
      // (2) would be to keep smallest, since that would make storage better.
      // Fixed-size vectors are often used to create better storage situations,
      // and we don't have a guarantee this won't be put inside of classes (it often is).
      //
      // Therefore, we pick (2) to prioritize the more general use cases.
#if 0
				// (1)
				return TLayoutFirst{};
#else
      // (2)
      if constexpr (sizeof(TLayoutFirst) < sizeof(TLayoutB))
      {
        return TLayoutFirst {};
      }
      else
      {
        return TLayoutB {};
      }
#endif
    }
  }

  template <typename T, std::size_t Capacity, bool = std::is_trivial_v<T>>
  class Storage
  {
  private:
    template <typename, std::size_t>
    friend class InlineVector;
    template <typename, std::size_t>
    friend class InlineBasicString;

    using value_type = T;
    using storage_type = Krys::Uninit<value_type>;

    struct SizeFirst
    {
      optimal_storage_type_t<Capacity> Size;
      storage_type Elements[Capacity];
    };
    struct SizeLast
    {
      storage_type Elements[Capacity];
      optimal_storage_type_t<Capacity> Size;
    };

    using StorageLayout = decltype(GetOptimalLayout<SizeFirst, SizeLast>());

  public:
    constexpr Storage() noexcept : _layout()
    {
    }

    constexpr add_pointer_t<storage_type> StorageDataAt(std::size_t index) noexcept
    {
      return std::addressof(this->_layout.Elements[index]);
    }

    constexpr add_pointer_t<add_const_t<storage_type>> StorageDataAt(std::size_t index) const noexcept
    {
      return (this->_layout.Elements + index);
    }

    constexpr add_pointer_t<value_type> Data(std::size_t index) noexcept
    {
      return std::addressof((this->_layout.Elements + index)->value);
    }

    constexpr add_pointer_t<add_const_t<value_type>> Data(std::size_t index) const noexcept
    {
      return std::addressof((this->_layout.Elements + index)->value);
    }

    add_pointer_t<value_type> data() noexcept
    {
      return std::addressof((this->_layout.Elements + 0)->value);
    }

    std::add_pointer_t<std::add_const_t<value_type>> data() const noexcept
    {
      return std::addressof((this->_layout.Elements + 0)->value);
    }

    ~Storage()
    {
      this->DestroyAll();
    }

  private:
    StorageLayout _layout;

    constexpr void DestroyAll() noexcept
    {
      if constexpr (NoThrowDestructible<T>)
      {
        for (std::size_t index = this->_layout.Size; index-- > 0;)
        {
          this->Data(index)->~value_type();
        }
        this->_layout.Size = 0;
      }
      else
      {
        for (std::size_t index = this->_layout.Size; index-- > 0;)
        {
          this->Data(index)->~value_type();
          this->_layout.Size -= 1;
        }
      }
    }
  };

  template <typename T, std::size_t Capacity>
  class Storage<T, Capacity, true>
  {
  private:
    template <typename, std::size_t>
    friend class InlineVector;
    template <typename, std::size_t>
    friend class InlineBasicString;

    using value_type = T;
    using storage_type = value_type;

    struct SizeFirst
    {
      optimal_storage_type_t<Capacity> Size;
      storage_type Elements[Capacity];
    };
    struct SizeLast
    {
      storage_type Elements[Capacity];
      optimal_storage_type_t<Capacity> Size;
    };

    using StorageLayout = decltype(GetOptimalLayout<SizeFirst, SizeLast>());

  public:
    constexpr Storage() noexcept : _layout()
    {
    }

    constexpr std::add_pointer_t<storage_type> StorageDataAt(std::size_t index) noexcept
    {
      return this->_layout.Elements + index;
    }

    constexpr std::add_pointer_t<std::add_const_t<storage_type>>
      StorageDataAt(std::size_t index) const noexcept
    {
      return this->_layout.Elements + index;
    }

    constexpr std::add_pointer_t<T> Data(std::size_t index) noexcept
    {
      return this->_layout.Elements + index;
    }

    constexpr std::add_pointer_t<std::add_const_t<T>> Data(std::size_t index) const noexcept
    {
      return this->_layout.Elements + index;
    }

    constexpr std::add_pointer_t<T> data() noexcept
    {
      return this->_layout.Elements + 0;
    }

    constexpr std::add_pointer_t<std::add_const_t<T>> data() const noexcept
    {
      return this->_layout.Elements + 0;
    }

  private:
    StorageLayout _layout;

    constexpr void DestroyAll() noexcept
    {
      if constexpr (NoThrowDestructible<T>)
      {
        for (std::size_t index = this->_layout.Size; index-- > 0;)
        {
          this->Data(index)->~value_type();
        }
        this->_layout.Size = 0;
      }
      else
      {
        for (std::size_t index = this->_layout.Size; index-- > 0;)
        {
          this->Data(index)->~value_type();
          this->_layout.Size -= 1;
        }
      }
    }
  };
}
