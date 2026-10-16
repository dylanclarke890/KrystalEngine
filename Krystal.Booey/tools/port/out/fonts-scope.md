# Port graph

Entries: `platform/graphics/FontCascade.cpp`, `platform/graphics/WidthIterator.cpp`, `platform/graphics/ComplexTextController.cpp`

- Files to port: **169**
- Boundary `animation`: 1
- Boundary `css`: 1
- Boundary `external`: 29
- Boundary `loader`: 1
- Boundary `platform`: 11
- Boundary `rendering`: 2
- Boundary `wtf`: 79
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `platform/graphics/ColorMatrix.h` |  | platform/graphics/ColorTypes.h |
| 6 | `platform/graphics/ColorModels.h` |  | platform/graphics/ColorTypes.h |
| 6 | `platform/graphics/ColorTransferFunctions.h` |  | platform/graphics/ColorTypes.h |
| 6 | `platform/graphics/CopyImageOptions.h` |  | platform/graphics/ImageBufferBackend.h |
| 6 | `platform/graphics/GraphicsLayerContentsDisplayDelegate.h` |  | platform/graphics/ImageBufferBackend.h |
| 6 | `platform/graphics/GraphicsTypesGL.h` |  | platform/graphics/ImageBufferBackend.h |
| 6 | `platform/graphics/ImageBufferBackendParameters.h` |  | platform/graphics/ImageBufferBackend.h |
| 6 | `platform/graphics/IntSizeHash.h` | yes | platform/graphics/cg/IOSurfacePool.h |
| 6 | `platform/graphics/PixelBuffer.h` |  | platform/graphics/filters/FilterImage.h |
| 6 | `platform/graphics/PixelBufferFormat.h` |  | platform/graphics/ImageBufferAllocator.h, platform/graphics/ImageBufferBackend.h, platform/graphics/PixelBuffer.h |
| 6 | `platform/graphics/PlatformLayer.h` |  | platform/graphics/GraphicsLayerContentsDisplayDelegate.h, platform/graphics/ImageBufferBackend.h |
| 6 | `platform/graphics/cg/IOSurfacePoolIdentifier.h` | yes | platform/graphics/cg/IOSurfacePool.h |
| 6 | `platform/graphics/filters/FilterEffectApplier.h` |  | platform/graphics/filters/FilterEffect.h |
| 5 | `platform/graphics/ColorComponents.h` |  | platform/graphics/ColorTypes.h, platform/graphics/ColorUtilities.h, platform/graphics/cg/GradientRendererCG.h |
| 5 | `platform/graphics/ColorTypes.h` |  | platform/graphics/ColorConversion.h, platform/graphics/ColorInterpolationMethod.h, platform/graphics/ColorSpace.h, platform/graphics/ColorUtilities.h |
| 5 | `platform/graphics/GradientColorStop.h` |  | platform/graphics/GradientColorStops.h |
| 5 | `platform/graphics/ImageBufferAllocator.h` |  | platform/graphics/ImageBuffer.h, platform/graphics/ImageBufferBackend.h |
| 5 | `platform/graphics/ImageBufferBackend.h` |  | platform/graphics/ImageBuffer.h |
| 5 | `platform/graphics/RotationDirection.h` |  | platform/graphics/PathSegmentData.h |
| 5 | `platform/graphics/cg/IOSurfacePool.h` | yes | platform/graphics/ImageBuffer.h |
| 5 | `platform/graphics/cocoa/IOSurface.h` | yes | platform/graphics/ImageBuffer.h, platform/graphics/ImageBufferBackend.h, platform/graphics/cg/IOSurfacePool.h |
| 5 | `platform/graphics/filters/FilterEffect.h` |  | platform/graphics/filters/FilterEffectVector.h |
| 5 | `platform/graphics/filters/FilterEffectGeometry.h` |  | platform/graphics/filters/FilterFunction.h |
| 5 | `platform/graphics/filters/FilterImage.h` |  | platform/graphics/filters/FilterFunction.h |
| 5 | `platform/graphics/filters/FilterImageVector.h` |  | platform/graphics/filters/FilterEffect.h, platform/graphics/filters/FilterEffectApplier.h, platform/graphics/filters/FilterFunction.h |
| 5 | `platform/graphics/filters/FilterRenderingMode.h` |  | platform/graphics/filters/FilterFunction.h |
| 5 | `platform/graphics/filters/FilterStyle.h` |  | platform/graphics/filters/FilterFunction.h |
| 5 | `platform/graphics/re/DynamicContentScalingResourceCache.h` | yes | platform/graphics/ImageBuffer.h |
| 4 | `platform/graphics/ColorConversion.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/ColorInterpolationMethod.h` |  | platform/graphics/Gradient.h, platform/graphics/cg/GradientRendererCG.h |
| 4 | `platform/graphics/ColorSpace.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/ColorUtilities.h` |  | platform/graphics/Color.h |
| 4 | `platform/graphics/ContentsFormat.h` |  | platform/graphics/PixelFormat.h |
| 4 | `platform/graphics/FloatPoint3D.h` |  | platform/graphics/filters/Filter.h |
| 4 | `platform/graphics/GradientColorStops.h` |  | platform/graphics/Gradient.h |
| 4 | `platform/graphics/ImageBuffer.h` |  | platform/graphics/filters/Filter.h, platform/graphics/filters/FilterImage.h |
| 4 | `platform/graphics/LayoutRoundedRect.h` |  | platform/graphics/FloatRoundedRect.h |
| 4 | `platform/graphics/PathSegmentData.h` |  | platform/graphics/PathSegment.h |
| 4 | `platform/graphics/PlatformImage.h` |  | platform/graphics/NativeImage.h |
| 4 | `platform/graphics/Region.h` |  | platform/graphics/FloatRoundedRect.h |
| 4 | `platform/graphics/RenderingResource.h` |  | platform/graphics/Gradient.h, platform/graphics/NativeImage.h, platform/graphics/filters/FilterFunction.h |
| 4 | `platform/graphics/SourceBrushLogicalGradient.h` |  | platform/graphics/SourceBrush.h |
| 4 | `platform/graphics/cg/GradientRendererCG.h` | yes | platform/graphics/Gradient.h |
| 4 | `platform/graphics/controls/ControlStyle.h` |  | platform/graphics/controls/PlatformControl.h |
| 4 | `platform/graphics/filters/FilterEffectVector.h` |  | platform/graphics/filters/Filter.h |
| 4 | `platform/graphics/filters/FilterFunction.h` |  | platform/graphics/filters/Filter.h, platform/graphics/filters/FilterEffect.h |
| 3 | `platform/graphics/AlphaPremultiplication.h` |  | platform/graphics/ColorInterpolationMethod.h, platform/graphics/PixelBufferFormat.h, platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/Color.h` |  | platform/graphics/FontPaletteValues.h, platform/graphics/Gradient.h, platform/graphics/GradientColorStop.h, platform/graphics/GraphicsStyle.h, platform/graphics/GraphicsTypes.h, platform/graphics/SourceBrush.h, platform/graphics/controls/ControlPart.h, platform/graphics/controls/ControlStyle.h, platform/graphics/coretext/DrawGlyphsRecorder.h |
| 3 | `platform/graphics/DecodingOptions.h` |  | platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h |
| 3 | `platform/graphics/DoublePoint.h` |  | platform/graphics/FloatPoint.h, platform/graphics/LayoutPoint.h |
| 3 | `platform/graphics/FloatRoundedRect.h` |  | platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h, platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/FontBaseline.h` |  | platform/graphics/FontMetrics.h |
| 3 | `platform/graphics/FontFamilySpecificationNull.h` | yes | platform/graphics/FontCascadeDescription.h |
| 3 | `platform/graphics/FontFeatureValues.h` |  | platform/graphics/FontCreationContext.h |
| 3 | `platform/graphics/FontPalette.h` |  | platform/graphics/FontDescription.h |
| 3 | `platform/graphics/FontPaletteValues.h` |  | platform/graphics/FontCreationContext.h |
| 3 | `platform/graphics/FontRanges.h` |  | platform/graphics/FontCascadeFonts.h, platform/graphics/FontSelector.h |
| 3 | `platform/graphics/FontSelectionAlgorithm.h` |  | platform/graphics/FontCreationContext.h, platform/graphics/FontDescription.h, platform/graphics/SystemFontDatabase.h, platform/graphics/cocoa/FontDatabase.h |
| 3 | `platform/graphics/FontSizeAdjust.h` |  | platform/graphics/FontDescription.h |
| 3 | `platform/graphics/Gradient.h` |  | platform/graphics/FontPaletteValues.h, platform/graphics/SourceBrush.h, platform/graphics/SourceBrushLogicalGradient.h, platform/graphics/coretext/DrawGlyphsRecorder.h, platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/GraphicsStyle.h` |  | platform/graphics/GraphicsContextState.h, platform/graphics/filters/FilterStyle.h |
| 3 | `platform/graphics/GraphicsTypes.h` |  | platform/graphics/Gradient.h, platform/graphics/GraphicsContextState.h, platform/graphics/ImagePaintingOptions.h, platform/graphics/filters/Filter.h |
| 3 | `platform/graphics/ImageAdapter.h` |  | platform/graphics/Image.h |
| 3 | `platform/graphics/ImageTypes.h` |  | platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h, platform/graphics/NativeImage.h |
| 3 | `platform/graphics/IntSize.h` |  | platform/graphics/DecodingOptions.h, platform/graphics/DoubleSize.h, platform/graphics/ImageBufferBackendParameters.h, platform/graphics/IntPoint.h, platform/graphics/IntSizeHash.h, platform/graphics/LayoutSize.h, platform/graphics/PixelBuffer.h, platform/graphics/SourceImage.h, platform/graphics/cg/IOSurfacePool.h, platform/graphics/cocoa/IOSurface.h, platform/graphics/win/DIBPixelData.h |
| 3 | `platform/graphics/LayoutSize.h` |  | platform/graphics/LayoutPoint.h |
| 3 | `platform/graphics/NativeImage.h` |  | platform/graphics/ImageAdapter.h, platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/PathElement.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h, platform/graphics/PathSegmentData.h |
| 3 | `platform/graphics/PathImpl.h` |  | platform/graphics/Path.h |
| 3 | `platform/graphics/PathSegment.h` |  | platform/graphics/Path.h, platform/graphics/PathImpl.h |
| 3 | `platform/graphics/PixelFormat.h` |  | platform/graphics/ImageBufferBackendParameters.h, platform/graphics/ImageBufferFormat.h, platform/graphics/PixelBufferFormat.h, platform/graphics/cocoa/IOSurface.h |
| 3 | `platform/graphics/PlatformColorSpace.h` |  | platform/graphics/DestinationColorSpace.h |
| 3 | `platform/graphics/PlatformDynamicRangeLimit.h` |  | platform/graphics/ImagePaintingOptions.h |
| 3 | `platform/graphics/PlatformPath.h` |  | platform/graphics/Path.h |
| 3 | `platform/graphics/ShouldLocalizeAxisNames.h` |  | platform/graphics/FontPlatformData.h, platform/graphics/cocoa/FontCacheCoreText.h |
| 3 | `platform/graphics/SourceBrush.h` |  | platform/graphics/GraphicsContextState.h |
| 3 | `platform/graphics/SourceImage.h` |  | platform/graphics/Pattern.h |
| 3 | `platform/graphics/SystemFontDatabase.h` |  | platform/graphics/cocoa/SystemFontDatabaseCoreText.h |
| 3 | `platform/graphics/SystemImage.h` |  | platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/WidthCache.h` |  | platform/graphics/FontCascadeFonts.h |
| 3 | `platform/graphics/WindRule.h` |  | platform/graphics/GraphicsContextState.h, platform/graphics/GraphicsTypes.h, platform/graphics/Path.h |
| 3 | `platform/graphics/cairo/RefPtrCairo.h` | yes | platform/graphics/FontPlatformData.h, platform/graphics/ImageBufferBackend.h, platform/graphics/PlatformImage.h, platform/graphics/PlatformPath.h |
| 3 | `platform/graphics/cocoa/FontFamilySpecificationCoreText.h` | yes | platform/graphics/FontCascadeDescription.h |
| 3 | `platform/graphics/controls/ControlFactory.h` |  | platform/graphics/controls/ControlPart.h |
| 3 | `platform/graphics/controls/PlatformControl.h` |  | platform/graphics/controls/ControlPart.h |
| 3 | `platform/graphics/coretext/DrawGlyphsRecorder.h` | yes | platform/graphics/displaylists/DisplayListRecorder.h |
| 3 | `platform/graphics/displaylists/DisplayListItem.h` |  | platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/filters/Filter.h` |  | platform/graphics/displaylists/DisplayListItems.h |
| 3 | `platform/graphics/freetype/FcUniquePtr.h` |  | platform/graphics/FontPlatformData.h, platform/graphics/freetype/FontSetCache.h |
| 3 | `platform/graphics/freetype/RefPtrFontconfig.h` |  | platform/graphics/FontPlatformData.h, platform/graphics/freetype/FontSetCache.h |
| 3 | `platform/graphics/harfbuzz/HbUniquePtr.h` | yes | platform/graphics/FontPlatformData.h, platform/graphics/opentype/OpenTypeMathData.h, platform/graphics/skia/SkiaHarfBuzzFont.h |
| 3 | `platform/graphics/opentype/FontMemoryResource.h` | yes | platform/graphics/FontPlatformData.h |
| 3 | `platform/graphics/skia/SkiaHarfBuzzFont.h` | yes | platform/graphics/skia/SkiaHarfBuzzFontCache.h |
| 3 | `platform/graphics/transforms/AffineTransform.h` |  | platform/graphics/ImageOrientation.h, platform/graphics/Pattern.h, platform/graphics/coretext/DrawGlyphsRecorder.h |
| 3 | `platform/graphics/win/DIBPixelData.h` | yes | platform/graphics/PlatformGraphicsContext.h |
| 3 | `platform/graphics/win/SharedGDIObject.h` | yes | platform/graphics/FontPlatformData.h |
| 2 | `platform/graphics/DashArray.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/displaylists/DisplayListItems.h |
| 2 | `platform/graphics/DestinationColorSpace.h` |  | platform/graphics/Color.h, platform/graphics/GraphicsContext.h, platform/graphics/Image.h, platform/graphics/ImageBufferBackend.h, platform/graphics/ImageBufferBackendParameters.h, platform/graphics/PixelBufferFormat.h, platform/graphics/cg/GradientRendererCG.h, platform/graphics/cocoa/IOSurface.h, platform/graphics/filters/FilterEffect.h |
| 2 | `platform/graphics/DoubleSize.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatSize.h |
| 2 | `platform/graphics/FloatPoint.h` |  | platform/graphics/ComplexTextController.h, platform/graphics/FloatPoint3D.h, platform/graphics/FloatRect.h, platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/Gradient.h, platform/graphics/LayoutPoint.h, platform/graphics/PathElement.h, platform/graphics/transforms/AffineTransform.h |
| 2 | `platform/graphics/FloatSegment.h` |  | platform/graphics/FontCascade.h, platform/graphics/GraphicsContext.h |
| 2 | `platform/graphics/FontCascadeCache.h` |  | platform/graphics/FontCache.h, platform/graphics/cocoa/FontFamilySpecificationCoreTextCache.h, platform/graphics/freetype/FontSetCache.h |
| 2 | `platform/graphics/FontCascadeDescription.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontCascadeFonts.h |
| 2 | `platform/graphics/FontCascadeFonts.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontCascadeCache.h |
| 2 | `platform/graphics/FontCreationContext.h` |  | platform/graphics/FontCache.h |
| 2 | `platform/graphics/FontDescription.h` |  | platform/graphics/FontCache.h, platform/graphics/FontCascadeCache.h, platform/graphics/FontCascadeDescription.h, platform/graphics/cocoa/FontCacheCoreText.h, platform/graphics/cocoa/SystemFontDatabaseCoreText.h, platform/graphics/freetype/FontSetCache.h |
| 2 | `platform/graphics/FontMetrics.h` |  | platform/graphics/Font.h, platform/graphics/FontSizeAdjust.h |
| 2 | `platform/graphics/FontPlatformData.h` |  | platform/graphics/Font.h, platform/graphics/FontCache.h |
| 2 | `platform/graphics/FontSelector.h` |  | platform/graphics/FontCache.h, platform/graphics/FontCascadeFonts.h |
| 2 | `platform/graphics/FontTaggedSettings.h` |  | platform/graphics/FontCache.h, platform/graphics/FontCascadeCache.h, platform/graphics/FontCreationContext.h, platform/graphics/FontDescription.h, platform/text/TextFlags.h |
| 2 | `platform/graphics/Glyph.h` |  | platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/GlyphMetricsMap.h, platform/graphics/GlyphPage.h, platform/graphics/opentype/OpenTypeMathData.h, platform/graphics/opentype/OpenTypeVerticalData.h |
| 2 | `platform/graphics/GlyphBufferMembers.h` |  | platform/graphics/GlyphBuffer.h |
| 2 | `platform/graphics/GlyphMetricsMap.h` |  | platform/graphics/Font.h |
| 2 | `platform/graphics/GlyphPage.h` |  | platform/graphics/Font.h, platform/graphics/FontCascadeFonts.h |
| 2 | `platform/graphics/GraphicsContextState.h` |  | platform/graphics/GraphicsContext.h |
| 2 | `platform/graphics/GraphicsContextStateSaver.h` |  | platform/graphics/GraphicsContext.h |
| 2 | `platform/graphics/Image.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/displaylists/DisplayListItems.h, platform/graphics/displaylists/DisplayListRecorder.h |
| 2 | `platform/graphics/ImageBufferFormat.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/ImageBuffer.h, platform/graphics/ImageBufferBackendParameters.h |
| 2 | `platform/graphics/ImageOrientation.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/Image.h, platform/graphics/ImagePaintingOptions.h |
| 2 | `platform/graphics/ImagePaintingOptions.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/Image.h, platform/graphics/ImageBufferBackend.h |
| 2 | `platform/graphics/IntPoint.h` |  | platform/graphics/DoublePoint.h, platform/graphics/FloatPoint.h, platform/graphics/FloatSize.h, platform/graphics/IntRect.h |
| 2 | `platform/graphics/IntRect.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/ImageBufferBackend.h, platform/graphics/LayoutRect.h, platform/graphics/Region.h, platform/graphics/filters/FilterImage.h, platform/graphics/win/DIBPixelData.h |
| 2 | `platform/graphics/LayoutPoint.h` |  | platform/graphics/LayoutRect.h |
| 2 | `platform/graphics/Path.h` |  | platform/graphics/FontCascade.h, platform/graphics/GlyphMetricsMap.h |
| 2 | `platform/graphics/Pattern.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/SourceBrush.h, platform/graphics/coretext/DrawGlyphsRecorder.h |
| 2 | `platform/graphics/PlatformGraphicsContext.h` |  | platform/graphics/GraphicsContext.h |
| 2 | `platform/graphics/RenderingMode.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/ImageBuffer.h, platform/graphics/ImageBufferAllocator.h, platform/graphics/ImageBufferBackend.h, platform/graphics/ImageBufferBackendParameters.h, platform/graphics/filters/Filter.h, platform/graphics/filters/FilterImage.h |
| 2 | `platform/graphics/RenderingResourceIdentifier.h` |  | platform/graphics/Font.h, platform/graphics/ImageBuffer.h, platform/graphics/RenderingResource.h, platform/graphics/SourceImage.h, platform/graphics/displaylists/DisplayListItem.h |
| 2 | `platform/graphics/SystemFallbackFontCache.h` |  | platform/graphics/FontCache.h |
| 2 | `platform/graphics/TabSize.h` |  | platform/graphics/TextRun.h |
| 2 | `platform/graphics/cairo/FontCascadeCairoInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/cocoa/FontCacheCoreText.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/cocoa/FontCascadeCocoaInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/cocoa/FontDatabase.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/cocoa/FontFamilySpecificationCoreTextCache.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/cocoa/SystemFontDatabaseCoreText.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/controls/ControlPart.h` |  | platform/graphics/GraphicsContext.h, platform/graphics/displaylists/DisplayListItems.h |
| 2 | `platform/graphics/displaylists/DisplayListItems.h` |  | platform/graphics/displaylists/DisplayList.h, platform/graphics/displaylists/DisplayListRecorder.h |
| 2 | `platform/graphics/displaylists/DisplayListRecorder.h` |  | platform/graphics/displaylists/DisplayListRecorderImpl.h |
| 2 | `platform/graphics/freetype/FontSetCache.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/opentype/OpenTypeMathData.h` | yes | platform/graphics/Font.h |
| 2 | `platform/graphics/opentype/OpenTypeVerticalData.h` | yes | platform/graphics/Font.h |
| 2 | `platform/graphics/skia/FontCascadeSkiaInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/graphics/skia/SkiaHarfBuzzFontCache.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/skia/SkiaSystemFallbackFontCache.h` | yes | platform/graphics/FontCache.h |
| 2 | `platform/graphics/win/FontCascadeWinInlines.h` | yes | platform/graphics/FontCascadeInlines.h |
| 2 | `platform/text/TextFlags.h` |  | platform/graphics/FontDescription.h, platform/graphics/FontPlatformData.h, platform/graphics/FontSelectionAlgorithm.h, platform/graphics/GlyphPage.h, platform/graphics/SystemFallbackFontCache.h, platform/graphics/TextRun.h, platform/graphics/cocoa/FontDatabase.h, platform/graphics/coretext/DrawGlyphsRecorder.h, platform/graphics/displaylists/DisplayListItems.h, platform/graphics/displaylists/DisplayListRecorder.h |
| 2 | `platform/text/WritingMode.h` |  | platform/graphics/TextRun.h, platform/graphics/WidthIterator.h |
| 1 | `platform/graphics/ComplexTextController.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/FontCascade.cpp |
| 1 | `platform/graphics/ComposedCharacterClusterTextIterator.h` |  | platform/graphics/WidthIterator.cpp |
| 1 | `platform/graphics/FloatRect.h` |  | platform/graphics/FloatRoundedRect.h, platform/graphics/Font.h, platform/graphics/FontCascade.cpp, platform/graphics/GraphicsContext.h, platform/graphics/Image.h, platform/graphics/ImageBufferBackend.h, platform/graphics/LayoutRect.h, platform/graphics/PathSegmentData.h, platform/graphics/controls/PlatformControl.h, platform/graphics/filters/Filter.h, platform/graphics/filters/FilterEffectGeometry.h, platform/graphics/filters/FilterFunction.h, platform/graphics/filters/FilterImage.h, platform/graphics/filters/FilterStyle.h |
| 1 | `platform/graphics/FloatSize.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/FloatPoint.h, platform/graphics/FloatRoundedRect.h, platform/graphics/GlyphBuffer.h, platform/graphics/GlyphBufferMembers.h, platform/graphics/GraphicsStyle.h, platform/graphics/GraphicsTypes.h, platform/graphics/ImageOrientation.h, platform/graphics/LayoutSize.h, platform/graphics/transforms/AffineTransform.h |
| 1 | `platform/graphics/Font.h` |  | platform/graphics/FontCascade.h, platform/graphics/FontCascadeFonts.h, platform/graphics/FontRanges.h, platform/graphics/GlyphPage.h, platform/graphics/WidthIterator.cpp, platform/graphics/displaylists/DisplayListItems.h |
| 1 | `platform/graphics/FontCache.h` |  | platform/graphics/FontCascade.cpp |
| 1 | `platform/graphics/FontCascade.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/FontCascade.cpp, platform/graphics/GraphicsContext.h, platform/graphics/WidthIterator.cpp, platform/graphics/cairo/FontCascadeCairoInlines.h, platform/graphics/cocoa/FontCascadeCocoaInlines.h, platform/graphics/skia/FontCascadeSkiaInlines.h, platform/graphics/win/FontCascadeWinInlines.h |
| 1 | `platform/graphics/FontCascadeInlines.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/FontCascade.cpp, platform/graphics/WidthIterator.cpp |
| 1 | `platform/graphics/GlyphBuffer.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/ComplexTextController.h, platform/graphics/Font.h, platform/graphics/FontCascade.cpp, platform/graphics/WidthIterator.cpp, platform/graphics/WidthIterator.h, platform/graphics/displaylists/DisplayListItems.h |
| 1 | `platform/graphics/GraphicsContext.h` |  | platform/graphics/FontCascade.cpp, platform/graphics/GraphicsContextStateSaver.h, platform/graphics/coretext/DrawGlyphsRecorder.h, platform/graphics/displaylists/DisplayListItems.h, platform/graphics/displaylists/DisplayListRecorder.h |
| 1 | `platform/graphics/Latin1TextIterator.h` |  | platform/graphics/WidthIterator.cpp |
| 1 | `platform/graphics/LayoutRect.h` |  | platform/graphics/FontCascade.cpp, platform/graphics/LayoutRoundedRect.h |
| 1 | `platform/graphics/SurrogatePairAwareTextIterator.h` |  | platform/graphics/WidthIterator.cpp |
| 1 | `platform/graphics/TextRun.h` |  | platform/graphics/ComplexTextController.cpp, platform/graphics/FontCascade.cpp, platform/graphics/WidthCache.h |
| 1 | `platform/graphics/WidthIterator.h` |  | platform/graphics/FontCascade.cpp, platform/graphics/WidthIterator.cpp |
| 1 | `platform/graphics/displaylists/DisplayList.h` |  | platform/graphics/FontCascade.cpp, platform/graphics/displaylists/DisplayListRecorder.h |
| 1 | `platform/graphics/displaylists/DisplayListRecorderImpl.h` |  | platform/graphics/FontCascade.cpp |
| 1 | `platform/text/TextSpacing.h` |  | platform/graphics/ComplexTextController.h, platform/graphics/FontCascade.h, platform/graphics/FontDescription.h, platform/graphics/TextRun.h, platform/graphics/WidthCache.h, platform/graphics/WidthIterator.cpp |
| 0 | `platform/graphics/ComplexTextController.cpp` |  | (entry) |
| 0 | `platform/graphics/FontCascade.cpp` |  | (entry) |
| 0 | `platform/graphics/WidthIterator.cpp` |  | (entry) |

## Boundaries

### animation (1)

- `animation/CompositeOperation.h` ← 1 file(s)

### css (1)

- `css/values/primitives/CSSPrimitiveNumericRange.h` ← 1 file(s)

### external (29)

- `CoreFoundation/CoreFoundation.h` ← 1 file(s)
- `CoreGraphics/CGFont.h` ← 1 file(s)
- `CoreGraphics/CGGeometry.h` ← 1 file(s)
- `CoreGraphics/CGPath.h` ← 1 file(s)
- `CoreGraphics/CoreGraphics.h` ← 6 file(s)
- `CoreText/CTFont.h` ← 1 file(s)
- `CoreText/CoreText.h` ← 4 file(s)
- `JavaScriptCore/Forward.h` ← 1 file(s)
- `fontconfig/fontconfig.h` ← 1 file(s)
- `pal/cf/OTSVGTable.h` ← 1 file(s)
- `pal/spi/cf/CoreTextSPI.h` ← 3 file(s)
- `pal/spi/cg/CoreGraphicsSPI.h` ← 1 file(s)
- `skia/core/SkColor.h` ← 1 file(s)
- `skia/core/SkColorSpace.h` ← 1 file(s)
- `skia/core/SkData.h` ← 1 file(s)
- `skia/core/SkFont.h` ← 2 file(s)
- `skia/core/SkFontMgr.h` ← 1 file(s)
- `skia/core/SkFontStyle.h` ← 1 file(s)
- `skia/core/SkImage.h` ← 1 file(s)
- `skia/core/SkPicture.h` ← 1 file(s)
- `skia/core/SkPictureRecorder.h` ← 1 file(s)
- `skia/core/SkShader.h` ← 2 file(s)
- `skia/core/SkTextBlob.h` ← 1 file(s)
- `skia/core/SkTypeface.h` ← 2 file(s)
- `unicode/ubrk.h` ← 1 file(s)
- `unicode/umachine.h` ← 2 file(s)
- `unicode/uscript.h` ← 1 file(s)
- `unicode/utf16.h` ← 3 file(s)
- `unicode/utypes.h` ← 1 file(s)

### loader (1)

- `loader/cache/TrustedFonts.h` ← 1 file(s)

### platform (11)

- `platform/BoxExtents.h` ← 3 file(s)
- `platform/FloatConversion.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 2 file(s)
- `platform/PlatformExportMacros.h` ← 4 file(s)
- `platform/PlatformScreen.h` ← 1 file(s)
- `platform/ProcessIdentity.h` ← 2 file(s)
- `platform/SharedBuffer.h` ← 2 file(s)
- `platform/StyleAppearance.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/ios/wak/WebCoreThread.h` ← 1 file(s)
- `platform/win/COMPtr.h` ← 1 file(s)

### rendering (2)

- `rendering/RenderBlock.h` ← 1 file(s)
- `rendering/RenderText.h` ← 1 file(s)

### wtf (79)

- `wtf/AbstractCanMakeCheckedPtr.h` ← 1 file(s)
- `wtf/ArgumentCoder.h` ← 6 file(s)
- `wtf/Assertions.h` ← 3 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/BitVector.h` ← 1 file(s)
- `wtf/Box.h` ← 1 file(s)
- `wtf/CheckedArithmetic.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 3 file(s)
- `wtf/CheckedRef.h` ← 7 file(s)
- `wtf/Compiler.h` ← 3 file(s)
- `wtf/CrossThreadCopier.h` ← 1 file(s)
- `wtf/DataRef.h` ← 1 file(s)
- `wtf/Deque.h` ← 1 file(s)
- `wtf/EnumeratedArray.h` ← 2 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/Forward.h` ← 23 file(s)
- `wtf/Function.h` ← 4 file(s)
- `wtf/GetPtr.h` ← 1 file(s)
- `wtf/HashCountedSet.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 5 file(s)
- `wtf/HashMap.h` ← 11 file(s)
- `wtf/HashSet.h` ← 4 file(s)
- `wtf/HashTraits.h` ← 7 file(s)
- `wtf/Hasher.h` ← 13 file(s)
- `wtf/JSONValues.h` ← 2 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Lock.h` ← 3 file(s)
- `wtf/MainThread.h` ← 2 file(s)
- `wtf/Markable.h` ← 3 file(s)
- `wtf/MathExtras.h` ← 11 file(s)
- `wtf/MemoryPressureHandler.h` ← 1 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 4 file(s)
- `wtf/ObjectIdentifier.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 8 file(s)
- `wtf/Platform.h` ← 22 file(s)
- `wtf/PointerComparison.h` ← 4 file(s)
- `wtf/RecursiveLockAdapter.h` ← 1 file(s)
- `wtf/Ref.h` ← 8 file(s)
- `wtf/RefCounted.h` ← 13 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 2 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 10 file(s)
- `wtf/RetainPtr.h` ← 10 file(s)
- `wtf/RobinHoodHashMap.h` ← 2 file(s)
- `wtf/RobinHoodHashSet.h` ← 2 file(s)
- `wtf/RunLoop.h` ← 1 file(s)
- `wtf/SortedArrayMap.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 5 file(s)
- `wtf/TZoneMalloc.h` ← 32 file(s)
- `wtf/TZoneMallocInlines.h` ← 7 file(s)
- `wtf/ThreadSafeRefCounted.h` ← 8 file(s)
- `wtf/ThreadSafeWeakPtr.h` ← 2 file(s)
- `wtf/TriState.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/UniqueRef.h` ← 4 file(s)
- `wtf/Variant.h` ← 1 file(s)
- `wtf/Vector.h` ← 24 file(s)
- `wtf/WeakHashSet.h` ← 2 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/WeakRef.h` ← 2 file(s)
- `wtf/WorkQueue.h` ← 1 file(s)
- `wtf/ZippedRange.h` ← 1 file(s)
- `wtf/cocoa/RuntimeApplicationChecksCocoa.h` ← 1 file(s)
- `wtf/spi/cocoa/IOSurfaceSPI.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 8 file(s)
- `wtf/text/AtomStringHash.h` ← 3 file(s)
- `wtf/text/CharacterProperties.h` ← 4 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringCommon.h` ← 1 file(s)
- `wtf/text/StringHash.h` ← 2 file(s)
- `wtf/text/StringImpl.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 1 file(s)
- `wtf/text/TextBreakIterator.h` ← 2 file(s)
- `wtf/text/TextStream.h` ← 6 file(s)
- `wtf/text/WTFString.h` ← 9 file(s)
- `wtf/text/WYHash.h` ← 1 file(s)
- `wtf/unicode/CharacterNames.h` ← 3 file(s)
- `wtf/win/GDIObject.h` ← 2 file(s)

## Problems

### unresolved (2)

- `CSSValueKeywords.h` ← 1 file(s)
- `WebKitFontFamilyNames.h` ← 1 file(s)
