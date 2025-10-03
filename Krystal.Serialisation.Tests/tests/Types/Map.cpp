#include "Krystal.Serialisation/Types/Map.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.Serialisation/Types/List.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

#pragma region Setup

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveTypeMap()
  {
    Map<int, string> input {{1, "3"}, {4, "12"}};
    Map<int, string> output {};
    List<byte> data;

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

    for (const auto &[inputKey, inputValue] : input)
    {
      REQUIRE(input[inputKey] == output[inputKey]);
    }
  }

#pragma endregion

  TEST_CASE("BinaryArchive Type: Map", "[BinaryArchive][Types]")
  {
    TestArchiveTypeMap<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: Map", "[JsonArchive][Types]")
  {
    TestArchiveTypeMap<JsonArchiveReader, JsonArchiveWriter>();
  }
}