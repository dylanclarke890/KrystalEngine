#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::html
{
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
      if (text::IsASCIICaselessEqual(token, dom::DOMStringView(u8"stylesheet")))
      {
        _relIsStyleSheet = true;
      }
      else if (text::IsASCIICaselessEqual(token, dom::DOMStringView(u8"alternate")))
      {
        _relIsAlternate = true;
      }
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

    if (localName == u8"rel")
    {
      bool wasStyleSheet = _relIsStyleSheet;
      bool wasAlternate = _relIsAlternate;
      ParseRel(value.value_or(dom::DOMStringView()));
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
      Maybe<dom::DOMString> type = value ? Maybe<dom::DOMString>(dom::DOMString(*value)) : null;
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
      if (Blocking().Contains(u8"render"))
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
        treatAsStyleSheet = text::IsASCIICaselessEqual(mimeType, dom::DOMStringView(u8"text/css"));
      }
    }

    if (_disabledState != DisabledState::Disabled && treatAsStyleSheet && document.Frame() && _url.IsValid())
    {
      // FIXME: The charset attribute's encoding, or else the document's, once sheets decode other encodings.
      auto charset = GetAttribute(u8"charset").value_or(dom::DOMString());

      if (CachedResourceHandle cachedSheet = std::exchange(_cachedSheet, nullptr))
      {
        RemovePendingSheet();
        cachedSheet->RemoveClient(*this);
      }

      _loading = true;

      // Don't hold up render tree construction and script execution on stylesheets
      // that are not needed for the rendering at the moment.
      bool isActive = MediaAttributeMatches() && !IsAlternate();
      AddPendingSheet(isActive ? PendingSheetType::Active : PendingSheetType::Inactive);

      if (isActive)
      {
        PotentiallyBlockRendering();
      }
      else
      {
        UnblockRendering();
      }

      // FIXME: Load stylesheets that are not needed for the rendering immediately with low priority, the
      // nonce, integrity, referrer policy and fetch priority, once there are network loads.
      CachedResourceRequest request(ResourceRequest(url::URL(_url)),
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
    bool explicitRenderBlocking = Blocking().Contains(u8"render");
    if (explicitRenderBlocking || IsImplicitlyPotentiallyRenderBlocking())
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

    // FIXME: Whether the cached sheet is CORS-same-origin, once there are origins.
    _sheet = css::CSSStyleSheet::Create(krys::move(styleSheet), *this, true);
    _sheet->SetMediaQueries(css::mq::MediaQueryParser::Parse(_media, context));
    if (!dom::TreeQueries::IsInShadowTree(*this))
    {
      _sheet->SetTitle(css::CSSOMString(Title()));
    }

    if (!_sheet->CanAccessRules())
    {
      _sheet->Contents().SetAsLoadedFromOpaqueSource();
    }
  }

  void HTMLLinkElement::SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL,
                                         utf8_stringview charset,
                                         const CachedCSSStyleSheet *cachedStyleSheet) noexcept
  {
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
      InitializeStyleSheet(Ref<css::StyleSheetContents>(restoredSheet), parserContext);

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
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLLinkElement::Href(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLLinkElement::CrossOrigin() const noexcept
  {
    // FIXME: This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin");
  }

  void HTMLLinkElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rel() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLLinkElement::Rel(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::As() const noexcept
  {
    // FIXME: This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"as").value_or(u8"");
  }

  void HTMLLinkElement::As(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"as", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  dom::DOMString HTMLLinkElement::Media() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLLinkElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Integrity() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLLinkElement::Integrity(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::HrefLang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLLinkElement::HrefLang(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"hreflang", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLinkElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
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
    return Reflection::Reflect<dom::USVString>(*this, u8"imagesrcset");
  }

  void HTMLLinkElement::ImageSrcset(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"imagesrcset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ImageSizes() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"imagesizes").value_or(u8"");
  }

  void HTMLLinkElement::ImageSizes(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"imagesizes", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ReferrerPolicy() const noexcept
  {
    // FIXME: This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLLinkElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

  bool HTMLLinkElement::Disabled() const noexcept
  {
    // FIXME: Whenever the disabled attribute is removed, set the link element's
    // explicitly enabled attribute to true. This is more than just changing this method because the attribute
    // can be set in other ways.
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLLinkElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::FetchPriority() const noexcept
  {
    // FIXME: This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLLinkElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLinkElement Obsolete members

  dom::DOMString HTMLLinkElement::Charset() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLLinkElement::Charset(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rev() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLLinkElement::Rev(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Target() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLLinkElement::Target(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

#pragma endregion
}
