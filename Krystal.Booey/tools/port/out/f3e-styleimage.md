# Port graph

Entries: `rendering/style/StyleImage.h`

- Files to port: **45**
- Boundary `animation`: 1
- Boundary `external`: 5
- Boundary `loader`: 1
- Boundary `platform`: 5
- Boundary `rendering`: 1
- Boundary `wtf`: 45
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 7 | `platform/graphics/ColorMatrix.h` |  | platform/graphics/ColorTypes.h |
| 7 | `platform/graphics/ColorModels.h` |  | platform/graphics/ColorTypes.h |
| 7 | `platform/graphics/ColorTransferFunctions.h` |  | platform/graphics/ColorTypes.h |
| 6 | `platform/graphics/ColorComponents.h` |  | platform/graphics/ColorTypes.h, platform/graphics/ColorUtilities.h |
| 6 | `platform/graphics/ColorTypes.h` |  | platform/graphics/ColorConversion.h, platform/graphics/ColorSpace.h, platform/graphics/ColorUtilities.h |
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h |
| 5 | `platform/graphics/ColorConversion.h` |  | platform/graphics/Color.h |
| 5 | `platform/graphics/ColorSpace.h` |  | platform/graphics/Color.h |
| 5 | `platform/graphics/ColorUtilities.h` |  | platform/graphics/Color.h |
| 5 | `platform/graphics/RenderingResourceIdentifier.h` |  | platform/graphics/RenderingResource.h |
| 5 | `platform/graphics/cairo/RefPtrCairo.h` | yes | platform/graphics/PlatformImage.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `platform/graphics/Color.h` |  | platform/graphics/GraphicsTypes.h |
| 4 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h |
| 4 | `platform/graphics/PlatformImage.h` |  | platform/graphics/NativeImage.h |
| 4 | `platform/graphics/RenderingResource.h` |  | platform/graphics/NativeImage.h |
| 4 | `platform/graphics/WindRule.h` |  | platform/graphics/GraphicsTypes.h |
| 3 | `css/calc/CSSCalcSymbolTable.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/values/CSSNoConversionDataRequiredToken.h` |  | style/values/StyleValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSURLModifiers.h, style/values/StyleValueTypes.h |
| 3 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 3 | `platform/graphics/FloatPoint.h` |  | platform/graphics/FloatRect.h, platform/graphics/transforms/AffineTransform.h |
| 3 | `platform/graphics/GraphicsTypes.h` |  | platform/graphics/ImagePaintingOptions.h |
| 3 | `platform/graphics/IntSize.h` |  | platform/graphics/DecodingOptions.h, platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h |
| 3 | `platform/graphics/NativeImage.h` |  | platform/graphics/ImageAdapter.h |
| 3 | `platform/graphics/PlatformColorSpace.h` |  | platform/graphics/DestinationColorSpace.h |
| 3 | `platform/graphics/PlatformDynamicRangeLimit.h` |  | platform/graphics/ImagePaintingOptions.h |
| 3 | `platform/graphics/transforms/AffineTransform.h` |  | platform/graphics/ImageOrientation.h |
| 2 | `css/values/primitives/CSSURL.h` |  | style/values/primitives/StyleURL.h |
| 2 | `platform/graphics/DecodingOptions.h` |  | platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h |
| 2 | `platform/graphics/DestinationColorSpace.h` |  | platform/graphics/Color.h, platform/graphics/Image.h |
| 2 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 2 | `platform/graphics/FloatRect.h` |  | platform/graphics/Image.h |
| 2 | `platform/graphics/ImageAdapter.h` |  | platform/graphics/Image.h |
| 2 | `platform/graphics/ImageOrientation.h` |  | platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h |
| 2 | `platform/graphics/ImagePaintingOptions.h` |  | platform/graphics/Image.h |
| 2 | `platform/graphics/ImageTypes.h` |  | platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h, platform/graphics/NativeImage.h |
| 2 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h |
| 2 | `style/values/StyleValueTypes.h` |  | style/values/primitives/StyleURL.h |
| 1 | `css/CSSValue.h` |  | css/values/CSSValueTypes.h, rendering/style/StyleImage.h |
| 1 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/GraphicsTypes.h, platform/graphics/ImageOrientation.h, platform/graphics/transforms/AffineTransform.h, rendering/style/StyleImage.h |
| 1 | `platform/graphics/Image.h` |  | rendering/style/StyleImage.h |
| 1 | `style/values/primitives/StyleURL.h` |  | rendering/style/StyleImage.h |
| 0 | `rendering/style/StyleImage.h` |  | (entry) |

## Boundaries

### animation (1)

- `animation/CompositeOperation.h` ← 1 file(s)

### external (5)

- `CoreGraphics/CoreGraphics.h` ← 1 file(s)
- `skia/core/SkColor.h` ← 1 file(s)
- `skia/core/SkColorSpace.h` ← 1 file(s)
- `skia/core/SkData.h` ← 1 file(s)
- `skia/core/SkImage.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (5)

- `platform/BoxExtents.h` ← 1 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/PlatformExportMacros.h` ← 4 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### rendering (1)

- `rendering/RenderObject.h` ← 1 file(s)

### wtf (45)

- `wtf/AbstractCanMakeCheckedPtr.h` ← 1 file(s)
- `wtf/ArgumentCoder.h` ← 1 file(s)
- `wtf/Assertions.h` ← 3 file(s)
- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 1 file(s)
- `wtf/Compiler.h` ← 3 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 7 file(s)
- `wtf/GetPtr.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 7 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/Platform.h` ← 9 file(s)
- `wtf/Ref.h` ← 2 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 5 file(s)
- `wtf/RetainPtr.h` ← 4 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 6 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 1 file(s)
- `wtf/ThreadSafeWeakPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 3 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 1 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 2 file(s)
