# Port graph

Entries: `css/CSSFontStyleRangeValue.cpp`

- Files to port: **23**
- Boundary `external`: 1
- Boundary `platform`: 3
- Boundary `wtf`: 26
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 8 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 7 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 7 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 6 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 6 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 5 | `css/calc/CSSCalcType.h` |  | css/calc/CSSCalcTree.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/calc/CSSCalcTree.h |
| 4 | `css/CSSUnits.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/calc/CSSCalcTree.h` |  | css/calc/CSSCalcValue.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/calc/CSSCalcTree.h, css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 3 | `css/CSSAttrValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/calc/CSSCalcValue.h` |  | css/CSSPrimitiveValue.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/CSSPrimitiveValue.h, css/values/primitives/CSSPrimitiveNumericRaw.h |
| 2 | `css/CSSPrimitiveValue.h` |  | css/CSSFontStyleRangeValue.h |
| 2 | `css/CSSValue.h` |  | css/CSSAttrValue.h, css/CSSFontStyleRangeValue.h, css/CSSPrimitiveValue.h, css/CSSValueList.h, css/values/CSSValueTypes.h |
| 2 | `css/CSSValueList.h` |  | css/CSSFontStyleRangeValue.h |
| 1 | `css/CSSFontStyleRangeValue.h` |  | css/CSSFontStyleRangeValue.cpp |
| 0 | `css/CSSFontStyleRangeValue.cpp` |  | (entry) |

## Boundaries

### external (1)

- `unicode/umachine.h` ← 1 file(s)

### platform (3)

- `platform/LayoutUnit.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (26)

- `wtf/Brigand.h` ← 4 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 4 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
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
- `wtf/Vector.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 2 file(s)
- `CSSValueKeywords.h` ← 3 file(s)
