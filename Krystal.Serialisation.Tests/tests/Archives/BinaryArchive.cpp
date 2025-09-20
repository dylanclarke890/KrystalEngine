#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"

#include "catch_all.hpp"

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("BinaryArchiveWriter", "[BinaryArchive]")
  {
    int iValue = 42;
    float fValue = 3.14f;
    bool bValue = true;
    byte byteValue = byte {0x11};
    string strValue = "Hello, World!";

    List<byte> data(1'024);

    {
      IO::MemoryStreamWriter streamWriter(data);
      BinaryArchiveWriter archive(streamWriter);

      archive(iValue);
      archive(fValue);
      archive(bValue);
      archive(byteValue);
      archive(strValue);
    }

    {
      int iValueOut = 0;
      float fValueOut = 0.0f;
      bool bValueOut = false;
      byte byteValueOut = byte {0};
      string strValueOut;

      IO::MemoryStreamReader streamReader(data);
      BinaryArchiveReader archive(streamReader);

      archive(iValueOut);
      archive(fValueOut);
      archive(bValueOut);
      archive(byteValueOut);
      archive(strValueOut);

      REQUIRE(iValueOut == iValue);
      REQUIRE_THAT(fValueOut, Catch::Matchers::WithinRel(fValue));
      REQUIRE(bValueOut == bValue);
      REQUIRE(byteValueOut == byteValue);
      REQUIRE(strValueOut == strValue);
    }
  }
}