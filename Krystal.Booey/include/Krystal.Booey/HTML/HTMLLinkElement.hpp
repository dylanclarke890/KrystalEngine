#pragma once

#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/EventSender.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
  class StyleSheetContents;
  struct ParserContext;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::html
{
  class HTMLLinkElement;
  using LinkEventSender = dom::EventSender<HTMLLinkElement>;

  /// @see https://html.spec.whatwg.org/#htmllinkelement
  // FIXME: The link types other than style sheets (preloads, icons, manifests; LinkLoader), internal resource
  // links (expect), integrity, CORS and referrer policies, and fetch priorities, once there are network
  // loads.
  class HTMLLinkElement : public HTMLElement, public CachedStyleSheetClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

    enum class PendingSheetType : uint8
    {
      Unknown,
      Inactive,
      Active
    };

    enum class DisabledState : uint8
    {
      Unset,
      EnabledViaScript,
      Disabled
    };

  private:
    UniquePtr<dom::DOMTokenList> _relList;
    UniquePtr<dom::DOMTokenList> _sizes;
    UniquePtr<dom::DOMTokenList> _blocking;

    RefPtr<css::CSSStyleSheet> _sheet;
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    WeakPtr<style::Scope> _styleScope;
    url::URL _url;
    Maybe<dom::DOMString> _type;
    dom::DOMString _media;
    bool _relIsStyleSheet {false};
    bool _relIsAlternate {false};
    DisabledState _disabledState {DisabledState::Unset};
    PendingSheetType _pendingSheetType {PendingSheetType::Unknown};
    bool _loading {false};
    bool _createdByParser {false};
    bool _loadedResource {false};
    bool _isRenderBlocking {false};

  public:
    HTMLLinkElement(dom::Document &document) noexcept;
    ~HTMLLinkElement() noexcept override;

#pragma region HTMLLinkElement - https://html.spec.whatwg.org/#htmllinkelement

    /// @see https://html.spec.whatwg.org/#dom-link-href
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-href
    dom::ExceptionOr<void> Href(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    KRYS_NODISCARD dom::DOMString As() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    void As(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    void Media(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    KRYS_NODISCARD dom::DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    void Integrity(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    KRYS_NODISCARD dom::DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    void HrefLang(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-sizes
    KRYS_NODISCARD dom::DOMTokenList &Sizes() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> ImageSrcset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    dom::ExceptionOr<void> ImageSrcset(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    KRYS_NODISCARD dom::DOMString ImageSizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    void ImageSizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-blocking
    KRYS_NODISCARD dom::DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    KRYS_NODISCARD dom::DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    void FetchPriority(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLLinkElement Obsolete members - https://html.spec.whatwg.org/#HTMLLinkElement-partial

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    KRYS_NODISCARD dom::DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    void Charset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    KRYS_NODISCARD dom::DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    void Rev(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    void Target(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region LinkStyle - https://drafts.csswg.org/cssom/#the-linkstyle-interface

    /// @brief The associated CSS style sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept;

#pragma endregion

    /// @brief Whether the sheet is disabled by the `disabled` attribute.
    KRYS_NODISCARD bool IsDisabled() const noexcept
    {
      return _disabledState == DisabledState::Disabled;
    }
    KRYS_NODISCARD bool IsEnabledViaScript() const noexcept
    {
      return _disabledState == DisabledState::EnabledViaScript;
    }
    /// @brief Whether the sheet, or one it imports, is still loading.
    KRYS_NODISCARD bool StyleSheetIsLoading() const noexcept;
    KRYS_NODISCARD bool IsAlternate() const noexcept
    {
      return _relIsStyleSheet && _relIsAlternate;
    }

    void DispatchPendingEvent(LinkEventSender *eventSender, const StringAtom &eventType) noexcept;
    /// @brief Dispatches the `load` and `error` events waiting to be dispatched soon, of every link.
    static void DispatchPendingLoadEvents() noexcept;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void FinishParsingChildren() noexcept override;

    bool SheetLoaded() noexcept override;
    void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept override;
    void StartLoadingDynamicSheet() noexcept override;

  private:
    void SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL, utf8_stringview charset,
                          const CachedCSSStyleSheet *cachedStyleSheet) noexcept override;

    void Process() noexcept;
    void ClearSheet() noexcept;
    void SetDisabledState(bool disabled) noexcept;
    void ParseRel(dom::DOMStringView value) noexcept;
    KRYS_NODISCARD url::URL NonEmptyHrefURL() const noexcept;
    KRYS_NODISCARD bool MediaAttributeMatches() const noexcept;
    void InitializeStyleSheet(Ref<css::StyleSheetContents> &&styleSheet,
                              const css::ParserContext &context) noexcept;
    void AddPendingSheet(PendingSheetType type) noexcept;
    void RemovePendingSheet() noexcept;
    void PotentiallyBlockRendering() noexcept;
    void UnblockRendering() noexcept;
    KRYS_NODISCARD bool IsImplicitlyPotentiallyRenderBlocking() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLinkElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLinkElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLLinkElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();