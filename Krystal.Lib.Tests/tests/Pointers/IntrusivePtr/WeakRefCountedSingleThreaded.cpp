#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <catch_all.hpp>
#include <cstdint>
#include <type_traits>

namespace Krys::Tests
{
  namespace
  {
    int derived_count = 0;

    struct derived_counted : SingleThreadWeakRefCounted<derived_counted>
    {
      friend RefCounted;

    public:
      derived_counted() noexcept
      {
        ++derived_count;
      }

    private:
      ~derived_counted() noexcept
      {
        auto weak = get_weak_ptr();
        CHECK(weak);
        --derived_count;
      }
    };

    int wrapped_count = 0;
    struct wrapped
    {
      wrapped()
      {
        ++wrapped_count;
      }
      ~wrapped()
      {
        --wrapped_count;
      }

      int value = 5;
    };

    using wrapped_counted = SingleThreadWeakRefCountedAdapter<wrapped>;

    int with_custom_weak_reference_count = 0;

    struct custom_weak_reference;

    class with_custom_weak_reference
        : public RefCounted<with_custom_weak_reference,
                             RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>
    {
      friend RefCounted;

    public:
      with_custom_weak_reference()
      {
        ++with_custom_weak_reference_count;
      }

    private:
      ~with_custom_weak_reference() noexcept
      {
        auto weak = get_weak_ptr();
        CHECK(weak);
        --with_custom_weak_reference_count;
      }

      custom_weak_reference *make_weak_reference(intptr_t count) const;
    };

    struct custom_weak_reference : WeakReference<with_custom_weak_reference>
    {
      custom_weak_reference(intptr_t count, with_custom_weak_reference *obj) : WeakReference(count, obj)
      {
      }

      ~custom_weak_reference()
      {
        CHECK(on_owner_destruction_called);
        CHECK(with_custom_weak_reference_count == 0);
      }

      void on_owner_destruction() const
      {
        CHECK(with_custom_weak_reference_count
              == 0); // owner is still alive but its refcount is 0, you cannot ressurrect!
        on_owner_destruction_called = true;
      }

      mutable bool on_owner_destruction_called = false;
    };

    inline custom_weak_reference *with_custom_weak_reference::make_weak_reference(intptr_t count) const
    {
      return new custom_weak_reference(count, const_cast<with_custom_weak_reference *>(this));
    }
  }

  TEST_CASE("RefCounted - Weak ref counted st type traits are correct")
  {
    SECTION("Base")
    {
      CHECK(SingleThreadWeakRefCounted<derived_counted>::single_threaded);
      CHECK(!std::is_default_constructible_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_copy_constructible_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_move_constructible_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_copy_assignable_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_move_assignable_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_swappable_v<SingleThreadWeakRefCounted<derived_counted>>);
      CHECK(!std::is_destructible_v<SingleThreadWeakRefCounted<derived_counted>>);
    }

    SECTION("Derived")
    {
      CHECK(derived_counted::single_threaded);
      CHECK(!std::is_default_constructible_v<derived_counted>);
      CHECK(!std::is_copy_constructible_v<derived_counted>);
      CHECK(!std::is_move_constructible_v<derived_counted>);
      CHECK(!std::is_copy_assignable_v<derived_counted>);
      CHECK(!std::is_move_assignable_v<derived_counted>);
      CHECK(!std::is_swappable_v<derived_counted>);
      CHECK(!std::is_destructible_v<derived_counted>);
    }

    SECTION("Derived WeakRef")
    {
      CHECK(derived_counted::weak_value_type::single_threaded);
      CHECK(!std::is_default_constructible_v<derived_counted::weak_value_type>);
      CHECK(!std::is_copy_constructible_v<derived_counted::weak_value_type>);
      CHECK(!std::is_move_constructible_v<derived_counted::weak_value_type>);
      CHECK(!std::is_copy_assignable_v<derived_counted::weak_value_type>);
      CHECK(!std::is_move_assignable_v<derived_counted::weak_value_type>);
      CHECK(!std::is_swappable_v<derived_counted::weak_value_type>);
      CHECK(!std::is_destructible_v<derived_counted::weak_value_type>);
    }

    SECTION("Wrapped")
    {
      CHECK(wrapped_counted::single_threaded);
      CHECK(!std::is_default_constructible_v<wrapped_counted>);
      CHECK(!std::is_copy_constructible_v<wrapped_counted>);
      CHECK(!std::is_move_constructible_v<wrapped_counted>);
      CHECK(!std::is_copy_assignable_v<wrapped_counted>);
      CHECK(!std::is_move_assignable_v<wrapped_counted>);
      CHECK(!std::is_destructible_v<wrapped_counted>);
    }

    SECTION("Wrapped WeakRef")
    {
      CHECK(wrapped_counted::weak_value_type::single_threaded);
      CHECK(!std::is_default_constructible_v<wrapped_counted::weak_value_type>);
      CHECK(!std::is_copy_constructible_v<wrapped_counted::weak_value_type>);
      CHECK(!std::is_move_constructible_v<wrapped_counted::weak_value_type>);
      CHECK(!std::is_copy_assignable_v<wrapped_counted::weak_value_type>);
      CHECK(!std::is_move_assignable_v<wrapped_counted::weak_value_type>);
      CHECK(!std::is_destructible_v<wrapped_counted::weak_value_type>);
    }
  }

  TEST_CASE("RefCounted - Weak ref counted st works")
  {
    SECTION("Derived")
    {
      auto original = refcnt_attach(new derived_counted());
      auto weak1 = original->get_weak_ptr();
      CHECK(derived_count == 1);
      auto strong1 = weak1->lock();
      CHECK(original == strong1);
      auto weak2 = strong1->get_weak_ptr();
      CHECK(weak1 == weak2);
      auto weak3 = weak_cast(strong1);
      CHECK(weak1 == weak3);
      original.reset();
      strong1.reset();
      CHECK(derived_count == 0);

      strong1 = weak1->lock();
      CHECK(!strong1);
    }

    SECTION("Const Derived")
    {
      RefPtr<const derived_counted> original = refcnt_attach(new derived_counted());
      auto weak1 = original->get_weak_ptr();
      CHECK(derived_count == 1);
      auto strong1 = weak1->lock();
      CHECK(original == strong1);
      auto weak2 = strong1->get_weak_ptr();
      CHECK(weak1 == weak2);
      original.reset();
      strong1.reset();
      CHECK(derived_count == 0);

      strong1 = weak1->lock();
      CHECK(!strong1);
      strong1 = strong_cast(weak2);
      CHECK(!strong1);
    }

    SECTION("Wrapped")
    {
      auto p = refcnt_attach(new wrapped_counted());
      CHECK(wrapped_count == 1);
      CHECK(p->value == 5);
      p.reset();
      CHECK(wrapped_count == 0);
    }

    SECTION("Custom Weak Reference")
    {
      auto strong = refcnt_attach(new with_custom_weak_reference);
      CHECK(with_custom_weak_reference_count == 1);
      auto weak = strong->get_weak_ptr();
      auto strong1 = weak->lock();
      CHECK(strong1 == strong);
      CHECK(with_custom_weak_reference_count == 1);
      strong.reset();
      CHECK(with_custom_weak_reference_count == 1);
      strong1.reset();
      CHECK(with_custom_weak_reference_count == 0);
      strong1 = weak->lock();
      CHECK(!strong1);
    }
  }
}