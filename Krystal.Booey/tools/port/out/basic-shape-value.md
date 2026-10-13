# Port graph

Entries: `css/CSSBasicShapeValue.cpp`

- Files to port: **34**
- Boundary `platform`: 2
- Boundary `svg`: 1
- Boundary `wtf`: 21
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 5 | `css/CSSUnits.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h |
| 4 | `css/values/borders/CSSBorderRadius.h` |  | css/values/shapes/CSSInsetFunction.h, css/values/shapes/CSSRectFunction.h, css/values/shapes/CSSXywhFunction.h |
| 4 | `css/values/primitives/CSSPosition.h` |  | css/values/shapes/CSSCircleFunction.h, css/values/shapes/CSSEllipseFunction.h, css/values/shapes/CSSShapeFunction.h |
| 4 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h, css/values/shapes/CSSPolygonFunction.h, css/values/shapes/CSSShapeFunction.h |
| 3 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/shapes/CSSCircleFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSEllipseFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSInsetFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSPathFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSPolygonFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSRectFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSShapeFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 3 | `css/values/shapes/CSSXywhFunction.h` |  | css/values/shapes/CSSBasicShape.h |
| 2 | `css/CSSValue.h` |  | css/CSSBasicShapeValue.h, css/values/CSSValueTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h |
| 2 | `css/values/shapes/CSSBasicShape.h` |  | css/CSSBasicShapeValue.h |
| 1 | `css/CSSBasicShapeValue.h` |  | css/CSSBasicShapeValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/CSSBasicShapeValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | css/CSSBasicShapeValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/CSSBasicShapeValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/CSSBasicShapeValue.cpp, css/values/borders/CSSBorderRadius.h, css/values/primitives/CSSPosition.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/shapes/CSSCircleFunction.h, css/values/shapes/CSSEllipseFunction.h, css/values/shapes/CSSInsetFunction.h, css/values/shapes/CSSPathFunction.h, css/values/shapes/CSSPolygonFunction.h, css/values/shapes/CSSRectFunction.h, css/values/shapes/CSSShapeFunction.h, css/values/shapes/CSSXywhFunction.h |
| 0 | `css/CSSBasicShapeValue.cpp` |  | (entry) |

## Boundaries

### platform (2)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 2 file(s)

### svg (1)

- `svg/SVGPathByteStream.h` ← 1 file(s)

### wtf (21)

- `wtf/Brigand.h` ← 4 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 2 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/Ref.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 3 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
