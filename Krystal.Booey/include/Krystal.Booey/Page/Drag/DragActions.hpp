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
    Edit,
    Load
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::DragDestinationAction, 3uz);

namespace krys::boo
{
  KRYS_NODISCARD constexpr EnumSet<DragDestinationAction> AnyDragDestinationAction() noexcept
  {
    return {DragDestinationAction::DHTML, DragDestinationAction::Edit, DragDestinationAction::Load};
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
  /// taking files, by loading what's dropped, by setting a colour input, or by the page itself.
  enum class DragHandlingMethod : uint8
  {
    None,
    EditPlainText,
    EditRichText,
    UploadFile,
    PageLoad,
    SetColor,
    NonDefault
  };
}
