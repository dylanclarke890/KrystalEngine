#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include "Krystal.Serialisation/Archives/XmlArchive.hpp"
#include "Krystal.Serialisation/Types/List.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

  struct VersionedType1
  {
    int A {0};
    float B {0};

    KRYS_CLASS_VERSION(1);
  };

  template <typename Archive>
  void Transfer(Archive &archive, VersionedType1 &value, const Version) noexcept
  {
    archive(value.A, value.B);
  }

  struct VersionedType2
  {
    int A {0};
    float B {0};
    string C {};

    KRYS_CLASS_VERSION(2);

    template <typename Archive>
    void Transfer(Archive &archive, const Version version) noexcept
    {
      archive(A, B);

      if (version >= 2)
      {
        archive(C);
      }
    }
  };

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveVersioning()
  {
    VersionedType1 input1 {.A = 42, .B = 3.14f};
    VersionedType2 output {};
    List<byte> data;

    // static_assert(IsVersioned<VersionedType1>);
    // static_assert(HasVersionedTransferMember<ArchiveReader, VersionedType1>);
    // static_assert(HasVersionedTransferMember<ArchiveWriter, VersionedType1>);
    // static_assert(HasVersionedTransferMember<ArchiveReader, VersionedType2>);
    // static_assert(HasVersionedTransferMember<ArchiveWriter, VersionedType2>);

    {
      IO::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(KRYS_NAMED_FIELD(input1));
    }

    {
      IO::MemoryStreamReader stream(data);
      ArchiveReader archive(stream);
      archive(output);
    }

    REQUIRE(input1.A == output.A);
    REQUIRE(input1.B == output.B);
    REQUIRE(output.C.empty());

    string xml = string((char *)data.data(), data.size());

    data.clear();
    VersionedType2 input2 {.A = 69, .B = 23.48f, .C = "Hello world!"};

    {
      IO::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(input2);
    }

    {
      IO::MemoryStreamReader stream(data);
      ArchiveReader archive(stream);
      archive(output);
    }

    REQUIRE(input2.A == output.A);
    REQUIRE(input2.B == output.B);
    REQUIRE(input2.C == output.C);
  }

  TEST_CASE("BinaryArchive Versioning", "[BinaryArchive][Versioning]")
  {
    TestArchiveVersioning<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Versioning", "[JsonArchive][Versioning]")
  {
    TestArchiveVersioning<JsonArchiveReader, JsonArchiveWriter>();
  }

  TEST_CASE("XmlArchive Versioning", "[XmlArchive][Versioning]")
  {
    TestArchiveVersioning<XmlArchiveReader, XmlArchiveWriter>();
  }
}