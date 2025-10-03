#include "Krystal.Serialisation/Builtins.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

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
    int32 Int32;
    uint32 Uint32;
    int64 Int64;
    uint64 Uint64;
    byte Byte;
    float Float;
    double Double;
    string String;
    int IntArray[4];
    float FloatArray[4];
    NestedValue Nested {};

    template <typename Archive>
    void Transfer(Archive &archive) noexcept
    {
      archive(KRYS_NAMED_FIELD(Bool), Int32, Uint32, Int64, Uint64, Byte, Float, Double, String, IntArray,
              KRYS_NAMED_FIELD(FloatArray), Nested);
    }
  };

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveBuiltins()
  {
    BasicTypes input {
      .Bool = true,
      .Int32 = -123,
      .Uint32 = 123,
      .Int64 = -1'234,
      .Uint64 = 1'234,
      .Byte = byte {127},
      .Float = 3.14f,
      .Double = 3.123412123123,
      .String = "Hello, world!",
      .IntArray = {1, 2, 3, 4},
      .FloatArray = {1.5f, 3.f, 4.5f, 6.f},
      .Nested = {5},
    };
    BasicTypes output {};
    List<byte> data(sizeof(BasicTypes));

    {
      IO::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(input);
    }

    string json {};
    for (auto &ch : data)
      json += static_cast<char>(ch);

    {
      IO::MemoryStreamReader stream(data);
      ArchiveReader archive(stream);
      archive(output);
    }

    REQUIRE(input.Bool == output.Bool);
    REQUIRE(input.Int32 == output.Int32);
    REQUIRE(input.Uint32 == output.Uint32);
    REQUIRE(input.Int64 == output.Int64);
    REQUIRE(input.Uint64 == output.Uint64);
    REQUIRE(input.Byte == output.Byte);
    REQUIRE(input.Float == output.Float);
    REQUIRE(input.Double == output.Double);
    REQUIRE(input.String == output.String);
    for (size_t i = 0; i < 4; ++i)
    {
      REQUIRE(input.IntArray[i] == output.IntArray[i]);
      REQUIRE(input.FloatArray[i] == output.FloatArray[i]);
    }
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
}