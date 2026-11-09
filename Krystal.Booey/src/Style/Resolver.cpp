/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012, 2013 Google Inc. All rights reserved.
 * Copyright (C) 2014 Igalia S.L.
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

#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Animation/Keyframes/BlendingKeyframes.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/Builder/Builder.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Builder/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Rules/ElementRuleCollector.hpp"
#include "Krystal.Booey/Style/Rules/MatchResultCache.hpp"
#include "Krystal.Booey/Style/Rules/RuleSetBuilder.hpp"
#include "Krystal.Booey/Style/Rules/UserAgentStyle.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/Style/Values/Animations/Animation.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRangeName.hpp"
#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::style
{
  class Resolver::State
  {
  private:
    const dom::Element *_element {};
    UniquePtr<RenderStyle> _style;
    const RenderStyle *_parentStyle {};
    UniquePtr<const RenderStyle> _ownedParentStyle;
    const RenderStyle *_rootElementStyle {};
    style::TreeResolutionState *_treeResolutionState {};

  public:
    State() noexcept = default;

    State(const dom::Element &element, const RenderStyle *parentStyle,
          const RenderStyle *documentElementStyle, style::TreeResolutionState *treeResolutionState) noexcept
        : _element(&element), _parentStyle(parentStyle), _treeResolutionState(treeResolutionState)
    {
      krys_debug_assert(element.IsConnected());

      auto &document = element.NodeDocument();
      auto documentElement = document.DocumentElement();
      if (!documentElement || documentElement.get() == &element)
      {
        _rootElementStyle = document.InitialContainingBlockStyle();
      }
      else if (documentElementStyle)
      {
        _rootElementStyle = documentElementStyle;
      }
      else if (auto *documentElementRenderStyle = documentElement->GetRenderStyle())
      {
        _rootElementStyle = documentElementRenderStyle;
      }
      else
      {
        _rootElementStyle = document.InitialContainingBlockStyle();
      }
    }

    State(State &&) noexcept = default;
    State &operator=(State &&) noexcept = default;

    KRYS_NODISCARD const dom::Element *Element() const noexcept
    {
      return _element;
    }

    void SetStyle(UniquePtr<RenderStyle> style) noexcept
    {
      _style = krys::move(style);
    }

    KRYS_NODISCARD RenderStyle *Style() const noexcept
    {
      return _style.get();
    }

    KRYS_NODISCARD UniquePtr<RenderStyle> TakeStyle() noexcept
    {
      return krys::move(_style);
    }

    void SetParentStyle(UniquePtr<RenderStyle> parentStyle) noexcept
    {
      _ownedParentStyle = krys::move(parentStyle);
      _parentStyle = _ownedParentStyle.get();
    }

    KRYS_NODISCARD const RenderStyle *ParentStyle() const noexcept
    {
      return _parentStyle;
    }

    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept
    {
      return _rootElementStyle;
    }

    KRYS_NODISCARD style::TreeResolutionState *GetTreeResolutionState() const noexcept
    {
      return _treeResolutionState;
    }
  };

  Ref<Resolver> Resolver::Create(dom::Document &document, ScopeType scopeType) noexcept
  {
    return AdoptRef(*new Resolver(document, scopeType));
  }

  Resolver::Resolver(dom::Document &document, ScopeType scopeType) noexcept
      : _document(CreateWeakPtr(&document)), _scopeType(scopeType), _ruleSets(*this),
        _matchedDeclarationsCache(*this)
  {
    // FIXME: Settings: authorAndUserStylesEnabled (on by default).
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
    auto *view = Document().View();
    if (view)
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {view->MediaType()};
    }
    else
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {};
    }

    if (auto documentElement = Document().DocumentElement())
    {
      _rootDefaultStyle = StyleForElement(*documentElement, {Document().InitialContainingBlockStyle()},
                                          RuleMatchingBehavior::MatchOnlyUserAgentRules)
                            .Style;
      // Turn off assertion against font lookups during style resolver initialization. We may need root style
      // font for media queries.
      auto &fontSelector = Document().GetFontSelector();
      fontSelector.IncrementIsComputingRootStyleFont();
      _rootDefaultStyle->GetFontCascade().Update(ShareRefPtr<FontSelector>(&fontSelector));
      std::ignore = _rootDefaultStyle->GetFontCascade().PrimaryFont();
      fontSelector.DecrementIsComputingRootStyleFont();
    }

    if (_rootDefaultStyle && view)
    {
      _mediaQueryEvaluator =
        css::mq::MediaQueryEvaluator {view->MediaType(), Document(), _rootDefaultStyle.get()};
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

    if (Document().GetRenderView())
    {
      Document().RenderViewStyle().GetFontCascade().Update(
        ShareRefPtr<FontSelector>(&Document().GetFontSelector()));
    }
  }

  KeyframesRuleMap &Resolver::UserAgentKeyframes() noexcept
  {
    static NeverDestroyed<KeyframesRuleMap> keyframes;
    return keyframes;
  }

  void Resolver::AddUserAgentKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    UserAgentKeyframes().insert_or_assign(animationName, krys::move(rule));
  }

  void Resolver::AddKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    _keyframesRuleMap.insert_or_assign(animationName, krys::move(rule));
    Document().KeyframesRuleDidChange(css::CSSOMString(animationName.View()));
  }

  auto Resolver::InitializeStateAndStyle(const dom::Element &element, const ResolutionContext &context,
                                         UniquePtr<RenderStyle> &&initialStyle) noexcept -> State
  {
    auto state =
      State {element, context.ParentStyle, context.DocumentElementStyle, context.TreeResolutionState};

    if (initialStyle)
    {
      state.SetStyle(krys::move(initialStyle));
    }
    else if (state.ParentStyle())
    {
      state.SetStyle(RenderStyle::CreatePtrWithRegisteredInitialValues(
        Document().StyleScope().GetCustomPropertyRegistry()));
      if (&element == Document().DocumentElement().get() && !context.IsSVGUseTreeRoot)
      {
        // Initial values for custom properties are inserted to the document element style. Don't overwrite
        // them.
        state.Style()->InheritIgnoringCustomPropertiesFrom(*state.ParentStyle());
      }
      else
      {
        state.Style()->InheritFrom(*state.ParentStyle());
      }
    }
    else
    {
      state.SetStyle(DefaultStyleForElement(Document(), &element));
      state.SetParentStyle(RenderStyle::ClonePtr(*state.Style()));
    }

    // BuilderState::useSVGZoomRulesForLength equivalent
    if (auto *svgElement = DynamicDowncast<svg::SVGElement>(element);
        svgElement && !(Is<svg::SVGSvgElement>(*svgElement) && element.ParentNode()))
    {
      state.Style()->SetUseSVGZoomRulesForLength(true);
    }

    if (css::IsLink(element))
    {
      state.Style()->SetIsLink(true);
    }

    return state;
  }

  BuilderContext Resolver::GetBuilderContext(State &state) const noexcept
  {
    return BuilderContext {.Document = ShareRefPtr(&Document()),
                           .ParentStyle = state.ParentStyle(),
                           .RootElementStyle = state.RootElementStyle(),
                           .Element = ShareRefPtr(state.Element()),
                           .TreeResolutionState = state.GetTreeResolutionState()};
  }

  UnadjustedStyle Resolver::UnadjustedStyleForElement(dom::Element &element, const ResolutionContext &context,
                                                      RuleMatchingBehavior matchingBehavior) noexcept
  {
    auto state = InitializeStateAndStyle(element, context);
    auto &style = *state.Style();

    UserAgentStyle::EnsureDefaultStyleSheetsForElement(element);

    ElementRuleCollector collector(element, _ruleSets, context.SelectorMatchingState);
    collector.SetMedium(_mediaQueryEvaluator);

    if (matchingBehavior == RuleMatchingBehavior::MatchOnlyUserAgentRules)
    {
      collector.MatchUARules();
    }
    else
    {
      collector.MatchAllRules(_matchAuthorAndUserStyles,
                              matchingBehavior != RuleMatchingBehavior::MatchAllRulesExcludingSMIL);
    }

    if (collector.MatchedPseudoElements())
    {
      style.SetHasPseudoStyles(collector.MatchedPseudoElements());
    }

    auto elementStyleRelations = CommitRelationsToRenderStyle(style, element, collector.StyleRelations());

    ApplyMatchedProperties(state, collector.GetMatchResult(), PropertyCascade::NormalProperties());

    return {.Style = state.TakeStyle(),
            .Relations = krys::move(elementStyleRelations),
            .MatchResult = collector.ReleaseMatchResult()};
  }

  ResolvedStyle Resolver::StyleForElement(dom::Element &element, const ResolutionContext &context,
                                          RuleMatchingBehavior matchingBehavior) noexcept
  {
    auto unadjustedStyle = UnadjustedStyleForElement(element, context, matchingBehavior);
    auto &parentStyle = context.ParentStyle ? *context.ParentStyle : RenderStyle::DefaultStyleSingleton();

    auto style = krys::move(unadjustedStyle.Style);

    Adjuster adjuster(Document(), parentStyle, context.ParentBoxStyle, &element);
    adjuster.Adjust(*style);

    return {.Style = krys::move(style),
            .Relations = krys::move(unadjustedStyle.Relations),
            .MatchResult = krys::move(unadjustedStyle.MatchResult)};
  }

  UnadjustedStyle Resolver::UnadjustedStyleForCachedMatchResult(dom::Element &element,
                                                                const ResolutionContext &context,
                                                                CachedMatchResult &&cachedResult) noexcept
  {
    auto &unadjustedStyle = cachedResult.UnadjustedStyle;

    if (cachedResult.ChangedProperties.IsEmpty())
    {
      // The cached result can be used as-is.
      return krys::move(unadjustedStyle);
    }

    bool applyPartially = !cachedResult.ChangedProperties.Ids.empty();

    auto state =
      InitializeStateAndStyle(element, context, applyPartially ? krys::move(unadjustedStyle.Style) : nullptr);
    if (!applyPartially)
    {
      state.Style()->CopyPseudoElementBitsFrom(*unadjustedStyle.Style);
      CopyRelations(*state.Style(), *unadjustedStyle.Style);
    }

    ApplyMatchedProperties(state, *unadjustedStyle.MatchResult, krys::move(cachedResult.ChangedProperties));

    return {.Style = state.TakeStyle(),
            .Relations = krys::move(unadjustedStyle.Relations),
            .MatchResult = unadjustedStyle.MatchResult};
  }

  UniquePtr<RenderStyle> Resolver::StyleForKeyframe(dom::Element &element, const RenderStyle &elementStyle,
                                                    const ResolutionContext &context,
                                                    const css::KeyframeRule &keyframe,
                                                    BlendingKeyframe &blendingKeyframe) const noexcept
  {
    // Add all the animating properties to the keyframe.
    bool hasRevert = false;
    for (auto propertyReference : keyframe.Properties())
    {
      auto unresolvedProperty = propertyReference.Id();
      // The animation-composition and animation-timing-function within keyframes are special
      // because they are not animated; they just describe the composite operation and timing
      // function between this keyframe and the next.
      if (css::Property::IsDirectionAwareProperty(unresolvedProperty))
      {
        blendingKeyframe.SetContainsDirectionAwareProperty(true);
      }
      if (const auto *value = propertyReference.Value())
      {
        auto resolvedProperty =
          css::Property::ResolveDirectionAwareProperty(unresolvedProperty, elementStyle.GetWritingMode());
        if (resolvedProperty != css::PropertyId::AnimationTimingFunction
            && resolvedProperty != css::PropertyId::AnimationComposition)
        {
          if (const auto *customValue = DynamicDowncast<css::CustomPropertyValue>(*value))
          {
            blendingKeyframe.AddProperty(customValue->Name());
          }
          else
          {
            blendingKeyframe.AddProperty(resolvedProperty);
          }
        }
        if (css::IsValueId(*value, css::ValueId::Revert))
        {
          hasRevert = true;
        }
      }
    }

    auto state = State(element, nullptr, context.DocumentElementStyle, context.TreeResolutionState);

    state.SetStyle(RenderStyle::ClonePtr(elementStyle));
    state.SetParentStyle(RenderStyle::ClonePtr(context.ParentStyle ? *context.ParentStyle : elementStyle));

    ElementRuleCollector collector(element, _ruleSets, context.SelectorMatchingState);

    auto pseudoElementIdentifier = elementStyle.GetPseudoElementIdentifier();
    if (pseudoElementIdentifier)
    {
      collector.SetPseudoElementRequest(PseudoElementRequest {*pseudoElementIdentifier});
    }

    if (hasRevert)
    {
      // In the animation origin, 'revert' rolls back the cascaded value to the user level.
      // Therefore, we need to collect UA and user rules.
      collector.SetMedium(_mediaQueryEvaluator);
      collector.MatchUARules();
      collector.MatchUserRules();
    }
    collector.AddAuthorKeyframeRules(keyframe);
    Builder builder(*state.Style(), GetBuilderContext(state), collector.GetMatchResult());
    builder.State().SetIsBuildingKeyframeStyle();
    builder.ApplyAllProperties();

    Adjuster adjuster(Document(), *state.ParentStyle(), nullptr,
                      !pseudoElementIdentifier ? &element : nullptr);
    adjuster.Adjust(*state.Style());

    return state.TakeStyle();
  }

  bool Resolver::IsAnimationNameValid(const css::CSSOMString &name) const noexcept
  {
    auto atom = css::CSSOMStringAtom(name);
    return _keyframesRuleMap.contains(atom) || UserAgentKeyframes().contains(atom);
  }

  List<Ref<css::KeyframeRule>>
    Resolver::KeyframeRulesForName(const css::CSSOMStringAtom &animationName,
                                   const TimingFunction *defaultTimingFunction) const noexcept
  {
    if (animationName.View().empty())
    {
      return {};
    }

    // Check author map first then check user-agent map.
    auto it = _keyframesRuleMap.find(animationName);
    if (it == _keyframesRuleMap.end())
    {
      it = UserAgentKeyframes().find(animationName);
      if (it == UserAgentKeyframes().end())
      {
        return {};
      }
    }

    auto compositeOperationForKeyframe = [](const css::KeyframeRule &keyframe) -> CompositeOperation
    {
      return KeyframeRuleCompositeOperation(keyframe).value_or(Animation::InitialCompositeOperation());
    };

    auto timingFunctionForKeyframe = [&](const css::KeyframeRule &keyframe) -> RefPtr<const TimingFunction>
    {
      if (auto timingFunctionCSSValue =
            keyframe.Properties().GetPropertyCSSValue(css::PropertyId::AnimationTimingFunction))
      {
        if (auto timingFunction = CreateTimingFunctionDeprecated(*timingFunctionCSSValue))
        {
          return timingFunction;
        }
      }
      if (defaultTimingFunction)
      {
        return ShareRefPtr(defaultTimingFunction);
      }
      return ShareRefPtr<const TimingFunction>(&CubicBezierTimingFunction::DefaultTimingFunction());
    };

    List<RefPtr<const TimingFunction>> timingFunctions;
    auto uniqueTimingFunctionForKeyframe =
      [&](const css::KeyframeRule &keyframe) -> RefPtr<const TimingFunction>
    {
      auto timingFunction = timingFunctionForKeyframe(keyframe);
      for (const auto &existingTimingFunction : timingFunctions)
      {
        if (*timingFunction == *existingTimingFunction)
        {
          return existingTimingFunction;
        }
      }
      timingFunctions.push_back(timingFunction);
      return timingFunction;
    };

    const auto &keyframes = it->second->Keyframes();

    struct KeyframeUniqueKey
    {
      css::KeyframeRule::Key Key;
      RefPtr<const TimingFunction> TimingFunction;
      CompositeOperation CompositeOperation;

      KRYS_NODISCARD bool operator==(const KeyframeUniqueKey &other) const noexcept
      {
        return Key == other.Key && TimingFunction.get() == other.TimingFunction.get()
               && CompositeOperation == other.CompositeOperation;
      }
    };

    auto hasDuplicateKeys = [&]() -> bool
    {
      List<KeyframeUniqueKey> uniqueKeyframeKeys;
      for (const auto &keyframe : keyframes)
      {
        auto compositeOperation = compositeOperationForKeyframe(*keyframe);
        auto timingFunction = uniqueTimingFunctionForKeyframe(*keyframe);
        for (auto key : keyframe->Keys())
        {
          KeyframeUniqueKey uniqueKey {key, timingFunction, compositeOperation};
          if (std::ranges::find(uniqueKeyframeKeys, uniqueKey) != uniqueKeyframeKeys.end())
          {
            return true;
          }
          uniqueKeyframeKeys.push_back(krys::move(uniqueKey));
        }
      }
      return false;
    }();

    if (!hasDuplicateKeys)
    {
      return keyframes;
    }

    // Merge keyframes with a similar offset and timing function ensuring that merged keyframes
    // move to the end of the list if the offset is a timeline range.
    List<Ref<css::KeyframeRule>> deduplicatedKeyframes;
    List<Pair<KeyframeUniqueKey, Ref<css::KeyframeRule>>> keyframesMap;
    for (const auto &originalKeyframe : keyframes)
    {
      auto compositeOperation = compositeOperationForKeyframe(*originalKeyframe);
      auto timingFunction = uniqueTimingFunctionForKeyframe(*originalKeyframe);
      for (auto key : originalKeyframe->Keys())
      {
        KeyframeUniqueKey uniqueKey {key, timingFunction, compositeOperation};
        auto existing =
          std::ranges::find_if(keyframesMap, [&](const auto &entry) { return entry.first == uniqueKey; });
        if (existing != keyframesMap.end())
        {
          Ref<css::KeyframeRule> existingStyleRuleKeyframe = existing->second;
          existingStyleRuleKeyframe->MutableProperties().MergeAndOverrideOnConflict(
            originalKeyframe->Properties());
          if (existingStyleRuleKeyframe->Keys()[0].RangeName == css::ValueId::Normal)
          {
            continue;
          }
          std::erase_if(deduplicatedKeyframes, [&](const Ref<css::KeyframeRule> &styleRuleKeyframe)
                        { return &*styleRuleKeyframe == &*existingStyleRuleKeyframe; });
          deduplicatedKeyframes.push_back(existingStyleRuleKeyframe);
        }
        else
        {
          auto styleRuleKeyframe = css::KeyframeRule::Create(css::MutableStyleProperties::Create());
          styleRuleKeyframe->SetKey(key);
          styleRuleKeyframe->MutableProperties().MergeAndOverrideOnConflict(originalKeyframe->Properties());
          keyframesMap.emplace_back(krys::move(uniqueKey), styleRuleKeyframe);
          deduplicatedKeyframes.push_back(styleRuleKeyframe);
        }
      }
    }

    return deduplicatedKeyframes;
  }

  bool Resolver::KeyframeStylesForAnimation(dom::Element &element, const RenderStyle &elementStyle,
                                            const ResolutionContext &context, BlendingKeyframes &list,
                                            const TimingFunction *defaultTimingFunction) const noexcept
  {
    list.Clear();

    auto keyframeRules = KeyframeRulesForName(list.KeyframesName(), defaultTimingFunction);
    if (keyframeRules.empty())
    {
      return false;
    }

    // Construct and populate the style for each keyframe.
    for (const auto &keyframeRule : keyframeRules)
    {
      // Add this keyframe style to all the indicated key times
      for (const auto &key : keyframeRule->Keys())
      {
        BlendingKeyframe blendingKeyframe(
          {ConvertValueIdToSingleAnimationRangeName(key.RangeName), key.Offset}, nullptr);
        blendingKeyframe.SetStyle(
          StyleForKeyframe(element, elementStyle, context, *keyframeRule, blendingKeyframe));
        if (auto timingFunctionCSSValue =
              keyframeRule->Properties().GetPropertyCSSValue(css::PropertyId::AnimationTimingFunction))
        {
          blendingKeyframe.SetTimingFunction(CreateTimingFunctionDeprecated(*timingFunctionCSSValue));
        }
        if (auto compositeOperation = KeyframeRuleCompositeOperation(*keyframeRule))
        {
          blendingKeyframe.SetCompositeOperation(*compositeOperation);
        }
        list.Insert(krys::move(blendingKeyframe));
        list.UpdatePropertiesMetadata(keyframeRule->Properties());
      }
    }

    return true;
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

  Maybe<ResolvedStyle> Resolver::StyleForPseudoElement(dom::Element &element,
                                                       const PseudoElementRequest &pseudoElementRequest,
                                                       const ResolutionContext &context) noexcept
  {
    auto state =
      State(element, context.ParentStyle, context.DocumentElementStyle, context.TreeResolutionState);

    if (state.ParentStyle())
    {
      state.SetStyle(RenderStyle::CreatePtrWithRegisteredInitialValues(
        Document().StyleScope().GetCustomPropertyRegistry()));
      state.Style()->InheritFrom(*state.ParentStyle());
    }
    else
    {
      state.SetStyle(DefaultStyleForElement(Document(), &element));
      state.SetParentStyle(RenderStyle::ClonePtr(*state.Style()));
    }

    ElementRuleCollector collector(element, _ruleSets, context.SelectorMatchingState);
    collector.SetPseudoElementRequest(pseudoElementRequest);
    collector.SetMedium(_mediaQueryEvaluator);
    collector.MatchUARules();

    if (_matchAuthorAndUserStyles)
    {
      collector.MatchUserRules();
      collector.MatchAuthorRules();
    }

    krys_debug_assert(!collector.MatchedPseudoElements());

    if (collector.GetMatchResult().IsEmpty())
    {
      return {};
    }

    state.Style()->SetPseudoElementIdentifier(
      Maybe<PseudoElementIdentifier> {pseudoElementRequest.Identifier()});

    ApplyMatchedProperties(state, collector.GetMatchResult(), PropertyCascade::NormalProperties());

    Adjuster adjuster(Document(), *state.ParentStyle(), context.ParentBoxStyle, nullptr);
    adjuster.Adjust(*state.Style());

    // FIXME: Adjuster::adjustVisibilityForPseudoElement comes with the visibility adjustment of elements.

    return ResolvedStyle {state.TakeStyle(), nullptr, collector.ReleaseMatchResult()};
  }

  UniquePtr<RenderStyle> Resolver::DefaultStyleForElement(const dom::Document &document,
                                                          const dom::Element *element) noexcept
  {
    auto style =
      RenderStyle::CreatePtrWithRegisteredInitialValues(document.StyleScope().GetCustomPropertyRegistry());

    FontCascadeDescription fontDescription;
    fontDescription.SetOneFamily(css::GenericFontFamilyNames::StandardFamily());
    fontDescription.SetKeywordSizeFromIdentifier(css::ValueId::Medium);

    auto size = FontSizeForKeyword(static_cast<uint32>(css::ValueId::Medium), false, document);
    fontDescription.SetSpecifiedSize(size);
    auto computedFontSize =
      ComputedFontSizeFromSpecifiedSize(size, fontDescription.IsAbsoluteSize(),
                                        element && Is<svg::SVGElement>(*element), style.get(), document);
    fontDescription.SetComputedSize(computedFontSize.Size, computedFontSize.UsedZoomFactor);

    // FIXME: The settings' shouldAllowUserInstalledFonts comes with Settings (its default, Yes, is the
    // description's).
    style->SetFontDescription(krys::move(fontDescription));

    return style;
  }

  List<RefPtr<const css::StyleRule>> Resolver::StyleRulesForElement(const dom::Element *element,
                                                                    uint32 rulesToInclude) noexcept
  {
    return PseudoStyleRulesForElement(element, {}, rulesToInclude);
  }

  List<RefPtr<const css::StyleRule>>
    Resolver::PseudoStyleRulesForElement(const dom::Element *element,
                                         const Maybe<PseudoElementRequest> &pseudoElementIdentifier,
                                         uint32 rulesToInclude) noexcept
  {
    if (!element)
    {
      return {};
    }

    ElementRuleCollector collector(*element, _ruleSets, nullptr, css::SelectorChecker::Mode::CollectingRules);
    if (pseudoElementIdentifier)
    {
      collector.SetPseudoElementRequest(*pseudoElementIdentifier);
    }
    collector.SetMedium(_mediaQueryEvaluator);
    collector.SetIncludeEmptyRules(rulesToInclude & EmptyCSSRules);

    if (rulesToInclude & UAAndUserCSSRules)
    {
      // First we match rules from the user agent sheet.
      collector.MatchUARules();

      // Now we check user sheet rules.
      if (_matchAuthorAndUserStyles)
      {
        collector.MatchUserRules();
      }
    }

    if (_matchAuthorAndUserStyles && (rulesToInclude & AuthorCSSRules))
    {
      collector.MatchAuthorRules();
    }

    return collector.MatchedRuleList();
  }

  void Resolver::InvalidateMatchedDeclarationsCache() noexcept
  {
    _matchedDeclarationsCache.Invalidate();
  }

  void Resolver::ClearCachedDeclarationsAffectedByViewportUnits() noexcept
  {
    _matchedDeclarationsCache.ClearEntriesAffectedByViewportUnits();
  }

  void Resolver::ApplyMatchedProperties(State &state, const MatchResult &matchResult,
                                        PropertyCascade::IncludedProperties &&includedProperties) noexcept
  {
    auto &style = *state.Style();
    auto &parentStyle = *state.ParentStyle();
    auto &element = *state.Element();

    auto cacheHash =
      MatchedDeclarationsCache::ComputeHash(matchResult, parentStyle.InheritedCustomProperties());

    auto cacheResult = _matchedDeclarationsCache.Find(cacheHash, matchResult,
                                                      parentStyle.InheritedCustomProperties(), parentStyle);

    auto hasUsableEntry = cacheResult && MatchedDeclarationsCache::IsCacheable(element, style, parentStyle);
    if (hasUsableEntry)
    {
      auto &cacheEntry = cacheResult->Entry;
      bool inheritedEqual = cacheResult->InheritedEqual;
      // We can build up the style by copying non-inherited properties from an earlier style object built
      // using the same exact style declarations. We then only need to apply the inherited properties, if any,
      // as their values can depend on the element context. This is fast and saves memory by reusing the style
      // data structures.
      style.CopyNonInheritedFrom(*cacheEntry.RenderStyle);

      bool hasExplicitlyInherited = cacheEntry.RenderStyle->HasExplicitlyInheritedProperties();
      bool explicitlyInheritedEqual =
        !hasExplicitlyInherited || parentStyle.NonInheritedEqual(*cacheEntry.ParentRenderStyle);

      if (inheritedEqual)
      {
        // If the cache item parent style has identical inherited properties to the current parent style then
        // the resulting style will be identical too. We copy the inherited properties over from the cache and
        // are done.
        style.InheritFrom(*cacheEntry.RenderStyle);
      }

      includedProperties = {};

      if (!inheritedEqual)
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::Inherited);
      }
      if (!explicitlyInheritedEqual)
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::ExplicitlyInherited);
      }
      if (!matchResult.NonCacheablePropertyIds.empty())
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::NonCacheable);
      }

      if (includedProperties.IsEmpty())
      {
        return;
      }
    }

    Builder builder(style, GetBuilderContext(state), matchResult, krys::move(includedProperties));

    // Top priority properties may affect resolution of high priority ones.
    builder.ApplyTopPriorityProperties();

    // High priority properties may affect resolution of other properties (they are mostly font related).
    builder.ApplyHighPriorityProperties();

    if (cacheResult && !cacheResult->Entry.IsUsableAfterHighPriorityProperties(style))
    {
      // High-priority properties may affect resolution of other properties. Kick out the existing cache entry
      // and try again.
      _matchedDeclarationsCache.Remove(cacheHash);
      ApplyMatchedProperties(state, matchResult, PropertyCascade::NormalProperties());
      return;
    }

    builder.ApplyNonHighPriorityProperties();
    builder.AdjustAfterApplying();

    SetGlobalStateAfterApplyingProperties(builder.State());

    if (((cacheHash && !cacheResult) || (cacheResult && !cacheResult->InheritedEqual))
        && MatchedDeclarationsCache::IsCacheable(element, style, parentStyle))
    {
      _matchedDeclarationsCache.Add(style, parentStyle, cacheHash, matchResult);
    }
  }

  void Resolver::SetGlobalStateAfterApplyingProperties(const BuilderState &builderState) noexcept
  {
    // FIXME: This stuff should be somewhere else.
    for (auto &contentAttribute : builderState.RegisteredContentAttributes())
    {
      RuleSets().MutableFeatures().RegisterContentAttribute(contentAttribute);
    }
    if (builderState.Style().UsesViewportUnits())
    {
      Document().SetHasStyleWithViewportUnits();
    }
  }

}
