#include "Krystal.Booey/Platform/DragData.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"

namespace krys::boo
{
  DragData::DragData(const SelectionData *data, const IntPoint &clientPosition,
                     const IntPoint &globalPosition, EnumSet<DragOperation> sourceOperationMask,
                     EnumSet<DragApplicationFlags> flags,
                     EnumSet<DragDestinationAction> destinationActionMask) noexcept
      : _clientPosition(clientPosition), _globalPosition(globalPosition), _platformDragData(data),
        _draggingSourceOperationMask(sourceOperationMask), _applicationFlags(flags),
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
