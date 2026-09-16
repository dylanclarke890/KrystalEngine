#include "Krystal.Core/Serialisation/Types/HashMap.hpp"
#include "Krystal.Core/IO/Streams/MemoryStream.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/XmlArchive.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"
#include "Krystal.Core/Serialisation/Types/List.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::Serialisation;

#pragma region Setup

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveTypeMap()
  {
    HashMap<int, string> input {{1, "3"}, {4, "12"}};
    HashMap<int, string> output {};
    List<byte> data;

    {
      io::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(input);
    }

    {
      io::MemoryStreamReader stream(data);
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

  TEST_CASE("XmlArchive Type: Map", "[XmlArchive][Types]")
  {
    TestArchiveTypeMap<XmlArchiveReader, XmlArchiveWriter>();
  }
}