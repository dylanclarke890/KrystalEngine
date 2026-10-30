#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include <algorithm>

namespace krys::boo
{
  NavigationHistoryEntry::NavigationHistoryEntry(Navigation &navigation, Ref<HistoryItem> &&historyItem,
                                                 dom::USVString &&urlString, dom::DOMString &&key,
                                                 RefPtr<html::SerializedScriptValue> &&state,
                                                 dom::DOMString &&id) noexcept
      : _navigation(CreateWeakPtr(&navigation)), _urlString(krys::move(urlString)), _key(krys::move(key)),
        _id(krys::move(id)), _state(krys::move(state)), _associatedHistoryItem(krys::move(historyItem))
  {
    if (auto *window = navigation.Window())
    {
      _document = CreateWeakPtr(window->Document());
    }
  }

  NavigationHistoryEntry::~NavigationHistoryEntry() noexcept = default;

  Ref<NavigationHistoryEntry> NavigationHistoryEntry::Create(Navigation &navigation,
                                                             Ref<HistoryItem> &&historyItem) noexcept
  {
    dom::USVString urlString(historyItem->URL().String());
    dom::DOMString key(historyItem->UUIDIdentifier());
    return CreateRef<NavigationHistoryEntry>(navigation, krys::move(historyItem), krys::move(urlString),
                                             krys::move(key), nullptr, CreateVersion4UUIDString());
  }

  Ref<NavigationHistoryEntry> NavigationHistoryEntry::Create(Navigation &navigation,
                                                             const NavigationHistoryEntry &other) noexcept
  {
    Ref<HistoryItem> historyItem = other._associatedHistoryItem;
    RefPtr<html::SerializedScriptValue> state = ShareRefPtr(historyItem->NavigationAPIStateObject());
    if (!state)
    {
      state = other._state;
    }
    return CreateRef<NavigationHistoryEntry>(navigation, krys::move(historyItem),
                                             dom::USVString(other._urlString), dom::DOMString(other._key),
                                             krys::move(state), dom::DOMString(other._id));
  }

  bool NavigationHistoryEntry::IsDocumentFullyActive() const noexcept
  {
    auto *document = _document.get();
    return document && document->IsFullyActive();
  }

  Maybe<dom::USVString> NavigationHistoryEntry::URL() const noexcept
  {
    if (!IsDocumentFullyActive())
    {
      return null;
    }
    // FIXME: Hiding the URL of another document's entry for its referrer policy, once there are referrer
    // policies.
    return _urlString;
  }

  dom::DOMString NavigationHistoryEntry::Key() const noexcept
  {
    return IsDocumentFullyActive() ? _key : dom::DOMString();
  }

  dom::DOMString NavigationHistoryEntry::Id() const noexcept
  {
    return IsDocumentFullyActive() ? _id : dom::DOMString();
  }

  int64 NavigationHistoryEntry::Index() const noexcept
  {
    auto *navigation = _navigation.get();
    if (!IsDocumentFullyActive() || !navigation)
    {
      return -1;
    }
    auto &entries = navigation->Entries();
    auto found = std::ranges::find_if(entries, [this](auto &entry) { return entry.get() == this; });
    return found == entries.end() ? -1 : static_cast<int64>(found - entries.begin());
  }

  bool NavigationHistoryEntry::SameDocument() const noexcept
  {
    if (!IsDocumentFullyActive())
    {
      return false;
    }
    auto *frame = _document->Frame();
    auto *currentItem = frame ? frame->Loader().History().CurrentItem() : nullptr;
    return currentItem
           && currentItem->DocumentSequenceNumber() == _associatedHistoryItem->DocumentSequenceNumber();
  }

  html::SerializedScriptValue *NavigationHistoryEntry::GetState() const noexcept
  {
    return IsDocumentFullyActive() ? _state.get() : nullptr;
  }

  void NavigationHistoryEntry::SetState(RefPtr<html::SerializedScriptValue> &&state) noexcept
  {
    _state = state;
    _associatedHistoryItem->SetNavigationAPIStateObject(krys::move(state));
  }

  void NavigationHistoryEntry::DispatchDisposeEvent() noexcept
  {
    krys_debug_assert(!_hasDispatchedDisposeEvent);
    _hasDispatchedDisposeEvent = true;
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Dispose), false, false);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }
}
