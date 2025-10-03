#include "Krystal.Serialisation/Access.hpp"
#include "Krystal.IO/Streams/MemoryStream.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Serialisation/Archives/BinaryArchive.hpp"
#include "Krystal.Serialisation/Archives/JsonArchive.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Serialisation;

#pragma region Setup

  struct FreeTransfer
  {
    int A;
    float B;
  };

  template <typename Archive>
  void Transfer(Archive &archive, FreeTransfer &obj) noexcept
  {
    archive(obj.A);
    archive(obj.B);
  }

  struct FreeSaveLoad
  {
    int A;
    float B;
  };

  template <typename Archive>
  void Load(Archive &archive, FreeSaveLoad &obj) noexcept
  {
    archive(obj.A, obj.B);
  }

  template <typename Archive>
  void Save(Archive &archive, const FreeSaveLoad &obj) noexcept
  {
    archive(obj.A, obj.B);
  }

  struct MemberPublicTransfer
  {
    int A;
    float B;

    template <typename Archive>
    void Transfer(Archive &archive) noexcept
    {
      archive(A, B);
    }
  };

  struct MemberPrivateSaveLoad
  {
    int A;
    float B;

  private:
    friend class Access;

    template <typename Archive>
    void Load(Archive &archive) noexcept
    {
      archive(A, B);
    }

    template <typename Archive>
    void Save(Archive &archive) const noexcept
    {
      archive(A, B);
    }
  };

  template <typename ArchiveReader, typename ArchiveWriter>
  void TestArchiveAccess()
  {
    FreeTransfer inputFreeTransfer {69, 420.f};
    FreeSaveLoad inputFreeSaveLoad {69, 420.f};
    MemberPublicTransfer inputMemberTransfer {69, 420.f};
    MemberPrivateSaveLoad inputMemberSaveLoad {69, 420.f};

    FreeTransfer outputFreeTransfer {};
    FreeSaveLoad outputFreeSaveLoad {};
    MemberPublicTransfer outputMemberTransfer {};
    MemberPrivateSaveLoad outputMemberSaveLoad {};

    List<byte> data(sizeof(FreeTransfer) * 4);

    {
      IO::MemoryStreamWriter stream(data);
      ArchiveWriter archive(stream);
      archive(inputFreeTransfer, inputFreeSaveLoad, inputMemberTransfer, inputMemberSaveLoad);
    }

    {
      IO::MemoryStreamReader stream(data);
      ArchiveReader archive(stream);
      archive(outputFreeTransfer, outputFreeSaveLoad, outputMemberTransfer, outputMemberSaveLoad);
    }

    REQUIRE(inputFreeTransfer.A == outputFreeTransfer.A);
    REQUIRE(inputFreeTransfer.B == outputFreeTransfer.B);

    REQUIRE(inputFreeSaveLoad.A == outputFreeSaveLoad.A);
    REQUIRE(inputFreeSaveLoad.B == outputFreeSaveLoad.B);

    REQUIRE(inputMemberTransfer.A == outputMemberTransfer.A);
    REQUIRE(inputMemberTransfer.B == outputMemberTransfer.B);

    REQUIRE(inputMemberSaveLoad.A == outputMemberSaveLoad.A);
    REQUIRE(inputMemberSaveLoad.B == outputMemberSaveLoad.B);
  }

#pragma endregion

  TEST_CASE("BinaryArchive Access", "[BinaryArchive][Access]")
  {
    TestArchiveAccess<BinaryArchiveReader, BinaryArchiveWriter>();
  }

  TEST_CASE("JsonArchive Access", "[JsonArchive][Access]")
  {
    TestArchiveAccess<JsonArchiveReader, JsonArchiveWriter>();
  }
}