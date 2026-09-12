#include "Krystal.Core/Serialisation/Types/Array.hpp"
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
  void TestArchiveTypeArray()
  {
    Array<int, 5> input {1, 2, 3, 4, 5};
    Array<int, 5> output {0, 0, 0, 0, 0};
    List<byte> data(input.size() * sizeof(int));

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

  TEST_CASE("XmlArchive Type: Array", "[XmlArchive][Types]")
  {
    TestArchiveTypeArray<XmlArchiveReader, XmlArchiveWriter>();
  }
}