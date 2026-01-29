#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <catch_all.hpp>

#include <stdexcept>
#include <vector>

namespace Krys::Tests
{
  namespace
  {
    struct minimal_counted : RefCounted<minimal_counted, RefCountedFlags::None, char>
    {
      friend RefCounted;

    private:
      ~minimal_counted() noexcept
      {
      }
    };

    static_assert(!std::is_default_constructible_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_copy_constructible_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_move_constructible_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_copy_assignable_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_move_assignable_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_swappable_v<RefCounted<minimal_counted>>);
    static_assert(!std::is_destructible_v<RefCounted<minimal_counted>>);

    static_assert(sizeof(minimal_counted) == sizeof(char));
    static_assert(!std::is_default_constructible_v<minimal_counted>);
    static_assert(!std::is_copy_constructible_v<minimal_counted>);
    static_assert(!std::is_move_constructible_v<minimal_counted>);
    static_assert(!std::is_copy_assignable_v<minimal_counted>);
    static_assert(!std::is_move_assignable_v<minimal_counted>);
    static_assert(!std::is_swappable_v<minimal_counted>);
    static_assert(!std::is_destructible_v<minimal_counted>);

    struct adapded
    {
      char c;
    };
    using minimal_adapded_counted = RefCountedAdapter<adapded, RefCountedFlags::None, char>;

    static_assert(sizeof(minimal_adapded_counted) == 2 * sizeof(char));
    static_assert(!std::is_default_constructible_v<minimal_adapded_counted>);
    static_assert(!std::is_copy_constructible_v<minimal_adapded_counted>);
    static_assert(!std::is_move_constructible_v<minimal_adapded_counted>);
    static_assert(!std::is_copy_assignable_v<minimal_adapded_counted>);
    static_assert(!std::is_move_assignable_v<minimal_adapded_counted>);
    static_assert(!std::is_destructible_v<minimal_adapded_counted>);

    using minimal_wrapped_counted = RefCountedWrapper<char, RefCountedFlags::None, char>;

    static_assert(sizeof(minimal_wrapped_counted) == 2 * sizeof(char));
    static_assert(!std::is_default_constructible_v<minimal_wrapped_counted>);
    static_assert(!std::is_copy_constructible_v<minimal_wrapped_counted>);
    static_assert(!std::is_move_constructible_v<minimal_wrapped_counted>);
    static_assert(!std::is_copy_assignable_v<minimal_wrapped_counted>);
    static_assert(!std::is_move_assignable_v<minimal_wrapped_counted>);
    static_assert(!std::is_destructible_v<minimal_wrapped_counted>);

    struct simple_counted : RefCounted<simple_counted>
    {
      friend RefCounted;

      static inline int instance_count = 0;

      simple_counted() noexcept
      {
        ++instance_count;
      }

      simple_counted(int)
      {
        throw std::runtime_error("x");
      }

    private:
      ~simple_counted() noexcept
      {
        --instance_count;
      }
    };

    static_assert(!std::is_default_constructible_v<simple_counted>);
    static_assert(!std::is_copy_constructible_v<simple_counted>);
    static_assert(!std::is_move_constructible_v<simple_counted>);
    static_assert(!std::is_copy_assignable_v<simple_counted>);
    static_assert(!std::is_move_assignable_v<simple_counted>);
    static_assert(!std::is_destructible_v<simple_counted>);

  }

  TEST_CASE("RefCounted - Minimal ref counted works")
  {
    SECTION("derived")
    {
      auto p1 = refcnt_attach(new minimal_counted());
      CHECK(p1);
      auto p2 = p1;
      CHECK(p2 == p1);
    }

    SECTION("adapted")
    {
      auto p1 = refcnt_attach(new minimal_adapded_counted(adapded {'a'}));
      CHECK(p1);
      CHECK(p1->c == 'a');
      auto p2 = p1;
      CHECK(p2 == p1);
    }

    SECTION("wrapped")
    {
      auto p1 = refcnt_attach(new minimal_wrapped_counted('a'));
      CHECK(p1);
      CHECK(p1->wrapped == 'a');
      auto p2 = p1;
      CHECK(p2 == p1);
    }
  }

  TEST_CASE("RefCounted - Simple ref counted works")
  {
    auto p1 = refcnt_attach(new simple_counted());
    CHECK(simple_counted::instance_count == 1);
    auto p2 = p1;
    CHECK(simple_counted::instance_count == 1);
    p1.reset();
    CHECK(simple_counted::instance_count == 1);
    p2.reset();
    CHECK(simple_counted::instance_count == 0);
  }

  TEST_CASE("RefCounted - Ref counted with ctor exception")
  {
    try
    {
      auto p1 = refcnt_attach(new simple_counted(2));
    }
    catch (std::exception &)
    {
      CHECK(simple_counted::instance_count == 0);
    }
  }

  TEST_CASE("RefCounted - Custom destroy")
  {
    struct custom_destroy : RefCounted<custom_destroy>
    {
      friend RefCounted;

      custom_destroy(bool *d) : destroyed(d)
      {
      }

      bool *destroyed;

    private:
      void destroy() const noexcept
      {
        *destroyed = true;
        free((void *)this);
      }
    };

    bool destroyed = false;
    auto p1 = refcnt_attach(new (malloc(sizeof(custom_destroy))) custom_destroy {&destroyed});
    p1.reset();
    CHECK(destroyed);
  }

  TEST_CASE("RefCounted - Ref counted wrapper")
  {
    SECTION("adapter")
    {
      auto p1 = refcnt_attach(new RefCountedAdapter<std::vector<char>>(5));
      CHECK(p1->size() == 5);
    }
    SECTION("wrapper")
    {
      auto p1 = refcnt_attach(new RefCountedWrapper<std::vector<char>>(5));
      CHECK(p1->wrapped.size() == 5);
    }
  }
}