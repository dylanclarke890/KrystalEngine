#include "Krystal.Booey/Storage/Storage.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Storage/StorageEvent.hpp"

namespace krys::boo
{
  Storage::Storage(LocalDOMWindow &window, Ref<StorageArea> &&storageArea) noexcept
      : _window(CreateWeakPtr(&window)), _storageArea(krys::move(storageArea))
  {
  }

  Storage::~Storage() noexcept = default;

  LocalFrame *Storage::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  uint32 Storage::Length() const noexcept
  {
    return _storageArea->Length();
  }

  Maybe<dom::DOMString> Storage::Key(uint32 index) const noexcept
  {
    return _storageArea->Key(index);
  }

  Maybe<dom::DOMString> Storage::GetItem(dom::DOMStringView key) const noexcept
  {
    return _storageArea->Item(key);
  }

  dom::ExceptionOr<void> Storage::SetItem(const dom::DOMString &key, const dom::DOMString &value) noexcept
  {
    if (!Frame())
    {
      return dom::ExceptionCode::InvalidAccessError;
    }

    // Let oldValue be null; if this's map[key] exists, set oldValue to it, and if it's value, then return.
    Maybe<dom::DOMString> oldValue = _storageArea->Item(key);
    if (oldValue == value)
    {
      return {};
    }

    // If value cannot be stored, then throw a "QuotaExceededError" DOMException.
    if (!_storageArea->SetItem(key, value, oldValue))
    {
      return dom::ExceptionCode::QuotaExceededError;
    }

    // Broadcast this with key, oldValue, and value.
    Broadcast(key, oldValue, value);
    return {};
  }

  dom::ExceptionOr<void> Storage::RemoveItem(const dom::DOMString &key) noexcept
  {
    if (!Frame())
    {
      return dom::ExceptionCode::InvalidAccessError;
    }

    // If this's map[key] does not exist, then return. Otherwise remove it and broadcast with its old value
    // and null.
    auto oldValue = _storageArea->RemoveItem(key);
    if (!oldValue)
    {
      return {};
    }
    Broadcast(key, oldValue, null);
    return {};
  }

  dom::ExceptionOr<void> Storage::Clear() noexcept
  {
    if (!Frame())
    {
      return dom::ExceptionCode::InvalidAccessError;
    }

    // If this's map is empty, then return. Otherwise clear it and broadcast with null, null, and null.
    if (!_storageArea->Clear())
    {
      return {};
    }
    Broadcast(null, null, null);
    return {};
  }

  List<dom::DOMString> Storage::SupportedPropertyNames() const noexcept
  {
    List<dom::DOMString> names;
    auto length = _storageArea->Length();
    names.reserve(length);
    for (uint32 i = 0; i < length; ++i)
    {
      if (auto key = _storageArea->Key(i))
      {
        names.push_back(krys::move(*key));
      }
    }
    return names;
  }

  void Storage::Broadcast(const Maybe<dom::DOMString> &key, const Maybe<dom::DOMString> &oldValue,
                          const Maybe<dom::DOMString> &newValue) noexcept
  {
    RefPtr<LocalDOMWindow> sourceWindow = ShareRefPtr(_window.get());
    auto *sourceDocument = sourceWindow ? sourceWindow->Document() : nullptr;
    if (!sourceDocument)
    {
      return;
    }

    auto url = dom::USVString(sourceDocument->GetURL().String());
    auto origin = sourceWindow->Origin();
    auto storageType = _storageArea->GetStorageType();
    auto *sourcePage = sourceWindow->GetPage();

    // For each Window whose document is fully active, of the same origin (and, for session storage, of the
    // same browsing session), that uses the area through another Storage object: queue a task to fire storage
    // at it, with its own Storage object as the storageArea.
    List<Ref<LocalDOMWindow>> windows;
    LocalDOMWindow::ForEachWindow(
      [&](LocalDOMWindow &window)
      {
        auto *storage =
          storageType == StorageType::Local ? window.OptionalLocalStorage() : window.OptionalSessionStorage();
        if (!storage || storage == this || &storage->Area() != _storageArea.get())
        {
          return;
        }
        if (!window.IsCurrentlyDisplayedInFrame() || window.Origin() != origin)
        {
          return;
        }
        if (storageType == StorageType::Session && window.GetPage() != sourcePage)
        {
          return;
        }
        windows.push_back(ShareRef(window));
      });

    for (auto &window : windows)
    {
      RefPtr<Storage> storage =
        ShareRefPtr(storageType == StorageType::Local ? window->OptionalLocalStorage()
                                                      : window->OptionalSessionStorage());
      window->Document()->QueueTaskToDispatchEventOnWindow(
        html::TaskSource::DOMManipulation,
        StorageEvent::Create(key, oldValue, newValue, url, krys::move(storage)));
    }
  }

  StorageEvent::StorageEvent(dom::DOMStringAtom type, const StorageEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _key(eventInitDict.Key), _oldValue(eventInitDict.OldValue),
        _newValue(eventInitDict.NewValue), _url(eventInitDict.URL), _storageArea(eventInitDict.StorageArea)
  {
  }

  StorageEvent::~StorageEvent() noexcept = default;

  Ref<StorageEvent> StorageEvent::Create(Maybe<dom::DOMString> key, Maybe<dom::DOMString> oldValue,
                                         Maybe<dom::DOMString> newValue, dom::USVString url,
                                         RefPtr<Storage> storageArea) noexcept
  {
    StorageEventInit init;
    init.Key = krys::move(key);
    init.OldValue = krys::move(oldValue);
    init.NewValue = krys::move(newValue);
    init.URL = krys::move(url);
    init.StorageArea = krys::move(storageArea);
    auto event = CreateRef<StorageEvent>(dom::DOMStringAtom(EventNames::Storage), init);
    event->_isTrusted = true;
    return event;
  }

  Storage *StorageEvent::StorageArea() const noexcept
  {
    return _storageArea.get();
  }

  void StorageEvent::InitStorageEvent(dom::DOMStringAtom type, bool bubbles, bool cancelable,
                                      Maybe<dom::DOMString> key, Maybe<dom::DOMString> oldValue,
                                      Maybe<dom::DOMString> newValue, dom::USVString url,
                                      Storage *storageArea) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    Initialize(*this, type, bubbles, cancelable);

    _key = krys::move(key);
    _oldValue = krys::move(oldValue);
    _newValue = krys::move(newValue);
    _url = krys::move(url);
    _storageArea = ShareRefPtr(storageArea);
  }
}
