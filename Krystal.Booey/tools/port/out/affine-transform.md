# Port graph

Entries: `platform/graphics/transforms/AffineTransform.cpp`, `platform/graphics/transforms/TransformationMatrix.cpp`, `platform/graphics/Region.cpp`, `platform/graphics/FloatQuad.cpp`, `platform/graphics/FloatRect.cpp`, `platform/graphics/IntRect.cpp`, `platform/graphics/GeometryUtilities.cpp`

- Files to port: **32**
- Boundary `animation`: 1
- Boundary `external`: 3
- Boundary `wtf`: 24

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 4 | `platform/BoxSides.h` |  | platform/RectEdges.h |
| 4 | `platform/PlatformExportMacros.h` |  | platform/graphics/IntSize.h |
| 4 | `platform/text/WritingMode.h` |  | platform/BoxSides.h, platform/RectEdges.h |
| 3 | `platform/RectEdges.h` |  | platform/BoxExtents.h |
| 3 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 3 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 3 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h, platform/graphics/LayoutSize.h |
| 3 | `platform/graphics/LayoutSize.h` |  | platform/graphics/LayoutPoint.h |
| 2 | `platform/BoxExtents.h` |  | platform/graphics/FloatRect.h, platform/graphics/LayoutRect.h |
| 2 | `platform/LayoutUnit.h` |  | platform/BoxExtents.h, platform/graphics/IntRect.h, platform/graphics/LayoutSize.h |
| 2 | `platform/graphics/FloatPoint.h` |  | platform/graphics/FloatPoint3D.h, platform/graphics/FloatRect.h, platform/graphics/LayoutPoint.h, platform/graphics/transforms/AffineTransform.h, platform/graphics/transforms/TransformationMatrix.h |
| 2 | `platform/graphics/FloatPoint3D.h` |  | platform/graphics/transforms/TransformationMatrix.h |
| 2 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutSize.h, platform/graphics/transforms/AffineTransform.h |
| 2 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h, platform/graphics/IntRect.h, platform/graphics/transforms/TransformationMatrix.h |
| 2 | `platform/graphics/LayoutPoint.h` |  | platform/graphics/LayoutRect.h |
| 2 | `platform/graphics/transforms/Quaternion.h` |  | platform/graphics/transforms/TransformationMatrix.h |
| 1 | `platform/FloatConversion.h` |  | platform/graphics/FloatRect.cpp, platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/AffineTransform.h |
| 1 | `platform/graphics/FloatQuad.h` |  | platform/graphics/FloatQuad.cpp, platform/graphics/GeometryUtilities.cpp, platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 1 | `platform/graphics/FloatRect.h` |  | platform/graphics/FloatQuad.h, platform/graphics/FloatRect.cpp, platform/graphics/GeometryUtilities.h, platform/graphics/IntRect.cpp, platform/graphics/LayoutRect.h, platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 1 | `platform/graphics/GeometryUtilities.h` |  | platform/graphics/FloatQuad.cpp, platform/graphics/GeometryUtilities.cpp, platform/graphics/transforms/AffineTransform.cpp |
| 1 | `platform/graphics/IntRect.h` |  | platform/graphics/FloatQuad.h, platform/graphics/FloatRect.cpp, platform/graphics/GeometryUtilities.h, platform/graphics/IntRect.cpp, platform/graphics/LayoutRect.h, platform/graphics/Region.h, platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 1 | `platform/graphics/LayoutRect.h` |  | platform/graphics/IntRect.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 1 | `platform/graphics/Region.h` |  | platform/graphics/Region.cpp, platform/graphics/transforms/AffineTransform.cpp |
| 1 | `platform/graphics/transforms/AffineTransform.h` |  | platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 1 | `platform/graphics/transforms/TransformationMatrix.h` |  | platform/graphics/transforms/AffineTransform.cpp, platform/graphics/transforms/TransformationMatrix.cpp |
| 0 | `platform/graphics/FloatQuad.cpp` |  | (entry) |
| 0 | `platform/graphics/FloatRect.cpp` |  | (entry) |
| 0 | `platform/graphics/GeometryUtilities.cpp` |  | (entry) |
| 0 | `platform/graphics/IntRect.cpp` |  | (entry) |
| 0 | `platform/graphics/Region.cpp` |  | (entry) |
| 0 | `platform/graphics/transforms/AffineTransform.cpp` |  | (entry) |
| 0 | `platform/graphics/transforms/TransformationMatrix.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/CompositeOperation.h` ← 3 file(s)

### external (3)

- `CoreGraphics/CGBase.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 1 file(s)
- `simd/simd.h` ← 1 file(s)

### wtf (24)

- `wtf/ArgumentCoder.h` ← 2 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/CheckedArithmetic.h` ← 2 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/ExportMacros.h` ← 1 file(s)
- `wtf/Forward.h` ← 6 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 9 file(s)
- `wtf/NeverDestroyed.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Platform.h` ← 8 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/SaturatedArithmetic.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 7 file(s)
- `wtf/TZoneMallocInlines.h` ← 5 file(s)
- `wtf/Vector.h` ← 3 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 8 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems
