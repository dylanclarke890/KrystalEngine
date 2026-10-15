# Port graph

Entries: `style/StyleUpdate.cpp`

- Files to port: **3**
- Boundary `dom`: 7
- Boundary `rendering`: 1
- Boundary `svg`: 1
- Boundary `wtf`: 5

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `style/StyleChange.h` |  | style/StyleUpdate.h |
| 1 | `style/StyleUpdate.h` |  | style/StyleUpdate.cpp |
| 0 | `style/StyleUpdate.cpp` |  | (entry) |

## Boundaries

### dom (7)

- `dom/ComposedTreeAncestorIterator.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 1 file(s)
- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeRenderStyle.h` ← 1 file(s)
- `dom/Text.h` ← 1 file(s)

### rendering (1)

- `rendering/RenderElement.h` ← 1 file(s)

### svg (1)

- `svg/SVGElement.h` ← 1 file(s)

### wtf (5)

- `wtf/HashMap.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)

## Problems
