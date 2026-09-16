#include "Krystal.Core/Serialisation/Types/Vector.hpp"
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
  void TestArchiveTypeVector()
  {
    krys::Vector<int, 4> input {1, 2, 3, 4};
    krys::Vector<int, 4> output {};
    List<byte> data {};

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