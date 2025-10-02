#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.Serialisation/Types/List.hpp"
#include <catch_all.hpp>

namespace Krys::Serialisation
{
  struct FreeTransfer
  {
    int Int;
    float Float;
  };

  template <typename Archive>
  void Transfer(Archive &archive, FreeTransfer &obj) noexcept
  {
    archive(obj.Int);
    archive(obj.Float);
  }

  struct FreeSaveLoad
  {
    int Int;
    float Float;
  };

  template <typename Archive>
  void Load(Archive &archive, FreeSaveLoad &obj) noexcept
  {
    archive(obj.Int, obj.Float);
  }

  template <typename Archive>
  void Save(Archive &archive, const FreeSaveLoad &obj) noexcept
  {
    archive(obj.Int, obj.Float);
  }

  struct MemberPublicTransfer
  {
    int Int;
    float Float;

    template <typename Archive>
    void Transfer(Archive &archive) noexcept
    {
      archive(KRYS_NAMED_FIELD(Int), KRYS_NAMED_FIELD(Float));
    }
  };

  struct MemberPrivateSaveLoad
  {
    MemberPrivateSaveLoad() = default;

    MemberPrivateSaveLoad(int i, float f) : Int(i), Float(f)
    {
    }

    bool operator==(const MemberPrivateSaveLoad &other) const noexcept
    {
      return Int == other.Int && Float == other.Float;
    }

  private:
    friend class Access;

    int Int;
    float Float;

    template <typename Archive>
    void Load(Archive &archive) noexcept
    {
      archive(KRYS_NAMED_FIELD(Int), KRYS_NAMED_FIELD(Float));
    }

    template <typename Archive>
    void Save(Archive &archive) const noexcept
    {
      archive(KRYS_NAMED_FIELD(Int), KRYS_NAMED_FIELD(Float));
    }
  };

  struct TestStruct
  {
    int Int;
    float Float;
    bool Bool;
    byte Byte;
    string String;
    FreeTransfer NestedA;
    MemberPublicTransfer NestedB;
    FreeSaveLoad NestedC;
    MemberPrivateSaveLoad NestedD;
    List<int> Ints;

    template <typename Archive>
    void Save(Archive &archive) const noexcept
    {
      archive(Int, Float, Bool, Byte, String, NestedA, NestedB, NestedC, NestedD, Ints);
    }
  };

  template <typename Archive>
  void Load(Archive &archive, TestStruct &obj) noexcept
  {
    archive(obj.Int, obj.Float, obj.Bool, obj.Byte, obj.String, obj.NestedA, obj.NestedB, obj.NestedC,
            obj.NestedD, obj.Ints);
  }
}

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  TEST_CASE("JsonArchiveWriter", "[JsonArchive]")
  {
    TestStruct input {
      .Int = 42,
      .Float = 3.14f,
      .Bool = true,
      .Byte = byte {0x7F},
      .String = "Hello, World!",
      .NestedA = {100, 1.23f},
      .NestedB = {200, 4.56f},
      .NestedC = {300, 7.89f},
      .NestedD = {400, 0.12f},
      .Ints = {1, 2, 3, 4, 5},
    };
    TestStruct output {};
    List<byte> data;

    {
      IO::MemoryStreamWriter stream(data);
      JsonArchiveWriter archive(stream);
      archive(KRYS_NAMED_FIELD(input));
    }
    string json((const char *)data.data(), data.size());

    {
      IO::MemoryStreamReader stream(data);
      JsonArchiveReader archive(stream);
      archive(output);
    }

    REQUIRE(input.Int == output.Int);
    REQUIRE(input.Float == output.Float);
    REQUIRE(input.Bool == output.Bool);
    REQUIRE(input.Byte == output.Byte);
    REQUIRE(input.String == output.String);
    REQUIRE(input.NestedA.Int == output.NestedA.Int);
    REQUIRE(input.NestedA.Float == output.NestedA.Float);
    REQUIRE(input.NestedB.Int == output.NestedB.Int);
    REQUIRE(input.NestedB.Float == output.NestedB.Float);
    REQUIRE(input.NestedC.Int == output.NestedC.Int);
    REQUIRE(input.NestedC.Float == output.NestedC.Float);
    REQUIRE(input.NestedD == output.NestedD);
    REQUIRE(input.Ints.size() == output.Ints.size());
    for (size_t i = 0; i < input.Ints.size(); i++)
    {
      REQUIRE(input.Ints[i] == output.Ints[i]);
    }
  }
}