#include "Krystal.Booey/Graphics/TextRunIterator.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("TextRunIterator: steps by code point", "[Graphics][TextRunIterator]")
  {
    // One, two, three and four bytes.
    const utf8_stringview text = u8"aéא\U0001F600";
    TextRun run(text);

    TextRunIterator iterator(&run, 0);
    CHECK(iterator.Offset() == 0);
    CHECK(iterator.Current() == U'a');
    CHECK(iterator.Direction() == U_LEFT_TO_RIGHT);
    CHECK(iterator.NextOffset() == 1);

    iterator.Increment();
    CHECK(iterator.Offset() == 1);
    CHECK(iterator.Current() == U'é');
    CHECK(iterator.NextOffset() == 3);

    iterator.Increment();
    CHECK(iterator.Current() == U'א');
    CHECK(iterator.Direction() == U_RIGHT_TO_LEFT);
    CHECK(iterator.NextOffset() == 5);

    iterator.Increment();
    CHECK(iterator.Current() == U'\U0001F600');
    CHECK(iterator.NextOffset() == 9);
    CHECK_FALSE(iterator.AtEnd());

    iterator.Increment();
    CHECK(iterator.AtEnd());
    CHECK(iterator == TextRunIterator(&run, run.Length()));
    CHECK(iterator.Direction() == U_OTHER_NEUTRAL);

    // An iterator without a run is at its end.
    CHECK(TextRunIterator().AtEnd());
  }
}
