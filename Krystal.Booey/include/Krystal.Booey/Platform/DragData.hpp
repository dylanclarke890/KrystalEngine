#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
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
  // with the smart paste type; promised files, which are Cocoa's.
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
    List<utf8_string> _fileNames;
    EnumSet<DragDestinationAction> _dragDestinationActionMask;
    bool _disallowFileAccess {false};

  public:
    /// @param clientPosition The position of the drag within the target window, with (0,0) at the top left.
    DragData(const SelectionData *data, const IntPoint &clientPosition, const IntPoint &globalPosition,
             EnumSet<DragOperation> sourceOperationMask, EnumSet<DragApplicationFlags> flags = {},
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
