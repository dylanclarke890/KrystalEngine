/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Rob Buis (rwlbuis@gmail.com)
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::html
{
  /// @brief A link's `as`: a keyword for each potential destination ("fetch", or a destination other than the
  /// empty string), with no missing or invalid value default.
  /// @see https://html.spec.whatwg.org/#attr-link-as
  template <>
  struct EnumeratedAttributeTraits<"as", HTMLLinkElement>
  {
    using State = AsAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"fetch", State::Fetch},
      {u8"audio", State::Audio},
      {u8"audioworklet", State::AudioWorklet},
      {u8"document", State::Document},
      {u8"embed", State::Embed},
      {u8"font", State::Font},
      {u8"frame", State::Frame},
      {u8"iframe", State::IFrame},
      {u8"image", State::Image},
      {u8"json", State::JSON},
      {u8"manifest", State::Manifest},
      {u8"object", State::Object},
      {u8"paintworklet", State::PaintWorklet},
      {u8"report", State::Report},
      {u8"script", State::Script},
      {u8"serviceworker", State::ServiceWorker},
      {u8"sharedworker", State::SharedWorker},
      {u8"style", State::Style},
      {u8"track", State::Track},
      {u8"video", State::Video},
      {u8"webidentity", State::WebIdentity},
      {u8"worker", State::Worker},
      {u8"xslt", State::XSLT},
    };
  };

  namespace
  {
    KRYS_NODISCARD LinkEventSender &LinkLoadEventSenderSingleton() noexcept
    {
      static MainThreadNeverDestroyed<LinkEventSender> sharedLoadEventSender;
      return sharedLoadEventSender;
    }

    KRYS_NODISCARD const StringAtom &LoadEventName() noexcept
    {
      static const StringAtom name {EventNames::Load};
      return name;
    }

    KRYS_NODISCARD const StringAtom &ErrorEventName() noexcept
    {
      static const StringAtom name {EventNames::Error};
      return name;
    }
  }

  HTMLLinkElement::HTMLLinkElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Link)
  {
  }

  HTMLLinkElement::~HTMLLinkElement() noexcept
  {
    if (_sheet)
    {
      _sheet->ClearOwnerNode();
    }

    if (_cachedSheet)
    {
      _cachedSheet->RemoveClient(*this);
    }

    if (auto *styleScope = _styleScope.get())
    {
      styleScope->RemoveStyleSheetCandidateNode(*this);
    }

    LinkLoadEventSenderSingleton().CancelEvent(*this);
  }

#pragma region Style sheet loading

  void HTMLLinkElement::SetDisabledState(bool disabled) noexcept
  {
    DisabledState oldDisabledState = _disabledState;
    _disabledState = disabled ? DisabledState::Disabled : DisabledState::EnabledViaScript;
    if (oldDisabledState == _disabledState)
    {
      return;
    }

    krys_debug_assert(IsConnected() || !StyleSheetIsLoading());
    if (!IsConnected())
    {
      return;
    }

    // If we change the disabled state while the sheet is still loading, then we have to
    // perform three checks:
    if (StyleSheetIsLoading())
    {
      // Check #1: The sheet becomes disabled while loading.
      if (_disabledState == DisabledState::Disabled)
      {
        RemovePendingSheet();
      }

      // Check #2: An alternate sheet becomes enabled while it is still loading.
      if (_relIsAlternate && _disabledState == DisabledState::EnabledViaScript)
      {
        AddPendingSheet(PendingSheetType::Active);
      }

      // Check #3: A main sheet becomes enabled while it was still loading and
      // after it was disabled via script. It takes really terrible code to make this
      // happen (a double toggle for no reason essentially).
      if (!_relIsAlternate && _disabledState == DisabledState::EnabledViaScript
          && oldDisabledState == DisabledState::Disabled)
      {
        AddPendingSheet(PendingSheetType::Active);
      }

      // If the sheet is already loading just bail.
      return;
    }

    // Load the sheet, since it's never been loaded before.
    if (!_sheet && _disabledState == DisabledState::EnabledViaScript)
    {
      Process();
    }
    else
    {
      if (auto *styleScope = _styleScope.get())
      {
        styleScope->DidChangeActiveStyleSheetCandidates();
      }
      if (_sheet)
      {
        ClearSheet();
      }
    }
  }

  void HTMLLinkElement::ParseRel(dom::DOMStringView value) noexcept
  {
    // FIXME: The other link types (LinkRelAttribute), once they're loaded.
    _relIsStyleSheet = false;
    _relIsAlternate = false;
    _relIsIcon = false;
    size_t position = 0;
    while (position < value.size())
    {
      while (position < value.size() && text::IsASCIIWhitespace(value[position]))
      {
        ++position;
      }
      auto start = position;
      while (position < value.size() && !text::IsASCIIWhitespace(value[position]))
      {
        ++position;
      }
      auto token = value.substr(start, position - start);
      if (text::IsASCIICaselessEqual(token, u8"stylesheet"))
      {
        _relIsStyleSheet = true;
      }
      else if (text::IsASCIICaselessEqual(token, u8"alternate"))
      {
        _relIsAlternate = true;
      }
      else if (text::IsASCIICaselessEqual(token, u8"icon"))
      {
        _relIsIcon = true;
      }
    }
  }

  void HTMLLinkElement::IconMayHaveChanged() noexcept
  {
    if (IsConnected())
    {
      NodeDocument().IconLinksMayHaveChanged();
    }
  }

  url::URL HTMLLinkElement::NonEmptyHrefURL() const noexcept
  {
    auto href = GetAttribute(u8"href");
    if (!href || infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(*href).empty())
    {
      return {};
    }
    return NodeDocument().CompleteURL(*href);
  }

  void HTMLLinkElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // The token lists follow their attributes.
    if (localName == u8"rel" && _relList)
    {
      _relList->AssociatedAttributeValueChanged();
    }
    if (localName == u8"sizes" && _sizes)
    {
      _sizes->AssociatedAttributeValueChanged();
    }
    if (localName == u8"blocking" && _blocking)
    {
      _blocking->AssociatedAttributeValueChanged();
    }

    // The document's icons follow its icon links.
    if (_relIsIcon && (localName == u8"href" || localName == u8"type" || localName == u8"sizes"))
    {
      IconMayHaveChanged();
    }

    if (localName == u8"rel")
    {
      bool wasStyleSheet = _relIsStyleSheet;
      bool wasAlternate = _relIsAlternate;
      bool wasIcon = _relIsIcon;
      ParseRel(value.value_or(dom::DOMStringView()));
      if (wasIcon != _relIsIcon)
      {
        IconMayHaveChanged();
      }
      if (wasStyleSheet != _relIsStyleSheet || wasAlternate != _relIsAlternate)
      {
        Process();
      }
    }
    else if (localName == u8"href")
    {
      auto url = NonEmptyHrefURL();
      if (url == _url)
      {
        return;
      }
      _url = krys::move(url);
      Process();
    }
    else if (localName == u8"type")
    {
      Maybe<dom::DOMString> type = value ? Maybe<dom::DOMString>(*value) : null;
      if (type == _type)
      {
        return;
      }
      _type = krys::move(type);
      Process();
    }
    else if (localName == u8"media")
    {
      auto media = text::ToASCIILower(dom::DOMString(value.value_or(dom::DOMStringView())));
      if (media == _media)
      {
        return;
      }
      _media = krys::move(media);
      Process();
      if (_sheet && !IsDisabled())
      {
        if (auto *styleScope = _styleScope.get())
        {
          styleScope->DidChangeActiveStyleSheetCandidates();
        }
      }
    }
    else if (localName == u8"disabled")
    {
      SetDisabledState(value.has_value());
    }
    else if (localName == u8"title")
    {
      if (_sheet && !dom::TreeQueries::IsInShadowTree(*this))
      {
        _sheet->SetTitle(css::CSSOMString(value.value_or(dom::DOMStringView())));
      }
    }
    else if (localName == u8"blocking")
    {
      if (RenderBlocking::HasRenderBlockingToken(*this))
      {
        if (_loading && MediaAttributeMatches() && !IsAlternate())
        {
          PotentiallyBlockRendering();
        }
      }
      else if (!IsImplicitlyPotentiallyRenderBlocking())
      {
        UnblockRendering();
      }
    }
  }

  void HTMLLinkElement::Process() noexcept
  {
    if (!IsConnected())
    {
      krys_debug_assert(!_sheet);
      return;
    }

    // FIXME: Internal resource links (processInternalResourceLink) and the other link types (LinkLoader),
    // once there are any.

    auto &document = NodeDocument();

    bool treatAsStyleSheet = false;
    if (_relIsStyleSheet)
    {
      if (!_type)
      {
        treatAsStyleSheet = true;
      }
      else
      {
        // FIXME: A content type parser (ParsedContentType), once there's one.
        auto mimeType = dom::DOMStringView(*_type);
        mimeType = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(
          mimeType.substr(0, mimeType.find(u8';')));
        treatAsStyleSheet = text::IsASCIICaselessEqual(mimeType, u8"text/css");
      }
    }

    // The linked resource fetch setup steps for a stylesheet:
    // https://html.spec.whatwg.org/#link-type-stylesheet:linked-resource-fetch-setup-steps
    // 1. If el's disabled attribute is set, then return false.
    if (_disabledState != DisabledState::Disabled && treatAsStyleSheet && document.Frame() && _url.IsValid())
    {
      // Style sheets are UTF-8 only, whatever the charset attribute says (see DIFFERENCES.md).
      auto charset = GetAttribute(u8"charset").value_or(dom::DOMString());

      if (CachedResourceHandle cachedSheet = std::exchange(_cachedSheet, nullptr))
      {
        RemovePendingSheet();
        cachedSheet->RemoveClient(*this);
      }

      _loading = true;

      // 2. If el contributes a script-blocking style sheet, append el to its node document's script-blocking
      //    style sheet set.
      // (Pending sheets hold up rendering, but there are no scripts to block.)
      // Don't hold up render tree construction and script execution on stylesheets
      // that are not needed for the rendering at the moment.
      bool isActive = MediaAttributeMatches() && !IsAlternate();
      AddPendingSheet(isActive ? PendingSheetType::Active : PendingSheetType::Inactive);

      // 3. If el's media attribute's value matches the environment and el is potentially render-blocking,
      //    then block rendering on el.
      // 4. If el is currently render-blocking, then set request's render-blocking to true.
      // (Requests have no render-blocking flag: the host loads them all.)
      // 5. Return true.
      if (isActive)
      {
        PotentiallyBlockRendering();
      }
      else
      {
        UnblockRendering();
      }

      CachedResourceRequest request(ResourceRequest(_url),
                                    CachedResourceLoader::DefaultCachedResourceOptions());
      request.SetCharset(krys::move(charset));
      request.SetInitiatorType(LocalName().View());

      krys_debug_assert(!_cachedSheet);
      auto requested = document.GetCachedResourceLoader().RequestCSSStyleSheet(krys::move(request));
      _cachedSheet = requested ? *requested : CachedResourceHandle<CachedCSSStyleSheet>();

      if (CachedResourceHandle cachedSheet = _cachedSheet)
      {
        cachedSheet->AddClient(*this);
      }
      else
      {
        // The request may have been denied if (for example) the stylesheet is local and the document is
        // remote.
        _loading = false;
        (void)SheetLoaded();
        NotifyLoadedSheetAndAllCriticalSubresources(true);
        UnblockRendering();
      }

      return;
    }

    UnblockRendering();

    if (_sheet)
    {
      // we no longer contain a stylesheet, e.g. perhaps rel or type was changed
      ClearSheet();
      if (auto *styleScope = _styleScope.get())
      {
        styleScope->DidChangeActiveStyleSheetCandidates();
      }
    }
  }

  void HTMLLinkElement::ClearSheet() noexcept
  {
    krys_debug_assert(_sheet);
    _sheet->ClearOwnerNode();
    _sheet = nullptr;
  }

  void HTMLLinkElement::PotentiallyBlockRendering() noexcept
  {
    // Whether the element is potentially render-blocking:
    // https://html.spec.whatwg.org/#potentially-render-blocking
    if (RenderBlocking::HasRenderBlockingToken(*this) || IsImplicitlyPotentiallyRenderBlocking())
    {
      RenderBlocking::BlockRendering(*this);
      _isRenderBlocking = true;
    }
  }

  void HTMLLinkElement::UnblockRendering() noexcept
  {
    if (_isRenderBlocking)
    {
      RenderBlocking::UnblockRendering(*this);
      _isRenderBlocking = false;
    }
  }

  // https://html.spec.whatwg.org/multipage/links.html#link-type-stylesheet
  bool HTMLLinkElement::IsImplicitlyPotentiallyRenderBlocking() const noexcept
  {
    return _relIsStyleSheet && _createdByParser;
  }

  void HTMLLinkElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (!IsConnected())
    {
      return;
    }

    _createdByParser = !IsFinishedParsingChildren();
    auto &styleScope = style::Scope::ForNode(*this);
    _styleScope = CreateWeakPtr(&styleScope);
    styleScope.AddStyleSheetCandidateNode(*this, _createdByParser);

    _url = NonEmptyHrefURL();
    Process();
    if (_relIsIcon)
    {
      IconMayHaveChanged();
    }
  }

  void HTMLLinkElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (!oldAncestor.IsConnected())
    {
      return;
    }

    bool wasLoading = StyleSheetIsLoading();

    if (_sheet)
    {
      ClearSheet();
    }

    if (wasLoading)
    {
      RemovePendingSheet();
    }

    if (auto *styleScope = _styleScope.get())
    {
      styleScope->RemoveStyleSheetCandidateNode(*this);
      _styleScope = nullptr;
    }

    UnblockRendering();

    if (_relIsIcon)
    {
      NodeDocument().IconLinksMayHaveChanged();
    }
  }

  void HTMLLinkElement::FinishParsingChildren() noexcept
  {
    _createdByParser = false;
    HTMLElement::FinishParsingChildren();
  }

  void HTMLLinkElement::InitializeStyleSheet(Ref<css::StyleSheetContents> &&styleSheet,
                                             const css::ParserContext &context) noexcept
  {
    if (_sheet)
    {
      _sheet->ClearOwnerNode();
    }

    _sheet = css::CSSStyleSheet::Create(krys::move(styleSheet), *this, true);
    _sheet->SetMediaQueries(css::mq::MediaQueryParser::Parse(_media, context));
    if (!dom::TreeQueries::IsInShadowTree(*this))
    {
      _sheet->SetTitle(css::CSSOMString(Title()));
    }
  }

  // Processing the linked resource, for a stylesheet that loaded:
  // https://html.spec.whatwg.org/#link-type-stylesheet:process-the-linked-resource
  // (The load or error event is fired once the sheet's imports load, by
  // NotifyLoadedSheetAndAllCriticalSubresources.)
  void HTMLLinkElement::SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL,
                                         utf8_stringview charset,
                                         const CachedCSSStyleSheet *cachedStyleSheet) noexcept
  {
    // 7. Unblock rendering on el.
    UnblockRendering();
    if (!IsConnected())
    {
      krys_debug_assert(!_sheet);
      return;
    }
    auto &document = NodeDocument();
    if (!document.Frame())
    {
      return;
    }

    // Completing the sheet load may cause the element to be removed.
    Ref protectedThis = ShareRef(*this);

    // FIXME: Integrity checks, once there's subresource integrity.

    css::ParserContext parserContext(document, baseURL, charset);

    if (auto restoredSheet =
          const_cast<CachedCSSStyleSheet *>(cachedStyleSheet)->RestoreParsedStyleSheet(parserContext))
    {
      krys_debug_assert(restoredSheet->IsCacheable());
      krys_debug_assert(!restoredSheet->IsLoading());
      InitializeStyleSheet(krys::move(restoredSheet), parserContext);

      _loading = false;
      (void)SheetLoaded();
      NotifyLoadedSheetAndAllCriticalSubresources(false);
      return;
    }

    auto styleSheet = css::StyleSheetContents::Create(css::CSSOMString(href), parserContext);
    InitializeStyleSheet(Ref<css::StyleSheetContents>(styleSheet), parserContext);

    if (!styleSheet->ParseAuthorStyleSheet(cachedStyleSheet))
    {
      _loading = false;
      (void)SheetLoaded();
      NotifyLoadedSheetAndAllCriticalSubresources(true);
      return;
    }

    _loading = false;
    styleSheet->NotifyLoadedSheet(cachedStyleSheet);
    styleSheet->CheckLoaded();

    if (styleSheet->IsCacheable())
    {
      const_cast<CachedCSSStyleSheet *>(cachedStyleSheet)->SaveParsedStyleSheet(krys::move(styleSheet));
    }
  }

  bool HTMLLinkElement::StyleSheetIsLoading() const noexcept
  {
    if (_loading)
    {
      return true;
    }
    if (!_sheet)
    {
      return false;
    }
    return _sheet->Contents().IsLoading();
  }

  bool HTMLLinkElement::MediaAttributeMatches() const noexcept
  {
    if (_media.empty())
    {
      return true;
    }

    auto &document = NodeDocument();
    Maybe<RenderStyle> documentStyle;
    if (document.HasLivingRenderTree())
    {
      documentStyle = style::ResolveForDocument(document);
    }
    auto mediaQueryList = css::mq::MediaQueryParser::Parse(_media, document.CssParserContext());

    auto *frameView = document.View();
    krys_debug_assert(frameView);
    css::mq::MediaQueryEvaluator evaluator(frameView ? frameView->MediaType()
                                                     : css::CSSOMStringAtom(u8"screen"),
                                           document, documentStyle ? &*documentStyle : nullptr);
    return evaluator.Evaluate(mediaQueryList);
  }

  bool HTMLLinkElement::SheetLoaded() noexcept
  {
    if (!StyleSheetIsLoading())
    {
      RemovePendingSheet();
      return true;
    }
    return false;
  }

  void HTMLLinkElement::DispatchPendingLoadEvents() noexcept
  {
    LinkLoadEventSenderSingleton().DispatchPendingEvents();
  }

  void HTMLLinkElement::DispatchPendingEvent(LinkEventSender *eventSender,
                                             const StringAtom &eventType) noexcept
  {
    krys_debug_assert(eventSender == &LinkLoadEventSenderSingleton());
    (void)eventSender;
    (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(eventType), *this);
  }

  void HTMLLinkElement::NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept
  {
    _loadedResource = !errorOccurred;
    LinkLoadEventSenderSingleton().DispatchEventSoon(*this,
                                                     _loadedResource ? LoadEventName() : ErrorEventName());
  }

  void HTMLLinkElement::StartLoadingDynamicSheet() noexcept
  {
    // We don't support multiple active sheets.
    krys_debug_assert(_pendingSheetType < PendingSheetType::Active);
    AddPendingSheet(PendingSheetType::Active);
  }

  void HTMLLinkElement::AddPendingSheet(PendingSheetType type) noexcept
  {
    if (type <= _pendingSheetType)
    {
      return;
    }
    _pendingSheetType = type;

    if (_pendingSheetType == PendingSheetType::Inactive)
    {
      return;
    }
    if (auto *styleScope = _styleScope.get())
    {
      styleScope->AddPendingSheet(*this);
    }
  }

  void HTMLLinkElement::RemovePendingSheet() noexcept
  {
    PendingSheetType type = _pendingSheetType;
    _pendingSheetType = PendingSheetType::Unknown;

    if (type == PendingSheetType::Unknown)
    {
      return;
    }

    auto *styleScope = _styleScope.get();
    if (!styleScope)
    {
      return;
    }
    if (type == PendingSheetType::Inactive)
    {
      // Document just needs to know about the sheet for exposure through document.styleSheets
      styleScope->DidChangeActiveStyleSheetCandidates();
      return;
    }

    styleScope->RemovePendingSheet(*this);
  }

#pragma endregion

#pragma region LinkStyle

  css::CSSStyleSheet *HTMLLinkElement::Sheet() const noexcept
  {
    return _sheet.get();
  }

#pragma endregion

#pragma region HTMLLinkElement

  dom::USVString HTMLLinkElement::Href() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLLinkElement::Href(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLLinkElement::CrossOrigin() const noexcept
  {
    return ReflectEnumerated<"crossorigin", HTMLElement, Maybe<dom::DOMString>>(*this);
  }

  void HTMLLinkElement::CrossOrigin(Maybe<dom::DOMString> value) noexcept
  {
    Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rel() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLLinkElement::Rel(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::As() const noexcept
  {
    // The as IDL attribute must reflect the as content attribute, limited to only known values.
    return ReflectEnumerated<"as", HTMLLinkElement>(*this);
  }

  void HTMLLinkElement::As(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"as", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"rel",
        // The link types Booey processes.
        [](dom::Document &, dom::DOMStringView token)
        {
          return text::IsASCIICaselessEqual(token, u8"stylesheet")
                 || text::IsASCIICaselessEqual(token, u8"alternate")
                 || text::IsASCIICaselessEqual(token, u8"icon");
        });
    }

    return *_relList;
  }

  dom::DOMString HTMLLinkElement::Media() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLLinkElement::Media(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Integrity() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLLinkElement::Integrity(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::HrefLang() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLLinkElement::HrefLang(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"hreflang", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLinkElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::Sizes() noexcept
  {
    if (!_sizes)
    {
      _sizes = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"sizes");
    }

    return *_sizes;
  }

  dom::USVString HTMLLinkElement::ImageSrcset() const noexcept
  {
    return Reflect<dom::USVString>(*this, u8"imagesrcset");
  }

  void HTMLLinkElement::ImageSrcset(dom::USVString value) noexcept
  {
    return Reflect<dom::USVString>(*this, u8"imagesrcset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ImageSizes() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"imagesizes").value_or(u8"");
  }

  void HTMLLinkElement::ImageSizes(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"imagesizes", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ReferrerPolicy() const noexcept
  {
    return ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLLinkElement::ReferrerPolicy(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"blocking", [](dom::Document &, dom::DOMStringView token)
        { return text::IsASCIICaselessEqual(token, u8"render"); });
    }

    return *_blocking;
  }

  bool HTMLLinkElement::Disabled() const noexcept
  {
    return Reflect<bool>(*this, u8"disabled");
  }

  void HTMLLinkElement::Disabled(bool value) noexcept
  {
    Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::FetchPriority() const noexcept
  {
    return ReflectEnumerated<"fetchpriority">(*this);
  }

  void HTMLLinkElement::FetchPriority(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLinkElement Obsolete members

  dom::DOMString HTMLLinkElement::Charset() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLLinkElement::Charset(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rev() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLLinkElement::Rev(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Target() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLLinkElement::Target(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

#pragma endregion
}
