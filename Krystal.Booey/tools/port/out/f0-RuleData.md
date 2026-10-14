# Port graph

Entries: `style/RuleData.cpp`

- Files to port: **85**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 6
- Boundary `external`: 1
- Boundary `loader`: 5
- Boundary `page`: 1
- Boundary `platform`: 16
- Boundary `rendering`: 3
- Boundary `wtf`: 52
- Unresolved: 7

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 9 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 8 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 6 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 6 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 5 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 5 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h |
| 5 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 3 | `css/parser/CSSParserEnum.h` |  | css/CSSRule.h |
| 3 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 3 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/AnchorPositionEvaluator.h` |  | style/TreeResolutionState.h |
| 3 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 3 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 3 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 2 | `css/CSSFontFace.h` |  | css/CSSFontFaceSet.h, css/CSSFontSelector.h |
| 2 | `css/CSSFontFaceSet.h` |  | css/CSSFontSelector.h |
| 2 | `css/CSSRule.h` |  | css/CSSKeyframesRule.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 2 | `css/StyleRuleType.h` |  | css/CSSRule.h, css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h, css/StyleSheetContents.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/StyleRuleImport.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 2 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 2 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 2 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 2 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 2 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 2 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 2 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h |
| 2 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 2 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleResolver.h |
| 2 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 2 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 1 | `css/CSSFontSelector.h` |  | style/RuleData.cpp |
| 1 | `css/CSSKeyframesRule.h` |  | style/RuleData.cpp |
| 1 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/RuleData.cpp, style/StyleResolver.h |
| 1 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleData.cpp, style/RuleFeature.h, style/RuleSet.h |
| 1 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.cpp |
| 1 | `css/SelectorFilter.h` |  | style/RuleData.cpp, style/RuleData.h, style/SelectorMatchingState.h |
| 1 | `css/StyleRule.h` |  | css/CSSFontSelector.h, css/CSSKeyframesRule.h, css/StyleRuleImport.h, style/RuleData.cpp, style/RuleData.h, style/RuleSet.h |
| 1 | `css/StyleRuleImport.h` |  | style/RuleData.cpp |
| 1 | `css/StyleSheetContents.h` |  | style/RuleData.cpp |
| 1 | `style/RuleData.h` |  | style/RuleData.cpp, style/RuleSet.h |
| 1 | `style/StyleResolver.h` |  | style/RuleData.cpp |
| 0 | `style/RuleData.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (6)

- `dom/ActiveDOMObject.h` ← 1 file(s)
- `dom/Element.h` ← 3 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ScriptExecutionContext.h` ← 2 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (5)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/cache/CachedCSSStyleSheet.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 2 file(s)
- `loader/cache/CachedStyleSheetClient.h` ← 1 file(s)
- `loader/cache/TrustedFonts.h` ← 1 file(s)

### page (1)

- `page/SecurityOrigin.h` ← 1 file(s)

### platform (16)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/graphics/Font.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/graphics/FontSelectionAlgorithm.h` ← 1 file(s)
- `platform/graphics/FontSelector.h` ← 1 file(s)
- `platform/graphics/FontTaggedSettings.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 1 file(s)
- `platform/graphics/LayoutSize.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 2 file(s)

### rendering (3)

- `rendering/style/PositionTryOrder.h` ← 1 file(s)
- `rendering/style/RenderStyle.h` ← 2 file(s)
- `rendering/style/RenderStyleConstants.h` ← 5 file(s)

### wtf (52)

- `wtf/AbstractRefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
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
- `wtf/HashSet.h` ← 4 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Observer.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 3 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 8 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 8 file(s)
- `wtf/TypeCasts.h` ← 4 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 11 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/WeakHashSet.h` ← 3 file(s)
- `wtf/WeakPtr.h` ← 3 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 11 file(s)
- `wtf/text/AtomStringHash.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 3 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (7)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
- `HTMLNames.h` ← 1 file(s)
- `Settings.h` ← 1 file(s)
- `UserAgentParts.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
