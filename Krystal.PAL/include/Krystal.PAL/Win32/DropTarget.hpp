#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.PAL/DragAndDrop.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <ole2.h>
#include <shobjidl.h>

namespace krys::pal::win32
{
  /// @brief Reads what's dragged from @p data, in the formats Krystal reads.
  KRYS_NODISCARD DragContent ReadDragContent(IDataObject &data) noexcept;

  /// @brief A window's OLE drop target: it reports the drags from other applications over the window, and
  /// answers their sources with what the host last said a drop would do.
  /// @note The host answers a drag's input after it's reported (on its next frame), so the source hears each
  /// answer at its next move, and at the drop.
  class DropTarget final : public IDropTarget
  {
    KRYS_NON_COPY_MOVABLE(DropTarget);

  private:
    ULONG _refCount {1};
    HWND _window {nullptr};
    Func<void(const DragInput &)> _handler;
    /// @brief Shows the source's drag image over the window, as other applications' drop targets do.
    IDropTargetHelper *_helper {nullptr};
    /// @brief What a drop where the cursor is would do, as the host last said.
    Maybe<DropOperation> _operation;

  public:
    /// @param handler Called with each of the drags' input.
    DropTarget(HWND window, Func<void(const DragInput &)> &&handler) noexcept;
    ~DropTarget() noexcept;

    /// @brief Sets what a drop where the cursor is would do, as the host answered the drag's last input.
    void SetOperation(Maybe<DropOperation> operation) noexcept;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void **object) noexcept override;
    ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ULONG STDMETHODCALLTYPE Release() noexcept override;

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *data, DWORD keys, POINTL point,
                                        DWORD *effect) noexcept override;
    HRESULT STDMETHODCALLTYPE DragOver(DWORD keys, POINTL point, DWORD *effect) noexcept override;
    HRESULT STDMETHODCALLTYPE DragLeave() noexcept override;
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *data, DWORD keys, POINTL point,
                                   DWORD *effect) noexcept override;

  private:
    /// @brief The drag's input for @p action, at @p point on the screen, with @p keys (`MK_*` flags) held
    /// and @p allowed (`DROPEFFECT_*` flags) allowed by the source.
    KRYS_NODISCARD DragInput Input(DragAction action, DWORD keys, POINTL point, DWORD allowed) const noexcept;
    /// @brief The effect the source is told: what the host said a drop would do, if the source allows it.
    KRYS_NODISCARD DWORD Effect(DWORD allowed) const noexcept;
  };
}
