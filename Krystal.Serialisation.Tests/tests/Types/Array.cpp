#include "Krystal.Serialisation/Types/Array.hpp"
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
  void TestArchiveTypeArray()
  {
    Array<int, 5> input {1, 2, 3, 4, 5};
    Array<int, 5> output {0, 0, 0, 0, 0};
    List<byte> data(input.size() * sizeof(int));

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

    for (size_t i = 0; i < input.size(); ++i)
    {
      REQUIRE(input[i] == output[i]);
      REQUIRE(input[i] == output[i]);
    }
  }

#pragma endregion

  TEST_CASE("BinaryArchive Type: Array", "[BinaryArchive][Types]")
  {
    TestArchiveTypeArray<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: Array", "[JsonArchive][Types]")
  {
    TestArchiveTypeArray<JsonArchiveReader, JsonArchiveWriter>();
  }
}