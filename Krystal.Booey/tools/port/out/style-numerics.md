# Port graph

Entries: `style/values/primitives/StylePrimitiveNumericTypes.h`, `style/values/primitives/StyleLengthWrapper.h`, `style/values/primitives/StyleLengthWrapperData.cpp`, `style/calc/StyleCalculationValueMap.cpp`

- Files to port: **35**
- Boundary `platform`: 3
- Boundary `wtf`: 27
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `css/CSSUnits.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/CSSValue.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/calc/CSSCalcOperator.h` |  | style/calc/StyleCalculationTree.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/StyleValueTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h |
| 3 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 2 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h, style/values/primitives/StylePrimitiveNumericConcepts.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h, style/values/primitives/StylePrimitiveNumericOrKeyword.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h, style/calc/StyleCalculationValue.h, style/values/primitives/StylePrimitiveNumeric+Forward.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 2 | `style/calc/StyleCalculationTree.h` |  | style/calc/StyleCalculationValue.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/primitives/StylePrimitiveNumeric+Forward.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/primitives/StylePrimitiveNumericConcepts.h` |  | style/values/primitives/StylePrimitiveNumeric.h, style/values/primitives/StyleUnevaluatedCalculation.h |
| 2 | `style/values/primitives/StyleUnevaluatedCalculation.h` |  | style/values/primitives/StylePrimitiveNumeric.h |
| 2 | `style/values/viewport/StyleZoomPrimitives.h` |  | style/values/primitives/StyleLengthWrapperData.h, style/values/primitives/StylePrimitiveNumeric.h |
| 1 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h, style/values/primitives/StyleLengthWrapper.h |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 1 | `style/calc/StyleCalculationValue.h` |  | style/calc/StyleCalculationValueMap.h, style/values/primitives/StyleLengthWrapperData.cpp, style/values/primitives/StyleLengthWrapperData.h |
| 1 | `style/calc/StyleCalculationValueMap.h` |  | style/calc/StyleCalculationValueMap.cpp, style/values/primitives/StyleLengthWrapperData.cpp |
| 1 | `style/values/primitives/StyleLengthWrapperData.h` |  | style/values/primitives/StyleLengthWrapper.h, style/values/primitives/StyleLengthWrapperData.cpp |
| 1 | `style/values/primitives/StylePrimitiveNumeric.h` |  | style/values/primitives/StylePrimitiveNumericOrKeyword.h, style/values/primitives/StylePrimitiveNumericTypes.h |
| 1 | `style/values/primitives/StylePrimitiveNumericOrKeyword.h` |  | style/values/primitives/StylePrimitiveNumericTypes.h |
| 0 | `style/calc/StyleCalculationValueMap.cpp` |  | (entry) |
| 0 | `style/values/primitives/StyleLengthWrapper.h` |  | (entry) |
| 0 | `style/values/primitives/StyleLengthWrapperData.cpp` |  | (entry) |
| 0 | `style/values/primitives/StylePrimitiveNumericTypes.h` |  | (entry) |

## Boundaries

### platform (3)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (27)

- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CompactVariant.h` ← 2 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/FlatteningVariantAdaptor.h` ← 1 file(s)
- `wtf/Forward.h` ← 6 file(s)
- `wtf/HashMap.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 2 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 3 file(s)
