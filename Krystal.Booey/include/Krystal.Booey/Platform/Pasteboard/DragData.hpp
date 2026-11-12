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

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Page/Drag/DragActions.hpp"
#include "Krystal.Booey/Platform/Events/PlatformEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class SelectionData;

  enum class DragApplicationFlags : uint8
  {
    IsModal,
    IsSource,
    HasAttachedSheet,
    IsCopyKeyDown
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::DragApplicationFlags, 4uz);

namespace krys::boo
{
  /// @brief A drag over a page, as the host reports it: the data dragged, where the cursor is, and what the
  /// source allows a drop to do with it.
  /// @note The data dragged is the host's, which outlives the drag data.
  // FIXME: Colours (asColor, containsColor), with dragging colour inputs; smart replacing (canSmartReplace),
  // with the smart paste type.
  class DragData
  {
  public:
    enum FilenameConversionPolicy : bool
    {
      DoNotConvertFilenames,
      ConvertFilenames
    };

    enum class DraggingPurpose : uint8
    {
      ForEditing,
      ForFileUpload,
      ForColorControl
    };

  private:
    IntPoint _clientPosition;
    IntPoint _globalPosition;
    const SelectionData *_platformDragData {nullptr};
    EnumSet<DragOperation> _draggingSourceOperationMask;
    EnumSet<DragApplicationFlags> _applicationFlags;
    EnumSet<PlatformEventModifier> _modifiers;
    List<utf8_string> _fileNames;
    EnumSet<DragDestinationAction> _dragDestinationActionMask;
    bool _disallowFileAccess {false};

  public:
    /// @param clientPosition The position of the drag within the target window, with (0,0) at the top left.
    /// @param modifiers The modifier keys held, which the drag events report.
    DragData(const SelectionData *data, const IntPoint &clientPosition, const IntPoint &globalPosition,
             EnumSet<DragOperation> sourceOperationMask, EnumSet<DragApplicationFlags> flags = {},
             EnumSet<PlatformEventModifier> modifiers = {},
             EnumSet<DragDestinationAction> destinationActionMask = AnyDragDestinationAction()) noexcept;

    KRYS_NODISCARD const IntPoint &ClientPosition() const noexcept
    {
      return _clientPosition;
    }
    KRYS_NODISCARD const IntPoint &GlobalPosition() const noexcept
    {
      return _globalPosition;
    }
    void SetClientPosition(const IntPoint &clientPosition) noexcept
    {
      _clientPosition = clientPosition;
    }
    KRYS_NODISCARD EnumSet<DragApplicationFlags> Flags() const noexcept
    {
      return _applicationFlags;
    }
    KRYS_NODISCARD EnumSet<PlatformEventModifier> Modifiers() const noexcept
    {
      return _modifiers;
    }
    KRYS_NODISCARD const SelectionData *PlatformData() const noexcept
    {
      return _platformDragData;
    }
    KRYS_NODISCARD EnumSet<DragOperation> DraggingSourceOperationMask() const noexcept
    {
      return _draggingSourceOperationMask;
    }

    KRYS_NODISCARD bool
      ContainsURL(FilenameConversionPolicy filenamePolicy = ConvertFilenames) const noexcept;
    KRYS_NODISCARD bool ContainsPlainText() const noexcept;
    KRYS_NODISCARD bool
      ContainsCompatibleContent(DraggingPurpose purpose = DraggingPurpose::ForEditing) const noexcept;
    KRYS_NODISCARD utf8_string AsURL(FilenameConversionPolicy filenamePolicy = ConvertFilenames,
                                     utf8_string *title = nullptr) const noexcept;
    KRYS_NODISCARD utf8_string AsPlainText() const noexcept;
    KRYS_NODISCARD List<utf8_string> AsFilenames() const noexcept;
    KRYS_NODISCARD bool CanSmartReplace() const noexcept;
    KRYS_NODISCARD bool ContainsColor() const noexcept;
    KRYS_NODISCARD bool ContainsFiles() const noexcept;
    KRYS_NODISCARD uint32 NumberOfFiles() const noexcept;

    KRYS_NODISCARD EnumSet<DragDestinationAction> DragDestinationActionMask() const noexcept
    {
      return _dragDestinationActionMask;
    }

    void SetFileNames(List<utf8_string> &fileNames) noexcept
    {
      _fileNames = krys::move(fileNames);
    }
    KRYS_NODISCARD const List<utf8_string> &FileNames() const noexcept
    {
      return _fileNames;
    }

    void DisallowFileAccess() noexcept;

    KRYS_NODISCARD bool ShouldMatchStyleOnDrop() const noexcept;
  };
}
