# Port graph

Entries: `css/values/primitives/CSSURLModifiers.cpp`

- Files to port: **7**
- Boundary `loader`: 2
- Boundary `platform`: 2
- Boundary `wtf`: 17
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h |
| 3 | `css/CSSValue.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 2 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSURLModifiers.h |
| 1 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURLModifiers.cpp |
| 0 | `css/values/primitives/CSSURLModifiers.cpp` |  | (entry) |

## Boundaries

### loader (2)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)

### platform (2)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (17)

- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
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
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
