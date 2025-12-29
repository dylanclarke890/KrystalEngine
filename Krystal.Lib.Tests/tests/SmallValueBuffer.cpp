#include "Krystal.Lib/SmallValueBuffer.hpp"
#include <array>
#include <catch_all.hpp>

namespace Krys::Tests
{
  constexpr size_t kBufferSize = 4;

  TEST_CASE("SmallValueBuffer::copy_assignment_with_overflow", "[SmallValueBuffer]")
  {
    std::array<uint16_t, kBufferSize + 1> handles {};

    SmallValueBuffer<kBufferSize> buffer1;
    for (size_t i = 0; i < kBufferSize + 1; ++i)
    {
      handles[i] = buffer1.Push(static_cast<uint32_t>(i));
    }

    SmallValueBuffer<kBufferSize> buffer2 = buffer1;
    for (size_t i = 0; i < kBufferSize + 1; ++i)
    {
      REQUIRE(buffer2.GetU32(handles[i]) == i);
    }

    auto handle = buffer1.Push(42u);
    REQUIRE(buffer1.GetU32(handle) == 42);

    REQUIRE_THROWS_AS(buffer2.GetU32(handle), std::logic_error);
  }

  TEST_CASE("SmallValueBuffer::push_32", "[SmallValueBuffer]")
  {
    uint32_t magic = 88'567'114u;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic);
    REQUIRE(buffer.GetU32(handle) == magic);
  }

  TEST_CASE("SmallValueBuffer::push_overflow", "[SmallValueBuffer]")
  {
    uint32_t magic1 = 88'567'114u;
    uint32_t magic2 = 351'012'214u;
    uint32_t magic3 = 146'122'128u;
    uint32_t magic4 = 2'171'092'154u;
    uint32_t magic5 = 2'269'016'953u;

    SmallValueBuffer<kBufferSize> buffer;
    REQUIRE(buffer.GetU32(buffer.Push(magic1)) == magic1);
    REQUIRE(buffer.GetU32(buffer.Push(magic2)) == magic2);
    REQUIRE(buffer.GetU32(buffer.Push(magic3)) == magic3);
    REQUIRE(buffer.GetU32(buffer.Push(magic4)) == magic4);
    REQUIRE(buffer.GetU32(buffer.Push(magic5)) == magic5);
  }

  TEST_CASE("SmallValueBuffer::push_64", "[SmallValueBuffer]")
  {
    uint64_t magic = 118'138'934'255'546'108ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic);
    REQUIRE(buffer.GetU64(handle) == magic);
  }

  TEST_CASE("SmallValueBuffer::push_64_overflow", "[SmallValueBuffer]")
  {
    uint64_t magic1 = 1'401'612'388'342'512ull;
    uint64_t magic2 = 118'712'305'386'210ull;
    uint64_t magic3 = 752'431'801'563'359'011ull;
    uint64_t magic4 = 118'138'934'255'546'108ull;
    uint64_t magic5 = 237'115'443'124'116'111ull;

    SmallValueBuffer<kBufferSize> buffer;
    REQUIRE(buffer.GetU64(buffer.Push(magic1)) == magic1);
    REQUIRE(buffer.GetU64(buffer.Push(magic2)) == magic2);
    REQUIRE(buffer.GetU64(buffer.Push(magic3)) == magic3);
    REQUIRE(buffer.GetU64(buffer.Push(magic4)) == magic4);
    REQUIRE(buffer.GetU64(buffer.Push(magic5)) == magic5);
  }

  TEST_CASE("SmallValueBuffer::push_64_after_32", "[SmallValueBuffer]")
  {
    uint32_t magic32 = 88'567'114u;
    uint64_t magic64 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle32 = buffer.Push(magic32);
    REQUIRE(buffer.GetU32(handle32) == magic32);

    auto handle64 = buffer.Push(magic64);
    REQUIRE(buffer.GetU64(handle64) == magic64);
  }

  TEST_CASE("SmallValueBuffer::push_32_after_64", "[SmallValueBuffer]")
  {
    uint32_t magic32 = 88'567'114u;
    uint64_t magic64 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle64 = buffer.Push(magic64);
    REQUIRE(buffer.GetU64(handle64) == magic64);

    auto handle32 = buffer.Push(magic32);
    REQUIRE(buffer.GetU32(handle32) == magic32);
  }

  TEST_CASE("SmallValueBuffer::replace_32_with_32", "[SmallValueBuffer]")
  {
    uint32_t magic1 = 88'567'114u;
    uint32_t magic2 = 351'012'214u;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic1);

    REQUIRE(buffer.GetU32(buffer.Replace(handle, magic2)) == magic2);
  }

  TEST_CASE("SmallValueBuffer::replace_32_with_64", "[SmallValueBuffer]")
  {
    uint32_t magic32 = 88'567'114u;
    uint64_t magic64 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic32);

    REQUIRE(buffer.GetU64(buffer.Replace(handle, magic64)) == magic64);
  }

  TEST_CASE("SmallValueBuffer::replace_32_with_64_causes_overflow", "[SmallValueBuffer]")
  {
    uint32_t magic1 = 88'567'114u;
    uint32_t magic2 = 351'012'214u;
    uint32_t magic3 = 146'122'128u;
    uint32_t magic4 = 2'171'092'154u;

    uint64_t magic64 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle1 = buffer.Push(magic1);
    buffer.Push(magic2);
    buffer.Push(magic3);
    buffer.Push(magic4);

    REQUIRE(buffer.GetU64(buffer.Replace(handle1, magic64)) == magic64);
  }

  TEST_CASE("SmallValueBuffer::replace_64_with_32", "[SmallValueBuffer]")
  {
    uint32_t magic32 = 88'567'114u;
    uint64_t magic64 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic64);

    REQUIRE(buffer.GetU32(buffer.Replace(handle, magic32)) == magic32);
  }

  TEST_CASE("SmallValueBuffer::replace_64_with_64)", "[SmallValueBuffer]")
  {
    uint64_t magic1 = 1'401'612'388'342'512ull;
    uint64_t magic2 = 118'712'305'386'210ull;

    SmallValueBuffer<kBufferSize> buffer;
    auto handle = buffer.Push(magic1);

    REQUIRE(buffer.GetU64(buffer.Replace(handle, magic2)) == magic2);
  }

}