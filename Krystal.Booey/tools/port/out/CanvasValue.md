# Port graph

Entries: `css/CSSCanvasValue.cpp`

- Files to port: **3**
- Boundary `rendering`: 1
- Boundary `wtf`: 9

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `css/CSSValue.h` |  | css/CSSCanvasValue.h |
| 1 | `css/CSSCanvasValue.h` |  | css/CSSCanvasValue.cpp |
| 0 | `css/CSSCanvasValue.cpp` |  | (entry) |

## Boundaries

### rendering (1)

- `rendering/style/StyleCanvasImage.h` ← 1 file(s)

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
