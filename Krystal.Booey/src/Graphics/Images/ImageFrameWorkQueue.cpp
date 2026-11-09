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

#include "Krystal.Booey/Graphics/Images/ImageFrameWorkQueue.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/Images/BitmapImageSource.hpp"
#include "Krystal.Booey/Graphics/Images/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Threading/MainThread.hpp"
#include "Krystal.Core/Threading/WorkQueue.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    /// @brief What a decoding job gives back: its decoder, and the frame it decoded (null if it failed).
    struct DecodedFrame
    {
      Ref<ImageDecoder> Decoder;
      PlatformImagePtr Image;
    };
  }

  ImageFrameWorkQueue::ImageFrameWorkQueue(BitmapImageSource &source) noexcept : _source(&source)
  {
  }

  ImageFrameWorkQueue::~ImageFrameWorkQueue() noexcept = default;

  WorkQueue *ImageFrameWorkQueue::HostWorkQueue() noexcept
  {
    return PlatformStrategies::Singleton().GetWorkQueue();
  }

  void ImageFrameWorkQueue::Dispatch(const Request &request) noexcept
  {
    krys_debug_assert(IsMainThread());
    _decodeQueue.push_back(request);
    DecodeNext();
  }

  void ImageFrameWorkQueue::DecodeNext() noexcept
  {
    if (_decoding || _decodeQueue.empty() || !_source)
    {
      return;
    }
    auto *workQueue = HostWorkQueue();
    if (!workQueue)
    {
      // The host took its work queue away: the requests are dropped, and drawing decodes what it needs.
      Stop();
      return;
    }

    // The queue's own decoder, over the complete data, made on the main thread.
    if (!_decoder)
    {
      _decoder = _source->CreateWorkQueueDecoder();
      if (!_decoder)
      {
        Stop();
        return;
      }
    }

    _decoding = true;
    auto request = _decodeQueue.front();
    Ref<ImageDecoder> decoder = krys::move(_decoder);
    _decoder = nullptr;
    workQueue->Dispatch(
      [decoder = krys::move(decoder), request]() mutable
      {
        // The job owns the decoder until it hands it back.
        auto frame = decoder->CreateFrameImageAtIndex(request.Index, request.Level,
                                                      DecodingOptions(DecodingMode::Synchronous));
        // The frame's image keeps its pixels: the decoder keeps only what later frames are decoded from, so
        // an animation's frames aren't held twice.
        decoder->ClearFrameBufferCache(request.Index);
        return DecodedFrame {krys::move(decoder), krys::move(frame)};
      },
      html::EventLoop::Current().Inbox(),
      [queue = CreateWeakPtr(this), generation = _generation, request](DecodedFrame decoded) mutable
      {
        if (auto *self = queue.get())
        {
          self->DecodeFinished(generation, krys::move(decoded.Decoder), request, krys::move(decoded.Image));
        }
      });
  }

  void ImageFrameWorkQueue::DecodeFinished(uint64 generation, Ref<ImageDecoder> &&decoder,
                                           const Request &request, PlatformImagePtr &&image) noexcept
  {
    // A frame decoded before a stop is dropped, with the decoder: its requests are gone.
    if (generation != _generation)
    {
      return;
    }

    Ref<ImageFrameWorkQueue> protectedThis = ShareRef(*this);
    _decoder = krys::move(decoder);
    _decoding = false;
    krys_debug_assert(!_decodeQueue.empty() && _decodeQueue.front() == request);
    _decodeQueue.pop_front();

    if (auto *source = _source)
    {
      // Even if the frame failed to decode, the source has to know.
      source->ImageFrameDecodeAtIndexHasFinished(request.Index, request.Level, request.AnimatingState,
                                                 request.Options, NativeImage::Create(krys::move(image)));
    }
    DecodeNext();
  }

  void ImageFrameWorkQueue::Stop() noexcept
  {
    krys_debug_assert(IsMainThread());

    auto decodeQueue = krys::move(_decodeQueue);
    _decodeQueue.clear();
    if (auto *source = _source)
    {
      for (auto &request : decodeQueue)
      {
        source->DestroyNativeImageAtIndex(request.Index);
      }
    }

    ++_generation;
    _decoding = false;
    _decoder = nullptr;
  }

  void ImageFrameWorkQueue::Detach() noexcept
  {
    _source = nullptr;
    _decodeQueue.clear();
    ++_generation;
    _decoding = false;
    _decoder = nullptr;
  }

  bool ImageFrameWorkQueue::IsPendingDecodingAtIndex(uint32 index, SubsamplingLevel level,
                                                     const DecodingOptions &options) const noexcept
  {
    return std::ranges::any_of(_decodeQueue,
                               [&](const Request &request)
                               {
                                 return request.Index == index && level >= request.Level
                                        && request.Options.IsCompatibleWith(options);
                               });
  }
}
