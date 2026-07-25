#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Tags.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include <bit>

namespace Krys
{
  template <bool IsTriviallyDestructible, typename T>
  struct UniqueArrayMaker;

  template <typename T>
  struct UniqueArrayFree
  {
    static_assert(TriviallyDestructible<T>);

    void operator()(T *pointer) const noexcept
    {
      std::free(const_cast<remove_cv_t<T> *>(pointer));
    }
  };

  template <typename T>
  struct UniqueArrayFree<T[]>
  {
    static_assert(TriviallyDestructible<T>);

    void operator()(T *pointer) const noexcept
    {
      std::free(const_cast<remove_cv_t<T> *>(pointer));
    }
  };

  template <typename T>
  struct UniqueArrayMaker<true, T>
  {
    using ResultType = typename UniquePtr<T[], UniqueArrayFree<T[]>>;

    KRYS_NODISCARD static ResultType Create(size_t size) noexcept
    {
      // C++ `new T[N]` stores its `N` to somewhere. Otherwise, `delete []` cannot destroy
      // these N elements. But we do not want to increase the size of allocated memory.
      // So this UniqueArray<T> only accepts the type T which has a trivial destructor. This allows us to skip
      // calling destructors for N elements. And this allows UniqueArray<T> not to store its N size.
      static_assert(TriviallyDestructible<T>);

      // Do not use placement new like `new (storage) T[size]()`. `new T[size]()` requires
      // larger storage than the `sizeof(T) * size` storage since it want to store `size`
      // to somewhere.
      auto storage = std::malloc(sizeof(T) * size);
      if constexpr (TriviallyConstructible<T>)
      {
        std::memset(storage, 0, sizeof(T) * size);
      }
      else
      {
        for (T *current = static_cast<T *>(storage); current != static_cast<T *>(storage) + size; ++current)
        {
          new (NotNullTag {}, current) T();
        }
      }

      return ResultType(storage);
    }
  };

  template <typename T>
  struct UniqueArrayMaker<false, T>
  {
    // Since we do not know how to store/retrieve N size to/from allocated memory when calling new [] and
    // delete [], we use new [] and delete [] operators simply. We create UniqueArrayElement container for the
    // type T. We allocate UniqueArrayElement[] and cast it to T[]. When deleting, the custom deleter casts
    // T[] to UniqueArrayElement[] and deletes it.
    class UniqueArrayElement
    {
    public:
      struct Deleter
      {
        void operator()(T *pointer) noexcept
        {
          delete[] std::bit_cast<UniqueArrayElement *>(pointer);
        };
      };

      UniqueArrayElement() noexcept = default;

      T Value {};
    };
    static_assert(sizeof(T) == sizeof(UniqueArrayElement));

    using ResultType = typename UniquePtr<T[], typename UniqueArrayElement::Deleter>;

    KRYS_NODISCARD static ResultType Create(size_t size) noexcept
    {
      return ResultType(std::bit_cast<T *>(new UniqueArrayElement[size]()));
    }
  };

  template <typename T>
  using UniqueArray = typename UniqueArrayMaker<TriviallyDestructible<T>, T>::ResultType;

  template <typename T>
  KRYS_NODISCARD UniqueArray<T> CreateUniqueArray(size_t size) noexcept
  {
    static_assert(SameType<remove_extent_t<T>, T>);
    return UniqueArrayMaker<TriviallyDestructible<T>, T>::Create(size);
  }
}