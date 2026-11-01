# HTML review checklist: 4.8.4 Images

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 19 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html images` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.8.4 Images](https://html.spec.whatwg.org/#images)

Status: unreviewed

##### [4.8.4.1 Introduction](https://html.spec.whatwg.org/#introduction-3)

Status: unreviewed
<!-- definitions: device-pixel-ratio viewport-based-selection art-direction image-format-based-selection -->

###### [4.8.4.1.1 Adaptive images](https://html.spec.whatwg.org/#adaptive-images)

Status: unreviewed

##### [4.8.4.2 Attributes common to source, img, and link elements](https://html.spec.whatwg.org/#attributes-common-to-source-and-img-elements)

Status: unreviewed

###### [4.8.4.2.1 Srcset attributes](https://html.spec.whatwg.org/#srcset-attributes)

Status: unreviewed
<!-- definitions: srcset-attribute width-descriptor width-descriptor-value pixel-density-descriptor pixel-density-descriptor-value -->

- [ ] [image candidate string](https://html.spec.whatwg.org/#image-candidate-string) — An image candidate string consists of the following components, in order, with the further restrictions described below this list:
  - 1. Zero or more ASCII whitespace.
  - 2. A valid non-empty URL that does not start or end with a U+002C COMMA character (,), referencing a non-interactive, optionally animated, image resource that is neither paged nor scripted.
  - 3. Zero or more ASCII whitespace.
  - 4. Zero or one of the following: A width descriptor, consisting of: ASCII whitespace, a valid non-negative integer giving a number greater than zero representing the width descriptor value, and a U+0077 LATIN SMALL LETTER W character.; A pixel density descriptor, consisting of: ASCII whitespace, a valid floating-point number giving a number greater than zero representing the pixel density descriptor…
  - 5. Zero or more ASCII whitespace.

###### [4.8.4.2.2 Sizes attributes](https://html.spec.whatwg.org/#sizes-attributes)

Status: unreviewed
<!-- definitions: sizes-attribute valdef-sizes-auto -->

- [ ] [valid source size list](https://html.spec.whatwg.org/#valid-source-size-list) — A valid source size list is a string that matches the following grammar: [CSSVALUES] [MQ]

##### [4.8.4.3 Processing model](https://html.spec.whatwg.org/#images-processing-model)

Status: unreviewed
<!-- definitions: current-request pending-request image-request img-req-state img-req-url img-req-data img-none img-inc img-all img-error last-selected-source last-auto-sizes-width current-pixel-density preferred-density-corrected-dimensions source-set image-source source-size-2 concept-microsyntax-parse-error -->

- [ ] [available](https://html.spec.whatwg.org/#img-available) — When an image request's state is either partially available or completely available, the image request is said to be available.

- [ ] [fully decodable](https://html.spec.whatwg.org/#img-good) — When an img element's current request's state is completely available and the user agent can decode the media data without errors, then the img element is said to be fully decodable.

- [ ] [use srcset or picture](https://html.spec.whatwg.org/#use-srcset-or-picture) — An img element is said to use srcset or picture if it has a srcset attribute specified or if it has a parent that is a picture element.

- [ ] [density-corrected natural width and height](https://html.spec.whatwg.org/#density-corrected-intrinsic-width-and-height) — To determine the density-corrected natural width and height of an img element img:
  - 1. Let density be img's current request's current pixel density.
  - 2. Let dimensions be img's current request's preferred density-corrected dimensions.
  - 3. If dimensions is not null, then set dimensions's width to dimensions's width divided by density, set dimensions's height to dimensions's height divided by density, and return dimensions.
  - 4. Let intrinsicWidth, intrinsicHeight, and intrinsicRatio be img's intrinsic width, intrinsic height, and intrinsic aspect ratio, if any, respectively.
  - 5. If intrinsicWidth is not absent, then set intrinsicWidth to intrinsicWidth divided by density.
  - 6. If intrinsicHeight is not absent, then set intrinsicHeight to intrinsicHeight divided by density.
  - 7. Return the result of applying the default sizing algorithm with intrinsicWidth, intrinsicHeight, and intrinsicRatio, using a default object size of 300 by 150.

###### [4.8.4.3.1 When to obtain images](https://html.spec.whatwg.org/#when-to-obtain-images)

Status: unreviewed

###### [4.8.4.3.2 Reacting to DOM mutations](https://html.spec.whatwg.org/#reacting-to-dom-mutations)

Status: unreviewed

- [ ] [relevant mutations](https://html.spec.whatwg.org/#relevant-mutations) — The relevant mutations for an img element are as follows:

###### [4.8.4.3.3 The list of available images](https://html.spec.whatwg.org/#the-list-of-available-images)

Status: unreviewed
<!-- definitions: list-of-available-images ignore-higher-layer-caching -->

###### [4.8.4.3.4 Decoding images](https://html.spec.whatwg.org/#decoding-images)

Status: unreviewed
<!-- definitions: img-decoding-process image-decoding-hint -->

###### [4.8.4.3.5 Updating the image data](https://html.spec.whatwg.org/#updating-the-image-data)

Status: unreviewed

- [ ] [update the image data](https://html.spec.whatwg.org/#update-the-image-data) — When the user agent is to update the image data of an img element element, optionally with the restart animation flag set, optionally with the maybe omit events flag set, it must run the following steps:
  - 1. If element's node document is not fully active:
    - 1a. Continue running this algorithm in parallel.
    - 1b. Wait until element's node document is fully active.
    - 1c. If another instance of this algorithm for element was started after this instance (even if it aborted and is no longer running), then return.
    - 1d. Queue a microtask to continue this algorithm.
  - 2. If the user agent cannot support images, or its support for images has been disabled, then abort the image request for the current request and the pending request, set the current request's state to unavailable, set the pending request to null, and return.
  - 3. Let previousURL be the current request's current URL.
  - 4. Let selected source be null and selected pixel density be undefined.
  - 5. If element does not use srcset or picture and it has a src attribute specified whose value is not the empty string, then set selected source to the value of element's src attribute and set selected pixel density to 1.0.
  - 6. Set element's last selected source to selected source.
  - 7. If selected source is not null:
    - 7a. Let urlString be the result of encoding-parsing-and-serializing a URL given selected source, relative to element's node document.
    - 7b. If urlString is failure, then abort this inner set of steps.
    - 7c. Let key be a tuple consisting of urlString, element's crossorigin attribute's mode, and, if that mode is not No CORS, element's node document's origin.
    - 7d. If the list of available images contains an entry for key:
      - 7d.i. Set the ignore higher-layer caching flag for that entry.
      - 7d.ii. Abort the image request for the current request and the pending request.
      - 7d.iii. Set the pending request to null.
      - 7d.iv. Set the current request to a new image request whose image data is that of the entry and whose state is completely available.
      - 7d.v. Prepare the current request for presentation given element.
      - 7d.vi. Set the current request's current pixel density to selected pixel density.
      - 7d.vii. Queue an element task on the DOM manipulation task source given element and the following steps:
        - 7d.vii.A. If restart animation is set, then restart the animation.
        - 7d.vii.B. Set the current request's current URL to urlString.
        - 7d.vii.C. If maybe omit events is not set or previousURL is not equal to urlString, then fire an event named load at element.
      - 7d.viii. Abort the update the image data algorithm.
  - 8. Queue a microtask to perform the rest of this algorithm, allowing the task that invoked this algorithm to continue.
  - 9. If another instance of this algorithm for element was started after this instance (even if it aborted and is no longer running), then return.
  - 10. Let selected source and selected pixel density be the URL and pixel density that results from selecting an image source, respectively.
  - 11. If selected source is null:
    - 11a. Set the current request's state to broken, abort the image request for the current request and the pending request, and set the pending request to null.
    - 11b. Queue an element task on the DOM manipulation task source given element and the following steps:
      - 11b.i. Change the current request's current URL to the empty string.
      - 11b.ii. If all of the following are true: element has a src attribute or it uses srcset or picture; and; maybe omit events is not set or previousURL is not the empty string, then fire an event named error at element.
    - 11c. Return.
  - 12. Let urlString be the result of encoding-parsing-and-serializing a URL given selected source, relative to element's node document.
  - 13. If urlString is failure:
    - 13a. Abort the image request for the current request and the pending request.
    - 13b. Set the current request's state to broken.
    - 13c. Set the pending request to null.
    - 13d. Queue an element task on the DOM manipulation task source given element and the following steps:
      - 13d.i. Change the current request's current URL to selected source.
      - 13d.ii. If maybe omit events is not set or previousURL is not equal to selected source, then fire an event named error at element.
    - 13e. Return.
  - 14. If the pending request is not null and urlString is the same as the pending request's current URL, then return.
  - 15. If urlString is the same as the current request's current URL and the current request's state is partially available:
    - 15a. Abort the image request for the pending request.
    - 15b. If restart animation is set, then queue an element task on the DOM manipulation task source given element to restart the animation.
    - 15c. Return.
  - 16. Abort the image request for the pending request.
  - 17. Set image request to a new image request whose current URL is urlString.
  - 18. If the current request's state is unavailable or broken, then set the current request to image request. Otherwise, set the pending request to image request.
  - 19. Let request be the result of creating a potential-CORS request given urlString, "image", and the current state of element's crossorigin content attribute.
  - 20. Set request's client to element's node document's relevant settings object.
  - 21. If element uses srcset or picture, set request's initiator to "imageset".
  - 22. Set request's referrer policy to the current state of element's referrerpolicy attribute.
  - 23. Set request's priority to the current state of element's fetchpriority attribute.
  - 24. Let delay load event be true if the img's lazy loading attribute is in the Eager state, or if scripting is disabled for the img, and false otherwise.
  - 25. If the will lazy load element steps given the img return true:
    - 25a. Set the img's lazy load resumption steps to the rest of this algorithm starting with the step labeled fetch the image.
    - 25b. Start intersection-observing a lazy loading element for element.
    - 25c. Return.
  - 26. Fetch the image: Fetch request. Return from this algorithm, and run the remaining steps as part of the fetch's processResponse for the response response. The resource obtained in this fashion, if any, is image request's image data. It can be either CORS-same-origin or CORS-cross-origin; this affects the image's interaction with other APIs (e.g., when used on a canvas). When delay load event is tr…
  - 27. As soon as possible, jump to the first applicable entry from the following list: If the resource type is multipart/x-mixed-replace The next task that is queued by the networking task source while the image is being fetched must run the following steps: Each task that is queued by the networking task source while the image is being fetched must update the presentation of the image, but as each new…
    - 27a. If image request is the pending request and at least one body part has been completely decoded, abort the image request for the current request, and upgrade the pending request to the current request.
    - 27b. Otherwise, if image request is the pending request and the user agent is able to determine that image request's image is corrupted in some fatal way such that the image dimensions cannot be obtained, abort the image request for the current request, upgrade the pending request to the current request, and set the current request's state to broken.
    - 27c. Otherwise, if image request is the current request, its state is unavailable, and the user agent is able to determine image request's image's width and height, set the current request's state to partially available.
    - 27d. Otherwise, if image request is the current request, its state is unavailable, and the user agent is able to determine that image request's image is corrupted in some fatal way such that the image dimensions cannot be obtained, set the current request's state to broken.
    - 27a. Set element's current request's state to completely available.
    - 27b. If maybe omit events is not set or previousURL is not equal to urlString, then queue an element task on the DOM manipulation task source given element to fire an event named load at element.
    - 27a. If the user agent is able to determine image request's image's width and height, and image request is the pending request, set image request's state to partially available.
    - 27b. Otherwise, if the user agent is able to determine image request's image's width and height, and image request is the current request, prepare image request for presentation given element and set image request's state to partially available.
    - 27c. Otherwise, if the user agent is able to determine that image request's image is corrupted in some fatal way such that the image dimensions cannot be obtained, and image request is the pending request:
      - 27c.i. Abort the image request for the current request and the pending request.
      - 27c.ii. Upgrade the pending request to the current request.
      - 27c.iii. Set the current request's state to broken.
      - 27c.iv. Fire an event named error at element.
    - 27d. Otherwise, if the user agent is able to determine that image request's image is corrupted in some fatal way such that the image dimensions cannot be obtained, and image request is the current request:
      - 27d.i. Abort the image request for image request.
      - 27d.ii. If maybe omit events is not set or previousURL is not equal to urlString, then fire an event named error at element.
    - 27a. If image request is the pending request, abort the image request for the current request, upgrade the pending request to the current request, and prepare image request for presentation given element.
    - 27b. Set image request to the completely available state.
    - 27c. Add the image to the list of available images using the key key, with the ignore higher-layer caching flag set.
    - 27d. If maybe omit events is not set or previousURL is not equal to urlString, then fire an event named load at element.

- [ ] [abort the image request](https://html.spec.whatwg.org/#abort-the-image-request) — To abort the image request for an image request or null image request means to run the following steps:
  - 1. If image request is null, then return.
  - 2. Forget image request's image data, if any.
  - 3. Abort any instance of the fetching algorithm for image request, discarding any pending tasks generated by that algorithm.

- [ ] [upgrade the pending request to the current request](https://html.spec.whatwg.org/#upgrade-the-pending-request-to-the-current-request) — To upgrade the pending request to the current request for an img element means to run the following steps:
  - 1. Set the img element's current request to the pending request.
  - 2. Set the img element's pending request to null.

###### [4.8.4.3.6 Preparing an image for presentation](https://html.spec.whatwg.org/#preparing-an-image-for-presentation)

Status: unreviewed

- [ ] [prepare an image for presentation](https://html.spec.whatwg.org/#prepare-an-image-for-presentation) — To prepare an image for presentation for an image request req given image element img:
  - 1. Let exifTagMap be the EXIF tags obtained from req's image data, as defined by the relevant codec. [EXIF]
  - 2. Let physicalWidth and physicalHeight be the width and height obtained from req's image data, as defined by the relevant codec.
  - 3. Let dimX be the value of exifTagMap's tag 0xA002 (PixelXDimension).
  - 4. Let dimY be the value of exifTagMap's tag 0xA003 (PixelYDimension).
  - 5. Let resX be the value of exifTagMap's tag 0x011A (XResolution).
  - 6. Let resY be the value of exifTagMap's tag 0x011B (YResolution).
  - 7. Let resUnit be the value of exifTagMap's tag 0x0128 (ResolutionUnit).
  - 8. If all the following are true: dimX is a positive integer;; dimY is a positive integer;; resX is a positive floating-point number;; resY is a positive floating-point number;; physicalWidth × 72 / resX is dimX;; physicalHeight × 72 / resY is dimY;; resUnit is 2 (Inch), then:
    - 8a. If req's image data is CORS-cross-origin, then set img's natural dimensions to dimX and dimY, and scale img's pixel data accordingly.
    - 8b. Otherwise, set req's preferred density-corrected dimensions to a struct with its width set to dimX and its height set to dimY.
  - 9. Update req's img element's presentation appropriately.

###### [4.8.4.3.7 Selecting an image source](https://html.spec.whatwg.org/#selecting-an-image-source)

Status: unreviewed

- [ ] [select an image source](https://html.spec.whatwg.org/#select-an-image-source) — To select an image source given an img element el:
  - 1. Update the source set for el.
  - 2. If el's source set is empty, return null as the URL and undefined as the pixel density.
  - 3. Return the result of selecting an image from el's source set.

- [ ] [select an image source from a source set](https://html.spec.whatwg.org/#select-an-image-source-from-a-source-set) — To select an image source from a source set given a source set sourceSet:
  - 1. If an entry b in sourceSet has the same associated pixel density descriptor as an earlier entry a in sourceSet, then remove entry b. Repeat this step until none of the entries in sourceSet have the same associated pixel density descriptor as an earlier entry.
  - 2. In an implementation-defined manner, choose one image source from sourceSet. Let selectedSource be this choice.
  - 3. Return selectedSource and its associated pixel density.

###### [4.8.4.3.8 Creating a source set from attributes](https://html.spec.whatwg.org/#creating-a-source-set-from-attributes)

Status: unreviewed

- [ ] [create a source set](https://html.spec.whatwg.org/#create-a-source-set) — When asked to create a source set given a string default source, a string srcset, a string sizes, and an element or null img:
  - 1. Let source set be an empty source set.
  - 2. If srcset is not an empty string, then set source set to the result of parsing srcset.
  - 3. Set source set's source size to the result of parsing sizes with img.
  - 4. If default source is not the empty string and source set does not contain an image source with a pixel density descriptor value of 1, and no image source with a width descriptor, append default source to source set.
  - 5. Normalize the source densities of source set.
  - 6. Return source set.

###### [4.8.4.3.9 Updating the source set](https://html.spec.whatwg.org/#updating-the-source-set)

Status: unreviewed

- [ ] [update the source set](https://html.spec.whatwg.org/#update-the-source-set) — When asked to update the source set for a given img or link element el, user agents must do the following:
  - 1. Set el's source set to an empty source set.
  - 2. Let elements be « el ».
  - 3. If el is an img element whose parent node is a picture element, then replace the contents of elements with el's parent node's child elements, retaining relative order.
  - 4. Let img be el if el is an img element, otherwise null.
  - 5. For each child in elements:
    - 5a. If child is el:
      - 5a.i. Let default source be the empty string.
      - 5a.ii. Let srcset be the empty string.
      - 5a.iii. Let sizes be the empty string.
      - 5a.iv. If el is an img element that has a srcset attribute, then set srcset to that attribute's value.
      - 5a.v. Otherwise, if el is a link element that has an imagesrcset attribute, then set srcset to that attribute's value.
      - 5a.vi. If el is an img element that has a sizes attribute, then set sizes to that attribute's value.
      - 5a.vii. Otherwise, if el is a link element that has an imagesizes attribute, then set sizes to that attribute's value.
      - 5a.viii. If el is an img element that has a src attribute, then set default source to that attribute's value.
      - 5a.ix. Otherwise, if el is a link element that has an href attribute, then set default source to that attribute's value.
      - 5a.x. Set el's source set to the result of creating a source set given default source, srcset, sizes, and img.
      - 5a.xi. Return.
    - 5b. If child is not a source element, then continue.
    - 5c. If child does not have a srcset attribute, continue to the next child.
    - 5d. Parse child's srcset attribute and let source set be the returned source set.
    - 5e. If source set has zero image sources, continue to the next child.
    - 5f. If child has a media attribute, and its value does not match the environment, continue to the next child.
    - 5g. Parse child's sizes attribute with img, and let source set's source size be the returned value.
    - 5h. If child has a type attribute, and its value is an unknown or unsupported MIME type, continue to the next child.
    - 5i. If child has width or height attributes, set el's dimension attribute source to child. Otherwise, set el's dimension attribute source to el.
    - 5j. Normalize the source densities of source set.
    - 5k. Set el's source set to source set.
    - 5l. Return.

###### [4.8.4.3.10 Parsing a srcset attribute](https://html.spec.whatwg.org/#parsing-a-srcset-attribute)

Status: unreviewed

- [ ] [parse a srcset attribute](https://html.spec.whatwg.org/#parse-a-srcset-attribute) — When asked to parse a srcset attribute from an element, parse the value of the element's srcset attribute as follows:
  - 1. Let input be the value passed to this algorithm.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Let candidates be an initially empty source set.
  - 4. Splitting loop: Collect a sequence of code points that are ASCII whitespace or U+002C COMMA characters from input given position. If any U+002C COMMA characters were collected, that is a parse error.
  - 5. If position is past the end of input, return candidates.
  - 6. Collect a sequence of code points that are not ASCII whitespace from input given position, and let url be the result.
  - 7. Let descriptors be a new empty list.
  - 8. If url ends with U+002C (,): Otherwise:
    - 8a. Remove all trailing U+002C COMMA characters from url. If this removed more than one character, that is a parse error.
    - 8a. Descriptor tokenizer: Skip ASCII whitespace within input given position.
    - 8b. Let current descriptor be the empty string.
    - 8c. Let state be in descriptor.
    - 8d. Let c be the character at position. Do the following depending on the value of state. For the purpose of this step, "EOF" is a special character representing that position is past the end of input. In descriptor Do the following, depending on the value of c: ASCII whitespaceIf current descriptor is not empty, append current descriptor to descriptors and let current descriptor be the empty string.…
  - 9. Descriptor parser: Let error be no.
  - 10. Let width be absent.
  - 11. Let density be absent.
  - 12. Let future-compat-h be absent.
  - 13. For each descriptor in descriptors, run the appropriate set of steps from the following list: If the descriptor consists of a valid non-negative integer followed by a U+0077 LATIN SMALL LETTER W character If the descriptor consists of a valid floating-point number followed by a U+0078 LATIN SMALL LETTER X character If the descriptor consists of a valid non-negative integer followed by a U+0068 LA…
    - 13a. If the user agent does not support the sizes attribute, let error be yes.
    - 13b. If width and density are not both absent, then let error be yes.
    - 13c. Apply the rules for parsing non-negative integers to the descriptor. If the result is 0, let error be yes. Otherwise, let width be the result.
    - 13a. If width, density and future-compat-h are not all absent, then let error be yes.
    - 13b. Apply the rules for parsing floating-point number values to the descriptor. If the result is less than 0, let error be yes. Otherwise, let density be the result.
    - 13a. If future-compat-h and density are not both absent, then let error be yes.
    - 13b. Apply the rules for parsing non-negative integers to the descriptor. If the result is 0, let error be yes. Otherwise, let future-compat-h be the result.
  - 14. If future-compat-h is not absent and width is absent, let error be yes.
  - 15. If error is still no, then append a new image source to candidates whose URL is url, associated with a width width if not absent and a pixel density density if not absent. Otherwise, there is a parse error.
  - 16. Return to the step labeled splitting loop.

###### [4.8.4.3.11 Parsing a sizes attribute](https://html.spec.whatwg.org/#parsing-a-sizes-attribute)

Status: unreviewed

- [ ] [parse a sizes attribute](https://html.spec.whatwg.org/#parse-a-sizes-attribute) — When asked to parse a sizes attribute from an element element, with an img element or null img:
  - 1. Let unparsed sizes list be the result of parsing a comma-separated list of component values from the value of element's sizes attribute (or the empty string, if the attribute is absent). [CSSSYNTAX]
  - 2. Let size be null.
  - 3. For each unparsed size in unparsed sizes list:
    - 3a. Remove all consecutive <whitespace-token>s from the end of unparsed size. If unparsed size is now empty, then that is a parse error; continue.
    - 3b. If the last component value in unparsed size is a valid non-negative <source-size-value>, then set size to its value and remove the component value from unparsed size. Any CSS function other than the math functions is invalid. Otherwise, there is a parse error; continue.
    - 3c. If size is auto, and img is not null, and img allows auto-sizes:
      - 3c.i. If img is being rendered, then set size to the concrete object size width of img, in CSS pixels, and set img's last auto-sizes width to size.
      - 3c.ii. Otherwise, if img's last auto-sizes width is not null, then set size to img's last auto-sizes width.
    - 3d. Remove all consecutive <whitespace-token>s from the end of unparsed size. If unparsed size is now empty:
      - 3d.i. If this was not the last item in unparsed sizes list, that is a parse error.
      - 3d.ii. If size is not auto, then return size. Otherwise, continue.
    - 3e. Parse the remaining component values in unparsed size as a <media-condition>. If it does not parse correctly, or it does parse correctly but the <media-condition> evaluates to false, continue. [MQ]
    - 3f. If size is not auto, then return size. Otherwise, continue.
  - 4. Return 100vw.

###### [4.8.4.3.12 Normalizing the source densities](https://html.spec.whatwg.org/#normalizing-the-source-densities)

Status: unreviewed

- [ ] [normalize the source densities](https://html.spec.whatwg.org/#normalise-the-source-densities) — When asked to normalize the source densities of a source set source set, the user agent must do the following:
  - 1. Let source size be source set's source size.
  - 2. For each image source in source set:
    - 2a. If the image source has a pixel density descriptor, continue to the next image source.
    - 2b. Otherwise, if the image source has a width descriptor, replace the width descriptor with a pixel density descriptor with a value of the width descriptor value divided by source size and a unit of x.
    - 2c. Otherwise, give the image source a pixel density descriptor of 1x.

###### [4.8.4.3.13 Reacting to environment changes](https://html.spec.whatwg.org/#reacting-to-environment-changes)

Status: unreviewed

- [ ] [react to changes in the environment](https://html.spec.whatwg.org/#img-environment-changes) — The user agent may at any time run the following algorithm to update an img element's image in order to react to changes in the environment. (User agents are not required to ever run this algorithm; for example, if the user is not looking at the page any more, the user agent might want to wait unti…
  - 1. Await a stable state. The synchronous section consists of all the remaining steps of this algorithm until the algorithm says the synchronous section has ended. (Steps in synchronous sections are marked with ⌛.)
  - 2. ⌛ If the img element does not use srcset or picture, its node document is not fully active, it has image data whose resource type is multipart/x-mixed-replace, or its pending request is not null, then return.
  - 3. ⌛ Let selected source and selected pixel density be the URL and pixel density that results from selecting an image source, respectively.
  - 4. ⌛ If selected source is null, then return.
  - 5. ⌛ If selected source and selected pixel density are the same as the element's last selected source and current pixel density, then return.
  - 6. ⌛ Let urlString be the result of encoding-parsing-and-serializing a URL given selected source, relative to the element's node document.
  - 7. ⌛ If urlString is failure, then return.
  - 8. ⌛ Let corsAttributeState be the state of the element's crossorigin content attribute.
  - 9. ⌛ Let origin be the img element's node document's origin.
  - 10. ⌛ Let client be the img element's node document's relevant settings object.
  - 11. ⌛ Let key be a tuple consisting of urlString, corsAttributeState, and, if corsAttributeState is not No CORS, origin.
  - 12. ⌛ Let image request be a new image request whose current URL is urlString.
  - 13. ⌛ Set the element's pending request to image request.
  - 14. End the synchronous section, continuing the remaining steps in parallel.
  - 15. Let finishUpdate be the following steps:
    - 15a. If the img element has experienced relevant mutations since this algorithm started, then set the pending request to null and abort these steps.
    - 15b. Set the img element's last selected source to selected source and the img element's current pixel density to selected pixel density.
    - 15c. Set the image request's state to completely available.
    - 15d. Add the image to the list of available images using the key key, with the ignore higher-layer caching flag set.
    - 15e. Upgrade the pending request to the current request.
    - 15f. Prepare image request for presentation given the img element.
    - 15g. Fire an event named load at the img element.
  - 16. If the list of available images contains an entry for key:
    - 16a. Set image request's image data to that of the entry.
    - 16b. Queue an element task on the DOM manipulation task source given the img element and finishUpdate.
  - 17. Otherwise:
    - 17a. Let request be the result of creating a potential-CORS request given urlString, "image", and corsAttributeState.
    - 17b. Set request's client to client and set request's initiator to "imageset".
    - 17c. Set request's referrer policy to the current state of the element's referrerpolicy attribute.
    - 17d. Set request's priority to the current state of the element's fetchpriority attribute.
    - 17e. Fetch request, with processResponse set to the following steps given response response:
      - 17e.i. If response's unsafe response is a network error or if the image format is unsupported (as determined by applying the image sniffing rules, again as mentioned earlier), or if the user agent is able to determine that image request's image is corrupted in some fatal way such that the image dimensions cannot be obtained, or if the resource type is multipart/x-mixed-replace, then set the pending requ…
      - 17e.ii. Otherwise, response's unsafe response is image request's image data. It can be either CORS-same-origin or CORS-cross-origin; this affects the image's interaction with other APIs (e.g., when used on a canvas).
      - 17e.iii. Queue an element task on the DOM manipulation task source given the img element and finishUpdate.

##### [4.8.4.4 Requirements for providing text to act as an alternative for images](https://html.spec.whatwg.org/#alt)

Status: unreviewed

###### [4.8.4.4.1 General guidelines](https://html.spec.whatwg.org/#general-guidelines)

Status: unreviewed

###### [4.8.4.4.2 A link or button containing nothing but the image](https://html.spec.whatwg.org/#a-link-or-button-containing-nothing-but-the-image)

Status: unreviewed

###### [4.8.4.4.3 A phrase or paragraph with an alternative graphical representation: charts, diagrams, graphs, maps, illustrations](https://html.spec.whatwg.org/#a-phrase-or-paragraph-with-an-alternative-graphical-representation:-charts,-diagrams,-graphs,-maps,-illustrations)

Status: unreviewed

###### [4.8.4.4.4 A short phrase or label with an alternative graphical representation: icons, logos](https://html.spec.whatwg.org/#a-short-phrase-or-label-with-an-alternative-graphical-representation:-icons,-logos)

Status: unreviewed

###### [4.8.4.4.5 Text that has been rendered to a graphic for typographical effect](https://html.spec.whatwg.org/#text-that-has-been-rendered-to-a-graphic-for-typographical-effect)

Status: unreviewed

###### [4.8.4.4.6 A graphical representation of some of the surrounding text](https://html.spec.whatwg.org/#a-graphical-representation-of-some-of-the-surrounding-text)

Status: unreviewed

###### [4.8.4.4.7 Ancillary images](https://html.spec.whatwg.org/#ancillary-images)

Status: unreviewed

###### [4.8.4.4.8 A purely decorative image that doesn't add any information](https://html.spec.whatwg.org/#a-purely-decorative-image-that-doesn't-add-any-information)

Status: unreviewed

###### [4.8.4.4.9 A group of images that form a single larger picture with no links](https://html.spec.whatwg.org/#a-group-of-images-that-form-a-single-larger-picture-with-no-links)

Status: unreviewed

###### [4.8.4.4.10 A group of images that form a single larger picture with links](https://html.spec.whatwg.org/#a-group-of-images-that-form-a-single-larger-picture-with-links)

Status: unreviewed

###### [4.8.4.4.11 A key part of the content](https://html.spec.whatwg.org/#a-key-part-of-the-content)

Status: unreviewed

###### [4.8.4.4.12 An image not intended for the user](https://html.spec.whatwg.org/#an-image-not-intended-for-the-user)

Status: unreviewed

###### [4.8.4.4.13 An image in an email or private document intended for a specific person who is known to be able to view images](https://html.spec.whatwg.org/#an-image-in-an-e-mail-or-private-document-intended-for-a-specific-person-who-is-known-to-be-able-to-view-images)

Status: unreviewed

###### [4.8.4.4.14 Guidance for markup generators](https://html.spec.whatwg.org/#guidance-for-markup-generators)

Status: unreviewed
<!-- definitions: attr-img-generator-unable-to-provide-required-alt -->

###### [4.8.4.4.15 Guidance for conformance checkers](https://html.spec.whatwg.org/#guidance-for-conformance-checkers)

Status: unreviewed
