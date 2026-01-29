#include "Krystal.Lib.Tests/Pointers/IntrusivePtr/Mocks.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include <catch_all.hpp>
#include <cstdint>
#include <sstream>
#include <type_traits>

namespace Krys::Tests
{
  TEST_CASE("IntrusivePtr - Type traits are correct")
  {
    using ptr = mock_ptr<instrumented_counted<1>>;

    SECTION("Construction, destruction and assignment")
    {
      CHECK(sizeof(ptr) == sizeof(instrumented_counted<1> *));
      CHECK(AlignmentOf<ptr> == AlignmentOf<instrumented_counted<1> *>);

      CHECK(DefaultConstructible<ptr>);
      CHECK(NoThrowDefaultConstructible<ptr>);
      CHECK(!TriviallyDefaultConstructible<ptr>);

      CHECK(CopyConstructible<ptr>);
      CHECK(!TriviallyCopyConstructible<ptr>);
      CHECK(NoThrowCopyConstructible<ptr>);

      CHECK(MoveConstructible<ptr>);
      CHECK(!TriviallyMoveConstructible<ptr>);
      CHECK(NoThrowMoveConstructible<ptr>);

      CHECK(CopyAssignable<ptr>);
      CHECK(!TriviallyCopyAssignable<ptr>);
      CHECK(NoThrowCopyAssignable<ptr>);

      CHECK(MoveAssignable<ptr>);
      CHECK(!TriviallyMoveAssignable<ptr>);
      CHECK(NoThrowMoveAssignable<ptr>);

      CHECK(Swappable<ptr>);
      CHECK(NoThrowSwappable<ptr>);

      CHECK(Destructible<ptr>);
      CHECK(!TriviallyDestructible<ptr>);
      CHECK(NoThrowDestructible<ptr>);

      CHECK(Constructible<ptr, std::nullptr_t>);
      CHECK(NoThrowConstructible<ptr, std::nullptr_t>);
      CHECK(!TriviallyConstructible<ptr, std::nullptr_t>);
    }

    using another_ptr = mock_ptr<derived_instrumented_counted<1>>;

    SECTION("Conversions to/from other ptr types")
    {
      CHECK(ConvertibleTo<another_ptr, ptr>);

      CHECK(Constructible<ptr, another_ptr>);
      CHECK(!TriviallyConstructible<ptr, another_ptr>);
      CHECK(NoThrowConstructible<ptr, another_ptr>);

      CHECK(Assignable<ptr, another_ptr>);
      CHECK(!TriviallyAssignable<ptr, another_ptr>);
      CHECK(NoThrowAssignable<ptr, another_ptr>);

      CHECK(!ConvertibleTo<ptr, another_ptr>);
      CHECK(!Constructible<another_ptr, ptr>);
      CHECK(!Assignable<another_ptr, ptr>);
    }

    using ptr_different_traits = mock_ptr_different_traits<instrumented_counted<1>>;
    SECTION("Conversions to/from other traits")
    {
      CHECK(ConvertibleTo<ptr_different_traits, ptr>);
      CHECK(ConvertibleTo<ptr, ptr_different_traits>);

      CHECK(Constructible<ptr, ptr_different_traits>);
      CHECK(!TriviallyConstructible<ptr, ptr_different_traits>);
      CHECK(NoThrowConstructible<ptr, ptr_different_traits>);

      CHECK(Assignable<ptr, ptr_different_traits>);
      CHECK(!TriviallyAssignable<ptr, ptr_different_traits>);
      CHECK(NoThrowAssignable<ptr, ptr_different_traits>);
    }
  }

  TEST_CASE("IntrusivePtr - Default constructed and constructed from nullptr ptr behaves like nullptr",
            "[Memory]")
  {
    mock_ptr<instrumented_counted<>> empty;

    SECTION("Default constructed")
    {
      CHECK(empty.get() == nullptr);
      CHECK(!bool(empty));
      CHECK(empty.operator->() == nullptr);
    }

    SECTION("Construsted from nullptr")
    {
      mock_ptr<instrumented_counted<>> empty1(nullptr);

      CHECK(empty1.get() == nullptr);
      CHECK(!bool(empty1));
      CHECK(empty1.operator->() == nullptr);
    }

    SECTION("Comparing to another empty ptr")
    {
      mock_ptr<instrumented_counted<>> empty1;

      CHECK(empty == empty1);
      CHECK(!(empty != empty1));
      CHECK(!(empty < empty1));
      CHECK(empty <= empty1);
      CHECK(!(empty > empty1));
      CHECK(empty >= empty1);
    }

    SECTION("Comparing to a ptr created from nullptr")
    {
      mock_ptr<instrumented_counted<>> empty1(nullptr);

      CHECK(empty == empty1);
      CHECK(!(empty != empty1));
      CHECK(!(empty < empty1));
      CHECK(empty <= empty1);
      CHECK(!(empty > empty1));
      CHECK(empty >= empty1);
    }

    SECTION("Comparing to nullptr")
    {
      CHECK(empty == nullptr);
      CHECK(nullptr == empty);
      CHECK(!(empty != nullptr));
      CHECK(!(nullptr != empty));
    }

    SECTION("Comparing to raw pointer")
    {
      instrumented_counted<> *raw = nullptr;

      CHECK(empty == raw);
      CHECK(raw == empty);
      CHECK(!(empty != raw));
      CHECK(!(raw != empty));
      CHECK(!(empty < raw));
      CHECK(!(raw < empty));
      CHECK(empty <= raw);
      CHECK(raw <= empty);
      CHECK(!(empty > raw));
      CHECK(!(raw > empty));
      CHECK(empty >= raw);
      CHECK(raw >= empty);
    }

    SECTION("Comparing to void pointer")
    {
      void *raw = nullptr;

      CHECK(empty == raw);
      CHECK(raw == empty);
      CHECK(!(empty != raw));
      CHECK(!(raw != empty));
      CHECK(!(empty < raw));
      CHECK(!(raw < empty));
      CHECK(empty <= raw);
      CHECK(raw <= empty);
      CHECK(!(empty > raw));
      CHECK(!(raw > empty));
      CHECK(empty >= raw);
      CHECK(raw >= empty);
    }

    SECTION("Comparing to different traits")
    {
      mock_ptr_different_traits<instrumented_counted<>> empty1;

      CHECK(empty == empty1);
      CHECK(!(empty != empty1));
      CHECK(!(empty < empty1));
      CHECK(empty <= empty1);
      CHECK(!(empty > empty1));
      CHECK(empty >= empty1);
    }
  }
  TEST_CASE("IntrusivePtr - Basic counting", "[Memory]")
  {
    SECTION("Attach")
    {
      instrumented_counted<> object;
      auto ptr = mock_noref(&object);
      REQUIRE(&object == ptr.get());
      REQUIRE(object.count == 1);
      CHECK(bool(ptr));
      CHECK(ptr.operator->() == &object);
    }

    SECTION("Ref")
    {
      instrumented_counted<> object;
      auto ptr = mock_ref(&object);
      REQUIRE(&object == ptr.get());
      REQUIRE(object.count == 2);
      CHECK(bool(ptr));
      CHECK(ptr.operator->() == &object);

      mock_traits<>::SubRef(&object);
    }

    SECTION("Release")
    {
      instrumented_counted<> object;
      auto ptr = mock_noref(&object);
      REQUIRE(object.count == 1);
      auto p = ptr.release();
      CHECK(p == &object);
      CHECK(ptr.get() == nullptr);
      CHECK(!bool(ptr));
      CHECK(ptr.operator->() == nullptr);

      mock_traits<>::SubRef(&object);
    }

    SECTION("Reset")
    {
      instrumented_counted<> object;
      auto ptr = mock_noref(&object);
      REQUIRE(object.count == 1);
      ptr.reset();
      CHECK(ptr.get() == nullptr);
      CHECK(!bool(ptr));
      CHECK(ptr.operator->() == nullptr);
    }

    SECTION("Copy and assignment")
    {
      instrumented_counted<> object;
      auto ptr = mock_noref(&object);
      REQUIRE(object.count == 1);

      auto ptr2(ptr);
      REQUIRE(object.count == 2);

      auto ptr3(mock_ref(&object));
      REQUIRE(object.count == 3);

      mock_ptr<instrumented_counted<>> ptr4;
      ptr4 = ptr;
      REQUIRE(object.count == 4);

      mock_ptr<instrumented_counted<>> ptr5;
      ptr5 = mock_ref(&object);
      REQUIRE(object.count == 5);
    }

    SECTION("Self assignment")
    {
      instrumented_counted<> object;
      auto ptr = mock_noref(&object);

      ptr = ptr;
      REQUIRE(object.count == 1);

      ptr = std::move(ptr);
      REQUIRE(ptr.get() == &object);
      REQUIRE(object.count == 1);
    }

    SECTION("Swap ")
    {
      instrumented_counted<> object1, object2;
      auto ptr1 = mock_noref(&object1);
      auto ptr2 = mock_noref(&object2);
      swap(ptr1, ptr2);
      CHECK(ptr1.get() == &object2);
      CHECK(ptr2.get() == &object1);
      REQUIRE(object1.count == 1);
      REQUIRE(object2.count == 1);

      ptr1.swap(ptr2);
      CHECK(ptr1.get() == &object1);
      CHECK(ptr2.get() == &object2);
      REQUIRE(object1.count == 1);
      REQUIRE(object2.count == 1);

      ptr2.swap(ptr1);
      CHECK(ptr1.get() == &object2);
      CHECK(ptr2.get() == &object1);
      REQUIRE(object1.count == 1);
      REQUIRE(object2.count == 1);
    }
  }

  TEST_CASE("IntrusivePtr - Conversions", "[Memory]")
  {
    SECTION("Attach")
    {
      derived_instrumented_counted<> object;
      mock_ptr<instrumented_counted<>> ptr = mock_noref(&object);
      REQUIRE(&object == ptr.get());
      REQUIRE(object.count == 1);
      CHECK(bool(ptr));
      CHECK(ptr.operator->() == &object);
    }

    SECTION("Ref")
    {
      derived_instrumented_counted<> object;
      mock_ptr<instrumented_counted<>> ptr = mock_ref(&object);
      REQUIRE(&object == ptr.get());
      REQUIRE(object.count == 2);
      CHECK(bool(ptr));
      CHECK(ptr.operator->() == &object);

      mock_traits<>::SubRef(&object);
    }

    SECTION("Release")
    {
      derived_instrumented_counted<> object;
      mock_ptr<instrumented_counted<>> ptr = mock_noref(&object);
      REQUIRE(object.count == 1);
      auto p = ptr.release();
      CHECK(p == &object);
      CHECK(ptr.get() == nullptr);
      CHECK(!bool(ptr));
      CHECK(ptr.operator->() == nullptr);

      mock_traits<>::SubRef(&object);
    }

    SECTION("Reset")
    {
      derived_instrumented_counted<> object;
      mock_ptr<instrumented_counted<>> ptr = mock_noref(&object);
      REQUIRE(object.count == 1);
      ptr.reset();
      CHECK(ptr.get() == nullptr);
      CHECK(!bool(ptr));
      CHECK(ptr.operator->() == nullptr);
    }

    SECTION("Copy and assignment")
    {
      derived_instrumented_counted<> object;
      auto ptr = mock_noref(&object);
      REQUIRE(object.count == 1);

      mock_ptr<instrumented_counted<>> ptr2(ptr);
      REQUIRE(object.count == 2);

      mock_ptr<instrumented_counted<>> ptr3(mock_ref(&object));
      REQUIRE(object.count == 3);

      mock_ptr<instrumented_counted<>> ptr4;
      ptr4 = ptr;
      REQUIRE(object.count == 4);

      mock_ptr<instrumented_counted<>> ptr5;
      ptr5 = mock_ref(&object);
      REQUIRE(object.count == 5);
    }
  }

  TEST_CASE("IntrusivePtr - Different traits", "[Memory]")
  {
    SECTION("Copy")
    {
      instrumented_counted<> object;
      mock_ptr_different_traits<instrumented_counted<>> ptr = mock_noref_different_traits(&object);
      mock_ptr<instrumented_counted<>> ptr1(ptr);
      REQUIRE(&object == ptr1.get());
      REQUIRE(object.count == 2);
    }

    SECTION("Move")
    {
      instrumented_counted<> object;
      mock_ptr_different_traits<instrumented_counted<>> ptr = mock_noref_different_traits(&object);
      mock_ptr<instrumented_counted<>> ptr1(std::move(ptr));
      REQUIRE(&object == ptr1.get());
      REQUIRE(object.count == 1);
    }
  }

  TEST_CASE("IntrusivePtr - Casts", "[Memory]")
  {
    derived_instrumented_counted<> object;
    const instrumented_counted<> const_object;
    derived_instrumented_counted<> derived_object;

    mock_ptr<instrumented_counted<>> ptr = mock_noref(&object);
    auto ptr_const = mock_noref(&const_object);
    auto ptr_derived = mock_noref(&derived_object);

    SECTION("Const cast")
    {
      auto res = intrusive_const_cast<mock_ptr<instrumented_counted<>>>(ptr_const);
      CHECK(res.get() == &const_object);
      CHECK(const_object.count == 2);

      res = intrusive_const_cast<mock_ptr<instrumented_counted<>>>(mock_ref(&const_object));
      CHECK(res.get() == &const_object);
      CHECK(const_object.count == 2);
    }

    SECTION("Dynamic cast")
    {
      auto res = intrusive_dynamic_cast<mock_ptr<derived_instrumented_counted<>>>(ptr);
      CHECK(res.get() == &object);
      CHECK(object.count == 2);

      res = intrusive_dynamic_cast<mock_ptr<derived_instrumented_counted<>>>(
        mock_ptr<instrumented_counted<>>(ptr));
      CHECK(res.get() == &object);
      CHECK(object.count == 2);

      auto res1 = intrusive_dynamic_cast<mock_ptr<const derived_instrumented_counted<>>>(ptr_const);
      CHECK(!res1);

      res1 = intrusive_dynamic_cast<mock_ptr<const derived_instrumented_counted<>>>(mock_ref(&const_object));
      CHECK(!res1);
    }

    SECTION("Static cast")
    {
      auto res = intrusive_static_cast<mock_ptr<derived_instrumented_counted<>>>(ptr);
      CHECK(res.get() == &object);
      CHECK(object.count == 2);

      res = intrusive_static_cast<mock_ptr<derived_instrumented_counted<>>>(
        mock_ptr<instrumented_counted<>>(ptr));
      CHECK(res.get() == &object);
      CHECK(object.count == 2);
    }
  }

  TEST_CASE("IntrusivePtr - Output", "[Memory]")
  {
    instrumented_counted<> object;
    auto ptr = mock_noref(&object);

    std::ostringstream str1;

    auto &res = (str1 << ptr);
    CHECK(&res == &str1);

    std::ostringstream str2;
    str2 << &object;

    CHECK(str1.str() == str2.str());
  }

  TEST_CASE("IntrusivePtr - Format", "[Memory]")
  {
    instrumented_counted<> object;
    auto ptr = mock_noref(&object);

    std::string res = std::format("{}", ptr);
    std::string expected = std::format("{}", (void *)ptr.get());

    CHECK(res == expected);
  }

  TEST_CASE("IntrusivePtr - Output param", "[Memory]")
  {
    instrumented_counted<> object, object1;
    auto ptr = mock_noref(&object);
    auto ptr1 = mock_noref(&object1);

    auto func = [&object1](instrumented_counted<> **out)
    {
      mock_traits<>::AddRef(&object1);
      *out = &object1;
    };

    func(ptr.GetOutputParam());
    CHECK(ptr.get() == &object1);
    CHECK(object.count == -1);
    CHECK(object1.count == 2);
  }

  TEST_CASE("IntrusivePtr - Input Output param", "[Memory]")
  {
    instrumented_counted<> object, object1;
    auto ptr = mock_noref(&object);
    auto ptr1 = mock_noref(&object1);

    auto func = [&](instrumented_counted<> **inout)
    {
      CHECK(*inout == &object);
      mock_traits<>::SubRef(&object);
      mock_traits<>::AddRef(&object1);
      *inout = &object1;
    };

    func(ptr.GetInOutParam());
    CHECK(ptr.get() == &object1);
    CHECK(object.count == -1);
    CHECK(object1.count == 2);
  }

  TEST_CASE("IntrusivePtr - Member pointer", "[Memory]")
  {
    instrumented_counted<> object;
    auto ptr = mock_noref(&object);

    int instrumented_counted<>::*pcount = &instrumented_counted<>::count;

    auto x = ptr->*pcount;
    CHECK(x == 1);
  }

  TEST_CASE("IntrusivePtr - Hash code", "[Memory]")
  {
    instrumented_counted<> object;
    auto ptr = mock_noref(&object);

    CHECK(std::hash<decltype(ptr)>()(ptr) == std::hash<instrumented_counted<> *>()(&object));
    CHECK(std::hash<decltype(ptr)>()(ptr) == hash_value(ptr));
  }
}