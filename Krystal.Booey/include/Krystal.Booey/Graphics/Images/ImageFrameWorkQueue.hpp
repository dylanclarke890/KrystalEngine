/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/Images/PlatformImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  class WorkQueue;
}

namespace krys::boo
{
  class BitmapImageSource;
  class ImageDecoder;

  /// @brief Decodes a bitmap image's frames off the main thread, on the host's work queue: requests run one
  /// at a time, in order, each with the queue's own decoder (never the source's, which the main thread uses)
  /// over the image's complete data. The decoder goes with the job and comes back with the frame through the
  /// event loop's inbox, so it's only ever used by one thread and always destroyed on the main thread.
  class ImageFrameWorkQueue final : public RefCounted<ImageFrameWorkQueue>,
                                    public CanMakeWeakPtr<ImageFrameWorkQueue>
  {
    KRYS_NON_COPY_MOVABLE(ImageFrameWorkQueue);

  public:
    struct Request
    {
      uint32 Index;
      SubsamplingLevel Level;
      ImageAnimatingState AnimatingState;
      DecodingOptions Options;

      KRYS_NODISCARD friend bool operator==(const Request &, const Request &) noexcept = default;
    };

  private:
    BitmapImageSource *_source;
    /// @brief The requests not yet finished, oldest first; the first is being decoded.
    Deque<Request> _decodeQueue;
    /// @brief The queue's decoder, while no job has it.
    RefPtr<ImageDecoder> _decoder;
    bool _decoding {false};
    /// @brief Counts the stops, so that a frame decoded before one is dropped.
    uint64 _generation {0};

  public:
    explicit ImageFrameWorkQueue(BitmapImageSource &source) noexcept;
    ~ImageFrameWorkQueue() noexcept;

    KRYS_NODISCARD static Ref<ImageFrameWorkQueue> Create(BitmapImageSource &source) noexcept
    {
      return CreateRef<ImageFrameWorkQueue>(source);
    }

    /// @brief The host's work queue, which frames are decoded on; null if the host has none.
    KRYS_NODISCARD static WorkQueue *HostWorkQueue() noexcept;

    /// @brief Asks for the frame @p request names to be decoded.
    void Dispatch(const Request &request) noexcept;

    /// @brief Drops the requests not yet finished, and their frames' images; a frame being decoded is
    /// dropped when it arrives.
    void Stop() noexcept;

    /// @brief Forgets the source, which is going away: the frames that arrive are dropped.
    void Detach() noexcept;

    KRYS_NODISCARD bool IsIdle() const noexcept
    {
      return _decodeQueue.empty();
    }

    /// @brief Whether a request for the frame at @p index, at @p level or finer and compatible with
    /// @p options, isn't finished.
    KRYS_NODISCARD bool IsPendingDecodingAtIndex(uint32 index, SubsamplingLevel level,
                                                 const DecodingOptions &options) const noexcept;

  private:
    /// @brief Sends the oldest request to the host's work queue, unless one is being decoded.
    void DecodeNext() noexcept;

    /// @brief The frame @p request asked for arrived, as @p image (null if it failed), with the decoder.
    void DecodeFinished(uint64 generation, Ref<ImageDecoder> &&decoder, const Request &request,
                        PlatformImagePtr &&image) noexcept;
  };
}
