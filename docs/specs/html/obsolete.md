# HTML review checklist: 16 Obsolete features

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 18 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html obsolete` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [16 Obsolete features](https://html.spec.whatwg.org/#obsolete)

Status: reviewed (2026-11-02)

### [16.1 Obsolete but conforming features](https://html.spec.whatwg.org/#obsolete-but-conforming-features)

Status: n/a (author conformance)

#### [16.1.1 Warnings for obsolete but conforming features](https://html.spec.whatwg.org/#warnings-for-obsolete-but-conforming-features)

Status: n/a (author conformance)

### [16.2 Non-conforming features](https://html.spec.whatwg.org/#non-conforming-features)

Status: n/a (author conformance)
<!-- definitions: applet acronym bgsound dir noframes isindex keygen listing menuitem nextid noembed param plaintext rb rtc strike xmp basefont big blink center font multicol nobr spacer tt attr-a-charset attr-link-charset attr-script-charset attr-a-coords attr-a-shape attr-a-methods attr-link-methods attr-a-name attr-embed-name attr-img-name attr-option-name attr-a-rev attr-link-rev attr-a-urn attr-link-urn attr-form-accept attr-area-hreflang attr-area-type attr-area-nohref attr-head-profile attr-html-manifest attr-html-version attr-input-ismap attr-input-usemap attr-object-usemap attr-iframe-longdesc attr-img-longdesc attr-img-lowsrc attr-link-target attr-menu-type attr-menu-label attr-contextmenu handler-onshow attr-meta-scheme attr-object-archive attr-object-classid attr-object-code attr-object-codebase attr-object-codetype attr-object-declare attr-object-standby attr-object-typemustmatch attr-script-language attr-script-event attr-script-for attr-style-type attr-table-datapagesize attr-table-summary attr-td-abbr attr-tdth-axis attr-td-scope attr-datasrc attr-datafld attr-dataformatas attr-dropzone attr-body-alink attr-body-bgcolor attr-body-bottommargin attr-body-leftmargin attr-body-link attr-body-marginheight attr-body-marginwidth attr-body-rightmargin attr-body-text attr-body-topmargin attr-body-vlink attr-br-clear attr-caption-align attr-col-align attr-col-char attr-col-charoff attr-col-valign attr-col-width attr-div-align attr-dl-compact attr-embed-align attr-embed-hspace attr-embed-vspace attr-hr-align attr-hr-color attr-hr-noshade attr-hr-size attr-hr-width attr-hx-align attr-iframe-align attr-iframe-allowtransparency attr-iframe-frameborder attr-iframe-framespacing attr-iframe-hspace attr-iframe-marginheight attr-iframe-marginwidth attr-iframe-scrolling attr-iframe-vspace attr-input-align attr-input-border attr-input-hspace attr-input-vspace attr-img-align attr-img-border attr-img-hspace attr-img-vspace attr-legend-align attr-li-type attr-menu-compact attr-object-align attr-object-border attr-object-hspace attr-object-vspace attr-ol-compact attr-p-align attr-pre-width attr-table-align attr-table-bgcolor attr-table-border attr-table-bordercolor attr-table-cellpadding attr-table-cellspacing attr-table-frame attr-table-height attr-table-rules attr-table-width attr-tbody-align attr-tbody-char attr-tbody-charoff attr-tbody-height attr-tbody-valign attr-tdth-align attr-tdth-bgcolor attr-tdth-char attr-tdth-charoff attr-tdth-height attr-tdth-nowrap attr-tdth-valign attr-tdth-width attr-tr-align attr-tr-bgcolor attr-tr-char attr-tr-charoff attr-tr-height attr-tr-valign attr-ul-compact attr-ul-type attr-background -->

### [16.3 Requirements for implementations](https://html.spec.whatwg.org/#requirements-for-implementations)

Status: reviewed (2026-11-02)
> Every obsolete element interface and IDL attribute is present but `HTMLFrameElement` (no frame tree) and the marquee members below; `ch` and `chOff` are `Char()` and `CharOff()`.
> Fixed: `dir` and `param` were `HTMLUnknownElement`s rather than `HTMLDirectoryElement` and `HTMLParamElement`.

#### [16.3.1 The marquee element](https://html.spec.whatwg.org/#the-marquee-element)

Status: reviewed (2026-11-02)
> `HTMLMarqueeElement` has the reflected string attributes and its presentational hints, but no `hspace`, `vspace`, `loop`, `scrollAmount`, `scrollDelay`, `trueSpeed`, `start()` or `stop()`, and marquees don't scroll (FIXME, see `KNOWN-ISSUES.md`).
<!-- definitions: concept-marquee-on concept-marquee-off attr-marquee-behavior attr-marquee-direction attr-marquee-truespeed attr-marquee-loop marquee-current-loop-index -->

- [x] [HTMLMarqueeElement.start()](https://html.spec.whatwg.org/#dom-marquee-start) — When the start() method is called, the marquee element must be turned on.

- [x] [HTMLMarqueeElement.stop()](https://html.spec.whatwg.org/#dom-marquee-stop) — When the stop() method is called, the marquee element must be turned off.

- [x] [marquee scroll interval](https://html.spec.whatwg.org/#marquee-scroll-interval) — A marquee element has a marquee scroll interval, which is obtained as follows:
  - 1. If the element has a scrolldelay attribute, and parsing its value using the rules for parsing non-negative integers does not return an error, then let delay be the parsed value. Otherwise, let delay be 85.
  - 2. If the element does not have a truespeed attribute, and the delay value is less than 60, then let delay be 60 instead.
  - 3. The marquee scroll interval is delay, interpreted in milliseconds.

- [x] [marquee scroll distance](https://html.spec.whatwg.org/#marquee-scroll-distance) — A marquee element has a marquee scroll distance, which, if the element has a scrollamount attribute, and parsing its value using the rules for parsing non-negative integers does not return an error, is the parsed value interpreted in CSS pixels, and otherwise is 6 CSS pixels.

- [x] [marquee loop count](https://html.spec.whatwg.org/#marquee-loop-count) — A marquee element has a marquee loop count, which, if the element has a loop attribute, and parsing its value using the rules for parsing integers does not return an error or a number less than 1, is the parsed value, and otherwise is −1.

- [x] [HTMLMarqueeElement.loop](https://html.spec.whatwg.org/#dom-marquee-loop) — The loop IDL attribute, on getting, must return the element's marquee loop count; and on setting, if the new value is different than the element's marquee loop count and either greater than zero or equal to −1, must set the element's loop content attribute (adding it if necessary) to the valid integer that represents the new value. (Other values are ignored.)

- [x] [increment the marquee current loop index](https://html.spec.whatwg.org/#increment-the-marquee-current-loop-index) — The rendering layer will occasionally increment the marquee current loop index, which must cause the following steps to be run:
  - 1. If the marquee loop count is −1, then return.
  - 2. Increment the marquee current loop index by one.
  - 3. If the marquee current loop index is now greater than or equal to the element's marquee loop count, turn off the marquee element.

#### [16.3.2 Frames](https://html.spec.whatwg.org/#frames)

Status: n/a (no frame tree; `<frame>` is an `HTMLUnknownElement`)
<!-- definitions: frameset process-frame-initial-insertion -->

- [x] [The frame HTML element post-connection steps, given insertedNode,](https://html.spec.whatwg.org/#frame) — The frame HTML element post-connection steps, given insertedNode, are:
  - 1. If insertedNode's node document's browsing context is null, then return.
  - 2. Create a new child navigable for insertedNode.
  - 3. Process the frame attributes for insertedNode, with initialInsertion set to true.
  > n/a: no frame tree; `<frame>` is an `HTMLUnknownElement`

- [x] [process the frame attributes](https://html.spec.whatwg.org/#process-the-frame-attributes) — To process the frame attributes for an element element, with an optional boolean initialInsertion (default false):
  - 1. Let url be the result of running the shared attribute processing steps for iframe and frame elements given element and initialInsertion.
  - 2. If url is null, then return.
  - 3. If url matches about:blank and initialInsertion is true:
    - 3a. Fire an event named load at element.
    - 3b. Return.
  - 4. Navigate an iframe or frame given element, url, the empty string, null, and initialInsertion.
  > n/a: no frame tree; `<frame>` is an `HTMLUnknownElement`

- [x] [HTMLFrameElement.contentDocument](https://html.spec.whatwg.org/#dom-frame-contentdocument) — The contentDocument getter steps are to return this's content document.
  > n/a: no frame tree; `<frame>` is an `HTMLUnknownElement`

- [x] [HTMLFrameElement.contentWindow](https://html.spec.whatwg.org/#dom-frame-contentwindow) — The contentWindow getter steps are to return this's content window.
  > n/a: no frame tree; `<frame>` is an `HTMLUnknownElement`

#### [16.3.3 Other elements, attributes and APIs](https://html.spec.whatwg.org/#other-elements,-attributes-and-apis)

Status: reviewed (2026-11-02)
> `Document`'s obsolete members, `LocalDOMWindow::CaptureEvents`/`ReleaseEvents` and `External`. `document.all` is n/a, as `HTMLAllCollection` is (see 2.6.4.1).
> Fixed: `Document` had no `fgColor`, `linkColor`, `vlinkColor`, `alinkColor`, `bgColor`, `anchors`, `applets`, `clear()`, `captureEvents()` or `releaseEvents()`.
<!-- definitions: dom-document-captureevents dom-document-releaseevents dom-window-releaseevents dom-external-issearchproviderinstalled -->

- [x] [Document.anchors](https://html.spec.whatwg.org/#dom-document-anchors) — The anchors attribute must return an HTMLCollection rooted at the Document node, whose filter matches only a elements with name attributes.

- [x] [Document.applets](https://html.spec.whatwg.org/#dom-document-applets) — The applets attribute must return an HTMLCollection rooted at the Document node, whose filter matches nothing. (It exists for historical reasons.)

- [x] [Document.clear()](https://html.spec.whatwg.org/#dom-document-clear) — The clear(), captureEvents(), and releaseEvents() methods must do nothing.

- [x] [Document.all](https://html.spec.whatwg.org/#dom-document-all) — The all attribute must return an HTMLAllCollection rooted at the Document node, whose filter matches all elements.

- [x] [Window.captureEvents()](https://html.spec.whatwg.org/#dom-window-captureevents) — The captureEvents() and releaseEvents() methods must do nothing.

- [x] [Window.external](https://html.spec.whatwg.org/#dom-external) — The external attribute of the Window interface must return an instance of the External interface:

- [x] [External.AddSearchProvider()](https://html.spec.whatwg.org/#dom-external-addsearchprovider) — The AddSearchProvider() and IsSearchProviderInstalled() methods must do nothing.
