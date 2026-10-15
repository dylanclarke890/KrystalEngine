# Port graph

Entries: `style/MatchResultCache.cpp`

- Files to port: **67**
- Boundary `animation`: 1
- Boundary `cssjit`: 2
- Boundary `dom`: 4
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 8
- Boundary `wtf`: 41
- Unresolved: 6

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 9 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 9 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 8 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 8 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 7 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 7 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 7 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 6 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 6 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 6 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 6 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 6 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 6 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryEvaluator.h, css/query/GenericMediaQueryTypes.h |
| 5 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 5 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 5 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 5 | `css/query/GenericMediaQueryEvaluator.h` |  | style/ContainerQueryEvaluator.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `style/StyleUpdate.h` |  | style/ContainerQueryEvaluator.h |
| 4 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 4 | `css/SelectorFilter.h` |  | style/RuleData.h, style/SelectorMatchingState.h |
| 4 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 4 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 4 | `css/query/ContainerQuery.h` |  | css/StyleRule.h, style/ContainerQueryEvaluator.h, style/SelectorMatchingState.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/GenericMediaQueryEvaluator.h, css/query/MediaQuery.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `style/ContainerQueryEvaluator.h` |  | style/SelectorMatchingState.h |
| 4 | `style/HasSelectorFilter.h` |  | style/SelectorMatchingState.h |
| 3 | `css/CSSPrimitiveValue.h` |  | css/StylePropertiesInlines.h, css/parser/CSSParserToken.h |
| 3 | `css/CSSPropertyInitialValues.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/SelectorChecker.h, css/StyleRule.h, style/HasSelectorFilter.h |
| 3 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.h, style/RuleSet.h |
| 3 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/CSSProperty.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 3 | `css/ImmutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/IsImportant.h` |  | css/CSSProperty.h |
| 3 | `css/MutableStyleProperties.h` |  | css/StylePropertiesInlines.h |
| 3 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleSet.h |
| 3 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 3 | `rendering/style/RenderStyleConstants.h` |  | css/CSSSelector.h, style/PropertyAllowlist.h, style/PseudoElementIdentifier.h |
| 3 | `style/PseudoElementIdentifier.h` |  | css/SelectorChecker.h |
| 3 | `style/RuleData.h` |  | style/RuleSet.h |
| 3 | `style/RuleFeature.h` |  | style/RuleSet.h |
| 3 | `style/SelectorMatchingState.h` |  | css/SelectorChecker.h |
| 3 | `style/StyleScrollbarState.h` |  | css/SelectorChecker.h |
| 2 | `css/CSSProperty.h` |  | css/StyleProperties.h |
| 2 | `css/SelectorChecker.h` |  | style/MatchResult.h |
| 2 | `css/StylePropertiesInlines.h` |  | style/MatchResult.h |
| 2 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/style/RenderStyle+SettersInlines.h |
| 2 | `rendering/style/RenderStyle.h` |  | rendering/style/RenderStyle+GettersInlines.h, style/ResolvedStyle.h |
| 2 | `style/PropertyAllowlist.h` |  | style/MatchResult.h, style/RuleData.h |
| 2 | `style/PropertyCascade.h` |  | style/MatchResultCache.h |
| 2 | `style/RuleSet.h` |  | style/MatchResult.h |
| 2 | `style/StyleRelations.h` |  | css/SelectorChecker.h, style/ResolvedStyle.h |
| 2 | `style/StyleScopeOrdinal.h` |  | css/SelectorChecker.h, style/ContainerQueryEvaluator.h, style/MatchResult.h |
| 1 | `css/StyleProperties.h` |  | css/ImmutableStyleProperties.h, css/MutableStyleProperties.h, style/MatchResultCache.cpp |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | style/MatchResultCache.cpp |
| 1 | `style/MatchResult.h` |  | style/MatchResultCache.cpp, style/PropertyCascade.h, style/ResolvedStyle.h |
| 1 | `style/MatchResultCache.h` |  | style/MatchResultCache.cpp |
| 1 | `style/ResolvedStyle.h` |  | style/MatchResultCache.cpp, style/MatchResultCache.h |
| 0 | `style/MatchResultCache.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/WebAnimationTypes.h` ← 1 file(s)

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (4)

- `dom/Element.h` ← 3 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/StyledElement.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (8)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 1 file(s)

### wtf (41)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BitSet.h` ← 3 file(s)
- `wtf/BloomFilter.h` ← 2 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 2 file(s)
- `wtf/EnumTraits.h` ← 7 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 10 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 4 file(s)
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
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/TZoneMalloc.h` ← 6 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 5 file(s)
- `wtf/WeakHashMap.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 7 file(s)
- `wtf/text/AtomStringHash.h` ← 3 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 4 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (6)

- `CSSPropertyNames.h` ← 6 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 6 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
