# Port graph

Entries: `style/PageRuleCollector.cpp`

- Files to port: **63**
- Boundary `cssjit`: 2
- Boundary `dom`: 3
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 8
- Boundary `rendering`: 1
- Boundary `wtf`: 40
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 10 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 9 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 9 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 8 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 7 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 7 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 6 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 6 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 6 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 5 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 5 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 5 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 4 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 4 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 4 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 4 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h |
| 4 | `style/RuleData.h` |  | style/RuleSet.h |
| 4 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleRelations.h` |  | css/SelectorChecker.h |
| 4 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/SelectorChecker.h` |  | style/MatchResult.h |
| 3 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 3 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h, style/StyleScopeRuleSets.h |
| 3 | `style/RuleSet.h` |  | style/MatchResult.h, style/StyleScopeRuleSets.h |
| 3 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 2 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 2 | `style/MatchResult.h` |  | style/PageRuleCollector.h |
| 2 | `style/StyleScopeRuleSets.h` |  | style/PageRuleCollector.h |
| 1 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, style/PageRuleCollector.cpp |
| 1 | `css/StyleRule.h` |  | style/PageRuleCollector.cpp, style/RuleData.h, style/RuleSet.h |
| 1 | `style/PageRuleCollector.h` |  | style/PageRuleCollector.cpp |
| 1 | `style/UserAgentStyle.h` |  | style/PageRuleCollector.cpp, style/StyleScopeRuleSets.h |
| 0 | `style/PageRuleCollector.cpp` |  | (entry) |

## Boundaries

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (3)

- `dom/Element.h` ← 3 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (8)

- `platform/CommonAtomStrings.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 3 file(s)

### wtf (40)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 6 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 11 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 5 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 3 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 4 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/TZoneMalloc.h` ← 5 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 6 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 7 file(s)
- `wtf/text/AtomStringHash.h` ← 3 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 6 file(s)
