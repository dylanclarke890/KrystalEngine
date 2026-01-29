#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <catch_all.hpp>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Krys::Tests
{
  namespace
  {
    template <class T, class Inner, class Converter>
    class RefCountedDelegatingTraits;

    template <class T, class Inner, class Converter>
    struct RefCountedWeakDelegatingTraits
    {
      using strong_value_type = Inner;
      using strong_ptr_traits = RefCountedDelegatingTraits<T, Inner, Converter>;

      static void AddRef(const void *Ref) noexcept
      {
        RealWeakFromDelegatingWeak(Ref)->AddRef();
      }

      static void SubRef(const void *Ref) noexcept
      {
        RealWeakFromDelegatingWeak(Ref)->SubRef();
      }

      static const Inner *Lock(const void *Ref) noexcept
      {
        auto outer_strong = RealWeakFromDelegatingWeak(Ref)->LockOwner();
        return outer_strong ? DelegatingFromReal(outer_strong) : nullptr;
      }

      static Inner *Lock(void *Ref) noexcept
      {
        auto outer_strong = RealWeakFromDelegatingWeak(Ref)->LockOwner();
        return outer_strong ? DelegatingFromReal(outer_strong) : nullptr;
      }

    private:
      static T *RealFromDelegating(Inner *pinner)
      {
        return const_cast<T *>(Converter::RealFromDelegating(const_cast<const Inner *>(pinner)));
      }
      static const T *RealFromDelegating(const Inner *pinner)
      {
        return const_cast<const T *>(Converter::RealFromDelegating(pinner));
      }

      static Inner *DelegatingFromReal(T *pouter)
      {
        return const_cast<Inner *>(Converter::DelegatingFromReal(const_cast<const T *>(pouter)));
      }
      static const Inner *DelegatingFromReal(const T *pouter)
      {
        return const_cast<const Inner *>(Converter::DelegatingFromReal(pouter));
      }

      static typename T::weak_value_type *RealWeakFromDelegatingWeak(void *Ref)
      {
        return static_cast<typename T::weak_value_type *>(Ref);
      }
      static const typename T::weak_value_type *RealWeakFromDelegatingWeak(const void *Ref)
      {
        return static_cast<const typename T::weak_value_type *>(Ref);
      }
    };

    template <class T, class Inner, class Converter>
    class RefCountedDelegatingTraits
    {
    public:
      using weak_value_type = void;

    public:
      using weak_ptr_traits = conditional_t<T::ProvidesWeakReferences,
                                            RefCountedWeakDelegatingTraits<T, Inner, Converter>, void>;

      static void AddRef(const Inner *pinner) noexcept
      {
        RealFromDelegating(pinner)->AddRef();
      }

      static void SubRef(const Inner *pinner) noexcept
      {
        RealFromDelegating(pinner)->SubRef();
      }

      static const void *GetWeakValue(const Inner *pinner)
      {
        return RealFromDelegating(pinner)->GetWeakValue();
      }

      static void *GetWeakValue(Inner *pinner)
      {
        return const_cast<void *>(
          RefCountedDelegatingTraits::GetWeakValue(const_cast<const Inner *>(pinner)));
      }

    private:
      static T *RealFromDelegating(Inner *pinner)
      {
        return const_cast<T *>(Converter::RealFromDelegating(const_cast<const Inner *>(pinner)));
      }
      static const T *RealFromDelegating(const Inner *pinner)
      {
        return const_cast<const T *>(Converter::RealFromDelegating(pinner));
      }
    };

    template <class T, class Inner, class Converter>
    inline IntrusivePtr<void, RefCountedWeakDelegatingTraits<T, Inner, Converter>>
      weak_cast(const IntrusivePtr<Inner, RefCountedDelegatingTraits<T, Inner, Converter>> &src)
    {
      using dst_type = IntrusivePtr<void, RefCountedWeakDelegatingTraits<T, Inner, Converter>>;
      return dst_type::NoRef(RefCountedDelegatingTraits<T, Inner, Converter>::GetWeakValue(src.get()));
    }

    template <class T, class Inner, class Converter>
    inline IntrusivePtr<const void, RefCountedWeakDelegatingTraits<T, Inner, Converter>>
      weak_cast(const IntrusivePtr<const Inner, RefCountedDelegatingTraits<T, Inner, Converter>> &src)
    {
      using dst_type = IntrusivePtr<const void, RefCountedWeakDelegatingTraits<T, Inner, Converter>>;
      return dst_type::NoRef(RefCountedDelegatingTraits<T, Inner, Converter>::GetWeakValue(src.get()));
    }

    template <class T, class Inner, class Converter>
    inline IntrusivePtr<Inner, RefCountedDelegatingTraits<T, Inner, Converter>> strong_cast(
      const IntrusivePtr<void, RefCountedWeakDelegatingTraits<T, Inner, Converter>> &src) noexcept
    {
      using dst_type = IntrusivePtr<Inner, RefCountedDelegatingTraits<T, Inner, Converter>>;
      return dst_type::NoRef(RefCountedWeakDelegatingTraits<T, Inner, Converter>::Lock(src.get()));
    }

    template <class T, class Inner, class Converter>
    inline IntrusivePtr<const Inner, RefCountedDelegatingTraits<T, Inner, Converter>> strong_cast(
      const IntrusivePtr<const void, RefCountedWeakDelegatingTraits<T, Inner, Converter>> &src) noexcept
    {
      using dst_type = IntrusivePtr<const Inner, RefCountedDelegatingTraits<T, Inner, Converter>>;
      return dst_type::NoRef(RefCountedWeakDelegatingTraits<T, Inner, Converter>::Lock(src.get()));
    }

    class outer : public RefCounted<outer>
    {
      friend RefCounted;

    private:
      struct inner_converter
      {
        static const outer *RealFromDelegating(const int *pinner) noexcept
        {
          outer *dummy = nullptr;
          size_t distance = (uintptr_t)&(dummy->_inner) - (uintptr_t)dummy;
          return (const outer *)((std::byte *)pinner - distance);
          // return offsetof(outer, _inner);
        }
      };

      using inner_traits = RefCountedDelegatingTraits<outer, int, inner_converter>;
      friend RefCountedDelegatingTraits<outer, int, inner_converter>;

    public:
      using inner_ptr = IntrusivePtr<int, inner_traits>;
      using const_inner_ptr = IntrusivePtr<const int, inner_traits>;

      inner_ptr GetInnerPtr() noexcept
      {
        return inner_ptr::Ref(&_inner);
      }
      const_inner_ptr GetInnerPtr() const noexcept
      {
        return const_inner_ptr::Ref(&_inner);
      }

      int &inner()
      {
        return _inner;
      }

    private:
      int _inner = 0;
    };

    class weak_outer : public RefCounted<weak_outer, RefCountedFlags::ProvideWeakReferences>
    {
      friend RefCounted;

    private:
      struct inner_converter
      {
        static const weak_outer *RealFromDelegating(const int *pinner) noexcept
        {
          weak_outer *dummy = nullptr;
          size_t distance = (uintptr_t)&(dummy->_inner) - (uintptr_t)dummy;
          return (const weak_outer *)((std::byte *)pinner - distance);
          // return offsetof(outer, _inner);
        }
        static const int *DelegatingFromReal(const weak_outer *pouter) noexcept
        {
          return &pouter->_inner;
        }
      };

      using inner_traits = RefCountedDelegatingTraits<weak_outer, int, inner_converter>;
      friend RefCountedDelegatingTraits<weak_outer, int, inner_converter>;

    public:
      using inner_ptr = IntrusivePtr<int, inner_traits>;
      using const_inner_ptr = IntrusivePtr<const int, inner_traits>;

      using weak_inner_ptr = IntrusivePtr<void, inner_traits::weak_ptr_traits>;
      using const_weak_inner_ptr = IntrusivePtr<const void, inner_traits::weak_ptr_traits>;

      class weak_value_type : public WeakReference<weak_outer>
      {
        friend weak_outer;
        friend RefCountedWeakDelegatingTraits<weak_outer, int, inner_converter>;

      private:
        weak_value_type(intptr_t count, weak_outer *owner) : WeakReference(count, owner) {};
      };

      inner_ptr GetInnerPtr() noexcept
      {
        return inner_ptr::Ref(&_inner);
      }
      const_inner_ptr GetInnerPtr() const noexcept
      {
        return const_inner_ptr::Ref(&_inner);
      }

      weak_inner_ptr get_weak_inner_ptr()
      {
        return weak_inner_ptr::NoRef(inner_traits::GetWeakValue(&_inner));
      }

      const_weak_inner_ptr get_weak_inner_ptr() const
      {
        return const_weak_inner_ptr::NoRef(inner_traits::GetWeakValue(&_inner));
      }

      weak_value_type *MakeWeakReference(intptr_t count) const
      {
        return new weak_value_type(count, const_cast<weak_outer *>(this));
      }

      int &inner()
      {
        return _inner;
      }

    private:
      int _inner = 0;
    };
  }

  TEST_CASE("RefCounted - Inner counting")
  {
    auto pouter = RefPtrAttach(new outer());
    auto pinner = pouter->GetInnerPtr();
    CHECK(pinner);
    *pinner = 3;
    CHECK(pouter->inner() == 3);
  }

  TEST_CASE("RefCounted - Weak inner counting")
  {
    SECTION("Non const")
    {
      auto pouter = RefPtrAttach(new weak_outer());
      auto pinner1 = pouter->GetInnerPtr();
      CHECK(pinner1);
      auto weak1 = pouter->get_weak_inner_ptr();
      CHECK(weak1);
      auto weak2 = weak_cast(pinner1);
      CHECK(weak1 == weak2);

      auto pinner2 = strong_cast(weak2);
      CHECK(pinner2 == pinner1);
    }

    SECTION("Const")
    {
      auto pouter = RefPtrAttach(const_cast<const weak_outer *>(new weak_outer()));
      auto pinner1 = pouter->GetInnerPtr();
      CHECK(pinner1);
      auto weak1 = pouter->get_weak_inner_ptr();
      CHECK(weak1);
      auto weak2 = weak_cast(pinner1);
      CHECK(weak1 == weak2);

      auto pinner2 = strong_cast(weak2);
      CHECK(pinner2 == pinner1);
    }
  }
}