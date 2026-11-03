# HTML review checklist: 7.2 APIs related to navigation and session history

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 173 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html nav-history-apis` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [7.2 APIs related to navigation and session history](https://html.spec.whatwg.org/#nav-traversal-apis)

Status: reviewed (2026-11-01)
> The sections below.

#### [7.2.1 Security infrastructure for Window, WindowProxy, and Location objects](https://html.spec.whatwg.org/#cross-origin-objects)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.1.1 Integration with IDL](https://html.spec.whatwg.org/#integration-with-idl)

Status: n/a (no script engine: these are JavaScript object internals)

- [x] [When perform a security check is invoked, with a platformObject, identifier, an…](https://html.spec.whatwg.org/#integration-with-idl) — When perform a security check is invoked, with a platformObject, identifier, and type, run these steps:
  - 1. If platformObject is not a Window or Location object, then return.
  - 2. For each e of CrossOriginProperties(platformObject):
    - 2a. If SameValue(e.[[Property]], identifier) is true:
      - 2a.i. If type is "method" and e has neither [[NeedsGetter]] nor [[NeedsSetter]], then return.
      - 2a.ii. Otherwise, if type is "getter" and e.[[NeedsGetter]] is true, then return.
      - 2a.iii. Otherwise, if type is "setter" and e.[[NeedsSetter]] is true, then return.
  - 3. If IsPlatformObjectSameOrigin(platformObject) is false, then throw a "SecurityError" DOMException.

##### [7.2.1.2 Shared internal slot: [[CrossOriginPropertyDescriptorMap]]](https://html.spec.whatwg.org/#shared-internal-slot:-crossoriginpropertydescriptormap)

Status: n/a (no script engine: these are JavaScript object internals)
<!-- definitions: crossoriginpropertydescriptormap -->

##### [7.2.1.3 Shared abstract operations](https://html.spec.whatwg.org/#shared-abstract-operations)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.1 CrossOriginProperties ( O )](https://html.spec.whatwg.org/#crossoriginproperties-%28-o-%29)

Status: n/a (no script engine: these are JavaScript object internals)

- [x] [cross-origin accessible window property name](https://html.spec.whatwg.org/#cross-origin-accessible-window-property-name) — A JavaScript property name P is a cross-origin accessible window property name if it is "window", "self", "location", "close", "closed", "focus", "blur", "frames", "length", "top", "opener", "parent", "postMessage", or an array index property name.

###### [7.2.1.3.2 CrossOriginPropertyFallback ( P )](https://html.spec.whatwg.org/#crossoriginpropertyfallback-%28-p-%29)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.3 IsPlatformObjectSameOrigin ( O )](https://html.spec.whatwg.org/#isplatformobjectsameorigin-%28-o-%29)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.4 CrossOriginGetOwnPropertyHelper ( O, P )](https://html.spec.whatwg.org/#crossorigingetownpropertyhelper-%28-o,-p-%29)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.5 CrossOriginGet ( O, P, Receiver )](https://html.spec.whatwg.org/#crossoriginget-%28-o,-p,-receiver-%29)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.6 CrossOriginSet ( O, P, V, Receiver )](https://html.spec.whatwg.org/#crossoriginset-%28-o,-p,-v,-receiver-%29)

Status: n/a (no script engine: these are JavaScript object internals)

###### [7.2.1.3.7 CrossOriginOwnPropertyKeys ( O )](https://html.spec.whatwg.org/#crossoriginownpropertykeys-%28-o-%29)

Status: n/a (no script engine: these are JavaScript object internals)

#### [7.2.2 The Window object](https://html.spec.whatwg.org/#the-window-object)

Status: reviewed (2026-11-01)
> `LocalDOMWindow` (with `DOMWindow` merged in): `window`, `self` and `frames` are the window itself, as there's no `WindowProxy`; `Document::DefaultView` is the window while the document is its frame's.
<!-- definitions: concept-document-window dom-frames dom-self htmldocument -->

- [x] [browsing context (Window)](https://html.spec.whatwg.org/#window-bc) — A Window's browsing context is its associated Document's browsing context.

- [x] [navigable (Window)](https://html.spec.whatwg.org/#window-navigable) — A Window's navigable is the navigable whose active document is the Window's associated Document's, or null if there is no such navigable.

- [x] [Window.window](https://html.spec.whatwg.org/#dom-window) — The window, frames, and self getter steps are to return this's relevant realm.[[GlobalEnv]].[[GlobalThisValue]].

- [x] [Window.document](https://html.spec.whatwg.org/#dom-document-2) — The document getter steps are to return this's associated Document.

- [x] [Document.defaultView](https://html.spec.whatwg.org/#dom-document-defaultview) — The defaultView getter steps are:
  - 1. If this's browsing context is null, then return null.
  - 2. Return this's browsing context's WindowProxy object.

##### [7.2.2.1 Opening and closing windows](https://html.spec.whatwg.org/#apis-for-creating-and-navigating-browsing-contexts-by-name)

Status: reviewed (2026-11-01)
> Fixed: `window.open()` with `noopener` returns null for an existing browsing context other than `_self`, `_parent` and `_top` (step 18).
> `close()` doesn't check the page is script-closable and `stop()` doesn't stop loading (see `KNOWN-ISSUES.md`); a new window is the host's, and never returned (see `DIFFERENCES.md`).

- [x] [get noopener for window open](https://html.spec.whatwg.org/#get-noopener-for-window-open) — To get noopener for window open, given a Document sourceDocument, an ordered map tokenizedFeatures, and a URL record-or-null url, perform the following steps. They return a boolean.
  - 1. If url is not null and url's blob URL entry is not null:
    - 1a. Let blobOrigin be url's blob URL entry's environment's origin.
    - 1b. Let topLevelOrigin be sourceDocument's relevant settings object's top-level origin.
    - 1c. If blobOrigin is not same site with topLevelOrigin, then return true.
  - 2. Let noopener be false.
  - 3. If tokenizedFeatures["noopener"] exists, then set noopener to the result of parsing tokenizedFeatures["noopener"] as a boolean feature.
  - 4. Return noopener.

- [x] [window open steps](https://html.spec.whatwg.org/#window-open-steps) — The window open steps, given a string url, a string target, and a string features, are as follows:
  - 1. If the event loop's termination nesting level is nonzero, then return null.
  - 2. Let sourceDocument be the entry global object's associated Document.
  - 3. Let urlRecord be null.
  - 4. If url is not the empty string:
    - 4a. Set urlRecord to the result of encoding-parsing a URL given url, relative to sourceDocument.
    - 4b. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 5. If target is the empty string, then set target to "_blank".
  - 6. Let tokenizedFeatures be the result of tokenizing features.
  - 7. Let noreferrer be false.
  - 8. If tokenizedFeatures["noreferrer"] exists, then set noreferrer to the result of parsing tokenizedFeatures["noreferrer"] as a boolean feature.
  - 9. Let noopener be the result of getting noopener for window open with sourceDocument, tokenizedFeatures, and urlRecord.
  - 10. Remove tokenizedFeatures["noopener"] and tokenizedFeatures["noreferrer"].
  - 11. Let referrerPolicy be the empty string.
  - 12. If noreferrer is true, then set noopener to true and set referrerPolicy to "no-referrer".
  - 13. Let targetNavigable and windowType be the result of applying the rules for choosing a navigable given target, sourceDocument's node navigable, and noopener.
  - 14. If targetNavigable is null, then return null.
  - 15. If windowType is either "new and unrestricted" or "new with no opener":
    - 15a. Set targetNavigable's active browsing context's is popup to the result of checking if a popup window is requested, given tokenizedFeatures.
    - 15b. Set up browsing context features for targetNavigable's active browsing context given tokenizedFeatures. [CSSOMVIEW]
    - 15c. If urlRecord is null, then set urlRecord to a URL record representing about:blank.
    - 15d. If urlRecord matches about:blank, then perform the URL and history update steps given targetNavigable's active document and urlRecord.
    - 15e. Otherwise, navigate targetNavigable to urlRecord using sourceDocument, with referrerPolicy set to referrerPolicy and exceptionsEnabled set to true.
  - 16. Otherwise:
    - 16a. If urlRecord is not null, then navigate targetNavigable to urlRecord using sourceDocument, with referrerPolicy set to referrerPolicy and exceptionsEnabled set to true.
    - 16b. If noopener is false, then set targetNavigable's active browsing context's opener browsing context to sourceDocument's browsing context.
  - 17. If windowType is "new with no opener", then return null.
  - 18. If noopener is true and target is not an ASCII case-insensitive match for "_self", "_parent", or "_top", then return null.
  - 19. Return targetNavigable's active WindowProxy.

- [x] [Window.open(url, target, features)](https://html.spec.whatwg.org/#dom-open) — The open(url, target, features) method steps are to run the window open steps with url, target, and features.

- [x] [tokenize the features argument](https://html.spec.whatwg.org/#concept-window-open-features-tokenize) — To tokenize the features argument:
  - 1. Let tokenizedFeatures be a new ordered map.
  - 2. Let position point at the first code point of features.
  - 3. While position is not past the end of features:
    - 3a. Let name be the empty string.
    - 3b. Let value be the empty string.
    - 3c. Collect a sequence of code points that are feature separators from features given position. This skips past leading separators before the name.
    - 3d. Collect a sequence of code points that are not feature separators from features given position. Set name to the collected characters, converted to ASCII lowercase.
    - 3e. Set name to the result of normalizing the feature name name.
    - 3f. While position is not past the end of features and the code point at position in features is not U+003D (=):
      - 3f.i. If the code point at position in features is U+002C (,), or if it is not a feature separator, then break.
      - 3f.ii. Advance position by 1.
    - 3g. If the code point at position in features is a feature separator:
      - 3g.i. While position is not past the end of features and the code point at position in features is a feature separator:
        - 3g.i.A. If the code point at position in features is U+002C (,), then break.
        - 3g.i.B. Advance position by 1.
      - 3g.ii. Collect a sequence of code points that are not feature separators code points from features given position. Set value to the collected code points, converted to ASCII lowercase.
    - 3h. If name is not the empty string, then set tokenizedFeatures[name] to value.
  - 4. Return tokenizedFeatures.

- [x] [check if a window feature is set](https://html.spec.whatwg.org/#window-feature-is-set) — To check if a window feature is set, given tokenizedFeatures, featureName, and defaultValue:
  - 1. If tokenizedFeatures[featureName] exists, then return the result of parsing tokenizedFeatures[featureName] as a boolean feature.
  - 2. Return defaultValue.

- [x] [check if a popup window is requested](https://html.spec.whatwg.org/#popup-window-is-requested) — To check if a popup window is requested, given tokenizedFeatures:
  - 1. If tokenizedFeatures is empty, then return false.
  - 2. If tokenizedFeatures["popup"] exists, then return the result of parsing tokenizedFeatures["popup"] as a boolean feature.
  - 3. Let location be the result of checking if a window feature is set, given tokenizedFeatures, "location", and false.
  - 4. Let toolbar be the result of checking if a window feature is set, given tokenizedFeatures, "toolbar", and false.
  - 5. If location and toolbar are both false, then return true.
  - 6. Let menubar be the result of checking if a window feature is set, given tokenizedFeatures, "menubar", and false.
  - 7. If menubar is false, then return true.
  - 8. Let resizable be the result of checking if a window feature is set, given tokenizedFeatures, "resizable", and true.
  - 9. If resizable is false, then return true.
  - 10. Let scrollbars be the result of checking if a window feature is set, given tokenizedFeatures, "scrollbars", and false.
  - 11. If scrollbars is false, then return true.
  - 12. Let status be the result of checking if a window feature is set, given tokenizedFeatures, "status", and false.
  - 13. If status is false, then return true.
  - 14. Return false.

- [x] [feature separator](https://html.spec.whatwg.org/#feature-separator) — A code point is a feature separator if it is ASCII whitespace, U+003D (=), or U+002C (,).

- [x] [normalize a feature name](https://html.spec.whatwg.org/#normalizing-the-feature-name) — For legacy reasons, there are some aliases of some feature names. To normalize a feature name name, switch on name:
  - • "screenx": Return "left".
  - • "screeny": Return "top".
  - • "innerwidth": Return "width".
  - • "innerheight": Return "height".
  - • Anything else: Return name.

- [x] [parse a boolean feature](https://html.spec.whatwg.org/#concept-window-open-features-parse-boolean) — To parse a boolean feature given a string value:
  - 1. If value is the empty string, then return true.
  - 2. If value is "yes", then return true.
  - 3. If value is "true", then return true.
  - 4. Let parsed be the result of parsing value as an integer.
  - 5. If parsed is an error, then set it to 0.
  - 6. Return false if parsed is 0, and true otherwise.

- [x] [Window.name](https://html.spec.whatwg.org/#dom-name) — The name getter steps are:
  - 1. If this's navigable is null, then return the empty string.
  - 2. Return this's navigable's target name.

- [x] [The name setter steps](https://html.spec.whatwg.org/#dom-name) — The name setter steps are:
  - 1. If this's navigable is null, then return.
  - 2. Set this's navigable's active session history entry's document state's navigable target name to the given value.

- [x] [Window.close()](https://html.spec.whatwg.org/#dom-window-close) — The close() method steps are:
  - 1. Let thisTraversable be this's navigable.
  - 2. If thisTraversable is not a top-level traversable, then return.
  - 3. If thisTraversable's is closing is true, then return.
  - 4. Let browsingContext be thisTraversable's active browsing context.
  - 5. Let sourceSnapshotParams be the result of snapshotting source snapshot params given thisTraversable's active document.
  - 6. If all the following are true: thisTraversable is script-closable;; the incumbent global object's browsing context is familiar with browsingContext; and; the incumbent global object's navigable is allowed by sandboxing to navigate thisTraversable, given sourceSnapshotParams, then:
    - 6a. Set thisTraversable's is closing to true.
    - 6b. Queue a task on the DOM manipulation task source to definitely close thisTraversable.

- [x] [script-closable](https://html.spec.whatwg.org/#script-closable) — A navigable is script-closable if it is a top-level traversable, and any of the following are true:

- [x] [Window.closed](https://html.spec.whatwg.org/#dom-window-closed) — The closed getter steps are to return true if this's navigable is null or its is closing is true; otherwise false.

- [x] [Window.stop()](https://html.spec.whatwg.org/#dom-window-stop) — The stop() method steps are:
  - 1. If this's navigable is null, then return.
  - 2. Stop loading this's navigable.

##### [7.2.2.2 Indexed access on the Window object](https://html.spec.whatwg.org/#accessing-other-browsing-contexts)

Status: reviewed (2026-11-01)
> `Length` is 0, as there are no child navigables.

- [x] [Window.length](https://html.spec.whatwg.org/#dom-length) — The length getter steps are to return this's associated Document's document-tree child navigables's size.

##### [7.2.2.3 Named access on the Window object](https://html.spec.whatwg.org/#named-access-on-the-window-object)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::NamedItem` and `IsNamedObject`; there are no child navigables to name.

- [x] [document-tree child navigable target name property set](https://html.spec.whatwg.org/#document-tree-child-navigable-target-name-property-set) — The document-tree child navigable target name property set of a Window object window is the return value of running these steps:
  - 1. Let children be the document-tree child navigables of window's associated Document.
  - 2. Let firstNamedChildren be an empty ordered set.
  - 3. For each navigable of children:
    - 3a. Let name be navigable's target name.
    - 3b. If name is the empty string, then continue.
    - 3c. If firstNamedChildren contains a navigable whose target name is name, then continue.
    - 3d. Append navigable to firstNamedChildren.
  - 4. Let names be an empty ordered set.
  - 5. For each navigable of firstNamedChildren:
    - 5a. Let name be navigable's target name.
    - 5b. If navigable's active document's origin is same origin with window's relevant settings object's origin, then append name to names.
  - 6. Return names.

- [x] [To determine the value of a named property name in a Window object window, the…](https://html.spec.whatwg.org/#document-tree-child-navigable-target-name-property-set) — To determine the value of a named property name in a Window object window, the user agent must return the value obtained using the following steps:
  - 1. Let objects be the list of named objects of window with the name name.
  - 2. If objects contains a navigable:
    - 2a. Let container be the first navigable container in window's associated Document's descendants whose content navigable is in objects.
    - 2b. Return container's content navigable's active WindowProxy.
  - 3. Otherwise, if objects has only one element, return that element.
  - 4. Otherwise, return an HTMLCollection rooted at window's associated Document, whose filter matches only named objects of window with the name name. (By definition, these will all be elements.)

- [x] [Named objects](https://html.spec.whatwg.org/#dom-window-nameditem-filter) — Named objects of Window object window with the name name, for the purposes of the above algorithm, consist of the following:

##### [7.2.2.4 Accessing related windows](https://html.spec.whatwg.org/#navigating-nested-browsing-contexts-in-the-dom)

Status: reviewed (2026-11-01)
> A frame is the only one in its tree: `top` and `parent` are the window, `opener` and `frameElement` are null. The opener setter is n/a without script.

- [x] [Window.top](https://html.spec.whatwg.org/#dom-top) — The top getter steps are:
  - 1. If this's navigable is null, then return null.
  - 2. Return this's navigable's top-level traversable's active WindowProxy.

- [x] [Window.opener](https://html.spec.whatwg.org/#dom-opener) — The opener getter steps are:
  - 1. Let current be this's browsing context.
  - 2. If current is null, then return null.
  - 3. If current's opener browsing context is null, then return null.
  - 4. Return current's opener browsing context's WindowProxy object.

- [x] [The opener setter steps](https://html.spec.whatwg.org/#dom-opener) — The opener setter steps are:
  - 1. If the given value is null and this's browsing context is non-null, then set this's browsing context's opener browsing context to null.
  - 2. If the given value is non-null, then perform ? DefinePropertyOrThrow(this, "opener", { [[Value]]: the given value, [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true }).

- [x] [Window.parent](https://html.spec.whatwg.org/#dom-parent) — The parent getter steps are:
  - 1. Let navigable be this's navigable.
  - 2. If navigable is null, then return null.
  - 3. If navigable's parent is not null, then set navigable to navigable's parent.
  - 4. Return navigable's active WindowProxy.

- [x] [Window.frameElement](https://html.spec.whatwg.org/#dom-frameelement) — The frameElement getter steps are:
  - 1. Let current be this's node navigable.
  - 2. If current is null, then return null.
  - 3. Let container be current's container.
  - 4. If container is null, then return null.
  - 5. If container's node document's origin is not same origin-domain with the current settings object's origin, then return null.
  - 6. Return container.

##### [7.2.2.5 Historical browser interface element APIs](https://html.spec.whatwg.org/#browser-interface-elements)

Status: reviewed (2026-11-01)
> `BarProp::Visible` is true, as there's no popup; `status` keeps its value.
<!-- definitions: the-location-bar-barprop-object the-menu-bar-barprop-object the-personal-bar-barprop-object the-scrollbar-barprop-object the-status-bar-barprop-object the-toolbar-barprop-object -->

- [x] [BarProp.visible](https://html.spec.whatwg.org/#dom-barprop-visible) — The visible getter steps are:
  - 1. Let browsingContext be this's relevant global object's browsing context.
  - 2. If browsingContext is null, then return true.
  - 3. Return the negation of browsingContext's top-level browsing context's is popup.

- [x] [Window.locationbar](https://html.spec.whatwg.org/#dom-window-locationbar) — The locationbar attribute must return the location bar BarProp object.

- [x] [Window.menubar](https://html.spec.whatwg.org/#dom-window-menubar) — The menubar attribute must return the menu bar BarProp object.

- [x] [Window.personalbar](https://html.spec.whatwg.org/#dom-window-personalbar) — The personalbar attribute must return the personal bar BarProp object.

- [x] [Window.scrollbars](https://html.spec.whatwg.org/#dom-window-scrollbars) — The scrollbars attribute must return the scrollbar BarProp object.

- [x] [Window.statusbar](https://html.spec.whatwg.org/#dom-window-statusbar) — The statusbar attribute must return the status bar BarProp object.

- [x] [Window.toolbar](https://html.spec.whatwg.org/#dom-window-toolbar) — The toolbar attribute must return the toolbar BarProp object.

- [x] [Window.status](https://html.spec.whatwg.org/#dom-window-status) — For historical reasons, the status attribute on the Window object must, on getting, return the last string it was set to, and on setting, must set itself to the new value. When the Window object is created, the attribute must be set to the empty string. It does not do anything else.

##### [7.2.2.6 Script settings for Window objects](https://html.spec.whatwg.org/#script-settings-for-window-objects)

Status: n/a (no script engine: there are no environment settings objects)

- [x] [set up a window environment settings object](https://html.spec.whatwg.org/#set-up-a-window-environment-settings-object) — To set up a window environment settings object, given a URL creationURL, a JavaScript execution context execution context, null or an environment reservedEnvironment, a URL topLevelCreationURL, and an origin topLevelOrigin, run these steps:
  - 1. Let realm be the value of execution context's Realm component.
  - 2. Let window be realm's global object.
  - 3. Let settings object be a new environment settings object whose algorithms are defined as follows: The realm execution context Return execution context. The module map Return the module map of window's associated Document. The API base URL Return the current base URL of window's associated Document. The origin Return the origin of window's associated Document. The has cross-site ancestor The polic…
    - 3a. If window's navigable's parent is null, then return false.
    - 3b. Let parentDocument be window's navigable's parent's active document.
    - 3c. If parentDocument's relevant settings object's has cross-site ancestor is true, then return true.
    - 3d. If parentDocument's origin is not same site with window's associated Document's origin, then return true.
    - 3e. Return false.
  - 4. If reservedEnvironment is non-null:
    - 4a. Set settings object's id to reservedEnvironment's id, target browsing context to reservedEnvironment's target browsing context, and active service worker to reservedEnvironment's active service worker.
    - 4b. Set reservedEnvironment's id to the empty string.
  - 5. Otherwise, set settings object's id to a new unique opaque string, settings object's target browsing context to null, and settings object's active service worker to null.
  - 6. Set settings object's creation URL to creationURL, settings object's top-level creation URL to topLevelCreationURL, and settings object's top-level origin to topLevelOrigin.
  - 7. Set realm's [[HostDefined]] field to settings object.

#### [7.2.3 The WindowProxy exotic object](https://html.spec.whatwg.org/#the-windowproxy-exotic-object)

Status: n/a (no script engine: these are JavaScript object internals)
<!-- definitions: windowproxy concept-windowproxy-window -->

##### [7.2.3.1 [[GetPrototypeOf]] ( )](https://html.spec.whatwg.org/#windowproxy-getprototypeof)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.2 [[SetPrototypeOf]] ( V )](https://html.spec.whatwg.org/#windowproxy-setprototypeof)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.3 [[IsExtensible]] ( )](https://html.spec.whatwg.org/#windowproxy-isextensible)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.4 [[PreventExtensions]] ( )](https://html.spec.whatwg.org/#windowproxy-preventextensions)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.5 [[GetOwnProperty]] ( P )](https://html.spec.whatwg.org/#windowproxy-getownproperty)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.6 [[DefineOwnProperty]] ( P, Desc )](https://html.spec.whatwg.org/#windowproxy-defineownproperty)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.7 [[Get]] ( P, Receiver )](https://html.spec.whatwg.org/#windowproxy-get)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.8 [[Set]] ( P, V, Receiver )](https://html.spec.whatwg.org/#windowproxy-set)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.9 [[Delete]] ( P )](https://html.spec.whatwg.org/#windowproxy-delete)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.3.10 [[OwnPropertyKeys]] ( )](https://html.spec.whatwg.org/#windowproxy-ownpropertykeys)

Status: n/a (no script engine: these are JavaScript object internals)

#### [7.2.4 The Location interface](https://html.spec.whatwg.org/#the-location-interface)

Status: reviewed (2026-11-01)
> Fixed: setting `protocol` throws `SyntaxError` for a value that isn't a scheme and doesn't navigate to a scheme that isn't HTTP(S); setting `hash` doesn't navigate to the fragment the URL has already, and the empty string is an empty fragment.
> There's one origin, so the same origin-domain checks are left out.
<!-- definitions: concept-location-empty-domstringlist defaultproperties -->

- [x] [To create a Location object, run these steps](https://html.spec.whatwg.org/#the-location-interface) — To create a Location object, run these steps:
  - 1. Let location be a new Location platform object.
  - 2. Let valueOf be location's relevant realm.[[Intrinsics]].[[%Object.prototype.valueOf%]].
  - 3. Perform ! location.[[DefineOwnProperty]]("valueOf", { [[Value]]: valueOf, [[Writable]]: false, [[Enumerable]]: false, [[Configurable]]: false }).
  - 4. Perform ! location.[[DefineOwnProperty]](%Symbol.toPrimitive%, { [[Value]]: undefined, [[Writable]]: false, [[Enumerable]]: false, [[Configurable]]: false }).
  - 5. Set the value of the [[DefaultProperties]] internal slot of location to location.[[OwnPropertyKeys]]().
  - 6. Return location.

- [x] [Document.location](https://html.spec.whatwg.org/#dom-document-location) — The Document object's location getter steps are to return this's relevant global object's Location object, if this is fully active, and null otherwise.

- [x] [Window.location](https://html.spec.whatwg.org/#dom-location) — The Window object's location getter steps are to return this's Location object.

- [x] [relevant Document](https://html.spec.whatwg.org/#relevant-document) — A Location object has an associated relevant Document, which is its relevant global object's browsing context's active document, if this Location object's relevant global object's browsing context is non-null, and null otherwise.

- [x] [url](https://html.spec.whatwg.org/#concept-location-url) — A Location object has an associated url, which is this Location object's relevant Document's URL, if this Location object's relevant Document is non-null, and about:blank otherwise.

- [x] [Location-object navigate](https://html.spec.whatwg.org/#location-object-navigate) — To Location-object navigate a Location object location to a URL url, optionally given a NavigationHistoryBehavior historyHandling (default "auto"):
  - 1. Let navigable be location's relevant global object's navigable.
  - 2. Let sourceDocument be the incumbent global object's associated Document.
  - 3. If location's relevant Document is not yet completely loaded, and the incumbent global object does not have transient activation, then set historyHandling to "replace".
  - 4. Navigate navigable to url using sourceDocument, with exceptionsEnabled set to true and historyHandling set to historyHandling.

- [x] [Location.href](https://html.spec.whatwg.org/#dom-location-href) — The href getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. Return this's url, serialized.

- [x] [The href setter steps](https://html.spec.whatwg.org/#dom-location-href) — The href setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. Let url be the result of encoding-parsing a URL given the given value, relative to the entry settings object.
  - 3. If url is failure, then throw a "SyntaxError" DOMException.
  - 4. Location-object navigate this to url.

- [x] [Location.origin](https://html.spec.whatwg.org/#dom-location-origin) — The origin getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. Return the serialization of this's url's origin.

- [x] [Location.protocol](https://html.spec.whatwg.org/#dom-location-protocol) — The protocol getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. Return this's url's scheme, followed by ":".

- [x] [The protocol setter steps](https://html.spec.whatwg.org/#dom-location-protocol) — The protocol setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. Let possibleFailure be the result of basic URL parsing the given value, followed by ":", with copyURL as url and scheme start state as state override.
  - 5. If possibleFailure is failure, then throw a "SyntaxError" DOMException.
  - 6. If copyURL's scheme is not an HTTP(S) scheme, then terminate these steps.
  - 7. Location-object navigate this to copyURL.

- [x] [Location.host](https://html.spec.whatwg.org/#dom-location-host) — The host getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. Let url be this's url.
  - 3. If url's host is null, return the empty string.
  - 4. If url's port is null, return url's host, serialized.
  - 5. Return url's host, serialized, followed by ":" and url's port, serialized.

- [x] [The host setter steps](https://html.spec.whatwg.org/#dom-location-host) — The host setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. If copyURL has an opaque path, then return.
  - 5. Basic URL parse the given value, with copyURL as url and host state as state override.
  - 6. Location-object navigate this to copyURL.

- [x] [Location.hostname](https://html.spec.whatwg.org/#dom-location-hostname) — The hostname getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. If this's url's host is null, return the empty string.
  - 3. Return this's url's host, serialized.

- [x] [The hostname setter steps](https://html.spec.whatwg.org/#dom-location-hostname) — The hostname setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. If copyURL has an opaque path, then return.
  - 5. Basic URL parse the given value, with copyURL as url and hostname state as state override.
  - 6. Location-object navigate this to copyURL.

- [x] [Location.port](https://html.spec.whatwg.org/#dom-location-port) — The port getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. If this's url's port is null, return the empty string.
  - 3. Return this's url's port, serialized.

- [x] [The port setter steps](https://html.spec.whatwg.org/#dom-location-port) — The port setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. If copyURL cannot have a username/password/port, then return.
  - 5. If the given value is the empty string, then set copyURL's port to null.
  - 6. Otherwise, basic URL parse the given value, with copyURL as url and port state as state override.
  - 7. Location-object navigate this to copyURL.

- [x] [Location.pathname](https://html.spec.whatwg.org/#dom-location-pathname) — The pathname getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. Return the result of URL path serializing this Location object's url.

- [x] [The pathname setter steps](https://html.spec.whatwg.org/#dom-location-pathname) — The pathname setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. If copyURL has an opaque path, then return.
  - 5. Set copyURL's path to the empty list.
  - 6. Basic URL parse the given value, with copyURL as url and path start state as state override.
  - 7. Location-object navigate this to copyURL.

- [x] [Location.search](https://html.spec.whatwg.org/#dom-location-search) — The search getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. If this's url's query is either null or the empty string, return the empty string.
  - 3. Return "?", followed by this's url's query.

- [x] [The search setter steps](https://html.spec.whatwg.org/#dom-location-search) — The search setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. If the given value is the empty string, set copyURL's query to null.
  - 5. Otherwise, run these substeps:
    - 5a. Let input be the given value with a single leading "?" removed, if any.
    - 5b. Set copyURL's query to the empty string.
    - 5c. Basic URL parse input, with null, the relevant Document's document's character encoding, copyURL as url, and query state as state override.
  - 6. Location-object navigate this to copyURL.

- [x] [Location.hash](https://html.spec.whatwg.org/#dom-location-hash) — The hash getter steps are:
  - 1. If this's relevant Document is non-null and its origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 2. If this's url's fragment is either null or the empty string, return the empty string.
  - 3. Return "#", followed by this's url's fragment.

- [x] [The hash setter steps](https://html.spec.whatwg.org/#dom-location-hash) — The hash setter steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let copyURL be a copy of this's url.
  - 4. Let thisURLFragment be copyURL's fragment if it is non-null; otherwise the empty string.
  - 5. Let input be the given value with a single leading "#" removed, if any.
  - 6. Set copyURL's fragment to the empty string.
  - 7. Basic URL parse input, with copyURL as url and fragment state as state override.
  - 8. If copyURL's fragment is thisURLFragment, then return.
  - 9. Location-object navigate this to copyURL.

- [x] [Location.assign(url)](https://html.spec.whatwg.org/#dom-location-assign) — The assign(url) method steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Let urlRecord be the result of encoding-parsing a URL given url, relative to the entry settings object.
  - 4. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 5. Location-object navigate this to urlRecord.

- [x] [Location.replace(url)](https://html.spec.whatwg.org/#dom-location-replace) — The replace(url) method steps are:
  - 1. If this's relevant Document is null, then return.
  - 2. Let urlRecord be the result of encoding-parsing a URL given url, relative to the entry settings object.
  - 3. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 4. Location-object navigate this to urlRecord given "replace".

- [x] [Location.reload()](https://html.spec.whatwg.org/#dom-location-reload) — The reload() method steps are:
  - 1. Let document be this's relevant Document.
  - 2. If document is null, then return.
  - 3. If document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 4. Reload document's node navigable.

- [x] [Location.ancestorOrigins](https://html.spec.whatwg.org/#dom-location-ancestororigins) — The ancestorOrigins getter steps are:
  - 1. If this's relevant Document is null, then return this's empty DOMStringList.
  - 2. If this's relevant Document's origin is not same origin-domain with the entry settings object's origin, then throw a "SecurityError" DOMException.
  - 3. Assert: this's relevant Document's ancestor origins list is not null.
  - 4. Otherwise, return this's relevant Document's ancestor origins list.

##### [7.2.4.1 [[GetPrototypeOf]] ( )](https://html.spec.whatwg.org/#location-getprototypeof)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.2 [[SetPrototypeOf]] ( V )](https://html.spec.whatwg.org/#location-setprototypeof)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.3 [[IsExtensible]] ( )](https://html.spec.whatwg.org/#location-isextensible)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.4 [[PreventExtensions]] ( )](https://html.spec.whatwg.org/#location-preventextensions)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.5 [[GetOwnProperty]] ( P )](https://html.spec.whatwg.org/#location-getownproperty)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.6 [[DefineOwnProperty]] ( P, Desc )](https://html.spec.whatwg.org/#location-defineownproperty)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.7 [[Get]] ( P, Receiver )](https://html.spec.whatwg.org/#location-get)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.8 [[Set]] ( P, V, Receiver )](https://html.spec.whatwg.org/#location-set)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.9 [[Delete]] ( P )](https://html.spec.whatwg.org/#location-delete)

Status: n/a (no script engine: these are JavaScript object internals)

##### [7.2.4.10 [[OwnPropertyKeys]] ( )](https://html.spec.whatwg.org/#location-ownpropertykeys)

Status: n/a (no script engine: these are JavaScript object internals)

#### [7.2.5 The History interface](https://html.spec.whatwg.org/#the-history-interface)

Status: reviewed (2026-11-01)
> Fixed: a document whose URL isn't HTTP(S) or `file:` can only have its fragment rewritten (`CanHaveURLRewritten`, shared with the Navigation API, which had compared origins and so allowed another username or password).
> The state limits are upstream's exceptions (see `DIFFERENCES.md`).
<!-- definitions: doc-history concept-history-state concept-history-length concept-history-index -->

- [x] [Window.history](https://html.spec.whatwg.org/#dom-history) — The history getter steps are to return this's associated Document's history object.

- [x] [History.length](https://html.spec.whatwg.org/#dom-history-length) — The length getter steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then throw a "SecurityError" DOMException.
  - 2. Return this's length.

- [x] [History.scrollRestoration](https://html.spec.whatwg.org/#dom-history-scroll-restoration) — The scrollRestoration getter steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then throw a "SecurityError" DOMException.
  - 2. Return this's relevant global object's navigable's active session history entry's scroll restoration mode.

- [x] [The scrollRestoration setter steps](https://html.spec.whatwg.org/#dom-history-scroll-restoration) — The scrollRestoration setter steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then throw a "SecurityError" DOMException.
  - 2. Set this's relevant global object's navigable's active session history entry's scroll restoration mode to the given value.

- [x] [History.state](https://html.spec.whatwg.org/#dom-history-state) — The state getter steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then throw a "SecurityError" DOMException.
  - 2. Return this's state.

- [x] [History.go(delta)](https://html.spec.whatwg.org/#dom-history-go) — The go(delta) method steps are to delta traverse this given delta.

- [x] [History.back()](https://html.spec.whatwg.org/#dom-history-back) — The back() method steps are to delta traverse this given −1.

- [x] [History.forward()](https://html.spec.whatwg.org/#dom-history-forward) — The forward() method steps are to delta traverse this given +1.

- [x] [delta traverse](https://html.spec.whatwg.org/#delta-traverse) — To delta traverse a History object history given an integer delta:
  - 1. Let document be history's relevant global object's associated Document.
  - 2. If document is not fully active, then throw a "SecurityError" DOMException.
  - 3. If history's relevant global object's navigable's allowed to perform a navigation or history update returns blocked, then return.
  - 4. If delta is 0, then reload document's node navigable, and return.
  - 5. Traverse the history by a delta given document's node navigable's traversable navigable, delta, and with sourceDocument set to document.

- [x] [History.pushState(data, unused, url)](https://html.spec.whatwg.org/#dom-history-pushstate) — The pushState(data, unused, url) method steps are to run the shared history push/replace state steps given this, data, url, and "push".

- [x] [History.replaceState(data, unused, url)](https://html.spec.whatwg.org/#dom-history-replacestate) — The replaceState(data, unused, url) method steps are to run the shared history push/replace state steps given this, data, url, and "replace".

- [x] [shared history push/replace state steps](https://html.spec.whatwg.org/#shared-history-push/replace-state-steps) — The shared history push/replace state steps, given a History history, a value data, a scalar value string-or-null url, and a history handling behavior historyHandling, are:
  - 1. Let document be history's relevant global object's associated Document.
  - 2. If document is not fully active, then throw a "SecurityError" DOMException.
  - 3. Let serializedData be StructuredSerializeForStorage(data). Rethrow any exceptions.
  - 4. Let newURL be document's URL.
  - 5. If url is not null or the empty string:
    - 5a. Set newURL to the result of encoding-parsing a URL given url, relative to the relevant settings object of history.
    - 5b. If newURL is failure, then throw a "SecurityError" DOMException.
    - 5c. If document cannot have its URL rewritten to newURL, then throw a "SecurityError" DOMException.
  - 6. If history's relevant global object's navigable's allowed to perform a navigation or history update returns blocked, then return.
  - 7. Let navigation be history's relevant global object's navigation API.
  - 8. Let continue be the result of firing a push/replace/reload navigate event at navigation with navigationType set to historyHandling, isSameDocument set to true, destinationURL set to newURL, and classicHistoryAPIState set to serializedData.
  - 9. If continue is false, then return.
  - 10. Run the URL and history update steps given document and newURL, with serializedData set to serializedData and historyHandling set to historyHandling.

- [x] [can have its URL rewritten](https://html.spec.whatwg.org/#can-have-its-url-rewritten) — A Document document can have its URL rewritten to a URL targetURL if the following algorithm returns true:
  - 1. Let documentURL be document's URL.
  - 2. If targetURL and documentURL differ in their scheme, username, password, host, or port components, then return false.
  - 3. If targetURL's scheme is an HTTP(S) scheme, then return true.
  - 4. If targetURL's scheme is "file":
    - 4a. If targetURL and documentURL differ in their path component, then return false.
    - 4b. Return true.
  - 5. If targetURL and documentURL differ in their path component or query components, then return false.
  - 6. Return true.

#### [7.2.6 The navigation API](https://html.spec.whatwg.org/#navigation-api)

Status: reviewed (2026-11-01)
> The sections below.

##### [7.2.6.1 Introduction](https://html.spec.whatwg.org/#navigation-api-intro)

Status: reviewed (2026-11-01)
> No requirements.

##### [7.2.6.2 The Navigation interface](https://html.spec.whatwg.org/#navigation-interface)

Status: reviewed (2026-11-01)
> `Navigation`, the window's `GetNavigation()`.

- [x] [navigation API](https://html.spec.whatwg.org/#window-navigation-api) — Each Window has an associated navigation API, which is a Navigation object. Upon creation of the Window object, its navigation API must be set to a new Navigation object created in the Window object's relevant realm.

- [x] [Window.navigation](https://html.spec.whatwg.org/#dom-navigation) — The navigation getter steps are to return this's navigation API.

##### [7.2.6.3 Core infrastructure](https://html.spec.whatwg.org/#navigation-api-core)

Status: reviewed (2026-11-01)
> Opaque origins don't disable entries and events, as there are no origins (FIXME).
<!-- definitions: navigation-entry-list navigation-current-entry-index dom-navigationtype-push dom-navigationtype-replace dom-navigationtype-reload dom-navigationtype-traverse -->

- [x] [current entry](https://html.spec.whatwg.org/#navigation-current-entry) — The current entry of a Navigation navigation is the result of running the following steps:
  - 1. If navigation has entries and events disabled, then return null.
  - 2. Assert: navigation's current entry index is not −1.
  - 3. Return navigation's entry list[navigation's current entry index].

- [x] [has entries and events disabled](https://html.spec.whatwg.org/#has-entries-and-events-disabled) — A Navigation navigation has entries and events disabled if the following steps return true:
  - 1. Let document be navigation's relevant global object's associated Document.
  - 2. If document is not fully active, then return true.
  - 3. If document's is initial about:blank is true, then return true.
  - 4. If document's origin is opaque, then return true.
  - 5. Return false.

- [x] [get the navigation API entry index](https://html.spec.whatwg.org/#getting-the-navigation-api-entry-index) — To get the navigation API entry index of a session history entry she within a Navigation navigation:
  - 1. Let index be 0.
  - 2. For each nhe of navigation's entry list:
    - 2a. If nhe's session history entry is equal to she, then return index.
    - 2b. Increment index by 1.
  - 3. Return −1.

##### [7.2.6.4 Initializing and updating the entry list](https://html.spec.whatwg.org/#navigation-api-entry-updates)

Status: reviewed (2026-11-01)
> `InitializeForNewWindow`, `UpdateForReactivation` and `UpdateForNavigation` follow upstream's structure, keyed by history items.

- [x] [initialize the navigation API entries for a new document](https://html.spec.whatwg.org/#initialize-the-navigation-api-entries-for-a-new-document) — To initialize the navigation API entries for a new document given a Navigation navigation, a list of session history entries newSHEs, and a session history entry initialSHE:
  - 1. Assert: navigation's entry list is empty.
  - 2. Assert: navigation's current entry index is −1.
  - 3. If navigation has entries and events disabled, then return.
  - 4. For each newSHE of newSHEs:
    - 4a. Let newNHE be a new NavigationHistoryEntry created in the relevant realm of navigation.
    - 4b. Set newNHE's session history entry to newSHE.
    - 4c. Append newNHE to navigation's entry list.
  - 5. Set navigation's current entry index to the result of getting the navigation API entry index of initialSHE within navigation.

- [x] [update the navigation API entries for reactivation](https://html.spec.whatwg.org/#update-the-navigation-api-entries-for-reactivation) — To update the navigation API entries for reactivation given a Navigation navigation, a list of session history entries newSHEs, and a session history entry reactivatedSHE:
  - 1. If navigation has entries and events disabled, then return.
  - 2. Let newNHEs be a new empty list.
  - 3. Let oldNHEs be a clone of navigation's entry list.
  - 4. For each newSHE of newSHEs:
    - 4a. Let newNHE be null.
    - 4b. If oldNHEs contains a NavigationHistoryEntry matchingOldNHE whose session history entry is newSHE:
      - 4b.i. Set newNHE to matchingOldNHE.
      - 4b.ii. Remove matchingOldNHE from oldNHEs.
    - 4c. Otherwise:
      - 4c.i. Set newNHE to a new NavigationHistoryEntry created in the relevant realm of navigation.
      - 4c.ii. Set newNHE's session history entry to newSHE.
    - 4d. Append newNHE to newNHEs.
  - 5. Set navigation's entry list to newNHEs.
  - 6. Set navigation's current entry index to the result of getting the navigation API entry index of reactivatedSHE within navigation.
  - 7. Queue a global task on the navigation and traversal task source given navigation's relevant global object to run the following steps:
    - 7a. For each disposedNHE of oldNHEs:
      - 7a.i. Fire an event named dispose at disposedNHE.

- [x] [update the navigation API entries for a same-document navigation](https://html.spec.whatwg.org/#update-the-navigation-api-entries-for-a-same-document-navigation) — To update the navigation API entries for a same-document navigation given a Navigation navigation, a session history entry destinationSHE, and a NavigationType navigationType:
  - 1. If navigation has entries and events disabled, then return.
  - 2. Let oldCurrentNHE be the current entry of navigation.
  - 3. Let disposedNHEs be a new empty list.
  - 4. If navigationType is "traverse":
    - 4a. Set navigation's current entry index to the result of getting the navigation API entry index of destinationSHE within navigation.
    - 4b. Assert: navigation's current entry index is not −1.
  - 5. Otherwise, if navigationType is "push":
    - 5a. Set navigation's current entry index to navigation's current entry index + 1.
    - 5b. Let i be navigation's current entry index.
    - 5c. While i < navigation's entry list's size:
      - 5c.i. Append navigation's entry list[i] to disposedNHEs.
      - 5c.ii. Set i to i + 1.
    - 5d. Remove all items in disposedNHEs from navigation's entry list.
  - 6. Otherwise, if navigationType is "replace":
    - 6a. Append oldCurrentNHE to disposedNHEs.
  - 7. If navigationType is "push" or "replace":
    - 7a. Let newNHE be a new NavigationHistoryEntry created in the relevant realm of navigation.
    - 7b. Set newNHE's session history entry to destinationSHE.
    - 7c. Set navigation's entry list[navigation's current entry index] to newNHE.
  - 8. If navigation's ongoing API method tracker is non-null, then notify about the committed-to entry given navigation's ongoing API method tracker and the current entry of navigation.
  - 9. Let navigateEvent be navigation's ongoing navigate event.
  - 10. Let apiMethodTracker be navigation's ongoing API method tracker.
  - 11. Prepare to run script given navigation's relevant settings object.
  - 12. Fire an event named currententrychange at navigation using NavigationCurrentEntryChangeEvent, with its navigationType attribute initialized to navigationType and its from initialized to oldCurrentNHE.
  - 13. For each disposedNHE of disposedNHEs:
    - 13a. Fire an event named dispose at disposedNHE.
  - 14. Run the navigate event intercept commit handler steps given navigation, navigateEvent, and apiMethodTracker.
  - 15. Clean up after running script given navigation's relevant settings object.

##### [7.2.6.5 The NavigationHistoryEntry interface](https://html.spec.whatwg.org/#the-navigationhistoryentry-interface)

Status: reviewed (2026-11-01)
> Fixed: `url` is the empty string, not null, for an entry whose document isn't fully active.
> Another document's URL isn't hidden for its referrer policy (see `KNOWN-ISSUES.md`).
<!-- definitions: nhe-she -->

- [x] [key](https://html.spec.whatwg.org/#concept-navigationhistoryentry-key) — The key of a NavigationHistoryEntry nhe is given by the return value of the following algorithm:
  - 1. If nhe's relevant global object's associated Document is not fully active, then return the empty string.
  - 2. Return nhe's session history entry's navigation API key.

- [x] [ID](https://html.spec.whatwg.org/#concept-navigationhistoryentry-id) — The ID of a NavigationHistoryEntry nhe is given by the return value of the following algorithm:
  - 1. If nhe's relevant global object's associated Document is not fully active, then return the empty string.
  - 2. Return nhe's session history entry's navigation API ID.

- [x] [index](https://html.spec.whatwg.org/#concept-navigationhistoryentry-index) — The index of a NavigationHistoryEntry nhe is given by the return value of the following algorithm:
  - 1. If nhe's relevant global object's associated Document is not fully active, then return −1.
  - 2. Return the result of getting the navigation API entry index of this's session history entry within this's relevant global object's navigation API.

- [x] [NavigationHistoryEntry.url](https://html.spec.whatwg.org/#dom-navigationhistoryentry-url) — The url getter steps are:
  - 1. Let document be this's relevant global object's associated Document.
  - 2. If document is not fully active, then return the empty string.
  - 3. Let she be this's session history entry.
  - 4. If she's document does not equal document, and she's document state's request referrer policy is "no-referrer" or "origin", then return null.
  - 5. Return she's URL, serialized.

- [x] [NavigationHistoryEntry.key](https://html.spec.whatwg.org/#dom-navigationhistoryentry-key) — The key getter steps are to return this's key.

- [x] [NavigationHistoryEntry.id](https://html.spec.whatwg.org/#dom-navigationhistoryentry-id) — The id getter steps are to return this's ID.

- [x] [NavigationHistoryEntry.index](https://html.spec.whatwg.org/#dom-navigationhistoryentry-index) — The index getter steps are to return this's index.

- [x] [NavigationHistoryEntry.sameDocument](https://html.spec.whatwg.org/#dom-navigationhistoryentry-samedocument) — The sameDocument getter steps are:
  - 1. Let document be this's relevant global object's associated Document.
  - 2. If document is not fully active, then return false.
  - 3. Return true if this's session history entry's document equals document, and false otherwise.

- [x] [NavigationHistoryEntry.getState()](https://html.spec.whatwg.org/#dom-navigationhistoryentry-getstate) — The getState() method steps are:
  - 1. If this's relevant global object's associated Document is not fully active, then return undefined.
  - 2. Return StructuredDeserialize(this's session history entry's navigation API state). Rethrow any exceptions.

##### [7.2.6.6 The history entry list](https://html.spec.whatwg.org/#the-history-entry-list)

Status: reviewed (2026-11-01)

- [x] [Navigation.entries()](https://html.spec.whatwg.org/#dom-navigation-entries) — The entries() method steps are:
  - 1. If this has entries and events disabled, then return the empty list.
  - 2. Return this's entry list.

- [x] [Navigation.currentEntry](https://html.spec.whatwg.org/#dom-navigation-currententry) — The currentEntry getter steps are to return the current entry of this.

- [x] [Navigation.updateCurrentEntry(options)](https://html.spec.whatwg.org/#dom-navigation-updatecurrententry) — The updateCurrentEntry(options) method steps are:
  - 1. Let current be the current entry of this.
  - 2. If current is null, then throw an "InvalidStateError" DOMException.
  - 3. Let serializedState be StructuredSerializeForStorage(options["state"]), rethrowing any exceptions.
  - 4. Set current's session history entry's navigation API state to serializedState.
  - 5. Fire an event named currententrychange at this using NavigationCurrentEntryChangeEvent, with its navigationType attribute initialized to null and its from initialized to current.

- [x] [Navigation.canGoBack](https://html.spec.whatwg.org/#dom-navigation-cangoback) — The canGoBack getter steps are:
  - 1. If this has entries and events disabled, then return false.
  - 2. Assert: this's current entry index is not −1.
  - 3. If this's current entry index is 0, then return false.
  - 4. Return true.

- [x] [Navigation.canGoForward](https://html.spec.whatwg.org/#dom-navigation-cangoforward) — The canGoForward getter steps are:
  - 1. If this has entries and events disabled, then return false.
  - 2. Assert: this's current entry index is not −1.
  - 3. If this's current entry index is equal to this's entry list's size − 1, then return false.
  - 4. Return true.

##### [7.2.6.7 Initiating navigations](https://html.spec.whatwg.org/#navigation-api-initiating-navigations)

Status: reviewed (2026-11-01)
> Fixed: `navigate()` refuses a push only from the initial about:blank document, not from any about:blank.

- [x] [Navigation.navigate(url, options)](https://html.spec.whatwg.org/#dom-navigation-navigate) — The navigate(url, options) method steps are:
  - 1. Let urlRecord be the result of parsing a URL given url, relative to this's relevant settings object.
  - 2. If urlRecord is failure, then return an early error result for a "SyntaxError" DOMException.
  - 3. If urlRecord's scheme is "javascript", then return an early error result for a "NotSupportedError" DOMException.
  - 4. Let document be this's relevant global object's associated Document.
  - 5. If options["history"] is "push", and the navigation must be a replace given urlRecord and document, then return an early error result for a "NotSupportedError" DOMException.
  - 6. Let state be options["state"], if it exists; otherwise, undefined.
  - 7. Let serializedState be StructuredSerializeForStorage(state). If this throws an exception, then return an early error result for that exception.
  - 8. If document is not fully active, then return an early error result for an "InvalidStateError" DOMException.
  - 9. If document's unload counter is greater than 0, then return an early error result for an "InvalidStateError" DOMException.
  - 10. Let info be options["info"], if it exists; otherwise, undefined.
  - 11. Let apiMethodTracker be the result of setting up a navigate/reload API method tracker for this given info and serializedState.
  - 12. Navigate document's node navigable to urlRecord using document, with historyHandling set to options["history"], navigationAPIState set to serializedState, and apiMethodTracker set to apiMethodTracker.
  - 13. Return a navigation API method tracker-derived result for apiMethodTracker.

- [x] [Navigation.reload(options)](https://html.spec.whatwg.org/#dom-navigation-reload) — The reload(options) method steps are:
  - 1. Let document be this's relevant global object's associated Document.
  - 2. Let serializedState be StructuredSerializeForStorage(undefined).
  - 3. If options["state"] exists, then set serializedState to StructuredSerializeForStorage(options["state"]). If this throws an exception, then return an early error result for that exception.
  - 4. Otherwise:
    - 4a. Let current be the current entry of this.
    - 4b. If current is not null, then set serializedState to current's session history entry's navigation API state.
  - 5. If document is not fully active, then return an early error result for an "InvalidStateError" DOMException.
  - 6. If document's unload counter is greater than 0, then return an early error result for an "InvalidStateError" DOMException.
  - 7. Let info be options["info"], if it exists; otherwise, undefined.
  - 8. Let apiMethodTracker be the result of setting up a navigate/reload API method tracker for this given info and serializedState.
  - 9. Reload document's node navigable with navigationAPIState set to serializedState and apiMethodTracker set to apiMethodTracker.
  - 10. Return a navigation API method tracker-derived result for apiMethodTracker.

- [x] [Navigation.traverseTo(key, options)](https://html.spec.whatwg.org/#dom-navigation-traverseto) — The traverseTo(key, options) method steps are:
  - 1. If this's current entry index is −1, then return an early error result for an "InvalidStateError" DOMException.
  - 2. If this's entry list does not contain a NavigationHistoryEntry whose session history entry's navigation API key equals key, then return an early error result for an "InvalidStateError" DOMException.
  - 3. Return the result of performing a navigation API traversal given this, key, and options.

- [x] [Navigation.back(options)](https://html.spec.whatwg.org/#dom-navigation-back) — The back(options) method steps are:
  - 1. If this's current entry index is −1 or 0, then return an early error result for an "InvalidStateError" DOMException.
  - 2. Let key be this's entry list[this's current entry index − 1]'s session history entry's navigation API key.
  - 3. Return the result of performing a navigation API traversal given this, key, and options.

- [x] [Navigation.forward(options)](https://html.spec.whatwg.org/#dom-navigation-forward) — The forward(options) method steps are:
  - 1. If this's current entry index is −1 or is equal to this's entry list's size − 1, then return an early error result for an "InvalidStateError" DOMException.
  - 2. Let key be this's entry list[this's current entry index + 1]'s session history entry's navigation API key.
  - 3. Return the result of performing a navigation API traversal given this, key, and options.

- [x] [perform a navigation API traversal](https://html.spec.whatwg.org/#performing-a-navigation-api-traversal) — To perform a navigation API traversal given a Navigation navigation, a string key, and a NavigationOptions options:
  - 1. Let document be navigation's relevant global object's associated Document.
  - 2. If document is not fully active, then return an early error result for an "InvalidStateError" DOMException.
  - 3. If document's unload counter is greater than 0, then return an early error result for an "InvalidStateError" DOMException.
  - 4. Let current be the current entry of navigation.
  - 5. If key equals current's session history entry's navigation API key, then return «[ "committed" → a promise resolved with current, "finished" → a promise resolved with current ]».
  - 6. If navigation's upcoming traverse API method trackers[key] exists, then return a navigation API method tracker-derived result for navigation's upcoming traverse API method trackers[key].
  - 7. Let info be options["info"], if it exists; otherwise, undefined.
  - 8. Let apiMethodTracker be the result of adding an upcoming traverse API method tracker for navigation given key and info.
  - 9. Let navigable be document's node navigable.
  - 10. Let traversable be navigable's traversable navigable.
  - 11. Let sourceSnapshotParams be the result of snapshotting source snapshot params given document.
  - 12. Append the following session history traversal steps to traversable:
    - 12a. Let navigableSHEs be the result of getting session history entries given navigable.
    - 12b. Let targetSHE be the session history entry in navigableSHEs whose navigation API key is key. If no such entry exists:
      - 12b.i. Queue a global task on the navigation and traversal task source given navigation's relevant global object to reject the finished promise for apiMethodTracker with an "InvalidStateError" DOMException.
      - 12b.ii. Abort these steps.
    - 12c. If targetSHE is navigable's active session history entry:
      - 12c.i. Queue a global task on the navigation and traversal task source given navigation's relevant global object to reject the finished promise for apiMethodTracker with an "InvalidStateError" DOMException.
      - 12c.ii. Abort these steps.
    - 12d. Let result be the result of applying the traverse history step given by targetSHE's step to traversable, given sourceSnapshotParams, navigable, and "none".
    - 12e. If result is "canceled-by-beforeunload", then queue a global task on the navigation and traversal task source given navigation's relevant global object to reject the finished promise for apiMethodTracker with a new "AbortError" DOMException created in navigation's relevant realm.
    - 12f. If result is "initiator-disallowed", then queue a global task on the navigation and traversal task source given navigation's relevant global object to reject the finished promise for apiMethodTracker with a new "SecurityError" DOMException created in navigation's relevant realm.
  - 13. Return a navigation API method tracker-derived result for apiMethodTracker.

- [x] [early error result](https://html.spec.whatwg.org/#navigation-api-early-error-result) — An early error result for an exception e is a NavigationResult dictionary instance given by «[ "committed" → a promise rejected with e, "finished" → a promise rejected with e ]».

- [x] [navigation API method tracker-derived result](https://html.spec.whatwg.org/#navigation-api-method-tracker-derived-result) — To compute the navigation API method tracker-derived result for a navigation API method tracker-or-null apiMethodTracker:
  - 1. If apiMethodTracker is pending, then return an early error result for an "AbortError" DOMException.
  - 2. Return a NavigationResult dictionary instance given by «[ "committed" → apiMethodTracker's committed promise, "finished" → apiMethodTracker's finished promise ]».

##### [7.2.6.8 Ongoing navigation tracking](https://html.spec.whatwg.org/#ongoing-navigation-tracking)

Status: reviewed (2026-11-01)
> Fixed: `NavigationTransition` has `to` and `committed`.
> The trackers follow upstream's `NavigationAPIMethodTracker`; there are no child navigables to be destroyed.
<!-- definitions: ongoing-navigate-event focus-changed-during-ongoing-navigation suppress-normal-scroll-restoration-during-ongoing-navigation ongoing-api-method-tracker upcoming-traverse-api-method-trackers navigation-api-method-tracker navigation-api-method-tracker-navigation navigation-api-method-tracker-key navigation-api-method-tracker-info navigation-api-method-tracker-state navigation-api-method-tracker-committed-to-entry navigation-api-method-tracker-committed navigation-api-method-tracker-finished navigation-api-method-tracker-pending concept-navigation-transition concept-navigationtransition-navigationtype concept-navigationtransition-from concept-navigationtransition-destination concept-navigationtransition-committed concept-navigationtransition-finished -->

- [x] [set up a navigate/reload API method tracker](https://html.spec.whatwg.org/#set-up-a-navigate/reload-api-method-tracker) — To set up a navigate/reload API method tracker given a Navigation navigation, a JavaScript value info, and a serialized state-or-null serializedState:
  - 1. Let committedPromise and finishedPromise be new promises created in navigation's relevant realm.
  - 2. Mark as handled finishedPromise.
  - 3. Return a new navigation API method tracker with: navigation objectnavigationkeynullinfoinfoserialized stateserializedStatecommitted-to entrynullcommitted promisecommittedPromisefinished promisefinishedPromisependingfalse if navigation has entries and events disabled; otherwise true

- [x] [add an upcoming traverse API method tracker](https://html.spec.whatwg.org/#add-an-upcoming-traverse-api-method-tracker) — To add an upcoming traverse API method tracker given a Navigation navigation, a string destinationKey, and a JavaScript value info:
  - 1. Let committedPromise and finishedPromise be new promises created in navigation's relevant realm.
  - 2. Mark as handled finishedPromise.
  - 3. Let apiMethodTracker be a new navigation API method tracker with: navigation objectnavigationkeydestinationKeyinfoinfoserialized statenullcommitted-to entrynullcommitted promisecommittedPromisefinished promisefinishedPromisependingfalse
  - 4. Set navigation's upcoming traverse API method trackers[destinationKey] to apiMethodTracker.
  - 5. Return apiMethodTracker.

- [x] [clean up](https://html.spec.whatwg.org/#navigation-api-method-tracker-clean-up) — To clean up a navigation API method tracker apiMethodTracker:
  - 1. Let navigation be apiMethodTracker's navigation object.
  - 2. If navigation's ongoing API method tracker is apiMethodTracker, then set navigation's ongoing API method tracker to null.
  - 3. Otherwise:
    - 3a. Let key be apiMethodTracker's key.
    - 3b. Assert: key is not null.
    - 3c. Assert: navigation's upcoming traverse API method trackers[key] exists.
    - 3d. Remove navigation's upcoming traverse API method trackers[key].

- [x] [notify about the committed-to entry](https://html.spec.whatwg.org/#notify-about-the-committed-to-entry) — To notify about the committed-to entry given a navigation API method tracker apiMethodTracker and a NavigationHistoryEntry nhe:
  - 1. Set apiMethodTracker's committed-to entry to nhe.
  - 2. If apiMethodTracker's serialized state is not null, then set nhe's session history entry's navigation API state to apiMethodTracker's serialized state.
  - 3. Resolve apiMethodTracker's committed promise with nhe.

- [x] [resolve the finished promise](https://html.spec.whatwg.org/#resolve-the-finished-promise) — To resolve the finished promise for a navigation API method tracker apiMethodTracker:
  - 1. Assert: apiMethodTracker's committed-to entry is not null.
  - 2. Resolve apiMethodTracker's finished promise with its committed-to entry.
  - 3. Clean up apiMethodTracker.

- [x] [reject the finished promise](https://html.spec.whatwg.org/#reject-the-finished-promise) — To reject the finished promise for a navigation API method tracker apiMethodTracker with a JavaScript value exception:
  - 1. Reject apiMethodTracker's committed promise with exception.
  - 2. Reject apiMethodTracker's finished promise with exception.
  - 3. Clean up apiMethodTracker.

- [x] [abort the ongoing navigation](https://html.spec.whatwg.org/#abort-the-ongoing-navigation) — To abort the ongoing navigation given a Navigation navigation and an optional DOMException error:
  - 1. Let event be navigation's ongoing navigate event.
  - 2. Assert: event is not null.
  - 3. Set navigation's focus changed during ongoing navigation to false.
  - 4. Set navigation's suppress normal scroll restoration during ongoing navigation to false.
  - 5. If error was not given, then let error be a new "AbortError" DOMException created in navigation's relevant realm.
  - 6. If event's dispatch flag is set, then set event's canceled flag to true.
  - 7. Abort event given error.

- [x] [abort a NavigateEvent](https://html.spec.whatwg.org/#abort-a-navigateevent) — To abort a NavigateEvent event given reason:
  - 1. Let navigation be event's relevant global object's navigation API.
  - 2. Set navigation's ongoing navigate event to null.
  - 3. Signal abort on event's abort controller given reason.
  - 4. Let errorInfo be the result of extracting error information from reason.
  - 5. If navigation's ongoing API method tracker is non-null, then reject the finished promise for apiMethodTracker with reason.
  - 6. Fire an event named navigateerror at navigation using ErrorEvent, with additional attributes initialized according to errorInfo.
  - 7. If navigation's transition is null, then return.
  - 8. Reject navigation's transition's committed promise with reason.
  - 9. Reject navigation's transition's finished promise with reason.
  - 10. Set navigation's transition to null.

- [x] [inform the navigation API about aborting navigation](https://html.spec.whatwg.org/#inform-the-navigation-api-about-aborting-navigation) — To inform the navigation API about aborting navigation in a navigable navigable:
  - 1. If this algorithm is running on navigable's active window's relevant agent's event loop, then continue on to the following steps. Otherwise, queue a global task on the navigation and traversal task source given navigable's active window to run the following steps.
  - 2. Let navigation be navigable's active window's navigation API.
  - 3. While navigation's ongoing navigate event is not null:
    - 3a. Abort the ongoing navigation given navigation.

- [x] [inform the navigation API about child navigable destruction](https://html.spec.whatwg.org/#inform-the-navigation-api-about-child-navigable-destruction) — To inform the navigation API about child navigable destruction given a navigable navigable:
  - 1. Inform the navigation API about aborting navigation in navigable.
  - 2. Let navigation be navigable's active window's navigation API.
  - 3. Let traversalAPIMethodTrackers be a clone of navigation's upcoming traverse API method trackers.
  - 4. For each apiMethodTracker of traversalAPIMethodTrackers: reject the finished promise for apiMethodTracker with a new "AbortError" DOMException created in navigation's relevant realm.

- [x] [Navigation.transition](https://html.spec.whatwg.org/#dom-navigation-transition) — The transition getter steps are to return this's transition.

- [x] [NavigationTransition.navigationType](https://html.spec.whatwg.org/#dom-navigationtransition-navigationtype) — The navigationType getter steps are to return this's navigation type.

- [x] [NavigationTransition.from](https://html.spec.whatwg.org/#dom-navigationtransition-from) — The from getter steps are to return this's from entry.

- [x] [NavigationTransition.to](https://html.spec.whatwg.org/#dom-navigationtransition-to) — The to getter steps are to return this's destination.

- [x] [NavigationTransition.committed](https://html.spec.whatwg.org/#dom-navigationtransition-committed) — The committed getter steps are to return this's committed promise.

- [x] [NavigationTransition.finished](https://html.spec.whatwg.org/#dom-navigationtransition-finished) — The finished getter steps are to return this's finished promise.

##### [7.2.6.9 The NavigationActivation interface](https://html.spec.whatwg.org/#navigation-activation-interface)

Status: reviewed (2026-11-01)
<!-- definitions: navigation-activation nav-activation-old-entry nav-activation-new-entry nav-activation-navigation-type -->

- [x] [Navigation.activation](https://html.spec.whatwg.org/#dom-navigation-activation) — The activation getter steps are to return this's activation.

- [x] [NavigationActivation.from](https://html.spec.whatwg.org/#dom-navigationactivation-from) — The from getter steps are to return this's old entry.

- [x] [NavigationActivation.entry](https://html.spec.whatwg.org/#dom-navigationactivation-entry) — The entry getter steps are to return this's new entry.

- [x] [NavigationActivation.navigationType](https://html.spec.whatwg.org/#dom-navigationactivation-navigationtype) — The navigationType getter steps are to return this's navigation type.

##### [7.2.6.10 The navigate event](https://html.spec.whatwg.org/#the-navigate-event)

Status: reviewed (2026-11-01)
> The sections below.

###### [7.2.6.10.1 The NavigateEvent interface](https://html.spec.whatwg.org/#the-navigateevent-interface)

Status: reviewed (2026-11-01)
> There's no `precommitHandler` (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-navigateevent-interception-state concept-navigateevent-navigation-precommit-handler-list concept-navigateevent-navigation-handler-list concept-navigateevent-focusreset concept-navigateevent-scroll concept-navigateevent-abort-controller concept-navigateevent-classic-history-api-state dom-navigateevent-destination dom-navigateevent-canintercept dom-navigateevent-userinitiated dom-navigateevent-hashchange dom-navigateevent-signal dom-navigateevent-formdata dom-navigateevent-downloadrequest dom-navigateevent-info dom-navigateevent-hasuavisualtransition dom-navigateevent-sourceelement -->

- [x] [NavigateEvent.navigationType](https://html.spec.whatwg.org/#dom-navigateevent-navigationtype) — The navigationType, destination, canIntercept, userInitiated, hashChange, signal, formData, downloadRequest, info, hasUAVisualTransition, and sourceElement attributes must return the values they are initialized to.

- [x] [NavigateEvent.intercept(options)](https://html.spec.whatwg.org/#dom-navigateevent-intercept) — The intercept(options) method steps are:
  - 1. Perform shared checks given this.
  - 2. If this's canIntercept attribute was initialized to false, then throw a "SecurityError" DOMException.
  - 3. If this's dispatch flag is unset, then throw an "InvalidStateError" DOMException.
  - 4. If options["precommitHandler"] exists:
    - 4a. If this's cancelable attribute is initialized to false, then throw an "InvalidStateError" DOMException.
    - 4b. Append options["precommitHandler"] to this's navigation precommit handler list.
  - 5. Assert: this's interception state is either "none" or "intercepted".
  - 6. Set this's interception state to "intercepted".
  - 7. If options["handler"] exists, then append it to this's navigation handler list.
  - 8. If options["focusReset"] exists:
    - 8a. If this's focus reset behavior is not null, and it is not equal to options["focusReset"], then the user agent may report a warning to the console indicating that the focusReset option for a previous call to intercept() was overridden by this new value, and the previous value will be ignored.
    - 8b. Set this's focus reset behavior to options["focusReset"].
  - 9. If options["scroll"] exists:
    - 9a. If this's scroll behavior is not null, and it is not equal to options["scroll"], then the user agent may report a warning to the console indicating that the scroll option for a previous call to intercept() was overridden by this new value, and the previous value will be ignored.
    - 9b. Set this's scroll behavior to options["scroll"].

- [x] [NavigateEvent.scroll()](https://html.spec.whatwg.org/#dom-navigateevent-scroll) — The scroll() method steps are:
  - 1. Perform shared checks given this.
  - 2. If this's interception state is not "committed", then throw an "InvalidStateError" DOMException.
  - 3. Process scroll behavior given this.

- [x] [perform shared checks](https://html.spec.whatwg.org/#navigateevent-perform-shared-checks) — To perform shared checks for a NavigateEvent event:
  - 1. If event's relevant global object's associated Document is not fully active, then throw an "InvalidStateError" DOMException.
  - 2. If event's isTrusted attribute was initialized to false, then throw a "SecurityError" DOMException.
  - 3. If event's canceled flag is set, then throw an "InvalidStateError" DOMException.

###### [7.2.6.10.2 The NavigationPrecommitController interface](https://html.spec.whatwg.org/#the-navigationprecommitcontroller-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-navigationprecommitcontroller-event -->

- [x] [NavigationPrecommitController.redirect(url, options)](https://html.spec.whatwg.org/#dom-navigationprecommitcontroller-redirect) — The redirect(url, options) method steps are:
  - 1. Assert: this's event's interception state is not "none".
  - 2. Perform shared checks given this's event.
  - 3. If this's event's interception state is not "intercepted", then throw an "InvalidStateError" DOMException.
  - 4. If this's event's navigationType is neither "push" nor "replace", then throw an "InvalidStateError" DOMException.
  - 5. Let document be this's relevant global object's associated Document.
  - 6. Let destinationURL be the result of parsing url given document.
  - 7. If destinationURL is failure, then throw a "SyntaxError" DOMException.
  - 8. If document cannot have its URL rewritten to destinationURL, then throw a "SecurityError" DOMException.
  - 9. If options["history"] is "push" or "replace", then set this's event's navigationType to options["history"].
  - 10. If options["state"] exists:
    - 10a. Let serializedState be the result of calling StructuredSerializeForStorage(options["state"]). This may throw an exception.
    - 10b. Set this's event's destination's state to serializedState.
    - 10c. If this's event's target's ongoing API method tracker is non-null, then set this's event's target's ongoing API method tracker's serialized state to serializedState.
  - 11. Set this's event's destination's URL to destinationURL.
  - 12. If options["info"] exists, then set this's event's info to options["info"].

- [x] [NavigationPrecommitController.addHandler(handler,)](https://html.spec.whatwg.org/#dom-navigationprecommitcontroller-addhandler) — The addHandler(handler,) method steps are:
  - 1. Assert: this's event's interception state is not "none".
  - 2. Perform shared checks given this's event.
  - 3. If this's event's interception state is not "intercepted", then throw an "InvalidStateError" DOMException.
  - 4. Append handler to this's event's navigation handler list.

###### [7.2.6.10.3 The NavigationDestination interface](https://html.spec.whatwg.org/#the-navigationdestination-interface)

Status: reviewed (2026-11-01)
<!-- definitions: concept-navigationdestination-url concept-navigationdestination-entry concept-navigationdestination-state concept-navigationdestination-samedocument -->

- [x] [NavigationDestination.url](https://html.spec.whatwg.org/#dom-navigationdestination-url) — The url getter steps are to return this's URL, serialized.

- [x] [NavigationDestination.key](https://html.spec.whatwg.org/#dom-navigationdestination-key) — The key getter steps are:
  - 1. If this's entry is null, then return the empty string.
  - 2. Return this's entry's key.

- [x] [NavigationDestination.id](https://html.spec.whatwg.org/#dom-navigationdestination-id) — The id getter steps are:
  - 1. If this's entry is null, then return the empty string.
  - 2. Return this's entry's ID.

- [x] [NavigationDestination.index](https://html.spec.whatwg.org/#dom-navigationdestination-index) — The index getter steps are:
  - 1. If this's entry is null, then return −1.
  - 2. Return this's entry's index.

- [x] [NavigationDestination.sameDocument](https://html.spec.whatwg.org/#dom-navigationdestination-samedocument) — The sameDocument getter steps are to return this's is same document.

- [x] [NavigationDestination.getState()](https://html.spec.whatwg.org/#dom-navigationdestination-getstate) — The getState() method steps are to return StructuredDeserialize(this's state).

###### [7.2.6.10.4 Firing the event](https://html.spec.whatwg.org/#navigate-event-firing)

Status: reviewed (2026-11-01)
> Fixed: `hashChange` is only true for a same-document navigation.
> `InnerDispatchNavigateEvent` follows upstream's structure, committing at once as there are no precommit handlers; `userInitiated` is transient activation (see `DIFFERENCES.md`). There are no download request navigate events, as there are no downloads.
<!-- definitions: fire-navigate-traverse-destinationshe fire-navigate-traverse-userinvolvement fire-navigate-prr-navigationtype fire-navigate-prr-destinationurl fire-navigate-prr-issamedocument fire-navigate-prr-userinvolvement fire-navigate-prr-sourceelement fire-navigate-prr-formdataentrylist fire-navigate-prr-navigationapistate fire-navigate-prr-classichistoryapistate fire-navigate-prr-api-method-tracker fire-navigate-download-destinationurl fire-navigate-download-userinvolvement fire-navigate-download-sourceelement fire-navigate-download-filename -->

- [x] [fire a traverse navigate event](https://html.spec.whatwg.org/#fire-a-traverse-navigate-event) — To fire a traverse navigate event at a Navigation navigation given a session history entry destinationSHE and an optional user navigation involvement userInvolvement (default "none"):
  - 1. Let event be the result of creating an event given NavigateEvent, in navigation's relevant realm.
  - 2. Set event's classic history API state to null.
  - 3. Let destination be a new NavigationDestination created in navigation's relevant realm.
  - 4. Set destination's URL to destinationSHE's URL.
  - 5. Let destinationNHE be the NavigationHistoryEntry in navigation's entry list whose session history entry is destinationSHE, or null if no such NavigationHistoryEntry exists.
  - 6. If destinationNHE is non-null:
    - 6a. Set destination's entry to destinationNHE.
    - 6b. Set destination's state to destinationSHE's navigation API state.
  - 7. Otherwise,
    - 7a. Set destination's entry to null.
    - 7b. Set destination's state to StructuredSerializeForStorage(null).
  - 8. Set destination's is same document to true if destinationSHE's document is equal to navigation's relevant global object's associated Document; otherwise false.
  - 9. Return the result of performing the inner navigate event firing algorithm given navigation, "traverse", event, destination, userInvolvement, null, null, and null.

- [x] [fire a push/replace/reload navigate event](https://html.spec.whatwg.org/#fire-a-push/replace/reload-navigate-event) — To fire a push/replace/reload navigate event at a Navigation navigation given a NavigationType navigationType, a URL destinationURL, a boolean isSameDocument, an optional user navigation involvement userInvolvement (default "none"), an optional Element-or-null sourceElement (default null), an optio…
  - 1. Let document be navigation's relevant global object's associated Document.
  - 2. Inform the navigation API about aborting navigation in document's node navigable.
  - 3. If navigation has entries and events disabled, and apiMethodTracker is not null:
    - 3a. Set apiMethodTracker's pending to false.
    - 3b. Set apiMethodTracker to null.
  - 4. If document is not fully active, then return false.
  - 5. Let event be the result of creating an event given NavigateEvent, in navigation's relevant realm.
  - 6. Set event's classic history API state to classicHistoryAPIState.
  - 7. Let destination be a new NavigationDestination created in navigation's relevant realm.
  - 8. Set destination's URL to destinationURL.
  - 9. Set destination's entry to null.
  - 10. Set destination's state to navigationAPIState.
  - 11. Set destination's is same document to isSameDocument.
  - 12. Return the result of performing the inner navigate event firing algorithm given navigation, navigationType, event, destination, userInvolvement, sourceElement, formDataEntryList, null, and apiMethodTracker.

- [x] [fire a download request navigate event](https://html.spec.whatwg.org/#fire-a-download-request-navigate-event) — To fire a download request navigate event at a Navigation navigation given a URL destinationURL, a user navigation involvement userInvolvement, an Element-or-null sourceElement, and a string filename:
  - 1. Let event be the result of creating an event given NavigateEvent, in navigation's relevant realm.
  - 2. Set event's classic history API state to null.
  - 3. Let destination be a new NavigationDestination created in navigation's relevant realm.
  - 4. Set destination's URL to destinationURL.
  - 5. Set destination's entry to null.
  - 6. Set destination's state to StructuredSerializeForStorage(null).
  - 7. Set destination's is same document to false.
  - 8. Return the result of performing the inner navigate event firing algorithm given navigation, "push", event, destination, userInvolvement, sourceElement, null, and filename.

- [x] [inner navigate event firing algorithm](https://html.spec.whatwg.org/#inner-navigate-event-firing-algorithm) — The inner navigate event firing algorithm consists of the following steps, given a Navigation navigation, a NavigationType navigationType, a NavigateEvent event, a NavigationDestination destination, a user navigation involvement userInvolvement, an Element-or-null sourceElement, an entry list-or-nu…
  - 1. If navigation has entries and events disabled:
    - 1a. Assert: navigation's ongoing API method tracker is null.
    - 1b. Assert: navigation's upcoming traverse API method trackers is empty.
    - 1c. Assert: apiMethodTracker is null.
    - 1d. Return true.
  - 2. Assert: navigation's ongoing API method tracker is null.
  - 3. If destination's entry is non-null:
    - 3a. Assert: apiMethodTracker is null.
    - 3b. Let destinationKey be destination's entry's key.
    - 3c. Assert: destinationKey is not the empty string.
    - 3d. If navigation's upcoming traverse API method trackers[destinationKey] exists:
      - 3d.i. Set apiMethodTracker to navigation's upcoming traverse API method trackers[destinationKey].
      - 3d.ii. Remove navigation's upcoming traverse API method trackers[destinationKey].
  - 4. If apiMethodTracker is not null:
    - 4a. Set navigation's ongoing API method tracker to apiMethodTracker.
    - 4b. Set apiMethodTracker's pending to false.
  - 5. Let navigable be navigation's relevant global object's navigable.
  - 6. Let document be navigation's relevant global object's associated Document.
  - 7. If document can have its URL rewritten to destination's URL, and either destination's is same document is true or navigationType is not "traverse", then initialize event's canIntercept to true. Otherwise, initialize it to false.
  - 8. Let traverseCanBeCanceled be true if all of the following are true: navigable is a top-level traversable;; destination's is same document is true; and; either userInvolvement is not "browser UI", or navigation's relevant global object has history-action activation. Otherwise, let it be false.
  - 9. If either: navigationType is not "traverse"; or; traverseCanBeCanceled is true, then initialize event's cancelable to true. Otherwise, initialize it to false.
  - 10. Initialize event's type to "navigate".
  - 11. Initialize event's navigationType to navigationType.
  - 12. Initialize event's destination to destination.
  - 13. Initialize event's downloadRequest to downloadRequestFilename.
  - 14. If apiMethodTracker is not null, then initialize event's info to apiMethodTracker's info. Otherwise, initialize it to undefined.
  - 15. Initialize event's hasUAVisualTransition to true if a visual transition, to display a cached rendered state of the document's latest entry, was done by the user agent. Otherwise, initialize it to false.
  - 16. Initialize event's sourceElement to sourceElement.
  - 17. Set event's abort controller to a new AbortController created in navigation's relevant realm.
  - 18. Initialize event's signal to event's abort controller's signal.
  - 19. Let currentURL be document's URL.
  - 20. If all of the following are true: event's classic history API state is null;; destination's is same document is true;; destination's URL equals currentURL with exclude fragments set to true; and; destination's URL's fragment is not identical to currentURL's fragment, then initialize event's hashChange to true. Otherwise, initialize it to false.
  - 21. If userInvolvement is not "none", then initialize event's userInitiated to true. Otherwise, initialize it to false.
  - 22. If formDataEntryList is not null, then initialize event's formData to a new FormData created in navigation's relevant realm, associated to formDataEntryList. Otherwise, initialize it to null.
  - 23. Assert: navigation's ongoing navigate event is null.
  - 24. Set navigation's ongoing navigate event to event.
  - 25. Set navigation's focus changed during ongoing navigation to false.
  - 26. Set navigation's suppress normal scroll restoration during ongoing navigation to false.
  - 27. Let dispatchResult be the result of dispatching event at navigation.
  - 28. If dispatchResult is false:
    - 28a. If navigationType is "traverse", then consume history-action user activation given navigation's relevant global object.
    - 28b. If event's abort controller's signal is not aborted, then abort the ongoing navigation given navigation.
    - 28c. Return false.
  - 29. If event's interception state is not "none":
    - 29a. Let fromNHE be the current entry of navigation.
    - 29b. Assert: fromNHE is not null.
    - 29c. Set navigation's transition to a new NavigationTransition created in navigation's relevant realm, with navigation typenavigationTypefrom entryfromNHEdestinationevent's destinationcommitted promisea new promise created in navigation's relevant realmfinished promisea new promise created in navigation's relevant realm
    - 29d. Mark as handled navigation's transition's finished promise.
    - 29e. Mark as handled navigation's transition's committed promise.
  - 30. If event's navigation precommit handler list is empty then commit event given apiMethodTracker.
  - 31. Otherwise:
    - 31a. Let precommitController be a new NavigationPrecommitController created in navigation's relevant realm, whose event is event.
    - 31b. Let precommitPromisesList be an empty list.
    - 31c. For each handler of event's navigation precommit handler list:
      - 31c.i. Append the result of invoking handler with « precommitController » to precommitPromisesList.
    - 31d. Wait for all precommitPromisesList with the following success steps: commit event given apiMethodTracker, and the following failure step given reason: process navigate event handler failure given event and reason.
  - 32. If event's interception state is "none", then return true.
  - 33. Return false.

- [x] [navigate event intercept commit handler steps](https://html.spec.whatwg.org/#navigate-event-intercept-commit-handler-steps) — The navigate event intercept commit handler steps, given a Navigation navigation, a NavigateEvent object event, and a navigation API method tracker apiMethodTracker are as follows:
  - 1. Let promisesList be an empty list.
  - 2. For each handler of event's navigation handler list:
    - 2a. Append the result of invoking handler with an empty arguments list to promisesList.
  - 3. If promisesList's size is 0, then set promisesList to « a promise resolved with undefined ».
  - 4. Wait for all of promisesList, with the following success steps: and the following failure step given reason: process navigate event handler failure given event and reason.
    - 4a. If event's relevant global object is not fully active, then abort these steps.
    - 4b. If event's abort controller's signal is aborted, then abort these steps.
    - 4c. Assert: event equals navigation's ongoing navigate event.
    - 4d. Set navigation's ongoing navigate event to null.
    - 4e. Finish event given true.
    - 4f. If apiMethodTracker is non-null, then resolve the finished promise for apiMethodTracker.
    - 4g. Fire an event named navigatesuccess at navigation.
    - 4h. If navigation's transition is not null, then resolve navigation's transition's finished promise with undefined.
    - 4i. Set navigation's transition to null.

- [x] [commit a navigate event](https://html.spec.whatwg.org/#commit-a-navigate-event) — To commit a navigate event given a NavigateEvent object event and a navigation API method tracker apiMethodTracker:
  - 1. Let navigation be event's target.
  - 2. Let navigable be event's relevant global object's navigable.
  - 3. If event's relevant global object's associated Document is not fully active, then return.
  - 4. If event's abort controller's signal is aborted, then return.
  - 5. Let endResultIsSameDocument be true if event's interception state is not "none" or event's destination's is same document is true.
  - 6. Prepare to run script given navigation's relevant settings object.
  - 7. If event's interception state is not "none":
    - 7a. Set event's interception state to "committed".
    - 7b. Switch on event's navigationType: "push""replace"Run the URL and history update steps given event's relevant global object's associated Document and event's destination's URL, with serializedData set to event's classic history API state and historyHandling set to event's navigationType."reload"Update the navigation API entries for a same-document navigation given navigation, navigable's active se…
      - 7b.i. Set navigation's suppress normal scroll restoration during ongoing navigation to true.
      - 7b.ii. Let userInvolvement be "none".
      - 7b.iii. If event's userInitiated is true, then set userInvolvement to "activation".
      - 7b.iv. Append the following session history traversal steps to navigable's traversable navigable:
        - 7b.iv.A. Resume applying the traverse history step given event's destination's entry's session history entry's step, navigable's traversable navigable, and userInvolvement.
  - 8. If navigation's transition is not null, then resolve navigation's transition's committed promise with undefined.
  - 9. If endResultIsSameDocument is false and apiMethodTracker is non-null, then clean up apiMethodTracker.
  - 10. Clean up after running script given navigation's relevant settings object.

- [x] [process navigate event handler failure](https://html.spec.whatwg.org/#process-navigate-event-handler-failure) — To process navigate event handler failure given a NavigateEvent object event and a reason:
  - 1. If event's relevant global object's associated Document is not fully active, then return.
  - 2. If event's abort controller's signal is aborted, then return.
  - 3. Assert: event is event's relevant global object's navigation API's ongoing navigate event.
  - 4. If event's interception state is not "intercepted", then finish event given false.
  - 5. Abort event given reason.

###### [7.2.6.10.5 Scroll and focus behavior](https://html.spec.whatwg.org/#navigate-event-scroll-focus)

Status: reviewed (2026-11-01)
> Fixed: the focus reset starts from the document's autofocus delegate, falls back to the viewport and moves the sequential focus navigation starting point; a fragment that matches nothing scrolls to the beginning.
> An intercepted reload restores the scroll position (see `DIFFERENCES.md`).

- [x] [finish](https://html.spec.whatwg.org/#navigateevent-finish) — To finish a NavigateEvent event, given a boolean didFulfill:
  - 1. Assert: event's interception state is not "finished".
  - 2. If event's interception state is "intercepted":
    - 2a. Assert: didFulfill is false.
    - 2b. Assert: event's navigation precommit handler list is not empty.
    - 2c. Set event's interception state to "finished".
    - 2d. Return.
  - 3. If event's interception state is "none", then return.
  - 4. Potentially reset the focus given event.
  - 5. If didFulfill is true, then potentially process scroll behavior given event.
  - 6. Set event's interception state to "finished".

- [x] [potentially reset the focus](https://html.spec.whatwg.org/#potentially-reset-the-focus) — To potentially reset the focus given a NavigateEvent event:
  - 1. Assert: event's interception state is "committed" or "scrolled".
  - 2. Let navigation be event's relevant global object's navigation API.
  - 3. Let focusChanged be navigation's focus changed during ongoing navigation.
  - 4. Set navigation's focus changed during ongoing navigation to false.
  - 5. If focusChanged is true, then return.
  - 6. If event's focus reset behavior is "manual", then return.
  - 7. Let document be event's relevant global object's associated Document.
  - 8. Let focusTarget be the autofocus delegate for document.
  - 9. If focusTarget is null, then set focusTarget to document's body element.
  - 10. If focusTarget is null, then set focusTarget to document's document element.
  - 11. Run the focusing steps for focusTarget, with document's viewport as the fallback target.
  - 12. Move the sequential focus navigation starting point to focusTarget.

- [x] [potentially process scroll behavior](https://html.spec.whatwg.org/#potentially-process-scroll-behavior) — To potentially process scroll behavior given a NavigateEvent event:
  - 1. Assert: event's interception state is "committed" or "scrolled".
  - 2. If event's interception state is "scrolled", then return.
  - 3. If event's scroll behavior is "manual", then return.
  - 4. Process scroll behavior given event.

- [x] [process scroll behavior](https://html.spec.whatwg.org/#process-scroll-behavior) — To process scroll behavior given a NavigateEvent event:
  - 1. Assert: event's interception state is "committed".
  - 2. Set event's interception state to "scrolled".
  - 3. If event's navigationType was initialized to "traverse", then restore scroll position data given event's relevant global object's navigable's active session history entry.
  - 4. Otherwise:
    - 4a. Let document be event's relevant global object's associated Document.
    - 4b. If document's indicated part is null, then scroll to the beginning of the document given document. [CSSOMVIEW]
    - 4c. Otherwise, scroll to the fragment given document.

#### [7.2.7 Event interfaces](https://html.spec.whatwg.org/#nav-traversal-event-interfaces)

Status: reviewed (2026-11-01)
> The sections below.

##### [7.2.7.1 The NavigationCurrentEntryChangeEvent interface](https://html.spec.whatwg.org/#the-navigationcurrententrychangeevent-interface)

Status: reviewed (2026-11-01)
<!-- definitions: dom-navigationcurrententrychangeevent-from -->

- [x] [NavigationCurrentEntryChangeEvent.navigationType](https://html.spec.whatwg.org/#dom-navigationcurrententrychangeevent-navigationtype) — The navigationType and from attributes must return the values they were initialized to.

##### [7.2.7.2 The PopStateEvent interface](https://html.spec.whatwg.org/#the-popstateevent-interface)

Status: reviewed (2026-11-01)
> `hasUAVisualTransition` is always false.

- [x] [PopStateEvent.state](https://html.spec.whatwg.org/#dom-popstateevent-state) — The state attribute must return the value it was initialized to. It represents the context information for the event, or null, if the state represented is the initial state of the Document.

- [x] [PopStateEvent.hasUAVisualTransition](https://html.spec.whatwg.org/#dom-popstateevent-hasuavisualtransition) — The hasUAVisualTransition attribute must return the value it was initialized to.

##### [7.2.7.3 The HashChangeEvent interface](https://html.spec.whatwg.org/#the-hashchangeevent-interface)

Status: reviewed (2026-11-01)

- [x] [HashChangeEvent.oldURL](https://html.spec.whatwg.org/#dom-hashchangeevent-oldurl) — The oldURL attribute must return the value it was initialized to. It represents context information for the event, specifically the URL of the session history entry that was traversed from.

- [x] [HashChangeEvent.newURL](https://html.spec.whatwg.org/#dom-hashchangeevent-newurl) — The newURL attribute must return the value it was initialized to. It represents context information for the event, specifically the URL of the session history entry that was traversed to.

##### [7.2.7.4 The PageSwapEvent interface](https://html.spec.whatwg.org/#the-pageswapevent-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: dom-pageswapevent-viewtransition -->

- [x] [PageSwapEvent.activation](https://html.spec.whatwg.org/#dom-pageswapevent-activation) — The activation and viewTransition attributes must return the values they were initialized to.

##### [7.2.7.5 The PageRevealEvent interface](https://html.spec.whatwg.org/#the-pagerevealevent-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).

- [x] [PageRevealEvent.viewTransition](https://html.spec.whatwg.org/#dom-pagerevealevent-viewtransition) — The viewTransition attribute must return the value it was initialized to.

##### [7.2.7.6 The PageTransitionEvent interface](https://html.spec.whatwg.org/#the-pagetransitionevent-interface)

Status: reviewed (2026-11-01)
> Fixed: `pageshow` and `pagehide` bubble and are cancelable.

- [x] [PageTransitionEvent.persisted](https://html.spec.whatwg.org/#dom-pagetransitionevent-persisted) — The persisted attribute must return the value it was initialized to. It represents the context information for the event.

- [x] [fire a page transition event](https://html.spec.whatwg.org/#fire-a-page-transition-event) — To fire a page transition event named eventName at a Window window with a boolean persisted, fire an event named eventName at window, using PageTransitionEvent, with the persisted attribute initialized to persisted, the cancelable attribute initialized to true, the bubbles attribute initialized to true, and legacy target override flag set.

##### [7.2.7.7 The BeforeUnloadEvent interface](https://html.spec.whatwg.org/#the-beforeunloadevent-interface)

Status: reviewed (2026-11-01)

- [x] [BeforeUnloadEvent.returnValue](https://html.spec.whatwg.org/#dom-beforeunloadevent-returnvalue) — The returnValue attribute controls the process of checking if unloading is canceled. When the event is created, the attribute must be set to the empty string. On getting, it must return the last value it was set to. On setting, the attribute must be set to the new value.

#### [7.2.8 The NotRestoredReasons interface](https://html.spec.whatwg.org/#the-notrestoredreasons-interface)

Status: reviewed (2026-11-01)
> Not implemented (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-not-restored-reason-details-backing-struct nrr-details-struct nrr-details-reason blocking-fetch blocking-navigation-failure blocking-parser-aborted blocking-websocket blocking-weblock blocking-masked ua-specific-blocking-reasons blocking-audio-capture blocking-background-work blocking-broadcastchannel-message blocking-idb-event blocking-idledetector blocking-keyboard-lock blocking-mediastream blocking-midi blocking-modals blocking-navigating blocking-navigation-canceled blocking-non-trivial-bcg blocking-otp blocking-outstanding-network-request blocking-payment blocking-picture-in-picture blocking-plugins blocking-method-not-get blocking-auth-required blocking-ccns blocking-ccnc blocking-keepalive blocking-not-http-https blocking-response-not-ok blocking-rtc blocking-rtc-used-with-ccns blocking-sensors sw-added sw-claimed blocking-sw-message blocking-sw-activation blocking-sw-unregistered blocking-sharedworker blocking-sharedworker-message blocking-sharedworker-with-no-active-client blocking-smartcard blocking-speech-reco blocking-storage blocking-unload blocking-video-capture blocking-webhid blocking-webshare blocking-websocket-used-with-ccns blocking-webtransport blocking-webtransport-used-with-ccns blocking-webxr concept-not-restored-reasons-backing-struct concept-not-restored-reasons-reasons concept-not-restored-reasons-children nrr-struct nrr-src nrr-id nrr-name nrr-url nrr-reasons nrr-children -->

- [x] [NotRestoredReasonDetails.reason](https://html.spec.whatwg.org/#dom-not-restored-reason-details-reason) — The reason getter steps are to return this's backing struct's reason.

- [x] [create a NotRestoredReasonDetails object](https://html.spec.whatwg.org/#create-a-notrestoredreasondetails-object) — To create a NotRestoredReasonDetails object given a not restored reason details backingStruct and a realm realm:
  - 1. Let notRestoredReasonDetails be a new NotRestoredReasonDetails object created in realm.
  - 2. Set notRestoredReasonDetails's backing struct to backingStruct.
  - 3. Return notRestoredReasonDetails.

- [x] [NotRestoredReasons.src](https://html.spec.whatwg.org/#dom-not-restored-reasons-src) — The src getter steps are to return this's backing struct's src.

- [x] [NotRestoredReasons.id](https://html.spec.whatwg.org/#dom-not-restored-reasons-id) — The id getter steps are to return this's backing struct's id.

- [x] [NotRestoredReasons.name](https://html.spec.whatwg.org/#dom-not-restored-reasons-name) — The name getter steps are to return this's backing struct's name.

- [x] [NotRestoredReasons.url](https://html.spec.whatwg.org/#dom-not-restored-reasons-url) — The url getter steps are:
  - 1. If this's backing struct's URL is null, then return null.
  - 2. Return this's backing struct's URL, serialized.

- [x] [NotRestoredReasons.reasons](https://html.spec.whatwg.org/#dom-not-restored-reasons-reasons) — The reasons getter steps are to return this's reasons array.

- [x] [NotRestoredReasons.children](https://html.spec.whatwg.org/#dom-not-restored-reasons-children) — The children getter steps are to return this's children array.

- [x] [create a NotRestoredReasons object](https://html.spec.whatwg.org/#create-a-notrestoredreasons-object) — To create a NotRestoredReasons object given a not restored reasons backingStruct and a realm realm:
  - 1. Let notRestoredReasons be a new NotRestoredReasons object created in realm.
  - 2. Set notRestoredReasons's backing struct to backingStruct.
  - 3. If backingStruct's reasons is null, set notRestoredReasons's reasons array to null.
  - 4. Otherwise:
    - 4a. Let reasonsArray be an empty list.
    - 4b. For each reason of backingStruct's reasons:
      - 4b.i. Create a NotRestoredReasonDetails object given reason and realm, and append it to reasonsArray.
    - 4c. Set notRestoredReasons's reasons array to the result of creating a frozen array given reasonsArray.
  - 5. If backingStruct's children is null, set notRestoredReasons's children array to null.
  - 6. Otherwise:
    - 6a. Let childrenArray be an empty list.
    - 6b. For each child of backingStruct's children:
      - 6b.i. Create a NotRestoredReasons object given child and realm and append it to childrenArray.
    - 6c. Set notRestoredReasons's children array to the result of creating a frozen array given childrenArray.
  - 7. Return notRestoredReasons.

- [x] [Document's not restored reasons (Document)](https://html.spec.whatwg.org/#concept-document-nrr) — A Document's not restored reasons is its node navigable's active session history entry's document state's not restored reasons, if Document's node navigable is a top-level traversable; otherwise null.
