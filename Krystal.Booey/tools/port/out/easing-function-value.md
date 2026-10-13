# Port graph

Entries: `css/CSSEasingFunctionValue.cpp`

- Files to port: **26**
- Boundary `platform`: 2
- Boundary `wtf`: 21
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/CSSUnits.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 6 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 5 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 5 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 4 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/easing/CSSCubicBezierEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSLinearEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSSpringEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/easing/CSSStepsEasingFunction.h` |  | css/values/easing/CSSEasingFunction.h |
| 3 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h |
| 2 | `css/CSSValue.h` |  | css/CSSEasingFunctionValue.h, css/values/CSSValueTypes.h |
| 2 | `css/values/easing/CSSEasingFunction.h` |  | css/CSSEasingFunctionValue.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/values/easing/CSSCubicBezierEasingFunction.h, css/values/easing/CSSLinearEasingFunction.h, css/values/easing/CSSSpringEasingFunction.h, css/values/easing/CSSStepsEasingFunction.h, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h |
| 1 | `css/CSSEasingFunctionValue.h` |  | css/CSSEasingFunctionValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/CSSEasingFunctionValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/CSSEasingFunctionValue.cpp |
| 0 | `css/CSSEasingFunctionValue.cpp` |  | (entry) |

## Boundaries

### platform (2)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

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
