#pragma once

#include "Krystal.Booey/DOM/Events/EventSender.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class CSSStyleSheet;
  class DOMTokenList;
  class HTMLLinkElement;
}

namespace krys::boo::css
{
  struct ParserContext;
  class StyleSheetContents;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::html
{
  using LinkEventSender = dom::EventSender<HTMLLinkElement>;
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmllinkelement
  class HTMLLinkElement : public HTMLElement, public CachedStyleSheetClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

    enum class PendingSheetType : uint8
    {
      Unknown,
      Inactive,
      Active,
    };

    enum class DisabledState : uint8
    {
      Unset,
      EnabledViaScript,
      Disabled,
    };

  private:
    UniquePtr<DOMTokenList> _relList;
    UniquePtr<DOMTokenList> _sizes;
    UniquePtr<DOMTokenList> _blocking;

    RefPtr<CSSStyleSheet> _sheet;
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    WeakPtr<style::Scope> _styleScope;
    net::Url _url;
    Maybe<DOMString> _type;
    DOMString _media;
    bool _relIsStyleSheet {false};
    bool _relIsAlternate {false};
    bool _relIsIcon {false};
    DisabledState _disabledState {DisabledState::Unset};
    PendingSheetType _pendingSheetType {PendingSheetType::Unknown};
    bool _loading {false};
    bool _createdByParser {false};
    bool _loadedResource {false};
    bool _isRenderBlocking {false};

  public:
    HTMLLinkElement(Document &document) noexcept;

    ~HTMLLinkElement() noexcept override;

#pragma region HTMLLinkElement - https://html.spec.whatwg.org/#htmllinkelement

    /// @see https://html.spec.whatwg.org/#dom-link-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-href
    void Href(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-crossorigin
    void CrossOrigin(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rel
    void Rel(DOMString value) noexcept;

    /// @brief The `as` attribute's potential destination keyword, in its canonical case, or the empty string.
    /// @see https://html.spec.whatwg.org/#dom-link-as
    /// @see https://html.spec.whatwg.org/#attr-link-as
    KRYS_NODISCARD DOMString As() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-as
    void As(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-media
    void Media(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    KRYS_NODISCARD DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-integrity
    void Integrity(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    KRYS_NODISCARD DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-hreflang
    void HrefLang(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-sizes
    KRYS_NODISCARD DOMTokenList &Sizes() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    KRYS_NODISCARD dom::USVString ImageSrcset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesrcset
    void ImageSrcset(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    KRYS_NODISCARD DOMString ImageSizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-imagesizes
    void ImageSizes(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-fetchpriority
    void FetchPriority(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLLinkElement Obsolete members - https://html.spec.whatwg.org/#HTMLLinkElement-partial

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-charset
    void Charset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    KRYS_NODISCARD DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-rev
    void Rev(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-link-target
    void Target(DOMString value) noexcept;

#pragma endregion

#pragma region LinkStyle - https://drafts.csswg.org/cssom/#the-linkstyle-interface

    /// @brief The associated CSS style sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD CSSStyleSheet *Sheet() const noexcept;

#pragma endregion

    /// @brief Whether the sheet is disabled by the `disabled` attribute.
    KRYS_NODISCARD bool IsDisabled() const noexcept
    {
      return _disabledState == DisabledState::Disabled;
    }

    /// @brief Whether the sheet was enabled from script (by setting `disabled` to false).
    KRYS_NODISCARD bool IsEnabledViaScript() const noexcept
    {
      return _disabledState == DisabledState::EnabledViaScript;
    }

    /// @brief Whether the sheet, or one it imports, is still loading.
    KRYS_NODISCARD bool StyleSheetIsLoading() const noexcept;

    /// @brief Whether it's an alternative style sheet that hasn't been explicitly enabled (by removing its
    /// `disabled` attribute).
    /// @see https://html.spec.whatwg.org/#explicitly-enabled
    KRYS_NODISCARD bool IsAlternate() const noexcept
    {
      return _disabledState == DisabledState::Unset && _relIsStyleSheet && _relIsAlternate;
    }

    /// @brief Whether it names an icon (its `rel` has `icon`).
    /// @see https://html.spec.whatwg.org/#rel-icon
    KRYS_NODISCARD bool IsIcon() const noexcept
    {
      return _relIsIcon;
    }

    /// @brief Fires the `load` or `error` event that was waiting to be dispatched.
    void DispatchPendingEvent(html::LinkEventSender *eventSender, const StringAtom &eventType) noexcept;

    /// @brief Dispatches the `load` and `error` events waiting to be dispatched soon, of every link.
    static void DispatchPendingLoadEvents() noexcept;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void FinishParsingChildren() noexcept override;

    bool SheetLoaded() noexcept override;

    void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept override;

    void StartLoadingDynamicSheet() noexcept override;

  private:
    void SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                          const CachedCSSStyleSheet *cachedStyleSheet) noexcept override;

    /// @brief Fetches and processes the linked resource (a style sheet), or drops the sheet the link no
    /// longer has.
    /// @see https://html.spec.whatwg.org/#fetch-and-process-the-linked-resource
    void Process() noexcept;

    /// @brief Detaches the link's sheet from it.
    void ClearSheet() noexcept;

    /// @brief Disables or enables the sheet from script, loading or holding up rendering as that changes.
    void SetDisabledState(bool disabled) noexcept;

    /// @brief Records the link types of the `rel` attribute that Booey uses: stylesheet, alternate and icon.
    void ParseRel(DOMStringView value) noexcept;

    /// @brief Tells the document its icons may have changed.
    void IconMayHaveChanged() noexcept;

    /// @brief The `href` resolved against the document, or null if it's missing or only whitespace.
    KRYS_NODISCARD net::Url NonEmptyHrefUrl() const noexcept;

    /// @brief Whether the `media` attribute matches the environment (an empty list does).
    /// @see https://html.spec.whatwg.org/#matches-the-environment
    KRYS_NODISCARD bool MediaAttributeMatches() const noexcept;

    /// @brief Makes the link's sheet from @p styleSheet, with its media and title.
    void InitializeStyleSheet(Ref<css::StyleSheetContents> &&styleSheet,
                              const css::ParserContext &context) noexcept;

    /// @brief Has the style scope count the sheet as pending (holding up rendering if it's active), or stop.
    void AddPendingSheet(PendingSheetType type) noexcept;

    void RemovePendingSheet() noexcept;

    /// @brief Blocks rendering on the link if it's potentially render-blocking, or stops.
    /// @see https://html.spec.whatwg.org/#potentially-render-blocking
    void PotentiallyBlockRendering() noexcept;

    void UnblockRendering() noexcept;

    /// @brief Whether it's a style sheet link the parser created, which blocks rendering without a `blocking`
    /// attribute.
    KRYS_NODISCARD bool IsImplicitlyPotentiallyRenderBlocking() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLLinkElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLLinkElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::HTMLElement>(target);
    return element && element->IsHTMLLinkElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
