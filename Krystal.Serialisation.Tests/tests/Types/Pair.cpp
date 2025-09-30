#include "Krystal.Serialisation/Types/Pair.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("Serialise(Pair)", "[Serialisation][Types]")
  {
    SECTION("Pair<int, int>")
    {
      using Type = Pair<int, int>;

      List<byte> buffer(sizeof(Type));

      Type type(1, 2);
      {
        IO::MemoryStreamWriter stream(buffer);
        BinaryArchiveWriter archive(stream);
        archive(type);
      }

      Type deserialised;
      {
        IO::MemoryStreamReader stream(buffer);
        BinaryArchiveReader archive(stream);
        archive(deserialised);
      }

      REQUIRE(deserialised.first == type.first);
      REQUIRE(deserialised.second == type.second);
    }

    SECTION("Pair<std::string, int>")
    {
      using Type = Pair<string, int>;

      List<byte> buffer(sizeof(Type));

      Type type("Test", 2);
      {
        IO::MemoryStreamWriter stream(buffer);
        BinaryArchiveWriter archive(stream);
        archive(type);
      }

      Type deserialised;
      {
        IO::MemoryStreamReader stream(buffer);
        BinaryArchiveReader archive(stream);
        archive(deserialised);
      }

      REQUIRE(deserialised.first == type.first);
      REQUIRE(deserialised.second == type.second);
    }
  }
}