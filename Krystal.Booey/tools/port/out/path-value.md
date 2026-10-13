# Port graph

Entries: `css/CSSPathValue.cpp`

- Files to port: **65**
- Boundary `dom`: 1
- Boundary `external`: 5
- Boundary `platform`: 6
- Boundary `wtf`: 43
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 10 | `platform/graphics/LayoutSize.h` |  | platform/graphics/LayoutPoint.h |
| 9 | `platform/graphics/LayoutPoint.h` |  | platform/graphics/LayoutRect.h |
| 8 | `platform/graphics/ColorMatrix.h` |  | platform/graphics/ColorTypes.h |
| 8 | `platform/graphics/ColorModels.h` |  | platform/graphics/ColorTypes.h |
| 8 | `platform/graphics/ColorTransferFunctions.h` |  | platform/graphics/ColorTypes.h |
| 8 | `platform/graphics/IntRect.h` |  | platform/graphics/LayoutRect.h, platform/graphics/Region.h |
| 8 | `platform/graphics/LayoutRect.h` |  | platform/graphics/LayoutRoundedRect.h |
| 7 | `platform/graphics/ColorComponents.h` |  | platform/graphics/ColorTypes.h, platform/graphics/ColorUtilities.h |
| 7 | `platform/graphics/ColorTypes.h` |  | platform/graphics/ColorConversion.h, platform/graphics/ColorSpace.h, platform/graphics/ColorUtilities.h |
| 7 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 7 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h, platform/graphics/LayoutSize.h |
| 7 | `platform/graphics/LayoutRoundedRect.h` |  | platform/graphics/FloatRoundedRect.h |
| 7 | `platform/graphics/PlatformColorSpace.h` |  | platform/graphics/DestinationColorSpace.h |
| 7 | `platform/graphics/Region.h` |  | platform/graphics/FloatRoundedRect.h |
| 7 | `platform/graphics/RotationDirection.h` |  | platform/graphics/PathSegmentData.h |
| 6 | `css/values/color/CSSColorType.h` |  | css/parser/CSSPropertyParserConsumer+Color.h |
| 6 | `platform/graphics/ColorConversion.h` |  | platform/graphics/Color.h |
| 6 | `platform/graphics/ColorSpace.h` |  | platform/graphics/Color.h |
| 6 | `platform/graphics/ColorUtilities.h` |  | platform/graphics/Color.h |
| 6 | `platform/graphics/DestinationColorSpace.h` |  | platform/graphics/Color.h |
| 6 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 6 | `platform/graphics/FloatRoundedRect.h` |  | platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h |
| 6 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/FloatRoundedRect.h, platform/graphics/LayoutSize.h |
| 6 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h, platform/graphics/IntRect.h |
| 6 | `platform/graphics/PathSegmentData.h` |  | platform/graphics/PathSegment.h |
| 6 | `platform/graphics/cairo/RefPtrCairo.h` | yes | platform/graphics/PlatformPath.h |
| 5 | `css/CSSUnits.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/parser/CSSPropertyParserConsumer+Color.h` |  | svg/properties/SVGPropertyTraits.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` |  | css/values/primitives/CSSPrimitiveNumericUnits.h |
| 5 | `platform/graphics/Color.h` |  | svg/properties/SVGPropertyTraits.h |
| 5 | `platform/graphics/FloatPoint.h` |  | platform/graphics/FloatRect.h, platform/graphics/LayoutPoint.h, platform/graphics/PathElement.h, svg/SVGPathConsumer.h, svg/properties/SVGPropertyTraits.h |
| 5 | `platform/graphics/FloatRect.h` |  | platform/graphics/FloatRoundedRect.h, platform/graphics/LayoutRect.h, platform/graphics/PathSegmentData.h, svg/properties/SVGPropertyTraits.h |
| 5 | `platform/graphics/PathElement.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h |
| 5 | `platform/graphics/PathImpl.h` |  | platform/graphics/Path.h |
| 5 | `platform/graphics/PathSegment.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h |
| 5 | `platform/graphics/PlatformPath.h` |  | platform/graphics/Path.h |
| 5 | `platform/graphics/WindRule.h` |  | platform/graphics/Path.h |
| 5 | `svg/SVGParserUtilities.h` |  | svg/properties/SVGPropertyTraits.h |
| 5 | `svg/SVGPathConsumer.h` |  | svg/SVGPathUtilities.h |
| 4 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h, css/values/primitives/CSSPrimitiveNumericConcepts.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSUnevaluatedCalc.h, css/values/shapes/CSSFillRule.h |
| 4 | `css/values/primitives/CSSPrimitiveKeywordList.h` |  | css/values/primitives/CSSPrimitiveData.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/values/primitives/CSSPrimitiveNumericConcepts.h, css/values/primitives/CSSPrimitiveNumericRaw.h, css/values/primitives/CSSPrimitiveNumericUnits.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` |  | css/values/primitives/CSSPrimitiveNumericRaw.h |
| 4 | `platform/graphics/Path.h` |  | svg/SVGPathByteStream.h |
| 4 | `svg/SVGPathUtilities.h` |  | svg/SVGPathByteStream.h |
| 4 | `svg/properties/SVGPropertyTraits.h` |  | svg/SVGPathByteStream.h |
| 3 | `css/values/primitives/CSSPrimitiveData.h` |  | css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h, css/values/primitives/CSSPrimitiveNumericUnits.h, css/values/primitives/CSSUnevaluatedCalc.h |
| 3 | `css/values/primitives/CSSPrimitiveNumericRaw.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveKeywordList.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/primitives/CSSUnevaluatedCalc.h` |  | css/values/primitives/CSSPrimitiveData.h, css/values/primitives/CSSPrimitiveNumeric.h |
| 3 | `css/values/shapes/CSSFillRule.h` |  | css/values/shapes/CSSPathFunction.h |
| 3 | `svg/SVGPathByteStream.h` |  | css/values/shapes/CSSPathFunction.h |
| 2 | `css/CSSValue.h` |  | css/CSSPathValue.h, css/values/CSSValueTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumeric.h` |  | css/values/primitives/CSSPrimitiveNumericOrKeyword.h, css/values/primitives/CSSPrimitiveNumericTypes.h |
| 2 | `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` |  | css/values/primitives/CSSPrimitiveNumericTypes.h |
| 2 | `css/values/shapes/CSSPathFunction.h` |  | css/CSSPathValue.h |
| 1 | `css/CSSPathValue.h` |  | css/CSSPathValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` |  | css/CSSPathValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h` |  | css/CSSPathValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h` |  | css/CSSPathValue.cpp |
| 1 | `css/values/primitives/CSSPrimitiveNumericTypes.h` |  | css/CSSPathValue.cpp, css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h, css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h, css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h, css/values/shapes/CSSPathFunction.h |
| 0 | `css/CSSPathValue.cpp` |  | (entry) |

## Boundaries

### dom (1)

- `dom/QualifiedName.h` ← 1 file(s)

### external (5)

- `CoreGraphics/CGPath.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 1 file(s)
- `skia/core/SkColor.h` ← 1 file(s)
- `skia/core/SkColorSpace.h` ← 1 file(s)
- `skia/core/SkData.h` ← 1 file(s)

### platform (6)

- `platform/BoxExtents.h` ← 2 file(s)
- `platform/CommonAtomStrings.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/PlatformExportMacros.h` ← 3 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (43)

- `wtf/ArgumentCoder.h` ← 2 file(s)
- `wtf/Assertions.h` ← 3 file(s)
- `wtf/Brigand.h` ← 4 file(s)
- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/Compiler.h` ← 1 file(s)
- `wtf/DataRef.h` ← 1 file(s)
- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/EnumTraits.h` ← 4 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/GetPtr.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/Hasher.h` ← 2 file(s)
- `wtf/IterationStatus.h` ← 2 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 9 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 2 file(s)
- `wtf/Platform.h` ← 9 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/RetainPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 4 file(s)
- `wtf/TZoneMalloc.h` ← 8 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 2 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 5 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/text/ASCIILiteral.h` ← 2 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)
- `wtf/text/ParsingUtilities.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 2 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
