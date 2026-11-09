/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Page/Drag/DragActions.hpp"
#include "Krystal.Booey/Page/Drag/DragItem.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  class DataTransfer;
}

namespace krys::boo
{
  class DragData;
  class LocalFrame;

  /// @brief What a page's drag controller asks of its host: which drags may start, and running a drag that
  /// starts in the page, calling the drag controller back as the cursor moves and the drag ends.
  class DragClient
  {
    KRYS_NON_COPY_MOVABLE(DragClient);

  public:
    DragClient() noexcept = default;
    virtual ~DragClient() noexcept = default;

    virtual void WillPerformDragDestinationAction(DragDestinationAction action,
                                                  const DragData &dragData) noexcept = 0;
    virtual void WillPerformDragSourceAction(DragSourceAction action, const IntPoint &point,
                                             dom::DataTransfer &dataTransfer) noexcept = 0;
    virtual void DidConcludeEditDrag() noexcept
    {
    }
    /// @brief The drags that may start at @p rootViewPoint.
    KRYS_NODISCARD virtual EnumSet<DragSourceAction>
      DragSourceActionMaskForPoint(const IntPoint &rootViewPoint) noexcept = 0;

    /// @brief Runs a drag of @p dataTransfer's data from @p frame, as @p dragItem describes.
    virtual void StartDrag(DragItem dragItem, dom::DataTransfer &dataTransfer,
                           LocalFrame &frame) noexcept = 0;
    virtual void DragEnded() noexcept
    {
    }
  };

  /// @brief The drag client of a page that has no host: no drag starts in the page.
  class EmptyDragClient final : public DragClient
  {
  public:
    void WillPerformDragDestinationAction(DragDestinationAction, const DragData &) noexcept final
    {
    }
    void WillPerformDragSourceAction(DragSourceAction, const IntPoint &, dom::DataTransfer &) noexcept final
    {
    }
    KRYS_NODISCARD EnumSet<DragSourceAction> DragSourceActionMaskForPoint(const IntPoint &) noexcept final
    {
      return {};
    }
    void StartDrag(DragItem, dom::DataTransfer &, LocalFrame &) noexcept final
    {
    }
  };
}
