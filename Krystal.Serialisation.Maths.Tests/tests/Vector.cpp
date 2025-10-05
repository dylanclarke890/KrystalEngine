#include "Krystal.Serialisation.Maths/Vector.hpp"
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
  void TestArchiveTypeVector()
  {
    Krys::Maths::Vector<int, 4> input {1, 2, 3, 4};
    Krys::Maths::Vector<int, 4> output {};
    List<byte> data {};

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

    REQUIRE(input.x == output.x);
    REQUIRE(input.y == output.y);
    REQUIRE(input.z == output.z);
    REQUIRE(input.w == output.w);
  }

#pragma endregion

  TEST_CASE("BinaryArchive Type: Vector", "[BinaryArchive][Types]")
  {
    TestArchiveTypeVector<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: Vector", "[JsonArchive][Types]")
  {
    TestArchiveTypeVector<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Type: Vector", "[XmlArchive][Types]")
  {
    TestArchiveTypeVector<XmlArchiveReader, XmlArchiveWriter>();
  }
}