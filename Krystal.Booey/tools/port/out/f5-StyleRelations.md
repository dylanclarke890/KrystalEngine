# Port graph

Entries: `style/StyleRelations.cpp`

- Files to port: **5**
- Boundary `dom`: 3
- Boundary `wtf`: 5
- Unresolved: 1

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 1 | `rendering/style/RenderStyle.h` |  | style/StyleRelations.cpp |
| 1 | `style/StyleRelations.h` |  | style/StyleRelations.cpp |
| 1 | `style/StyleUpdate.h` |  | style/StyleRelations.cpp |
| 0 | `style/StyleRelations.cpp` |  | (entry) |

## Boundaries

### dom (3)

- `dom/Element.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeRenderStyle.h` ← 1 file(s)

### wtf (5)

- `wtf/Forward.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)

## Problems

### unresolved (1)

- `RenderStyleProperties.h` ← 1 file(s)
