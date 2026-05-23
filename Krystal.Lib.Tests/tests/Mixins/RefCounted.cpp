#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  class TestRefCounted : public RefCounted<TestRefCounted>
  {
  public:
    TestRefCounted() = default;
    virtual ~TestRefCounted() = default;
  };

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
    REQUIRE(object->GetDebugger().DeletionHasBegun());
  }
}