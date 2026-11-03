# HTML review checklist: 4.8 Embedded content

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 22 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html embedded-content` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.8 Embedded content](https://html.spec.whatwg.org/#embedded-content)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.8.1 The picture element](https://html.spec.whatwg.org/#the-picture-element)

Status: reviewed (2026-11-01)
> `HTMLPictureElement`: `SourcesChanged` counts a relevant mutation for its images.

#### [4.8.2 The source element](https://html.spec.whatwg.org/#the-source-element)

Status: reviewed (2026-11-01)
> `HTMLSourceElement`: its attributes are reflected, and changes to `srcset`, `sizes`, `media` and `type` are relevant mutations for its picture's images. Media elements don't select resources (video and audio come later).
> A source counts only for the images it precedes, as only they consider it. Fixed: moving a source (`moveBefore()`) wasn't a relevant mutation; moving one into a picture counts too (see `DIFFERENCES.md`).
<!-- definitions: attr-source-type attr-source-media attr-source-srcset attr-source-sizes attr-source-src -->

- [x] [The source HTML element insertion steps, given insertedNode,](https://html.spec.whatwg.org/#the-source-element) — The source HTML element insertion steps, given insertedNode, are:
  - 1. Let parent be insertedNode's parent.
  - 2. If parent is a media element that has no src attribute and whose networkState has the value NETWORK_EMPTY, then invoke that media element's resource selection algorithm.
  - 3. If parent is a picture element, then for each child of parent's children, if child is an img element, then count this as a relevant mutation for child.

- [x] [The source HTML element moving steps, given movedNode, isSubtreeRoot, and oldAn…](https://html.spec.whatwg.org/#the-source-element) — The source HTML element moving steps, given movedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If movedNode's parent is a media element that has no src attribute and whose networkState has the value NETWORK_EMPTY, then invoke that media element's resource selection algorithm.
  - 2. If isSubtreeRoot is true and oldAncestor is a picture element, then for each child of oldAncestor's children: if child is an img element, then count this as a relevant mutation for child.

- [x] [The source HTML element removing steps, given removedNode, isSubtreeRoot, and o…](https://html.spec.whatwg.org/#the-source-element) — The source HTML element removing steps, given removedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If isSubtreeRoot is true and oldAncestor is a picture element, then for each child of oldAncestor's children: if child is an img element, then count this as a relevant mutation for child.

#### [4.8.3 The img element](https://html.spec.whatwg.org/#the-img-element)

Status: reviewed (2026-11-01)
> `HTMLImageElement`. Fixed: `decoding` wasn't limited to its known values. `decode()`, `Image()` and `sizes=auto` aren't implemented (see `KNOWN-ISSUES.md`); the next page has the image data's processing.
<!-- definitions: concept-img-dimension-attribute-source attr-img-src attr-img-srcset attr-img-alt attr-img-sizes attr-img-crossorigin attr-img-referrerpolicy attr-img-decoding attr-img-fetchpriority attr-img-ismap attr-img-controls dom-img-naturalheight -->

- [x] [allows auto-sizes](https://html.spec.whatwg.org/#allows-auto-sizes) — An img element allows auto-sizes if:
  > n/a: no auto sizes (see `KNOWN-ISSUES.md`).

- [x] [When the loading attribute's state is changed to the Eager state, the user agen…](https://html.spec.whatwg.org/#attr-img-loading) — When the loading attribute's state is changed to the Eager state, the user agent must run these steps:
  - 1. Let resumptionSteps be the img element's lazy load resumption steps.
  - 2. If resumptionSteps is null, then return.
  - 3. Set the img's lazy load resumption steps to null.
  - 4. Invoke resumptionSteps.
  > `HTMLImageElement::OnAttributeChanged` loads a deferred image when `loading` stops being lazy.

- [x] [The img HTML element insertion steps, given insertedNode,](https://html.spec.whatwg.org/#the-img-element) — The img HTML element insertion steps, given insertedNode, are:
  - 1. If insertedNode's parent is a picture element, then, count this as a relevant mutation for insertedNode.

- [x] [The img HTML element moving steps, given movedNode, isSubtreeRoot, and oldAnces…](https://html.spec.whatwg.org/#the-img-element) — The img HTML element moving steps, given movedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If isSubtreeRoot is true and oldAncestor is a picture element, then count this as a relevant mutation for movedNode.

- [x] [The img HTML element removing steps, given removedNode, isSubtreeRoot, and oldA…](https://html.spec.whatwg.org/#the-img-element) — The img HTML element removing steps, given removedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If isSubtreeRoot is true and oldAncestor is a picture element, then count this as a relevant mutation for removedNode.
  - 2. Set removedNode's last auto-sizes width to null.

- [x] [Otherwise, the user agent should display some sort of indicator that there is a…](https://html.spec.whatwg.org/#attr-img-loading) — Otherwise, the user agent should display some sort of indicator that there is an image that is not being rendered, and may, if requested by the user, or if so configured, or when required to provide contextual information in response to navigation, provide caption information for the image, derived…
  - 1. If the image has a title attribute whose value is not the empty string, then return the value of that attribute.
  - 2. If the image is a descendant of a figure element that has a child figcaption element, and, ignoring the figcaption element and its descendants, the figure element has no flow content descendants other than inter-element whitespace and the img element, then return the contents of the first such figcaption element.
  - 3. Return nothing. (There is no caption information.)
  > n/a: there's no user interface to provide caption information.

- [x] [HTMLImageElement.crossOrigin](https://html.spec.whatwg.org/#dom-img-crossorigin) — The crossOrigin IDL attribute must reflect the crossorigin content attribute, limited to only known values.

- [x] [HTMLImageElement.referrerPolicy](https://html.spec.whatwg.org/#dom-img-referrerpolicy) — The referrerPolicy IDL attribute must reflect the referrerpolicy content attribute, limited to only known values.

- [x] [HTMLImageElement.decoding](https://html.spec.whatwg.org/#dom-img-decoding) — The decoding IDL attribute must reflect the decoding content attribute, limited to only known values.

- [x] [HTMLImageElement.loading](https://html.spec.whatwg.org/#dom-img-loading) — The loading IDL attribute must reflect the loading content attribute, limited to only known values.

- [x] [HTMLImageElement.fetchPriority](https://html.spec.whatwg.org/#dom-img-fetchpriority) — The fetchPriority IDL attribute must reflect the fetchpriority content attribute, limited to only known values.

- [x] [dimensions](https://html.spec.whatwg.org/#img-dimensions) — To determine the dimensions of an img element image:
  - 1. Let naturalDimensions be null.
  - 2. If image is available, then set naturalDimensions to image's density-corrected natural width and height.
  - 3. Return the result of determining the dimensions of image given naturalDimensions.
  > `Width` and `Height`: the rendered size, else the `width` and `height` attributes, else the image's size.

- [x] [HTMLImageElement.width](https://html.spec.whatwg.org/#dom-img-width) — The width getter steps are to return the width of this's dimensions.

- [x] [HTMLImageElement.height](https://html.spec.whatwg.org/#dom-img-height) — The height getter steps are to return the height of this's dimensions.

- [x] [HTMLImageElement.naturalWidth](https://html.spec.whatwg.org/#dom-img-naturalwidth) — The naturalWidth and naturalHeight getter steps are:
  - 1. If the image is not available, then return 0.
  - 2. Return the respective component of the image's density-corrected natural width and height, in CSS pixels. [CSS]

- [x] [HTMLImageElement.complete](https://html.spec.whatwg.org/#dom-img-complete) — The complete getter steps are:
  - 1. If any of the following are true: both the src attribute and the srcset attribute are omitted;; the srcset attribute is omitted and the src attribute's value is the empty string;; the img element's current request's state is completely available and its pending request is null; or; the img element's current request's state is broken and its pending request is null, then return true.
  - 2. Return false.

- [x] [HTMLImageElement.currentSrc](https://html.spec.whatwg.org/#dom-img-currentsrc) — The currentSrc IDL attribute must return the img element's current request's current URL.

- [x] [HTMLImageElement.decode()](https://html.spec.whatwg.org/#dom-img-decode) — The decode() method, when invoked, must perform the following steps:
  - 1. Let promise be a new promise.
  - 2. Queue a microtask to perform the following steps:
    - 2a. Let global be this's relevant global object.
    - 2b. If any of the following are true: this's node document is not fully active; or; this's current request's state is broken, then reject promise with an "EncodingError" DOMException.
    - 2c. Otherwise, in parallel, wait for one of the following cases to occur, and perform the corresponding actions: This img element's node document stops being fully activeThis img element's current request changes or is mutatedThis img element's current request's state becomes brokenQueue a global task on the DOM manipulation task source with global to reject promise with an "EncodingError" DOMExcepti…
  - 3. Return promise.
  > n/a: not implemented (see `KNOWN-ISSUES.md`).

- [x] [Image(width, height)](https://html.spec.whatwg.org/#dom-image) — A legacy factory function is provided for creating HTMLImageElement objects (in addition to the factory methods from DOM such as createElement()): Image(width, height). When invoked, the legacy factory function must perform the following steps:
  - 1. Let document be the current global object's associated Document.
  - 2. Let img be the result of creating an element given document, "img", and the HTML namespace.
  - 3. If width is given, then set an attribute value for img using "width" and width.
  - 4. If height is given, then set an attribute value for img using "height" and height.
  - 5. Return img.
  > n/a: not implemented (see `KNOWN-ISSUES.md`).
