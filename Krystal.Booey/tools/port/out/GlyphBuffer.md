# Port graph

Entries: `platform/graphics/GlyphBuffer.h`

- Files to port: **9**
- Boundary `external`: 3
- Boundary `platform`: 1
- Boundary `wtf`: 11

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h |
| 2 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h |
| 2 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 2 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h |
| 1 | `platform/graphics/FloatPoint.h` |  | platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h |
| 1 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h |
| 1 | `platform/graphics/Glyph.h` |  | platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h |
| 1 | `platform/graphics/GlyphBufferMembers.h` |  | platform/graphics/GlyphBuffer.h |
| 0 | `platform/graphics/GlyphBuffer.h` |  | (entry) |

## Boundaries

### external (3)

- `CoreGraphics/CGFont.h` ← 1 file(s)
- `CoreGraphics/CGGeometry.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 1 file(s)

### platform (1)

- `platform/PlatformExportMacros.h` ← 1 file(s)

### wtf (11)

- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/Forward.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 3 file(s)
- `wtf/Platform.h` ← 5 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 2 file(s)
- `wtf/Vector.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems
