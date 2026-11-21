/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <memory>

namespace krys
{
  class FragmentedSharedBuffer;
  class SharedBuffer;
  class SharedBufferBuilder;
  class SharedBufferDataView;

  /// @brief Immutable bytes, shared by the buffers that hold them. To change or combine data, make a new
  /// segment.
  class DataSegment final : public ThreadSafeRefCounted<DataSegment>
  {
  public:
    /// @brief Bytes owned elsewhere, read through a callback that keeps them alive.
    struct Provider
    {
      MoveOnlyFunc<Span<const uint8>() const> Data;
    };

  private:
    friend class FragmentedSharedBuffer;

    Variant<List<uint8>, Provider> _immutableData;

    explicit DataSegment(List<uint8> data) noexcept : _immutableData(krys::move(data))
    {
    }

    explicit DataSegment(Provider &&provider) noexcept : _immutableData(krys::move(provider))
    {
    }

  public:
    KRYS_NODISCARD static Ref<DataSegment> Create(List<uint8> data) noexcept;

    KRYS_NODISCARD static Ref<DataSegment> Create(Span<const uint8> data) noexcept;

    KRYS_NODISCARD static Ref<DataSegment> Create(Provider &&provider) noexcept;

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return GetSpan().size();
    }
  };

  /// @brief Shared bytes held as a list of segments, such as a resource's data as it arrives. A
  /// `SharedBuffer` is one whose bytes are contiguous.
  class FragmentedSharedBuffer : public ThreadSafeRefCounted<FragmentedSharedBuffer>
  {
  public:
    /// @brief A segment and the position of its first byte in the buffer.
    struct DataSegmentVectorEntry
    {
      size_t BeginPosition;
      Ref<const DataSegment> Segment;

      KRYS_NODISCARD bool operator==(const DataSegmentVectorEntry &) const noexcept = default;
    };

    using DataSegmentVector = SmallList<DataSegmentVectorEntry, 1>;

  private:
    friend class SharedBufferBuilder;

    size_t _size {0};
    DataSegmentVector _segments;
    const bool _contiguous {false};

  public:
    virtual ~FragmentedSharedBuffer() noexcept = default;

    /// @brief A copy of the bytes.
    KRYS_NODISCARD List<uint8> CopyData() const noexcept;

    /// @brief A copy of at most @p length bytes from @p offset.
    KRYS_NODISCARD List<uint8> Read(size_t offset, size_t length) const noexcept;

    /// @brief The bytes, taken rather than copied when nothing else shares the buffer.
    KRYS_NODISCARD List<uint8> ExtractData() noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !_size;
    }

    KRYS_NODISCARD bool IsContiguous() const noexcept
    {
      return _contiguous;
    }

    /// @brief A buffer sharing these segments.
    KRYS_NODISCARD Ref<FragmentedSharedBuffer> Copy() const noexcept;

    /// @brief Copies bytes from @p offset into @p destination, as many as fit or remain.
    void CopyTo(Span<uint8> destination, size_t offset = 0) const noexcept;

    /// @brief Calls @p apply with each segment's bytes in turn.
    template <typename TFunction>
    void ForEachSegment(TFunction &&apply) const
    {
      auto segments = _segments;
      for (const auto &entry : segments)
      {
        apply(entry.Segment->GetSpan());
      }
    }

    /// @brief Calls @p apply with each segment as a buffer of its own.
    template <typename TFunction>
    void ForEachSegmentAsSharedBuffer(TFunction &&apply) const;

    KRYS_NODISCARD bool StartsWith(Span<const uint8> prefix) const noexcept;

    KRYS_NODISCARD DataSegmentVector::const_iterator begin() const noexcept
    {
      return _segments.begin();
    }

    KRYS_NODISCARD DataSegmentVector::const_iterator end() const noexcept
    {
      return _segments.end();
    }

    KRYS_NODISCARD size_t SegmentsCount() const noexcept
    {
      return _segments.size();
    }

    /// @brief The rest of the segment holding the byte at @p position. Takes O(log n) time.
    KRYS_NODISCARD SharedBufferDataView GetSomeData(size_t position) const noexcept;

    /// @brief At most @p length bytes from @p position, combined into one buffer if they span segments.
    KRYS_NODISCARD Ref<SharedBuffer> GetContiguousData(size_t position, size_t length) const noexcept;

    /// @brief The bytes as upper-case hexadecimal, two digits each.
    KRYS_NODISCARD utf8_string ToHexString() const noexcept;

    /// @brief This buffer if it's contiguous, else one with its segments combined.
    KRYS_NODISCARD Ref<SharedBuffer> MakeContiguous() const noexcept;

    /// @brief Whether the buffers hold the same bytes, however they're split into segments.
    KRYS_NODISCARD bool operator==(const FragmentedSharedBuffer &other) const noexcept;

  protected:
    enum class Contiguous : bool
    {
      No,
      Yes,
    };

    explicit FragmentedSharedBuffer(Contiguous contiguous) noexcept
        : _contiguous(contiguous == Contiguous::Yes)
    {
    }

    /// @brief A contiguous buffer of @p segment, for `SharedBuffer`.
    explicit FragmentedSharedBuffer(Ref<const DataSegment> segment) noexcept;

    KRYS_NODISCARD const DataSegmentVector &Segments() const noexcept
    {
      return _segments;
    }

  private:
    FragmentedSharedBuffer(size_t size, const DataSegmentVector &segments) noexcept;

    void Clear() noexcept;

    /// @brief Combines the segments into a list and returns it, clearing the buffer.
    KRYS_NODISCARD List<uint8> TakeData() noexcept;

    /// @brief The entries from the one holding the byte at @p position to the end.
    KRYS_NODISCARD Span<const DataSegmentVectorEntry> SegmentForPosition(size_t position) const noexcept;

    KRYS_NODISCARD static bool HaveIdenticalContent(const DataSegmentVector &a,
                                                    const DataSegmentVector &b) noexcept;

    KRYS_NODISCARD bool InternallyConsistent() const noexcept;

    KRYS_NODISCARD static bool InternallyConsistent(size_t size, const DataSegmentVector &segments) noexcept;
  };

  /// @brief A buffer whose bytes are contiguous, so can be read directly.
  class SharedBuffer final : public FragmentedSharedBuffer
  {
  private:
    friend class SharedBufferBuilder;

    SharedBuffer() noexcept : FragmentedSharedBuffer(Contiguous::Yes)
    {
    }

    explicit SharedBuffer(Ref<const DataSegment> segment) noexcept;

  public:
    KRYS_NODISCARD static Ref<SharedBuffer> Create() noexcept
    {
      return AdoptRef(*new SharedBuffer());
    }

    KRYS_NODISCARD static Ref<SharedBuffer> Create(List<uint8> data) noexcept;

    KRYS_NODISCARD static Ref<SharedBuffer> Create(Span<const uint8> data) noexcept;

    KRYS_NODISCARD static Ref<SharedBuffer> Create(Ref<const DataSegment> segment) noexcept
    {
      return AdoptRef(*new SharedBuffer(krys::move(segment)));
    }

    KRYS_NODISCARD static Ref<SharedBuffer> Create(DataSegment::Provider &&provider) noexcept;

    KRYS_NODISCARD static Ref<SharedBuffer> Create(const Ref<FragmentedSharedBuffer> &buffer) noexcept
    {
      return buffer->MakeContiguous();
    }

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept;

    KRYS_NODISCARD const uint8 *Data() const noexcept
    {
      return GetSpan().data();
    }

    /// @brief The byte at @p position.
    KRYS_NODISCARD uint8 operator[](size_t position) const noexcept;

    /// @brief Whether @p span ends within this buffer's bytes.
    template <typename T>
    KRYS_NODISCARD bool IsSpanWithinBounds(Span<T> span) const noexcept
    {
      auto bytes = GetSpan();

      auto *otherEnd = reinterpret_cast<const uint8 *>(std::to_address(span.end()));
      auto *thisBegin = std::to_address(bytes.begin());
      if (otherEnd < thisBegin)
      {
        return false;
      }

      return static_cast<size_t>(otherEnd - thisBegin) <= Size(); // The end itself is in bounds.
    }
  };

  /// @brief Builds a buffer from appended data, without copying segments it's given.
  class SharedBufferBuilder
  {
    KRYS_NON_COPYABLE(SharedBufferBuilder);

  private:
    enum class State : uint8
    {
      Null,
      Stale,
      Fresh,
    };

    mutable State _state {State::Null};
    mutable RefPtr<FragmentedSharedBuffer> _buffer;
    size_t _size {0};
    FragmentedSharedBuffer::DataSegmentVector _segments;

  public:
    SharedBufferBuilder() noexcept = default;

    SharedBufferBuilder(SharedBufferBuilder &&) noexcept = default;

    explicit SharedBufferBuilder(const FragmentedSharedBuffer &buffer) noexcept
    {
      Append(buffer);
    }

    SharedBufferBuilder &operator=(SharedBufferBuilder &&) noexcept = default;

    void Append(const FragmentedSharedBuffer &buffer) noexcept;

    void Append(Span<const uint8> data) noexcept;

    void Append(List<uint8> data) noexcept;

    void AppendSpans(Span<const Span<const uint8>> spans) noexcept;

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return !IsNull();
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _state == State::Null;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !_size;
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD bool IsContiguous() const noexcept
    {
      return _segments.size() <= 1;
    }

    KRYS_NODISCARD bool HasOneSegment() const noexcept
    {
      return _segments.size() == 1;
    }

    KRYS_NODISCARD FragmentedSharedBuffer::DataSegmentVector::const_iterator begin() const noexcept
    {
      return _segments.begin();
    }

    KRYS_NODISCARD FragmentedSharedBuffer::DataSegmentVector::const_iterator end() const noexcept
    {
      return _segments.end();
    }

    /// @brief Empties the builder and makes it null.
    void Reset() noexcept
    {
      Empty();
      _state = State::Null;
    }

    /// @brief Empties the builder, which stays non-null.
    void Empty() noexcept
    {
      _state = State::Stale;
      _segments.clear();
      _size = 0;
      _buffer = nullptr;
    }

    /// @brief The buffer built so far, or null if nothing has been appended.
    KRYS_NODISCARD FragmentedSharedBuffer *Buffer() const noexcept
    {
      UpdateBufferIfNeeded();
      return _buffer.get();
    }

    KRYS_NODISCARD Ref<FragmentedSharedBuffer> CopyBuffer() const noexcept
    {
      return CreateBuffer();
    }

    /// @brief The buffer built, resetting the builder.
    KRYS_NODISCARD Ref<FragmentedSharedBuffer> TakeBuffer() noexcept;

    KRYS_NODISCARD Ref<SharedBuffer> TakeBufferAsContiguous() noexcept;

    KRYS_NODISCARD bool operator==(const SharedBufferBuilder &other) const noexcept;

  private:
    void UpdateBufferIfNeeded() const noexcept;

    void AppendDataSegment(Ref<DataSegment> segment) noexcept;

    KRYS_NODISCARD Ref<FragmentedSharedBuffer> CreateBuffer() const noexcept;
  };

  /// @brief A view of part of one segment, keeping it alive.
  class SharedBufferDataView
  {
  private:
    Ref<const DataSegment> _segment;
    size_t _positionWithinSegment;
    size_t _size;

  public:
    SharedBufferDataView(Ref<const DataSegment> segment, size_t positionWithinSegment,
                         Maybe<size_t> size = {}) noexcept;

    SharedBufferDataView(const SharedBufferDataView &other, size_t size) noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept
    {
      return _segment->GetSpan().subspan(_positionWithinSegment, _size);
    }

    KRYS_NODISCARD Ref<SharedBuffer> CreateSharedBuffer() const noexcept;
  };

  inline List<uint8> FragmentedSharedBuffer::ExtractData() noexcept
  {
    if (HasOneRef())
    {
      return TakeData();
    }

    return CopyData();
  }

  template <typename TFunction>
  void FragmentedSharedBuffer::ForEachSegmentAsSharedBuffer(TFunction &&apply) const
  {
    Ref<const FragmentedSharedBuffer> protectedThis = ShareRef(*this);
    for (const auto &entry : _segments)
    {
      apply(SharedBuffer::Create(entry.Segment));
    }
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::SharedBuffer)
  KRYS_NODISCARD static bool IsType(const krys::FragmentedSharedBuffer &buffer) noexcept
  {
    return buffer.IsContiguous();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
