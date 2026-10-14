# Port graph

Entries: `style/RuleFeature.cpp`

- Files to port: **41**
- Boundary `cssjit`: 2
- Boundary `dom`: 2
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 6
- Boundary `rendering`: 1
- Boundary `wtf`: 39
- Unresolved: 4

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 11 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 11 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 10 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 10 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 9 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 9 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 8 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 8 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 8 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 7 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 7 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h, css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 4 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSPrimitiveValue.h, css/query/GenericMediaQueryTypes.h, css/values/CSSValueTypes.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/SelectorFilter.h` |  | style/RuleData.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h |
| 3 | `css/query/GenericMediaQueryTypes.h` |  | css/query/ContainerQuery.h, css/query/MediaQuery.h |
| 3 | `style/PropertyAllowlist.h` |  | style/RuleData.h |
| 2 | `css/CSSVariableData.h` |  | css/StyleRule.h |
| 2 | `css/StyleRuleType.h` |  | css/StyleRule.h, css/parser/CSSParserContext.h |
| 2 | `css/query/ContainerQuery.h` |  | css/StyleRule.h |
| 2 | `css/query/MediaQuery.h` |  | css/StyleRule.h, style/RuleSet.h |
| 2 | `style/RuleData.h` |  | style/RuleSet.h |
| 1 | `css/CSSSelector.h` |  | css/CSSSelectorList.h, css/StyleRule.h, style/RuleFeature.cpp |
| 1 | `css/CSSSelectorList.h` |  | css/StyleRule.h, style/RuleFeature.cpp, style/RuleFeature.h, style/RuleSet.h |
| 1 | `css/StyleRule.h` |  | style/RuleData.h, style/RuleFeature.cpp, style/RuleSet.h |
| 1 | `style/RuleFeature.h` |  | style/RuleFeature.cpp, style/RuleSet.h |
| 1 | `style/RuleSet.h` |  | style/RuleFeature.cpp |
| 0 | `style/RuleFeature.cpp` |  | (entry) |

## Boundaries

### cssjit (2)

- `cssjit/CompiledSelector.h` ← 1 file(s)
- `cssjit/SelectorCompiler.h` ← 1 file(s)

### dom (2)

- `dom/Element.h` ← 2 file(s)
- `dom/QualifiedName.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (6)

- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/FontFeatureValues.h` ← 1 file(s)
- `platform/graphics/FontPaletteValues.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (39)

- `wtf/Assertions.h` ← 1 file(s)
- `wtf/BloomFilter.h` ← 1 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CompactRefPtrTuple.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 5 file(s)
- `wtf/FixedVector.h` ← 3 file(s)
- `wtf/Forward.h` ← 8 file(s)
- `wtf/GenericHashKey.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 2 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 2 file(s)
- `wtf/Ref.h` ← 3 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 2 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/TZoneMalloc.h` ← 3 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 5 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 6 file(s)
- `wtf/text/AtomStringHash.h` ← 2 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (4)

- `CSSPropertyNames.h` ← 5 file(s)
- `CSSSelectorEnums.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 4 file(s)
- `HTMLNames.h` ← 1 file(s)
