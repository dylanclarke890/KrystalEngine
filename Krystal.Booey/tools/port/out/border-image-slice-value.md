# Port graph

Entries: `css/CSSBorderImageSliceValue.cpp`

- Files to port: **5**
- Boundary `wtf`: 9

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `css/RectBase.h` |  | css/Quad.h |
| 2 | `css/CSSValue.h` |  | css/CSSBorderImageSliceValue.h, css/RectBase.h |
| 2 | `css/Quad.h` |  | css/CSSBorderImageSliceValue.h |
| 1 | `css/CSSBorderImageSliceValue.h` |  | css/CSSBorderImageSliceValue.cpp |
| 0 | `css/CSSBorderImageSliceValue.cpp` |  | (entry) |

## Boundaries

### wtf (9)

- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems
