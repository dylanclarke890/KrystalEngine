# Port graph

Entries: `platform/graphics/TextRun.h`

- Files to port: **6**
- Boundary `external`: 1
- Boundary `wtf`: 14

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 2 | `platform/graphics/FontTaggedSettings.h` |  | platform/text/TextFlags.h |
| 1 | `platform/graphics/TabSize.h` |  | platform/graphics/TextRun.h |
| 1 | `platform/text/TextFlags.h` |  | platform/graphics/TextRun.h |
| 1 | `platform/text/TextSpacing.h` |  | platform/graphics/TextRun.h |
| 1 | `platform/text/WritingMode.h` |  | platform/graphics/TextRun.h |
| 0 | `platform/graphics/TextRun.h` |  | (entry) |

## Boundaries

### external (1)

- `unicode/umachine.h` ← 1 file(s)

### wtf (14)

- `wtf/ArgumentCoder.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/Forward.h` ← 1 file(s)
- `wtf/HashCountedSet.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/CharacterProperties.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/unicode/CharacterNames.h` ← 1 file(s)

## Problems
