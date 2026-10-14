# Port graph

Entries: `css/CSSCustomPropertyValue.cpp`

- Files to port: **33**
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 3
- Boundary `wtf`: 32
- Unresolved: 2

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
| 7 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 5 | `css/CSSPrimitiveValue.h` |  | css/parser/CSSParserToken.h |
| 5 | `css/parser/CSSTokenizerInputStream.h` |  | css/parser/CSSTokenizer.h |
| 4 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 4 | `css/parser/CSSParserToken.h` |  | css/parser/CSSParserTokenRange.h, css/parser/CSSTokenizer.h |
| 4 | `css/parser/CSSTokenizer.h` |  | css/parser/CSSParserTokenRange.h |
| 3 | `css/parser/CSSParserContext.h` |  | css/CSSVariableData.h |
| 3 | `css/parser/CSSParserTokenRange.h` |  | css/CSSVariableData.h, css/CSSVariableReferenceValue.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSCustomPropertyValue.h, css/CSSPrimitiveValue.h, css/CSSVariableReferenceValue.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSVariableData.h` |  | css/CSSCustomPropertyValue.h, css/CSSVariableReferenceValue.h |
| 2 | `css/CSSVariableReferenceValue.h` |  | css/CSSCustomPropertyValue.h |
| 2 | `css/CSSWideKeyword.h` |  | css/CSSCustomPropertyValue.h |
| 1 | `css/CSSCustomPropertyValue.h` |  | css/CSSCustomPropertyValue.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/CSSCustomPropertyValue.cpp |
| 0 | `css/CSSCustomPropertyValue.cpp` |  | (entry) |

## Boundaries

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (3)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (32)

- `wtf/Brigand.h` ← 4 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 5 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 2 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 5 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 4 file(s)
- `CSSValueKeywords.h` ← 5 file(s)
