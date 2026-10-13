# Port graph

Entries: `css/CSSRectValue.cpp`

- Files to port: **5**
- Boundary `wtf`: 8

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `css/CSSValue.h` |  | css/RectBase.h |
| 3 | `css/RectBase.h` |  | css/Rect.h |
| 2 | `css/Rect.h` |  | css/CSSRectValue.h |
| 1 | `css/CSSRectValue.h` |  | css/CSSRectValue.cpp |
| 0 | `css/CSSRectValue.cpp` |  | (entry) |

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
