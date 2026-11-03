# HTML review checklist: 15 Rendering

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 33 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html rendering` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [15 Rendering](https://html.spec.whatwg.org/#rendering)

Status: reviewed (2026-11-02)
> Booey renders with upstream's user agent style sheets (`codegen/user-agent-style-sheets/`: `html.css`, `quirks.css`, `popover.css` and the form control sheets) and each element's `CollectPresentationalHintsForAttribute`; where they differ from this section's rules is in `DIFFERENCES.md`.
<!-- definitions: expected -->

### [15.1 Introduction](https://html.spec.whatwg.org/#introduction-17)

Status: reviewed (2026-11-02)
> Being rendered is having a renderer (`FocusAlgorithms.cpp`'s `IsBeingRendered`).

- [x] [being rendered](https://html.spec.whatwg.org/#being-rendered) — An element is being rendered if it has any associated CSS layout boxes, SVG layout boxes, or some equivalent in other styling languages.

- [x] [intersect the viewport](https://html.spec.whatwg.org/#intersect-the-viewport) — An element is said to intersect the viewport when it is being rendered and its associated CSS layout box intersects the viewport.

- [x] [delegating its rendering to its children](https://html.spec.whatwg.org/#delegating-its-rendering-to-its-children) — An element is delegating its rendering to its children if it is not being rendered but its children (if any) could be rendered, as a result of CSS 'display: contents', or some equivalent in other styling languages. [CSSDISPLAY]

### [15.2 The CSS user agent style sheet and presentational hints](https://html.spec.whatwg.org/#the-css-user-agent-style-sheet-and-presentational-hints)

Status: reviewed (2026-11-02)
> The mappings are `HTMLElement`'s `AddHTMLLengthToStyle`, `AddHTMLMultiLengthToStyle`, `AddHTMLPixelsToStyle` and `ApplyAspectRatioFromWidthAndHeightAttributesToStyle`; aligning descendants is the `-webkit-left`, `-webkit-center` and `-webkit-right` values of `text-align`.
<!-- definitions: presentational-hints -->

- [x] [maps to the pixel length property](https://html.spec.whatwg.org/#maps-to-the-pixel-length-property) — When the text below says that an attribute attribute on an element element maps to the pixel length property (or properties) properties, it means that if element has an attribute attribute set, and parsing that attribute's value using the rules for parsing non-negative integers doesn't generate an error, then the user agent is expected to use the parsed value as a pixel length for a presentational hint for properties.

- [x] [maps to the dimension property](https://html.spec.whatwg.org/#maps-to-the-dimension-property) — When the text below says that an attribute attribute on an element element maps to the dimension property (or properties) properties, it means that if element has an attribute attribute set, and parsing that attribute's value using the rules for parsing dimension values doesn't generate an error, then the user agent is expected to use the parsed dimension as the value for a presentational hint for properties, with the value given as a pixel length if the dimension was a length, and with the value given as a percentage if the dimension was a percentage.

- [x] [maps to the dimension property (ignoring zero)](https://html.spec.whatwg.org/#maps-to-the-dimension-property-%28ignoring-zero%29) — When the text below says that an attribute attribute on an element element maps to the dimension property (ignoring zero) (or properties) properties, it means that if element has an attribute attribute set, and parsing that attribute's value using the rules for parsing nonzero dimension values doesn't generate an error, then the user agent is expected to use the parsed dimension as the value for a presentational hint for properties, with the value given as a pixel length if the dimension was a length, and with the value given as a percentage if the dimension was a percentage.

- [x] [map to the aspect-ratio property](https://html.spec.whatwg.org/#map-to-the-aspect-ratio-property) — When the text below says that a pair of attributes w and h on an element element map to the aspect-ratio property, it means that if element has both attributes w and h, and parsing those attributes' values using the rules for parsing non-negative integers doesn't generate an error for either, then the user agent is expected to use the parsed integers as a presentational hint for the 'aspect-ratio' property of the form auto w / h.

- [x] [map to the aspect-ratio property (using dimension rules)](https://html.spec.whatwg.org/#map-to-the-aspect-ratio-property-%28using-dimension-rules%29) — When the text below says that a pair of attributes w and h on an element element map to the aspect-ratio property (using dimension rules), it means that if element has both attributes w and h, and parsing those attributes' values using the rules for parsing dimension values doesn't generate an error or return a percentage for either, then the user agent is expected to use the parsed dimensions as a presentational hint for the 'aspect-ratio' property of the form auto w / h.

- [x] [align descendants](https://html.spec.whatwg.org/#align-descendants) — When a user agent is to align descendants of a node, the user agent is expected to align only those descendants that have both their 'margin-inline-start' and 'margin-inline-end' properties computing to a value other than 'auto', that are over-constrained and that have one of those two margins with a used value forced to a greater value, and that do not themselves have an applicable align attribute. When multiple elements are to align a particular descendant, the most deeply nested such element is expected to override the others. Aligned elements are expected to be aligned by having the used values of their margins on the line-left and line-right sides be set accordingly. [CSSLOGICAL] [CSSWM]

### [15.3 Non-replaced elements](https://html.spec.whatwg.org/#non-replaced-elements)

Status: reviewed (2026-11-02)

#### [15.3.1 Hidden elements](https://html.spec.whatwg.org/#hidden-elements)

Status: reviewed (2026-11-02)
> The rules are in `html.css` (see `DIFFERENCES.md` for where it differs). `hidden` is a hint in `HTMLElement` (`content-visibility: hidden` for `until-found`), and `embed`'s gives it no size.

#### [15.3.2 The page](https://html.spec.whatwg.org/#the-page)

Status: reviewed (2026-11-02)
> `HTMLBodyElement`'s hints and its `link`, `vlink` and `alink` document colours. There's no frame tree, so no container frame element and no `scrolling` attribute.

- [x] [container frame element](https://html.spec.whatwg.org/#container-frame-element) — If the body element's node document's node navigable is a child navigable, and the container of that navigable is a frame or iframe element, then the container frame element of the body element is that frame or iframe element. Otherwise, there is no container frame element.

#### [15.3.3 Flow content](https://html.spec.whatwg.org/#flow-content-3)

Status: reviewed (2026-11-02)
> The rules are in `html.css` (see `DIFFERENCES.md` for where it differs). The popover rules are in `popover.css`; `pre`'s `wrap` and `div`'s `align` are hints.

#### [15.3.4 Phrasing content](https://html.spec.whatwg.org/#phrasing-content-3)

Status: reviewed (2026-11-02)
> The rules are in `html.css` (see `DIFFERENCES.md` for where it differs). `br`'s `clear` and `font`'s attributes are hints.
> Fixed: a legacy font size too large for an integer gave the smallest size (or the medium one, signed) rather than being clamped.

- [x] [rules for parsing a legacy font size](https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-font-size) — When a font element has a size attribute, the user agent is expected to use the following steps, known as the rules for parsing a legacy font size, to treat the attribute as a presentational hint setting the element's 'font-size' property:
  - 1. Let input be the attribute's value.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Skip ASCII whitespace within input given position.
  - 4. If position is past the end of input, there is no presentational hint. Return.
  - 5. If the character at position is a U+002B PLUS SIGN character (+), then let mode be relative-plus, and advance position to the next character. Otherwise, if the character at position is a U+002D HYPHEN-MINUS character (-), then let mode be relative-minus, and advance position to the next character. Otherwise, let mode be absolute.
  - 6. Collect a sequence of code points that are ASCII digits from input given position, and let digits be the resulting sequence.
  - 7. If digits is the empty string, there is no presentational hint. Return.
  - 8. Interpret digits as a base-ten integer. Let value be the resulting number.
  - 9. If mode is relative-plus, then increment value by 3. If mode is relative-minus, then let value be the result of subtracting value from 3.
  - 10. If value is greater than 7, let it be 7.
  - 11. If value is less than 1, let it be 1.
  - 12. Set the 'font-size' property to the keyword corresponding to the value of value according to the following table:

#### [15.3.5 Bidirectional text](https://html.spec.whatwg.org/#bidi-rendering)

Status: reviewed (2026-11-02)
> `dir` is a hint in `HTMLElement`; `bdi`, `bdo`, `output` and `input[type=tel]` are in `html.css` (see `DIFFERENCES.md`).

#### [15.3.6 Sections and headings](https://html.spec.whatwg.org/#sections-and-headings)

Status: reviewed (2026-11-02)
> `h1` to `h6` in `html.css`; `:heading` isn't supported (see `KNOWN-ISSUES.md`).

#### [15.3.7 Lists](https://html.spec.whatwg.org/#lists)

Status: reviewed (2026-11-02)
> The rules are in `html.css` (see `DIFFERENCES.md` for where it differs). The `type` attributes and `li`'s `value` are hints; `ol`'s `start` and `reversed` are applied by list item numbering (see `DIFFERENCES.md`).
> Fixed: `type=round` on `ul` and `li` gave a non-standard list style.

- [x] [When an ol element has a start attribute or a reversed attribute, or both, the…](https://html.spec.whatwg.org/#lists) — When an ol element has a start attribute or a reversed attribute, or both, the user agent is expected to use the following steps to treat the attributes as a presentational hint for the 'counter-reset' property:
  - 1. Let value be null.
  - 2. If the element has a start attribute, then set value to the result of parsing the attribute's value using the rules for parsing integers.
  - 3. If the element has a reversed attribute:
    - 3a. If value is an integer, then increment value by 1 and return reversed(list-item) value.
    - 3b. Otherwise, return reversed(list-item).
  - 4. Otherwise:
    - 4a. If value is an integer, then decrement value by 1 and return list-item value.
    - 4b. Otherwise, there is no presentational hint.

#### [15.3.8 Tables](https://html.spec.whatwg.org/#tables-2)

Status: reviewed (2026-11-02)
> The rules are in `html.css` (see `DIFFERENCES.md` for where it differs). The table elements' hints, with the table's additional styles for its cells and sections (`AdditionalPresentationalHintStyle`) for the rules that depend on `border`, `rules` and `cellpadding`; the quirks are in `quirks.css`, and the `nowrap` with a fixed width quirk is in `RenderTableCell`.
> Fixed: forms that are children of table parts were rendered, as `html.css` had lost the rule hiding them (it needs `:-internal-html-document`, which is now a user agent pseudo-class).

- [x] [only if border is not equivalent to zero](https://html.spec.whatwg.org/#magic-border-selector) — Rules marked "only if border is not equivalent to zero" in the CSS block above is expected to only be applied if the border attribute mentioned in the selectors for the rule is not only present but, when parsed using the rules for parsing non-negative integers, is also found to have a value other than zero or to generate an error.

#### [15.3.9 Margin collapsing quirks](https://html.spec.whatwg.org/#margin-collapsing-quirks)

Status: reviewed (2026-11-02)
> The quirky margins are the `__qem` unit in `html.css` and `RenderBlockFlow`'s margin collapsing.

- [x] [substantial](https://html.spec.whatwg.org/#concept-rendering-substantial) — A node is substantial if it is a text node that is not inter-element whitespace, or if it is an element node.

- [x] [blank](https://html.spec.whatwg.org/#concept-rendering-blank) — A node is blank if it is an element that contains no substantial nodes.

- [x] [elements with default margins](https://html.spec.whatwg.org/#concept-rendering-elements-with-margins) — The elements with default margins are the following elements: blockquote, dir, dl, h1, h2, h3, h4, h5, h6, listing, menu, ol, p, plaintext, pre, ul, xmp.

#### [15.3.10 Form controls](https://html.spec.whatwg.org/#form-controls)

Status: reviewed (2026-11-02)
> `html.css` and the form control sheets, with `RenderTheme`.

#### [15.3.11 The hr element](https://html.spec.whatwg.org/#the-hr-element-2)

Status: reviewed (2026-11-02)
> `html.css` and `HTMLHRElement`'s hints, as upstream (see `DIFFERENCES.md`).

#### [15.3.12 The fieldset and legend elements](https://html.spec.whatwg.org/#the-fieldset-and-legend-elements)

Status: reviewed (2026-11-02)
> `RenderBlock`'s fieldset support; legends aren't placed in the border and `legend`'s `align` isn't applied (see `KNOWN-ISSUES.md`).

- [x] [rendered legend](https://html.spec.whatwg.org/#rendered-legend) — If the element's box has a child box that matches the conditions in the list below, then the first such child box is the 'fieldset' element's rendered legend:

- [x] [anonymous fieldset content box](https://html.spec.whatwg.org/#anonymous-fieldset-content-box) — A fieldset element's anonymous fieldset content box is expected to act as follows:

### [15.4 Replaced elements](https://html.spec.whatwg.org/#replaced-elements)

Status: reviewed (2026-11-02)

#### [15.4.1 Embedded content](https://html.spec.whatwg.org/#embedded-content-rendering-rules)

Status: reviewed (2026-11-02)
> `RenderReplaced` and its subclasses; `iframe` and `video` in `html.css`.

#### [15.4.2 Images](https://html.spec.whatwg.org/#images-3)

Status: reviewed (2026-11-02)
> `RenderImage`; `img`'s `align` margins are in `quirks.css`. `sizes=auto` gets no size containment.

- [x] [restart the animation](https://html.spec.whatwg.org/#restart-the-animation) — When a user agent is to restart the animation for an img element showing an animated image, all animated images with the same absolute URL and the same image data in that img element's node document are expected to restart their animation from the beginning.

#### [15.4.3 Attributes for embedded content and images](https://html.spec.whatwg.org/#attributes-for-embedded-content-and-images)

Status: reviewed (2026-11-02)
> The `align`, `hspace`, `vspace`, `border`, `width`, `height` and `frameborder` hints of the image, plug-in, iframe, video, canvas and image button elements.

#### [15.4.4 Image maps](https://html.spec.whatwg.org/#image-maps-2)

Status: reviewed (2026-11-02)
> `HTMLAreaElement` and `HTMLMapElement`: areas are focusable and hit tested through their image's renderer.

### [15.5 Widgets](https://html.spec.whatwg.org/#widgets)

Status: reviewed (2026-11-02)

#### [15.5.1 Native appearance](https://html.spec.whatwg.org/#native-appearance-2)

Status: reviewed (2026-11-02)
> `RenderTheme` paints the native appearances; there's no base appearance for selects (see `KNOWN-ISSUES.md`).

#### [15.5.2 Writing mode](https://html.spec.whatwg.org/#writing-mode)

Status: reviewed (2026-11-02)

- [x] [horizontal writing mode](https://html.spec.whatwg.org/#horizontal-writing-mode) — A horizontal writing mode is when resolving the 'writing-mode' property of the control results in a computed value of 'horizontal-tb'.

- [x] [vertical writing mode](https://html.spec.whatwg.org/#vertical-writing-mode) — A vertical writing mode is when resolving the 'writing-mode' property of the control results in a computed value of either 'vertical-rl', 'vertical-lr', 'sideways-rl' or 'sideways-lr'.

#### [15.5.3 Button layout](https://html.spec.whatwg.org/#button-layout)

Status: reviewed (2026-11-02)
> `RenderButton`, a flexible box.

- [x] [Button layout](https://html.spec.whatwg.org/#button-layout-2) — Button layout is as follows:

#### [15.5.4 The button element](https://html.spec.whatwg.org/#the-button-element-2)

Status: reviewed (2026-11-02)

#### [15.5.5 The details and summary elements](https://html.spec.whatwg.org/#the-details-and-summary-elements)

Status: reviewed (2026-11-02)
> `html.css` and `detailsElementShadow.css`, with `HTMLDetailsElement`'s shadow tree.
<!-- definitions: default-summary -->

#### [15.5.6 The input element as a text entry widget](https://html.spec.whatwg.org/#the-input-element-as-a-text-entry-widget)

Status: reviewed (2026-11-02)
> `RenderTextControlSingleLine::PreferredContentLogicalWidth` gives (size - 1) × avg + max.

- [x] [An input element whose type attribute is in one of the above states is an eleme…](https://html.spec.whatwg.org/#the-input-element-as-a-text-entry-widget) — An input element whose type attribute is in one of the above states is an element with default preferred size, and user agents are expected to apply the 'field-sizing' CSS property to the element. User agents are expected to determine the inline size of its intrinsic size by the following steps:
  - 1. If the 'field-sizing' property on the element has a computed value of 'content', the inline size is determined by the text which the element shows. The text is either a value or a short hint specified by the placeholder attribute. User agents may take the text caret size into account in the inline size.
  - 2. If the element has a size attribute, and parsing that attribute's value using the rules for parsing non-negative integers doesn't generate an error, return the value obtained from applying the converting a character width to pixels algorithm to the value of the attribute.
  - 3. Otherwise, return the value obtained from applying the converting a character width to pixels algorithm to the number 20.

- [x] [converting a character width to pixels](https://html.spec.whatwg.org/#converting-a-character-width-to-pixels) — The converting a character width to pixels algorithm returns (size-1)×avg + max, where size is the character width to convert, avg is the average character width of the primary font for the element for which the algorithm is being run, in pixels, and max is the maximum character width of that same font, also in pixels. (The element's 'letter-spacing' property does not affect the result.)

#### [15.5.7 The input element as domain-specific widgets](https://html.spec.whatwg.org/#the-input-element-as-domain-specific-widgets)

Status: reviewed (2026-11-02)
> The date and time inputs' shadow trees (`DateTimeEditElement`).

#### [15.5.8 The input element as a range control](https://html.spec.whatwg.org/#the-input-element-as-a-range-control)

Status: reviewed (2026-11-02)
> `RangeInputType` and `SliderThumbElement`.

#### [15.5.9 The input element as a color well](https://html.spec.whatwg.org/#the-input-element-as-a-colour-well)

Status: reviewed (2026-11-02)
> `ColorInputType`.

#### [15.5.10 The input element as a checkbox and radio button widgets](https://html.spec.whatwg.org/#the-input-element-as-a-checkbox-and-radio-button-widgets)

Status: reviewed (2026-11-02)
> `RenderTheme`.

#### [15.5.11 The input element as a file upload control](https://html.spec.whatwg.org/#the-input-element-as-a-file-upload-control)

Status: reviewed (2026-11-02)
> `FileInputType`; there's no file chooser (see `KNOWN-ISSUES.md`).

#### [15.5.12 The input element as a button](https://html.spec.whatwg.org/#the-input-element-as-a-button)

Status: reviewed (2026-11-02)
> `html.css` and `RenderButton`.

#### [15.5.13 The marquee element](https://html.spec.whatwg.org/#the-marquee-element-2)

Status: reviewed (2026-11-02)
> Marquees don't scroll (see `KNOWN-ISSUES.md`).

#### [15.5.14 The meter element](https://html.spec.whatwg.org/#the-meter-element-2)

Status: reviewed (2026-11-02)
> `meterElementShadow.css` and `RenderMeter`.

#### [15.5.15 The progress element](https://html.spec.whatwg.org/#the-progress-element-2)

Status: reviewed (2026-11-02)
> `RenderProgress`.

#### [15.5.16 The select element](https://html.spec.whatwg.org/#the-select-element-2)

Status: reviewed (2026-11-02)
> `RenderMenuList` and `RenderListBox`; there's no customizable select (see `KNOWN-ISSUES.md`).
<!-- definitions: list-box select-button-slot select-fallback-button-text select-popover select-popover-slot select-list-box-slot optgroup-base-appearance optgroup-legend-slot optgroup-label-element optgroup-contents-slot option-base-appearance option-contents-slot option-label-element -->

- [x] [When the element renders as a list box, it is a devolvable widget expected to r…](https://html.spec.whatwg.org/#drop-down-box) — When the element renders as a list box, it is a devolvable widget expected to render as an 'inline-block' box. The inline size of its intrinsic size is the width of the select's labels plus the width of a scrollbar. The block size of its intrinsic size is determined by the following steps:
  - 1. If the 'field-sizing' property on the element has a computed value of 'content', return the height necessary to contain all rows for items.
  - 2. If the size attribute is absent or it has no valid value, return the height necessary to contain four rows.
  - 3. Otherwise, return the height necessary to contain as many rows for items as given by the element's display size.

- [x] [When a select is being rendered as a drop-down box with base appearance, it is…](https://html.spec.whatwg.org/#drop-down-box) — When a select is being rendered as a drop-down box with base appearance, it is expected to render with a shadow tree that contains the following elements:
  - 1. A select button slot, which is a slot element. It is appended to the select's shadow root as the first child. It is expected to take the first child element of the select if the first child element is a button.
  - 2. A select fallback button text, which is a div element. It is appended to the select button slot.
  - 3. A select popover, which is a div element. It is appended to the select's shadow root as the second child, after the select button slot. The select element's '::picker' pseudo-element is the select popover if the provided argument is select.
  - 4. A select popover slot, which is a slot element. It is appended to the select popover. It is expected to take all child nodes of the select except for the first child button, which is taken by the select button slot.

- [x] [select's contents are being rendered with base appearance](https://html.spec.whatwg.org/#select's-options-are-being-rendered-with-base-appearance) — To determine if a select's contents are being rendered with base appearance, given a select element select:
  - 1. If select is being rendered as a list box with base appearance, then return true.
  - 2. If select is being rendered as a drop-down box with base appearance, and its select popover is being rendered with base appearance, and select does not have the multiple attribute set, then return true.
  - 3. Return false.

- [x] [width of the select's labels](https://html.spec.whatwg.org/#width-of-the-select's-labels) — The width of the select's labels is the wider of the width necessary to render the widest optgroup, and the width necessary to render the widest option element in the element's list of options (including its indent, if any). If the select has the multiple attribute and is being rendered as a drop-down box, then the width should also be wide enough to accommodate the text rendered in the select with any combination of options selected.

#### [15.5.17 The textarea element](https://html.spec.whatwg.org/#the-textarea-element-2)

Status: reviewed (2026-11-02)
> `RenderTextControlMultiLine`: the effective width is cols × avg plus a scrollbar, the height rows lines.

- [x] [textarea effective width](https://html.spec.whatwg.org/#textarea-effective-width) — The textarea effective width of a textarea element is size×avg + sbw, where size is the element's character width, avg is the average character width of the primary font of the element, in CSS pixels, and sbw is the width of a scrollbar, in CSS pixels. (The element's 'letter-spacing' property does not affect the result.)

- [x] [textarea effective height](https://html.spec.whatwg.org/#textarea-effective-height) — The textarea effective height of a textarea element is the height in CSS pixels of the number of lines given by the element's character height, plus the height of a scrollbar in CSS pixels.

### [15.6 Frames and framesets](https://html.spec.whatwg.org/#frames-and-framesets)

Status: n/a (no frame tree)
<!-- definitions: has-a-border frame-border-colour -->

- [x] [User agents are expected to render frameset elements as a box with the height a…](https://html.spec.whatwg.org/#frames-and-framesets) — User agents are expected to render frameset elements as a box with the height and width of the viewport, with a surface rendered according to the following layout algorithm:
  - 1. The cols and rows variables are lists of zero or more pairs consisting of a number and a unit, the unit being one of percentage, relative, and absolute. Use the rules for parsing a list of dimensions to parse the value of the element's cols attribute, if there is one. Let cols be the result, or an empty list if there is no such attribute. Use the rules for parsing a list of dimensions to parse th…
  - 2. For any of the entries in cols or rows that have the number zero and the unit relative, change the entry's number to one.
  - 3. If cols has no entries, then add a single entry consisting of the value 1 and the unit relative to cols. If rows has no entries, then add a single entry consisting of the value 1 and the unit relative to rows.
  - 4. Invoke the algorithm defined below to convert a list of dimensions to a list of pixel values using cols as the input list, and the width of the surface that the frameset is being rendered into, in CSS pixels, as the input dimension. Let sized cols be the resulting list. Invoke the algorithm defined below to convert a list of dimensions to a list of pixel values using rows as the input list, and t…
  - 5. Split the surface into a grid of w×h rectangles, where w is the number of entries in sized cols and h is the number of entries in sized rows. Size the columns so that each column in the grid is as many CSS pixels wide as the corresponding entry in the sized cols list. Size the rows so that each row in the grid is as many CSS pixels high as the corresponding entry in the sized rows list.
  - 6. Let children be the list of frame and frameset elements that are children of the frameset element for which the algorithm was invoked.
  - 7. For each row of the grid of rectangles created in the previous step, from top to bottom, run these substeps:
    - 7a. For each rectangle in the row, from left to right, run these substeps:
      - 7a.i. If there are any elements left in children, take the first element in the list, and assign it to the rectangle. If this is a frameset element, then recurse the entire frameset layout algorithm for that frameset element, with the rectangle as the surface. Otherwise, it is a frame element; render its content navigable, positioned and sized to fit the rectangle.
      - 7a.ii. If there are any elements left in children, remove the first element from children.
  - 8. If the frameset element has a border, draw an outer set of borders around the rectangles, using the element's frame border color. For each rectangle, if there is an element assigned to that rectangle, and that element has a border, draw an inner set of borders around that rectangle, using the element's frame border color. For each (visible) border that does not abut a rectangle that is assigned a…
    - 8a. If the element has a frameborder attribute whose value is not the empty string and whose first character is either a U+0031 DIGIT ONE (1) character, a U+0079 LATIN SMALL LETTER Y character (y), or a U+0059 LATIN CAPITAL LETTER Y character (Y), then return true.
    - 8b. Otherwise, if the element has a frameborder attribute, return false.
    - 8c. Otherwise, if the element has a parent element that is a frameset element, then return true if that element has a border, and false if it does not.
    - 8d. Otherwise, return true.
    - 8a. If the element has a bordercolor attribute, and applying the rules for parsing a legacy color value to that attribute's value does not return failure, then return the color so obtained.
    - 8b. Otherwise, if the element has a parent element that is a frameset element, then return the frame border color of that element.
    - 8c. Otherwise, return gray.

- [x] [convert a list of dimensions to a list of pixel values](https://html.spec.whatwg.org/#convert-a-list-of-dimensions-to-a-list-of-pixel-values) — The algorithm to convert a list of dimensions to a list of pixel values consists of the following steps:
  - 1. Let input list be the list of numbers and units passed to the algorithm. Let output list be a list of numbers the same length as input list, all zero. Entries in output list correspond to the entries in input list that have the same position.
  - 2. Let input dimension be the size passed to the algorithm.
  - 3. Let total percentage be the sum of all the numbers in input list whose unit is percentage. Let total relative be the sum of all the numbers in input list whose unit is relative. Let total absolute be the sum of all the numbers in input list whose unit is absolute. Let remaining space be the value of input dimension.
  - 4. If total absolute is greater than remaining space, then for each entry in input list whose unit is absolute, set the corresponding value in output list to the number of the entry in input list multiplied by remaining space and divided by total absolute. Then, set remaining space to zero. Otherwise, for each entry in input list whose unit is absolute, set the corresponding value in output list to…
  - 5. If total percentage multiplied by the input dimension and divided by 100 is greater than remaining space, then for each entry in input list whose unit is percentage, set the corresponding value in output list to the number of the entry in input list multiplied by remaining space and divided by total percentage. Then, set remaining space to zero. Otherwise, for each entry in input list whose unit…
  - 6. For each entry in input list whose unit is relative, set the corresponding value in output list to the number of the entry in input list multiplied by remaining space and divided by total relative.
  - 7. Return output list.

### [15.7 Interactive media](https://html.spec.whatwg.org/#interactive-media)

Status: reviewed (2026-11-02)

#### [15.7.1 Links, forms, and navigation](https://html.spec.whatwg.org/#links,-forms,-and-navigation)

Status: reviewed (2026-11-02)
> The event handler shows the hand cursor over links (`UseHandCursor`).

#### [15.7.2 The title attribute](https://html.spec.whatwg.org/#the-title-attribute-2)

Status: reviewed (2026-11-02)
> `title` is never shown (see `KNOWN-ISSUES.md`).

#### [15.7.3 Editing hosts](https://html.spec.whatwg.org/#editing-hosts)

Status: reviewed (2026-11-02)
> `contenteditable` is a hint in `HTMLElement`; the caret is drawn by `FrameSelection`.

#### [15.7.4 Text rendered in native user interfaces](https://html.spec.whatwg.org/#text-rendered-in-native-user-interfaces)

Status: reviewed (2026-11-02)
> There's no native user interface for the host to render text into.

### [15.8 Print media](https://html.spec.whatwg.org/#print-media)

Status: n/a (no printing)
<!-- definitions: obtain-a-physical-form -->

### [15.9 Unstyled XML documents](https://html.spec.whatwg.org/#unstyled-xml-documents)

Status: reviewed (2026-11-02)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: an-unstyled-document-view -->

- [x] [unstyled document](https://html.spec.whatwg.org/#unstyled-document) — A Document is an unstyled document while it matches the following conditions:
