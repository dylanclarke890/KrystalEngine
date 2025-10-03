#include "Krystal.Serialisation/Types/Pair.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Serialisation/Archives/XmlArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

#pragma region Setup

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveTypePair()
  {
    Pair<int, float> input {42, 3.14f};
    Pair<int, float> output {0, 0.0f};
    List<byte> data(sizeof(Pair<int, float>));

    {
      IO::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(input);
    }

    {
      IO::MemoryStreamReader stream(data);
      ArchiveReader archive(stream);
      archive(output);
    }

    REQUIRE(input.first == output.first);
    REQUIRE(input.second == output.second);
  }

#pragma endregion

  TEST_CASE("BinaryArchive Type: Pair", "[BinaryArchive][Types]")
  {
    TestArchiveTypePair<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: Pair", "[JsonArchive][Types]")
  {
    TestArchiveTypePair<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Type: Pair", "[XmlArchive][Types]")
  {
    TestArchiveTypePair<XmlArchiveReader, XmlArchiveWriter>();
  }
}