# Port graph

Entries: `css/CSSGridTemplateAreasValue.cpp`

- Files to port: **9**
- Boundary `platform`: 2
- Boundary `rendering`: 1
- Boundary `wtf`: 20
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/grid/CSSGridNamedAreaMap.h, css/values/grid/CSSGridTemplateAreas.h |
| 3 | `css/values/grid/CSSGridNamedAreaMap.h` |  | css/values/grid/CSSGridTemplateAreas.h |
| 2 | `css/CSSValue.h` |  | css/CSSGridTemplateAreasValue.h, css/values/CSSValueTypes.h |
| 2 | `css/values/grid/CSSGridTemplateAreas.h` |  | css/CSSGridTemplateAreasValue.h |
| 1 | `css/CSSGridTemplateAreasValue.h` |  | css/CSSGridTemplateAreasValue.cpp |
| 0 | `css/CSSGridTemplateAreasValue.cpp` |  | (entry) |

## Boundaries

### platform (2)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### rendering (1)

- `rendering/style/GridArea.h` ← 1 file(s)

### wtf (20)

- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 2 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 3 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
