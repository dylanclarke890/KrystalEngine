# HTML review checklist: 4.8.12 The map element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 5 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html image-maps` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.8.12 The map element](https://html.spec.whatwg.org/#the-map-element)

Status: reviewed (2026-11-01)
> `HTMLMapElement`: `name` is reflected and `areas` is a live collection of its area descendants.
<!-- definitions: attr-map-name -->

- [x] [HTMLMapElement.areas](https://html.spec.whatwg.org/#dom-map-areas) — The areas attribute must return an HTMLCollection rooted at the map element, whose filter matches only area elements.

#### [4.8.13 The area element](https://html.spec.whatwg.org/#the-area-element)

Status: reviewed (2026-11-01)
> `HTMLAreaElement`: a hyperlink (4.6), whose `shape` and `coords` give its shape (4.8.14.2).
<!-- definitions: attr-area-alt attr-area-shape attr-area-coords attr-area-shape-circle attr-area-shape-default attr-area-shape-poly attr-area-shape-rect -->

- [x] [HTMLAreaElement.referrerPolicy](https://html.spec.whatwg.org/#dom-area-referrerpolicy) — The IDL attribute referrerPolicy must reflect the referrerpolicy content attribute, limited to only known values.

#### [4.8.14 Image maps](https://html.spec.whatwg.org/#image-maps)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.8.14.1 Authoring](https://html.spec.whatwg.org/#authoring)

Status: reviewed (2026-11-01)
> `usemap` is `HTMLImageElement::AssociatedMapElement`, by the rules for parsing a hash-name reference.
<!-- definitions: image-map attr-hyperlink-usemap -->

##### [4.8.14.2 Processing model](https://html.spec.whatwg.org/#image-map-processing-model)

Status: reviewed (2026-11-01)
> `HTMLImageElement::AssociatedMapElement`, `HTMLMapElement::MapMouseEvent` and `HTMLAreaElement::GetRegion`. Shapes are hit before default areas (see `DIFFERENCES.md`).
> Fixed: a rectangle whose corners were given right to left or bottom to top had no area.

- [x] [If an img element has a usemap attribute specified, user agents must process it…](https://html.spec.whatwg.org/#image-map-processing-model) — If an img element has a usemap attribute specified, user agents must process it as follows:
  - 1. Parse the attribute's value using the rules for parsing a hash-name reference to a map element, with the element as the context node. This will return either an element (the map) or null.
  - 2. If that returned null, then return. The image is not associated with an image map after all.
  - 3. Otherwise, the user agent must collect all the area elements that are descendants of the map. Let areas be that list.

- [x] [If the user agent intends to show the text that the img element represents, the…](https://html.spec.whatwg.org/#image-map-processing-model) — If the user agent intends to show the text that the img element represents, then it must use the following steps:
  - 1. Remove all the area elements in areas that have no href attribute.
  - 2. Remove all the area elements in areas that have no alt attribute, or whose alt attribute's value is the empty string, if there is another area element in areas with the same value in the href attribute and with a non-empty alt attribute.
  - 3. Each remaining area element in areas represents a hyperlink. Those hyperlinks should all be made available to the user in a manner associated with the text of the img. In this context, user agents may represent area and img elements with no specified alt attributes, or whose alt attributes are the empty string or some other non-visible text, in an implementation-defined fashion intended to indica…
  > n/a: an image's alt text doesn't show its areas' links.

- [x] [Each area element in areas must be processed as follows to obtain a shape to la…](https://html.spec.whatwg.org/#image-map-processing-model) — Each area element in areas must be processed as follows to obtain a shape to layer onto the image:
  - 1. Find the state that the element's shape attribute represents.
  - 2. Use the rules for parsing a list of floating-point numbers to parse the element's coords attribute, if it is present, and let the coords list be the result. If the attribute is absent, let the coords list be the empty list.
  - 3. If the number of items in the coords list is less than the minimum number given for the area element's current state, as per the following table, then the shape is empty; return.
  - 4. Check for excess items in the coords list as per the entry in the following list corresponding to the shape attribute's state: Circle stateDrop any items in the list beyond the third.Default stateDrop all items in the list.Polygon stateDrop the last item if there's an odd number of items.Rectangle stateDrop any items in the list beyond the fourth.
  - 5. If the shape attribute represents the rectangle state, and the first number in the list is numerically greater than the third number in the list, then swap those two numbers around.
  - 6. If the shape attribute represents the rectangle state, and the second number in the list is numerically greater than the fourth number in the list, then swap those two numbers around.
  - 7. If the shape attribute represents the circle state, and the third number in the list is less than or equal to zero, then the shape is empty; return.
  - 8. Now, the shape represented by the element is the one described for the entry in the list below corresponding to the state of the shape attribute: Circle state Let x be the first number in coords, y be the second number, and r be the third number. The shape is a circle whose center is x CSS pixels from the left edge of the image and y CSS pixels from the top edge of the image, and whose radius is…
