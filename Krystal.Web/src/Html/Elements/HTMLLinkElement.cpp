#include "Krystal.Web/Html/Elements/HTMLLinkElement.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryParser.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Web/Dom/Collections/DOMTokenList.hpp"
#include "Krystal.Web/Dom/Events/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Algorithms/RenderBlocking.hpp"
#include "Krystal.Web/Html/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Style/ResolveForDocument.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web
{
  /// @brief A link's `as`: a keyword for each potential destination ("fetch", or a destination other than the
  /// empty string), with no missing or invalid value default.
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#attr-link-as
  template <>
  struct html::EnumeratedAttributeTraits<"as", HTMLLinkElement>
  {
    using State = html::AsAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
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

  HTMLLinkElement::HTMLLinkElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Link)
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

    // While the sheet loads, only whether it holds up rendering changes.
    if (StyleSheetIsLoading())
    {
      // Disabled, it doesn't.
      if (_disabledState == DisabledState::Disabled)
      {
        RemovePendingSheet();
      }

      // An alternate sheet enabled from script does.
      if (_relIsAlternate && _disabledState == DisabledState::EnabledViaScript)
      {
        AddPendingSheet(PendingSheetType::Active);
      }

      // So does a sheet enabled again after being disabled.
      if (!_relIsAlternate && _disabledState == DisabledState::EnabledViaScript
          && oldDisabledState == DisabledState::Disabled)
      {
        AddPendingSheet(PendingSheetType::Active);
      }

      return;
    }

    // A sheet enabled from script that never loaded (an alternate one) loads now.
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

  void HTMLLinkElement::ParseRel(DOMStringView value) noexcept
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

  net::Url HTMLLinkElement::NonEmptyHrefUrl() const noexcept
  {
    auto href = GetAttribute(u8"href");
    if (!href || infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(*href).empty())
    {
      return {};
    }
    return NodeDocument().CompleteUrl(*href);
  }

  void HTMLLinkElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
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
      ParseRel(value.value_or(DOMStringView()));
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
      auto url = NonEmptyHrefUrl();
      if (url == _url)
      {
        return;
      }
      _url = krys::move(url);
      Process();
    }
    else if (localName == u8"type")
    {
      Maybe<DOMString> type = value ? Maybe<DOMString>(*value) : null;
      if (type == _type)
      {
        return;
      }
      _type = krys::move(type);
      Process();
    }
    else if (localName == u8"media")
    {
      auto media = text::ToASCIILower(DOMString(value.value_or(DOMStringView())));
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
        _sheet->SetTitle(CSSOMString(value.value_or(DOMStringView())));
      }
    }
    else if (localName == u8"blocking")
    {
      if (html::RenderBlocking::HasRenderBlockingToken(*this))
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
        auto mimeType = DOMStringView(*_type);
        mimeType = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(
          mimeType.substr(0, mimeType.find(u8';')));
        treatAsStyleSheet = text::IsASCIICaselessEqual(mimeType, u8"text/css");
      }
    }

    // The linked resource fetch setup steps for a stylesheet:
    // https://html.spec.whatwg.org/multipage/links.html#link-type-stylesheet:linked-resource-fetch-setup-steps
    // 1. If el's disabled attribute is set, then return false.
    if (_disabledState != DisabledState::Disabled && treatAsStyleSheet && document.Frame() && _url.IsValid())
    {
      // Style sheets are UTF-8 only, whatever the charset attribute says (see DIFFERENCES.md).
      auto charset = GetAttribute(u8"charset").value_or(DOMString());

      if (CachedResourceHandle cachedSheet = std::exchange(_cachedSheet, nullptr))
      {
        RemovePendingSheet();
        cachedSheet->RemoveClient(*this);
      }

      _loading = true;

      // 2. If el contributes a script-blocking style sheet, append el to its node document's script-blocking
      //    style sheet set.
      // (Pending sheets hold up rendering, but there are no scripts to block.) Only a sheet that applies now
      // holds it up: one for other media, or an alternate one, doesn't.
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

      CachedResourceRequest request(net::ResourceRequest(_url),
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
      // The link no longer has a sheet (its rel or type changed).
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
    // https://html.spec.whatwg.org/multipage/urls-and-fetching.html#potentially-render-blocking
    if (html::RenderBlocking::HasRenderBlockingToken(*this) || IsImplicitlyPotentiallyRenderBlocking())
    {
      html::RenderBlocking::BlockRendering(*this);
      _isRenderBlocking = true;
    }
  }

  void HTMLLinkElement::UnblockRendering() noexcept
  {
    if (_isRenderBlocking)
    {
      html::RenderBlocking::UnblockRendering(*this);
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

    _url = NonEmptyHrefUrl();
    Process();
    if (_relIsIcon)
    {
      IconMayHaveChanged();
    }
  }

  void HTMLLinkElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
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

    _sheet = CSSStyleSheet::Create(krys::move(styleSheet), *this, true);
    _sheet->SetMediaQueries(css::mq::MediaQueryParser::Parse(_media, context));
    if (!dom::TreeQueries::IsInShadowTree(*this))
    {
      _sheet->SetTitle(CSSOMString(Title()));
    }
  }

  // Processing the linked resource, for a stylesheet that loaded:
  // https://html.spec.whatwg.org/multipage/links.html#link-type-stylesheet:process-the-linked-resource
  // (The load or error event is fired once the sheet's imports load, by
  // NotifyLoadedSheetAndAllCriticalSubresources.)
  void HTMLLinkElement::SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl,
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

    css::ParserContext parserContext(document, baseUrl, charset);

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

    auto styleSheet = css::StyleSheetContents::Create(CSSOMString(href), parserContext);
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
    css::mq::MediaQueryEvaluator evaluator(frameView ? frameView->MediaType() : CSSOMStringAtom(u8"screen"),
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

  void HTMLLinkElement::NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept
  {
    // https://html.spec.whatwg.org/multipage/links.html#link-type-stylesheet:process-the-linked-resource
    // 4b. Fire an event named load at el.
    // 5. Otherwise, fire an event named error at el.
    // (In an element task on the networking task source, which the fetch processes its response in.)
    html::EventLoop::Current().QueueTask(
      html::TaskSource::Networking, NodeDocument(), [element = ShareRef(*this), success = !errorOccurred]
      { dom::EventDispatcher::FireEvent(success ? event_names::Load : event_names::Error, *element); });
  }

  void HTMLLinkElement::StartLoadingDynamicSheet() noexcept
  {
    // A link has at most one sheet pending.
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
      // An inactive sheet doesn't hold up rendering; the scope only lists it in document.styleSheets.
      styleScope->DidChangeActiveStyleSheetCandidates();
      return;
    }

    styleScope->RemovePendingSheet(*this);
  }

#pragma endregion

#pragma region LinkStyle

  CSSStyleSheet *HTMLLinkElement::Sheet() const noexcept
  {
    return _sheet.get();
  }

#pragma endregion

#pragma region HTMLLinkElement

  dom::USVString HTMLLinkElement::Href() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"href");
  }

  void HTMLLinkElement::Href(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"href", krys::move(value));
  }

  Maybe<DOMString> HTMLLinkElement::CrossOrigin() const noexcept
  {
    return html::ReflectEnumerated<"crossorigin", HTMLElement, Maybe<DOMString>>(*this);
  }

  void HTMLLinkElement::CrossOrigin(Maybe<DOMString> value) noexcept
  {
    html::Reflect<Maybe<DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  DOMString HTMLLinkElement::Rel() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLLinkElement::Rel(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rel", krys::move(value));
  }

  DOMString HTMLLinkElement::As() const noexcept
  {
    // The as IDL attribute must reflect the as content attribute, limited to only known values.
    return html::ReflectEnumerated<"as", HTMLLinkElement>(*this);
  }

  void HTMLLinkElement::As(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"as", krys::move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = DOMTokenList::Create(*this, u8"rel",
                                      // The link types Krystal.Web processes.
                                      [](Document &, DOMStringView token)
                                      {
                                        return text::IsASCIICaselessEqual(token, u8"stylesheet")
                                               || text::IsASCIICaselessEqual(token, u8"alternate")
                                               || text::IsASCIICaselessEqual(token, u8"icon");
                                      });
    }

    return *_relList;
  }

  DOMString HTMLLinkElement::Media() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLLinkElement::Media(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"media", krys::move(value));
  }

  DOMString HTMLLinkElement::Integrity() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLLinkElement::Integrity(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"integrity", krys::move(value));
  }

  DOMString HTMLLinkElement::HrefLang() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLLinkElement::HrefLang(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"hreflang", krys::move(value));
  }

  DOMString HTMLLinkElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLinkElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::Sizes() noexcept
  {
    if (!_sizes)
    {
      _sizes = DOMTokenList::Create(*this, u8"sizes");
    }

    return *_sizes;
  }

  dom::USVString HTMLLinkElement::ImageSrcset() const noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"imagesrcset");
  }

  void HTMLLinkElement::ImageSrcset(dom::USVString value) noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"imagesrcset", krys::move(value));
  }

  DOMString HTMLLinkElement::ImageSizes() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"imagesizes").value_or(u8"");
  }

  void HTMLLinkElement::ImageSizes(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"imagesizes", krys::move(value));
  }

  DOMString HTMLLinkElement::ReferrerPolicy() const noexcept
  {
    return html::ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLLinkElement::ReferrerPolicy(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = DOMTokenList::Create(*this, u8"blocking", [](Document &, DOMStringView token)
                                       { return text::IsASCIICaselessEqual(token, u8"render"); });
    }

    return *_blocking;
  }

  bool HTMLLinkElement::Disabled() const noexcept
  {
    return html::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLLinkElement::Disabled(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  DOMString HTMLLinkElement::FetchPriority() const noexcept
  {
    return html::ReflectEnumerated<"fetchpriority">(*this);
  }

  void HTMLLinkElement::FetchPriority(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLinkElement Obsolete members

  DOMString HTMLLinkElement::Charset() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLLinkElement::Charset(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charset", krys::move(value));
  }

  DOMString HTMLLinkElement::Rev() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLLinkElement::Rev(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rev", krys::move(value));
  }

  DOMString HTMLLinkElement::Target() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLLinkElement::Target(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"target", krys::move(value));
  }

#pragma endregion
}
