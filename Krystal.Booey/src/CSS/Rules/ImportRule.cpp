#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"

namespace krys::boo::css
{
  Ref<ImportRule> ImportRule::Create(const CSSOMString &href, mq::MediaQueryList &&mediaQueries,
                                     Maybe<CascadeLayerName> &&cascadeLayerName,
                                     ImportRule::SupportsCondition &&supportsCondition) noexcept
  {
    return AdoptRef(*new ImportRule(href, krys::move(mediaQueries), krys::move(cascadeLayerName),
                                    krys::move(supportsCondition)));
  }

  ImportRule::ImportRule(const CSSOMString &href, mq::MediaQueryList &&mediaQueries,
                         Maybe<CascadeLayerName> &&cascadeLayerName,
                         ImportRule::SupportsCondition &&supportsCondition) noexcept
      : Rule(RuleType::Import), _styleSheetClient(*this), _href(href),
        _mediaQueries(krys::move(mediaQueries)), _cascadeLayerName(krys::move(cascadeLayerName)),
        _supportsCondition(krys::move(supportsCondition))
  {
  }

  ImportRule::~ImportRule() noexcept
  {
    if (_styleSheet)
    {
      _styleSheet->ClearOwnerRule();
    }
    if (_cachedSheet)
    {
      _cachedSheet->RemoveClient(_styleSheetClient);
    }
  }

  void ImportRule::SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL, utf8_stringview charset,
                                    const CachedCSSStyleSheet *cachedStyleSheet) noexcept
  {
    if (_styleSheet)
    {
      _styleSheet->ClearOwnerRule();
    }

    ParserContext context =
      _parentStyleSheet ? _parentStyleSheet->GetParserContext() : ParserContext(ParserMode::HTMLStandard);
    context.Charset = charset;
    if (!baseURL.IsNull())
    {
      context.BaseURL = baseURL;
    }

    _styleSheet = StyleSheetContents::Create(this, CSSOMString(href), context);
    // FIXME: Sheets that aren't CORS-same-origin load from an opaque source too, once there are origins.
    if (_parentStyleSheet && _parentStyleSheet->LoadedFromOpaqueSource() == LoadedFromOpaqueSource::Yes)
    {
      _styleSheet->SetAsLoadedFromOpaqueSource();
    }

    bool parseSucceeded = _styleSheet->ParseAuthorStyleSheet(cachedStyleSheet);

    _loading = false;

    if (_parentStyleSheet)
    {
      if (parseSucceeded)
      {
        _parentStyleSheet->NotifyLoadedSheet(cachedStyleSheet);
      }
      else
      {
        _parentStyleSheet->SetLoadErrorOccured();
      }
      _parentStyleSheet->CheckLoaded();
    }
  }

  Ref<ImportRule> ImportRule::Copy() const noexcept
  {
    krys_unreachable();
  }

  void ImportRule::CancelLoad() noexcept
  {
    if (!IsLoading())
    {
      return;
    }

    _loading = false;
    if (_parentStyleSheet)
    {
      _parentStyleSheet->CheckLoaded();
    }
  }

  bool ImportRule::IsLoading() const noexcept
  {
    return _loading || (_styleSheet && _styleSheet->IsLoading());
  }

  void ImportRule::RequestStyleSheet() noexcept
  {
    if (!_parentStyleSheet)
    {
      return;
    }
    auto *document = _parentStyleSheet->SingleOwnerDocument();
    if (!document)
    {
      return;
    }
    // A document without a frame loads nothing.
    if (!document->Frame())
    {
      return;
    }

    url::URL absURL;
    if (!_parentStyleSheet->BaseURL().IsNull())
    {
      // use parent styleheet's URL as the base URL
      absURL = url::URL(_parentStyleSheet->BaseURL(), _href);
    }
    else
    {
      absURL = document->CompleteURL(_href);
    }

    // Check for a cycle in our import chain. If we encounter a stylesheet
    // in our parent chain with the same URL, then just bail.
    StyleSheetContents *rootSheet = _parentStyleSheet;
    for (auto *sheet = _parentStyleSheet; sheet; sheet = sheet->ParentStyleSheet())
    {
      if (url::EqualIgnoringFragmentIdentifier(absURL, sheet->BaseURL())
          || url::EqualIgnoringFragmentIdentifier(absURL, document->CompleteURL(sheet->OriginalURL())))
      {
        return;
      }
      rootSheet = sheet;
    }

    // FIXME: Skip Content Security Policy check when stylesheet is in a user agent shadow tree.
    auto options = CachedResourceLoader::DefaultCachedResourceOptions();
    options.IsLoadedFromOpaqueSource = _parentStyleSheet->LoadedFromOpaqueSource();
    CachedResourceRequest request(ResourceRequest(krys::move(absURL)), options);
    request.SetCharset(utf8_string(_parentStyleSheet->Charset()));
    request.SetInitiatorType(CachedResourceRequestInitiatorTypes::CSS);
    if (_cachedSheet)
    {
      _cachedSheet->RemoveClient(_styleSheetClient);
    }
    // FIXME: User style sheets load with their own options (requestUserCSSStyleSheet), once there are any.
    auto requested = document->GetCachedResourceLoader().RequestCSSStyleSheet(krys::move(request));
    _cachedSheet = requested ? *requested : CachedResourceHandle<CachedCSSStyleSheet>();
    if (_cachedSheet)
    {
      // if the import rule is issued dynamically, the sheet may be
      // removed from the pending sheet count, so let the doc know
      // the sheet being imported is pending.
      if (_parentStyleSheet && _parentStyleSheet->LoadCompleted() && rootSheet == _parentStyleSheet)
      {
        _parentStyleSheet->StartLoadingDynamicSheet();
      }
      _loading = true;
      _cachedSheet->AddClient(_styleSheetClient);
    }
    else if (_parentStyleSheet)
    {
      _parentStyleSheet->SetLoadErrorOccured();
      _parentStyleSheet->CheckLoaded();
    }
  }
}
