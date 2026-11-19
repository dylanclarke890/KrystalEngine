#include "Krystal.Core/Strings/StringAtom.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Threading/Thread.hpp"

namespace krys::tests
{
  TEST_CASE("StringAtom - different strings do not compare equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = u8"world";

    REQUIRE(str1 != str2);
  }

  TEST_CASE("StringAtom - the same string compares equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = str1;

    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - different strings with the same text compare equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = u8"hello";

    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - empty strings compare equal", "[StringAtom]")
  {
    StringAtom str1 = StringAtom::Empty();
    StringAtom str2 = u8"";
    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - View returns correct string", "[StringAtom]")
  {
    utf8_stringview text = u8"example";
    StringAtom atomStr {text};
    REQUIRE(atomStr.View() == text);
  }

  TEST_CASE("StringAtom - ToString copies the string", "[StringAtom]")
  {
    StringAtom atom = u8"copied";
    CHECK(atom.ToString() == u8"copied");
    CHECK(StringAtom().ToString().empty());
  }

  TEST_CASE("StringAtom - incrementally created string compares equal", "[StringAtom]")
  {
    utf8_string original = u8"incremental";
    utf8_string temp = original.substr(0, 5); // "incre"
    temp += original.substr(5);               // "mental"
    StringAtom atomStr1 {original};
    StringAtom atomStr2 {krys::move(temp)};
    REQUIRE(atomStr1 == atomStr2);
  }

  TEST_CASE("StringAtom - the null atom reads as empty", "[StringAtom]")
  {
    REQUIRE(StringAtom {}.View().empty());
    REQUIRE(StringAtom {}.Storage() == nullptr);
    REQUIRE_FALSE(null == StringAtom::Empty());
  }

  TEST_CASE("StringAtom - null makes and compares with the null atom", "[StringAtom]")
  {
    StringAtom atom = null;
    CHECK(atom == null);
    CHECK(null == atom);
    CHECK(atom == StringAtom());

    atom = u8"named";
    CHECK(atom != null);
    atom = null;
    CHECK(atom == null);
  }

  TEST_CASE("StringAtom - each thread interns into its own pool", "[StringAtom]")
  {
    StringAtom mainAtom = u8"per-thread";
    bool sameOnOtherThread = true;
    bool equalWithinOtherThread = false;
    {
      Thread thread(u8"StringAtom test",
                    [&]()
                    {
                      StringAtom first = u8"per-thread";
                      StringAtom second = u8"per-thread";
                      sameOnOtherThread = first.Storage() == mainAtom.Storage();
                      equalWithinOtherThread = first == second;
                    });
    }

    CHECK_FALSE(sameOnOtherThread);
    CHECK(equalWithinOtherThread);
  }

  TEST_CASE("StringAtom - a string leaves its pool with its last atom", "[StringAtom]")
  {
    auto &pool = StringAtomPool::Current();
    auto before = pool.Size();
    {
      StringAtom first = u8"short-lived";
      CHECK(pool.Size() == before + 1);
      {
        StringAtom copy = first;
        StringAtom moved = krys::move(copy);
        CHECK(moved == first);
        CHECK(copy == null);
      }
      CHECK(pool.Size() == before + 1);
    }
    CHECK(pool.Size() == before);
  }

  TEST_CASE("StringAtom - storage given up and taken back keeps the string", "[StringAtom]")
  {
    auto &pool = StringAtomPool::Current();
    auto before = pool.Size();
    StringAtomStorage storage = StringAtom(u8"stored").LeakStorage();
    CHECK(pool.Size() == before + 1);
    StringAtom::RefStorage(storage);
    StringAtom::DerefStorage(storage);
    {
      auto atom = StringAtom::AdoptStorage(storage);
      CHECK(atom.View() == u8"stored");
    }
    CHECK(pool.Size() == before);
  }

  TEST_CASE("StringAtom - comparing with a string doesn't intern it", "[StringAtom]")
  {
    auto &pool = StringAtomPool::Current();
    StringAtom atom = u8"compared";
    auto before = pool.Size();

    CHECK(atom == u8"compared");
    CHECK(atom != u8"never-interned");
    CHECK(atom == utf8_string(u8"compared"));
    CHECK(utf8_stringview {u8"other"} != atom);
    CHECK(pool.Size() == before);

    CHECK(StringAtom() != u8"");
    CHECK(StringAtom::Empty() == u8"");
  }

  TEST_CASE("StringAtom - ToASCIILower and ToASCIIUpper convert ASCII letters", "[StringAtom]")
  {
    StringAtom mixed = u8"MiXeD-É";
    CHECK(mixed.ToASCIILower() == u8"mixed-É");
    CHECK(mixed.ToASCIIUpper() == u8"MIXED-É");
    CHECK(StringAtom().ToASCIILower() == null);
  }

  TEST_CASE("StringAtom - converting the case of an atom that has none to convert gives the same atom",
            "[StringAtom]")
  {
    auto &pool = StringAtomPool::Current();
    StringAtom lower = u8"already-lower";
    StringAtom upper = u8"ALREADY-UPPER";
    auto before = pool.Size();

    CHECK(lower.ToASCIILower().Storage() == lower.Storage());
    CHECK(upper.ToASCIIUpper().Storage() == upper.Storage());
    CHECK(pool.Size() == before);
  }
}
