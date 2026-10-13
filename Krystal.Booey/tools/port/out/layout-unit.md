# Port graph

Entries: `platform/LayoutUnit.h`, `platform/graphics/LayoutPoint.h`, `platform/graphics/LayoutSize.h`, `platform/graphics/FloatPoint.h`

- Files to port: **20**
- Boundary `animation`: 1
- Boundary `external`: 3
- Boundary `wtf`: 17

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `platform/BoxSides.h` |  | platform/RectEdges.h |
| 4 | `platform/text/WritingMode.h` |  | platform/BoxSides.h, platform/RectEdges.h |
| 3 | `platform/RectEdges.h` |  | platform/BoxExtents.h |
| 2 | `platform/BoxExtents.h` |  | platform/graphics/FloatRect.h |
| 2 | `platform/PlatformExportMacros.h` |  | platform/graphics/IntSize.h |
| 2 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 2 | `platform/graphics/FloatPoint3D.h` |  | platform/graphics/transforms/TransformationMatrix.h |
| 2 | `platform/graphics/transforms/Quaternion.h` |  | platform/graphics/transforms/TransformationMatrix.h |
| 1 | `platform/FloatConversion.h` |  | platform/graphics/FloatPoint.h, platform/graphics/transforms/AffineTransform.h |
| 1 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 1 | `platform/graphics/FloatRect.h` |  | platform/graphics/FloatPoint.h |
| 1 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutSize.h, platform/graphics/transforms/AffineTransform.h |
| 1 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h, platform/graphics/transforms/TransformationMatrix.h |
| 1 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h, platform/graphics/LayoutSize.h |
| 1 | `platform/graphics/transforms/AffineTransform.h` |  | platform/graphics/FloatPoint.h |
| 1 | `platform/graphics/transforms/TransformationMatrix.h` |  | platform/graphics/FloatPoint.h |
| 0 | `platform/LayoutUnit.h` |  | (entry) |
| 0 | `platform/graphics/FloatPoint.h` |  | (entry) |
| 0 | `platform/graphics/LayoutPoint.h` |  | (entry) |
| 0 | `platform/graphics/LayoutSize.h` |  | (entry) |

## Boundaries

### animation (1)

- `animation/CompositeOperation.h` ← 3 file(s)

### external (3)

- `CoreGraphics/CGBase.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 1 file(s)
- `simd/simd.h` ← 1 file(s)

### wtf (17)

- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/ExportMacros.h` ← 1 file(s)
- `wtf/Forward.h` ← 3 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 4 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Platform.h` ← 7 file(s)
- `wtf/SaturatedArithmetic.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 4 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 6 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems
