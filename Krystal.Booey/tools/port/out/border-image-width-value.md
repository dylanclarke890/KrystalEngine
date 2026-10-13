# Port graph

Entries: `css/CSSBorderImageWidthValue.cpp`

- Files to port: **5**
- Boundary `wtf`: 9

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `css/RectBase.h` |  | css/Quad.h |
| 2 | `css/CSSValue.h` |  | css/CSSBorderImageWidthValue.h, css/RectBase.h |
| 2 | `css/Quad.h` |  | css/CSSBorderImageWidthValue.h |
| 1 | `css/CSSBorderImageWidthValue.h` |  | css/CSSBorderImageWidthValue.cpp |
| 0 | `css/CSSBorderImageWidthValue.cpp` |  | (entry) |

## Boundaries

### wtf (9)

- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems
