#include "Krystal.Serialisation/Types/Array.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/ISerialiser.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("Serialise(Array)", "[Serialisation][Types]")
  {
    SECTION("Array<int>")
    {
      using Type = std::array<int, 1>;

      List<byte> buffer(sizeof(Type));

      Type type {};
      type[0] = 1;

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

    SECTION("Array<string>")
    {
      using Type = std::array<string, 1>;

      List<byte> buffer(sizeof(Type));

      Type type {};
      type[0] = "1";

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