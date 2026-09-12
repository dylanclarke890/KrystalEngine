#include "Krystal.Core/Serialisation/Types/List.hpp"
#include "Krystal.Core/IO/Streams/MemoryStream.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/XmlArchive.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::Serialisation;

#pragma region Setup

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveTypeList()
  {
    List<int> input {1, 2, 3, 4, 5, 6, 7, 8, 9};
    List<int> output {};
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

  TEST_CASE("BinaryArchive Type: List", "[BinaryArchive][Types]")
  {
    TestArchiveTypeList<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: List", "[JsonArchive][Types]")
  {
    TestArchiveTypeList<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Type: List", "[XmlArchive][Types]")
  {
    TestArchiveTypeList<XmlArchiveReader, XmlArchiveWriter>();
  }
}