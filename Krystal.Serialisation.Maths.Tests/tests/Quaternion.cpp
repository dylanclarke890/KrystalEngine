#include "Krystal.Serialisation.Maths/Quaternion.hpp"
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
  void TestArchiveTypeQuaternion()
  {
    Krys::Maths::Quaternion<int> input {1, 2, 3, 4};
    Krys::Maths::Quaternion<int> output {};
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

  TEST_CASE("BinaryArchive Type: Quaternion", "[BinaryArchive][Types]")
  {
    TestArchiveTypeQuaternion<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Type: Quaternion", "[JsonArchive][Types]")
  {
    TestArchiveTypeQuaternion<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Type: Quaternion", "[XmlArchive][Types]")
  {
    TestArchiveTypeQuaternion<XmlArchiveReader, XmlArchiveWriter>();
  }
}