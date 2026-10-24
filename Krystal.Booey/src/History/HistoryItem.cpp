#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/HTML/SerializedScriptValue.hpp"
#include <chrono>
#include <format>
#include <random>

namespace krys::boo
{
  BackForwardItemIdentifier GenerateBackForwardItemIdentifier() noexcept
  {
    static BackForwardItemIdentifier next = 0;
    return ++next;
  }

  dom::DOMString CreateVersion4UUIDString() noexcept
  {
    static std::mt19937_64 generator {std::random_device {}()};
    uint64 high = generator();
    uint64 low = generator();
    // The version (4) and the variant (10xx).
    high = (high & 0xFF'FF'FF'FF'FF'FF'0F'FFull) | 0x00'00'00'00'00'00'40'00ull;
    low = (low & 0x3F'FF'FF'FF'FF'FF'FF'FFull) | 0x80'00'00'00'00'00'00'00ull;
    auto string = std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}", high >> 32, (high >> 16) & 0xFFFF,
                              high & 0xFFFF, low >> 48, low & 0xFF'FF'FF'FF'FF'FFull);
    return dom::DOMString(string.begin(), string.end());
  }

  int64 HistoryItem::GenerateSequenceNumber() noexcept
  {
    // Initialize to the current time to reduce the likelihood of generating identifiers that overlap with
    // those from past/future browser sessions.
    static int64 next = std::chrono::duration_cast<std::chrono::microseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    return ++next;
  }

  HistoryItem::HistoryItem(url::URL &&url, dom::DOMString &&title, dom::DOMString &&alternateTitle,
                           Maybe<BackForwardItemIdentifier> itemID) noexcept
      : _url(url), _originalURL(krys::move(url)), _title(krys::move(title)),
        _displayTitle(krys::move(alternateTitle)),
        _itemID(itemID ? *itemID : GenerateBackForwardItemIdentifier()),
        _uuidIdentifier(CreateVersion4UUIDString())
  {
  }

  HistoryItem::~HistoryItem() noexcept = default;

  HistoryItem::HistoryItem(const HistoryItem &item) noexcept
      : _url(item._url), _originalURL(item._originalURL), _referrer(item._referrer), _target(item._target),
        _title(item._title), _displayTitle(item._displayTitle), _scrollPosition(item._scrollPosition),
        _lastVisitWasFailure(item._lastVisitWasFailure), _isTargetItem(item._isTargetItem),
        _itemSequenceNumber(item._itemSequenceNumber), _documentSequenceNumber(item._documentSequenceNumber),
        _substituteData(item._substituteData), _itemID(item._itemID),
        _uuidIdentifier(CreateVersion4UUIDString())
  {
  }

  Ref<HistoryItem> HistoryItem::Copy() const noexcept
  {
    return AdoptRef(*new HistoryItem(*this));
  }

  void HistoryItem::Reset() noexcept
  {
    _url = {};
    _originalURL = {};
    _referrer = {};
    _target = dom::DOMStringAtom::Null();
    _title = {};
    _displayTitle = {};
    _lastVisitWasFailure = false;
    _isTargetItem = false;
    _itemSequenceNumber = GenerateSequenceNumber();
    _stateObject = nullptr;
    _navigationAPIStateObject = nullptr;
    _documentSequenceNumber = GenerateSequenceNumber();
    _substituteData = null;
    _uuidIdentifier = CreateVersion4UUIDString();
  }

  bool HistoryItem::IsInBackForwardCache() const noexcept
  {
    return BackForwardCache::Singleton().IsInBackForwardCache(_itemID);
  }

  bool HistoryItem::HasCachedPageExpired() const noexcept
  {
    return BackForwardCache::Singleton().HasCachedPageExpired(_itemID);
  }

  void HistoryItem::SetURL(const url::URL &url) noexcept
  {
    BackForwardCache::Singleton().Remove(*this);
    SetURLString(url);
    ClearDocumentState();
  }

  void HistoryItem::SetStateObject(RefPtr<html::SerializedScriptValue> &&object) noexcept
  {
    _stateObject = krys::move(object);
  }

  void HistoryItem::SetNavigationAPIStateObject(RefPtr<html::SerializedScriptValue> &&object) noexcept
  {
    _navigationAPIStateObject = krys::move(object);
  }

  // We do same-document navigation if going to a different item and the following is true:
  // - The other item corresponds to the same document (for history entries created via pushState or fragment
  // changes).
  bool HistoryItem::ShouldDoSameDocumentNavigationTo(const HistoryItem &otherItem) const noexcept
  {
    if (_itemID == otherItem.ItemID())
    {
      return false;
    }

    return DocumentSequenceNumber() == otherItem.DocumentSequenceNumber();
  }

  bool HistoryItem::IsCurrentDocument(const dom::Document &document) const noexcept
  {
    // FIXME: We should find a better way to check if this is the current document.
    return url::EqualIgnoringFragmentIdentifier(URL(), document.GetURL());
  }
}
