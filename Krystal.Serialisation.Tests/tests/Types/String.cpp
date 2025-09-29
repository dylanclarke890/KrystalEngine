#include "Krystal.Serialisation/Types/String.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/ISerialiser.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("Serialise(string)", "[Serialisation][Types]")
  {
    string inputString = "Hello, World!";
    List<byte> buffer(inputString.size());

    {
      IO::MemoryStreamWriter stream(buffer);
      ISerialiser<BinaryArchiveWriter> serialiser(stream);
      serialiser.Serialise(inputString);
    }

    string deserialised;
    {
      IO::MemoryStreamReader stream(buffer);
      IDeserialiser<BinaryArchiveReader> deserialiser(stream);
      deserialiser.Deserialise(deserialised);
    }

    REQUIRE(deserialised.size() == inputString.size());
    REQUIRE(deserialised == inputString);
  }
}