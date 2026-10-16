# Port graph

Entries: `C:/Users/dylan/repos/WebKit/Source/WebCore/style/StyleDifference.cpp`

- Files to port: **6**
- Boundary `layout`: 2
- Boundary `rendering`: 2
- Boundary `wtf`: 6

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `style/PseudoElementIdentifier.h` |  | style/StyleExtractor.h, style/StyleExtractorState.h |
| 2 | `style/StyleExtractorState.h` |  | style/StyleExtractor.h |
| 1 | `style/InlineTextBoxStyle.h` |  | C:/Users/dylan/repos/WebKit/Source/WebCore/style/StyleDifference.cpp |
| 1 | `style/StyleDifference.h` |  | C:/Users/dylan/repos/WebKit/Source/WebCore/style/StyleDifference.cpp |
| 1 | `style/StyleExtractor.h` |  | C:/Users/dylan/repos/WebKit/Source/WebCore/style/StyleDifference.cpp |
| 0 | `C:/Users/dylan/repos/WebKit/Source/WebCore/style/StyleDifference.cpp` |  | (entry) |

## Boundaries

### layout (2)

- `layout/integration/inline/InlineIteratorInlineBox.h` ← 1 file(s)
- `layout/integration/inline/InlineIteratorLineBox.h` ← 1 file(s)

### rendering (2)

- `rendering/style/RenderStyle+GettersInlines.h` ← 1 file(s)
- `rendering/style/RenderStyleConstants.h` ← 3 file(s)

### wtf (6)

- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Ref.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)

## Problems
