#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Serialisation/ISerialiser.hpp"
#include "Krystal.Serialisation/Types/String.hpp"
#include <catch_all.hpp>

namespace Krys::Serialisation
{
  struct TestStruct
  {
    int IntValue;
    float FloatValue;
    bool BoolValue;
    byte ByteValue;
    string StringValue;
  };

  template <typename Archive>
  void Save(Archive &archive, const TestStruct &obj) noexcept
  {
    archive(obj.IntValue);
    archive(obj.FloatValue);
    archive(obj.BoolValue);
    archive(obj.ByteValue);
    archive(obj.StringValue);
  }

  template <typename Archive>
  void Load(Archive &archive, TestStruct &obj) noexcept
  {
    archive(obj.IntValue);
    archive(obj.FloatValue);
    archive(obj.BoolValue);
    archive(obj.ByteValue);
    archive(obj.StringValue);
  }
}

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
    TestStruct testStruct {iValue, fValue, bValue, byteValue, strValue};

    List<byte> data(1'024);

    {
      IO::MemoryStreamWriter stream(data);
      ISerialiser<BinaryArchiveWriter> serialiser(stream);

      serialiser.Serialise(iValue);
      serialiser.Serialise(fValue);
      serialiser.Serialise(bValue);
      serialiser.Serialise(byteValue);
      serialiser.Serialise(strValue);
      serialiser.Serialise(testStruct);
    }

    {
      int iValueOut = 0;
      float fValueOut = 0.0f;
      bool bValueOut = false;
      byte byteValueOut = byte {0};
      string strValueOut;
      TestStruct testStructOut {};

      IO::MemoryStreamReader stream(data);
      IDeserialiser<BinaryArchiveReader> deserialiser(stream);

      deserialiser.Deserialise(iValueOut);
      deserialiser.Deserialise(fValueOut);
      deserialiser.Deserialise(bValueOut);
      deserialiser.Deserialise(byteValueOut);
      deserialiser.Deserialise(strValueOut);
      deserialiser.Deserialise(testStructOut);

      REQUIRE(iValueOut == iValue);
      REQUIRE_THAT(fValueOut, Catch::Matchers::WithinRel(fValue));
      REQUIRE(bValueOut == bValue);
      REQUIRE(byteValueOut == byteValue);
      REQUIRE(strValueOut == strValue);
      REQUIRE(testStructOut.IntValue == testStruct.IntValue);
      REQUIRE_THAT(testStructOut.FloatValue, Catch::Matchers::WithinRel(testStruct.FloatValue));
      REQUIRE(testStructOut.BoolValue == testStruct.BoolValue);
      REQUIRE(testStructOut.ByteValue == testStruct.ByteValue);
      REQUIRE(testStructOut.StringValue == testStruct.StringValue);
    }
  }
}