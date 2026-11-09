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

#include "Krystal.Booey/Platform/Pasteboard/DragData.hpp"
#include "Krystal.Booey/Platform/Pasteboard/SelectionData.hpp"

namespace krys::boo
{
  DragData::DragData(const SelectionData *data, const IntPoint &clientPosition,
                     const IntPoint &globalPosition, EnumSet<DragOperation> sourceOperationMask,
                     EnumSet<DragApplicationFlags> flags, EnumSet<PlatformEventModifier> modifiers,
                     EnumSet<DragDestinationAction> destinationActionMask) noexcept
      : _clientPosition(clientPosition), _globalPosition(globalPosition), _platformDragData(data),
        _draggingSourceOperationMask(sourceOperationMask), _applicationFlags(flags), _modifiers(modifiers),
        _dragDestinationActionMask(destinationActionMask)
  {
  }

  void DragData::DisallowFileAccess() noexcept
  {
    _fileNames = {};
    _disallowFileAccess = true;
  }

  bool DragData::CanSmartReplace() const noexcept
  {
    return false;
  }

  bool DragData::ContainsColor() const noexcept
  {
    return false;
  }

  bool DragData::ContainsFiles() const noexcept
  {
    return !_disallowFileAccess && _platformDragData->HasFilenames();
  }

  uint32 DragData::NumberOfFiles() const noexcept
  {
    return !_disallowFileAccess ? static_cast<uint32>(_platformDragData->Filenames().size()) : 0;
  }

  List<utf8_string> DragData::AsFilenames() const noexcept
  {
    if (_disallowFileAccess)
    {
      return {};
    }

    return _platformDragData->Filenames();
  }

  bool DragData::ContainsPlainText() const noexcept
  {
    return _platformDragData->HasText();
  }

  utf8_string DragData::AsPlainText() const noexcept
  {
    return _platformDragData->Text();
  }

  bool DragData::ContainsCompatibleContent(DraggingPurpose) const noexcept
  {
    return ContainsPlainText() || ContainsURL() || _platformDragData->HasMarkup() || ContainsColor()
           || ContainsFiles();
  }

  bool DragData::ContainsURL(FilenameConversionPolicy filenamePolicy) const noexcept
  {
    return !AsURL(filenamePolicy).empty();
  }

  utf8_string DragData::AsURL(FilenameConversionPolicy filenamePolicy, utf8_string *title) const noexcept
  {
    if (!_platformDragData->HasURL())
    {
      return {};
    }

    if (filenamePolicy != ConvertFilenames)
    {
      if (_platformDragData->GetURL().ProtocolIsFile())
      {
        return {};
      }
    }

    if (title)
    {
      *title = _platformDragData->URLLabel();
    }
    return utf8_string(_platformDragData->GetURL().String());
  }

  bool DragData::ShouldMatchStyleOnDrop() const noexcept
  {
    return false;
  }
}
