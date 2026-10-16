# Port graph

Entries: `platform/graphics/WidthIterator.h`

- Files to port: **70**
- Boundary `external`: 14
- Boundary `loader`: 1
- Boundary `platform`: 6
- Boundary `wtf`: 52
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 8 | `platform/graphics/LayoutSize.h` |  | platform/graphics/LayoutPoint.h |
| 7 | `platform/graphics/LayoutPoint.h` |  | platform/graphics/LayoutRect.h |
| 6 | `platform/graphics/IntRect.h` |  | platform/graphics/LayoutRect.h, platform/graphics/Region.h |
| 6 | `platform/graphics/LayoutRect.h` |  | platform/graphics/LayoutRoundedRect.h |
| 5 | `platform/graphics/LayoutRoundedRect.h` |  | platform/graphics/FloatRoundedRect.h |
| 5 | `platform/graphics/Region.h` |  | platform/graphics/FloatRoundedRect.h |
| 5 | `platform/graphics/RotationDirection.h` |  | platform/graphics/PathSegmentData.h |
| 5 | `platform/graphics/TabSize.h` |  | platform/graphics/TextRun.h |
| 4 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | platform/graphics/FontDescription.h |
| 4 | `platform/graphics/FloatRoundedRect.h` |  | platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h |
| 4 | `platform/graphics/FontPalette.h` |  | platform/graphics/FontDescription.h |
| 4 | `platform/graphics/FontSelectionAlgorithm.h` |  | platform/graphics/FontDescription.h |
| 4 | `platform/graphics/FontSizeAdjust.h` |  | platform/graphics/FontDescription.h |
| 4 | `platform/graphics/FontTaggedSettings.h` |  | platform/graphics/FontDescription.h, platform/text/TextFlags.h |
| 4 | `platform/graphics/IntSize.h` |  | platform/graphics/DoubleSize.h, platform/graphics/IntPoint.h, platform/graphics/LayoutSize.h |
| 4 | `platform/graphics/PathSegmentData.h` |  | platform/graphics/PathSegment.h |
| 4 | `platform/graphics/TextRun.h` |  | platform/graphics/WidthCache.h |
| 3 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 3 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 3 | `platform/graphics/FontBaseline.h` |  | platform/graphics/FontMetrics.h |
| 3 | `platform/graphics/FontDescription.h` |  | platform/graphics/FontCascadeDescription.h |
| 3 | `platform/graphics/FontFamilySpecificationNull.h` | yes | platform/graphics/FontCascadeDescription.h |
| 3 | `platform/graphics/FontRanges.h` |  | platform/graphics/FontCascadeFonts.h, platform/graphics/FontSelector.h |
| 3 | `platform/graphics/FontSelector.h` |  | platform/graphics/FontCascadeFonts.h |
| 3 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h, platform/graphics/IntRect.h |
| 3 | `platform/graphics/PathElement.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h |
| 3 | `platform/graphics/PathImpl.h` |  | platform/graphics/Path.h |
| 3 | `platform/graphics/PathSegment.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h |
| 3 | `platform/graphics/PlatformPath.h` |  | platform/graphics/Path.h |
| 3 | `platform/graphics/ShouldLocalizeAxisNames.h` |  | platform/graphics/FontPlatformData.h |
| 3 | `platform/graphics/WidthCache.h` |  | platform/graphics/FontCascadeFonts.h |
| 3 | `platform/graphics/WindRule.h` |  | platform/graphics/Path.h |
| 3 | `platform/graphics/cairo/RefPtrCairo.h` | yes | platform/graphics/FontPlatformData.h, platform/graphics/PlatformPath.h |
| 3 | `platform/graphics/cocoa/FontFamilySpecificationCoreText.h` | yes | platform/graphics/FontCascadeDescription.h |
| 3 | `platform/graphics/freetype/FcUniquePtr.h` | yes | platform/graphics/FontPlatformData.h |
| 3 | `platform/graphics/freetype/RefPtrFontconfig.h` | yes | platform/graphics/FontPlatformData.h |
| 3 | `platform/graphics/harfbuzz/HbUniquePtr.h` | yes | platform/graphics/FontPlatformData.h, platform/graphics/opentype/OpenTypeMathData.h |
| 3 | `platform/graphics/opentype/FontMemoryResource.h` | yes | platform/graphics/FontPlatformData.h |
| 3 | `platform/graphics/win/SharedGDIObject.h` | yes | platform/graphics/FontPlatformData.h |
| 3 | `platform/text/TextFlags.h` |  | platform/graphics/FontDescription.h, platform/graphics/FontPlatformData.h, platform/graphics/FontSelectionAlgorithm.h, platform/graphics/GlyphPage.h, platform/graphics/TextRun.h |
| 2 | `platform/graphics/FloatPoint.h` |  | platform/graphics/FloatRect.h, platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/LayoutPoint.h, platform/graphics/PathElement.h |
| 2 | `platform/graphics/FloatRect.h` |  | platform/graphics/FloatRoundedRect.h, platform/graphics/Font.h, platform/graphics/LayoutRect.h, platform/graphics/PathSegmentData.h |
| 2 | `platform/graphics/FloatSegment.h` |  | platform/graphics/FontCascade.h |
| 2 | `platform/graphics/FloatSize.h` |  | platform/graphics/FloatPoint.h, platform/graphics/FloatRoundedRect.h, platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/LayoutSize.h |
| 2 | `platform/graphics/FontCascadeDescription.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontCascadeFonts.h |
| 2 | `platform/graphics/FontCascadeFonts.h` |  | platform/graphics/FontCascade.h |
| 2 | `platform/graphics/FontMetrics.h` |  | platform/graphics/Font.h, platform/graphics/FontSizeAdjust.h |
| 2 | `platform/graphics/FontPlatformData.h` |  | platform/graphics/Font.h |
| 2 | `platform/graphics/Glyph.h` |  | platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/GlyphMetricsMap.h, platform/graphics/GlyphPage.h, platform/graphics/opentype/OpenTypeMathData.h, platform/graphics/opentype/OpenTypeVerticalData.h |
| 2 | `platform/graphics/GlyphBufferMembers.h` |  | platform/graphics/GlyphBuffer.h |
| 2 | `platform/graphics/GlyphMetricsMap.h` |  | platform/graphics/Font.h |
| 2 | `platform/graphics/GlyphPage.h` |  | platform/graphics/Font.h, platform/graphics/FontCascadeFonts.h |
| 2 | `platform/graphics/Path.h` |  | platform/graphics/FontCascade.h, platform/graphics/GlyphMetricsMap.h |
| 2 | `platform/graphics/RenderingResourceIdentifier.h` |  | platform/graphics/Font.h |
| 2 | `platform/graphics/cairo/FontCascadeCairoInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/cocoa/FontCascadeCocoaInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/opentype/OpenTypeMathData.h` | yes | platform/graphics/Font.h |
| 2 | `platform/graphics/opentype/OpenTypeVerticalData.h` | yes | platform/graphics/Font.h |
| 2 | `platform/graphics/skia/FontCascadeSkiaInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/win/FontCascadeWinInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 1 | `platform/graphics/ComposedCharacterClusterTextIterator.h` |  | platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/Font.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontCascadeFonts.h, platform/graphics/FontRanges.h, platform/graphics/GlyphPage.h, platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/FontCascade.h` |  | platform/graphics/WidthIterator.h, platform/graphics/cairo/FontCascadeCairoInlines.h, platform/graphics/cocoa/FontCascadeCocoaInlines.h, platform/graphics/skia/FontCascadeSkiaInlines.h, platform/graphics/win/FontCascadeWinInlines.h |
| 1 | `platform/graphics/FontCascadeInlines.h` |  | platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/GlyphBuffer.h` |  | platform/graphics/Font.h, platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/Latin1TextIterator.h` |  | platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/SurrogatePairAwareTextIterator.h` |  | platform/graphics/WidthIterator.h |
| 1 | `platform/text/TextSpacing.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontDescription.h, platform/graphics/TextRun.h, platform/graphics/WidthCache.h, platform/graphics/WidthIterator.h |
| 1 | `platform/text/WritingMode.h` |  | platform/graphics/TextRun.h, platform/graphics/WidthIterator.h |
| 0 | `platform/graphics/WidthIterator.h` |  | (entry) |

## Boundaries

### external (14)

- `CoreFoundation/CoreFoundation.h` ← 1 file(s)
- `CoreGraphics/CGFont.h` ← 1 file(s)
- `CoreGraphics/CGGeometry.h` ← 1 file(s)
- `CoreGraphics/CGPath.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 2 file(s)
- `fontconfig/fontconfig.h` ← 1 file(s)
- `pal/cf/OTSVGTable.h` ← 1 file(s)
- `pal/spi/cf/CoreTextSPI.h` ← 1 file(s)
- `skia/core/SkFont.h` ← 1 file(s)
- `skia/core/SkTextBlob.h` ← 1 file(s)
- `unicode/umachine.h` ← 2 file(s)
- `unicode/uscript.h` ← 1 file(s)
- `unicode/utf16.h` ← 2 file(s)
- `unicode/utypes.h` ← 1 file(s)

### loader (1)

- `loader/cache/TrustedFonts.h` ← 1 file(s)

### platform (6)

- `platform/BoxExtents.h` ← 2 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/PlatformExportMacros.h` ← 1 file(s)
- `platform/SharedBuffer.h` ← 1 file(s)
- `platform/ios/wak/WebCoreThread.h` ← 1 file(s)
- `platform/win/COMPtr.h` ← 1 file(s)

### wtf (52)

- `wtf/ArgumentCoder.h` ← 3 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/BitVector.h` ← 1 file(s)
- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 1 file(s)
- `wtf/CheckedRef.h` ← 3 file(s)
- `wtf/DataRef.h` ← 1 file(s)
- `wtf/EnumeratedArray.h` ← 1 file(s)
- `wtf/Forward.h` ← 9 file(s)
- `wtf/Function.h` ← 2 file(s)
- `wtf/HashCountedSet.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 2 file(s)
- `wtf/HashMap.h` ← 2 file(s)
- `wtf/HashSet.h` ← 3 file(s)
- `wtf/HashTraits.h` ← 2 file(s)
- `wtf/Hasher.h` ← 7 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/MainThread.h` ← 1 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 7 file(s)
- `wtf/MemoryPressureHandler.h` ← 1 file(s)
- `wtf/ObjectIdentifier.h` ← 1 file(s)
- `wtf/Platform.h` ← 12 file(s)
- `wtf/PointerComparison.h` ← 1 file(s)
- `wtf/Ref.h` ← 4 file(s)
- `wtf/RefCounted.h` ← 5 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/RetainPtr.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 11 file(s)
- `wtf/TZoneMallocInlines.h` ← 2 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 1 file(s)
- `wtf/TriState.h` ← 1 file(s)
- `wtf/UniqueRef.h` ← 1 file(s)
- `wtf/Vector.h` ← 9 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/ZippedRange.h` ← 1 file(s)
- `wtf/cocoa/RuntimeApplicationChecksCocoa.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/CharacterProperties.h` ← 3 file(s)
- `wtf/text/StringCommon.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 1 file(s)
- `wtf/text/StringImpl.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 1 file(s)
- `wtf/text/TextBreakIterator.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 5 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)
- `wtf/text/WYHash.h` ← 1 file(s)
- `wtf/unicode/CharacterNames.h` ← 2 file(s)
- `wtf/win/GDIObject.h` ← 2 file(s)

## Problems

### unresolved (2)

- `CSSValueKeywords.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
