/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2015 Canon Inc. All rights reserved.
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

#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/HexNumber.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <cstring>
#include <utility>

namespace krys
{
  namespace
  {
    List<uint8> CombineSegmentsData(const FragmentedSharedBuffer::DataSegmentVector &segments,
                                    size_t size) noexcept
    {
      List<uint8> combinedData;
      combinedData.reserve(size);
      for (const auto &entry : segments)
      {
        auto bytes = entry.Segment->GetSpan();
        combinedData.insert(combinedData.end(), bytes.begin(), bytes.end());
      }

      krys_debug_assert(combinedData.size() == size);
      return combinedData;
    }

    /// @brief The index of the entry holding the byte at @p position: the one before the first entry
    /// beginning after it.
    size_t SegmentIndexForPosition(const FragmentedSharedBuffer::DataSegmentVector &segments,
                                   size_t position) noexcept
    {
      auto comparator = [](size_t position, const FragmentedSharedBuffer::DataSegmentVectorEntry &entry)
      {
        return position < entry.BeginPosition;
      };

      auto entry = std::upper_bound(segments.begin(), segments.end(), position, comparator);
      return static_cast<size_t>(entry - segments.begin()) - 1;
    }

    void AppendBytes(List<uint8> &data, Span<const uint8> bytes) noexcept
    {
      data.insert(data.end(), bytes.begin(), bytes.end());
    }

    void CopyBytes(Span<uint8> destination, Span<const uint8> source) noexcept
    {
      krys_debug_assert(destination.size() >= source.size());
      if (!source.empty())
      {
        std::memcpy(destination.data(), source.data(), source.size());
      }
    }
  }

#pragma region DataSegment

  Ref<DataSegment> DataSegment::Create(List<uint8> data) noexcept
  {
    data.shrink_to_fit();
    return AdoptRef(*new DataSegment(krys::move(data)));
  }

  Ref<DataSegment> DataSegment::Create(Span<const uint8> data) noexcept
  {
    return Create(List<uint8>(data.begin(), data.end()));
  }

  Ref<DataSegment> DataSegment::Create(Provider &&provider) noexcept
  {
    return AdoptRef(*new DataSegment(krys::move(provider)));
  }

  Span<const uint8> DataSegment::GetSpan() const noexcept
  {
    return SwitchOn(
      _immutableData, [](const List<uint8> &data) { return Span<const uint8>(data); },
      [](const Provider &provider) { return provider.Data(); });
  }

#pragma endregion

#pragma region FragmentedSharedBuffer

  FragmentedSharedBuffer::FragmentedSharedBuffer(Ref<const DataSegment> segment) noexcept
      : _size(segment->Size()), _contiguous(true)
  {
    _segments.push_back(DataSegmentVectorEntry {0, krys::move(segment)});
  }

  FragmentedSharedBuffer::FragmentedSharedBuffer(size_t size, const DataSegmentVector &segments) noexcept
      : _size(size), _segments(segments)
  {
    krys_debug_assert(InternallyConsistent());
  }

  Ref<SharedBuffer> FragmentedSharedBuffer::MakeContiguous() const noexcept
  {
    if (const auto *sharedBuffer = DynamicDowncast<SharedBuffer>(this))
    {
      return ShareRef(const_cast<SharedBuffer &>(*sharedBuffer));
    }

    if (_segments.empty())
    {
      return SharedBuffer::Create();
    }

    if (_segments.size() == 1)
    {
      return SharedBuffer::Create(_segments[0].Segment);
    }

    return SharedBuffer::Create(CombineSegmentsData(_segments, _size));
  }

  List<uint8> FragmentedSharedBuffer::CopyData() const noexcept
  {
    List<uint8> data;
    data.reserve(_size);
    ForEachSegment([&data](Span<const uint8> bytes) { AppendBytes(data, bytes); });
    return data;
  }

  List<uint8> FragmentedSharedBuffer::TakeData() noexcept
  {
    if (_segments.empty())
    {
      return {};
    }

    List<uint8> combinedData;
    auto &segment = const_cast<DataSegment &>(*_segments[0].Segment);
    if (_segments.size() == 1 && std::holds_alternative<List<uint8>>(segment._immutableData)
        && segment.HasOneRef())
    {
      combinedData = std::exchange(std::get<List<uint8>>(segment._immutableData), List<uint8>());
    }
    else
    {
      combinedData = CombineSegmentsData(_segments, _size);
    }

    Clear();
    return combinedData;
  }

  SharedBufferDataView FragmentedSharedBuffer::GetSomeData(size_t position) const noexcept
  {
    const auto &entry = SegmentForPosition(position).front();
    return {entry.Segment, position - entry.BeginPosition};
  }

  Ref<SharedBuffer> FragmentedSharedBuffer::GetContiguousData(size_t position, size_t length) const noexcept
  {
    if (position >= _size)
    {
      return SharedBuffer::Create();
    }
    length = std::min(_size - position, length);
    auto entries = SegmentForPosition(position);
    const auto &first = entries[0];
    size_t offsetInSegment = position - first.BeginPosition;
    krys_debug_assert(first.Segment->Size() > offsetInSegment);
    if (first.Segment->Size() - offsetInSegment >= length)
    {
      return SharedBufferDataView {first.Segment, offsetInSegment, length}.CreateSharedBuffer();
    }

    List<uint8> combinedData;
    combinedData.reserve(length);
    AppendBytes(combinedData, first.Segment->GetSpan().subspan(offsetInSegment));
    for (entries = entries.subspan(1); combinedData.size() < length && !entries.empty();
         entries = entries.subspan(1))
    {
      const auto &entry = entries[0];
      auto canCopy = std::min(length - combinedData.size(), entry.Segment->Size());
      AppendBytes(combinedData, entry.Segment->GetSpan().first(canCopy));
    }

    return SharedBuffer::Create(krys::move(combinedData));
  }

  Span<const FragmentedSharedBuffer::DataSegmentVectorEntry>
    FragmentedSharedBuffer::SegmentForPosition(size_t position) const noexcept
  {
    krys_assert(position < _size);
    return Span<const DataSegmentVectorEntry>(_segments).subspan(
      SegmentIndexForPosition(_segments, position));
  }

  utf8_string FragmentedSharedBuffer::ToHexString() const noexcept
  {
    StringBuilder builder;
    ForEachSegment(
      [&builder](Span<const uint8> bytes)
      {
        for (auto value : bytes)
        {
          builder.Append(Hex(value, 2));
        }
      });

    return krys::move(builder).ToString();
  }

  void FragmentedSharedBuffer::Clear() noexcept
  {
    _size = 0;
    _segments.clear();
    krys_debug_assert(InternallyConsistent());
  }

  Ref<FragmentedSharedBuffer> FragmentedSharedBuffer::Copy() const noexcept
  {
    if (_contiguous)
    {
      return _segments.empty() ? SharedBuffer::Create() : SharedBuffer::Create(_segments[0].Segment);
    }

    return AdoptRef(*new FragmentedSharedBuffer(_size, _segments));
  }

  bool FragmentedSharedBuffer::StartsWith(Span<const uint8> prefix) const noexcept
  {
    if (prefix.empty())
    {
      return true;
    }

    if (_size < prefix.size())
    {
      return false;
    }

    size_t remaining = prefix.size();
    for (const auto &entry : _segments)
    {
      size_t amountToCompareThisTime = std::min(remaining, entry.Segment->Size());
      auto bytes = entry.Segment->GetSpan().first(amountToCompareThisTime);
      if (!std::equal(bytes.begin(), bytes.end(), prefix.begin()))
      {
        return false;
      }

      remaining -= amountToCompareThisTime;
      if (!remaining)
      {
        return true;
      }

      prefix = prefix.subspan(amountToCompareThisTime);
    }

    return false;
  }

  List<uint8> FragmentedSharedBuffer::Read(size_t offset, size_t length) const noexcept
  {
    List<uint8> data;
    if (offset >= _size)
    {
      return data;
    }

    auto remaining = std::min(length, _size - offset);
    if (!remaining)
    {
      return data;
    }

    data.reserve(remaining);
    auto entries = SegmentForPosition(offset);
    const auto &first = entries[0];
    size_t offsetInSegment = offset - first.BeginPosition;
    size_t availableInSegment = std::min(first.Segment->Size() - offsetInSegment, remaining);
    AppendBytes(data, first.Segment->GetSpan().subspan(offsetInSegment, availableInSegment));

    remaining -= availableInSegment;

    while (remaining)
    {
      entries = entries.subspan(1);
      if (entries.empty())
      {
        break;
      }

      const auto &entry = entries[0];
      size_t lengthInSegment = std::min(entry.Segment->Size(), remaining);
      AppendBytes(data, entry.Segment->GetSpan().first(lengthInSegment));
      remaining -= lengthInSegment;
    }
    return data;
  }

  void FragmentedSharedBuffer::CopyTo(Span<uint8> destination, size_t offset) const noexcept
  {
    if (offset >= _size)
    {
      return;
    }

    auto remaining = std::min(destination.size(), _size - offset);
    if (!remaining)
    {
      return;
    }

    auto entries = Span<const DataSegmentVectorEntry>(_segments);
    if (offset >= entries[0].Segment->Size())
    {
      entries = entries.subspan(SegmentIndexForPosition(_segments, offset));
    }

    const auto &first = entries[0];
    size_t positionInSegment = offset - first.BeginPosition;
    size_t amountToCopyThisTime = std::min(remaining, first.Segment->Size() - positionInSegment);
    CopyBytes(destination, first.Segment->GetSpan().subspan(positionInSegment, amountToCopyThisTime));

    remaining -= amountToCopyThisTime;
    if (!remaining)
    {
      return;
    }

    destination = destination.subspan(amountToCopyThisTime);
    // Bytes are left, so another segment holds them.
    for (entries = entries.subspan(1); !entries.empty(); entries = entries.subspan(1))
    {
      const auto &entry = entries[0];
      amountToCopyThisTime = std::min(remaining, entry.Segment->Size());
      CopyBytes(destination, entry.Segment->GetSpan().first(amountToCopyThisTime));
      remaining -= amountToCopyThisTime;
      if (!remaining)
      {
        return;
      }
      destination = destination.subspan(amountToCopyThisTime);
    }
  }

  bool FragmentedSharedBuffer::InternallyConsistent() const noexcept
  {
    if (IsContiguous() && SegmentsCount() > 1)
    {
      return false;
    }

    return InternallyConsistent(_size, _segments);
  }

  bool FragmentedSharedBuffer::InternallyConsistent(size_t size, const DataSegmentVector &segments) noexcept
  {
    size_t position = 0;
    for (const auto &entry : segments)
    {
      if (entry.BeginPosition != position)
      {
        return false;
      }

      position += entry.Segment->Size();
    }

    return position == size;
  }

  bool FragmentedSharedBuffer::operator==(const FragmentedSharedBuffer &other) const noexcept
  {
    if (this == &other)
    {
      return true;
    }

    if (_size != other._size)
    {
      return false;
    }

    return HaveIdenticalContent(_segments, other._segments);
  }

  bool FragmentedSharedBuffer::HaveIdenticalContent(const DataSegmentVector &a,
                                                    const DataSegmentVector &b) noexcept
  {
    // Fast comparison.
    if (a == b)
    {
      return true;
    }

    auto thisEntries = Span<const DataSegmentVectorEntry>(a);
    size_t thisOffset = 0;
    auto otherEntries = Span<const DataSegmentVectorEntry>(b);
    size_t otherOffset = 0;

    while (!thisEntries.empty() && !otherEntries.empty())
    {
      const auto &thisSegment = *thisEntries[0].Segment;
      const auto &otherSegment = *otherEntries[0].Segment;

      if (&thisSegment == &otherSegment && !thisOffset && !otherOffset)
      {
        thisEntries = thisEntries.subspan(1);
        otherEntries = otherEntries.subspan(1);
        continue;
      }

      krys_debug_assert(thisOffset <= thisSegment.Size());
      krys_debug_assert(otherOffset <= otherSegment.Size());

      size_t thisRemaining = thisSegment.Size() - thisOffset;
      size_t otherRemaining = otherSegment.Size() - otherOffset;
      size_t remaining = std::min(thisRemaining, otherRemaining);

      auto thisBytes = thisSegment.GetSpan().subspan(thisOffset, remaining);
      auto otherBytes = otherSegment.GetSpan().subspan(otherOffset, remaining);
      if (!std::equal(thisBytes.begin(), thisBytes.end(), otherBytes.begin()))
      {
        return false;
      }

      thisOffset += remaining;
      otherOffset += remaining;

      if (thisOffset == thisSegment.Size())
      {
        thisEntries = thisEntries.subspan(1);
        thisOffset = 0;
      }

      if (otherOffset == otherSegment.Size())
      {
        otherEntries = otherEntries.subspan(1);
        otherOffset = 0;
      }
    }

    return true;
  }

#pragma endregion

#pragma region SharedBuffer

  SharedBuffer::SharedBuffer(Ref<const DataSegment> segment) noexcept
      : FragmentedSharedBuffer(krys::move(segment))
  {
  }

  Ref<SharedBuffer> SharedBuffer::Create(List<uint8> data) noexcept
  {
    return Create(Ref<const DataSegment>(DataSegment::Create(krys::move(data))));
  }

  Ref<SharedBuffer> SharedBuffer::Create(Span<const uint8> data) noexcept
  {
    return Create(Ref<const DataSegment>(DataSegment::Create(data)));
  }

  Ref<SharedBuffer> SharedBuffer::Create(DataSegment::Provider &&provider) noexcept
  {
    return Create(Ref<const DataSegment>(DataSegment::Create(krys::move(provider))));
  }

  Span<const uint8> SharedBuffer::GetSpan() const noexcept
  {
    if (!SegmentsCount())
    {
      return {};
    }

    return Segments()[0].Segment->GetSpan();
  }

  uint8 SharedBuffer::operator[](size_t position) const noexcept
  {
    return Segments()[0].Segment->GetSpan()[position];
  }

#pragma endregion

#pragma region SharedBufferBuilder

  Ref<FragmentedSharedBuffer> SharedBufferBuilder::TakeBuffer() noexcept
  {
    if (IsEmpty())
    {
      return SharedBuffer::Create();
    }

    UpdateBufferIfNeeded();
    Ref<FragmentedSharedBuffer> buffer = _buffer;
    Reset();

    return buffer;
  }

  Ref<SharedBuffer> SharedBufferBuilder::TakeBufferAsContiguous() noexcept
  {
    return TakeBuffer()->MakeContiguous();
  }

  void SharedBufferBuilder::UpdateBufferIfNeeded() const noexcept
  {
    if (_state == State::Null)
    {
      krys_debug_assert(!_buffer);
      return;
    }

    if (_state == State::Fresh)
    {
      return;
    }

    _buffer = CreateBuffer();
    _state = State::Fresh;
  }

  Ref<FragmentedSharedBuffer> SharedBufferBuilder::CreateBuffer() const noexcept
  {
    if (IsEmpty())
    {
      return SharedBuffer::Create();
    }

    if (_segments.size() == 1)
    {
      return SharedBuffer::Create(_segments[0].Segment);
    }

    return AdoptRef(*new FragmentedSharedBuffer(_size, _segments));
  }

  void SharedBufferBuilder::AppendDataSegment(Ref<DataSegment> segment) noexcept
  {
    _state = State::Stale;
    size_t size = segment->Size();
    _segments.push_back(FragmentedSharedBuffer::DataSegmentVectorEntry {_size, krys::move(segment)});
    _size += size;
  }

  void SharedBufferBuilder::Append(const FragmentedSharedBuffer &buffer) noexcept
  {
    _state = State::Stale;
    _segments.reserve(_segments.size() + buffer._segments.size());
    for (const auto &entry : buffer._segments)
    {
      _segments.push_back(FragmentedSharedBuffer::DataSegmentVectorEntry {_size, entry.Segment});
      _size += entry.Segment->Size();
    }

    krys_debug_assert(FragmentedSharedBuffer::InternallyConsistent(_size, _segments));
  }

  void SharedBufferBuilder::Append(Span<const uint8> data) noexcept
  {
    if (!data.empty())
    {
      AppendDataSegment(DataSegment::Create(data));
    }
  }

  void SharedBufferBuilder::Append(List<uint8> data) noexcept
  {
    if (!data.empty())
    {
      AppendDataSegment(DataSegment::Create(krys::move(data)));
    }
  }

  void SharedBufferBuilder::AppendSpans(Span<const Span<const uint8>> spans) noexcept
  {
    _state = State::Stale;
    _segments.reserve(_segments.size() + spans.size());
    for (auto span : spans)
    {
      _segments.push_back(FragmentedSharedBuffer::DataSegmentVectorEntry {_size, DataSegment::Create(span)});
      _size += span.size();
    }

    krys_debug_assert(FragmentedSharedBuffer::InternallyConsistent(_size, _segments));
  }

  bool SharedBufferBuilder::operator==(const SharedBufferBuilder &other) const noexcept
  {
    if (this == &other)
    {
      return true;
    }

    if (_size != other._size)
    {
      return false;
    }

    return FragmentedSharedBuffer::HaveIdenticalContent(_segments, other._segments);
  }

#pragma endregion

#pragma region SharedBufferDataView

  SharedBufferDataView::SharedBufferDataView(Ref<const DataSegment> segment, size_t positionWithinSegment,
                                             Maybe<size_t> size) noexcept
      : _segment(krys::move(segment)), _positionWithinSegment(positionWithinSegment),
        _size(size ? *size : _segment->Size() - positionWithinSegment)
  {
    krys_assert(_positionWithinSegment < _segment->Size());
    krys_assert(_size <= _segment->Size() - _positionWithinSegment);
  }

  SharedBufferDataView::SharedBufferDataView(const SharedBufferDataView &other, size_t size) noexcept
      : SharedBufferDataView(other._segment, other._positionWithinSegment, size)
  {
  }

  Ref<SharedBuffer> SharedBufferDataView::CreateSharedBuffer() const noexcept
  {
    return SharedBuffer::Create(
      DataSegment::Provider {[segment = _segment, data = GetSpan()]() { return data; }});
  }

#pragma endregion
}
