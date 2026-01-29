#include "Krystal.Lib.Tests/Pointers/IntrusivePtr/Mocks.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include <atomic>
#include <catch_all.hpp>
#include <type_traits>

namespace Krys::Tests
{
  TEST_CASE("IntrusivePtr - Atomic type traits are correct")
  {
    using ptr = std::atomic<mock_ptr<instrumented_counted<1>>>;

    SECTION("Construction, destruction and assignment")
    {
      CHECK(sizeof(ptr) == sizeof(std::atomic<instrumented_counted<> *>));
      CHECK(AlignmentOf<ptr> == AlignmentOf<std::atomic<instrumented_counted<> *>>);

      CHECK(DefaultConstructible<ptr>);
      CHECK(NoThrowDefaultConstructible<ptr>);
      CHECK(!TriviallyDefaultConstructible<ptr>);

      CHECK(!CopyConstructible<ptr>);
      CHECK(!MoveConstructible<ptr>);

      CHECK(!CopyAssignable<ptr>);
      CHECK(!MoveAssignable<ptr>);

      CHECK(!Swappable<ptr>);

      CHECK(Destructible<ptr>);
      CHECK(!TriviallyDestructible<ptr>);
      CHECK(NoThrowDestructible<ptr>);

      CHECK(!Assignable<ptr, ptr>);

      CHECK(Constructible<ptr, mock_ptr<instrumented_counted<1>>>);
      CHECK(NoThrowConstructible<ptr, mock_ptr<instrumented_counted<1>>>);
      CHECK(!TriviallyConstructible<ptr, mock_ptr<instrumented_counted<1>>>);

      CHECK(Assignable<ptr, mock_ptr<instrumented_counted<1>>>);
      CHECK(NoThrowAssignable<ptr, mock_ptr<instrumented_counted<1>>>);
      CHECK(!TriviallyAssignable<ptr, mock_ptr<instrumented_counted<1>>>);

      CHECK(ConvertibleTo<ptr, mock_ptr<instrumented_counted<1>>>);
      // CHECK( std::is_nothrow_convertible_v<ptr, IntrusivePtr<instrumented_counted<>>> );
    }
  }

  TEST_CASE("IntrusivePtr - Atomic load")
  {
    SECTION("Explicit")
    {
      instrumented_counted<> object;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object);
      auto ptr1 = ptr.load();
      CHECK(ptr1.get() == &object);
      CHECK(object.count == 2);
    }

    SECTION("Implicit")
    {
      instrumented_counted<> object;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object);
      mock_ptr<instrumented_counted<1>> ptr1 = ptr;
      CHECK(ptr1.get() == &object);
      CHECK(object.count == 2);
    }

    SECTION("Free function")
    {
      instrumented_counted<> object;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object);
      mock_ptr<instrumented_counted<1>> ptr1 = std::atomic_load(&ptr);
      CHECK(ptr1.get() == &object);
      CHECK(object.count == 2);
    }
  }

  TEST_CASE("IntrusivePtr - Atomic store")
  {
    SECTION("Explicit")
    {
      instrumented_counted<> object1, object2;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object1);
      auto ptr1 = mock_noref(&object2);

      ptr.store(ptr1);
      CHECK(ptr1.get() == &object2);
      CHECK(ptr.load().get() == &object2);
      CHECK(object1.count == -1);
      CHECK(object2.count == 2);
    }

    SECTION("Implicit")
    {
      instrumented_counted<> object1, object2;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object1);
      auto ptr1 = mock_noref(&object2);

      ptr = ptr1;
      CHECK(ptr1.get() == &object2);
      CHECK(ptr.load().get() == &object2);
      CHECK(object1.count == -1);
      CHECK(object2.count == 2);
    }

    SECTION("Free function")
    {
      instrumented_counted<> object1, object2;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr = mock_noref(&object1);
      auto ptr1 = mock_noref(&object2);

      std::atomic_store(&ptr, ptr1);
      CHECK(ptr1.get() == &object2);
      CHECK(ptr.load().get() == &object2);
      CHECK(object1.count == -1);
      CHECK(object2.count == 2);
    }
  }

  TEST_CASE("IntrusivePtr - Atomic compare and exchange")
  {
    SECTION("Strong")
    {
      instrumented_counted<> object1, object2, object3;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr1 = mock_noref(&object1);
      auto ptr2 = mock_noref(&object2);
      auto ptr3 = mock_noref(&object3);

      auto res = ptr1.compare_exchange_strong(ptr2, ptr3);
      CHECK(!res);
      CHECK(ptr1.load().get() == &object1);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 2);
      CHECK(object2.count == -1);
      CHECK(object3.count == 1);

      res = ptr1.compare_exchange_strong(ptr2, ptr3);
      CHECK(res);
      CHECK(ptr1.load().get() == &object3);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 1);
      CHECK(object3.count == 2);
    }

    SECTION("Strong 2 arg")
    {
      instrumented_counted<> object1, object2, object3;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr1 = mock_noref(&object1);
      auto ptr2 = mock_noref(&object2);
      auto ptr3 = mock_noref(&object3);

      auto res =
        ptr1.compare_exchange_strong(ptr2, ptr3, std::memory_order_seq_cst, std::memory_order_seq_cst);
      CHECK(!res);
      CHECK(ptr1.load().get() == &object1);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 2);
      CHECK(object2.count == -1);
      CHECK(object3.count == 1);

      res = ptr1.compare_exchange_strong(ptr2, ptr3, std::memory_order_seq_cst, std::memory_order_seq_cst);
      CHECK(res);
      CHECK(ptr1.load().get() == &object3);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 1);
      CHECK(object3.count == 2);
    }

    SECTION("Weak")
    {
      instrumented_counted<> object1, object2, object3;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr1 = mock_noref(&object1);
      auto ptr2 = mock_noref(&object2);
      auto ptr3 = mock_noref(&object3);

      auto res = ptr1.compare_exchange_weak(ptr2, ptr3);
      CHECK(!res);
      CHECK(ptr1.load().get() == &object1);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 2);
      CHECK(object2.count == -1);
      CHECK(object3.count == 1);

      res = ptr1.compare_exchange_weak(ptr2, ptr3);
      CHECK(res);
      CHECK(ptr1.load().get() == &object3);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 1);
      CHECK(object3.count == 2);
    }

    SECTION("Weak 2 arg")
    {
      instrumented_counted<> object1, object2, object3;
      std::atomic<mock_ptr<instrumented_counted<>>> ptr1 = mock_noref(&object1);
      auto ptr2 = mock_noref(&object2);
      auto ptr3 = mock_noref(&object3);

      auto res = ptr1.compare_exchange_weak(ptr2, ptr3, std::memory_order_seq_cst, std::memory_order_seq_cst);
      CHECK(!res);
      CHECK(ptr1.load().get() == &object1);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 2);
      CHECK(object2.count == -1);
      CHECK(object3.count == 1);

      res = ptr1.compare_exchange_weak(ptr2, ptr3, std::memory_order_seq_cst, std::memory_order_seq_cst);
      CHECK(res);
      CHECK(ptr1.load().get() == &object3);
      CHECK(ptr2.get() == &object1);
      CHECK(ptr3.get() == &object3);
      CHECK(object1.count == 1);
      CHECK(object3.count == 2);
    }
  }
}