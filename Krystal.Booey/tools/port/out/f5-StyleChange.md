# Port graph

Entries: `style/StyleChange.cpp`

- Files to port: **5**
- Boundary `wtf`: 4
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `rendering/style/RenderStyle.h` |  | rendering/style/RenderStyle+GettersInlines.h |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | style/StyleChange.cpp |
| 1 | `rendering/style/RenderStyleConstants.h` |  | style/StyleChange.cpp |
| 1 | `style/StyleChange.h` |  | style/StyleChange.cpp |
| 0 | `style/StyleChange.cpp` |  | (entry) |

## Boundaries

### wtf (4)

- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)

## Problems

### unresolved (2)

- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
