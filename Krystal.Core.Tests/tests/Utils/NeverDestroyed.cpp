#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  namespace
  {
    struct Counter
    {
      int Value {0};

      explicit Counter(int value) noexcept : Value(value)
      {
      }
    };

    LazyNeverDestroyed<Counter> LazyCounter;
  }

  TEST_CASE("NeverDestroyed", "[Utils][NeverDestroyed]")
  {
    static NeverDestroyed<Counter> counter(3);
    CHECK(counter.get().Value == 3);
    CHECK(counter->Value == 3);

    counter->Value = 4;
    const auto &constCounter = counter;
    CHECK(constCounter.get().Value == 4);
  }

  TEST_CASE("LazyNeverDestroyed", "[Utils][NeverDestroyed]")
  {
#if KRYS_DEBUG
    CHECK_FALSE(LazyCounter.IsConstructed());
#endif

    LazyCounter.Construct(7);

#if KRYS_DEBUG
    CHECK(LazyCounter.IsConstructed());
#endif
    CHECK(LazyCounter.get().Value == 7);
    CHECK(LazyCounter->Value == 7);
  }
}
