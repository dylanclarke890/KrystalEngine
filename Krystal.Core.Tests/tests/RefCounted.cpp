#include "Krystal.Core/RefCounted.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  namespace
  {
    class TestRefCounted : public RefCounted<TestRefCounted>
    {
    public:
      /// @brief Whether the last object destroyed had begun deletion first, as it must have.
      static inline bool DestroyedAfterDeletionBegan = false;

      TestRefCounted() noexcept
      {
        DestroyedAfterDeletionBegan = false;
      }

      virtual ~TestRefCounted() noexcept
      {
        DestroyedAfterDeletionBegan = DeletionHasBegun();
      }
    };
  }

  TEST_CASE("RefCounted: manual reference add/sub", "[RefCounted]")
  {
    auto *object = new TestRefCounted();
    REQUIRE(object->GetRefCount() == 1);
    REQUIRE(object->HasOneRef());

    object->AddRef();
    REQUIRE(object->GetRefCount() == 2);
    REQUIRE(!object->HasOneRef());

    object->SubRef();
    REQUIRE(object->GetRefCount() == 1);
    REQUIRE(object->HasOneRef());

    object->SubRef();
    REQUIRE(TestRefCounted::DestroyedAfterDeletionBegan);
  }

  TEST_CASE("RefCounted: costs only its count", "[RefCounted]")
  {
    // The deletion flag is the count's top bit, not a member of its own.
    STATIC_CHECK(sizeof(detail::RefCounted) == sizeof(uint32));
    STATIC_CHECK(sizeof(detail::ThreadSafeRefCounted) == sizeof(uint32));
    STATIC_CHECK(sizeof(detail::RefCountedWithInlineWeakPtrBase) == 2 * sizeof(uint32));

    // The flag doesn't show in the count.
    auto *object = new TestRefCounted();
    object->AddRef();
    object->SubRef();
    CHECK(object->GetRefCount() == 1);
    CHECK(object->HasOneRef());
    CHECK_FALSE(object->DeletionHasBegun());
    object->SubRef();
    CHECK(TestRefCounted::DestroyedAfterDeletionBegan);
  }
}