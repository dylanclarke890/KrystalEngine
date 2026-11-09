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
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief Immutable, shared bytes: a resource's data, or what of it has arrived.
  // FIXME: The fragmented buffer (FragmentedSharedBuffer, its data segments and builder), file and memory
  // mapped data, once loading streams data.
  class SharedBuffer final : public RefCounted<SharedBuffer>
  {
    KRYS_NON_COPY_MOVABLE(SharedBuffer);

  private:
    List<uint8> _data;

    explicit SharedBuffer(List<uint8> &&data) noexcept : _data(krys::move(data))
    {
    }

  public:
    KRYS_NODISCARD static Ref<SharedBuffer> Create() noexcept
    {
      return AdoptRef(*new SharedBuffer({}));
    }
    KRYS_NODISCARD static Ref<SharedBuffer> Create(List<uint8> &&data) noexcept
    {
      return AdoptRef(*new SharedBuffer(krys::move(data)));
    }
    KRYS_NODISCARD static Ref<SharedBuffer> Create(Span<const uint8> data) noexcept
    {
      return AdoptRef(*new SharedBuffer(List<uint8>(data.begin(), data.end())));
    }

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept
    {
      return {_data.data(), _data.size()};
    }
    KRYS_NODISCARD const uint8 *Data() const noexcept
    {
      return _data.data();
    }
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _data.size();
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _data.empty();
    }

    /// @brief The byte at @p position.
    KRYS_NODISCARD uint8 operator[](size_t position) const noexcept
    {
      return _data[position];
    }
  };
}
