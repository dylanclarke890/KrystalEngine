# Port graph

Entries: `css/CSSQuadValue.cpp`

- Files to port: **5**
- Boundary `wtf`: 8

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `css/RectBase.h` |  | css/Quad.h |
| 2 | `css/Quad.h` |  | css/CSSQuadValue.h |
| 1 | `css/CSSQuadValue.h` |  | css/CSSQuadValue.cpp |
| 1 | `css/CSSValue.h` |  | css/CSSQuadValue.cpp, css/RectBase.h |
| 0 | `css/CSSQuadValue.cpp` |  | (entry) |

## Boundaries

### wtf (8)

- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)

## Problems
