/*
 * Copyright (C) 2007-2020 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief What a drop over a page may do: run the page's drag events, edit the content under it, or load
  /// what's dropped.
  enum class DragDestinationAction : uint8
  {
    DHTML,
    Edit
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::DragDestinationAction, 2uz);

namespace krys::boo
{
  KRYS_NODISCARD constexpr EnumSet<DragDestinationAction> AnyDragDestinationAction() noexcept
  {
    return {DragDestinationAction::DHTML, DragDestinationAction::Edit};
  }

  /// @brief What a drag that starts in a page drags: an element the page made draggable, an image, a link,
  /// the selection or a colour input's colour.
  enum class DragSourceAction : uint8
  {
    DHTML,
    Image,
    Link,
    Selection,
    Color
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::DragSourceAction, 5uz);

namespace krys::boo
{
  KRYS_NODISCARD constexpr EnumSet<DragSourceAction> AnyDragSourceAction() noexcept
  {
    return {DragSourceAction::DHTML, DragSourceAction::Image, DragSourceAction::Link,
            DragSourceAction::Selection, DragSourceAction::Color};
  }

  /// @brief What a drop does with what's dragged.
  enum class DragOperation : uint8
  {
    Copy,
    Link,
    Generic,
    Private,
    Move,
    Delete
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::DragOperation, 6uz);

namespace krys::boo
{
  KRYS_NODISCARD constexpr EnumSet<DragOperation> AnyDragOperation() noexcept
  {
    return {DragOperation::Copy,    DragOperation::Link, DragOperation::Generic,
            DragOperation::Private, DragOperation::Move, DragOperation::Delete};
  }

  enum class MayExtendDragSession : bool
  {
    No,
    Yes
  };

  enum class HasNonDefaultPasteboardData : bool
  {
    No,
    Yes
  };

  /// @brief How a drag over a page is handled: not at all, by editing plain or rich text, by a file input
  /// taking files, by setting a colour input, or by the page itself.
  enum class DragHandlingMethod : uint8
  {
    None,
    EditPlainText,
    EditRichText,
    UploadFile,
    SetColor,
    NonDefault
  };
}
