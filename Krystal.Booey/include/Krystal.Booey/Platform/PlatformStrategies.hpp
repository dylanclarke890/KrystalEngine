/*
 * Copyright (C) 2010, 2011, 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys
{
  class WorkQueue;
}

namespace krys::boo
{
  class LoaderStrategy;
  class PasteboardStrategy;
  class PathStrokeStrategy;

  /// @brief The strategies the host provides for what Booey can't do itself.
  /// @note The host owns the strategies; they must outlive their use.
  class PlatformStrategies
  {
  private:
    LoaderStrategy *_loaderStrategy {nullptr};
    PasteboardStrategy *_pasteboardStrategy {nullptr};
    PathStrokeStrategy *_pathStrokeStrategy {nullptr};
    WorkQueue *_workQueue {nullptr};

  public:
    KRYS_NODISCARD static PlatformStrategies &Singleton() noexcept;

    /// @brief How resources are loaded; null until the host sets one, and then loads fail.
    KRYS_NODISCARD LoaderStrategy *GetLoaderStrategy() const noexcept
    {
      return _loaderStrategy;
    }
    void SetLoaderStrategy(LoaderStrategy *loaderStrategy) noexcept
    {
      _loaderStrategy = loaderStrategy;
    }

    /// @brief How the system clipboard is written; null until the host sets one, and then copying does
    /// nothing.
    KRYS_NODISCARD PasteboardStrategy *GetPasteboardStrategy() const noexcept
    {
      return _pasteboardStrategy;
    }
    void SetPasteboardStrategy(PasteboardStrategy *pasteboardStrategy) noexcept
    {
      _pasteboardStrategy = pasteboardStrategy;
    }

    /// @brief How strokes become geometry; null until the host sets one, and then nothing is inside a stroke
    /// and strokes' bounds are estimated.
    KRYS_NODISCARD PathStrokeStrategy *GetPathStrokeStrategy() const noexcept
    {
      return _pathStrokeStrategy;
    }
    void SetPathStrokeStrategy(PathStrokeStrategy *pathStrokeStrategy) noexcept
    {
      _pathStrokeStrategy = pathStrokeStrategy;
    }

    /// @brief Where work that can run off the main thread goes, its completions posting back to the event
    /// loop's inbox; null until the host sets one, and then that work runs on the main thread when it's
    /// needed.
    KRYS_NODISCARD WorkQueue *GetWorkQueue() const noexcept
    {
      return _workQueue;
    }

    void SetWorkQueue(WorkQueue *workQueue) noexcept
    {
      _workQueue = workQueue;
    }
  };
}
