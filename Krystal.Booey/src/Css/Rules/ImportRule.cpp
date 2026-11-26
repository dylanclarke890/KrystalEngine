#include "Krystal.Booey/Css/Rules/ImportRule.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo::css
{
  ImportRule::ImportRule(CSSOMString href, mq::MediaQueryList mediaQueries,
                         Maybe<CascadeLayerName> cascadeLayerName,
                         SupportsCondition supportsCondition) noexcept
      : Rule(RuleType::Import), _styleSheetClient(*this), _href(krys::move(href)),
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

  Ref<ImportRule> ImportRule::Create(CSSOMString href, mq::MediaQueryList mediaQueries,
                                     Maybe<CascadeLayerName> cascadeLayerName,
                                     SupportsCondition supportsCondition) noexcept
  {
    return AdoptRef(*new ImportRule(krys::move(href), krys::move(mediaQueries), krys::move(cascadeLayerName),
                                    krys::move(supportsCondition)));
  }

  Ref<ImportRule> ImportRule::Copy() noexcept
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
    // A sheet that isn't a document's, or a document without a frame, loads nothing.
    if (!document || !document->Frame())
    {
      return;
    }

    // The URL is relative to the importing sheet's, or to the document's if the sheet has none.
    auto url = _parentStyleSheet->BaseUrl().IsNull() ? document->CompleteUrl(_href)
                                                     : net::Url(_parentStyleSheet->BaseUrl(), _href);

    // A sheet that imports one of the sheets importing it would load forever, so it's skipped, as browsers
    // do. The walk also finds the root sheet, the one the document itself owns.
    StyleSheetContents *rootSheet = _parentStyleSheet;
    for (auto *sheet = _parentStyleSheet; sheet; sheet = sheet->ParentStyleSheet())
    {
      if (net::EqualIgnoringFragmentIdentifier(url, sheet->BaseUrl())
          || net::EqualIgnoringFragmentIdentifier(url, document->CompleteUrl(sheet->OriginalUrl())))
      {
        return;
      }
      rootSheet = sheet;
    }

    CachedResourceRequest request(net::ResourceRequest(krys::move(url)),
                                  CachedResourceLoader::DefaultCachedResourceOptions());
    request.SetCharset(utf8_string(_parentStyleSheet->Charset()));
    request.SetInitiatorType(CachedResourceRequestInitiatorTypes::CSS);
    if (_cachedSheet)
    {
      _cachedSheet->RemoveClient(_styleSheetClient);
    }
    // FIXME: User style sheets load with their own options (requestUserCSSStyleSheet), once there are any.
    auto requested = document->GetCachedResourceLoader().RequestCSSStyleSheet(krys::move(request));
    _cachedSheet = requested ? *requested : CachedResourceHandle<CachedCSSStyleSheet>();
    // Requesting can run code that removes the rule from its sheet.
    if (!_cachedSheet)
    {
      if (_parentStyleSheet)
      {
        _parentStyleSheet->SetLoadErrorOccured();
        _parentStyleSheet->CheckLoaded();
      }
      return;
    }

    // A rule inserted into a document's sheet after it finished loading makes it load again, which the
    // document has to wait for.
    if (_parentStyleSheet && _parentStyleSheet->LoadCompleted() && rootSheet == _parentStyleSheet)
    {
      _parentStyleSheet->StartLoadingDynamicSheet();
    }
    _loading = true;
    _cachedSheet->AddClient(_styleSheetClient);
  }

  void ImportRule::SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                                    const CachedCSSStyleSheet *cachedStyleSheet) noexcept
  {
    if (_styleSheet)
    {
      _styleSheet->ClearOwnerRule();
    }

    auto context =
      _parentStyleSheet ? _parentStyleSheet->GetParserContext() : ParserContext(ParserMode::HTMLStandard);
    context.Charset = charset;
    if (!baseUrl.IsNull())
    {
      context.BaseUrl = baseUrl;
    }
    _styleSheet = StyleSheetContents::Create(this, CSSOMString(href), context);
    bool parsed = _styleSheet->ParseAuthorStyleSheet(cachedStyleSheet);
    _loading = false;

    if (!_parentStyleSheet)
    {
      return;
    }
    if (parsed)
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
