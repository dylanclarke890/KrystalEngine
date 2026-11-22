# cssom-view-1 review checklist

Generated from <https://drafts.csswg.org/cssom-view-1/> on 2026-11-22 by `Krystal.Booey/tools/spec_checklist.py`: 104 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py cssom-view-1` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[cssom-view-1:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py cssom-view-1 --tick <section anchor>` ticks a section's algorithms and sets its status.

## [Abstract](https://drafts.csswg.org/cssom-view-1/#abstract)

Status: unreviewed

## [Status of this document](https://drafts.csswg.org/cssom-view-1/#sotd)

Status: unreviewed

## [1. Background](https://drafts.csswg.org/cssom-view-1/#background)

Status: unreviewed

## [2. Terminology](https://drafts.csswg.org/cssom-view-1/#terminology)

Status: unreviewed
<!-- definitions: potentially-scrollable scrolling-box overflow-directions scrolling-area scrolling-area-origin beginning-edges ending-edges associated-document svg-layout-box transforms -->

### [2.1. CSS pixels](https://drafts.csswg.org/cssom-view-1/#css-pixels)

Status: unreviewed

### [2.2. Zooming](https://drafts.csswg.org/cssom-view-1/#zooming)

Status: unreviewed
<!-- definitions: page-zoom -->

### [2.3. Web-exposed screen information](https://drafts.csswg.org/cssom-view-1/#web-exposed-screen-information)

Status: unreviewed

- [ ] [Web-exposed screen area](https://drafts.csswg.org/cssom-view-1/#web-exposed-screen-area) — The Web-exposed screen area must return the result of the following algorithm:
  - 1. Let target be this’s relevant global object’s browsing context.
  - 2. Let emulated screen area be the WebDriver BiDi emulated total screen area of target.
  - 3. If emulated screen area is not null, return emulated screen area.
  - 4. Otherwise, return one of the following: The area of the output device, in CSS pixels.; The area of the viewport, in CSS pixels.

- [ ] [Web-exposed available screen area](https://drafts.csswg.org/cssom-view-1/#web-exposed-available-screen-area) — The Web-exposed available screen area must return the result of the following algorithm:
  - 1. Let target be this’s relevant global object’s browsing context.
  - 2. Let emulated screen area be the WebDriver BiDi emulated available screen area for target.
  - 3. If emulated screen area is not null, return emulated screen area.
  - 4. Otherwise, return one of the following: The available area of the rendering surface of the output device, in CSS pixels.; The area of the output device, in CSS pixels.; The area of the viewport, in CSS pixels.

## [3. Common Infrastructure](https://drafts.csswg.org/cssom-view-1/#common-infrastructure)

Status: unreviewed

### [3.1. Scrolling](https://drafts.csswg.org/cssom-view-1/#scrolling)

Status: unreviewed
<!-- definitions: non-interrupted-scroll-result interrupted-scroll-result scroll-completed concept-smooth-scroll smooth-scroll-completed smooth-scroll-aborted concept-instant-scroll -->

- [ ] [perform a scroll](https://drafts.csswg.org/cssom-view-1/#perform-a-scroll) — When a user agent is to perform a scroll of a scrolling box box, to a given position position, an associated element or pseudo-element element and optionally a scroll behavior behavior (which is "auto" if omitted), the following steps must be run:
  - 1. Abort any ongoing smooth scroll for box.
  - 2. Resolve all pending scroll Promises whose scroll container is box with an interrupted scroll result.
  - 3. Let scrollPromise be a new Promise.
  - 4. Return scrollPromise, and run the remaining steps in parallel.
  - 5. If the user agent honors the scroll-behavior property and one of the following is true: behavior is "auto" and element is not null and its computed value of the scroll-behavior property is smooth, or; behavior is smooth then perform a smooth scroll of box to position; otherwise, perform an instant scroll of box to position.
  - 6. Wait until either the position has finished updating, or scrollPromise has been resolved.
  - 7. If scrollPromise is still in the pending state:
    - 7a. If the scroll position changed as a result of this call, emit the scrollend event.
    - 7b. Resolve scrollPromise with a non-interrupted scroll result.

- [ ] [perform a scroll (viewport)](https://drafts.csswg.org/cssom-view-1/#viewport-perform-a-scroll) — When a user agent is to perform a scroll of a viewport to a given position position and optionally a scroll behavior behavior (which is "auto" if omitted) it must perform a coordinated viewport scroll by following these steps:
  - 1. Let doc be the viewport’s associated Document.
  - 2. Let vv be the VisualViewport whose associated document is doc.
  - 3. Let maxX be the difference between viewport’s scrolling box’s width and the value of vv’s width attribute.
  - 4. Let maxY be the difference between viewport’s scrolling box’s height and the value of vv’s height attribute.
  - 5. Let dx be the horizontal component of position - the value vv’s pageLeft attribute
  - 6. Let dy be the vertical component of position - the value of vv’s pageTop attribute
  - 7. Let visual x be the value of vv’s offsetLeft attribute.
  - 8. Let visual y be the value of vv’s offsetTop attribute.
  - 9. Let visual dx be min(maxX, max(0, visual x + dx)) - visual x.
  - 10. Let visual dy be min(maxY, max(0, visual y + dy)) - visual y.
  - 11. Let layout dx be dx - visual dx
  - 12. Let layout dy be dy - visual dy
  - 13. Let element be doc’s root element if there is one, null otherwise.
  - 14. Perform a scroll of the viewport’s scrolling box to its current scroll position + (layout dx, layout dy) with element as the associated element, and behavior as the scroll behavior. Let scrollPromise1 be the Promise returned from this step.
  - 15. Perform a scroll of vv’s scrolling box to its current scroll position + (visual dx, visual dy) with element as the associated element, and behavior as the scroll behavior. Let scrollPromise2 be the Promise returned from this step.
  - 16. Let scrollPromise be a new Promise.
  - 17. Return scrollPromise, and run the remaining steps in parallel.
  - 18. Wait until both scrollPromise1 and scrollPromise2 have settled.
  - 19. If scrollPromise1 and scrollPromise2 were both resolved with a non-interrupted scroll result, resolve scrollPromise with a non-interrupted scroll result.
  - 20. Otherwise, resolve scrollPromise with an interrupted scroll result.

- [ ] [scroll to the beginning of the document](https://drafts.csswg.org/cssom-view-1/#scroll-to-the-beginning-of-the-document) — To scroll to the beginning of the document for a document document, follow these steps:
  - 1. Let viewport be the viewport that is associated with document.
  - 2. Let position be the scroll position viewport would have by aligning the beginning edges of the scrolling area with the beginning edges of viewport.
  - 3. If position is the same as viewport’s current scroll position, and viewport does not have an ongoing smooth scroll, abort these steps.
  - 4. Perform a scroll of viewport to position, and document’s root element as the associated element, if there is one, or null otherwise.

### [3.2. WebIDL values](https://drafts.csswg.org/cssom-view-1/#webidl-values)

Status: unreviewed
<!-- definitions: normalize-non-finite-values -->

## [4. Extensions to the Window Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-window-interface)

Status: unreviewed
<!-- definitions: dom-window-screenleft dom-window-screentop determine-the-device-pixel-ratio -->

- [ ] [Window.matchMedia(query)](https://drafts.csswg.org/cssom-view-1/#dom-window-matchmedia) — When the matchMedia(query) method is invoked these steps must be run:
  - 1. Let parsed media query list be the result of parsing query.
  - 2. Return a new MediaQueryList object, with this’s associated Document as the document, with parsed media query list as its associated media query list.

- [ ] [Window.screen](https://drafts.csswg.org/cssom-view-1/#dom-window-screen) — The screen attribute must return the Screen object associated with the Window object.

- [ ] [Window.visualViewport](https://drafts.csswg.org/cssom-view-1/#dom-window-visualviewport) — If the associated document is fully active, the visualViewport attribute must return the VisualViewport object associated with the Window object’s associated document. Otherwise, it must return null.

- [ ] [Window.moveTo(x, y)](https://drafts.csswg.org/cssom-view-1/#dom-window-moveto) — The moveTo(x, y) method must follow these steps:
  - 1. Optionally, return.
  - 2. Let target be this’s relevant global object’s browsing context.
  - 3. If target is not an auxiliary browsing context that was created by a script (as opposed to by an action of the user), then return.
  - 4. If target’s top-level traversable’s Is Document Picture-in-Picture boolean is true, then return.
  - 5. Optionally, clamp x and y in a user-agent-defined manner so that the window does not move outside the available space.
  - 6. Move target’s window such that the window’s top left corner is at coordinates (x, y) relative to the top left corner of the output device, measured in CSS pixels of target. The positive axes are rightward and downward.

- [ ] [Window.moveBy(x, y)](https://drafts.csswg.org/cssom-view-1/#dom-window-moveby) — The moveBy(x, y) method must follow these steps:
  - 1. Optionally, return.
  - 2. Let target be this’s relevant global object’s browsing context.
  - 3. If target is not an auxiliary browsing context that was created by a script (as opposed to by an action of the user), then return.
  - 4. If target’s top-level traversable’s Is Document Picture-in-Picture boolean is true, then return.
  - 5. Optionally, clamp x and y in a user-agent-defined manner so that the window does not move outside the available space.
  - 6. Move target’s window x CSS pixels of target rightward and y CSS pixels of target downward.

- [ ] [Window.resizeTo(width, height)](https://drafts.csswg.org/cssom-view-1/#dom-window-resizeto) — The resizeTo(width, height) method must follow these steps:
  - 1. Optionally, return.
  - 2. Let target be this’s relevant global object’s browsing context.
  - 3. If target is not an auxiliary browsing context that was created by a script (as opposed to by an action of the user), then return.
  - 4. If target’s top-level traversable’s Is Document Picture-in-Picture boolean is true, then:
    - 4a. If this’s relevant global object does not have transient activation, throw a "NotAllowedError" DOMException.
    - 4b. Consume user activation given this’s relevant global object.
  - 5. Optionally, clamp width and height in a user-agent-defined manner so that the window does not get too small or bigger than the available space.
  - 6. Resize target’s window by moving its right and bottom edges such that the distance between the left and right edges of the viewport are width CSS pixels of target and the distance between the top and bottom edges of the viewport are height CSS pixels of target.
  - 7. Optionally, move target’s window in a user-agent-defined manner so that it does not grow outside the available space.

- [ ] [Window.resizeBy(x, y)](https://drafts.csswg.org/cssom-view-1/#dom-window-resizeby) — The resizeBy(x, y) method must follow these steps:
  - 1. Optionally, return.
  - 2. Let target be this’s relevant global object’s browsing context.
  - 3. If target is not an auxiliary browsing context that was created by a script (as opposed to by an action of the user), then return.
  - 4. If target’s top-level traversable’s Is Document Picture-in-Picture boolean is true, then:
    - 4a. If this’s relevant global object does not have transient activation, throw a "NotAllowedError" DOMException.
    - 4b. Consume user activation given this’s relevant global object.
  - 5. Optionally, clamp x and y in a user-agent-defined manner so that the window does not get too small or bigger than the available space.
  - 6. Resize target’s window by moving its right edge x CSS pixels of target rightward and its bottom edge y CSS pixels of target downward.
  - 7. Optionally, move target’s window in a user-agent-defined manner so that it does not grow outside the available space.

- [ ] [Window.innerWidth](https://drafts.csswg.org/cssom-view-1/#dom-window-innerwidth) — The innerWidth attribute must return the viewport width including the size of a rendered scroll bar (if any), or zero if there is no viewport.

- [ ] [Window.innerHeight](https://drafts.csswg.org/cssom-view-1/#dom-window-innerheight) — The innerHeight attribute must return the viewport height including the size of a rendered scroll bar (if any), or zero if there is no viewport.

- [ ] [Window.scrollX](https://drafts.csswg.org/cssom-view-1/#dom-window-scrollx) — The scrollX attribute must return the x-coordinate, relative to the initial containing block origin, of the left of the viewport, or zero if there is no viewport.

- [ ] [Window.pageXOffset](https://drafts.csswg.org/cssom-view-1/#dom-window-pagexoffset) — The pageXOffset attribute must return the value returned by the scrollX attribute.

- [ ] [Window.scrollY](https://drafts.csswg.org/cssom-view-1/#dom-window-scrolly) — The scrollY attribute must return the y-coordinate, relative to the initial containing block origin, of the top of the viewport, or zero if there is no viewport.

- [ ] [Window.pageYOffset](https://drafts.csswg.org/cssom-view-1/#dom-window-pageyoffset) — The pageYOffset attribute must return the value returned by the scrollY attribute.

- [ ] [Window.scroll()](https://drafts.csswg.org/cssom-view-1/#dom-window-scroll) — When the scroll() method is invoked these steps must be run:
  - 1. If invoked with one argument, follow these substeps:
    - 1a. Let options be the argument.
    - 1b. Let x be the value of the left dictionary member of options, if present, or the viewport’s current scroll position on the x axis otherwise.
    - 1c. Let y be the value of the top dictionary member of options, if present, or the viewport’s current scroll position on the y axis otherwise.
  - 2. If invoked with two arguments, follow these substeps:
    - 2a. Let options be null converted to a ScrollToOptions dictionary. [WEBIDL]
    - 2b. Let x and y be the arguments, respectively.
  - 3. Normalize non-finite values for x and y.
  - 4. If there is no viewport, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 5. Let viewport width be the width of the viewport excluding the width of the scroll bar, if any.
  - 6. Let viewport height be the height of the viewport excluding the height of the scroll bar, if any.
  - 7. If the viewport has rightward overflow direction Let x be max(0, min(x, viewport scrolling area width - viewport width)). If the viewport has leftward overflow direction Let x be min(0, max(x, viewport width - viewport scrolling area width)).
  - 8. If the viewport has downward overflow direction Let y be max(0, min(y, viewport scrolling area height - viewport height)). If the viewport has upward overflow direction Let y be min(0, max(y, viewport height - viewport scrolling area height)).
  - 9. Let position be the scroll position the viewport would have by aligning the x-coordinate x of the viewport scrolling area with the left of the viewport and aligning the y-coordinate y of the viewport scrolling area with the top of the viewport.
  - 10. If position is the same as the viewport’s current scroll position, and the viewport does not have an ongoing smooth scroll, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 11. Let document be the viewport’s associated Document.
  - 12. Perform a scroll of the viewport to position, document’s root element as the associated element, if there is one, or null otherwise, and the scroll behavior being the value of the behavior dictionary member of options. Let scrollPromise be the Promise returned from this step.
  - 13. Return scrollPromise.

- [ ] [Window.scrollTo()](https://drafts.csswg.org/cssom-view-1/#dom-window-scrollto) — When the scrollTo() method is invoked, the user agent must act as if the scroll() method was invoked with the same arguments.

- [ ] [Window.scrollBy()](https://drafts.csswg.org/cssom-view-1/#dom-window-scrollby) — When the scrollBy() method is invoked, the user agent must run these steps:
  - 1. If invoked with two arguments, follow these substeps:
    - 1a. Let options be null converted to a ScrollToOptions dictionary. [WEBIDL]
    - 1b. Let x and y be the arguments, respectively.
    - 1c. Let the left dictionary member of options have the value x.
    - 1d. Let the top dictionary member of options have the value y.
  - 2. Normalize non-finite values for the left and top dictionary members of options.
  - 3. Add the value of scrollX to the left dictionary member.
  - 4. Add the value of scrollY to the top dictionary member.
  - 5. Return the Promise returned from scroll() after the method is invoked with options as the only argument.

- [ ] [Window.screenX](https://drafts.csswg.org/cssom-view-1/#dom-window-screenx) — The screenX and screenLeft attributes must return the x-coordinate, relative to the origin of the Web-exposed screen area, of the left of the client window as number of CSS pixels, or zero if there is no such thing.

- [ ] [Window.screenY](https://drafts.csswg.org/cssom-view-1/#dom-window-screeny) — The screenY and screenTop attributes must return the y-coordinate, relative to the origin of the screen of the Web-exposed screen area, of the top of the client window as number of CSS pixels, or zero if there is no such thing.

- [ ] [Window.outerWidth](https://drafts.csswg.org/cssom-view-1/#dom-window-outerwidth) — The outerWidth attribute must return the width of the client window. If there is no client window this attribute must return zero.

- [ ] [Window.outerHeight](https://drafts.csswg.org/cssom-view-1/#dom-window-outerheight) — The outerHeight attribute must return the height of the client window. If there is no client window this attribute must return zero.

- [ ] [Window.devicePixelRatio](https://drafts.csswg.org/cssom-view-1/#dom-window-devicepixelratio) — The devicePixelRatio attribute must return the result of determining the device pixel ratio for this’s associated Document.

### [4.1. The features argument to the open() method](https://drafts.csswg.org/cssom-view-1/#the-features-argument-to-the-open%28%29-method)

Status: unreviewed
<!-- definitions: supported-open-feature-name supported-open-feature-name-width supported-open-feature-name-height supported-open-feature-name-left supported-open-feature-name-top -->

- [ ] [set up browsing context features](https://drafts.csswg.org/cssom-view-1/#set-up-browsing-context-features) — To set up browsing context features for a browsing context target given a map tokenizedFeatures:
  - 1. Let x be null.
  - 2. Let y be null.
  - 3. Let width be null.
  - 4. Let height be null.
  - 5. If tokenizedFeatures["left"] exists:
    - 5a. Set x to the result of invoking the rules for parsing integers on tokenizedFeatures["left"].
    - 5b. If x is an error, set x to 0.
    - 5c. Optionally, clamp x in a user-agent-defined manner so that the window does not move outside the Web-exposed available screen area.
    - 5d. Optionally, move target’s window such that the window’s left edge is at the horizontal coordinate x relative to the left edge of the Web-exposed screen area, measured in CSS pixels of target. The positive axis is rightward.
  - 6. If tokenizedFeatures["top"] exists:
    - 6a. Set y to the result of invoking the rules for parsing integers on tokenizedFeatures["top"].
    - 6b. If y is an error, set y to 0.
    - 6c. Optionally, clamp y in a user-agent-defined manner so that the window does not move outside the Web-exposed available screen area.
    - 6d. Optionally, move target’s window such that the window’s top edge is at the vertical coordinate y relative to the top edge of the Web-exposed screen area, measured in CSS pixels of target. The positive axis is downward.
  - 7. If tokenizedFeatures["width"] exists:
    - 7a. Set width to the result of invoking the rules for parsing integers on tokenizedFeatures["width"].
    - 7b. If width is an error, set width to 0.
    - 7c. If width is not 0:
      - 7c.i. Optionally, clamp width in a user-agent-defined manner so that the window does not get too small or bigger than the Web-exposed available screen area.
      - 7c.ii. Optionally, size target’s window by moving its right edge such that the distance between the left and right edges of the viewport are width CSS pixels of target.
      - 7c.iii. Optionally, move target’s window in a user-agent-defined manner so that it does not grow outside the Web-exposed available screen area.
  - 8. If tokenizedFeatures["height"] exists:
    - 8a. Set height to the result of invoking the rules for parsing integers on tokenizedFeatures["height"].
    - 8b. If height is an error, set height to 0.
    - 8c. If height is not 0:
      - 8c.i. Optionally, clamp height in a user-agent-defined manner so that the window does not get too small or bigger than the Web-exposed available screen area.
      - 8c.ii. Optionally, size target’s window by moving its bottom edge such that the distance between the top and bottom edges of the viewport are height CSS pixels of target.
      - 8c.iii. Optionally, move target’s window in a user-agent-defined manner so that it does not grow outside the Web-exposed available screen area.

### [4.2. The MediaQueryList Interface](https://drafts.csswg.org/cssom-view-1/#the-mediaquerylist-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/CSS/CSSOM/MediaQueryList.cpp; css::MediaQueryMatcher evaluates a document's lists in creation order. matches re-evaluates when the environment may have changed since the last evaluation, as browsers do.
<!-- definitions: mediaquerylist-media-query-list mediaquerylist-document mediaquerylist-media mediaquerylist-matches-state -->

- [x] [evaluate media queries and report changes](https://drafts.csswg.org/cssom-view-1/#evaluate-media-queries-and-report-changes) — When asked to evaluate media queries and report changes for a Document doc, run these steps:
  - 1. For each MediaQueryList object target that has doc as its document, in the order they were created, oldest first, run these substeps:
    - 1a. If target’s matches state has changed since the last time these steps were run, fire an event named change at target using MediaQueryListEvent, with its isTrusted attribute initialized to true, its media attribute initialized to target’s media, and its matches attribute initialized to target’s matches state.

- [x] [MediaQueryList.media](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylist-media) — The media attribute must return the associated media.

- [x] [MediaQueryList.matches](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylist-matches) — The matches attribute must return the associated matches state.

- [x] [MediaQueryList.addListener(callback)](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylist-addlistener) — The addListener(callback) method, when invoked, must run these steps:
  - 1. Add an event listener with this and an event listener whose type is change, and callback is callback.

- [x] [MediaQueryList.removeListener(callback)](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylist-removelistener) — The removeListener(callback) method, when invoked, must run these steps:
  - 1. If this’s event listener list contains an event listener whose type is change, callback is callback, and capture is false, then remove an event listener with this and that event listener.

- [x] [MediaQueryListEvent.media](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylistevent-media) — The media attribute must return the value it was initialized to.

- [x] [MediaQueryListEvent.matches](https://drafts.csswg.org/cssom-view-1/#dom-mediaquerylistevent-matches) — The matches attribute must return the value it was initialized to.

#### [4.2.1. Event summary](https://drafts.csswg.org/cssom-view-1/#mediaquerylist-event-summary)

Status: unreviewed

### [4.3. The Screen Interface](https://drafts.csswg.org/cssom-view-1/#the-screen-interface)

Status: unreviewed
<!-- definitions: dom-screen-pixeldepth -->

- [ ] [Screen.availWidth](https://drafts.csswg.org/cssom-view-1/#dom-screen-availwidth) — The availWidth attribute must return the width of the Web-exposed available screen area.

- [ ] [Screen.availHeight](https://drafts.csswg.org/cssom-view-1/#dom-screen-availheight) — The availHeight attribute must return the height of the Web-exposed available screen area.

- [ ] [Screen.width](https://drafts.csswg.org/cssom-view-1/#dom-screen-width) — The width attribute must return the width of the Web-exposed screen area.

- [ ] [Screen.height](https://drafts.csswg.org/cssom-view-1/#dom-screen-height) — The height attribute must return the height of the Web-exposed screen area.

- [ ] [Screen.colorDepth](https://drafts.csswg.org/cssom-view-1/#dom-screen-colordepth) — The colorDepth and pixelDepth attributes should return the number of bits allocated to colors for a pixel in the output device, excluding the alpha channel. If the user agent is not able to return the number of bits used by the output device, it should return the closest estimation such as, for example, the number of bits used by the frame buffer sent to the display or any internal representation that would be the closest to the value the output device would use. The user agent must return a value for these attributes at least equal to the value of the color media feature multiplied by three. If the different color components are not represented with the same number of bits, the returned value may be greater than three times the value of the color media feature. If the user agent does not…

## [5. Extensions to the Document Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-document-interface)

Status: unreviewed

- [ ] [Document.elementFromPoint(x, y)](https://drafts.csswg.org/cssom-view-1/#dom-document-elementfrompoint) — The elementFromPoint(x, y) method must follow these steps:
  - 1. If either argument is negative, x is greater than the viewport width excluding the size of a rendered scroll bar (if any), or y is greater than the viewport height excluding the size of a rendered scroll bar (if any), or there is no viewport associated with the document, return null and terminate these steps.
  - 2. If there is a box in the viewport that would be a target for hit testing at coordinates x,y, when applying the transforms that apply to the descendants of the viewport, return the associated element and terminate these steps.
  - 3. If the document has a root element, return the root element and terminate these steps.
  - 4. Return null.

- [ ] [Document.elementsFromPoint(x, y)](https://drafts.csswg.org/cssom-view-1/#dom-document-elementsfrompoint) — The elementsFromPoint(x, y) method must follow these steps:
  - 1. Let sequence be a new empty sequence.
  - 2. If either argument is negative, x is greater than the viewport width excluding the size of a rendered scroll bar (if any), or y is greater than the viewport height excluding the size of a rendered scroll bar (if any), or there is no viewport associated with the document, return sequence and terminate these steps.
  - 3. For each box in the viewport, in paint order, starting with the topmost box, that would be a target for hit testing at coordinates x,y even if nothing would be overlapping it, when applying the transforms that apply to the descendants of the viewport, append the associated element to sequence.
  - 4. If the document has a root element, and the last item in sequence is not the root element, append the root element to sequence.
  - 5. Return sequence.

- [ ] [Document.caretPositionFromPoint(x, y, options)](https://drafts.csswg.org/cssom-view-1/#dom-document-caretpositionfrompoint) — The caretPositionFromPoint(x, y, options) method must return the result of running these steps:
  - 1. If there is no viewport associated with the document, return null.
  - 2. If either argument is negative, x is greater than the viewport width excluding the size of a rendered scroll bar (if any), y is greater than the viewport height excluding the size of a rendered scroll bar (if any) return null.
  - 3. If at the coordinates x,y in the viewport no text insertion point indicator would have been inserted when applying the transforms that apply to the descendants of the viewport, return null.
  - 4. If at the coordinates x,y in the viewport a text insertion point indicator would have been inserted in a text entry widget which is also a replaced element, when applying the transforms that apply to the descendants of the viewport, return a caret position with its properties set as follows: caret node The node corresponding to the text entry widget. caret offset The amount of 16-bit units to the…
  - 5. Otherwise:
    - 5a. Let caretPosition be a tuple consisting of a caretPositionNode (a node) and a caretPositionOffset (a non-negative integer) for the position where the text insertion point indicator would have been inserted when applying the transforms that apply to the descendants of the viewport.
    - 5b. Let startNode be the caretPositionNode of the caretPosition, and let startOffset be the caretPositionOffset of the caretPosition.
    - 5c. While startNode is a node, startNode’s root is a shadow root, and startNode’s root is not a shadow-including inclusive ancestor of any of options["shadowRoots"], repeat these steps:
      - 5c.i. Set startOffset to index of startNode’s root’s host.
      - 5c.ii. Set startNode to startNode’s root’s host’s parent.
    - 5d. Return a caret position with its properties set as follows:
      - 5d.i. caret node is set to startNode.
      - 5d.ii. caret offset is set to startOffset.

- [ ] [Document.scrollingElement](https://drafts.csswg.org/cssom-view-1/#dom-document-scrollingelement) — The scrollingElement attribute, on getting, must run these steps:
  - 1. If the Document is in quirks mode, follow these substeps:
    - 1a. If the body element exists, and it is not potentially scrollable in either axis, return the body element and abort these steps. For this purpose, a value of overflow:clip on the the body element’s parent element must be treated as overflow:hidden.
    - 1b. Return null and abort these steps.
  - 2. If there is a root element, return the root element and abort these steps.
  - 3. Return null.

### [5.1. The CaretPosition Interface](https://drafts.csswg.org/cssom-view-1/#the-caretposition-interface)

Status: unreviewed
<!-- definitions: caret-position caret-node caret-offset -->

- [ ] [CaretPosition.offsetNode](https://drafts.csswg.org/cssom-view-1/#dom-caretposition-offsetnode) — The offsetNode attribute must return the caret node.

- [ ] [CaretPosition.offset](https://drafts.csswg.org/cssom-view-1/#dom-caretposition-offset) — The offset attribute must return the caret offset.

- [ ] [CaretPosition.getClientRect()](https://drafts.csswg.org/cssom-view-1/#dom-caretposition-getclientrect) — The getClientRect() method must follow these steps, aborting on the first step that returns a value:
  - 1. If caret node is a text entry widget that is a replaced element, and that is in the document, return a scaled DOMRect object for the caret in the widget as represented by the caret offset value. The transforms that apply to the element and its ancestors are applied.
  - 2. Otherwise:
    - 2a. Let caretRange be a collapsed Range object whose start node and end node are set to caret node, and whose start offset and end offset are set to caret offset.
    - 2b. Return the DOMRect object which is the result of invoking the getBoundingClientRect() method on caretRange.

## [6. Extensions to the Element Interface](https://drafts.csswg.org/cssom-view-1/#extension-to-the-element-interface)

Status: unreviewed
<!-- definitions: element-get-the-bounding-box -->

- [ ] [Element.getClientRects()](https://drafts.csswg.org/cssom-view-1/#dom-element-getclientrects) — The getClientRects() method, when invoked, must return the result of the following algorithm:
  - 1. If the element on which it was invoked does not have an associated box return an empty DOMRectList object and stop this algorithm.
  - 2. If the element has an associated SVG layout box return a scaled DOMRectList object containing a single DOMRect object that describes the bounding box of the element as defined by the SVG specification, applying the transforms that apply to the element and its ancestors.
  - 3. Return a DOMRectList object containing DOMRect objects in content order, one for each box fragment, describing its border area (including those with a height or width of zero) with the following constraints: Apply the transforms that apply to the element and its ancestors.; If the element on which the method was invoked has a computed value for the display property of table or inline-table includ…

- [ ] [Element.getBoundingClientRect()](https://drafts.csswg.org/cssom-view-1/#dom-element-getboundingclientrect) — The getBoundingClientRect() method, when invoked on an element element, must return the result of getting the bounding box for element.

- [ ] [Element.checkVisibility(options)](https://drafts.csswg.org/cssom-view-1/#dom-element-checkvisibility) — The checkVisibility(options) method must run these steps, when called on an element this:
  - 1. If this does not have an associated box, return false.
  - 2. If an ancestor of this in the flat tree has content-visibility: hidden, return false.
  - 3. If either the opacityProperty or the checkOpacity dictionary members of options are true, and this, or an ancestor of this in the flat tree, has a computed opacity value of 0, return false.
  - 4. If either the visibilityProperty or the checkVisibilityCSS dictionary members of options are true, and this is invisible, return false.
  - 5. If the contentVisibilityAuto dictionary member of options is true and an ancestor of this in the flat tree skips its contents due to content-visibility: auto, return false.
  - 6. Return true.

- [ ] [Element.scrollIntoView(arg)](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollintoview) — The scrollIntoView(arg) method must run these steps:
  - 1. Let behavior be "auto".
  - 2. Let block be "start".
  - 3. Let inline be "nearest".
  - 4. Let container be null.
  - 5. If arg is a ScrollIntoViewOptions dictionary, then:
    - 5a. Set behavior to the behavior dictionary member of options.
    - 5b. Set block to the block dictionary member of options.
    - 5c. Set inline to the inline dictionary member of options.
    - 5d. If the container dictionary member of options is "nearest", set container to the element.
  - 6. Otherwise, if arg is false, then set block to "end".
  - 7. If the element does not have any associated box, or is not available to user-agent features, then return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 8. Scroll the element into view with behavior, block, inline, and container. Let scrollPromise be the Promise returned from this step
  - 9. Optionally perform some other action that brings the element to the user’s attention.
  - 10. Return scrollPromise.

- [ ] [Element.scroll()](https://drafts.csswg.org/cssom-view-1/#dom-element-scroll) — The scroll() method must run these steps:
  - 1. If invoked with one argument, follow these substeps:
    - 1a. Let options be the argument.
    - 1b. Normalize non-finite values for left and top dictionary members of options, if present.
    - 1c. Let x be the value of the left dictionary member of options, if present, or the element’s current scroll position on the x axis otherwise.
    - 1d. Let y be the value of the top dictionary member of options, if present, or the element’s current scroll position on the y axis otherwise.
  - 2. If invoked with two arguments, follow these substeps:
    - 2a. Let options be null converted to a ScrollToOptions dictionary. [WEBIDL]
    - 2b. Let x and y be the arguments, respectively.
    - 2c. Normalize non-finite values for x and y.
    - 2d. Let the left dictionary member of options have the value x.
    - 2e. Let the top dictionary member of options have the value y.
  - 3. Let document be the element’s node document.
  - 4. If document is not the active document, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 5. Let window be the value of document’s defaultView attribute.
  - 6. If window is null, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 7. If the element is the root element and document is in quirks mode, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 8. If the element is the root element, return the Promise returned by scroll() on window after the method is invoked with scrollX on window as first argument and y as second argument, and abort the remaining steps.
  - 9. If the element is the body element, document is in quirks mode, and the element is not potentially scrollable in either axis, return the Promise returned by scroll() on window after the method is invoked with options as the only argument, and abort the remaining steps.
  - 10. If the element does not have any associated box, the element has no associated scrolling box, or the element has no overflow, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 11. Scroll the element to x,y, with the scroll behavior being the value of the behavior dictionary member of options. Let scrollPromise be the Promise returned from this step.
  - 12. Return scrollPromise.

- [ ] [Element.scrollTo()](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollto) — When the scrollTo() method is invoked, the user agent must act as if the scroll() method was invoked with the same arguments.

- [ ] [Element.scrollBy()](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollby) — When the scrollBy() method is invoked, the user agent must run these steps:
  - 1. If invoked with one argument, follow these substeps:
    - 1a. Let options be the argument.
    - 1b. Normalize non-finite values for left and top dictionary members of options, if present.
  - 2. If invoked with two arguments, follow these substeps:
    - 2a. Let options be null converted to a ScrollToOptions dictionary. [WEBIDL]
    - 2b. Let x and y be the arguments, respectively.
    - 2c. Normalize non-finite values for x and y.
    - 2d. Let the left dictionary member of options have the value x.
    - 2e. Let the top dictionary member of options have the value y.
  - 3. Add the value of scrollLeft to the left dictionary member.
  - 4. Add the value of scrollTop to the top dictionary member.
  - 5. Return the Promise returned by scroll() after the method is invoked with options as the only argument.

- [ ] [Element.scrollTop](https://drafts.csswg.org/cssom-view-1/#dom-element-scrolltop) — The scrollTop attribute, on getting, must return the result of running these steps:
  - 1. Let document be the element’s node document.
  - 2. If document is not the active document, return zero and terminate these steps.
  - 3. Let window be the value of document’s defaultView attribute.
  - 4. If window is null, return zero and terminate these steps.
  - 5. If the element is the root element and document is in quirks mode, return zero and terminate these steps.
  - 6. If the element is the root element return the value of scrollY on window.
  - 7. If the element is the body element, document is in quirks mode, and the element is not potentially scrollable in at least one axis, return the value of scrollY on window.
  - 8. If the element does not have any associated box, return zero and terminate these steps.
  - 9. Return the y-coordinate of the scrolling area at the alignment point with the top of the padding edge of the element.

- [ ] [When setting the scrollTop attribute these steps must be run](https://drafts.csswg.org/cssom-view-1/#dom-element-scrolltop) — When setting the scrollTop attribute these steps must be run:
  - 1. Let y be the given value.
  - 2. Normalize non-finite values for y.
  - 3. Let document be the element’s node document.
  - 4. If document is not the active document, terminate these steps.
  - 5. Let window be the value of document’s defaultView attribute.
  - 6. If window is null, terminate these steps.
  - 7. If the element is the root element and document is in quirks mode, terminate these steps.
  - 8. If the element is the root element invoke scroll() on window with scrollX on window as first argument and y as second argument, and terminate these steps.
  - 9. If the element is the body element, document is in quirks mode, and the element is not potentially scrollable in at least one axis, invoke scroll() on window with scrollX as first argument and y as second argument, and terminate these steps.
  - 10. If the element does not have any associated box, the element has no associated scrolling box, or the element has no overflow, terminate these steps.
  - 11. Scroll the element to scrollLeft,y, with the scroll behavior being "auto".

- [ ] [Element.scrollLeft](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollleft) — The scrollLeft attribute, on getting, must return the result of running these steps:
  - 1. Let document be the element’s node document.
  - 2. If document is not the active document, return zero and terminate these steps.
  - 3. Let window be the value of document’s defaultView attribute.
  - 4. If window is null, return zero and terminate these steps.
  - 5. If the element is the root element and document is in quirks mode, return zero and terminate these steps.
  - 6. If the element is the root element return the value of scrollX on window.
  - 7. If the element is the body element, document is in quirks mode, and the element is not potentially scrollable in at least one axis, return the value of scrollX on window.
  - 8. If the element does not have any associated box, return zero and terminate these steps.
  - 9. Return the x-coordinate of the scrolling area at the alignment point with the left of the padding edge of the element.

- [ ] [When setting the scrollLeft attribute these steps must be run](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollleft) — When setting the scrollLeft attribute these steps must be run:
  - 1. Let x be the given value.
  - 2. Normalize non-finite values for x.
  - 3. Let document be the element’s node document.
  - 4. If document is not the active document, terminate these steps.
  - 5. Let window be the value of document’s defaultView attribute.
  - 6. If window is null, terminate these steps.
  - 7. If the element is the root element and document is in quirks mode, terminate these steps.
  - 8. If the element is the root element invoke scroll() on window with x as first argument and scrollY on window as second argument, and terminate these steps.
  - 9. If the element is the body element, document is in quirks mode, and the element is not potentially scrollable in at least one axis, invoke scroll() on window with x as first argument and scrollY on window as second argument, and terminate these steps.
  - 10. If the element does not have any associated box, the element has no associated scrolling box, or the element has no overflow, terminate these steps.
  - 11. Scroll the element to x,scrollTop, with the scroll behavior being "auto".

- [ ] [Element.scrollWidth](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollwidth) — The scrollWidth attribute must return the result of running these steps:
  - 1. Let document be the element’s node document.
  - 2. If document is not the active document, return zero and terminate these steps.
  - 3. Let viewport width be the width of the viewport excluding the width of the scroll bar, if any, or zero if there is no viewport.
  - 4. If the element is the root element and document is not in quirks mode return max(viewport scrolling area width, viewport width).
  - 5. If the element is the body element, document is in quirks mode and the element is not potentially scrollable in the x axis, return max(viewport scrolling area width, viewport width).
  - 6. If the element does not have any associated box return zero and terminate these steps.
  - 7. Return the width of the element’s scrolling area.

- [ ] [Element.scrollHeight](https://drafts.csswg.org/cssom-view-1/#dom-element-scrollheight) — The scrollHeight attribute must return the result of running these steps:
  - 1. Let document be the element’s node document.
  - 2. If document is not the active document, return zero and terminate these steps.
  - 3. Let viewport height be the height of the viewport excluding the height of the scroll bar, if any, or zero if there is no viewport.
  - 4. If the element is the root element and document is not in quirks mode return max(viewport scrolling area height, viewport height).
  - 5. If the element is the body element, document is in quirks mode and the element is not potentially scrollable in the y axis, return max(viewport scrolling area height, viewport height).
  - 6. If the element does not have any associated box return zero and terminate these steps.
  - 7. Return the height of the element’s scrolling area.

- [ ] [Element.clientTop](https://drafts.csswg.org/cssom-view-1/#dom-element-clienttop) — The clientTop attribute must run these steps:
  - 1. If the element has no associated box or if the box is inline, return zero.
  - 2. Return the unscaled computed value of the border-top-width property plus the height of any scrollbar rendered between the top padding edge and the top border edge, ignoring any transforms that apply to the element and its ancestors.

- [ ] [Element.clientLeft](https://drafts.csswg.org/cssom-view-1/#dom-element-clientleft) — The clientLeft attribute must run these steps:
  - 1. If the element has no associated box or if the box is inline, return zero.
  - 2. Return the unscaled computed value of the border-left-width property plus the width of any scrollbar rendered between the left padding edge and the left border edge, ignoring any transforms that apply to the element and its ancestors.

- [ ] [Element.clientWidth](https://drafts.csswg.org/cssom-view-1/#dom-element-clientwidth) — The clientWidth attribute must run these steps:
  - 1. If the element has no associated box or if the box is inline, return zero.
  - 2. If the element is the root element and the element’s node document is not in quirks mode, or if the element is the body element and the element’s node document is in quirks mode, return the viewport width excluding the size of a rendered scroll bar (if any).
  - 3. Return the unscaled width of the padding edge excluding the width of any rendered scrollbar between the padding edge and the border edge, ignoring any transforms or that apply to the element and its ancestors.

- [ ] [Element.clientHeight](https://drafts.csswg.org/cssom-view-1/#dom-element-clientheight) — The clientHeight attribute must run these steps:
  - 1. If the element has no associated box or if the box is inline, return zero.
  - 2. If the element is the root element and the element’s node document is not in quirks mode, or if the element is the body element and the element’s node document is in quirks mode, return the viewport height excluding the size of a rendered scroll bar (if any).
  - 3. Return the unscaled height of the padding edge excluding the height of any rendered scrollbar between the padding edge and the border edge, ignoring any transforms that apply to the element and its ancestors.

- [ ] [Element.currentCSSZoom](https://drafts.csswg.org/cssom-view-1/#dom-element-currentcsszoom) — The currentCSSZoom attribute must return the effective zoom of the element, or 1.0 if the element isn’t being rendered.

### [6.1. Element Scrolling Members](https://drafts.csswg.org/cssom-view-1/#element-scrolling-members)

Status: unreviewed
<!-- definitions: scroll-a-target-into-view -->

- [ ] [determine the scroll-into-view position](https://drafts.csswg.org/cssom-view-1/#determine-the-scroll-into-view-position) — To determine the scroll-into-view position of a target, which is an Element, pseudo-element, or Range, with a scroll behavior behavior, a block flow direction position block, an inline base direction position inline, and a scrolling box scrolling box, run the following steps:
  - 1. Let target bounding border box be the box represented by the return value of invoking Element’s getBoundingClientRect(), if target is an Element, or Range’s getBoundingClientRect(), if target is a Range.
  - 2. Let scrolling box edge A be the beginning edge in the block flow direction of scrolling box, and let element edge A be target bounding border box’s edge on the same physical side as that of scrolling box edge A.
  - 3. Let scrolling box edge B be the ending edge in the block flow direction of scrolling box, and let element edge B be target bounding border box’s edge on the same physical side as that of scrolling box edge B.
  - 4. Let scrolling box edge C be the beginning edge in the inline base direction of scrolling box, and let element edge C be target bounding border box’s edge on the same physical side as that of scrolling box edge C.
  - 5. Let scrolling box edge D be the ending edge in the inline base direction of scrolling box, and let element edge D be target bounding border box’s edge on the same physical side as that of scrolling box edge D.
  - 6. Let element height be the distance between element edge A and element edge B.
  - 7. Let scrolling box height be the distance between scrolling box edge A and scrolling box edge B.
  - 8. Let element width be the distance between element edge C and element edge D.
  - 9. Let scrolling box width be the distance between scrolling box edge C and scrolling box edge D.
  - 10. Let position be the scroll position scrolling box would have by following these steps:
    - 10a. If block is "start", then align element edge A with scrolling box edge A.
    - 10b. Otherwise, if block is "end", then align element edge B with scrolling box edge B.
    - 10c. Otherwise, if block is "center", then align the center of target bounding border box with the center of scrolling box in scrolling box’s block flow direction.
    - 10d. Otherwise, block is "nearest": If element edge A and element edge B are both outside scrolling box edge A and scrolling box edge B Do nothing. If element edge A is outside scrolling box edge A and element height is less than scrolling box height If element edge B is outside scrolling box edge B and element height is greater than scrolling box height Align element edge A with scrolling box edge A.…
    - 10e. If inline is "start", then align element edge C with scrolling box edge C.
    - 10f. Otherwise, if inline is "end", then align element edge D with scrolling box edge D.
    - 10g. Otherwise, if inline is "center", then align the center of target bounding border box with the center of scrolling box in scrolling box’s inline base direction.
    - 10h. Otherwise, inline is "nearest": If element edge C and element edge D are both outside scrolling box edge C and scrolling box edge D Do nothing. If element edge C is outside scrolling box edge C and element width is less than scrolling box width If element edge D is outside scrolling box edge D and element width is greater than scrolling box width Align element edge C with scrolling box edge C. If…
    - 10i. If target is an Element, and the target element defines some scroll snap positions, then the user agent must scroll snap the resulting position to one of that element’s scroll snap positions if its nearest scroll container is a scroll snap container. The user agent may also do this even when the scroll container has scroll-snap-type: none.
    - 10j. Return position.

- [ ] [scroll an element](https://drafts.csswg.org/cssom-view-1/#scroll-an-element) — To scroll an element (or pseudo-element) element to x,y optionally with a scroll behavior behavior (which is "auto" if omitted) means to:
  - 1. Let box be element’s associated scrolling box.
  - 2. If box has rightward overflow direction Let x be max(0, min(x, element scrolling area width - element padding edge width)). If box has leftward overflow direction Let x be min(0, max(x, element padding edge width - element scrolling area width)).
  - 3. If box has downward overflow direction Let y be max(0, min(y, element scrolling area height - element padding edge height)). If box has upward overflow direction Let y be min(0, max(y, element padding edge height - element scrolling area height)).
  - 4. Let position be the scroll position box would have by aligning scrolling area x-coordinate x with the left of box and aligning scrolling area y-coordinate y with the top of box.
  - 5. If position is the same as box’s current scroll position, and box does not have an ongoing smooth scroll, return a promise resolved with a non-interrupted scroll result and abort the remaining steps.
  - 6. Perform a scroll of box to position, element as the associated element and behavior as the scroll behavior. Let scrollPromise be the Promise returned from this step.
  - 7. Return scrollPromise.

## [7. Extensions to the HTMLElement Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-htmlelement-interface)

Status: unreviewed

- [ ] [HTMLElement.scrollParent](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-scrollparent) — The scrollParent attribute must return the result of running these steps:
  - 1. If any of the following holds true, return null and terminate this algorithm: The element does not have an associated box.; The element is the root element.; The element is the body element.; The element’s computed value of the position property is fixed and no ancestor establishes a fixed position containing block.
  - 2. Let ancestor be the containing block of the element in the flat tree and repeat these substeps:
    - 2a. If ancestor is the initial containing block, return the scrollingElement for the element’s document if it is not closed-shadow-hidden from the element, otherwise return null.
    - 2b. If ancestor is not closed-shadow-hidden from the element, and is a scroll container, terminate this algorithm and return ancestor.
    - 2c. If the computed value of the position property of ancestor is fixed, and no ancestor establishes a fixed position containing block, terminate this algorithm and return null.
    - 2d. Let ancestor be the containing block of ancestor in the flat tree.

- [ ] [HTMLElement.offsetParent](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-offsetparent) — The offsetParent attribute must return the result of running these steps:
  - 1. If any of the following holds true return null and terminate this algorithm: The element does not have an associated box.; The element is the root element.; The element is the body element.; The element’s computed value of the position property is fixed and no ancestor establishes a fixed position containing block.
  - 2. Let ancestor be the parent of the element in the flat tree and repeat these substeps:
    - 2a. If ancestor is closed-shadow-hidden from the element, its computed value of the position property is fixed, and no ancestor establishes a fixed position containing block, terminate this algorithm and return null.
    - 2b. If ancestor is not closed-shadow-hidden from the element and satisfies at least one of the following, terminate this algorithm and return ancestor. The element is in a fixed position containing block, and ancestor is a containing block for fixed-positioned descendants.; The element is not in a fixed position containing block, and: ancestor is a containing block of absolutely-positioned descendant…
    - 2c. If there is no more parent of ancestor in the flat tree, terminate this algorithm and return null.
    - 2d. Let ancestor be the parent of ancestor in the flat tree.

- [ ] [HTMLElement.offsetTop](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-offsettop) — The offsetTop attribute must return the result of running these steps:
  - 1. If the element is the body element or does not have any associated box return zero and terminate this algorithm.
  - 2. If the offsetParent of the element is null return the unscaled y-coordinate of the top border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transformsthat apply to the element and its ancestors and terminate this algorithm.
  - 3. Return the unscaled result of subtracting the y-coordinate of the top padding edge of the first box associated with the offsetParent of the element from the y-coordinate of the top border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transforms that apply to the element and its ancestors.

- [ ] [HTMLElement.offsetLeft](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-offsetleft) — The offsetLeft attribute must return the result of running these steps:
  - 1. If the element is the body element or does not have any associated box return zero and terminate this algorithm.
  - 2. If the offsetParent of the element is null return the unscaled x-coordinate of the left border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transforms that apply to the element and its ancestors, and terminate this algorithm.
  - 3. Return the unscaled result of subtracting the x-coordinate of the left padding edge of the first box associated with the offsetParent of the element from the x-coordinate of the left border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transforms that apply to the element and its ancestors.

- [ ] [HTMLElement.offsetWidth](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-offsetwidth) — The offsetWidth attribute must return the result of running these steps:
  - 1. If the element does not have any associated box return zero and terminate this algorithm.
  - 2. Return the unscaled width of the axis-aligned bounding box of the border boxes of all fragments generated by the element’s principal box, ignoring any transforms that apply to the element and its ancestors. If the element’s principal box is an inline-level box which was "split" by a block-level descendant, also include fragments generated by the block-level descendants, unless they are zero width…

- [ ] [HTMLElement.offsetHeight](https://drafts.csswg.org/cssom-view-1/#dom-htmlelement-offsetheight) — The offsetHeight attribute must return the result of running these steps:
  - 1. If the element does not have any associated box return zero and terminate this algorithm.
  - 2. Return the unscaled height of the axis-aligned bounding box of the border boxes of all fragments generated by the element’s principal box, ignoring any transforms that apply to the element and its ancestors. If the element’s principal box is an inline-level box which was "split" by a block-level descendant, also include fragments generated by the block-level descendants, unless they are zero widt…

## [8. Extensions to the HTMLImageElement Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-htmlimageelement-interface)

Status: unreviewed

- [ ] [HTMLImageElement.x](https://drafts.csswg.org/cssom-view-1/#dom-htmlimageelement-x) — The x attribute, on getting, must return the scaled x-coordinate of the left border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transforms that apply to the element and its ancestors, or zero if there is no box.

- [ ] [HTMLImageElement.y](https://drafts.csswg.org/cssom-view-1/#dom-htmlimageelement-y) — The y attribute, on getting, must return the scaled y-coordinate of the top border edge of the first box associated with the element, relative to the initial containing block origin, ignoring any transforms that apply to the element and its ancestors, or zero if there is no box.

## [9. Extensions to the Range Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-range-interface)

Status: unreviewed

- [ ] [Range.getClientRects()](https://drafts.csswg.org/cssom-view-1/#dom-range-getclientrects) — The getClientRects() method, when invoked, must return an empty DOMRectList object if the range is not in the document and otherwise a DOMRectList object containing a list of DOMRect objects in content order that matches the following constraints:

- [ ] [Range.getBoundingClientRect()](https://drafts.csswg.org/cssom-view-1/#dom-range-getboundingclientrect) — The getBoundingClientRect() method, when invoked, must return the result of the following algorithm:
  - 1. Let list be the result of invoking getClientRects() on the same range this method was invoked on.
  - 2. If list is empty return a DOMRect object whose x, y, width and height members are zero.
  - 3. If all rectangles in list have zero width or height, return the first rectangle in list.
  - 4. Otherwise, return a DOMRect object describing the smallest rectangle that includes all of the rectangles in list of which the height or width is not zero.

## [10. Extensions to the MouseEvent Interface](https://drafts.csswg.org/cssom-view-1/#extensions-to-the-mouseevent-interface)

Status: unreviewed

- [ ] [MouseEvent.screenX](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-screenx) — The screenX attribute must return the x-coordinate of the position where the event occurred relative to the origin of the Web-exposed screen area.

- [ ] [MouseEvent.screenY](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-screeny) — The screenY attribute must return the y-coordinate of the position where the event occurred relative to the origin of the Web-exposed screen area.

- [ ] [MouseEvent.pageX](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-pagex) — The pageX attribute must follow these steps:
  - 1. If the event’s dispatch flag is set, return the horizontal coordinate of the position where the event occurred relative to the origin of the initial containing block and terminate these steps.
  - 2. Let offset be the value of the scrollX attribute of the event’s associated Window object, if there is one, or zero otherwise.
  - 3. Return the sum of offset and the value of the event’s clientX attribute.

- [ ] [MouseEvent.pageY](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-pagey) — The pageY attribute must follow these steps:
  - 1. If the event’s dispatch flag is set, return the vertical coordinate of the position where the event occurred relative to the origin of the initial containing block and terminate these steps.
  - 2. Let offset be the value of the scrollY attribute of the event’s associated Window object, if there is one, or zero otherwise.
  - 3. Return the sum of offset and the value of the event’s clientY attribute.

- [ ] [MouseEvent.clientX](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-clientx) — The clientX attribute must return the x-coordinate of the position where the event occurred relative to the origin of the viewport.

- [ ] [MouseEvent.clientY](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-clienty) — The clientY attribute must return the y-coordinate of the position where the event occurred relative to the origin of the viewport.

- [ ] [MouseEvent.x](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-x) — The x attribute must return the value of clientX.

- [ ] [MouseEvent.y](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-y) — The y attribute must return the value of clientY.

- [ ] [MouseEvent.offsetX](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-offsetx) — The offsetX attribute must follow these steps:
  - 1. If the event’s dispatch flag is set, return the x-coordinate of the position where the event occurred relative to the origin of the padding edge of the target node, ignoring the transforms that apply to the element and its ancestors, and terminate these steps.
  - 2. Return the value of the event’s pageX attribute.

- [ ] [MouseEvent.offsetY](https://drafts.csswg.org/cssom-view-1/#dom-mouseevent-offsety) — The offsetY attribute must follow these steps:
  - 1. If the event’s dispatch flag is set, return the y-coordinate of the position where the event occurred relative to the origin of the padding edge of the target node, ignoring the transforms that apply to the element and its ancestors, and terminate these steps.
  - 2. Return the value of the event’s pageY attribute.

## [11. Geometry](https://drafts.csswg.org/cssom-view-1/#geometry)

Status: unreviewed

### [11.1. The GeometryUtils Interface](https://drafts.csswg.org/cssom-view-1/#the-geometryutils-interface)

Status: unreviewed
<!-- definitions: geometryutils-associated-document get-the-complete-transform adjust-a-point-to-the-border-box adjust-a-point-from-the-border-box -->

- [ ] [check the documents for coordinate conversion](https://drafts.csswg.org/cssom-view-1/#check-the-documents-for-coordinate-conversion) — To check the documents for coordinate conversion given two GeometryNodes node and from:
  - 1. Let nodeDocuments be a list containing node’s associated document. While the last document in nodeDocuments has a container document, append that container document to nodeDocuments.
  - 2. Let fromDocuments be a list constructed in the same way for from.
  - 3. Let documents contain the shortest prefixes of nodeDocuments and fromDocuments that end with a document common to both lists. If there is no such document, let documents contain only the first item of each list.
  - 4. If any document in documents is not same origin with the associated document of node, then throw a SecurityError.

- [ ] [GeometryUtils.getBoxQuads(options)](https://drafts.csswg.org/cssom-view-1/#dom-geometryutils-getboxquads) — The getBoxQuads(options) method must run the following steps:
  - 1. Let node be this.
  - 2. If node is a Document, let document be node.
  - 3. Otherwise, if node is a CSSPseudoElement, let document be node’s element’s node document.
  - 4. Otherwise, let document be node’s node document.
  - 5. If options.relativeTo is specified, let relativeTo be options.relativeTo. Otherwise, let relativeTo be document.
  - 6. Let result be an empty list of DOMQuad objects.
  - 7. If node is a Document, then:
    - 7a. Let rect be a DOMRect whose x and y are 0, and whose width and height are the width and height of the layout viewport.
    - 7b. Let quad be the result of invoking fromRect() with rect.
    - 7c. Append the result of invoking the convertQuadFromNode() method on relativeTo with quad and document to result.
  - 8. Otherwise, if node is a Text, then:
    - 8a. For each DOMRect rect returned by invoking the getClientRects() method on a Range whose boundary-points select exactly node, in content order:
      - 8a.i. Let quad be the result of invoking fromRect() with rect.
      - 8a.ii. Append the result of invoking the convertQuadFromNode() method on relativeTo with quad and document to result.
  - 9. Otherwise, for each box fragment of node, in flat tree order, run the following steps:
    - 9a. Let rect be a DOMRect describing the border area of the box fragment, positioned in the fragment’s local coordinate space with its border edge at x = 0 and y = 0.
    - 9b. If options.box is "margin", let rect instead be a DOMRect describing the fragment’s margin area in the same local coordinate space, with the fragment’s border edge still at x = 0 and y = 0.
    - 9c. If options.box is "padding", let rect instead be a DOMRect describing the fragment’s padding area in the same local coordinate space, with the fragment’s border edge still at x = 0 and y = 0.
    - 9d. If options.box is "content", let rect instead be a DOMRect describing the fragment’s content area in the same local coordinate space, with the fragment’s border edge still at x = 0 and y = 0.
    - 9e. Let quad be the result of invoking fromRect() with rect.
    - 9f. Append the result of invoking the convertQuadFromNode() method on relativeTo with quad and node to result.
  - 10. Return result.

- [ ] [GeometryUtils.convertQuadFromNode(quad, from, options)](https://drafts.csswg.org/cssom-view-1/#dom-geometryutils-convertquadfromnode) — The convertQuadFromNode(quad, from, options) method must run the following steps:
  - 1. Let node be this.
  - 2. Let convertedQuad be the result of invoking fromQuad() with quad.
  - 3. Let p1 be the result of invoking convertPointFromNode() on node with convertedQuad.p1, from, and options.
  - 4. Let p2 be the result of invoking convertPointFromNode() on node with convertedQuad.p2, from, and options.
  - 5. Let p3 be the result of invoking convertPointFromNode() on node with convertedQuad.p3, from, and options.
  - 6. Let p4 be the result of invoking convertPointFromNode() on node with convertedQuad.p4, from, and options.
  - 7. Return a new DOMQuad with p1, p2, p3, and p4.

- [ ] [GeometryUtils.convertRectFromNode(rect, from, options)](https://drafts.csswg.org/cssom-view-1/#dom-geometryutils-convertrectfromnode) — The convertRectFromNode(rect, from, options) method must run the following steps:
  - 1. Let node be this.
  - 2. Let quad be the result of invoking fromRect() with rect.
  - 3. Return the result of invoking convertQuadFromNode() on node with quad, from, and options.

- [ ] [GeometryUtils.convertPointFromNode(point, from, options)](https://drafts.csswg.org/cssom-view-1/#dom-geometryutils-convertpointfromnode) — The convertPointFromNode(point, from, options) method returns a point flattened to 2D and must run the following steps, ignoring the z and w members of point:
  - 1. Let node be this.
  - 2. Check the documents for coordinate conversion given node and from.
  - 3. Let thisTransformToViewport be the result of getting the complete transform of node relative to null.
  - 4. Let fromTransformToViewport be the result of getting the complete transform of from relative to null.
  - 5. Let adjustedPoint be the result of adjusting a point to the border box given point, from, and options.fromBox.
  - 6. Set adjustedPoint to the result of transforming adjustedPoint by fromTransformToViewport.
  - 7. Set adjustedPoint to the result of transforming adjustedPoint by the inverse of thisTransformToViewport.
  - 8. Return the result of adjusting a point from the border box given adjustedPoint, node, and options.toBox.

## [12. VisualViewport](https://drafts.csswg.org/cssom-view-1/#visualViewport)

Status: unreviewed

### [12.1. The VisualViewport Interface](https://drafts.csswg.org/cssom-view-1/#the-visualviewport-interface)

Status: unreviewed
<!-- definitions: dom-visualviewport-onresize dom-visualviewport-onscroll dom-visualviewport-onscrollend -->

- [ ] [VisualViewport.offsetLeft](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-offsetleft) — The offsetLeft attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the offset of the left edge of the visual viewport from the left edge of the layout viewport.

- [ ] [VisualViewport.offsetTop](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-offsettop) — The offsetTop attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the offset of the top edge of the visual viewport from the top edge of the layout viewport.

- [ ] [VisualViewport.pageLeft](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-pageleft) — The pageLeft attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the offset of the left edge of the visual viewport from the left edge of the initial containing block of the layout viewport’s document.

- [ ] [VisualViewport.pageTop](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-pagetop) — The pageTop attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the offset of the top edge of the visual viewport from the top edge of the initial containing block of the layout viewport’s document.

- [ ] [VisualViewport.width](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-width) — The width attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the width of the visual viewport excluding the width of any rendered vertical classic scrollbar that is fixed to the visual viewport.

- [ ] [VisualViewport.height](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-height) — The height attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0.
  - 2. Otherwise, return the height of the visual viewport excluding the height of any rendered horizontal classic scrollbar that is fixed to the visual viewport.

- [ ] [VisualViewport.scale](https://drafts.csswg.org/cssom-view-1/#dom-visualviewport-scale) — The scale attribute must run these steps:
  - 1. If the visual viewport’s associated document is not fully active, return 0 and abort these steps.
  - 2. If there is no output device, return 1 and abort these steps.
  - 3. Otherwise, return the visual viewport’s scale factor.

## [13. Events](https://drafts.csswg.org/cssom-view-1/#events)

Status: unreviewed

### [13.1. Resizing viewports](https://drafts.csswg.org/cssom-view-1/#resizing-viewports)

Status: unreviewed

- [ ] [run the resize steps (Document)](https://drafts.csswg.org/cssom-view-1/#document-run-the-resize-steps) — When asked to run the resize steps for a Document doc, run these steps:
  - 1. If doc’s viewport has had its width or height changed (e.g. as a result of the user resizing the browser window, or changing page zoom, or an iframe element’s dimensions are changed) since the last time these steps were run, fire an event named resize at the Window object associated with doc.
  - 2. If the VisualViewport associated with doc has had its scale, width, or height properties changed since the last time these steps were run, fire an event named resize at the VisualViewport.

### [13.2. Scrolling](https://drafts.csswg.org/cssom-view-1/#scrolling-events)

Status: unreviewed

- [ ] [Whenever a viewport gets scrolled (whether in response to user interaction or b…](https://drafts.csswg.org/cssom-view-1/#document-pending-scroll-events) — Whenever a viewport gets scrolled (whether in response to user interaction or by an API), the user agent must run these steps:
  - 1. Let doc be the viewport’s associated Document.
  - 2. If doc is a snap container, run the steps to update scrollsnapchanging targets for doc with doc’s eventual snap target in the block axis as newBlockTarget and doc’s eventual snap target in the inline axis as newInlineTarget.
  - 3. If (doc, "scroll") is already in doc’s pending scroll events, abort these steps.
  - 4. Append (doc, "scroll") to doc’s pending scroll events.

- [ ] [Whenever an element gets scrolled (whether in response to user interaction or b…](https://drafts.csswg.org/cssom-view-1/#document-pending-scroll-events) — Whenever an element gets scrolled (whether in response to user interaction or by an API), the user agent must run these steps:
  - 1. Let doc be the element’s node document.
  - 2. If the element is a snap container, run the steps to update scrollsnapchanging targets for the element with the element’s eventual snap target in the block axis as newBlockTarget and the element’s eventual snap target in the inline axis as newInlineTarget.
  - 3. If (element, "scroll") is already in doc’s pending scroll events, abort these steps.
  - 4. Append (element, "scroll") to doc’s pending scroll events.

- [ ] [Whenever a visual viewport gets scrolled (whether in response to user interacti…](https://drafts.csswg.org/cssom-view-1/#document-pending-scroll-events) — Whenever a visual viewport gets scrolled (whether in response to user interaction or by an API), the user agent must run these steps:
  - 1. Let vv be the VisualViewport object that was scrolled.
  - 2. Let doc be vv’s associated document.
  - 3. If (vv, "scroll") is already in doc’s pending scroll events, abort these steps.
  - 4. Append (vv, "scroll") to doc’s pending scroll events.

- [ ] [run the scroll steps (Document)](https://drafts.csswg.org/cssom-view-1/#document-run-the-scroll-steps) — When asked to run the scroll steps for a Document doc, run these steps:
  - 1. For each scrolling box box that was scrolled:
    - 1a. If box belongs to a viewport, let doc be the viewport’s associated Document and target be the viewport. If box belongs to a VisualViewport, let doc be the VisualViewport’s associated document and target be the VisualViewport. Otherwise, box belongs to an element and let doc be the element’s node document and target be the element.
    - 1b. If box belongs to a snap container, snapcontainer, run the update scrollsnapchange targets steps for snapcontainer.
    - 1c. If (target, "scrollend") is already in doc’s pending scroll events, abort these steps.
    - 1d. Append (target, "scrollend") to doc’s pending scroll events.
  - 2. For each item (target, type) in doc’s pending scroll events, in the order they were added to the list, run these substeps:
    - 2a. If target is a Document, and type is "scroll" or "scrollend", fire an event named type that bubbles at target.
    - 2b. Otherwise, if type is "scrollsnapchange", then:
      - 2b.i. Let blockTarget and inlineTarget be null initially.
      - 2b.ii. If the scrollsnapchangeTargetBlock associated with target is a pseudo-element, set blockTarget to the owning element of that scrollsnapchangeTargetBlock.
      - 2b.iii. Otherwise, set blockTarget to that scrollsnapchangeTargetBlock.
      - 2b.iv. If the scrollsnapchangeTargetInline associated with target is a pseudo-element, set inlineTarget to the owning element of that scrollsnapchangeTargetInline.
      - 2b.v. Otherwise, Set inlineTarget to that scrollsnapchangeTargetInline.
      - 2b.vi. Fire a SnapEvent, snapevent, named scrollsnapchange at target and let snapevent’s snapTargetBlock and snapTargetInline attributes be blockTarget and inlineTarget respectively.
    - 2c. Otherwise, if type is "scrollsnapchanging", then:
      - 2c.i. Let blockTarget and inlineTarget be null initially.
      - 2c.ii. If the scrollsnapchanging block-axis target associated with target is a pseudo-element, set blockTarget to the owning element of that scrollsnapchanging block-axis target.
      - 2c.iii. Otherwise, set blockTarget to that scrollsnapchanging block-axis target.
      - 2c.iv. If the scrollsnapchanging inline-axis target associated with target is a pseudo-element, set inlineTarget to the owning element of that scrollsnapchanging inline-axis target.
      - 2c.v. Otherwise, set inlineTarget to that scrollsnapchanging inline-axis target.
      - 2c.vi. Fire a SnapEvent, snapevent, named scrollsnapchanging at target and let snapevent’s snapTargetBlock and snapTargetInline attributes be blockTarget and inlineTarget, respectively.
    - 2d. Otherwise, fire an event named type at target.
  - 3. Empty doc’s pending scroll events.

### [13.3. Event summary](https://drafts.csswg.org/cssom-view-1/#event-summary)

Status: unreviewed

## [14. Post-Layout State Snapshotting](https://drafts.csswg.org/cssom-view-1/#post-layout-snapshot)

Status: unreviewed

- [ ] [run snapshot post-layout state steps](https://drafts.csswg.org/cssom-view-1/#run-snapshot-post-layout-state-steps) — When asked to run snapshot post-layout state steps for a Document doc, run these steps:
  - 1. For each CSS feature that needs to snapshot post-layout state, take a snapshot of the relevant state in doc.

## [15. Privacy Considerations](https://drafts.csswg.org/cssom-view-1/#privacy)

Status: unreviewed

## [16. Security Considerations](https://drafts.csswg.org/cssom-view-1/#security)

Status: unreviewed

## [17. Changes](https://drafts.csswg.org/cssom-view-1/#changes)

Status: unreviewed

### [Changes since the 17 March 2016 Working Draft](https://drafts.csswg.org/cssom-view-1/#changes-2016-03-17)

Status: unreviewed

### [Changes since the 17 December 2013 Working Draft](https://drafts.csswg.org/cssom-view-1/#changes-2013-12-17)

Status: unreviewed

### [Changes since the 04 August 2011 Working Draft](https://drafts.csswg.org/cssom-view-1/#changes-2011-08-04)

Status: unreviewed

### [Changes since the 04 August 2009 Working Draft](https://drafts.csswg.org/cssom-view-1/#changes-2009-08-04)

Status: unreviewed

### [Changes since the 22 February 2008 Working Draft](https://drafts.csswg.org/cssom-view-1/#changes-2008-02-22)

Status: unreviewed

## [18. Acknowledgements](https://drafts.csswg.org/cssom-view-1/#acks)

Status: unreviewed

## [Conformance](https://drafts.csswg.org/cssom-view-1/#w3c-conformance)

Status: unreviewed

### [Document conventions](https://drafts.csswg.org/cssom-view-1/#w3c-conventions)

Status: unreviewed

### [Conformance classes](https://drafts.csswg.org/cssom-view-1/#w3c-conformance-classes)

Status: unreviewed

### [Partial implementations](https://drafts.csswg.org/cssom-view-1/#w3c-partial)

Status: unreviewed

#### [Implementations of Unstable and Proprietary Features](https://drafts.csswg.org/cssom-view-1/#w3c-conform-future-proofing)

Status: unreviewed

### [Non-experimental implementations](https://drafts.csswg.org/cssom-view-1/#w3c-testing)

Status: unreviewed

## [Index](https://drafts.csswg.org/cssom-view-1/#index)

Status: unreviewed

### [Terms defined by this specification](https://drafts.csswg.org/cssom-view-1/#index-defined-here)

Status: unreviewed

### [Terms defined by reference](https://drafts.csswg.org/cssom-view-1/#index-defined-elsewhere)

Status: unreviewed

## [References](https://drafts.csswg.org/cssom-view-1/#references)

Status: unreviewed

### [Normative References](https://drafts.csswg.org/cssom-view-1/#normative)

Status: unreviewed

### [Non-Normative References](https://drafts.csswg.org/cssom-view-1/#informative)

Status: unreviewed

## [IDL Index](https://drafts.csswg.org/cssom-view-1/#idl-index)

Status: unreviewed

## [Issues Index](https://drafts.csswg.org/cssom-view-1/#issues-index)

Status: unreviewed
