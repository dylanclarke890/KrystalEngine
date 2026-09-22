#include "Krystal.Core/Serialisation/Builtins.hpp"
#include "Krystal.Core/IO/Streams/MemoryStream.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Serialisation/Access.hpp"
#include "Krystal.Core/Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Core/Serialisation/Archives/XmlArchive.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::tests
{
  using namespace krys::Serialisation;

#pragma region Setup

  struct NestedValue
  {
    int Value;

    template <typename Archive>
    void Transfer(Archive &archive) noexcept
    {
      archive(Value);
    }
  };

  struct BasicTypes
  {
    bool Bool;
    byte Byte;
    char Char;
    uchar Uchar;
    short Short;
    ushort Ushort;
    int8 Int8;
    uint8 Uint8;
    int16 Int16;
    uint16 Uint16;
    int32 Int32;
    uint32 Uint32;
    int64 Int64;
    uint64 Uint64;
    long Long;
    ulong Ulong;
    long long LongLong;
    unsigned long long UlongLong;
    float Float;
    double Double;
    long double LongDouble;
    string String;
    int IntArray[4];
    float FloatArray[4];
    NestedValue Nested {};

    template <typename Archive>
    void Transfer(Archive &archive) noexcept
    {
      archive(KRYS_NAMED_FIELD(Bool), Byte, KRYS_NAMED_FIELD(Char), Uchar, KRYS_NAMED_FIELD(Short), Ushort);
      archive(KRYS_NAMED_FIELD(Int8), Uint8, KRYS_NAMED_FIELD(Int16), Uint16, KRYS_NAMED_FIELD(Int32),
              Uint32);
      archive(KRYS_NAMED_FIELD(Int64), Uint64, KRYS_NAMED_FIELD(Long), Ulong, KRYS_NAMED_FIELD(LongLong),
              UlongLong);
      archive(KRYS_NAMED_FIELD(Float), Double, KRYS_NAMED_FIELD(LongDouble), String,
              KRYS_NAMED_FIELD(IntArray), FloatArray, Nested);
    }
  };

  template <IsArchiveReader ArchiveReader, IsArchiveWriter ArchiveWriter>
  void TestArchiveBuiltins()
  {
    BasicTypes input {
      .Bool = true,
      .Byte = byte {127},
      .Char = 'A',
      .Uchar = 'Z',
      .Short = -12'345,
      .Ushort = 12'345,
      .Int8 = -120,
      .Uint8 = 250,
      .Int16 = -12'345,
      .Uint16 = 12'345,
      .Int32 = -123,
      .Uint32 = 123,
      .Int64 = -1'234,
      .Uint64 = 1'234,
      .Long = -12'345,
      .Ulong = 12'345,
      .LongLong = -123'456,
      .UlongLong = 123'456,
      .Float = 3.14f,
      .Double = 3.123412123123,
      .LongDouble = 3.1234121231231234,
      .String = "Hello, world!",
      .IntArray = {1, 2, 3, 4},
      .FloatArray = {1.5f, 3.f, 4.5f, 6.f},
      .Nested = {5},
    };
    BasicTypes output {};
    List<byte> data(sizeof(BasicTypes));

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

    REQUIRE(input.Bool == output.Bool);
    REQUIRE(input.Byte == output.Byte);
    REQUIRE(input.Char == output.Char);
    REQUIRE(input.Uchar == output.Uchar);
    REQUIRE(input.Short == output.Short);
    REQUIRE(input.Ushort == output.Ushort);
    REQUIRE(input.Int8 == output.Int8);
    REQUIRE(input.Uint8 == output.Uint8);
    REQUIRE(input.Int16 == output.Int16);
    REQUIRE(input.Uint16 == output.Uint16);
    REQUIRE(input.Int32 == output.Int32);
    REQUIRE(input.Uint32 == output.Uint32);
    REQUIRE(input.Int64 == output.Int64);
    REQUIRE(input.Uint64 == output.Uint64);
    REQUIRE(input.Long == output.Long);
    REQUIRE(input.Ulong == output.Ulong);
    REQUIRE(input.LongLong == output.LongLong);
    REQUIRE(input.UlongLong == output.UlongLong);
    REQUIRE_THAT(input.Float, Catch::Matchers::WithinRel(output.Float));
    REQUIRE_THAT(input.Double, Catch::Matchers::WithinRel(output.Double));
    REQUIRE_THAT(input.LongDouble, Catch::Matchers::WithinRel(static_cast<double>(output.LongDouble)));
    REQUIRE(input.String == output.String);
    REQUIRE(input.IntArray[0] == output.IntArray[0]);
    REQUIRE(input.IntArray[1] == output.IntArray[1]);
    REQUIRE(input.IntArray[2] == output.IntArray[2]);
    REQUIRE(input.IntArray[3] == output.IntArray[3]);
    REQUIRE(input.FloatArray[0] == output.FloatArray[0]);
    REQUIRE(input.FloatArray[1] == output.FloatArray[1]);
    REQUIRE(input.FloatArray[2] == output.FloatArray[2]);
    REQUIRE(input.FloatArray[3] == output.FloatArray[3]);
    REQUIRE(input.Nested.Value == output.Nested.Value);
  }

#pragma endregion

  TEST_CASE("BinaryArchive Builtins", "[BinaryArchive][Builtins]")
  {
    TestArchiveBuiltins<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Builtins", "[JsonArchive][Builtins]")
  {
    TestArchiveBuiltins<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Builtins", "[XmlArchive][Builtins]")
  {
    TestArchiveBuiltins<XmlArchiveReader, XmlArchiveWriter>();
  }
}