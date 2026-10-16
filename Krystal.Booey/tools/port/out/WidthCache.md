# Port graph

Entries: `platform/graphics/WidthCache.h`

- Files to port: **7**
- Boundary `external`: 1
- Boundary `wtf`: 20

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `platform/graphics/FontTaggedSettings.h` |  | platform/text/TextFlags.h |
| 2 | `platform/graphics/TabSize.h` |  | platform/graphics/TextRun.h |
| 2 | `platform/text/TextFlags.h` |  | platform/graphics/TextRun.h |
| 2 | `platform/text/WritingMode.h` |  | platform/graphics/TextRun.h |
| 1 | `platform/graphics/TextRun.h` |  | platform/graphics/WidthCache.h |
| 1 | `platform/text/TextSpacing.h` |  | platform/graphics/TextRun.h, platform/graphics/WidthCache.h |
| 0 | `platform/graphics/WidthCache.h` |  | (entry) |

## Boundaries

### external (1)

- `unicode/umachine.h` ← 1 file(s)

### wtf (20)

- `wtf/ArgumentCoder.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/Forward.h` ← 2 file(s)
- `wtf/HashCountedSet.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashSet.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 3 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MemoryPressureHandler.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/ZippedRange.h` ← 1 file(s)
- `wtf/text/CharacterProperties.h` ← 1 file(s)
- `wtf/text/StringCommon.h` ← 1 file(s)
- `wtf/text/StringImpl.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/text/WYHash.h` ← 1 file(s)
- `wtf/unicode/CharacterNames.h` ← 1 file(s)

## Problems
