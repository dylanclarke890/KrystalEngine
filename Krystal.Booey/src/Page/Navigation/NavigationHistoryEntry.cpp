#include "Krystal.Booey/Page/Navigation/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Navigation/Navigation.hpp"
#include <algorithm>

namespace krys::boo
{
  NavigationHistoryEntry::NavigationHistoryEntry(Navigation &navigation, Ref<HistoryItem> &&historyItem,
                                                 dom::USVString urlString, dom::DOMString key,
                                                 RefPtr<html::SerializedScriptValue> &&state,
                                                 dom::DOMString id) noexcept
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
                                             dom::USVString(other._urlString), other._key, krys::move(state),
                                             other._id);
  }

  bool NavigationHistoryEntry::IsDocumentFullyActive() const noexcept
  {
    auto *document = _document.get();
    return document && document->IsFullyActive();
  }

  // https://html.spec.whatwg.org/#dom-navigationhistoryentry-url
  Maybe<dom::USVString> NavigationHistoryEntry::URL() const noexcept
  {
    // 1. Let document be this's relevant global object's associated Document.
    // 2. If document is not fully active, then return the empty string.
    if (!IsDocumentFullyActive())
    {
      return dom::USVString();
    }
    // 3. Let she be this's session history entry.
    // 4. If she's document does not equal document, and she's document state's request referrer policy is
    //    "no-referrer" or "origin", then return null.
    // FIXME: Hiding the URL of another document's entry for its referrer policy, once there are referrer
    // policies.
    // 5. Return she's URL, serialized.
    return _urlString;
  }

  // https://html.spec.whatwg.org/#dom-navigationhistoryentry-key
  dom::DOMString NavigationHistoryEntry::Key() const noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then return the empty
    //    string.
    // 2. Return this's key.
    return IsDocumentFullyActive() ? _key : dom::DOMString();
  }

  // https://html.spec.whatwg.org/#dom-navigationhistoryentry-id
  dom::DOMString NavigationHistoryEntry::Id() const noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then return the empty
    //    string.
    // 2. Return this's ID.
    return IsDocumentFullyActive() ? _id : dom::DOMString();
  }

  // https://html.spec.whatwg.org/#concept-navigationhistoryentry-index
  int64 NavigationHistoryEntry::Index() const noexcept
  {
    // 1. If nhe's relevant global object's associated Document is not fully active, then return −1.
    auto *navigation = _navigation.get();
    if (!IsDocumentFullyActive() || !navigation)
    {
      return -1;
    }
    // 2. Return the result of getting the navigation API entry index of this's session history entry within
    //    this's relevant global object's navigation API.
    auto &entries = navigation->Entries();
    auto found = std::ranges::find_if(entries, [this](auto &entry) { return entry.get() == this; });
    return found == entries.end() ? -1 : static_cast<int64>(found - entries.begin());
  }

  // https://html.spec.whatwg.org/#dom-navigationhistoryentry-samedocument
  bool NavigationHistoryEntry::SameDocument() const noexcept
  {
    // 1. Let document be this's relevant global object's associated Document.
    // 2. If document is not fully active, then return false.
    if (!IsDocumentFullyActive())
    {
      return false;
    }
    // 3. Return true if this's session history entry's document equals document, and false otherwise.
    auto *frame = _document->Frame();
    auto *currentItem = frame ? frame->Loader().History().CurrentItem() : nullptr;
    return currentItem
           && currentItem->DocumentSequenceNumber() == _associatedHistoryItem->DocumentSequenceNumber();
  }

  // https://html.spec.whatwg.org/#dom-navigationhistoryentry-getstate
  html::SerializedScriptValue *NavigationHistoryEntry::GetState() const noexcept
  {
    // 1. If this's relevant global object's associated Document is not fully active, then return undefined.
    // 2. Return StructuredDeserialize(this's session history entry's navigation API state).
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
