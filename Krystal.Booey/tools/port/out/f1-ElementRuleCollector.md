# Port graph

Entries: `style/ElementRuleCollector.cpp`

- Files to port: **91**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 17
- Boundary `editing`: 1
- Boundary `external`: 1
- Boundary `html`: 12
- Boundary `inspector`: 1
- Boundary `loader`: 4
- Boundary `page`: 4
- Boundary `platform`: 12
- Boundary `rendering`: 3
- Boundary `svg`: 1
- Boundary `wtf`: 58
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 8 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 7 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 6 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 5 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 4 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 3 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 3 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 3 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 2 | `css/CSSRule.h` |  | css/CSSKeyframeRule.h |
| 2 | `css/MediaList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/SelectorChecker.h` |  | css/SelectorCheckerTestFunctions.h, style/ElementRuleCollector.h, style/MatchResult.h |
| 2 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 2 | `css/StyleRule.h` |  | css/CSSKeyframeRule.h, css/StyleRuleImport.h, style/RuleData.h, style/RuleSet.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/StyleRuleImport.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 1 | `css/CSSKeyframeRule.h` |  | style/ElementRuleCollector.cpp |
| 1 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h, style/ElementRuleCollector.cpp |
| 1 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/ElementRuleCollector.cpp, style/HasSelectorFilter.h, style/StyleResolver.h |
| 1 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/ElementRuleCollector.cpp, style/RuleFeature.h, style/RuleSet.h |
| 1 | `css/CSSStyleSheet.h` |  | style/ElementRuleCollector.cpp |
| 1 | `css/SelectorCheckerTestFunctions.h` |  | style/ElementRuleCollector.cpp |
| 1 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, style/ElementRuleCollector.cpp |
| 1 | `css/StyleRuleImport.h` |  | style/ElementRuleCollector.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/ElementRuleCollector.cpp |
| 1 | `style/ContainerQueryEvaluator.h` |  | style/ElementRuleCollector.cpp, style/SelectorMatchingState.h |
| 1 | `style/DeclarationOrigin.h` |  | style/ElementRuleCollector.cpp |
| 1 | `style/ElementRuleCollector.h` |  | style/ElementRuleCollector.cpp, style/StyleResolver.h |
| 1 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h, style/ElementRuleCollector.cpp |
| 1 | `style/StyleResolver.h` |  | style/ElementRuleCollector.cpp |
| 1 | `style/StyleScope.h` |  | style/ElementRuleCollector.cpp |
| 1 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.cpp, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 1 | `style/StyleScopeRuleSets.h` |  | style/ElementRuleCollector.cpp, style/StyleResolver.h |
| 1 | `style/UserAgentStyle.h` |  | style/ElementRuleCollector.cpp, style/StyleScopeRuleSets.h |
| 0 | `style/ElementRuleCollector.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 2 file(s)

### dom (17)

- `dom/Document.h` ← 1 file(s)
- `dom/DocumentFullscreen.h` ← 1 file(s)
- `dom/DocumentImmersive.h` ← 1 file(s)
- `dom/DocumentOrShadowRootFullscreen.h` ← 1 file(s)
- `dom/DocumentPage.h` ← 1 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/ElementRareData.h` ← 1 file(s)
- `dom/ElementTextDirection.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 2 file(s)
- `dom/StyledElement.h` ← 1 file(s)
- `dom/ViewTransition.h` ← 1 file(s)

### editing (1)

- `editing/FrameSelection.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (12)

- `html/HTMLAttachmentElement.h` ← 1 file(s)
- `html/HTMLDialogElement.h` ← 1 file(s)
- `html/HTMLElement.h` ← 1 file(s)
- `html/HTMLFrameElement.h` ← 1 file(s)
- `html/HTMLIFrameElement.h` ← 1 file(s)
- `html/HTMLImageElement.h` ← 1 file(s)
- `html/HTMLInputElement.h` ← 1 file(s)
- `html/HTMLMediaElement.h` ← 1 file(s)
- `html/HTMLOptionElement.h` ← 1 file(s)
- `html/HTMLSlotElement.h` ← 1 file(s)
- `html/HTMLVideoElement.h` ← 1 file(s)
- `html/track/WebVTTElement.h` ← 1 file(s)

### inspector (1)

- `inspector/InspectorInstrumentation.h` ← 1 file(s)

### loader (4)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)

### page (4)

- `page/FocusController.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/LocalFrameInlines.h` ← 1 file(s)
- `page/Page.h` ← 1 file(s)

### platform (12)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutRect.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 2 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (3)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)

### svg (1)

- `svg/SVGElement.h` ← 1 file(s)

### wtf (58)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 4 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 14 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 11 file(s)
- `wtf/HashSet.h` ← 3 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/Identified.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 3 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 4 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 3 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 9 file(s)
- `wtf/SetForScope.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 9 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 5 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 13 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 3 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 4 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 8 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 4 file(s)

## Problems

### unresolved (4)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 9 file(s)
- `Settings.h` ← 1 file(s)
