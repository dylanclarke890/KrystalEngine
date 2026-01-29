#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include <catch_all.hpp>
#include <cstdint>
#include <type_traits>

namespace Krys::Tests
{
  namespace
  {
    bool counted_add_ref_called = false;
    bool counted_sub_ref_called = false;
    bool counted_destroy_called = false;
    bool make_weak_reference_called = false;
    bool get_weak_value_called = false;

    class abstract_weak_reference;

    class abstract_ref_counted
        : public RefCounted<abstract_ref_counted, RefCountedFlags::ProvideWeakReferences>
    {
      friend RefCounted;

    public:
      virtual void add_ref() const noexcept final
      {
        counted_add_ref_called = true;
        RefCounted::add_ref();
      }

      virtual void sub_ref() const noexcept final
      {
        counted_sub_ref_called = true;
        RefCounted::sub_ref();
      }

    protected:
      virtual ~abstract_ref_counted() noexcept
      {
      }

      virtual void destroy() const noexcept
      {
        counted_destroy_called = true;
        RefCounted::destroy();
      }

      virtual abstract_weak_reference *make_weak_reference(intptr_t count) const;

      virtual const WeakReference<abstract_ref_counted> *get_weak_value() const
      {
        get_weak_value_called = true;
        return RefCounted::get_weak_value();
      }
    };

    bool weak_add_ref_called = false;
    bool weak_sub_ref_called = false;
    bool weak_destroy_called = false;
    bool add_owner_ref_called = false;
    bool sub_owner_ref_called = false;
    bool lock_owner_called = false;
    bool on_owner_destruction_called = false;

    class abstract_weak_reference : public WeakReference<abstract_ref_counted>
    {
      friend WeakReference;
      friend abstract_ref_counted;

    public:
      virtual void add_ref() const noexcept final
      {
        weak_add_ref_called = true;
        WeakReference::add_ref();
      }

      virtual void sub_ref() const noexcept final
      {
        weak_sub_ref_called = true;
        WeakReference::sub_ref();
      }

    protected:
      virtual ~abstract_weak_reference() noexcept
      {
      }

      virtual void destroy() const noexcept
      {
        weak_destroy_called = true;
        WeakReference::destroy();
      }

    private:
      abstract_weak_reference(intptr_t count, abstract_ref_counted *owner) : WeakReference(count, owner)
      {
      }

      virtual void add_owner_ref() noexcept
      {
        add_owner_ref_called = true;
        WeakReference::add_owner_ref();
      }

      virtual void sub_owner_ref() noexcept
      {
        sub_owner_ref_called = true;
        WeakReference::sub_owner_ref();
      }

      virtual abstract_ref_counted *lock_owner() const noexcept
      {
        lock_owner_called = true;
        return static_cast<abstract_ref_counted *>(WeakReference::lock_owner());
      }

      virtual void on_owner_destruction() const noexcept
      {
        on_owner_destruction_called = true;
      }
    };

    inline auto abstract_ref_counted::make_weak_reference(intptr_t count) const -> abstract_weak_reference *
    {
      make_weak_reference_called = true;
      return new abstract_weak_reference(count, const_cast<abstract_ref_counted *>(this));
    }

    class simple : public abstract_ref_counted
    {
    };
  }

  TEST_CASE("RefCounted - Abstract")
  {
    SECTION("Simple")
    {
      auto p = refcnt_attach(new simple());
      decltype(p) p1 = p;
      p1.reset();
      auto w = weak_cast(p);
      static_assert(std::is_same_v<decltype(w)::element_type, WeakReference<abstract_ref_counted>>,
                    "invalid weak reference type");
      auto w1 = p->get_weak_ptr();
      static_assert(std::is_same_v<decltype(w1)::element_type, WeakReference<abstract_ref_counted>>,
                    "invalid weak reference type");
      auto p2 = strong_cast(w);
      auto p3 = p2;
      static_assert(std::is_same_v<decltype(p2)::element_type, abstract_ref_counted>,
                    "invalid weak reference type");

      p.reset();
      p2.reset();
      p3.reset();
      w.reset();
      w1.reset();

      CHECK(counted_add_ref_called);
      CHECK(counted_sub_ref_called);
      CHECK(counted_destroy_called);
      CHECK(make_weak_reference_called);
      CHECK(get_weak_value_called);
      CHECK(weak_add_ref_called);
      CHECK(weak_sub_ref_called);
      CHECK(weak_destroy_called);
      CHECK(add_owner_ref_called);
      CHECK(sub_owner_ref_called);
      CHECK(lock_owner_called);
      CHECK(on_owner_destruction_called);
    }
  }
}