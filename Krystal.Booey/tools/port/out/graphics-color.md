# Port graph

Entries: `platform/graphics/AlphaPremultiplication.h`, `platform/graphics/Color.cpp`, `platform/graphics/Color.h`, `platform/graphics/ColorBlending.cpp`, `platform/graphics/ColorBlending.h`, `platform/graphics/ColorComponents.h`, `platform/graphics/ColorConversion.cpp`, `platform/graphics/ColorConversion.h`, `platform/graphics/ColorHash.h`, `platform/graphics/ColorInterpolation.cpp`, `platform/graphics/ColorInterpolation.h`, `platform/graphics/ColorInterpolationMethod.cpp`, `platform/graphics/ColorInterpolationMethod.h`, `platform/graphics/ColorLuminance.cpp`, `platform/graphics/ColorLuminance.h`, `platform/graphics/ColorMatrix.h`, `platform/graphics/ColorModels.h`, `platform/graphics/ColorNormalization.h`, `platform/graphics/ColorSerialization.cpp`, `platform/graphics/ColorSerialization.h`, `platform/graphics/ColorSpace.cpp`, `platform/graphics/ColorSpace.h`, `platform/graphics/ColorTransferFunctions.h`, `platform/graphics/ColorTypes.h`, `platform/graphics/ColorUtilities.cpp`, `platform/graphics/ColorUtilities.h`, `platform/graphics/DestinationColorSpace.cpp`, `platform/graphics/DestinationColorSpace.h`

- Files to port: **31**
- Boundary `external`: 5
- Boundary `platform`: 3
- Boundary `wtf`: 23

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 1 | `platform/graphics/PlatformColorSpace.h` |  | platform/graphics/DestinationColorSpace.h |
| 1 | `platform/graphics/cg/ColorSpaceCG.h` | yes | platform/graphics/DestinationColorSpace.cpp, platform/graphics/DestinationColorSpace.h |
| 1 | `platform/graphics/skia/ColorSpaceSkia.h` | yes | platform/graphics/DestinationColorSpace.cpp, platform/graphics/DestinationColorSpace.h |
| 0 | `platform/graphics/AlphaPremultiplication.h` |  | (entry) |
| 0 | `platform/graphics/Color.cpp` |  | (entry) |
| 0 | `platform/graphics/Color.h` |  | (entry) |
| 0 | `platform/graphics/ColorBlending.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorBlending.h` |  | (entry) |
| 0 | `platform/graphics/ColorComponents.h` |  | (entry) |
| 0 | `platform/graphics/ColorConversion.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorConversion.h` |  | (entry) |
| 0 | `platform/graphics/ColorHash.h` |  | (entry) |
| 0 | `platform/graphics/ColorInterpolation.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorInterpolation.h` |  | (entry) |
| 0 | `platform/graphics/ColorInterpolationMethod.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorInterpolationMethod.h` |  | (entry) |
| 0 | `platform/graphics/ColorLuminance.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorLuminance.h` |  | (entry) |
| 0 | `platform/graphics/ColorMatrix.h` |  | (entry) |
| 0 | `platform/graphics/ColorModels.h` |  | (entry) |
| 0 | `platform/graphics/ColorNormalization.h` |  | (entry) |
| 0 | `platform/graphics/ColorSerialization.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorSerialization.h` |  | (entry) |
| 0 | `platform/graphics/ColorSpace.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorSpace.h` |  | (entry) |
| 0 | `platform/graphics/ColorTransferFunctions.h` |  | (entry) |
| 0 | `platform/graphics/ColorTypes.h` |  | (entry) |
| 0 | `platform/graphics/ColorUtilities.cpp` |  | (entry) |
| 0 | `platform/graphics/ColorUtilities.h` |  | (entry) |
| 0 | `platform/graphics/DestinationColorSpace.cpp` |  | (entry) |
| 0 | `platform/graphics/DestinationColorSpace.h` |  | (entry) |

## Boundaries

### external (5)

- `CoreGraphics/CoreGraphics.h` ← 1 file(s)
- `pal/spi/cg/CoreGraphicsSPI.h` ← 2 file(s)
- `skia/core/SkColor.h` ← 1 file(s)
- `skia/core/SkColorSpace.h` ← 2 file(s)
- `skia/core/SkData.h` ← 1 file(s)

### platform (3)

- `platform/NotImplemented.h` ← 2 file(s)
- `platform/PlatformExportMacros.h` ← 2 file(s)
- `platform/animation/AnimationUtilities.h` ← 2 file(s)

### wtf (23)

- `wtf/Assertions.h` ← 6 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/Forward.h` ← 5 file(s)
- `wtf/GetPtr.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/HexNumber.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 8 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Platform.h` ← 2 file(s)
- `wtf/Ref.h` ← 1 file(s)
- `wtf/RetainPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 2 file(s)
- `wtf/TZoneMalloc.h` ← 1 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/cf/TypeCastsCF.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 2 file(s)
- `wtf/text/StringBuilder.h` ← 4 file(s)
- `wtf/text/TextStream.h` ← 9 file(s)

## Problems
