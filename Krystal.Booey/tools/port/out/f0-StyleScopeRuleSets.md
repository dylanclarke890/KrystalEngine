# Port graph

Entries: `style/StyleScopeRuleSets.cpp`

- Files to port: **91**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 9
- Boundary `external`: 2
- Boundary `loader`: 3
- Boundary `page`: 2
- Boundary `platform`: 12
- Boundary `rendering`: 3
- Boundary `wtf`: 56
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 9 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 4 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 3 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h |
| 3 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 2 | `css/CSSRule.h` |  | css/CSSViewTransitionRule.h |
| 2 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/MediaList.h` |  | css/CSSStyleSheet.h |
| 2 | `css/StyleProperties.h` |  | css/CSSViewTransitionRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 2 | `css/StyleRule.h` |  | css/CSSViewTransitionRule.h, style/RuleData.h, style/RuleSet.h |
| 2 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/MediaQueryParserContext.h, css/StyleSheetContents.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, style/RuleSetBuilder.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h, css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h, style/RuleSetBuilder.h |
| 2 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleSetBuilder.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 2 | `style/Styleable.h` |  | style/StyleScope.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 2 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 1 | `css/CSSStyleSheet.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `css/CSSViewTransitionRule.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `css/parser/CSSPropertyParser.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h, style/StyleScopeRuleSets.cpp |
| 1 | `style/DeclarationOrigin.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `style/RuleSetBuilder.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `style/StyleResolver.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `style/StyleScope.h` |  | style/StyleScopeRuleSets.cpp |
| 1 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h, style/StyleScopeRuleSets.cpp |
| 0 | `style/StyleScopeRuleSets.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (9)

- `dom/Document.h` ← 1 file(s)
- `dom/DocumentInlines.h` ← 1 file(s)
- `dom/Element.h` ← 4 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/ExtensionStyleSheets.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (2)

- `JavaScriptCore/ConsoleTypes.h` ← 1 file(s)
- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (3)

- `loader/FrameLoader.h` ← 1 file(s)
- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/LocalFrameLoaderClient.h` ← 1 file(s)

### page (2)

- `page/LocalFrame.h` ← 1 file(s)
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

### wtf (56)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 4 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 15 file(s)
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
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 9 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
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
- `wtf/text/AtomString.h` ← 11 file(s)
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
- `CSSValueKeywords.h` ← 8 file(s)
- `HTMLNames.h` ← 1 file(s)
