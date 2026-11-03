# HTML review checklist: 4.12.5 The canvas element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 133 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html canvas` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.12.5 The canvas element](https://html.spec.whatwg.org/#the-canvas-element)

Status: reviewed (2026-11-01)
> `HTMLCanvasElement`; `GetContext` takes the type only, `GetContext2d` the options (see `OVERVIEW.md`). There's no placeholder mode or `transferControlToOffscreen()`, and canvases are always origin-clean (see `KNOWN-ISSUES.md`).
<!-- definitions: attr-canvas-width attr-canvas-height obtain-numeric-values concept-canvas-origin-clean concept-canvas-context-mode concept-canvas-none concept-canvas-placeholder concept-canvas-2d concept-canvas-bitmaprenderer concept-canvas-webgl concept-canvas-webgl2 concept-canvas-webgpu dom-canvas-height canvas-blob-serialisation-task-source -->

- [x] [being used as relevant canvas fallback content](https://html.spec.whatwg.org/#being-used-as-relevant-canvas-fallback-content) — An element whose nearest canvas element ancestor is being rendered and represents embedded content is an element that is being used as relevant canvas fallback content.

- [x] [HTMLCanvasElement.width](https://html.spec.whatwg.org/#dom-canvas-width) — The width and height IDL attributes must reflect the respective content attributes of the same name, with the same defaults.

- [x] [HTMLCanvasElement.getContext(contextId, options)](https://html.spec.whatwg.org/#dom-canvas-getcontext) — The getContext(contextId, options) method of the canvas element, when invoked, must run these steps:
  - 1. If options is not an object, then set options to null.
  - 2. Set options to the result of converting options to a JavaScript value.
  - 3. Run the steps in the cell of the following table whose column header matches this canvas element's canvas context mode and whose row header matches contextId: * For example, the "webgl" or "webgl2" value in the case of a user agent having exhausted the graphics hardware's abilities and having no software fallback implementation.

- [x] [HTMLCanvasElement.toDataURL(type, quality)](https://html.spec.whatwg.org/#dom-canvas-todataurl) — The toDataURL(type, quality) method, when invoked, must run these steps:
  - 1. If this canvas element's bitmap's origin-clean flag is set to false, then throw a "SecurityError" DOMException.
  - 2. If this canvas element's bitmap has no pixels (i.e. either its horizontal dimension or its vertical dimension is zero), then return the string "data:,". (This is the shortest data: URL; it represents the empty string in a text/plain resource.)
  - 3. Let file be a serialization of this canvas element's bitmap as a file, passing type and quality if given.
  - 4. If file is null, then return "data:,".
  - 5. Return a data: URL representing file. [RFC2397]

- [x] [HTMLCanvasElement.toBlob(callback, type, quality)](https://html.spec.whatwg.org/#dom-canvas-toblob) — The toBlob(callback, type, quality) method, when invoked, must run these steps:
  - 1. If this canvas element's bitmap's origin-clean flag is set to false, then throw a "SecurityError" DOMException.
  - 2. Let result be null.
  - 3. If this canvas element's bitmap has pixels (i.e., neither its horizontal dimension nor its vertical dimension is zero), then set result to a copy of this canvas element's bitmap.
  - 4. Run these steps in parallel:
    - 4a. If result is non-null, then set result to a serialization of result as a file with type and quality if given.
    - 4b. Queue an element task on the canvas blob serialization task source given the canvas element to run these steps:
      - 4b.i. If result is non-null, then set result to a new Blob object, created in the relevant realm of this canvas element, representing result. [FILEAPI]
      - 4b.ii. Invoke callback with « result » and "report".

- [x] [HTMLCanvasElement.transferControlToOffscreen()](https://html.spec.whatwg.org/#dom-canvas-transfercontroltooffscreen) — The transferControlToOffscreen() method, when invoked, must run these steps:
  - 1. If this canvas element's context mode is not set to none, throw an "InvalidStateError" DOMException.
  - 2. Let offscreenCanvas be a new OffscreenCanvas object with its width and height equal to the values of the width and height content attributes of this canvas element.
  - 3. Set the offscreenCanvas's placeholder canvas element to a weak reference to this canvas element.
  - 4. Set this canvas element's context mode to placeholder.
  - 5. Set the offscreenCanvas's inherited language to the language of this canvas element.
  - 6. Set the offscreenCanvas's inherited direction to the directionality of this canvas element.
  - 7. Return offscreenCanvas.

##### [4.12.5.1 The 2D rendering context](https://html.spec.whatwg.org/#2dcontext)

Status: reviewed (2026-11-01)
> `CanvasRenderingContext2D` and `CanvasRenderingContext2DBase`; set bitmap dimensions is `HTMLCanvasElement::DidUpdateSizeProperties`.
> Fixed: resetting the context (or resizing the canvas) after a transform that couldn't be inverted left paths and drawing ignored.
<!-- definitions: dom-canvascolortype-unorm8 dom-canvascolortype-float16 fill-rule dom-context-2d-fillrule-nonzero dom-context-2d-fillrule-evenodd dom-context-2d-imagesmoothingquality-low dom-context-2d-imagesmoothingquality-medium dom-context-2d-imagesmoothingquality-high -->

- [x] [2D context creation algorithm](https://html.spec.whatwg.org/#2d-context-creation-algorithm) — The 2D context creation algorithm, which is passed a target (a canvas element) and options, consists of running these steps:
  - 1. Let settings be the result of converting options to the dictionary type CanvasRenderingContext2DSettings. (This can throw an exception.)
  - 2. Let context be a new CanvasRenderingContext2D object.
  - 3. Initialize context's canvas attribute to point to target.
  - 4. Set context's output bitmap to the same bitmap as target's bitmap (so that they are shared).
  - 5. Set bitmap dimensions to the numeric values of target's width and height content attributes.
  - 6. Run the canvas settings output bitmap initialization algorithm, given context and settings.
  - 7. Return context.

- [x] [set bitmap dimensions](https://html.spec.whatwg.org/#concept-canvas-set-bitmap-dimensions) — When the user agent is to set bitmap dimensions to width and height, it must run these steps:
  - 1. Reset the rendering context to its default state.
  - 2. Resize the output bitmap to the new width and height.
  - 3. Let canvas be the canvas element to which the rendering context's canvas attribute was initialized.
  - 4. If the numeric value of canvas's width content attribute differs from width, then set canvas's width content attribute to the shortest possible string representing width as a valid non-negative integer.
  - 5. If the numeric value of canvas's height content attribute differs from height, then set canvas's height content attribute to the shortest possible string representing height as a valid non-negative integer.

- [x] [CanvasRenderingContext2D.canvas](https://html.spec.whatwg.org/#dom-context-2d-canvas) — The canvas attribute must return the value it was initialized to when the object was created.

###### [4.12.5.1.1 Implementation notes](https://html.spec.whatwg.org/#implementation-notes)

Status: n/a (advice)

###### [4.12.5.1.2 The canvas settings](https://html.spec.whatwg.org/#the-canvas-settings)

Status: reviewed (2026-11-01)
> `CanvasRenderingContext2DSettings`; the `alpha` setting and `float16` colour type are accepted but not drawn with (see `KNOWN-ISSUES.md`).
<!-- definitions: output-bitmap concept-canvas-alpha concept-canvas-desynchronized concept-canvas-will-read-frequently concept-canvas-color-space concept-canvas-color-type dom-canvasrenderingcontext2dsettings-alpha dom-canvasrenderingcontext2dsettings-desynchronized dom-canvasrenderingcontext2dsettings-colorspace dom-canvasrenderingcontext2dsettings-colortype dom-canvasrenderingcontext2dsettings-willreadfrequently -->

- [x] [initialize a CanvasSettings output bitmap](https://html.spec.whatwg.org/#canvas-setting-init-bitmap) — To initialize a CanvasSettings output bitmap, given a CanvasSettings context and a CanvasRenderingContext2DSettings settings:
  - 1. Set context's alpha to settings["alpha"].
  - 2. Set context's desynchronized to settings["desynchronized"].
  - 3. Set context's color space to settings["colorSpace"].
  - 4. Set context's color type to settings["colorType"].
  - 5. Set context's will read frequently to settings["willReadFrequently"].

- [x] [CanvasSettings.getContextAttributes()](https://html.spec.whatwg.org/#dom-context-2d-canvas-getcontextattributes) — The getContextAttributes() method steps are to return «[ "alpha" → this's alpha, "desynchronized" → this's desynchronized, "colorSpace" → this's color space, "colorType" → this's color type, "willReadFrequently" → this's will read frequently ]».

###### [4.12.5.1.3 The canvas state](https://html.spec.whatwg.org/#the-canvas-state)

Status: reviewed (2026-11-01)
> `Save`, `Restore` and `Reset`; saves are counted until something changes, and the stack has upstream's limit. `IsContextLost` is always false, as there's no context loss.
<!-- definitions: drawing-state concept-canvas-context-lost -->

- [x] [CanvasState.save()](https://html.spec.whatwg.org/#dom-context-2d-save) — The save() method steps are to push a copy of the current drawing state onto the drawing state stack.

- [x] [CanvasState.restore()](https://html.spec.whatwg.org/#dom-context-2d-restore) — The restore() method steps are to pop the top entry in the drawing state stack, and reset the drawing state it describes. If there is no saved state, then the method must do nothing.

- [x] [CanvasState.reset()](https://html.spec.whatwg.org/#dom-context-2d-reset) — The reset() method steps are to reset the rendering context to its default state.

- [x] [reset the rendering context to its default state](https://html.spec.whatwg.org/#reset-the-rendering-context-to-its-default-state) — To reset the rendering context to its default state:
  - 1. Clear canvas's bitmap to transparent black.
  - 2. Empty the list of subpaths in the context's current default path.
  - 3. Clear the context's drawing state stack.
  - 4. Reset everything that drawing state consists of to their initial values.

- [x] [CanvasState.isContextLost()](https://html.spec.whatwg.org/#dom-context-2d-iscontextlost) — The isContextLost() method steps are to return this's context lost.

###### [4.12.5.1.4 Line styles](https://html.spec.whatwg.org/#line-styles)

Status: reviewed (2026-11-01)
> The setters in `CanvasRenderingContext2DBase`; tracing a path is the graphics context's stroking (`Path::StrokeContains` for hit testing).
<!-- definitions: dom-context-2d-linecap dom-context-2d-linejoin dash-list -->

- [x] [CanvasPathDrawingStyles.lineWidth](https://html.spec.whatwg.org/#dom-context-2d-linewidth) — The lineWidth attribute gives the width of lines, in coordinate space units. On getting, it must return the current value. On setting, zero, negative, infinite, and NaN values must be ignored, leaving the value unchanged; other values must change the current value to the new value.

- [x] [CanvasPathDrawingStyles.miterLimit](https://html.spec.whatwg.org/#dom-context-2d-miterlimit) — When the lineJoin attribute has the value "miter", strokes use the miter limit ratio to decide how to render joins. The miter limit ratio can be explicitly set using the miterLimit attribute. On getting, it must return the current value. On setting, zero, negative, infinite, and NaN values must be ignored, leaving the value unchanged; other values must change the current value to the new value.

- [x] [CanvasPathDrawingStyles.setLineDash(segments)](https://html.spec.whatwg.org/#dom-context-2d-setlinedash) — The setLineDash(segments) method, when invoked, must run these steps:
  - 1. If any value in segments is not finite (e.g. an Infinity or a NaN value), or if any value is negative (less than zero), then return (without throwing an exception; user agents could show a message on a developer console, though, as that would be helpful for debugging).
  - 2. If the number of elements in segments is odd, then let segments be the concatenation of two copies of segments.
  - 3. Set the object's dash list to segments.

- [x] [CanvasPathDrawingStyles.getLineDash()](https://html.spec.whatwg.org/#dom-context-2d-getlinedash) — When the getLineDash() method is invoked, it must return a sequence whose values are the values of the object's dash list, in the same order.

- [x] [CanvasPathDrawingStyles.lineDashOffset](https://html.spec.whatwg.org/#dom-context-2d-linedashoffset) — It is sometimes useful to change the "phase" of the dash pattern, e.g. to achieve a "marching ants" effect. The phase can be set using the lineDashOffset attribute. On getting, it must return the current value. On setting, infinite and NaN values must be ignored, leaving the value unchanged; other values must change the current value to the new value.

- [x] [trace a path](https://html.spec.whatwg.org/#trace-a-path) — When a user agent is to trace a path, given an object style that implements the CanvasPathDrawingStyles interface, it must run the following algorithm. This algorithm returns a new path.
  - 1. Let path be a copy of the path being traced.
  - 2. Prune all zero-length line segments from path.
  - 3. Remove from path any subpaths containing no lines (i.e. subpaths with just one point).
  - 4. Replace each point in each subpath of path other than the first point and the last point of each subpath by a join that joins the line leading to that point to the line leading out of that point, such that the subpaths all consist of two points (a starting point with a line leading out of it, and an ending point with a line leading into it), one or more lines (connecting the points and the joins)…
  - 5. Add a straight closing line to each closed subpath in path connecting the last point and the first point of that subpath; change the last point to a join (from the previously last line to the newly added closing line), and change the first point to a join (from the newly added closing line to the first line).
  - 6. If style's dash list is empty, then jump to the step labeled convert.
  - 7. Let pattern width be the concatenation of all the entries of style's dash list, in coordinate space units.
  - 8. For each subpath subpath in path, run the following substeps. These substeps mutate the subpaths in path in vivo.
    - 8a. Let subpath width be the length of all the lines of subpath, in coordinate space units.
    - 8b. Let offset be the value of style's lineDashOffset, in coordinate space units.
    - 8c. While offset is greater than pattern width, decrement it by pattern width. While offset is less than zero, increment it by pattern width.
    - 8d. Define L to be a linear coordinate line defined along all lines in subpath, such that the start of the first line in the subpath is defined as coordinate 0, and the end of the last line in the subpath is defined as coordinate subpath width.
    - 8e. Let position be 0 minus offset.
    - 8f. Let index be 0.
    - 8g. Let current state be off (the other states being on and zero-on).
    - 8h. Dash on: Let segment length be the value of style's dash list's indexth entry.
    - 8i. Increment position by segment length.
    - 8j. If position is greater than subpath width, then end these substeps for this subpath and start them again for the next subpath; if there are no more subpaths, then jump to the step labeled convert instead.
    - 8k. If segment length is nonzero, then let current state be on.
    - 8l. Increment index by one.
    - 8m. Dash off: Let segment length be the value of style's dash list's indexth entry.
    - 8n. Let start be the offset position on L.
    - 8o. Increment position by segment length.
    - 8p. If position is less than zero, then jump to the step labeled post-cut.
    - 8q. If start is less than zero, then let start be zero.
    - 8r. If position is greater than subpath width, then let end be the offset subpath width on L. Otherwise, let end be the offset position on L.
    - 8s. Jump to the first appropriate step: If segment length is zero and current state is off Do nothing, just continue to the next step. If current state is off Cut the line on which end finds itself short at end and place a point there, cutting in two the subpath that it was in; remove all line segments, joins, points, and subpaths that are between start and end; and finally place a single point at st…
    - 8t. Post-cut: If position is greater than subpath width, then jump to the step labeled convert.
    - 8u. Increment index by one. If it is equal to the number of entries in style's dash list, then let index be 0.
    - 8v. Return to the step labeled dash on.
  - 9. Convert: This is the step that converts the path to a new path that represents its stroke. Create a new path that describes the edge of the areas that would be covered if a straight line of length equal to style's lineWidth was swept along each subpath in path while being kept at an angle such that the line is orthogonal to the path being swept, replacing each point with the end cap necessary to…
  - 10. Return the newly created path.

###### [4.12.5.1.5 Text styles](https://html.spec.whatwg.org/#text-styles)

Status: reviewed (2026-11-01)
> `SetFont` (`FontProxy`) and the text drawing style setters; `letterSpacing` and `wordSpacing` reject some units, and the text preparation algorithm replaces vertical tabs too (see `DIFFERENCES.md`).
<!-- definitions: font-style-source-object concept-canvastextdrawingstyles-language dom-context-2d-lang-inherit concept-canvastextdrawingstyles-letter-spacing concept-canvastextdrawingstyles-word-spacing dom-context-2d-textalign-start dom-context-2d-textalign-end dom-context-2d-textalign-left dom-context-2d-textalign-right dom-context-2d-textalign-center dom-context-2d-textbaseline-top dom-context-2d-textbaseline-hanging dom-context-2d-textbaseline-middle dom-context-2d-textbaseline-alphabetic dom-context-2d-textbaseline-ideographic dom-context-2d-textbaseline-bottom dom-context-2d-direction-ltr dom-context-2d-direction-rtl dom-context-2d-direction-inherit dom-context-2d-fontkerning-auto dom-context-2d-fontkerning-normal dom-context-2d-fontkerning-none dom-context-2d-fontstretch-ultra-condensed dom-context-2d-fontstretch-extra-condensed dom-context-2d-fontstretch-condensed dom-context-2d-fontstretch-semi-condensed dom-context-2d-fontstretch-normal dom-context-2d-fontstretch-semi-expanded dom-context-2d-fontstretch-expanded dom-context-2d-fontstretch-extra-expanded dom-context-2d-fontstretch-ultra-expanded dom-context-2d-fontvariantcaps-normal dom-context-2d-fontvariantcaps-small-caps dom-context-2d-fontvariantcaps-all-small-caps dom-context-2d-fontvariantcaps-petite-caps dom-context-2d-fontvariantcaps-all-petite-caps dom-context-2d-fontvariantcaps-unicase dom-context-2d-fontvariantcaps-titling-caps dom-context-2d-textrendering-auto dom-context-2d-textrendering-optimizespeed dom-context-2d-textrendering-optimizelegibility dom-context-2d-textrendering-geometricprecision -->

- [x] [CanvasTextDrawingStyles.font](https://html.spec.whatwg.org/#dom-context-2d-font) — The font IDL attribute, on setting, must be parsed as a CSS <'font'> value (but without supporting property-independent style sheet syntax like 'inherit'), and the resulting font must be assigned to the context, with the 'line-height' component forced to 'normal', with the 'font-size' component converted to CSS pixels, and with system fonts being computed to explicit values. If the new value is syntactically incorrect (including using property-independent style sheet syntax like 'inherit' or 'initial'), then it must be ignored, without assigning a new font value. [CSS]

- [x] [CanvasTextDrawingStyles.textAlign](https://html.spec.whatwg.org/#dom-context-2d-textalign) — The textAlign IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the textAlign attribute must initially have the value start.

- [x] [CanvasTextDrawingStyles.textBaseline](https://html.spec.whatwg.org/#dom-context-2d-textbaseline) — The textBaseline IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the textBaseline attribute must initially have the value alphabetic.

- [x] [CanvasTextDrawingStyles.lang](https://html.spec.whatwg.org/#dom-context-2d-lang) — The lang getter steps are to return this's language.

- [x] [The lang setter steps](https://html.spec.whatwg.org/#dom-context-2d-lang) — The lang setter steps are to set this's language to the given value.

- [x] [CanvasTextDrawingStyles.direction](https://html.spec.whatwg.org/#dom-context-2d-direction) — The direction IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the direction attribute must initially have the value "inherit".

- [x] [CanvasTextDrawingStyles.letterSpacing](https://html.spec.whatwg.org/#dom-context-2d-letterspacing) — The letterSpacing getter steps are to return the serialized form of this's letter spacing.

- [x] [The letterSpacing setter steps](https://html.spec.whatwg.org/#dom-context-2d-letterspacing) — The letterSpacing setter steps are:
  - 1. Let parsed be the result of parsing the given value as a CSS <length>.
  - 2. If parsed is failure, then return.
  - 3. Set this's letter spacing to parsed.

- [x] [CanvasTextDrawingStyles.wordSpacing](https://html.spec.whatwg.org/#dom-context-2d-wordspacing) — The wordSpacing getter steps are to return the serialized form of this's word spacing.

- [x] [The wordSpacing setter steps](https://html.spec.whatwg.org/#dom-context-2d-wordspacing) — The wordSpacing setter steps are:
  - 1. Let parsed be the result of parsing the given value as a CSS <length>.
  - 2. If parsed is failure, then return.
  - 3. Set this's word spacing to parsed.

- [x] [CanvasTextDrawingStyles.fontKerning](https://html.spec.whatwg.org/#dom-context-2d-fontkerning) — The fontKerning IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the fontKerning attribute must initially have the value "auto".

- [x] [CanvasTextDrawingStyles.fontStretch](https://html.spec.whatwg.org/#dom-context-2d-fontstretch) — The fontStretch IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the fontStretch attribute must initially have the value "normal".

- [x] [CanvasTextDrawingStyles.fontVariantCaps](https://html.spec.whatwg.org/#dom-context-2d-fontvariantcaps) — The fontVariantCaps IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the fontVariantCaps attribute must initially have the value "normal".

- [x] [CanvasTextDrawingStyles.textRendering](https://html.spec.whatwg.org/#dom-context-2d-textrendering) — The textRendering IDL attribute, on getting, must return the current value. On setting, the current value must be changed to the new value. When the object implementing the CanvasTextDrawingStyles interface is created, the textRendering attribute must initially have the value "auto".

- [x] [text preparation algorithm](https://html.spec.whatwg.org/#text-preparation-algorithm) — The text preparation algorithm is as follows. It takes as input a string text, a CanvasTextDrawingStyles object target, and an optional length maxWidth. It returns an array of glyph shapes, each positioned on a common coordinate space, a physical alignment whose value is one of left, right, and cen…
  - 1. If maxWidth was provided but is less than or equal to zero or equal to NaN, then return an empty array.
  - 2. Replace all ASCII whitespace in text with U+0020 SPACE characters.
  - 3. Let font be the current font of target, as given by that object's font attribute.
  - 4. Let language be the target's language.
  - 5. If language is "inherit":
    - 5a. Let sourceObject be object's font style source object.
    - 5b. If sourceObject is a canvas element, then set language to the sourceObject's language.
    - 5c. Otherwise:
      - 5c.i. Assert: sourceObject is an OffscreenCanvas object.
      - 5c.ii. Set language to the sourceObject's inherited language.
  - 6. If language is the empty string, then set language to explicitly unknown.
  - 7. Apply the appropriate step from the following list to determine the value of direction: If the target object's direction attribute has the value "ltr"Let direction be 'ltr'.If the target object's direction attribute has the value "rtl"Let direction be 'rtl'.If the target object's direction attribute has the value "inherit"
    - 7a. Let sourceObject be object's font style source object.
    - 7b. If sourceObject is a canvas element, then let direction be sourceObject's directionality.
    - 7c. Otherwise:
      - 7c.i. Assert: sourceObject is an OffscreenCanvas object.
      - 7c.ii. Let direction be sourceObject's inherited direction.
  - 8. Form a hypothetical infinitely-wide CSS line box containing a single inline box containing the text text, with the CSS content language set to language, and with its CSS properties set as follows: and with all other properties set to their initial values.
  - 9. If maxWidth was provided and the hypothetical width of the inline box in the hypothetical line box is greater than maxWidth CSS pixels, then change font to have a more condensed font (if one is available or if a reasonably readable one can be synthesized by applying a horizontal scale factor to the font) or a smaller font, and return to the previous step.
  - 10. The anchor point is a point on the inline box, and the physical alignment is one of the values left, right, and center. These variables are determined by the textAlign and textBaseline values as follows: Horizontal position: If textAlign is leftIf textAlign is start and direction is 'ltr'If textAlign is end and direction is 'rtl'Set the anchor point's horizontal position to the left edge of the i…
  - 11. Let result be an array constructed by iterating over each glyph in the inline box from left to right (if any), adding to the array, for each glyph, the shape of the glyph as it is in the inline box, positioned on a coordinate space using CSS pixels with its origin at the anchor point.
  - 12. Return result, physical alignment, and the inline box.

###### [4.12.5.1.6 Building paths](https://html.spec.whatwg.org/#building-paths)

Status: reviewed (2026-11-01)
> `CanvasPath`; the path methods do nothing while the transform can't be inverted, and `arcTo()` throws before ensuring a subpath (see `DIFFERENCES.md`).
<!-- definitions: concept-path line-segments need-new-subpath -->

- [x] [CanvasPath.moveTo(x, y)](https://html.spec.whatwg.org/#dom-context-2d-moveto) — The moveTo(x, y) method, when invoked, must run these steps:
  - 1. If either of the arguments are infinite or NaN, then return.
  - 2. Create a new subpath with the specified point as its first (and only) point.

- [x] [ensure there is a subpath](https://html.spec.whatwg.org/#ensure-there-is-a-subpath) — When the user agent is to ensure there is a subpath for a coordinate (x, y) on a path, the user agent must check to see if the path has its need new subpath flag set. If it does, then the user agent must create a new subpath with the point (x, y) as its first (and only) point, as if the moveTo() method had been called, and must then unset the path's need new subpath flag.

- [x] [CanvasPath.closePath()](https://html.spec.whatwg.org/#dom-context-2d-closepath) — The closePath() method, when invoked, must do nothing if the object's path has no subpaths. Otherwise, it must mark the last subpath as closed, create a new subpath whose first point is the same as the previous subpath's first point, and finally add this new subpath to the path.

- [x] [CanvasPath.lineTo(x, y)](https://html.spec.whatwg.org/#dom-context-2d-lineto) — The lineTo(x, y) method, when invoked, must run these steps:
  - 1. If either of the arguments are infinite or NaN, then return.
  - 2. If the object's path has no subpaths, then ensure there is a subpath for (x, y).
  - 3. Otherwise, connect the last point in the subpath to the given point (x, y) using a straight line, and then add the given point (x, y) to the subpath.

- [x] [CanvasPath.quadraticCurveTo(cpx, cpy, x, y)](https://html.spec.whatwg.org/#dom-context-2d-quadraticcurveto) — The quadraticCurveTo(cpx, cpy, x, y) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Ensure there is a subpath for (cpx, cpy).
  - 3. Connect the last point in the subpath to the given point (x, y) using a quadratic Bézier curve with control point (cpx, cpy). [BEZIER]
  - 4. Add the given point (x, y) to the subpath.

- [x] [CanvasPath.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y)](https://html.spec.whatwg.org/#dom-context-2d-beziercurveto) — The bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Ensure there is a subpath for (cp1x, cp1y).
  - 3. Connect the last point in the subpath to the given point (x, y) using a cubic Bézier curve with control points (cp1x, cp1y) and (cp2x, cp2y). [BEZIER]
  - 4. Add the point (x, y) to the subpath.

- [x] [CanvasPath.arcTo(x1, y1, x2, y2, radius)](https://html.spec.whatwg.org/#dom-context-2d-arcto) — The arcTo(x1, y1, x2, y2, radius) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Ensure there is a subpath for (x1, y1).
  - 3. If radius is negative, then throw an "IndexSizeError" DOMException.
  - 4. Let the point (x0, y0) be the last point in the subpath, transformed by the inverse of the current transformation matrix (so that it is in the same coordinate system as the points passed to the method).
  - 5. If the point (x0, y0) is equal to the point (x1, y1), or if the point (x1, y1) is equal to the point (x2, y2), or if radius is zero, then add the point (x1, y1) to the subpath, and connect that point to the previous point (x0, y0) by a straight line.
  - 6. Otherwise, if the points (x0, y0), (x1, y1), and (x2, y2) all lie on a single straight line, then add the point (x1, y1) to the subpath, and connect that point to the previous point (x0, y0) by a straight line.
  - 7. Otherwise, let The Arc be the shortest arc given by circumference of the circle that has radius radius, and that has one point tangent to the half-infinite line that crosses the point (x0, y0) and ends at the point (x1, y1), and that has a different point tangent to the half-infinite line that ends at the point (x1, y1) and crosses the point (x2, y2). The points at which this circle touches these…

- [x] [CanvasPath.arc(x, y, radius, startAngle, endAngle, counterclockwise)](https://html.spec.whatwg.org/#dom-context-2d-arc) — The arc(x, y, radius, startAngle, endAngle, counterclockwise) method, when invoked, must run the ellipse method steps with this, x, y, radius, radius, 0, startAngle, endAngle, and counterclockwise.

- [x] [CanvasPath.ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle, counterclockwise)](https://html.spec.whatwg.org/#dom-context-2d-ellipse) — The ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle, counterclockwise) method, when invoked, must run the ellipse method steps with this, x, y, radiusX, radiusY, rotation, startAngle, endAngle, and counterclockwise.

- [x] [determine the point on an ellipse steps](https://html.spec.whatwg.org/#determine-the-point-on-an-ellipse-steps) — The determine the point on an ellipse steps, given ellipse, and angle, are:
  - 1. Let eccentricCircle be the circle that shares its origin with ellipse, with a radius equal to the semi-major axis of ellipse.
  - 2. Let outerPoint be the point on eccentricCircle's circumference at angle measured in radians clockwise from ellipse's semi-major axis.
  - 3. Let chord be the line perpendicular to ellipse's major axis between this axis and outerPoint.
  - 4. Return the point on chord that crosses ellipse's circumference.

- [x] [ellipse method steps](https://html.spec.whatwg.org/#ellipse-method-steps) — The ellipse method steps, given canvasPath, x, y, radiusX, radiusY, rotation, startAngle, endAngle, and counterclockwise, are:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. If either radiusX or radiusY are negative, then throw an "IndexSizeError" DOMException.
  - 3. If canvasPath's path has any subpaths, then add a straight line from the last point in the subpath to the start point of the arc.
  - 4. Add the start and end points of the arc to the subpath, and connect them with an arc. The arc and its start and end points are defined as follows: Consider an ellipse that has its origin at (x, y), that has a major-axis radius radiusX and a minor-axis radius radiusY, and that is rotated about its origin such that its semi-major axis is inclined rotation radians clockwise from the x-axis. If count…

- [x] [CanvasPath.rect(x, y, w, h)](https://html.spec.whatwg.org/#dom-context-2d-rect) — The rect(x, y, w, h) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Create a new subpath containing just the four points (x, y), (x+w, y), (x+w, y+h), (x, y+h), in that order, with those four points connected by straight lines.
  - 3. Mark the subpath as closed.
  - 4. Create a new subpath with the point (x, y) as the only point in the subpath.

- [x] [CanvasPath.roundRect(x, y, w, h, radii)](https://html.spec.whatwg.org/#dom-context-2d-roundrect) — The roundRect(x, y, w, h, radii) method steps are:
  - 1. If any of x, y, w, or h are infinite or NaN, then return.
  - 2. If radii is an unrestricted double or DOMPointInit, then set radii to « radii ».
  - 3. If radii is not a list of size one, two, three, or four, then throw a RangeError.
  - 4. Let normalizedRadii be an empty list.
  - 5. For each radius of radii:
    - 5a. If radius is a DOMPointInit:
      - 5a.i. If radius["x"] or radius["y"] is infinite or NaN, then return.
      - 5a.ii. If radius["x"] or radius["y"] is negative, then throw a RangeError.
      - 5a.iii. Otherwise, append radius to normalizedRadii.
    - 5b. If radius is a unrestricted double:
      - 5b.i. If radius is infinite or NaN, then return.
      - 5b.ii. If radius is negative, then throw a RangeError.
      - 5b.iii. Otherwise, append «[ "x" → radius, "y" → radius ]» to normalizedRadii.
  - 6. Let upperLeft, upperRight, lowerRight, and lowerLeft be null.
  - 7. If normalizedRadii's size is 4, then set upperLeft to normalizedRadii[0], set upperRight to normalizedRadii[1], set lowerRight to normalizedRadii[2], and set lowerLeft to normalizedRadii[3].
  - 8. If normalizedRadii's size is 3, then set upperLeft to normalizedRadii[0], set upperRight and lowerLeft to normalizedRadii[1], and set lowerRight to normalizedRadii[2].
  - 9. If normalizedRadii's size is 2, then set upperLeft and lowerRight to normalizedRadii[0] and set upperRight and lowerLeft to normalizedRadii[1].
  - 10. If normalizedRadii's size is 1, then set upperLeft, upperRight, lowerRight, and lowerLeft to normalizedRadii[0].
  - 11. Corner curves must not overlap. Scale all radii to prevent this:
    - 11a. Let top be upperLeft["x"] + upperRight["x"].
    - 11b. Let right be upperRight["y"] + lowerRight["y"].
    - 11c. Let bottom be lowerRight["x"] + lowerLeft["x"].
    - 11d. Let left be upperLeft["y"] + lowerLeft["y"].
    - 11e. Let scale be the minimum value of the ratios w / top, h / right, w / bottom, h / left.
    - 11f. If scale is less than 1, then set the x and y members of upperLeft, upperRight, lowerLeft, and lowerRight to their current values multiplied by scale.
  - 12. Create a new subpath:
    - 12a. Move to the point (x + upperLeft["x"], y).
    - 12b. Draw a straight line to the point (x + w − upperRight["x"], y).
    - 12c. Draw an arc to the point (x + w, y + upperRight["y"]).
    - 12d. Draw a straight line to the point (x + w, y + h − lowerRight["y"]).
    - 12e. Draw an arc to the point (x + w − lowerRight["x"], y + h).
    - 12f. Draw a straight line to the point (x + lowerLeft["x"], y + h).
    - 12g. Draw an arc to the point (x, y + h − lowerLeft["y"]).
    - 12h. Draw a straight line to the point (x, y + upperLeft["y"]).
    - 12i. Draw an arc to the point (x + upperLeft["x"], y).
  - 13. Mark the subpath as closed.
  - 14. Create a new subpath with the point (x, y) as the only point in the subpath.

###### [4.12.5.1.7 Path2D objects](https://html.spec.whatwg.org/#path2d-objects)

Status: reviewed (2026-11-01)
> `Path2D`.
> Fixed: the path from SVG path data, and an added path, didn't end with a new subpath at their last point, and adding an empty path wasn't skipped.

- [x] [Path2D.Path2D(path)](https://html.spec.whatwg.org/#dom-path2d) — The Path2D(path) constructor, when invoked, must run these steps:
  - 1. Let output be a new Path2D object.
  - 2. If path is not given, then return output.
  - 3. If path is a Path2D object, then add all subpaths of path to output and return output. (In other words, it returns a copy of the argument.)
  - 4. Let svgPath be the result of parsing and interpreting path according to SVG 2's rules for path data. [SVG]
  - 5. Let (x, y) be the last point in svgPath.
  - 6. Add all the subpaths, if any, from svgPath to output.
  - 7. Create a new subpath in output with (x, y) as the only point in the subpath.
  - 8. Return output.

- [x] [Path2D.addPath(path, transform)](https://html.spec.whatwg.org/#dom-path2d-addpath) — The addPath(path, transform) method, when invoked on a Path2D object a, must run these steps:
  - 1. If the Path2D object path has no subpaths, then return.
  - 2. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
  - 3. If one or more of matrix's m11 element, m12 element, m21 element, m22 element, m41 element, or m42 element are infinite or NaN, then return.
  - 4. Create a copy of all the subpaths in path. Let c be this copy.
  - 5. Transform all the coordinates and lines in c by the transform matrix matrix.
  - 6. Let (x, y) be the last point in the last subpath of c.
  - 7. Add all the subpaths in c to a.
  - 8. Create a new subpath in a with (x, y) as the only point in the subpath.

###### [4.12.5.1.8 Transformations](https://html.spec.whatwg.org/#transformations)

Status: reviewed (2026-11-01)
> The transform methods in `CanvasRenderingContext2DBase`; nothing while the transform can't be inverted (see `DIFFERENCES.md`).
<!-- definitions: current-transformation-matrix -->

- [x] [CanvasTransform.scale(x, y)](https://html.spec.whatwg.org/#dom-context-2d-scale) — The scale(x, y) method, when invoked, must run these steps:
  - 1. If either of the arguments are infinite or NaN, then return.
  - 2. Add the scaling transformation described by the arguments to the current transformation matrix. The x argument represents the scale factor in the horizontal direction and the y argument represents the scale factor in the vertical direction. The factors are multiples.

- [x] [CanvasTransform.rotate(angle)](https://html.spec.whatwg.org/#dom-context-2d-rotate) — The rotate(angle) method, when invoked, must run these steps:
  - 1. If angle is infinite or NaN, then return.
  - 2. Add the rotation transformation described by the argument to the current transformation matrix. The angle argument represents a clockwise rotation angle expressed in radians.

- [x] [CanvasTransform.translate(x, y)](https://html.spec.whatwg.org/#dom-context-2d-translate) — The translate(x, y) method, when invoked, must run these steps:
  - 1. If either of the arguments are infinite or NaN, then return.
  - 2. Add the translation transformation described by the arguments to the current transformation matrix. The x argument represents the translation distance in the horizontal direction and the y argument represents the translation distance in the vertical direction. The arguments are in coordinate space units.

- [x] [CanvasTransform.transform(a, b, c, d, e, f)](https://html.spec.whatwg.org/#dom-context-2d-transform) — The transform(a, b, c, d, e, f) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Replace the current transformation matrix with the result of multiplying the current transformation matrix with the matrix described by:

- [x] [CanvasTransform.getTransform()](https://html.spec.whatwg.org/#dom-context-2d-gettransform) — The getTransform() method, when invoked, must return a newly created DOMMatrix representing a copy of the current transformation matrix matrix of the context.

- [x] [CanvasTransform.setTransform(a, b, c, d, e, f)](https://html.spec.whatwg.org/#dom-context-2d-settransform) — The setTransform(a, b, c, d, e, f) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Reset the current transformation matrix to the matrix described by:

- [x] [CanvasTransform.setTransform(transform)](https://html.spec.whatwg.org/#dom-context-2d-settransform-matrix) — The setTransform(transform) method, when invoked, must run these steps:
  - 1. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
  - 2. If one or more of matrix's m11 element, m12 element, m21 element, m22 element, m41 element, or m42 element are infinite or NaN, then return.
  - 3. Reset the current transformation matrix to matrix.

- [x] [CanvasTransform.resetTransform()](https://html.spec.whatwg.org/#dom-context-2d-resettransform) — The resetTransform() method, when invoked, must reset the current transformation matrix to the identity matrix.

###### [4.12.5.1.9 Image sources for 2D rendering contexts](https://html.spec.whatwg.org/#image-sources-for-2d-rendering-contexts)

Status: reviewed (2026-11-01)
> Checking the usability of an image is in each `DrawImage` and `CreatePattern` overload; there are no video, `ImageBitmap`, `OffscreenCanvas` or `VideoFrame` sources, and images are always origin-clean (see `KNOWN-ISSUES.md`).
> Fixed: drawing an image that failed to load (rather than to decode) didn't throw.

- [x] [check the usability of the image argument](https://html.spec.whatwg.org/#check-the-usability-of-the-image-argument) — To check the usability of the image argument, where image is a CanvasImageSource object, run these steps:
  - 1. Switch on image: HTMLOrSVGImageElement If image's current request's state is broken, then throw an "InvalidStateError" DOMException. If image is not fully decodable, then return bad. If image has a natural width or natural height (or both) equal to zero, then return bad. HTMLVideoElementIf image's readyState attribute is either HAVE_NOTHING or HAVE_METADATA, then return bad.HTMLCanvasElementOffsc…
  - 2. Return good.

- [x] [is not origin-clean](https://html.spec.whatwg.org/#the-image-argument-is-not-origin-clean) — An object image is not origin-clean if, switching on image's type:
  - • HTMLOrSVGImageElement: image's current request's image data is CORS-cross-origin.
  - • HTMLVideoElement: image's media data is CORS-cross-origin.
  - • HTMLCanvasElement / ImageBitmap / OffscreenCanvas: image's bitmap's origin-clean flag is false.

###### [4.12.5.1.10 Fill and stroke styles](https://html.spec.whatwg.org/#fill-and-stroke-styles)

Status: reviewed (2026-11-01)
> `SetFillStyle`, `SetStrokeStyle`, `CanvasGradient` and `CanvasPattern`; `createPattern()` checks its repetition first (see `DIFFERENCES.md`).
> Fixed: the gradient constructors threw a NotSupportedError, and `addColorStop()` an IndexSizeError, for non-finite arguments rather than a TypeError, and `CanvasPattern.setTransform()` didn't ignore a matrix that isn't finite.
<!-- definitions: concept-canvasfillstrokestyles-fill-style concept-canvasfillstrokestyles-stroke-style concept-canvas-pattern-not-origin-clean -->

- [x] [CanvasFillStrokeStyles.fillStyle](https://html.spec.whatwg.org/#dom-context-2d-fillstyle) — The fillStyle getter steps are:
  - 1. If this's fill style is a CSS color, then return the serialization of that color with HTML-compatible serialization requested.
  - 2. Return this's fill style.

- [x] [The fillStyle setter steps](https://html.spec.whatwg.org/#dom-context-2d-fillstyle) — The fillStyle setter steps are:
  - 1. If the given value is a string:
    - 1a. Let context be this's canvas attribute's value, if that is an element; otherwise null.
    - 1b. Let parsedValue be the result of parsing the given value with context if non-null.
    - 1c. If parsedValue is failure, then return.
    - 1d. Set this's fill style to parsedValue.
    - 1e. Return.
  - 2. If the given value is a CanvasPattern object that is marked as not origin-clean, then set this's origin-clean flag to false.
  - 3. Set this's fill style to the given value.

- [x] [CanvasFillStrokeStyles.strokeStyle](https://html.spec.whatwg.org/#dom-context-2d-strokestyle) — The strokeStyle getter steps are:
  - 1. If this's stroke style is a CSS color, then return the serialization of that color with HTML-compatible serialization requested.
  - 2. Return this's stroke style.

- [x] [The strokeStyle setter steps](https://html.spec.whatwg.org/#dom-context-2d-strokestyle) — The strokeStyle setter steps are:
  - 1. If the given value is a string:
    - 1a. Let context be this's canvas attribute's value, if that is an element; otherwise null.
    - 1b. Let parsedValue be the result of parsing the given value with context if non-null.
    - 1c. If parsedValue is failure, then return.
    - 1d. Set this's stroke style to parsedValue.
    - 1e. Return.
  - 2. If the given value is a CanvasPattern object that is marked as not origin-clean, then set this's origin-clean flag to false.
  - 3. Set this's stroke style to the given value.

- [x] [CanvasGradient.addColorStop(offset, color)](https://html.spec.whatwg.org/#dom-canvasgradient-addcolorstop) — The addColorStop(offset, color) method on the CanvasGradient, when invoked, must run these steps:
  - 1. If the offset is less than 0 or greater than 1, then throw an "IndexSizeError" DOMException.
  - 2. Let parsed color be the result of parsing color.
  - 3. If parsed color is failure, throw a "SyntaxError" DOMException.
  - 4. Place a new stop on the gradient, at offset offset relative to the whole gradient, and with the color parsed color. If multiple stops are added at the same offset on a gradient, then they must be placed in the order added, with the first one closest to the start of the gradient, and each subsequent one infinitesimally further along towards the end point (in effect causing all but the first and la…

- [x] [CanvasFillStrokeStyles.createLinearGradient(x0, y0, x1, y1)](https://html.spec.whatwg.org/#dom-context-2d-createlineargradient) — The createLinearGradient(x0, y0, x1, y1) method takes four arguments that represent the start point (x0, y0) and end point (x1, y1) of the gradient. The method, when invoked, must return a linear CanvasGradient initialized with the specified line.

- [x] [CanvasFillStrokeStyles.createRadialGradient(x0, y0, r0, x1, y1, r1)](https://html.spec.whatwg.org/#dom-context-2d-createradialgradient) — The createRadialGradient(x0, y0, r0, x1, y1, r1) method takes six arguments, the first three representing the start circle with origin (x0, y0) and radius r0, and the last three representing the end circle with origin (x1, y1) and radius r1. The values are in coordinate space units. If either of r0 or r1 are negative, then an "IndexSizeError" DOMException must be thrown. Otherwise, the method, when invoked, must return a radial CanvasGradient initialized with the two specified circles.

- [x] [Radial gradients must be rendered by following these steps](https://html.spec.whatwg.org/#dom-context-2d-createradialgradient) — Radial gradients must be rendered by following these steps:
  - 1. If x0 = x1 and y0 = y1 and r0 = r1, then the radial gradient must paint nothing. Return.
  - 2. Let x(ω) = (x1-x0)ω + x0. Let y(ω) = (y1-y0)ω + y0. Let r(ω) = (r1-r0)ω + r0. Let the color at ω be the color at that position on the gradient (with the colors coming from the interpolation and extrapolation described above).
  - 3. For all values of ω where r(ω) > 0, starting with the value of ω nearest to positive infinity and ending with the value of ω nearest to negative infinity, draw the circumference of the circle with radius r(ω) at position (x(ω), y(ω)), with the color at ω, but only painting on the parts of the bitmap that have not yet been painted on by earlier circles in this step for this rendering of the gradie…

- [x] [CanvasFillStrokeStyles.createConicGradient(startAngle, x, y)](https://html.spec.whatwg.org/#dom-context-2d-createconicgradient) — The createConicGradient(startAngle, x, y) method takes three arguments, the first argument, startAngle, represents the angle in radians at which the gradient begins, and the last two arguments, (x, y), represent the center of the gradient in CSS pixels. The method, when invoked, must return a conic CanvasGradient initialized with the specified center and angle.

- [x] [CanvasFillStrokeStyles.createPattern(image, repetition)](https://html.spec.whatwg.org/#dom-context-2d-createpattern) — The createPattern(image, repetition) method, when invoked, must run these steps:
  - 1. Let usability be the result of checking the usability of image.
  - 2. If usability is bad, then return null.
  - 3. Assert: usability is good.
  - 4. If repetition is the empty string, then set it to "repeat".
  - 5. If repetition is not identical to one of "repeat", "repeat-x", "repeat-y", or "no-repeat", then throw a "SyntaxError" DOMException.
  - 6. Let pattern be a new CanvasPattern object with the image image and the repetition behavior given by repetition.
  - 7. If image is not origin-clean, then mark pattern as not origin-clean.
  - 8. Return pattern.

- [x] [CanvasPattern.setTransform(transform)](https://html.spec.whatwg.org/#dom-canvaspattern-settransform) — The setTransform(transform) method, when invoked, must run these steps:
  - 1. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
  - 2. If one or more of matrix's m11 element, m12 element, m21 element, m22 element, m41 element, or m42 element are infinite or NaN, then return.
  - 3. Reset the pattern's transformation matrix to matrix.

- [x] [When a pattern](https://html.spec.whatwg.org/#dom-canvaspattern-settransform) — When a pattern is to be rendered within an area, the user agent must run the following steps to determine what is rendered:
  - 1. Create an infinite transparent black bitmap.
  - 2. Place a copy of the image on the bitmap, anchored such that its top left corner is at the origin of the coordinate space, with one coordinate space unit per CSS pixel of the image, then place repeated copies of this image horizontally to the left and right, if the repetition behavior is "repeat-x", or vertically up and down, if the repetition behavior is "repeat-y", or in all four directions all…
  - 3. Transform the resulting bitmap according to the pattern's transformation matrix.
  - 4. Transform the resulting bitmap again, this time according to the current transformation matrix.
  - 5. Replace any part of the image outside the area in which the pattern is to be rendered with transparent black.
  - 6. The resulting bitmap is what is to be rendered, with the same origin and same scale.

###### [4.12.5.1.11 Drawing rectangles to the bitmap](https://html.spec.whatwg.org/#drawing-rectangles-to-the-bitmap)

Status: reviewed (2026-11-01)
> `ClearRect`, `FillRect` and `StrokeRect`.
> Fixed: `fillRect()` with one of the width and height zero drew (and with `copy`, cleared the canvas).

- [x] [CanvasRect.clearRect(x, y, w, h)](https://html.spec.whatwg.org/#dom-context-2d-clearrect) — The clearRect(x, y, w, h) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Let pixels be the set of pixels in the specified rectangle that also intersect the current clipping region.
  - 3. Clear the pixels in pixels to a transparent black, erasing any previous image.

- [x] [CanvasRect.fillRect(x, y, w, h)](https://html.spec.whatwg.org/#dom-context-2d-fillrect) — The fillRect(x, y, w, h) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. If either w or h are zero, then return.
  - 3. Paint the specified rectangular area using this's fill style.

- [x] [CanvasRect.strokeRect(x, y, w, h)](https://html.spec.whatwg.org/#dom-context-2d-strokerect) — The strokeRect(x, y, w, h) method, when invoked, must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Take the result of tracing the path described below, using the CanvasPathDrawingStyles interface's line styles, and fill it with this's stroke style.

###### [4.12.5.1.12 Drawing text to the bitmap](https://html.spec.whatwg.org/#drawing-text-to-the-bitmap)

Status: reviewed (2026-11-01)
> `DrawTextInternal`, `DrawTextUnchecked` and `MeasureTextInternal`.
<!-- definitions: dom-context-2d-stroketext dom-textmetrics-width dom-textmetrics-actualboundingboxleft dom-textmetrics-actualboundingboxright dom-textmetrics-fontboundingboxascent dom-textmetrics-fontboundingboxdescent dom-textmetrics-actualboundingboxascent dom-textmetrics-actualboundingboxdescent dom-textmetrics-emheightascent dom-textmetrics-emheightdescent dom-textmetrics-hangingbaseline dom-textmetrics-alphabeticbaseline dom-textmetrics-ideographicbaseline -->

- [x] [CanvasText.fillText(text, x, y, maxWidth)](https://html.spec.whatwg.org/#dom-context-2d-filltext) — The fillText(text, x, y, maxWidth) and strokeText(text, x, y, maxWidth) methods render the given text at the given (x, y) coordinates ensuring that the text isn't wider than maxWidth if specified, using the current font, textAlign, and textBaseline values. Specifically, when the methods are invoked…
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Run the text preparation algorithm, passing it text, the object implementing the CanvasText interface, and, if the maxWidth argument was provided, that argument. Let glyphs be the result.
  - 3. Move all the shapes in glyphs to the right by x CSS pixels and down by y CSS pixels.
  - 4. Paint the shapes given in glyphs, as transformed by the current transformation matrix, with each CSS pixel in the coordinate space of glyphs mapped to one coordinate space unit. For fillText(), this's fill style must be applied to the shapes and this's stroke style must be ignored. For strokeText(), the reverse holds: this's stroke style must be applied to the result of tracing the shapes using t…

- [x] [CanvasText.measureText(text)](https://html.spec.whatwg.org/#dom-context-2d-measuretext) — The measureText(text) method steps are to run the text preparation algorithm, passing it text and the object implementing the CanvasText interface, and then using the returned inline box return a new TextMetrics object with members behaving as described in the following list: [CSS]

###### [4.12.5.1.13 Drawing paths to the canvas](https://html.spec.whatwg.org/#drawing-paths-to-the-canvas)

Status: reviewed (2026-11-01)
> `FillInternal`, `StrokeInternal`, `ClipInternal`, `IsPointInPathInternal` and `IsPointInStrokeInternal`.
<!-- definitions: current-default-path clipping-region -->

- [x] [CanvasDrawPath.beginPath()](https://html.spec.whatwg.org/#dom-context-2d-beginpath) — The beginPath() method steps are to empty the list of subpaths in this's current default path so that it once again has zero subpaths.

- [x] [intended path](https://html.spec.whatwg.org/#intended-path) — Where the following method definitions use the term intended path for a Path2D-or-null path, it means path itself if it is a Path2D object, or the current default path otherwise.

- [x] [CanvasDrawPath.fill(fillRule)](https://html.spec.whatwg.org/#dom-context-2d-fill) — The fill(fillRule) method steps are to run the fill steps given this, null, and fillRule.

- [x] [CanvasDrawPath.fill(path, fillRule)](https://html.spec.whatwg.org/#dom-context-2d-fill-path) — The fill(path, fillRule) method steps are to run the fill steps given this, path, and fillRule.

- [x] [fill steps](https://html.spec.whatwg.org/#fill-steps) — The fill steps, given a CanvasDrawPath context, a Path2D-or-null path, and a fill rule fillRule, are to fill all the subpaths of the intended path for path, using context's fill style, and using the fill rule indicated by fillRule. Open subpaths must be implicitly closed when being filled (without affecting the actual subpaths).

- [x] [CanvasDrawPath.stroke()](https://html.spec.whatwg.org/#dom-context-2d-stroke) — The stroke() method steps are to run the stroke steps given this and null.

- [x] [CanvasDrawPath.stroke(path)](https://html.spec.whatwg.org/#dom-context-2d-stroke-path) — The stroke(path) method steps are to run the stroke steps given this and path.

- [x] [stroke steps](https://html.spec.whatwg.org/#stroke-steps) — The stroke steps, given a CanvasDrawPath context and a Path2D-or-null path, are to trace the intended path for path, using context's line styles as set by its CanvasPathDrawingStyles mixin, and then fill the resulting path using context's stroke style, using the nonzero winding rule.

- [x] [CanvasDrawPath.clip(fillRule)](https://html.spec.whatwg.org/#dom-context-2d-clip) — The clip(fillRule) method steps are to run the clip steps given this, null, and fillRule.

- [x] [CanvasDrawPath.clip(path, fillRule)](https://html.spec.whatwg.org/#dom-context-2d-clip-path) — The clip(path, fillRule) method steps are to run the clip steps given this, path, and fillRule.

- [x] [clip steps](https://html.spec.whatwg.org/#clip-steps) — The clip steps, given a CanvasDrawPath context, a Path2D-or-null path, and a fill rule fillRule, are to create a new clipping region by calculating the intersection of context's current clipping region and the area described by the intended path for path, using the fill rule indicated by fillRule. Open subpaths must be implicitly closed when computing the clipping region, without affecting the actual subpaths. The new clipping region replaces the current clipping region.

- [x] [CanvasDrawPath.isPointInPath(x, y, fillRule)](https://html.spec.whatwg.org/#dom-context-2d-ispointinpath) — The isPointInPath(x, y, fillRule) method steps are to return the result of the is point in path steps given this, null, x, y, and fillRule.

- [x] [CanvasDrawPath.isPointInPath(path, x, y, fillRule)](https://html.spec.whatwg.org/#dom-context-2d-ispointinpath-path) — The isPointInPath(path, x, y, fillRule) method steps are to return the result of the is point in path steps given this, path, x, y, and fillRule.

- [x] [is point in path steps](https://html.spec.whatwg.org/#is-point-in-path-steps) — The is point in path steps, given a CanvasDrawPath context, a Path2D-or-null path, two numbers x and y, and a fill rule fillRule, are:
  - 1. If x or y are infinite or NaN, then return false.
  - 2. If the point given by the x and y coordinates, when treated as coordinates in the canvas coordinate space unaffected by the current transformation, is inside the intended path for path as determined by the fill rule indicated by fillRule, then return true. Open subpaths must be implicitly closed when computing the area inside the path, without affecting the actual subpaths. Points on the path its…
  - 3. Return false.

- [x] [CanvasDrawPath.isPointInStroke(x, y)](https://html.spec.whatwg.org/#dom-context-2d-ispointinstroke) — The isPointInStroke(x, y) method steps are to return the result of the is point in stroke steps given this, null, x, and y.

- [x] [CanvasDrawPath.isPointInStroke(path, x, y)](https://html.spec.whatwg.org/#dom-context-2d-ispointinstroke-path) — The isPointInStroke(path, x, y) method steps are to return the result of the is point in stroke steps given this, path, x, and y.

- [x] [is point in stroke steps](https://html.spec.whatwg.org/#is-point-in-stroke-steps) — The is point in stroke steps, given a CanvasDrawPath context, a Path2D-or-null path, and two numbers x and y, are:
  - 1. If x or y are infinite or NaN, then return false.
  - 2. If the point given by the x and y coordinates, when treated as coordinates in the canvas coordinate space unaffected by the current transformation, is inside the path that results from tracing the intended path for path, using the nonzero winding rule, and using context's line styles as set by its CanvasPathDrawingStyles mixin, then return true. Points on the resulting path must be considered to…
  - 3. Return false.

###### [4.12.5.1.14 Drawing focus rings](https://html.spec.whatwg.org/#drawing-focus-rings-and-scrolling-paths-into-view)

Status: reviewed (2026-11-01)
> `CanvasRenderingContext2D::DrawFocusIfNeededInternal`; the user isn't informed, as there's no accessibility.

- [x] [CanvasUserInterface.drawFocusIfNeeded(element)](https://html.spec.whatwg.org/#dom-context-2d-drawfocusifneeded) — The drawFocusIfNeeded(element) method steps are to draw focus if needed given this, element, and this's current default path.

- [x] [CanvasUserInterface.drawFocusIfNeeded(path, element)](https://html.spec.whatwg.org/#dom-context-2d-drawfocusifneeded-path-element) — The drawFocusIfNeeded(path, element) method steps are to draw focus if needed given this, element, and path.

- [x] [draw focus if needed](https://html.spec.whatwg.org/#draw-focus-if-needed) — To draw focus if needed, given an object implementing CanvasUserInterface context, an element element, and a path path:
  - 1. If element is not focused or is not a descendant of context's canvas element, then return.
  - 2. Draw a focus ring of the appropriate style along path, following platform conventions. The focus ring should not be subject to the shadow effects, the global alpha, the current compositing and blending operator, the fill style, the stroke style, or any of the members in the CanvasPathDrawingStyles, CanvasTextDrawingStyles interfaces, but should be subject to the clipping region. (The effect of tr…
  - 3. Inform the user that the focus is at the location given by the intended path. User agents may wait until the next time the event loop reaches its update the rendering step to optionally inform the user.

###### [4.12.5.1.15 Drawing images](https://html.spec.whatwg.org/#drawing-images)

Status: reviewed (2026-11-01)
> The `DrawImage` overloads.
> Fixed: drawing a canvas didn't ignore non-finite arguments (an empty canvas threw instead).

- [x] [When the drawImage() method is invoked, the user agent must run these steps](https://html.spec.whatwg.org/#dom-context-2d-drawimage) — When the drawImage() method is invoked, the user agent must run these steps:
  - 1. If any of the arguments are infinite or NaN, then return.
  - 2. Let usability be the result of checking the usability of image.
  - 3. If usability is bad, then return (without drawing anything).
  - 4. Establish the source and destination rectangles as follows: If not specified, the dw and dh arguments must default to the values of sw and sh, interpreted such that one CSS pixel in the image is treated as one unit in the output bitmap's coordinate space. If the sx, sy, sw, and sh arguments are omitted, then they must default to 0, 0, the image's natural width in image pixels, and the image's nat…
  - 5. If one of the sw or sh arguments is zero, then return. Nothing is painted.
  - 6. Paint the region of the image argument specified by the source rectangle on the region of the rendering context's output bitmap specified by the destination rectangle, after applying the current transformation matrix to the destination rectangle. The image data must be processed in the original direction, even if the dimensions given are negative. When scaling up, if the imageSmoothingEnabled att…
  - 7. If image is not origin-clean, then set the CanvasRenderingContext2D's origin-clean flag to false.

###### [4.12.5.1.16 Pixel manipulation](https://html.spec.whatwg.org/#pixel-manipulation)

Status: reviewed (2026-11-01)
> `CreateImageData`, `GetImageData` and `PutImageData`; image data can't be detached, as there's no script to transfer it.
> Fixed: `createImageData(imageData)` took the other image data's colour space rather than the context's, and `putImageData()` couldn't throw.

- [x] [CanvasImageData.createImageData(sw, sh, settings)](https://html.spec.whatwg.org/#dom-context-2d-createimagedata) — The createImageData(sw, sh, settings) method steps are:
  - 1. If one or both of sw and sh are zero, then throw an "IndexSizeError" DOMException.
  - 2. Let newImageData be a new ImageData object.
  - 3. Initialize newImageData given the absolute magnitude of sw, the absolute magnitude of sh, settings, and defaultColorSpace set to this's color space.
  - 4. Initialize the image data of newImageData to transparent black.
  - 5. Return newImageData.

- [x] [CanvasImageData.createImageData(imageData)](https://html.spec.whatwg.org/#dom-context-2d-createimagedata-imagedata) — The createImageData(imageData) method steps are:
  - 1. Let newImageData be a new ImageData object.
  - 2. Let settings be the ImageDataSettings object «[ "colorSpace" → this's colorSpace, "pixelFormat" → this's pixelFormat ]».
  - 3. Initialize newImageData given the value of imageData's width attribute, the value of imageData's height attribute, and settings.
  - 4. Initialize the image data of newImageData to transparent black.
  - 5. Return newImageData.

- [x] [CanvasImageData.getImageData(sx, sy, sw, sh, settings)](https://html.spec.whatwg.org/#dom-context-2d-getimagedata) — The getImageData(sx, sy, sw, sh, settings) method steps are:
  - 1. If either the sw or sh arguments are zero, then throw an "IndexSizeError" DOMException.
  - 2. If the CanvasRenderingContext2D's origin-clean flag is set to false, then throw a "SecurityError" DOMException.
  - 3. Let imageData be a new ImageData object.
  - 4. Initialize imageData given sw, sh, settings, and defaultColorSpace set to this's color space.
  - 5. Let the source rectangle be the rectangle whose corners are the four points (sx, sy), (sx+sw, sy), (sx+sw, sy+sh), (sx, sy+sh).
  - 6. Set the pixel values of imageData to be the pixels of this's output bitmap in the area specified by the source rectangle in the bitmap's coordinate space units, converted from this's color space to imageData's colorSpace using 'relative-colorimetric' rendering intent.
  - 7. Set the pixels values of imageData for areas of the source rectangle that are outside of the output bitmap to transparent black.
  - 8. Return imageData.

- [x] [CanvasImageData.putImageData(imageData, dx, dy)](https://html.spec.whatwg.org/#dom-context-2d-putimagedata-short) — The putImageData(imageData, dx, dy) method steps are to put pixels from an ImageData onto a bitmap, given imageData, this's output bitmap, dx, dy, 0, 0, imageData's width, and imageData's height.

- [x] [CanvasImageData.putImageData(imageData, dx, dy, dirtyX, dirtyY, dirtyWidth, dirtyHeight)](https://html.spec.whatwg.org/#dom-context-2d-putimagedata) — The putImageData(imageData, dx, dy, dirtyX, dirtyY, dirtyWidth, dirtyHeight) method steps are to put pixels from an ImageData onto a bitmap, given imageData, this's output bitmap, dx, dy, dirtyX, dirtyY, dirtyWidth, and dirtyHeight.

- [x] [put pixels from an ImageData onto a bitmap](https://html.spec.whatwg.org/#dom-context2d-putimagedata-common) — To put pixels from an ImageData onto a bitmap, given an ImageData imageData, an output bitmap bitmap, and numbers dx, dy, dirtyX, dirtyY, dirtyWidth, and dirtyHeight:
  - 1. Let buffer be imageData's data attribute value's [[ViewedArrayBuffer]] internal slot.
  - 2. If IsDetachedBuffer(buffer) is true, then throw an "InvalidStateError" DOMException.
  - 3. If dirtyWidth is negative, then let dirtyX be dirtyX+dirtyWidth, and let dirtyWidth be equal to the absolute magnitude of dirtyWidth. If dirtyHeight is negative, then let dirtyY be dirtyY+dirtyHeight, and let dirtyHeight be equal to the absolute magnitude of dirtyHeight.
  - 4. If dirtyX is negative, then let dirtyWidth be dirtyWidth+dirtyX, and let dirtyX be 0. If dirtyY is negative, then let dirtyHeight be dirtyHeight+dirtyY, and let dirtyY be 0.
  - 5. If dirtyX+dirtyWidth is greater than the width attribute of the imageData argument, then let dirtyWidth be the value of that width attribute, minus the value of dirtyX. If dirtyY+dirtyHeight is greater than the height attribute of the imageData argument, then let dirtyHeight be the value of that height attribute, minus the value of dirtyY.
  - 6. If, after those changes, either dirtyWidth or dirtyHeight are negative or zero, then return without affecting any bitmaps.
  - 7. For all integer values of x and y where dirtyX ≤ x < dirtyX+dirtyWidth and dirtyY ≤ y < dirtyY+dirtyHeight, set the pixel with coordinate (dx+x, dy+y) in bitmap to the color of the pixel at coordinate (x, y) in the imageData data structure's bitmap, converted from imageData's colorSpace to the color space of bitmap using 'relative-colorimetric' rendering intent.

###### [4.12.5.1.17 Compositing](https://html.spec.whatwg.org/#compositing)

Status: reviewed (2026-11-01)
> `SetGlobalAlpha` and `SetGlobalCompositeOperation`; the whole-canvas operators draw through a transparency layer.
<!-- definitions: concept-canvas-global-alpha current-compositing-and-blending-operator -->

- [x] [CanvasCompositing.globalAlpha](https://html.spec.whatwg.org/#dom-context-2d-globalalpha) — The globalAlpha getter steps are to return this's global alpha.

- [x] [The globalAlpha setter steps](https://html.spec.whatwg.org/#dom-context-2d-globalalpha) — The globalAlpha setter steps are:
  - 1. If the given value is either infinite, NaN, or not in the range 0.0 to 1.0, then return.
  - 2. Otherwise, set this's global alpha to the given value.

- [x] [CanvasCompositing.globalCompositeOperation](https://html.spec.whatwg.org/#dom-context-2d-globalcompositeoperation) — The globalCompositeOperation getter steps are to return this's current compositing and blending operator.

- [x] [The globalCompositeOperation setter steps](https://html.spec.whatwg.org/#dom-context-2d-globalcompositeoperation) — The globalCompositeOperation setter steps are:
  - 1. If the given value is not identical to any of the values that the <blend-mode> or the <composite-mode> properties are defined to take, then return. [COMPOSITE]
  - 2. Otherwise, set this's current compositing and blending operator to the given value.

###### [4.12.5.1.18 Image smoothing](https://html.spec.whatwg.org/#image-smoothing)

Status: reviewed (2026-11-01)
> `SetImageSmoothingEnabled` and `SetImageSmoothingQuality`.

- [x] [CanvasImageSmoothing.imageSmoothingEnabled](https://html.spec.whatwg.org/#dom-context-2d-imagesmoothingenabled) — The imageSmoothingEnabled attribute, on getting, must return the last value it was set to. On setting, it must be set to the new value. When the object implementing the CanvasImageSmoothing interface is created, the attribute must be set to true.

- [x] [CanvasImageSmoothing.imageSmoothingQuality](https://html.spec.whatwg.org/#dom-context-2d-imagesmoothingquality) — The imageSmoothingQuality attribute, on getting, must return the last value it was set to. On setting, it must be set to the new value. When the object implementing the CanvasImageSmoothing interface is created, the attribute must be set to "low".

###### [4.12.5.1.19 Shadows](https://html.spec.whatwg.org/#shadows)

Status: reviewed (2026-11-01)
> The shadow setters and `ShouldDrawShadows`; shadows are the graphics context's drop shadows.
<!-- definitions: concept-canvasshadowstyles-shadow-color dom-context-2d-shadowoffsetx dom-context-2d-shadowoffsety dom-context-2d-shadowblur -->

- [x] [CanvasShadowStyles.shadowColor](https://html.spec.whatwg.org/#dom-context-2d-shadowcolor) — The shadowColor getter steps are to return the serialization of this's shadow color with HTML-compatible serialization requested.

- [x] [The shadowColor setter steps](https://html.spec.whatwg.org/#dom-context-2d-shadowcolor) — The shadowColor setter steps are:
  - 1. Let context be this's canvas attribute's value, if that is an element; otherwise null.
  - 2. Let parsedValue be the result of parsing the given value with context if non-null.
  - 3. If parsedValue is failure, then return.
  - 4. Set this's shadow color to parsedValue.

- [x] [Shadows are only drawn if](https://html.spec.whatwg.org/#when-shadows-are-drawn) — Shadows are only drawn if the opacity component of the alpha component of the shadow color is nonzero and either the shadowBlur is nonzero, or the shadowOffsetX is nonzero, or the shadowOffsetY is nonzero.

- [x] [When shadows are drawn, they must be rendered as follows](https://html.spec.whatwg.org/#when-shadows-are-drawn) — When shadows are drawn, they must be rendered as follows:
  - 1. Let A be an infinite transparent black bitmap on which the source image for which a shadow is being created has been rendered.
  - 2. Let B be an infinite transparent black bitmap, with a coordinate space and an origin identical to A.
  - 3. Copy the alpha component of A to B, offset by shadowOffsetX in the positive x direction, and shadowOffsetY in the positive y direction.
  - 4. If shadowBlur is greater than 0: User agents may limit values of σ to an implementation-specific maximum value to avoid exceeding hardware limitations during the Gaussian blur operation.
    - 4a. Let σ be half the value of shadowBlur.
    - 4b. Perform a 2D Gaussian Blur on B, using σ as the standard deviation.
  - 5. Set the red, green, and blue components of every pixel in B to the red, green, and blue components (respectively) of the shadow color.
  - 6. Multiply the alpha component of every pixel in B by the alpha component of the shadow color.
  - 7. The shadow is in the bitmap B, and is rendered as part of the drawing model described below.

###### [4.12.5.1.20 Filters](https://html.spec.whatwg.org/#filters)

Status: reviewed (2026-11-01)
> `SetFilterString`, applied through `CanvasFilterContextSwitcher`.
<!-- definitions: concept-canvas-current-filter -->

- [x] [The filter getter steps](https://html.spec.whatwg.org/#dom-context-2d-filter) — The filter getter steps are to return this's current filter.

- [x] [The filter setter steps](https://html.spec.whatwg.org/#dom-context-2d-filter) — The filter setter steps are:
  - 1. If the given value is "none", then set this's current filter to "none" and return.
  - 2. Let parsedValue be the result of parsing the given values as a <filter-value-list>. If any property-independent style sheet syntax like 'inherit' or 'initial' is present, then this parsing must return failure.
  - 3. If parsedValue is failure, then return.
  - 4. Set this's current filter to the given value.

###### [4.12.5.1.21 Working with externally-defined SVG filters](https://html.spec.whatwg.org/#working-with-externally-defined-svg-filters)

Status: reviewed (2026-11-01)
> `url()` filters resolve through the canvas's style, as CSS filters do.

###### [4.12.5.1.22 Drawing model](https://html.spec.whatwg.org/#drawing-model)

Status: reviewed (2026-11-01)
> The graphics context's drawing, with `CanvasFilterContextSwitcher`, shadows, the transparency layers for compositing and the clip.

- [x] [When a shape or image is painted, user agents must follow these steps, in the o…](https://html.spec.whatwg.org/#drawing-model) — When a shape or image is painted, user agents must follow these steps, in the order given (or act as if they do):
  - 1. Render the shape or image onto an infinite transparent black bitmap, creating image A, as described in the previous sections. For shapes, the current fill, stroke, and line styles must be honored, and the stroke must itself also be subjected to the current transformation matrix.
  - 2. Convert the image A to the context's color space.
  - 3. Multiply the alpha component of every pixel in A by global alpha.
  - 4. When the current filter is set to a value other than "none" and all the externally-defined filters it references, if any, are in documents that are currently loaded, then use image A as the input to the current filter, creating image B. If the current filter is a string parseable as a <filter-value-list>, then draw using the current filter in the same manner as SVG. Otherwise, let B be an alias f…
  - 5. When shadows are drawn, render the shadow from image B, using the current shadow styles, creating image C.
  - 6. When shadows are drawn, composite C within the clipping region over the current output bitmap using the current compositing and blending operator.
  - 7. Composite B within the clipping region over the current output bitmap using the current compositing and blending operator.

###### [4.12.5.1.23 Best practices](https://html.spec.whatwg.org/#best-practices)

Status: n/a (advice)

###### [4.12.5.1.24 Examples](https://html.spec.whatwg.org/#examples)

Status: n/a (examples)

##### [4.12.5.2 The ImageBitmap rendering context](https://html.spec.whatwg.org/#the-imagebitmap-rendering-context)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).

###### [4.12.5.2.1 Introduction](https://html.spec.whatwg.org/#introduction-6)

Status: n/a (introduction)

###### [4.12.5.2.2 The ImageBitmapRenderingContext interface](https://html.spec.whatwg.org/#the-imagebitmaprenderingcontext-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-imagebitmaprenderingcontext-output-bitmap concept-imagebitmaprenderingcontext-bitmap-mode concept-imagebitmaprenderingcontext-valid concept-imagebitmaprenderingcontext-blank dom-imagebitmaprenderingcontextsettings-alpha -->

- [x] [ImageBitmapRenderingContext.canvas](https://html.spec.whatwg.org/#dom-imagebitmaprenderingcontext-canvas) — The canvas attribute must return the value it was initialized to when the object was created.

- [x] [alpha](https://html.spec.whatwg.org/#concept-imagebitmaprenderingcontext-alpha) — An ImageBitmapRenderingContext object also has an alpha flag, which can be set to true or false. When an ImageBitmapRenderingContext object has its alpha flag set to false, the contents of the canvas element to which the context is bound are obtained by compositing the context's output bitmap onto an opaque black bitmap of the same size using the source-over compositing operator. If the alpha flag is set to true, then the output bitmap is used as the contents of the canvas element to which the context is bound. [COMPOSITE]

- [x] [set an ImageBitmapRenderingContext's output bitmap](https://html.spec.whatwg.org/#set-an-imagebitmaprenderingcontext's-output-bitmap) — When a user agent is required to set an ImageBitmapRenderingContext's output bitmap, with a context argument that is an ImageBitmapRenderingContext object and an optional argument bitmap that refers to bitmap data, it must run these steps:
  - 1. If a bitmap argument was not provided:
    - 1a. Set context's bitmap mode to blank.
    - 1b. Let canvas be the canvas element to which context is bound.
    - 1c. Set context's output bitmap to be transparent black with a natural width equal to the numeric value of canvas's width attribute and a natural height equal to the numeric value of canvas's height attribute, those values being interpreted in CSS pixels.
    - 1d. Set the output bitmap's origin-clean flag to true.
  - 2. If a bitmap argument was provided:
    - 2a. Set context's bitmap mode to valid.
    - 2b. Set context's output bitmap to refer to the same underlying bitmap data as bitmap, without making a copy.

- [x] [ImageBitmapRenderingContext creation algorithm](https://html.spec.whatwg.org/#imagebitmaprenderingcontext-creation-algorithm) — The ImageBitmapRenderingContext creation algorithm, which is passed a target and options, consists of running these steps:
  - 1. Let settings be the result of converting options to the dictionary type ImageBitmapRenderingContextSettings. (This can throw an exception.)
  - 2. Let context be a new ImageBitmapRenderingContext object.
  - 3. Initialize context's canvas attribute to point to target.
  - 4. Set context's output bitmap to the same bitmap as target's bitmap (so that they are shared).
  - 5. Run the steps to set an ImageBitmapRenderingContext's output bitmap with context.
  - 6. Initialize context's alpha flag to true.
  - 7. Process each of the members of settings as follows: alphaIf false, then set context's alpha flag to false.
  - 8. Return context.

- [x] [ImageBitmapRenderingContext.transferFromImageBitmap(bitmap)](https://html.spec.whatwg.org/#dom-imagebitmaprenderingcontext-transferfromimagebitmap) — The transferFromImageBitmap(bitmap) method, when invoked, must run these steps:
  - 1. Let bitmapContext be the ImageBitmapRenderingContext object on which the transferFromImageBitmap() method was called.
  - 2. If bitmap is null, then run the steps to set an ImageBitmapRenderingContext's output bitmap, with bitmapContext as the context argument and no bitmap argument, then return.
  - 3. If the value of bitmap's [[Detached]] internal slot is set to true, then throw an "InvalidStateError" DOMException.
  - 4. Run the steps to set an ImageBitmapRenderingContext's output bitmap, with the context argument equal to bitmapContext, and the bitmap argument referring to bitmap's underlying bitmap data.
  - 5. Set the value of bitmap's [[Detached]] internal slot to true.
  - 6. Unset bitmap's bitmap data.

##### [4.12.5.3 The OffscreenCanvas interface](https://html.spec.whatwg.org/#the-offscreencanvas-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: offscreencanvas-placeholder offscreencanvas-bitmap offscreencanvas-inherited-lang offscreencanvas-inherited-direction offscreencanvas-context-mode offscreencanvas-context-none offscreencanvas-context-2d offscreencanvas-context-bitmaprenderer offscreencanvas-context-webgl offscreencanvas-context-webgl2 offscreencanvas-context-webgpu offscreencanvas-context-detached dom-offscreencanvas-height image-encode-options-type image-encode-options-quality -->

- [x] [OffscreenCanvas.new OffscreenCanvas(width, height)](https://html.spec.whatwg.org/#dom-offscreencanvas) — The new OffscreenCanvas(width, height) constructor steps are:
  - 1. Initialize the bitmap of this to a rectangular array of transparent black pixels of the dimensions specified by width and height.
  - 2. Initialize the width of this to width.
  - 3. Initialize the height of this to height.
  - 4. Set this's inherited language to explicitly unknown.
  - 5. Set this's inherited direction to "ltr".
  - 6. Let global be the relevant global object of this.
  - 7. If global is a Window object:
    - 7a. Let element be the document element of global's associated Document.
    - 7b. If element is not null:
      - 7b.i. Set the inherited language of this to element's language.
      - 7b.ii. Set the inherited direction of this to element's directionality.

- [x] [OffscreenCanvas objects are transferable. Their transfer steps, given value and…](https://html.spec.whatwg.org/#dom-offscreencanvas) — OffscreenCanvas objects are transferable. Their transfer steps, given value and dataHolder, are as follows:
  - 1. If value's context mode is not equal to none, then throw an "InvalidStateError" DOMException.
  - 2. Set value's context mode to detached.
  - 3. Let width and height be the dimensions of value's bitmap.
  - 4. Let language and direction be the values of value's inherited language and inherited direction.
  - 5. Unset value's bitmap.
  - 6. Set dataHolder.[[Width]] to width and dataHolder.[[Height]] to height.
  - 7. Set dataHolder.[[Language]] to language and dataHolder.[[Direction]] to direction.
  - 8. Set dataHolder.[[PlaceholderCanvas]] to be a weak reference to value's placeholder canvas element, if value has one, or null if it does not.

- [x] [Their transfer-receiving steps, given dataHolder and value,](https://html.spec.whatwg.org/#dom-offscreencanvas) — Their transfer-receiving steps, given dataHolder and value, are:
  - 1. Initialize value's bitmap to a rectangular array of transparent black pixels with width given by dataHolder.[[Width]] and height given by dataHolder.[[Height]].
  - 2. Set value's inherited language to dataHolder.[[Language]] and its inherited direction to dataHolder.[[Direction]].
  - 3. If dataHolder.[[PlaceholderCanvas]] is not null, set value's placeholder canvas element to dataHolder.[[PlaceholderCanvas]] (while maintaining the weak reference semantics).

- [x] [OffscreenCanvas.getContext(contextId, options)](https://html.spec.whatwg.org/#dom-offscreencanvas-getcontext) — The getContext(contextId, options) method of an OffscreenCanvas object, when invoked, must run these steps:
  - 1. If options is not an object, then set options to null.
  - 2. Set options to the result of converting options to a JavaScript value.
  - 3. Run the steps in the cell of the following table whose column header matches this OffscreenCanvas object's context mode and whose row header matches contextId:

- [x] [OffscreenCanvas.width](https://html.spec.whatwg.org/#dom-offscreencanvas-width) — If either the width or height attributes of an OffscreenCanvas object are set (to a new value or to the same value as before) and the OffscreenCanvas object's context mode is 2d, then reset the rendering context to its default state and resize the OffscreenCanvas object's bitmap to the new values of the width and height attributes.

- [x] [OffscreenCanvas.convertToBlob(options)](https://html.spec.whatwg.org/#dom-offscreencanvas-converttoblob) — The convertToBlob(options) method steps are:
  - 1. If the value of this's [[Detached]] internal slot is true, then return a promise rejected with an "InvalidStateError" DOMException.
  - 2. If this's context mode is 2d and the rendering context's output bitmap's origin-clean flag is set to false, then return a promise rejected with a "SecurityError" DOMException.
  - 3. If this's bitmap has no pixels (i.e., either its horizontal dimension or its vertical dimension is zero), then return a promise rejected with an "IndexSizeError" DOMException.
  - 4. Let bitmap be a copy of this's bitmap.
  - 5. Let result be a new promise object.
  - 6. Let global be this's relevant global object.
  - 7. Run these steps in parallel:
    - 7a. Let file be a serialization of bitmap as a file, with options's type and quality if present.
    - 7b. Queue a global task on the canvas blob serialization task source given global to run these steps:
      - 7b.i. If file is null, then reject result with an "EncodingError" DOMException.
      - 7b.ii. Otherwise, resolve result with a new Blob object, created in global's relevant realm, representing file. [FILEAPI]
  - 8. Return result.

- [x] [OffscreenCanvas.transferToImageBitmap()](https://html.spec.whatwg.org/#dom-offscreencanvas-transfertoimagebitmap) — The transferToImageBitmap() method, when invoked, must run the following steps:
  - 1. If the value of this OffscreenCanvas object's [[Detached]] internal slot is set to true, then throw an "InvalidStateError" DOMException.
  - 2. If this OffscreenCanvas object's context mode is set to none, then throw an "InvalidStateError" DOMException.
  - 3. Let image be a newly created ImageBitmap object that references the same underlying bitmap data as this OffscreenCanvas object's bitmap.
  - 4. Set this OffscreenCanvas object's bitmap to reference a newly created bitmap of the same dimensions and color space as the previous bitmap, and with its pixels initialized to transparent black, or opaque black if the rendering context's alpha is false.
  - 5. Return image.

###### [4.12.5.3.1 The offscreen 2D rendering context](https://html.spec.whatwg.org/#the-offscreen-2d-rendering-context)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: associated-offscreencanvas-object -->

- [x] [offscreen 2D context creation algorithm](https://html.spec.whatwg.org/#offscreen-2d-context-creation-algorithm) — The offscreen 2D context creation algorithm, which is passed a target (an OffscreenCanvas object) and optionally some arguments, consists of running the following steps:
  - 1. If the algorithm was passed some arguments, let arg be the first such argument. Otherwise, let arg be undefined.
  - 2. Let settings be the result of converting arg to the dictionary type CanvasRenderingContext2DSettings. (This can throw an exception.)
  - 3. Let context be a new OffscreenCanvasRenderingContext2D object.
  - 4. Set context's associated OffscreenCanvas object to target.
  - 5. Run the canvas settings output bitmap initialization algorithm, given context and settings.
  - 6. Set context's output bitmap to a newly created bitmap with the dimensions specified by the width and height attributes of target, and set target's bitmap to the same bitmap (so that they are shared).
  - 7. If context's alpha flag is set to true, initialize all the pixels of context's output bitmap to transparent black. Otherwise, initialize the pixels to opaque black.
  - 8. Return context.

- [x] [OffscreenCanvas.canvas](https://html.spec.whatwg.org/#offscreencontext2d-canvas) — The canvas attribute, on getting, must return this OffscreenCanvasRenderingContext2D's associated OffscreenCanvas object.

##### [4.12.5.4 Color spaces and color space conversion](https://html.spec.whatwg.org/#colour-spaces-and-colour-correction)

Status: reviewed (2026-11-01)
> Drawing is sRGB; `display-p3` is accepted and drawn as sRGB (see `KNOWN-ISSUES.md`).
<!-- definitions: dom-predefinedcolorspace-srgb dom-predefinedcolorspace-srgb-linear dom-predefinedcolorspace-display-p3 dom-predefinedcolorspace-display-p3-linear -->

##### [4.12.5.5 Serializing bitmaps to a file](https://html.spec.whatwg.org/#serialising-bitmaps-to-a-file)

Status: reviewed (2026-11-01)
> `ImageBuffer::ToData` and `ToDataURL`, through `MIMETypeRegistry`'s encoders (see `DIFFERENCES.md`).
<!-- definitions: a-serialisation-of-the-bitmap-as-a-file -->

##### [4.12.5.6 Security with canvas elements](https://html.spec.whatwg.org/#security-with-canvas-elements)

Status: reviewed (2026-11-01)
> Canvases are always origin-clean (FIXME, see `KNOWN-ISSUES.md`).

##### [4.12.5.7 Premultiplied alpha and the 2D rendering context](https://html.spec.whatwg.org/#premultiplied-alpha-and-the-2d-rendering-context)

Status: reviewed (2026-11-01)
> The buffer is premultiplied; `GetImageData` and `PutImageData` convert to and from unpremultiplied pixels.
<!-- definitions: concept-premultiplied-alpha convert-to-premultiplied convert-from-premultiplied -->
