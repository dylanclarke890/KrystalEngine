#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <catch_all.hpp>

#include <stdexcept>
#include <vector>

namespace Krys::Tests
{
  namespace
  {
    struct MinimalCounted : RefCounted<MinimalCounted, RefCountedFlag::None, char>
    {
      friend RefCounted;

    private:
      ~MinimalCounted() noexcept
      {
      }
    };

    static_assert(!DefaultConstructible<RefCounted<MinimalCounted>>);
    static_assert(!CopyConstructible<RefCounted<MinimalCounted>>);
    static_assert(!MoveConstructible<RefCounted<MinimalCounted>>);
    static_assert(!CopyAssignable<RefCounted<MinimalCounted>>);
    static_assert(!MoveAssignable<RefCounted<MinimalCounted>>);
    static_assert(!Swappable<RefCounted<MinimalCounted>>);
    static_assert(!Destructible<RefCounted<MinimalCounted>>);

    static_assert(sizeof(MinimalCounted) == sizeof(char));
    static_assert(!DefaultConstructible<MinimalCounted>);
    static_assert(!CopyConstructible<MinimalCounted>);
    static_assert(!MoveConstructible<MinimalCounted>);
    static_assert(!CopyAssignable<MinimalCounted>);
    static_assert(!MoveAssignable<MinimalCounted>);
    static_assert(!Swappable<MinimalCounted>);
    static_assert(!Destructible<MinimalCounted>);

    struct adapded
    {
      char c;
    };
    using minimal_adapded_counted = RefCountedAdapter<adapded, RefCountedFlag::None, char>;

    static_assert(sizeof(minimal_adapded_counted) == 2 * sizeof(char));
    static_assert(!DefaultConstructible<minimal_adapded_counted>);
    static_assert(!CopyConstructible<minimal_adapded_counted>);
    static_assert(!MoveConstructible<minimal_adapded_counted>);
    static_assert(!CopyAssignable<minimal_adapded_counted>);
    static_assert(!MoveAssignable<minimal_adapded_counted>);
    static_assert(!Destructible<minimal_adapded_counted>);

    using minimal_wrapped_counted = RefCountedWrapper<char, RefCountedFlag::None, char>;

    static_assert(sizeof(minimal_wrapped_counted) == 2 * sizeof(char));
    static_assert(!DefaultConstructible<minimal_wrapped_counted>);
    static_assert(!CopyConstructible<minimal_wrapped_counted>);
    static_assert(!MoveConstructible<minimal_wrapped_counted>);
    static_assert(!CopyAssignable<minimal_wrapped_counted>);
    static_assert(!MoveAssignable<minimal_wrapped_counted>);
    static_assert(!Destructible<minimal_wrapped_counted>);

    struct SimpleCounted : RefCounted<SimpleCounted>
    {
      friend RefCounted;

      static inline int InstanceCount = 0;

      SimpleCounted() noexcept
      {
        ++InstanceCount;
      }

      SimpleCounted(int)
      {
        throw std::runtime_error("x");
      }

    private:
      ~SimpleCounted() noexcept
      {
        --InstanceCount;
      }
    };

    static_assert(!DefaultConstructible<SimpleCounted>);
    static_assert(!CopyConstructible<SimpleCounted>);
    static_assert(!MoveConstructible<SimpleCounted>);
    static_assert(!CopyAssignable<SimpleCounted>);
    static_assert(!MoveAssignable<SimpleCounted>);
    static_assert(!Destructible<SimpleCounted>);

  }

  TEST_CASE("RefCounted - Minimal Ref counted works")
  {
    SECTION("derived")
    {
      auto p1 = IntrusiveRefPtrAttach(new MinimalCounted());
      CHECK(p1);
      auto p2 = p1;
      CHECK(p2 == p1);
    }

    SECTION("adapted")
    {
      auto p1 = IntrusiveRefPtrAttach(new minimal_adapded_counted(adapded {'a'}));
      CHECK(p1);
      CHECK(p1->c == 'a');
      auto p2 = p1;
      CHECK(p2 == p1);
    }

    SECTION("wrapped")
    {
      auto p1 = IntrusiveRefPtrAttach(new minimal_wrapped_counted('a'));
      CHECK(p1);
      CHECK(p1->Wrapped == 'a');
      auto p2 = p1;
      CHECK(p2 == p1);
    }
  }

  TEST_CASE("RefCounted - Simple Ref counted works")
  {
    auto p1 = IntrusiveRefPtrAttach(new SimpleCounted());
    CHECK(SimpleCounted::InstanceCount == 1);
    auto p2 = p1;
    CHECK(SimpleCounted::InstanceCount == 1);
    p1.reset();
    CHECK(SimpleCounted::InstanceCount == 1);
    p2.reset();
    CHECK(SimpleCounted::InstanceCount == 0);
  }

  TEST_CASE("RefCounted - Ref counted with ctor exception")
  {
    try
    {
      auto p1 = IntrusiveRefPtrAttach(new SimpleCounted(2));
    }
    catch (std::exception &)
    {
      CHECK(SimpleCounted::InstanceCount == 0);
    }
  }

  TEST_CASE("RefCounted - Custom destroy")
  {
    struct CustomDestroy : RefCounted<CustomDestroy>
    {
      friend RefCounted;

      CustomDestroy(bool *d) : Destroyed(d)
      {
      }

      bool *Destroyed;

    private:
      void Destroy() const noexcept
      {
        *Destroyed = true;
        free((void *)this);
      }
    };

    bool destroyed = false;
    auto p1 = IntrusiveRefPtrAttach(new (malloc(sizeof(CustomDestroy))) CustomDestroy {&destroyed});
    p1.reset();
    CHECK(destroyed);
  }

  TEST_CASE("RefCounted - Ref counted wrapper")
  {
    SECTION("adapter")
    {
      auto p1 = IntrusiveRefPtrAttach(new RefCountedAdapter<std::vector<char>>(5));
      CHECK(p1->size() == 5);
    }
    SECTION("wrapper")
    {
      auto p1 = IntrusiveRefPtrAttach(new RefCountedWrapper<std::vector<char>>(5));
      CHECK(p1->Wrapped.size() == 5);
    }
  }
}