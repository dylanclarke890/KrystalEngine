#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Html/DragAndDrop/DataTransfer.hpp"
#include "Krystal.Web/Input/PointerEvents/MouseEvent.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#drageventinit
  struct DragEventInit : MouseEventInit
  {
    RefPtr<DataTransfer> DataTransfer;
  };

  /// @brief A drag and drop event: `dragstart`, `drag` and `dragend` at the source, and `dragenter`,
  /// `dragover`, `dragleave` and `drop` at the target, carrying the drag's data.
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#the-dragevent-interface
  class DragEvent final : public MouseEvent
  {
  private:
    RefPtr<DataTransfer> _dataTransfer;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = DragEventInit;

    /// @see https://html.spec.whatwg.org/#dom-dragevent-dragevent
    DragEvent(DOMStringAtom type, const DragEventInit &eventInitDict = {}) noexcept;

    ~DragEvent() noexcept override;

    /// @brief A trusted drag event from its parts.
    KRYS_NODISCARD static Ref<DragEvent>
      Create(DOMStringAtom type, bool canBubble, bool isCancelable, bool isComposed,
             MonotonicTime::duration timestamp, web::FrameView *view, int32 detail,
             const PointD &screenLocation, const PointD &windowLocation, double movementX, double movementY,
             EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons, EventTarget *relatedTarget,
             double force, SyntheticClickType syntheticClickType,
             DataTransfer *dataTransfer = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-dragevent-datatransfer
    KRYS_NODISCARD DataTransfer *GetDataTransfer() const noexcept
    {
      return _dataTransfer.get();
    }

    KRYS_NODISCARD bool IsDragEvent() const noexcept final
    {
      return true;
    }

  private:
    DragEvent(DOMStringAtom type, const UIEventInit &eventInitDict, web::FrameView *view,
              const PointD &screenLocation, const PointD &windowLocation, double movementX, double movementY,
              EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons, EventTarget *relatedTarget,
              double force, SyntheticClickType syntheticClickType, DataTransfer *dataTransfer) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::DragEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsDragEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
