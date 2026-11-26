#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::tests
{
  namespace
  {
    List<uint8> Bytes(std::initializer_list<uint8> values) noexcept
    {
      return List<uint8>(values);
    }

    /// @brief A buffer of 1 2 3 | 4 5 | 6 7 8 9, in three segments.
    Ref<FragmentedSharedBuffer> ThreeSegments() noexcept
    {
      SharedBufferBuilder builder;
      builder.Append(Bytes({1, 2, 3}));
      builder.Append(Bytes({4, 5}));
      builder.Append(Bytes({6, 7, 8, 9}));
      return builder.TakeBuffer();
    }
  }

  TEST_CASE("SharedBuffer holds contiguous bytes", "[Types][SharedBuffer]")
  {
    auto empty = SharedBuffer::Create();
    CHECK(empty->IsEmpty());
    CHECK(empty->IsContiguous());
    CHECK(empty->GetSpan().empty());

    auto buffer = SharedBuffer::Create(Bytes({10, 20, 30}));
    CHECK(buffer->Size() == 3);
    CHECK(buffer->SegmentsCount() == 1);
    CHECK((*buffer)[1] == 20);
    CHECK(buffer->Data() == buffer->GetSpan().data());

    Array<uint8, 2> raw {7, 8};
    auto copied = SharedBuffer::Create(Span<const uint8>(raw));
    CHECK(copied->Data() != raw.data());
    CHECK(copied->CopyData() == Bytes({7, 8}));
  }

  TEST_CASE("SharedBufferBuilder appends segments without combining them", "[Types][SharedBuffer]")
  {
    SharedBufferBuilder builder;
    CHECK(builder.IsNull());

    builder.Append(Bytes({1, 2}));
    builder.Append(Span<const uint8>());
    CHECK(builder.HasOneSegment());

    Array<uint8, 2> more {3, 4};
    Array<Span<const uint8>, 2> spans {Span<const uint8>(more), Span<const uint8>(more).first(1)};
    builder.AppendSpans(spans);
    CHECK(builder.Size() == 5);
    CHECK_FALSE(builder.IsContiguous());

    auto buffer = builder.TakeBuffer();
    CHECK(builder.IsNull());
    CHECK(builder.IsEmpty());
    CHECK_FALSE(buffer->IsContiguous());
    CHECK(buffer->SegmentsCount() == 3);
    CHECK(buffer->CopyData() == Bytes({1, 2, 3, 4, 3}));

    SECTION("Appending a buffer shares its segments")
    {
      SharedBufferBuilder other(*buffer);
      other.Append(Bytes({9}));
      CHECK(other.Size() == 6);
      CHECK(other.begin()->Segment == buffer->begin()->Segment);
    }

    SECTION("A single segment builds a SharedBuffer")
    {
      SharedBufferBuilder single;
      single.Append(Bytes({5}));
      CHECK(single.TakeBuffer()->IsContiguous());
    }

    SECTION("Empty leaves the builder non-null")
    {
      SharedBufferBuilder emptied;
      emptied.Append(Bytes({5}));
      emptied.Empty();
      CHECK_FALSE(emptied.IsNull());
      CHECK(emptied.Buffer()->IsEmpty());
    }
  }

  TEST_CASE("FragmentedSharedBuffer reads across segments", "[Types][SharedBuffer]")
  {
    auto buffer = ThreeSegments();
    REQUIRE(buffer->Size() == 9);

    CHECK(buffer->Read(2, 4) == Bytes({3, 4, 5, 6}));
    CHECK(buffer->Read(7, 10) == Bytes({8, 9}));
    CHECK(buffer->Read(9, 1).empty());

    Array<uint8, 4> destination {};
    buffer->CopyTo(destination, 4);
    CHECK(destination == Array<uint8, 4> {5, 6, 7, 8});

    Array<uint8, 2> start {};
    buffer->CopyTo(start);
    CHECK(start == Array<uint8, 2> {1, 2});

    CHECK(buffer->StartsWith(Bytes({1, 2, 3, 4})));
    CHECK_FALSE(buffer->StartsWith(Bytes({1, 2, 4})));
    CHECK(buffer->StartsWith({}));

    CHECK(buffer->ToHexString() == u8"010203040506070809");
  }

  TEST_CASE("FragmentedSharedBuffer gives contiguous data", "[Types][SharedBuffer]")
  {
    auto buffer = ThreeSegments();

    auto some = buffer->GetSomeData(4);
    CHECK(some.Size() == 1);
    CHECK(some.GetSpan()[0] == 5);
    CHECK(some.CreateSharedBuffer()->CopyData() == Bytes({5}));

    auto withinSegment = buffer->GetContiguousData(5, 2);
    CHECK(withinSegment->CopyData() == Bytes({6, 7}));

    auto acrossSegments = buffer->GetContiguousData(1, 5);
    CHECK(acrossSegments->CopyData() == Bytes({2, 3, 4, 5, 6}));

    CHECK(buffer->GetContiguousData(20, 1)->IsEmpty());

    auto contiguous = buffer->MakeContiguous();
    CHECK(contiguous->SegmentsCount() == 1);
    CHECK(contiguous->CopyData() == buffer->CopyData());
    CHECK(contiguous->MakeContiguous().get() == contiguous.get());
  }

  TEST_CASE("FragmentedSharedBuffer compares content, not segments", "[Types][SharedBuffer]")
  {
    auto fragmented = ThreeSegments();
    auto contiguous = SharedBuffer::Create(Bytes({1, 2, 3, 4, 5, 6, 7, 8, 9}));
    CHECK(*fragmented == *contiguous);
    CHECK(*fragmented == *fragmented->Copy());
    CHECK_FALSE(*fragmented == *SharedBuffer::Create(Bytes({1, 2, 3, 4, 5, 6, 7, 8, 0})));
    CHECK_FALSE(*fragmented == *SharedBuffer::Create(Bytes({1, 2})));

    SharedBufferBuilder a(*fragmented);
    SharedBufferBuilder b(*contiguous);
    CHECK(a == b);
  }

  TEST_CASE("FragmentedSharedBuffer casts to SharedBuffer when contiguous", "[Types][SharedBuffer]")
  {
    Ref<FragmentedSharedBuffer> contiguous = SharedBuffer::Create(Bytes({1}));
    CHECK(Is<SharedBuffer>(*contiguous));
    CHECK(DynamicDowncast<SharedBuffer>(contiguous.get()) != nullptr);

    auto fragmented = ThreeSegments();
    CHECK_FALSE(Is<SharedBuffer>(*fragmented));
  }

  TEST_CASE("FragmentedSharedBuffer::ExtractData takes unshared data", "[Types][SharedBuffer]")
  {
    auto buffer = SharedBuffer::Create(Bytes({1, 2, 3}));
    const auto *bytes = buffer->Data();
    auto data = buffer->ExtractData();
    CHECK(data.data() == bytes);
    CHECK(buffer->IsEmpty());

    auto shared = SharedBuffer::Create(Bytes({4, 5}));
    Ref<SharedBuffer> other = shared;
    CHECK(shared->ExtractData() == Bytes({4, 5}));
    CHECK(other->Size() == 2);
  }

  TEST_CASE("SharedBuffer::IsSpanWithinBounds", "[Types][SharedBuffer]")
  {
    // The buffer is the first half of a larger array, so spans past its end stay valid.
    static const Array<uint8, 8> storage {1, 2, 3, 4, 5, 6, 7, 8};
    auto all = Span<const uint8>(storage);
    auto buffer = SharedBuffer::Create(DataSegment::Provider {[all] { return all.first(4); }});
    CHECK(buffer->IsSpanWithinBounds(all.subspan(1, 2)));
    CHECK(buffer->IsSpanWithinBounds(all.first(4)));
    CHECK_FALSE(buffer->IsSpanWithinBounds(all.subspan(2, 4)));
  }

  TEST_CASE("DataSegment::Provider reads bytes owned elsewhere", "[Types][SharedBuffer]")
  {
    static const Array<uint8, 3> external {7, 8, 9};
    auto buffer = SharedBuffer::Create(DataSegment::Provider {[] { return Span<const uint8>(external); }});
    CHECK(buffer->Data() == external.data());
    CHECK(buffer->Size() == 3);
  }
}
