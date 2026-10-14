# Port graph

Entries: `style/UserAgentStyle.cpp`

- Files to port: **105**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 7
- Boundary `external`: 1
- Boundary `html`: 14
- Boundary `loader`: 1
- Boundary `mathml`: 1
- Boundary `page`: 4
- Boundary `platform`: 14
- Boundary `rendering`: 4
- Boundary `svg`: 1
- Boundary `wtf`: 54
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 7 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 7 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 7 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 6 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 6 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 6 | `css/values/CSSNoConversionDataRequiredToken.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, style/values/StyleValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSSymbol.h` |  | css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 6 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 5 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h` |  | css/values/color/CSSColorDescriptors.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h, css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h, css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 5 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h, style/values/primitives/StylePrimitiveNumeric.h |
| 5 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | css/values/color/CSSColorDescriptors.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/color/CSSColor.h, css/values/color/CSSHexColor.h, css/values/color/CSSKeywordColor.h, css/values/color/CSSResolvedColor.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSSymbol.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 4 | `css/values/color/CSSColorDescriptors.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSHexColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSKeywordColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/color/CSSResolvedColor.h` |  | css/values/color/CSSColor.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 4 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h |
| 4 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/CSSCounterStyleDescriptors.h` |  | css/CSSCounterStyle.h |
| 3 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSColorValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 3 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 3 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h |
| 3 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `css/values/color/CSSColor.h` |  | css/CSSColorValue.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `style/AnchorPositionEvaluator.h` |  | style/TreeResolutionState.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h |
| 2 | `css/CSSColorValue.h` |  | css/CSSValuePool.h |
| 2 | `css/CSSCounterStyle.h` |  | css/CSSCounterStyleRegistry.h, css/CSSCounterStyleRule.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSValuePool.h, css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 2 | `css/CSSRule.h` |  | css/CSSCounterStyleRule.h, css/CSSKeyframesRule.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 2 | `css/StyleProperties.h` |  | css/CSSCounterStyleRule.h, css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 2 | `css/StyleRule.h` |  | css/CSSCounterStyleRule.h, css/CSSKeyframesRule.h, style/RuleData.h, style/RuleSet.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/StyleSheetContents.h |
| 2 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h, style/RuleSetBuilder.h |
| 2 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h, style/RuleSetBuilder.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleSetBuilder.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 1 | `css/CSSCounterStyleRegistry.h` |  | style/UserAgentStyle.cpp |
| 1 | `css/CSSCounterStyleRule.h` |  | style/UserAgentStyle.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | style/UserAgentStyle.cpp |
| 1 | `css/CSSValuePool.h` |  | style/UserAgentStyle.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/UserAgentStyle.cpp |
| 1 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h, style/UserAgentStyle.cpp |
| 1 | `style/RuleSetBuilder.h` |  | style/UserAgentStyle.cpp |
| 1 | `style/StyleResolver.h` |  | style/UserAgentStyle.cpp |
| 1 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h, style/UserAgentStyle.cpp |
| 0 | `style/UserAgentStyle.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (7)

- `dom/Document.h` ← 1 file(s)
- `dom/DocumentFullscreen.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (14)

- `html/HTMLAnchorElement.h` ← 1 file(s)
- `html/HTMLAttachmentElement.h` ← 1 file(s)
- `html/HTMLBRElement.h` ← 1 file(s)
- `html/HTMLBodyElement.h` ← 1 file(s)
- `html/HTMLDataListElement.h` ← 1 file(s)
- `html/HTMLDivElement.h` ← 1 file(s)
- `html/HTMLEmbedElement.h` ← 1 file(s)
- `html/HTMLHeadElement.h` ← 1 file(s)
- `html/HTMLHtmlElement.h` ← 1 file(s)
- `html/HTMLInputElement.h` ← 1 file(s)
- `html/HTMLMeterElement.h` ← 1 file(s)
- `html/HTMLObjectElement.h` ← 1 file(s)
- `html/HTMLProgressElement.h` ← 1 file(s)
- `html/HTMLSpanElement.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### mathml (1)

- `mathml/MathMLElement.h` ← 1 file(s)

### page (4)

- `page/Chrome.h` ← 1 file(s)
- `page/ChromeClient.h` ← 1 file(s)
- `page/Page.h` ← 1 file(s)
- `page/Quirks.h` ← 1 file(s)

### platform (14)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 3 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/ColorTypes.h` ← 2 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 3 file(s)

### rendering (4)

- `rendering/RenderTheme.h` ← 1 file(s)
- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 4 file(s)

### svg (1)

- `svg/SVGElement.h` ← 1 file(s)

### wtf (54)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 6 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 20 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 12 file(s)
- `wtf/HashSet.h` ← 2 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 5 file(s)
- `wtf/Ref.h` ← 5 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 7 file(s)
- `wtf/StdLibExtras.h` ← 6 file(s)
- `wtf/TZoneMalloc.h` ← 8 file(s)
- `wtf/TypeCasts.h` ← 3 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 10 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 3 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 3 file(s)
- `wtf/text/AtomString.h` ← 14 file(s)
- `wtf/text/AtomStringHash.h` ← 8 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (4)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 9 file(s)
- `UserAgentStyleSheets.h` ← 1 file(s)
