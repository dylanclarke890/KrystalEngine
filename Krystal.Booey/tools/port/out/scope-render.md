# Port graph

Entries: `rendering/RenderObject.h`, `rendering/RenderElement.h`, `rendering/RenderBox.h`, `rendering/RenderBlockFlow.h`, `rendering/RenderText.h`, `rendering/RenderView.h`, `rendering/updating/RenderTreeBuilder.h`

- Files to port: **274**
- Boundary `accessibility`: 2
- Boundary `css`: 3
- Boundary `dom`: 23
- Boundary `editing`: 7
- Boundary `external`: 5
- Boundary `html`: 18
- Boundary `loader`: 4
- Boundary `mathml`: 3
- Boundary `page`: 20
- Boundary `platform`: 66
- Boundary `style`: 40
- Boundary `svg`: 9
- Boundary `wtf`: 62
- Ambiguous: 1
- Unresolved: 8

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `layout/formattingContexts/FormattingContext.h` |  | layout/floats/FloatingContext.h |
| 6 | `layout/formattingContexts/inline/InlineContentBreaker.h` |  | layout/formattingContexts/inline/AbstractLineBuilder.h |
| 5 | `layout/floats/FloatingContext.h` |  | layout/formattingContexts/inline/InlineLineBuilder.h |
| 5 | `layout/floats/PlacedFloats.h` |  | layout/floats/FloatingContext.h, layout/formattingContexts/block/BlockLayoutState.h, layout/formattingContexts/inline/LineLayoutResult.h |
| 5 | `layout/formattingContexts/inline/AbstractLineBuilder.h` |  | layout/formattingContexts/inline/InlineLineBuilder.h |
| 5 | `layout/formattingContexts/inline/InlineLevelBox.h` |  | layout/formattingContexts/inline/InlineLineBox.h |
| 4 | `layout/formattingContexts/block/BlockLayoutState.h` |  | layout/formattingContexts/inline/InlineFormattingUtils.h, layout/formattingContexts/inline/InlineLayoutState.h, layout/integration/LayoutIntegrationUtils.h |
| 4 | `layout/formattingContexts/inline/AvailableLineWidthOverride.h` |  | layout/formattingContexts/inline/InlineLayoutState.h |
| 4 | `layout/formattingContexts/inline/InlineContentCache.h` |  | layout/formattingContexts/inline/IntrinsicWidthHandler.h |
| 4 | `layout/formattingContexts/inline/InlineLineBox.h` |  | layout/formattingContexts/inline/InlineQuirks.h |
| 4 | `layout/formattingContexts/inline/InlineLineBuilder.h` |  | layout/formattingContexts/inline/InlineFormattingUtils.h |
| 4 | `layout/formattingContexts/inline/InlineRect.h` |  | layout/formattingContexts/inline/InlineLevelBox.h, layout/formattingContexts/inline/InlineLineBox.h, layout/formattingContexts/inline/display/InlineDisplayBox.h, layout/formattingContexts/inline/display/InlineDisplayLine.h |
| 4 | `layout/formattingContexts/inline/LineLayoutResult.h` |  | layout/formattingContexts/inline/AbstractLineBuilder.h, layout/formattingContexts/inline/InlineContentCache.h, layout/formattingContexts/inline/IntrinsicWidthHandler.h |
| 4 | `layout/formattingContexts/inline/display/InlineDisplayLine.h` |  | layout/formattingContexts/inline/display/InlineDisplayContent.h |
| 4 | `rendering/BaselineAlignment.h` |  | rendering/GridBaselineAlignment.h |
| 4 | `rendering/BidiRun.h` |  | rendering/LegacyInlineIterator.h |
| 4 | `rendering/GridLayoutFunctions.h` |  | rendering/GridBaselineAlignment.h |
| 4 | `rendering/mathml/MathOperator.h` | yes | rendering/mathml/RenderMathMLOperator.h |
| 4 | `rendering/mathml/RenderMathMLToken.h` | yes | rendering/mathml/RenderMathMLOperator.h |
| 4 | `rendering/style/GridSpan.h` |  | rendering/style/GridArea.h |
| 3 | `layout/MarginTypes.h` |  | layout/LayoutUnits.h |
| 3 | `layout/formattingContexts/FormattingConstraints.h` |  | layout/formattingContexts/FormattingContext.h, layout/formattingContexts/inline/AbstractLineBuilder.h, layout/formattingContexts/inline/InlineContentBreaker.h, layout/formattingContexts/inline/InlineContentCache.h, layout/formattingContexts/inline/InlineFormattingConstraints.h, layout/integration/LayoutIntegrationBoxGeometryUpdater.h |
| 3 | `layout/formattingContexts/inline/InlineFormattingUtils.h` |  | layout/formattingContexts/inline/InlineFormattingContext.h |
| 3 | `layout/formattingContexts/inline/InlineLayoutState.h` |  | layout/formattingContexts/inline/AbstractLineBuilder.h, layout/formattingContexts/inline/InlineFormattingContext.h |
| 3 | `layout/formattingContexts/inline/InlineLineTypes.h` |  | layout/formattingContexts/inline/AbstractLineBuilder.h, layout/formattingContexts/inline/InlineFormattingUtils.h, layout/formattingContexts/inline/InlineLine.h, layout/formattingContexts/inline/LineLayoutResult.h, layout/formattingContexts/inline/invalidation/InlineDamage.h |
| 3 | `layout/formattingContexts/inline/InlineQuirks.h` |  | layout/formattingContexts/inline/InlineFormattingContext.h |
| 3 | `layout/formattingContexts/inline/InlineTextItem.h` |  | layout/formattingContexts/inline/InlineLine.h |
| 3 | `layout/formattingContexts/inline/IntrinsicWidthHandler.h` |  | layout/formattingContexts/inline/InlineFormattingContext.h |
| 3 | `layout/formattingContexts/inline/display/InlineDisplayBox.h` |  | layout/formattingContexts/inline/InlineLine.h, layout/formattingContexts/inline/display/InlineDisplayBoxInlines.h, layout/formattingContexts/inline/display/InlineDisplayContent.h |
| 3 | `layout/formattingContexts/inline/display/InlineDisplayBoxInlines.h` |  | layout/integration/inline/InlineIteratorBoxModernPathInlines.h |
| 3 | `layout/formattingContexts/inline/display/InlineDisplayContent.h` |  | layout/formattingContexts/inline/InlineContentCache.h, layout/formattingContexts/inline/InlineFormattingContext.h, layout/formattingContexts/inline/invalidation/InlineDamage.h, layout/integration/inline/LayoutIntegrationInlineContent.h |
| 3 | `layout/integration/LayoutIntegrationUtils.h` |  | layout/formattingContexts/inline/InlineFormattingContext.h |
| 3 | `layout/integration/inline/InlineIteratorBoxLegacyPath.h` |  | layout/integration/inline/InlineIteratorBox.h, layout/integration/inline/InlineIteratorBoxModernPath.h, layout/integration/inline/InlineIteratorLineBoxLegacyPath.h |
| 3 | `layout/integration/inline/InlineIteratorBoxModernPath.h` |  | layout/integration/inline/InlineIteratorBox.h, layout/integration/inline/InlineIteratorBoxModernPathInlines.h, layout/integration/inline/InlineIteratorLineBoxModernPath.h |
| 3 | `layout/integration/inline/LayoutIntegrationInlineContent.h` |  | layout/integration/inline/InlineIteratorBoxModernPath.h, layout/integration/inline/InlineIteratorLineBoxLegacyPath.h, layout/integration/inline/InlineIteratorLineBoxModernPath.h |
| 3 | `rendering/GridBaselineAlignment.h` |  | rendering/GridTrackSizingAlgorithm.h |
| 3 | `rendering/GridLayoutState.h` |  | rendering/GridTrackSizingAlgorithm.h |
| 3 | `rendering/LegacyInlineIterator.h` |  | rendering/LegacyInlineIteratorInlines.h, rendering/line/LineBreaker.h |
| 3 | `rendering/RegionContext.h` |  | rendering/AccessibilityRegionContext.h, rendering/EventRegion.h |
| 3 | `rendering/RenderFrameBase.h` |  | rendering/RenderIFrame.h |
| 3 | `rendering/mathml/RenderMathMLOperator.h` | yes | rendering/mathml/RenderMathMLFencedOperator.h |
| 3 | `rendering/style/BorderValue.h` |  | rendering/style/BorderData.h, rendering/style/CollapsedBorderValue.h |
| 3 | `rendering/style/GridArea.h` |  | rendering/GridMasonryLayout.h |
| 3 | `rendering/style/StyleBorderImageData.h` |  | rendering/style/BorderData.h |
| 3 | `rendering/svg/SVGTextMetrics.h` |  | rendering/svg/SVGTextLayoutAttributes.h |
| 3 | `rendering/svg/SVGTextMetricsBuilder.h` |  | rendering/svg/SVGTextLayoutAttributesBuilder.h |
| 3 | `rendering/svg/legacy/LegacyRenderSVGHiddenContainer.h` |  | rendering/svg/legacy/LegacyRenderSVGResourceContainer.h |
| 3 | `rendering/svg/legacy/LegacyRenderSVGResource.h` |  | rendering/svg/legacy/LegacyRenderSVGResourceContainer.h |
| 2 | `layout/LayoutUnits.h` |  | layout/formattingContexts/FormattingContext.h, layout/formattingContexts/inline/InlineContentBreaker.h, layout/formattingContexts/inline/InlineItem.h, layout/formattingContexts/inline/InlineLevelBox.h, layout/formattingContexts/inline/InlineLineTypes.h, layout/formattingContexts/inline/InlineRect.h, layout/formattingContexts/inline/LineLayoutResult.h, layout/formattingContexts/inline/text/TextUtil.h, layout/layouttree/LayoutBox.h, layout/layouttree/LayoutBoxGeometry.h, layout/layouttree/LayoutGeometryRect.h |
| 2 | `layout/formattingContexts/inline/InlineFormattingConstraints.h` |  | layout/formattingContexts/inline/InlineFormattingContext.h, layout/integration/LayoutIntegrationBoxGeometryUpdater.h, layout/integration/inline/LayoutIntegrationLineLayout.h |
| 2 | `layout/formattingContexts/inline/InlineFormattingContext.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h |
| 2 | `layout/formattingContexts/inline/InlineItem.h` |  | layout/formattingContexts/inline/InlineContentCache.h, layout/formattingContexts/inline/InlineLine.h, layout/formattingContexts/inline/InlineTextItem.h, layout/formattingContexts/inline/text/TextUtil.h |
| 2 | `layout/formattingContexts/inline/InlineLine.h` |  | layout/formattingContexts/inline/AbstractLineBuilder.h, layout/formattingContexts/inline/InlineFormattingUtils.h, layout/formattingContexts/inline/InlineLineBox.h, layout/formattingContexts/inline/LineLayoutResult.h, layout/formattingContexts/inline/text/TextUtil.h |
| 2 | `layout/formattingContexts/inline/invalidation/InlineDamage.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h |
| 2 | `layout/integration/LayoutIntegrationBoxGeometryUpdater.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h |
| 2 | `layout/integration/LayoutIntegrationBoxTreeUpdater.h` |  | layout/integration/LayoutIntegrationBoxGeometryUpdater.h, layout/integration/inline/LayoutIntegrationLineLayout.h |
| 2 | `layout/integration/inline/InlineIteratorBox.h` |  | layout/integration/inline/InlineIteratorBoxInlines.h, layout/integration/inline/InlineIteratorInlineBox.h, layout/integration/inline/InlineIteratorLineBoxInlines.h, layout/integration/inline/InlineIteratorTextBox.h, layout/integration/inline/LineSelection.h |
| 2 | `layout/integration/inline/InlineIteratorBoxModernPathInlines.h` |  | layout/integration/inline/InlineIteratorBoxInlines.h, layout/integration/inline/InlineIteratorTextBoxInlines.h |
| 2 | `layout/integration/inline/InlineIteratorLineBoxLegacyPath.h` |  | layout/integration/inline/InlineIteratorLineBox.h |
| 2 | `layout/integration/inline/InlineIteratorLineBoxModernPath.h` |  | layout/integration/inline/InlineIteratorLineBox.h |
| 2 | `layout/layouttree/LayoutGeometryRect.h` |  | layout/layouttree/LayoutBoxGeometry.h |
| 2 | `rendering/AccessibilityRegionContext.h` |  | rendering/PaintInfo.h |
| 2 | `rendering/CaretRectComputation.h` |  | rendering/RenderBlockFlowInlines.h |
| 2 | `rendering/ClipRect.h` |  | rendering/LayerFragment.h, rendering/RenderLayer.h |
| 2 | `rendering/EventRegion.h` |  | rendering/PaintInfo.h |
| 2 | `rendering/GapRects.h` |  | rendering/RenderBlock.h, rendering/RenderSelectionGeometry.h |
| 2 | `rendering/GlyphDisplayListCacheRemoval.h` |  | layout/formattingContexts/inline/display/InlineDisplayBox.h, rendering/LegacyInlineTextBox.h |
| 2 | `rendering/Grid.h` |  | rendering/GridTrackSizingAlgorithm.h, rendering/RenderGrid.h |
| 2 | `rendering/GridMasonryLayout.h` |  | rendering/RenderGrid.h |
| 2 | `rendering/GridTrackSizingAlgorithm.h` |  | rendering/GridMasonryLayout.h, rendering/RenderGrid.h |
| 2 | `rendering/HitTestSource.h` |  | rendering/HitTestRequest.h |
| 2 | `rendering/LayerAncestorClippingStack.h` |  | rendering/RenderLayerCompositor.h |
| 2 | `rendering/LayerFragment.h` |  | rendering/RenderFragmentContainer.h, rendering/RenderFragmentedFlow.h, rendering/RenderLayer.h, rendering/RenderMultiColumnSet.h |
| 2 | `rendering/LegacyInlineBox.h` |  | rendering/LegacyInlineFlowBox.h, rendering/LegacyInlineTextBox.h |
| 2 | `rendering/LegacyInlineFlowBox.h` |  | rendering/LegacyLineLayout.h, rendering/LegacyRootInlineBox.h, rendering/RenderLineBoxList.h |
| 2 | `rendering/LegacyInlineIteratorInlines.h` |  | rendering/line/BreakingContext.h |
| 2 | `rendering/LogicalSelectionOffsetCaches.h` |  | rendering/LogicalSelectionOffsetCachesInlines.h |
| 2 | `rendering/MotionPath.h` |  | rendering/TransformOperationData.h |
| 2 | `rendering/OrderIterator.h` |  | rendering/Grid.h, rendering/RenderFlexibleBox.h |
| 2 | `rendering/OverlapTestRequestClient.h` |  | rendering/RenderWidget.h |
| 2 | `rendering/PaintFrequencyTracker.h` |  | rendering/RenderLayer.h |
| 2 | `rendering/PaintPhase.h` |  | rendering/PaintInfo.h, rendering/RenderLayerModelObject.h |
| 2 | `rendering/RenderAncestorIterator.h` |  | rendering/RenderIterator.h |
| 2 | `rendering/RenderFragmentContainerSet.h` |  | rendering/RenderMultiColumnSet.h |
| 2 | `rendering/RenderHighlight.h` |  | rendering/RenderSelection.h |
| 2 | `rendering/RenderIFrame.h` |  | rendering/RenderObjectInlines.h |
| 2 | `rendering/RenderImageResource.h` |  | rendering/RenderImage.h, rendering/RenderImageResourceStyleImage.h |
| 2 | `rendering/RenderLayerModelObject.h` |  | rendering/MotionPath.h, rendering/RenderBoxModelObject.h, rendering/svg/RenderSVGModelObject.h, rendering/svg/SVGBoundingBoxComputation.h |
| 2 | `rendering/RenderLineBoxList.h` |  | rendering/LegacyLineLayout.h, rendering/RenderInline.h |
| 2 | `rendering/RenderMedia.h` | yes | rendering/RenderVideo.h |
| 2 | `rendering/RenderObjectDocument.h` |  | rendering/RenderElementInlines.h, rendering/RenderLayer.h, rendering/RenderObjectInlines.h |
| 2 | `rendering/RenderObjectNode.h` |  | rendering/RenderElementInlines.h, rendering/RenderObjectInlines.h, rendering/svg/RenderSVGInlineText.h |
| 2 | `rendering/RenderObjectStyle.h` |  | rendering/RenderObjectInlines.h |
| 2 | `rendering/RenderSelectionGeometry.h` |  | rendering/RenderSelection.h |
| 2 | `rendering/TextBoxSelectableRange.h` |  | layout/integration/inline/InlineIteratorBoxLegacyPath.h, layout/integration/inline/InlineIteratorBoxModernPath.h, rendering/LegacyInlineTextBox.h |
| 2 | `rendering/VisibleRectContext.h` |  | rendering/RenderObjectInlines.h |
| 2 | `rendering/line/LineBreaker.h` |  | rendering/line/BreakingContext.h |
| 2 | `rendering/line/LineInfo.h` |  | rendering/line/BreakingContext.h, rendering/line/LineBreaker.h, rendering/line/LineInlineHeaders.h |
| 2 | `rendering/line/LineInlineHeaders.h` |  | rendering/line/BreakingContext.h |
| 2 | `rendering/line/TrailingObjects.h` |  | rendering/LegacyLineLayout.h, rendering/line/BreakingContext.h |
| 2 | `rendering/line/WordTrailingSpace.h` |  | rendering/RenderTextInlines.h, rendering/line/BreakingContext.h |
| 2 | `rendering/mathml/MathMLStyle.h` | yes | rendering/mathml/RenderMathMLBlock.h |
| 2 | `rendering/mathml/RenderMathMLFencedOperator.h` | yes | rendering/mathml/RenderMathMLFenced.h |
| 2 | `rendering/mathml/RenderMathMLRow.h` | yes | rendering/mathml/RenderMathMLFenced.h |
| 2 | `rendering/shapes/LayoutShape.h` |  | layout/floats/PlacedFloats.h, rendering/shapes/ShapeOutsideInfo.h |
| 2 | `rendering/style/BorderData.h` |  | rendering/BorderPainter.h |
| 2 | `rendering/style/CollapsedBorderValue.h` |  | rendering/RenderTable.h |
| 2 | `rendering/style/StyleContentAlignmentData.h` |  | rendering/RenderTableCellInlines.h |
| 2 | `rendering/style/StyleFontData.h` |  | rendering/style/StyleInheritedData.h |
| 2 | `rendering/style/StyleImage.h` |  | rendering/RenderImageResource.h, rendering/RenderImageResourceStyleImage.h |
| 2 | `rendering/style/StyleSelfAlignmentData.h` |  | rendering/PositionedLayoutConstraints.h |
| 2 | `rendering/svg/RenderSVGContainer.h` |  | rendering/svg/RenderSVGHiddenContainer.h, rendering/svg/RenderSVGViewportContainer.h |
| 2 | `rendering/svg/RenderSVGHiddenContainer.h` |  | rendering/svg/RenderSVGResourceContainer.h |
| 2 | `rendering/svg/SVGBoundingBoxComputation.h` |  | rendering/svg/RenderSVGContainer.h, rendering/svg/RenderSVGModelObject.h, rendering/svg/RenderSVGRoot.h, rendering/svg/RenderSVGText.h |
| 2 | `rendering/svg/SVGTextChunk.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h, rendering/svg/RenderSVGText.h |
| 2 | `rendering/svg/SVGTextLayoutAttributes.h` |  | rendering/svg/RenderSVGInlineText.h, rendering/svg/SVGTextMetricsBuilder.h |
| 2 | `rendering/svg/SVGTextLayoutAttributesBuilder.h` |  | rendering/svg/RenderSVGText.h |
| 2 | `rendering/svg/legacy/LegacyRenderSVGResourceContainer.h` |  | rendering/svg/legacy/LegacyRenderSVGResourceClipper.h |
| 2 | `rendering/updating/RenderTreeUpdater.h` |  | rendering/updating/RenderTreeBuilderRuby.h, rendering/updating/RenderTreeBuilderTable.h |
| 1 | `layout/LayoutState.h` |  | layout/integration/LayoutIntegrationBoxGeometryUpdater.h, layout/integration/inline/LayoutIntegrationLineLayout.h, rendering/RenderView.h |
| 1 | `layout/formattingContexts/inline/text/TextUtil.h` |  | layout/formattingContexts/inline/display/InlineDisplayLine.h, rendering/RenderBlockFlow.h, rendering/RenderText.h |
| 1 | `layout/integration/LayoutIntegrationCoverage.h` |  | rendering/RenderObject.h |
| 1 | `layout/integration/LayoutIntegrationFormattingContextLayout.h` |  | rendering/RenderView.h |
| 1 | `layout/integration/inline/InlineIteratorBoxInlines.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderText.h |
| 1 | `layout/integration/inline/InlineIteratorInlineBox.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h, rendering/RenderBlockFlow.h, rendering/RenderBox.h |
| 1 | `layout/integration/inline/InlineIteratorLineBox.h` |  | layout/integration/inline/InlineIteratorLineBoxInlines.h, layout/integration/inline/InlineIteratorLogicalOrderTraversal.h, layout/integration/inline/LayoutIntegrationLineLayout.h, layout/integration/inline/LineSelection.h, rendering/RenderBox.h, rendering/RenderElement.h |
| 1 | `layout/integration/inline/InlineIteratorLineBoxInlines.h` |  | rendering/RenderBlockFlow.h, rendering/RenderText.h |
| 1 | `layout/integration/inline/InlineIteratorLogicalOrderTraversal.h` |  | rendering/RenderBlockFlow.h, rendering/RenderText.h |
| 1 | `layout/integration/inline/InlineIteratorSVGTextBox.h` |  | rendering/RenderText.h, rendering/svg/SVGTextChunk.h |
| 1 | `layout/integration/inline/InlineIteratorTextBox.h` |  | layout/integration/inline/InlineIteratorLogicalOrderTraversal.h, layout/integration/inline/InlineIteratorSVGTextBox.h, layout/integration/inline/InlineIteratorTextBoxInlines.h, layout/integration/inline/LayoutIntegrationLineLayout.h, rendering/AccessibilityRegionContext.h, rendering/RenderBlockFlow.h, rendering/RenderElement.h |
| 1 | `layout/integration/inline/InlineIteratorTextBoxInlines.h` |  | rendering/RenderText.h |
| 1 | `layout/integration/inline/LayoutIntegrationLineLayout.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderText.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `layout/integration/inline/LineSelection.h` |  | rendering/RenderBlockFlow.h, rendering/RenderText.h |
| 1 | `layout/layouttree/LayoutBox.h` |  | layout/formattingContexts/inline/InlineItem.h, layout/formattingContexts/inline/InlineLevelBox.h, layout/formattingContexts/inline/display/InlineDisplayBox.h, layout/layouttree/LayoutElementBox.h, layout/layouttree/LayoutInlineTextBox.h, rendering/RenderObject.h |
| 1 | `layout/layouttree/LayoutBoxGeometry.h` |  | layout/floats/PlacedFloats.h, layout/integration/LayoutIntegrationBoxGeometryUpdater.h, rendering/RenderView.h |
| 1 | `layout/layouttree/LayoutElementBox.h` |  | layout/LayoutState.h, layout/floats/FloatingContext.h, layout/floats/PlacedFloats.h, layout/formattingContexts/FormattingContext.h, layout/formattingContexts/inline/InlineLineBox.h, layout/integration/inline/InlineIteratorBoxModernPath.h, layout/layouttree/LayoutInitialContainingBlock.h, rendering/RenderElement.h |
| 1 | `layout/layouttree/LayoutInitialContainingBlock.h` |  | rendering/RenderView.h |
| 1 | `layout/layouttree/LayoutInlineTextBox.h` |  | layout/formattingContexts/inline/InlineTextItem.h, rendering/RenderText.h |
| 1 | `rendering/BackgroundPainter.h` |  | rendering/RenderBox.h |
| 1 | `rendering/BlockStepSizing.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/BorderPainter.h` |  | rendering/RenderBox.h |
| 1 | `rendering/BorderShape.h` |  | rendering/RenderBox.h, rendering/RenderElement.h |
| 1 | `rendering/BreakablePositions.h` |  | rendering/RenderText.h, rendering/line/BreakingContext.h |
| 1 | `rendering/FloatingObjects.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/HitTestLocation.h` |  | rendering/HitTestResult.h, rendering/RenderBlockFlow.h |
| 1 | `rendering/HitTestRequest.h` |  | rendering/HitTestResult.h, rendering/LegacyInlineBox.h, rendering/RenderElement.h |
| 1 | `rendering/HitTestResult.h` |  | rendering/RenderBox.h, rendering/RenderObject.h, rendering/RenderView.h |
| 1 | `rendering/ImageQualityController.h` |  | rendering/RenderView.h |
| 1 | `rendering/InlineWalker.h` |  | rendering/RenderBlockFlow.h, rendering/RenderElement.h |
| 1 | `rendering/LayoutRepainter.h` |  | rendering/RenderBlockFlow.h, rendering/svg/SVGRenderSupport.h |
| 1 | `rendering/LegacyInlineTextBox.h` |  | layout/integration/inline/InlineIteratorBoxLegacyPath.h, rendering/RenderBlockFlow.h, rendering/svg/SVGInlineTextBox.h |
| 1 | `rendering/LegacyLineLayout.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/LegacyRootInlineBox.h` |  | layout/integration/inline/InlineIteratorBoxLegacyPath.h, layout/integration/inline/InlineIteratorLineBoxLegacyPath.h, rendering/FloatingObjects.h, rendering/RenderBlockFlow.h |
| 1 | `rendering/LineClampUpdater.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/LogicalSelectionOffsetCachesInlines.h` |  | rendering/RenderText.h |
| 1 | `rendering/OutlinePainter.h` |  | rendering/RenderElement.h, rendering/RenderObject.h |
| 1 | `rendering/PaintInfo.h` |  | rendering/RenderBox.h, rendering/RenderLayer.h, rendering/RenderTheme.h, rendering/mathml/MathOperator.h, rendering/svg/SVGRenderSupport.h |
| 1 | `rendering/PositionedLayoutConstraints.h` |  | rendering/RenderBox.h |
| 1 | `rendering/ReferencedSVGResources.h` |  | rendering/RenderElement.h, rendering/RenderObject.h |
| 1 | `rendering/RenderBlock.h` |  | rendering/LogicalSelectionOffsetCachesInlines.h, rendering/RenderBlockFlow.h, rendering/RenderBlockInlines.h, rendering/RenderDeprecatedFlexibleBox.h, rendering/RenderElement.h, rendering/RenderFlexibleBox.h, rendering/RenderGrid.h, rendering/RenderScrollbarPart.h, rendering/RenderSelectionGeometry.h, rendering/RenderTable.h, rendering/RenderText.h, rendering/line/WordTrailingSpace.h, rendering/mathml/RenderMathMLBlock.h |
| 1 | `rendering/RenderBlockFlowInlines.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/RenderBlockInlines.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/line/BreakingContext.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderBoxFragmentInfo.h` |  | rendering/RenderBox.h |
| 1 | `rendering/RenderBoxInlines.h` |  | rendering/GridTrackSizingAlgorithm.h, rendering/RenderBlockFlow.h, rendering/RenderBlockInlines.h, rendering/RenderBox.h, rendering/RenderView.h, rendering/line/LineInlineHeaders.h |
| 1 | `rendering/RenderBoxModelObject.h` |  | rendering/BackgroundPainter.h, rendering/BorderPainter.h, rendering/LegacyInlineBox.h, rendering/RenderBox.h, rendering/RenderBoxModelObjectInlines.h, rendering/RenderInline.h, rendering/RenderLineBreak.h, rendering/RenderTextFragment.h |
| 1 | `rendering/RenderBoxModelObjectInlines.h` |  | layout/integration/inline/InlineIteratorLineBoxInlines.h, rendering/RenderBoxInlines.h, rendering/RenderElement.h, rendering/RenderView.h, rendering/line/LineInlineHeaders.h |
| 1 | `rendering/RenderButton.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderChildIterator.h` |  | rendering/LegacyInlineIterator.h, rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderIterator.h, rendering/RenderObject.h |
| 1 | `rendering/RenderCombineText.h` |  | rendering/RenderBlockFlow.h, rendering/RenderText.h |
| 1 | `rendering/RenderCounter.h` |  | rendering/RenderBlockFlow.h, rendering/RenderElement.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/line/BreakingContext.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderDeprecatedFlexibleBox.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderElement.h |
| 1 | `rendering/RenderDescendantIterator.h` |  | rendering/InlineWalker.h, rendering/RenderElement.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderElementInlines.h` |  | rendering/RenderBox.h, rendering/RenderBoxInlines.h, rendering/RenderElement.h, rendering/RenderElementStyleInlines.h, rendering/RenderLayerInlines.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderElementStyleInlines.h` |  | rendering/LogicalSelectionOffsetCachesInlines.h, rendering/RenderBlockFlow.h, rendering/RenderLayerInlines.h, rendering/RenderObject.h |
| 1 | `rendering/RenderEmbeddedObject.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderFlexibleBox.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderButton.h, rendering/RenderElement.h, rendering/RenderMenuList.h, rendering/RenderTextControl.h |
| 1 | `rendering/RenderFragmentContainer.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderFragmentContainerSet.h, rendering/RenderFragmentedFlow.h |
| 1 | `rendering/RenderFragmentedFlow.h` |  | rendering/RenderElement.h, rendering/RenderFragmentContainer.h, rendering/RenderMultiColumnFlow.h, rendering/RenderObject.h |
| 1 | `rendering/RenderGeometryMap.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderView.h |
| 1 | `rendering/RenderGrid.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderObject.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderHTMLCanvas.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderImage.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderMedia.h, rendering/RenderView.h |
| 1 | `rendering/RenderImageResourceStyleImage.h` |  | rendering/RenderElement.h |
| 1 | `rendering/RenderInline.h` |  | rendering/InlineWalker.h, rendering/LegacyInlineIterator.h, rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderObject.h, rendering/RenderQuote.h, rendering/RenderText.h, rendering/line/BreakingContext.h, rendering/svg/RenderSVGInline.h |
| 1 | `rendering/RenderIterator.h` |  | rendering/RenderAncestorIterator.h, rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderChildIterator.h, rendering/RenderDescendantIterator.h, rendering/RenderElement.h, rendering/RenderObject.h, rendering/RenderView.h |
| 1 | `rendering/RenderLayer.h` |  | rendering/LayerAncestorClippingStack.h, rendering/RenderBlockFlow.h, rendering/RenderElement.h, rendering/RenderLayerBacking.h, rendering/RenderLayerCompositor.h, rendering/RenderLayerInlines.h, rendering/RenderLayerScrollableArea.h, rendering/RenderObject.h, rendering/RenderText.h, rendering/line/BreakingContext.h, rendering/line/LineInlineHeaders.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderLayerBacking.h` |  | rendering/RenderObject.h, rendering/RenderView.h |
| 1 | `rendering/RenderLayerCompositor.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderLayerBacking.h, rendering/RenderObject.h, rendering/RenderView.h |
| 1 | `rendering/RenderLayerInlines.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderView.h |
| 1 | `rendering/RenderLayerScrollableArea.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderObject.h |
| 1 | `rendering/RenderLayoutState.h` |  | rendering/LineClampUpdater.h, rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderView.h |
| 1 | `rendering/RenderLineBreak.h` |  | rendering/RenderBlockFlow.h, rendering/RenderElement.h, rendering/RenderObject.h, rendering/line/BreakingContext.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderListItem.h` |  | rendering/RenderBlockFlow.h, rendering/RenderElement.h, rendering/updating/RenderTreeBuilderList.h |
| 1 | `rendering/RenderListMarker.h` |  | rendering/LegacyInlineIterator.h, rendering/RenderBox.h, rendering/RenderListItem.h, rendering/line/BreakingContext.h |
| 1 | `rendering/RenderMarquee.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/RenderMenuList.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderMultiColumnFlow.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderMultiColumnSet.h, rendering/RenderMultiColumnSpannerPlaceholder.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderMultiColumnSet.h` |  | rendering/RenderBlockFlow.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderMultiColumnSpannerPlaceholder.h` |  | rendering/RenderElement.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderObjectEnums.h` |  | layout/integration/inline/LayoutIntegrationLineLayout.h, rendering/CaretRectComputation.h, rendering/RenderObject.h |
| 1 | `rendering/RenderObjectInlines.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderLayerInlines.h, rendering/RenderObject.h, rendering/RenderText.h, rendering/line/LineInlineHeaders.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderOverflow.h` |  | rendering/LegacyInlineFlowBox.h, rendering/RenderBox.h, rendering/RenderBoxFragmentInfo.h |
| 1 | `rendering/RenderPtr.h` |  | rendering/RenderElement.h, rendering/RenderLayer.h |
| 1 | `rendering/RenderQuote.h` |  | rendering/RenderView.h |
| 1 | `rendering/RenderReplaced.h` |  | rendering/RenderHTMLCanvas.h, rendering/RenderImage.h, rendering/RenderObjectInlines.h, rendering/RenderViewTransitionCapture.h, rendering/RenderWidget.h, rendering/svg/RenderSVGRoot.h, rendering/svg/legacy/LegacyRenderSVGRoot.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderReplica.h` |  | rendering/RenderObject.h |
| 1 | `rendering/RenderScrollbarPart.h` |  | rendering/RenderObject.h |
| 1 | `rendering/RenderSelection.h` |  | rendering/RenderView.h |
| 1 | `rendering/RenderTable.h` |  | rendering/RenderTableSection.h, rendering/mathml/RenderMathMLBlock.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderTableCaption.h` |  | rendering/RenderElement.h |
| 1 | `rendering/RenderTableCell.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderTableCellInlines.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderTableCellInlines.h` |  | rendering/RenderBlockFlow.h, rendering/RenderBox.h |
| 1 | `rendering/RenderTableCol.h` |  | rendering/RenderElement.h |
| 1 | `rendering/RenderTableRow.h` |  | rendering/RenderElement.h, rendering/RenderObject.h, rendering/RenderTableCell.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderTableSection.h` |  | rendering/RenderTableCell.h, rendering/RenderTableRow.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderTextControl.h` |  | rendering/RenderObject.h |
| 1 | `rendering/RenderTextFragment.h` |  | rendering/RenderButton.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderTextInlines.h` |  | rendering/RenderText.h, rendering/line/BreakingContext.h |
| 1 | `rendering/RenderTextLineBoxes.h` |  | rendering/RenderText.h |
| 1 | `rendering/RenderTheme.h` |  | rendering/RenderBox.h, rendering/RenderElement.h, rendering/RenderObject.h |
| 1 | `rendering/RenderTreeMutationDisallowedScope.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderVideo.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RenderViewTransitionCapture.h` |  | rendering/RenderObject.h |
| 1 | `rendering/RenderWidget.h` |  | rendering/RenderEmbeddedObject.h, rendering/RenderFrameBase.h, rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/RepaintRectCalculation.h` |  | rendering/RenderObject.h, rendering/VisibleRectContext.h |
| 1 | `rendering/TextAutoSizing.h` |  | rendering/RenderBlockFlow.h, rendering/RenderElement.h |
| 1 | `rendering/TextBoxTrimmer.h` |  | rendering/RenderBlockFlow.h |
| 1 | `rendering/TransformOperationData.h` |  | rendering/RenderBox.h |
| 1 | `rendering/line/BreakingContext.h` |  | rendering/RenderText.h |
| 1 | `rendering/line/LineWidth.h` |  | rendering/LegacyLineLayout.h, rendering/RenderBlock.h, rendering/RenderBlockFlow.h, rendering/line/BreakingContext.h, rendering/line/LineInfo.h |
| 1 | `rendering/mathml/MathVariant.h` |  | rendering/RenderText.h |
| 1 | `rendering/mathml/RenderMathMLBlock.h` |  | rendering/RenderBox.h, rendering/mathml/RenderMathMLRow.h, rendering/mathml/RenderMathMLToken.h |
| 1 | `rendering/mathml/RenderMathMLFenced.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/shapes/ShapeOutsideInfo.h` |  | rendering/RenderBox.h |
| 1 | `rendering/style/RenderStyle+GettersInlines.h` |  | rendering/LegacyInlineBox.h, rendering/LegacyInlineIteratorInlines.h, rendering/RenderBlockInlines.h, rendering/RenderBox.h, rendering/RenderBoxModelObjectInlines.h, rendering/RenderElementInlines.h, rendering/RenderElementStyleInlines.h, rendering/RenderObjectInlines.h, rendering/RenderTableCellInlines.h, rendering/RenderTextInlines.h, rendering/RenderTheme.h, rendering/RenderView.h, rendering/style/RenderStyle+SettersInlines.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/style/RenderStyle+SettersInlines.h` |  | rendering/RenderElement.h |
| 1 | `rendering/style/RenderStyle.h` |  | layout/formattingContexts/inline/InlineLine.h, layout/layouttree/LayoutBox.h, rendering/RenderElement.h, rendering/RenderObjectStyle.h, rendering/TextAutoSizing.h, rendering/line/WordTrailingSpace.h, rendering/style/RenderStyle+GettersInlines.h |
| 1 | `rendering/style/RenderStyleConstants.h` |  | layout/layouttree/LayoutBox.h, rendering/BaselineAlignment.h, rendering/BorderShape.h, rendering/EventRegion.h, rendering/GridBaselineAlignment.h, rendering/LegacyLineLayout.h, rendering/RenderMarquee.h, rendering/RenderObject.h, rendering/style/BorderValue.h, rendering/style/StyleContentAlignmentData.h, rendering/style/StyleSelfAlignmentData.h, rendering/svg/legacy/LegacyRenderSVGResource.h |
| 1 | `rendering/style/StyleInheritedData.h` |  | rendering/RenderView.h |
| 1 | `rendering/svg/RenderSVGBlock.h` |  | rendering/RenderObject.h, rendering/svg/RenderSVGText.h |
| 1 | `rendering/svg/RenderSVGInline.h` |  | rendering/RenderObject.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/svg/RenderSVGInlineText.h` |  | layout/integration/inline/InlineIteratorBoxLegacyPath.h, layout/integration/inline/InlineIteratorSVGTextBox.h, rendering/RenderText.h, rendering/line/BreakingContext.h |
| 1 | `rendering/svg/RenderSVGModelObject.h` |  | rendering/RenderLayer.h, rendering/RenderObject.h, rendering/svg/RenderSVGContainer.h |
| 1 | `rendering/svg/RenderSVGResourceClipper.h` |  | rendering/RenderBox.h, rendering/RenderLayerInlines.h |
| 1 | `rendering/svg/RenderSVGResourceContainer.h` |  | rendering/RenderElement.h, rendering/svg/RenderSVGResourceClipper.h |
| 1 | `rendering/svg/RenderSVGRoot.h` |  | rendering/RenderView.h, rendering/svg/RenderSVGViewportContainer.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/svg/RenderSVGText.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/svg/RenderSVGViewportContainer.h` |  | rendering/RenderElement.h |
| 1 | `rendering/svg/SVGInlineTextBox.h` |  | layout/integration/inline/InlineIteratorBoxLegacyPath.h, rendering/RenderText.h |
| 1 | `rendering/svg/SVGRenderSupport.h` |  | rendering/RenderElement.h, rendering/RenderObject.h, rendering/svg/RenderSVGModelObject.h, rendering/svg/legacy/LegacyRenderSVGModelObject.h, rendering/svg/legacy/LegacyRenderSVGRoot.h |
| 1 | `rendering/svg/legacy/LegacyRenderSVGContainer.h` |  | rendering/svg/legacy/LegacyRenderSVGHiddenContainer.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/svg/legacy/LegacyRenderSVGModelObject.h` |  | rendering/RenderObject.h, rendering/svg/legacy/LegacyRenderSVGContainer.h |
| 1 | `rendering/svg/legacy/LegacyRenderSVGResourceClipper.h` |  | rendering/RenderBox.h |
| 1 | `rendering/svg/legacy/LegacyRenderSVGRoot.h` |  | rendering/RenderObject.h, rendering/RenderView.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderBlock.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderBlockFlow.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderContinuation.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderFirstLetter.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderFormControls.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderInline.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderList.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderMathML.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderMultiColumn.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderRuby.h` |  | rendering/RenderElement.h, rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderSVG.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreeBuilderTable.h` |  | rendering/updating/RenderTreeBuilder.h |
| 1 | `rendering/updating/RenderTreePosition.h` |  | rendering/updating/RenderTreeBuilder.h, rendering/updating/RenderTreeUpdater.h |
| 0 | `rendering/RenderBlockFlow.h` |  | (entry) |
| 0 | `rendering/RenderBox.h` |  | (entry) |
| 0 | `rendering/RenderElement.h` |  | (entry) |
| 0 | `rendering/RenderObject.h` |  | (entry) |
| 0 | `rendering/RenderText.h` |  | (entry) |
| 0 | `rendering/RenderView.h` |  | (entry) |
| 0 | `rendering/updating/RenderTreeBuilder.h` |  | (entry) |

## Boundaries

### accessibility (2)

- `accessibility/AXObjectCache.h` ← 5 file(s)
- `accessibility/AXUtilities.h` ← 1 file(s)

### css (3)

- `css/CSSFontSelector.h` ← 1 file(s)
- `css/CSSValue.h` ← 1 file(s)
- `css/values/primitives/CSSPrimitiveNumeric.h` ← 1 file(s)

### dom (23)

- `dom/BoundaryPointInlines.h` ← 1 file(s)
- `dom/ContainerNodeInlines.h` ← 4 file(s)
- `dom/ContentVisibilityDocumentState.h` ← 1 file(s)
- `dom/Document.h` ← 3 file(s)
- `dom/DocumentMarkerController.h` ← 1 file(s)
- `dom/DocumentPage.h` ← 3 file(s)
- `dom/DocumentResourceLoader.h` ← 1 file(s)
- `dom/DocumentView.h` ← 6 file(s)
- `dom/Element.h` ← 2 file(s)
- `dom/ElementAncestorIteratorInlines.h` ← 1 file(s)
- `dom/ElementChildIteratorInlines.h` ← 1 file(s)
- `dom/ElementInlines.h` ← 1 file(s)
- `dom/Node.h` ← 1 file(s)
- `dom/NodeDocument.h` ← 1 file(s)
- `dom/NodeIdentifier.h` ← 1 file(s)
- `dom/NodeInlines.h` ← 3 file(s)
- `dom/NodeTraversal.h` ← 1 file(s)
- `dom/PseudoElement.h` ← 2 file(s)
- `dom/Range.h` ← 1 file(s)
- `dom/RenderedDocumentMarker.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/Text.h` ← 3 file(s)
- `dom/ViewTransition.h` ← 2 file(s)

### editing (7)

- `editing/EditingInlines.h` ← 2 file(s)
- `editing/Editor.h` ← 2 file(s)
- `editing/FrameSelection.h` ← 4 file(s)
- `editing/InlineRunAndOffset.h` ← 3 file(s)
- `editing/RenderedPosition.h` ← 1 file(s)
- `editing/SelectionGeometryGatherer.h` ← 1 file(s)
- `editing/VisiblePosition.h` ← 3 file(s)

### external (5)

- `pal/HysteresisActivity.h` ← 1 file(s)
- `pal/spi/cocoa/FeatureFlagsSPI.h` ← 1 file(s)
- `unicode/ubidi.h` ← 3 file(s)
- `unicode/ubrk.h` ← 1 file(s)
- `unicode/utypes.h` ← 1 file(s)

### html (18)

- `html/HTMLAnchorElement.h` ← 1 file(s)
- `html/HTMLBRElement.h` ← 1 file(s)
- `html/HTMLBodyElement.h` ← 3 file(s)
- `html/HTMLButtonElement.h` ← 1 file(s)
- `html/HTMLFieldSetElement.h` ← 1 file(s)
- `html/HTMLFrameOwnerElement.h` ← 3 file(s)
- `html/HTMLFrameSetElement.h` ← 1 file(s)
- `html/HTMLHtmlElement.h` ← 3 file(s)
- `html/HTMLIFrameElement.h` ← 1 file(s)
- `html/HTMLImageElement.h` ← 3 file(s)
- `html/HTMLInputElement.h` ← 2 file(s)
- `html/HTMLLegendElement.h` ← 1 file(s)
- `html/HTMLMarqueeElement.h` ← 1 file(s)
- `html/HTMLSelectElement.h` ← 1 file(s)
- `html/HTMLTableCellElement.h` ← 1 file(s)
- `html/HTMLTableElement.h` ← 1 file(s)
- `html/HTMLTextAreaElement.h` ← 2 file(s)
- `html/SwitchTrigger.h` ← 1 file(s)

### loader (4)

- `loader/TextResourceDecoder.h` ← 1 file(s)
- `loader/cache/CachedImage.h` ← 1 file(s)
- `loader/cache/CachedImageClient.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)

### mathml (3)

- `mathml/MathMLElement.h` ← 2 file(s)
- `mathml/MathMLOperatorDictionary.h` ← 3 file(s)
- `mathml/MathMLOperatorElement.h` ← 1 file(s)

### page (20)

- `page/ChromeClient.h` ← 1 file(s)
- `page/EditorClient.h` ← 1 file(s)
- `page/EventHandler.h` ← 2 file(s)
- `page/FocusController.h` ← 1 file(s)
- `page/FrameDestructionObserverInlines.h` ← 1 file(s)
- `page/FrameInlines.h` ← 1 file(s)
- `page/InteractionRegion.h` ← 1 file(s)
- `page/IntersectionObserverMarginBox.h` ← 1 file(s)
- `page/LocalFrame.h` ← 8 file(s)
- `page/LocalFrameInlines.h` ← 1 file(s)
- `page/LocalFrameView.h` ← 5 file(s)
- `page/LocalFrameViewInlines.h` ← 2 file(s)
- `page/LocalFrameViewLayoutContext.h` ← 3 file(s)
- `page/Page.h` ← 5 file(s)
- `page/ResizeObserverSize.h` ← 1 file(s)
- `page/ScrollBehavior.h` ← 1 file(s)
- `page/SecurityOrigin.h` ← 1 file(s)
- `page/ios/ContentChangeObserver.h` ← 1 file(s)
- `page/scrolling/ScrollSnapOffsetsInfo.h` ← 1 file(s)
- `page/scrolling/ScrollingCoordinator.h` ← 1 file(s)

### platform (66)

- `platform/BoxSides.h` ← 1 file(s)
- `platform/EventTrackingRegions.h` ← 1 file(s)
- `platform/LayoutUnit.h` ← 13 file(s)
- `platform/Logging.h` ← 3 file(s)
- `platform/PODIntervalTree.h` ← 1 file(s)
- `platform/PopupMenuClient.h` ← 1 file(s)
- `platform/PopupMenuStyle.h` ← 1 file(s)
- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 3 file(s)
- `platform/ScrollAnimator.h` ← 1 file(s)
- `platform/ScrollTypes.h` ← 4 file(s)
- `platform/ScrollableArea.h` ← 1 file(s)
- `platform/ScrollbarTheme.h` ← 1 file(s)
- `platform/ScrollbarsController.h` ← 1 file(s)
- `platform/ThemeTypes.h` ← 1 file(s)
- `platform/Timer.h` ← 2 file(s)
- `platform/TouchAction.h` ← 1 file(s)
- `platform/Widget.h` ← 1 file(s)
- `platform/graphics/Color.h` ← 1 file(s)
- `platform/graphics/ColorHash.h` ← 1 file(s)
- `platform/graphics/DoublePoint.h` ← 1 file(s)
- `platform/graphics/FloatPoint.h` ← 2 file(s)
- `platform/graphics/FloatPoint3D.h` ← 1 file(s)
- `platform/graphics/FloatQuad.h` ← 7 file(s)
- `platform/graphics/FloatRect.h` ← 4 file(s)
- `platform/graphics/FloatRoundedRect.h` ← 3 file(s)
- `platform/graphics/FloatSize.h` ← 1 file(s)
- `platform/graphics/Font.h` ← 2 file(s)
- `platform/graphics/FontBaseline.h` ← 4 file(s)
- `platform/graphics/FontCascade.h` ← 9 file(s)
- `platform/graphics/GeometryUtilities.h` ← 1 file(s)
- `platform/graphics/GlyphPage.h` ← 1 file(s)
- `platform/graphics/GraphicsContext.h` ← 5 file(s)
- `platform/graphics/GraphicsLayer.h` ← 1 file(s)
- `platform/graphics/GraphicsLayerClient.h` ← 2 file(s)
- `platform/graphics/GraphicsLayerEnums.h` ← 3 file(s)
- `platform/graphics/GraphicsTypes.h` ← 3 file(s)
- `platform/graphics/Image.h` ← 1 file(s)
- `platform/graphics/ImageBuffer.h` ← 2 file(s)
- `platform/graphics/IntRect.h` ← 3 file(s)
- `platform/graphics/IntRectHash.h` ← 1 file(s)
- `platform/graphics/LayoutPoint.h` ← 4 file(s)
- `platform/graphics/LayoutRange.h` ← 2 file(s)
- `platform/graphics/LayoutRect.h` ← 15 file(s)
- `platform/graphics/LayoutRoundedRect.h` ← 2 file(s)
- `platform/graphics/LayoutSize.h` ← 4 file(s)
- `platform/graphics/Path.h` ← 1 file(s)
- `platform/graphics/PathUtilities.h` ← 1 file(s)
- `platform/graphics/PlatformLayerIdentifier.h` ← 1 file(s)
- `platform/graphics/Region.h` ← 2 file(s)
- `platform/graphics/SurrogatePairAwareTextIterator.h` ← 1 file(s)
- `platform/graphics/TextRun.h` ← 4 file(s)
- `platform/graphics/TextTransform.h` ← 1 file(s)
- `platform/graphics/WidthIterator.h` ← 1 file(s)
- `platform/graphics/opentype/OpenTypeMathData.h` ← 1 file(s)
- `platform/graphics/transforms/AffineTransform.h` ← 4 file(s)
- `platform/graphics/transforms/TransformState.h` ← 3 file(s)
- `platform/graphics/transforms/TransformationMatrix.h` ← 2 file(s)
- `platform/ios/SelectionGeometry.h` ← 2 file(s)
- `platform/text/BidiContext.h` ← 1 file(s)
- `platform/text/BidiResolver.h` ← 1 file(s)
- `platform/text/Hyphenation.h` ← 1 file(s)
- `platform/text/TextFlags.h` ← 2 file(s)
- `platform/text/TextSpacing.h` ← 1 file(s)
- `platform/text/UnicodeBidi.h` ← 1 file(s)
- `platform/text/WritingMode.h` ← 5 file(s)

### style (40)

- `style/AnchorPositionEvaluator.h` ← 2 file(s)
- `style/PseudoElementIdentifier.h` ← 1 file(s)
- `style/ResolvedStyle.h` ← 1 file(s)
- `style/StyleChange.h` ← 1 file(s)
- `style/StyleDifference.h` ← 1 file(s)
- `style/StylePendingResources.h` ← 1 file(s)
- `style/StyleResolver.h` ← 2 file(s)
- `style/StyleScope.h` ← 2 file(s)
- `style/StyleTransformResolver.h` ← 1 file(s)
- `style/StyleTreeResolver.h` ← 1 file(s)
- `style/StyleUpdate.h` ← 1 file(s)
- `style/Styleable.h` ← 1 file(s)
- `style/computed/StyleComputedStyle+InitialInlines.h` ← 1 file(s)
- `style/values/backgrounds/StyleBorderImage.h` ← 1 file(s)
- `style/values/backgrounds/StyleLineWidth.h` ← 1 file(s)
- `style/values/borders/StyleBorderRadius.h` ← 1 file(s)
- `style/values/borders/StyleBoxShadow.h` ← 1 file(s)
- `style/values/borders/StyleCornerShapeValue.h` ← 1 file(s)
- `style/values/box/StyleMargin.h` ← 1 file(s)
- `style/values/color/StyleColor.h` ← 3 file(s)
- `style/values/color/StyleOpacity.h` ← 1 file(s)
- `style/values/content/StyleContent.h` ← 1 file(s)
- `style/values/grid/StyleGridTrackSize.h` ← 1 file(s)
- `style/values/grid/StyleGridTrackSizingDirection.h` ← 3 file(s)
- `style/values/inline/StyleLineFitEdge.h` ← 1 file(s)
- `style/values/inline/StyleLineHeight.h` ← 2 file(s)
- `style/values/inline/StyleTextBoxEdge.h` ← 2 file(s)
- `style/values/non-standard/StyleWebKitBorderSpacing.h` ← 1 file(s)
- `style/values/non-standard/StyleWebKitLineBoxContain.h` ← 1 file(s)
- `style/values/position/StyleInset.h` ← 1 file(s)
- `style/values/primitives/StylePrimitiveNumericTypes+Evaluation.h` ← 4 file(s)
- `style/values/primitives/StyleURL.h` ← 1 file(s)
- `style/values/primitives/StyleUnevaluatedCalculation.h` ← 1 file(s)
- `style/values/shapes/StyleShapeForward.h` ← 1 file(s)
- `style/values/shapes/StyleShapeOutside.h` ← 1 file(s)
- `style/values/sizing/StyleMinimumSize.h` ← 1 file(s)
- `style/values/sizing/StylePreferredSize.h` ← 2 file(s)
- `style/values/text/StyleLetterSpacing.h` ← 1 file(s)
- `style/values/text/StyleTabSize.h` ← 1 file(s)
- `style/values/text/StyleWordSpacing.h` ← 1 file(s)

### svg (9)

- `svg/SVGClipPathElement.h` ← 1 file(s)
- `svg/SVGDocumentExtensions.h` ← 1 file(s)
- `svg/SVGElement.h` ← 1 file(s)
- `svg/SVGGraphicsElement.h` ← 1 file(s)
- `svg/SVGLengthContext.h` ← 1 file(s)
- `svg/SVGSVGElement.h` ← 2 file(s)
- `svg/SVGTextPositioningElement.h` ← 1 file(s)
- `svg/SVGUnitTypes.h` ← 2 file(s)
- `svg/graphics/SVGImage.h` ← 2 file(s)

### wtf (62)

- `wtf/ASCIICType.h` ← 1 file(s)
- `wtf/ArgumentCoder.h` ← 1 file(s)
- `wtf/Assertions.h` ← 2 file(s)
- `wtf/BitSet.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 14 file(s)
- `wtf/CheckedRef.h` ← 7 file(s)
- `wtf/DataRef.h` ← 3 file(s)
- `wtf/EnumSet.h` ← 7 file(s)
- `wtf/EnumTraits.h` ← 2 file(s)
- `wtf/EnumeratedArray.h` ← 1 file(s)
- `wtf/Forward.h` ← 2 file(s)
- `wtf/Function.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 16 file(s)
- `wtf/HashSet.h` ← 5 file(s)
- `wtf/HashTraits.h` ← 1 file(s)
- `wtf/HexNumber.h` ← 1 file(s)
- `wtf/InlineWeakPtr.h` ← 5 file(s)
- `wtf/InlineWeakRef.h` ← 1 file(s)
- `wtf/IteratorRange.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 4 file(s)
- `wtf/Markable.h` ← 2 file(s)
- `wtf/MathExtras.h` ← 1 file(s)
- `wtf/MonotonicTime.h` ← 1 file(s)
- `wtf/NeverDestroyed.h` ← 2 file(s)
- `wtf/Noncopyable.h` ← 3 file(s)
- `wtf/OptionSet.h` ← 9 file(s)
- `wtf/Packed.h` ← 1 file(s)
- `wtf/Platform.h` ← 9 file(s)
- `wtf/Range.h` ← 2 file(s)
- `wtf/Ref.h` ← 2 file(s)
- `wtf/RefCounted.h` ← 4 file(s)
- `wtf/RefCountedAndCanMakeWeakPtr.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 3 file(s)
- `wtf/RobinHoodHashMap.h` ← 1 file(s)
- `wtf/RuntimeApplicationChecks.h` ← 1 file(s)
- `wtf/Scope.h` ← 1 file(s)
- `wtf/SetForScope.h` ← 2 file(s)
- `wtf/SortedArrayMap.h` ← 1 file(s)
- `wtf/StackStats.h` ← 3 file(s)
- `wtf/StdLibExtras.h` ← 3 file(s)
- `wtf/StdMap.h` ← 1 file(s)
- `wtf/StdSet.h` ← 1 file(s)
- `wtf/TZoneMalloc.h` ← 49 file(s)
- `wtf/TZoneMallocInlines.h` ← 11 file(s)
- `wtf/TypeCasts.h` ← 5 file(s)
- `wtf/UniqueRef.h` ← 3 file(s)
- `wtf/UniquelyOwned.h` ← 2 file(s)
- `wtf/Vector.h` ← 15 file(s)
- `wtf/WeakHashSet.h` ← 9 file(s)
- `wtf/WeakListHashSet.h` ← 4 file(s)
- `wtf/WeakPtr.h` ← 11 file(s)
- `wtf/WeakRef.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/CharacterProperties.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/StringView.h` ← 1 file(s)
- `wtf/text/TextBreakIterator.h` ← 3 file(s)
- `wtf/text/TextStream.h` ← 2 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)
- `wtf/unicode/CharacterNames.h` ← 3 file(s)

## Problems

### ambiguous (1)

- `PopupMenu.h -> PAL/pal/system/mac/PopupMenu.h, platform/PopupMenu.h` ← 1 file(s)

### unresolved (8)

- `CSSPropertyNames.h` ← 1 file(s)
- `HTMLNames.h` ← 4 file(s)
- `RenderStyleProperties+GettersInlines.h` ← 1 file(s)
- `RenderStyleProperties+SettersInlines.h` ← 1 file(s)
- `RenderStyleProperties.h` ← 1 file(s)
- `SVGElementTypeHelpers.h` ← 4 file(s)
- `SVGNames.h` ← 1 file(s)
- `Settings.h` ← 7 file(s)
