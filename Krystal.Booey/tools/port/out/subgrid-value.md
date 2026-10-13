# Port graph

Entries: `css/CSSSubgridValue.cpp`

- Files to port: **4**
- Boundary `external`: 1
- Boundary `wtf`: 9

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `css/CSSValue.h` |  | css/CSSValueList.h |
| 2 | `css/CSSValueList.h` |  | css/CSSSubgridValue.h |
| 1 | `css/CSSSubgridValue.h` |  | css/CSSSubgridValue.cpp |
| 0 | `css/CSSSubgridValue.cpp` |  | (entry) |

## Boundaries

### external (1)

- `unicode/umachine.h` ← 1 file(s)

### wtf (9)

- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)

## Problems
