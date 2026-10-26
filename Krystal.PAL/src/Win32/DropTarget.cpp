#include "Krystal.PAL/Win32/DropTarget.hpp"
#include "Krystal.Core/Utils/Move.hpp"

#include <shlobj.h>

namespace krys::pal::win32
{
  namespace
  {
    KRYS_NODISCARD DropOperations ToDropOperations(DWORD effects) noexcept
    {
      DropOperations operations;
      if (effects & DROPEFFECT_COPY)
      {
        operations.Add(DropOperation::Copy);
      }
      if (effects & DROPEFFECT_MOVE)
      {
        operations.Add(DropOperation::Move);
      }
      if (effects & DROPEFFECT_LINK)
      {
        operations.Add(DropOperation::Link);
      }
      return operations;
    }

    KRYS_NODISCARD DWORD ToDropEffect(DropOperation operation) noexcept
    {
      switch (operation)
      {
        case DropOperation::Copy: return DROPEFFECT_COPY;
        case DropOperation::Move: return DROPEFFECT_MOVE;
        case DropOperation::Link: return DROPEFFECT_LINK;
      }
      return DROPEFFECT_NONE;
    }

    KRYS_NODISCARD KeyModifiers ToKeyModifiers(DWORD keys) noexcept
    {
      KeyModifiers modifiers;
      if (keys & MK_SHIFT)
      {
        modifiers.Add(KeyModifier::Shift);
      }
      if (keys & MK_CONTROL)
      {
        modifiers.Add(KeyModifier::Control);
      }
      if (keys & MK_ALT)
      {
        modifiers.Add(KeyModifier::Alt);
      }
      return modifiers;
    }
  }

  DropTarget::DropTarget(HWND window, Func<void(const DragInput &)> &&handler) noexcept
      : _window(window), _handler(krys::move(handler))
  {
    // Without the helper, the source's drag image disappears over the window.
    if (FAILED(
          ::CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_helper))))
    {
      _helper = nullptr;
    }
  }

  DropTarget::~DropTarget() noexcept
  {
    if (_helper)
    {
      _helper->Release();
    }
  }

  void DropTarget::SetOperation(Maybe<DropOperation> operation) noexcept
  {
    _operation = operation;
  }

  HRESULT DropTarget::QueryInterface(REFIID id, void **object) noexcept
  {
    if (!object)
    {
      return E_POINTER;
    }
    if (id == IID_IUnknown || id == IID_IDropTarget)
    {
      *object = static_cast<IDropTarget *>(this);
      AddRef();
      return S_OK;
    }
    *object = nullptr;
    return E_NOINTERFACE;
  }

  ULONG DropTarget::AddRef() noexcept
  {
    return ++_refCount;
  }

  ULONG DropTarget::Release() noexcept
  {
    const ULONG count = --_refCount;
    if (!count)
    {
      delete this;
    }
    return count;
  }

  HRESULT DropTarget::DragEnter(IDataObject *data, DWORD keys, POINTL point, DWORD *effect) noexcept
  {
    if (!effect)
    {
      return E_INVALIDARG;
    }

    // The host hasn't answered yet.
    _operation = null;
    auto input = Input(DragAction::Entered, keys, point, *effect);
    if (data)
    {
      input.Content = ReadDragContent(*data);
    }
    _handler(input);

    *effect = Effect(*effect);
    if (_helper && data)
    {
      POINT screenPoint {point.x, point.y};
      _helper->DragEnter(_window, data, &screenPoint, *effect);
    }
    return S_OK;
  }

  HRESULT DropTarget::DragOver(DWORD keys, POINTL point, DWORD *effect) noexcept
  {
    if (!effect)
    {
      return E_INVALIDARG;
    }

    _handler(Input(DragAction::Moved, keys, point, *effect));

    *effect = Effect(*effect);
    if (_helper)
    {
      POINT screenPoint {point.x, point.y};
      _helper->DragOver(&screenPoint, *effect);
    }
    return S_OK;
  }

  HRESULT DropTarget::DragLeave() noexcept
  {
    DragInput input;
    input.Action = DragAction::Left;
    _handler(input);
    _operation = null;

    if (_helper)
    {
      _helper->DragLeave();
    }
    return S_OK;
  }

  HRESULT DropTarget::Drop(IDataObject *data, DWORD keys, POINTL point, DWORD *effect) noexcept
  {
    if (!effect)
    {
      return E_INVALIDARG;
    }

    _handler(Input(DragAction::Dropped, keys, point, *effect));

    *effect = Effect(*effect);
    _operation = null;
    if (_helper && data)
    {
      POINT screenPoint {point.x, point.y};
      _helper->Drop(data, &screenPoint, *effect);
    }
    return S_OK;
  }

  DragInput DropTarget::Input(DragAction action, DWORD keys, POINTL point, DWORD allowed) const noexcept
  {
    POINT clientPoint {point.x, point.y};
    ::ScreenToClient(_window, &clientPoint);

    DragInput input;
    input.Action = action;
    input.ClientX = static_cast<float>(clientPoint.x);
    input.ClientY = static_cast<float>(clientPoint.y);
    input.Modifiers = ToKeyModifiers(keys);
    input.AllowedOperations = ToDropOperations(allowed);
    return input;
  }

  DWORD DropTarget::Effect(DWORD allowed) const noexcept
  {
    if (!_operation)
    {
      return DROPEFFECT_NONE;
    }
    return ToDropEffect(*_operation) & allowed;
  }
}
