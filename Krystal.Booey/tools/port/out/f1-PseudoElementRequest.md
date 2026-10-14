# Port graph

Entries: `style/PseudoElementRequest.h`

- Files to port: **3**
- Boundary `platform`: 1
- Boundary `rendering`: 1
- Boundary `wtf`: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 1 | `style/PseudoElementIdentifier.h` |  | style/PseudoElementRequest.h |
| 1 | `style/StyleScrollbarState.h` |  | style/PseudoElementRequest.h |
| 0 | `style/PseudoElementRequest.h` |  | (entry) |

## Boundaries

### platform (1)

- `platform/ScrollTypes.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 2 file(s)

### wtf (3)

- `wtf/text/AtomString.h` ← 2 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)

## Problems
