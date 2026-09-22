#include "Krystal.Core/Serialisation/Types/Pair.hpp"
#include "Krystal.Core/IO/Streams/MemoryStream.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/XmlArchive.hpp"
#include "Krystal.Core/Serialisation/Builtins.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  using namespace krys::Serialisation;

#pragma region Setup

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveTypePair()
  {
    Pair<int, float> input {42, 3.14f};
    Pair<int, float> output {0, 0.0f};
    List<byte> data(sizeof(Pair<int, float>));

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