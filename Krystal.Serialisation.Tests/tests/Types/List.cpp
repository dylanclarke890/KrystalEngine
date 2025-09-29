#include "Krystal.Serialisation/Types/List.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/ISerialiser.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("Serialise(Vector)", "[Serialisation][Types]")
  {
    SECTION("List<int>")
    {
      using Type = std::vector<int>;

      List<byte> buffer(sizeof(Type));

      Type type;
      type.push_back(1);

      {
        IO::MemoryStreamWriter stream(buffer);
        ISerialiser<BinaryArchiveWriter> serialiser(stream);
        serialiser.Serialise(type);
      }

      Type deserialised;
      {
        IO::MemoryStreamReader stream(buffer);
        IDeserialiser<BinaryArchiveReader> deserialiser(stream);
        deserialiser.Deserialise(deserialised);
      }

      REQUIRE(deserialised.size() == 1);
      REQUIRE(deserialised[0] == 1);
    }

    SECTION("List<string>")
    {
      using Type = std::vector<string>;

      List<byte> buffer(sizeof(Type));

      Type type;
      type.push_back("1");

      {
        IO::MemoryStreamWriter stream(buffer);
        ISerialiser<BinaryArchiveWriter> serialiser(stream);
        serialiser.Serialise(type);
      }

      Type deserialised;
      {
        IO::MemoryStreamReader stream(buffer);
        IDeserialiser<BinaryArchiveReader> deserialiser(stream);
        deserialiser.Deserialise(deserialised);
      }

      REQUIRE(deserialised.size() == 1);
      REQUIRE(deserialised[0] == "1");
    }
  }
}