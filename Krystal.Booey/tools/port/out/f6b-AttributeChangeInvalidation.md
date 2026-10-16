# Port graph

Entries: `style/AttributeChangeInvalidation.cpp`

- Files to port: **82**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 11
- Boundary `external`: 1
- Boundary `html`: 1
- Boundary `loader`: 1
- Boundary `platform`: 12
- Boundary `rendering`: 3
- Boundary `wtf`: 50
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 11 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 10 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 10 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 10 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 9 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 9 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 8 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 8 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 8 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 8 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 8 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 8 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 8 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 7 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 7 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h |
| 7 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 7 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 7 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 7 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 7 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 7 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 6 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 6 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 6 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 6 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 6 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 6 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 6 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 6 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 5 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 5 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 5 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 5 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 5 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 5 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 5 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 5 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 5 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h, style/PseudoElementRequest.h |
| 5 | `style/values/StyleValueTypes.h` |  | style/ScopedName.h |
| 4 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 4 | `css/SelectorChecker.h` |  | style/ElementRuleCollector.h, style/MatchResult.h |
| 4 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 4 | `css/query/GenericMediaQueryEvaluator.h` |  | css/query/MediaQueryEvaluator.h, style/ContainerQueryEvaluator.h |
| 4 | `css/query/MediaQuery.h` |  | css/StyleRule.h, css/query/MediaQueryEvaluator.h, style/RuleSet.h |
| 4 | `style/MatchResult.h` |  | style/ElementRuleCollector.h, style/MatchedDeclarationsCache.h, style/PropertyCascade.h |
| 4 | `style/PropertyAllowlist.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/RuleData.h |
| 4 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h, style/AnchorPositionEvaluator.h, style/PseudoElementRequest.h, style/Styleable.h |
| 4 | `style/PseudoElementRequest.h` |  | style/ElementRuleCollector.h |
| 4 | `style/ResolvedScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 4 | `style/RuleData.h` |  | style/RuleSet.h |
| 4 | `style/ScopedName.h` |  | style/AnchorPositionEvaluator.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h, style/StyleResolver.h |
| 3 | `css/query/MediaQueryEvaluator.h` |  | style/ElementRuleCollector.h, style/StyleResolver.h |
| 3 | `style/AnchorPositionEvaluator.h` |  | style/StyleScope.h, style/TreeResolutionState.h |
| 3 | `style/ElementRuleCollector.h` |  | style/StyleResolver.h |
| 3 | `style/InspectorCSSOMWrappers.h` |  | style/StyleResolver.h |
| 3 | `style/MatchedDeclarationsCache.h` |  | style/StyleResolver.h |
| 3 | `style/PropertyCascade.h` |  | style/StyleResolver.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleInvalidator.h, style/StyleScopeRuleSets.h |
| 3 | `style/RuleSet.h` |  | style/ElementRuleCollector.h, style/MatchResult.h, style/StyleInvalidator.h, style/StyleResolver.h, style/StyleScopeRuleSets.h |
| 3 | `style/StyleScopeIdentifier.h` |  | style/ResolvedScopedName.h, style/StyleScope.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/ElementRuleCollector.h, style/MatchResult.h, style/ScopedName.h, style/StyleScope.h |
| 3 | `style/Styleable.h` |  | style/StyleScope.h |
| 3 | `style/TreeResolutionState.h` |  | style/StyleResolver.h |
| 3 | `style/UserAgentStyle.h` |  | style/StyleScopeRuleSets.h |
| 2 | `style/StyleInvalidator.h` |  | style/AttributeChangeInvalidation.h |
| 2 | `style/StyleResolver.h` |  | style/StyleInvalidationFunctions.h |
| 2 | `style/StyleScope.h` |  | style/StyleInvalidationFunctions.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/StyleInvalidationFunctions.h, style/StyleResolver.h |
| 1 | `style/AttributeChangeInvalidation.h` |  | style/AttributeChangeInvalidation.cpp |
| 1 | `style/StyleInvalidationFunctions.h` |  | style/AttributeChangeInvalidation.cpp |
| 0 | `style/AttributeChangeInvalidation.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 2 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (11)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 5 file(s)
- `dom/ElementIterator.h` ← 1 file(s)
- `dom/ElementRareData.h` ← 1 file(s)
- `dom/EventTarget.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/NodeInlines.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### html (1)

- `html/HTMLSlotElement.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (12)

- `platform/CommonAtomStrings.h` ← 1 file(s)
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

### wtf (50)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 2 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CanMakeWeakPtr.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 13 file(s)
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
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 8 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 9 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 10 file(s)
- `wtf/WeakHashMap.h` ← 2 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakListHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 10 file(s)
- `wtf/text/AtomStringHash.h` ← 6 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 4 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 8 file(s)
