#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

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
      : Rule(RuleType::Import), _href(href), _mediaQueries(krys::move(mediaQueries)),
        _cascadeLayerName(krys::move(cascadeLayerName)), _supportsCondition(krys::move(supportsCondition))
  {
  }

  ImportRule::~ImportRule() noexcept
  {
    if (_styleSheet)
    {
      _styleSheet->ClearOwnerRule();
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
    // FIXME: Resolve the URL against the parent sheet (or the owner document), check the import chain for a
    // cycle and request the sheet through the resource loader once it exists.
  }
}
