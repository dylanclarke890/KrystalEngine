#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include "Krystal.Lib.Tests/Pointers/RefCounted/TestRefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
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
    REQUIRE(object->GetRefCountDebugger().DeletionHasBegun());
  }
}