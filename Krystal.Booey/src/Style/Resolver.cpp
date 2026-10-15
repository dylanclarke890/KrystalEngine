#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  Ref<Resolver> Resolver::Create(dom::Document &document, ScopeType scopeType) noexcept
  {
    return AdoptRef(*new Resolver(document, scopeType));
  }

  Resolver::Resolver(dom::Document &document, ScopeType scopeType) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document)), _scopeType(scopeType), _ruleSets(*this)
  {
    Initialize();
  }

  void Resolver::Initialize() noexcept
  {
    UserAgentStyle::InitDefaultStyleSheet();

    // construct document root element default style. this is needed
    // to evaluate media queries that contain relative constraints, like "screen and (max-width: 10em)"
    // This is here instead of constructor, because when constructor is run,
    // document doesn't have documentElement
    // NOTE: this assumes that element that gets passed to styleForElement -call
    // is always from the document that owns the style selector
    auto *environment = css::mq::MediaEnvironmentFor(Document());
    if (environment)
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {environment->MediaType};
    }
    else
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {};
    }

    if (Document().DocumentElement())
    {
      // FIXME: Resolve the document element's style from the user agent rules (styleForElement with
      // MatchOnlyUserAgentRules) once there is a style builder. Media queries only read its font, which until
      // then is the default font.
      _rootDefaultStyle = DefaultStyleForElement(Document(), Document().DocumentElement().get());
    }

    if (_rootDefaultStyle && environment)
    {
      _mediaQueryEvaluator =
        css::mq::MediaQueryEvaluator {environment->MediaType, Document(), _rootDefaultStyle.get()};
    }

    _ruleSets.ResetAuthorStyle();
    _ruleSets.ResetUserAgentMediaQueryStyle();
  }

  Resolver::~Resolver() noexcept = default;

  dom::Document &Resolver::Document() noexcept
  {
    return *_document;
  }

  const dom::Document &Resolver::Document() const noexcept
  {
    return *_document;
  }

  void Resolver::AppendAuthorStyleSheets(Span<const RefPtr<css::CSSStyleSheet>> styleSheets) noexcept
  {
    _ruleSets.AppendAuthorStyleSheets(styleSheets, _mediaQueryEvaluator);

    // FIXME: Update the render view's font cascade with the document's font selector, once there are fonts.
  }

  KeyframesRuleMap &Resolver::UserAgentKeyframes() noexcept
  {
    static NeverDestroyed<KeyframesRuleMap> keyframes;
    return keyframes;
  }

  void Resolver::AddUserAgentKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    UserAgentKeyframes().insert_or_assign(animationName, RefPtr<css::KeyframesRule> {krys::move(rule)});
  }

  void Resolver::AddKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    _keyframesRuleMap.insert_or_assign(animationName, RefPtr<css::KeyframesRule> {krys::move(rule)});
    DocumentKeyframesRuleDidChange(Document(), css::CSSOMString(animationName.View()));
  }

  bool Resolver::IsAnimationNameValid(const css::CSSOMString &name) const noexcept
  {
    auto atom = css::CSSOMStringAtom(name);
    return _keyframesRuleMap.contains(atom) || UserAgentKeyframes().contains(atom);
  }

  bool Resolver::HasSelectorForId(const css::CSSOMStringAtom &idValue) const noexcept
  {
    krys_debug_assert(!idValue.View().empty());
    return _ruleSets.Features().IdsInRules.contains(idValue);
  }

  bool Resolver::HasSelectorForAttribute(const dom::Element &element,
                                         const css::CSSOMStringAtom &attributeName) const noexcept
  {
    krys_debug_assert(!attributeName.View().empty());
    if (Is<html::HTMLElement>(element) && Is<dom::HTMLDocument>(element.NodeDocument()))
    {
      return _ruleSets.Features().AttributeLowercaseLocalNamesInRules.contains(attributeName);
    }
    return _ruleSets.Features().AttributeLocalNamesInRules.contains(attributeName);
  }

  bool Resolver::HasViewportDependentMediaQueries() const noexcept
  {
    return _ruleSets.HasViewportDependentMediaQueries();
  }

  Maybe<DynamicMediaQueryEvaluationChanges> Resolver::EvaluateDynamicMediaQueries() noexcept
  {
    return _ruleSets.EvaluateDynamicMediaQueryRules(_mediaQueryEvaluator);
  }

  UniquePtr<RenderStyle> Resolver::DefaultStyleForElement(const dom::Document &document,
                                                          const dom::Element *) noexcept
  {
    auto style =
      RenderStyle::CreatePtrWithRegisteredInitialValues(document.StyleScope().GetCustomPropertyRegistry());

    FontCascadeDescription fontDescription;
    fontDescription.SetOneFamily(css::WebKitFontFamilyNames::StandardFamily());
    fontDescription.SetKeywordSizeFromIdentifier(css::ValueId::Medium);

    // FIXME: The settings' default font size (`fontSizeForKeyword`) and the zoomed computed size
    // (`computedFontSizeFromSpecifiedSize`) come with the font size functions.
    constexpr float size = 16;
    fontDescription.SetSpecifiedSize(size);
    fontDescription.SetComputedSize(size);

    style->SetFontDescription(krys::move(fontDescription));
    return style;
  }

  RefPtr<css::ViewTransitionRule> Resolver::GetViewTransitionRule() const noexcept
  {
    return _ruleSets.GetViewTransitionRule();
  }
}
