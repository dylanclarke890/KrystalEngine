/*
 * Copyright (C) 2012, 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class LocalFrame;

  /// @brief A resource the host loaded: its response and all of its bytes.
  struct LoadedResource
  {
    ResourceResponse Response;
    Ref<SharedBuffer> Data;
  };

  using LoadResult = Expected<LoadedResource, ResourceError>;

  /// @brief How resources are loaded: the host implements it, Booey never reads files or the network itself.
  /// `data:` URLs are decoded by Booey and never reach the strategy.
  /// FIXME: Streamed responses (data arriving in parts), redirects, cancellation and synchronous loads.
  class LoaderStrategy
  {
  public:
    virtual ~LoaderStrategy() noexcept = default;

    /// @brief Loads @p request's URL for @p frame, then calls @p completionHandler once, with the resource
    /// or the reason it couldn't load.
    /// @note The handler may be called at any time on the main thread, even before this returns: Booey
    /// queues the completion as a networking task either way.
    virtual void LoadResource(LocalFrame &frame, const ResourceRequest &request,
                              const ResourceLoaderOptions &options,
                              Func<void(LoadResult &&)> &&completionHandler) noexcept = 0;
  };
}
