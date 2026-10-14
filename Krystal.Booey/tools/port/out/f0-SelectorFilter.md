# Port graph

Entries: `css/SelectorFilter.cpp`

- Files to port: **4**
- Boundary `dom`: 5
- Boundary `platform`: 1
- Boundary `rendering`: 1
- Boundary `wtf`: 6
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `css/CSSSelector.h` |  | css/CSSSelectorList.h |
| 1 | `css/CSSSelectorList.h` |  | css/SelectorFilter.cpp |
| 1 | `css/SelectorFilter.h` |  | css/SelectorFilter.cpp |
| 0 | `css/SelectorFilter.cpp` |  | (entry) |

## Boundaries

### dom (5)

- `dom/Element.h` ← 1 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/QualifiedName.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/StyledElement.h` ← 1 file(s)

### platform (1)

- `platform/CommonAtomStrings.h` ← 1 file(s)

### rendering (1)

- `rendering/style/RenderStyleConstants.h` ← 1 file(s)

### wtf (6)

- `wtf/BloomFilter.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 2 file(s)
- `wtf/UniqueArray.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)

## Problems

### unresolved (2)

- `CSSSelectorEnums.h` ← 1 file(s)
- `HTMLNames.h` ← 1 file(s)
