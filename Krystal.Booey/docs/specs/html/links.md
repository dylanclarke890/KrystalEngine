# HTML review checklist: 4.6 Links

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 56 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html links` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.6 Links](https://html.spec.whatwg.org/#links)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.6.1 Introduction](https://html.spec.whatwg.org/#introduction-2)

Status: reviewed (2026-11-01)
> Definitions only: hyperlinks are `<a>`, `<area>` and `<form>`'s (`HyperlinkAlgorithms`), external resource links `HTMLLinkElement`'s stylesheets.
<!-- definitions: external-resource-link hyperlink internal-resource-link hyperlink-annotation -->

#### [4.6.2 Links created by a and area elements](https://html.spec.whatwg.org/#links-created-by-a-and-area-elements)

Status: reviewed (2026-11-01)
> `HyperlinkAlgorithms::ActivationBehavior`, for `HTMLAnchorElement` and `HTMLAreaElement`; their attributes are reflected (4.5.1 and 4.8).
> Fixed: the server-side image map suffix was added only without a `usemap`, before the URL was parsed, without clamping or the trusted check (see `DIFFERENCES.md`).
<!-- definitions: attr-hyperlink-href attr-hyperlink-target attr-hyperlink-download ping attr-hyperlink-rel attr-hyperlink-referrerpolicy -->

- [x] [The activation behavior of an a or area element element given an event event](https://html.spec.whatwg.org/#the-a-element) — The activation behavior of an a or area element element given an event event is:
  - 1. If element has no href attribute, then return.
  - 2. Let hyperlinkSuffix be null.
  - 3. If element is an a element, and event's target is an img with an ismap attribute specified:
    - 3a. Let x and y be 0.
    - 3b. If event's isTrusted attribute is initialized to true, then set x to the distance in CSS pixels from the left edge of the image to the location of the click, and set y to the distance in CSS pixels from the top edge of the image to the location of the click.
    - 3c. If x is negative, set x to 0.
    - 3d. If y is negative, set y to 0.
    - 3e. Set hyperlinkSuffix to the concatenation of U+003F (?), the value of x expressed as a base-ten integer using ASCII digits, U+002C (,), and the value of y expressed as a base-ten integer using ASCII digits.
  - 4. Let userInvolvement be event's user navigation involvement.
  - 5. If the user has expressed a preference to download the hyperlink, then set userInvolvement to "browser UI".
  - 6. If element has a download attribute, or if the user has expressed a preference to download the hyperlink, then download the hyperlink created by element with hyperlinkSuffix set to hyperlinkSuffix and userInvolvement set to userInvolvement.
  - 7. Otherwise, follow the hyperlink created by element with hyperlinkSuffix set to hyperlinkSuffix and userInvolvement set to userInvolvement.

#### [4.6.3 API for hyperlink elements](https://html.spec.whatwg.org/#api-for-hyperlink-elements)

Status: reviewed (2026-11-01)
> `URLDecomposition`, shared with `Location`, computes the url again from `href` each time (`FullURL`), and updates `href` through `SetFullURL`. There are no origins to extract.
> Fixed: the setters wrote an `href` from the document's URL for a link without one, where they should do nothing.
<!-- definitions: hyperlink-element concept-hyperlink-url-set update-href attr-hyperlink-hreflang attr-hyperlink-type -->

- [x] [A hyperlink element has the following extract an origin steps](https://html.spec.whatwg.org/#concept-hyperlink-url) — A hyperlink element has the following extract an origin steps:
  - 1. If this's url is null, then return null.
  - 2. Return this's url's origin.
  > n/a: there are no origins.

- [x] [reinitialize url](https://html.spec.whatwg.org/#reinitialise-url) — A hyperlink element has an associated reinitialize url algorithm, which runs these steps:
  - 1. If the element's url is non-null, its scheme is "blob", and it has an opaque path, then terminate these steps.
  - 2. Set the url.
  > `FullURL` parses `href` each time; with no blob URL store, a blob URL needs nothing kept.

- [x] [HyperlinkElementUtils.origin](https://html.spec.whatwg.org/#dom-hyperlink-origin) — The origin getter steps are:
  - 1. Reinitialize url.
  - 2. If this's url is null, return the empty string.
  - 3. Return the serialization of this's url's origin.

- [x] [HyperlinkElementUtils.protocol](https://html.spec.whatwg.org/#dom-hyperlink-protocol) — The protocol getter steps are:
  - 1. Reinitialize url.
  - 2. If this's url is null, return ":".
  - 3. Return this's url's scheme, followed by ":".

- [x] [The protocol setter steps](https://html.spec.whatwg.org/#dom-hyperlink-protocol) — The protocol setter steps are:
  - 1. Reinitialize url.
  - 2. If this's url is null, then return.
  - 3. Basic URL parse the given value, followed by ":", with this's url as url and scheme start state as state override.
  - 4. Update href.

- [x] [HyperlinkElementUtils.username](https://html.spec.whatwg.org/#dom-hyperlink-username) — The username getter steps are:
  - 1. Reinitialize url.
  - 2. If this's url is null, return the empty string.
  - 3. Return this's url's username.

- [x] [The username setter steps](https://html.spec.whatwg.org/#dom-hyperlink-username) — The username setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url cannot have a username/password/port, then return.
  - 4. Set the username, given url and the given value.
  - 5. Update href.

- [x] [HyperlinkElementUtils.password](https://html.spec.whatwg.org/#dom-hyperlink-password) — The password getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, then return the empty string.
  - 4. Return url's password.

- [x] [The password setter steps](https://html.spec.whatwg.org/#dom-hyperlink-password) — The password setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url cannot have a username/password/port, then return.
  - 4. Set the password, given url and the given value.
  - 5. Update href.

- [x] [HyperlinkElementUtils.host](https://html.spec.whatwg.org/#dom-hyperlink-host) — The host getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url or url's host is null, return the empty string.
  - 4. If url's port is null, return url's host, serialized.
  - 5. Return url's host, serialized, followed by ":" and url's port, serialized.

- [x] [The host setter steps](https://html.spec.whatwg.org/#dom-hyperlink-host) — The host setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url has an opaque path, then return.
  - 4. Basic URL parse the given value, with url as url and host state as state override.
  - 5. Update href.

- [x] [HyperlinkElementUtils.hostname](https://html.spec.whatwg.org/#dom-hyperlink-hostname) — The hostname getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url or url's host is null, return the empty string.
  - 4. Return url's host, serialized.

- [x] [The hostname setter steps](https://html.spec.whatwg.org/#dom-hyperlink-hostname) — The hostname setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url has an opaque path, then return.
  - 4. Basic URL parse the given value, with url as url and hostname state as state override.
  - 5. Update href.

- [x] [HyperlinkElementUtils.port](https://html.spec.whatwg.org/#dom-hyperlink-port) — The port getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url or url's port is null, return the empty string.
  - 4. Return url's port, serialized.

- [x] [The port setter steps](https://html.spec.whatwg.org/#dom-hyperlink-port) — The port setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url cannot have a username/password/port, then return.
  - 4. If the given value is the empty string, then set url's port to null.
  - 5. Otherwise, basic URL parse the given value, with url as url and port state as state override.
  - 6. Update href.

- [x] [HyperlinkElementUtils.pathname](https://html.spec.whatwg.org/#dom-hyperlink-pathname) — The pathname getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, then return the empty string.
  - 4. Return the result of URL path serializing url.

- [x] [The pathname setter steps](https://html.spec.whatwg.org/#dom-hyperlink-pathname) — The pathname setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null or url has an opaque path, then return.
  - 4. Set url's path to the empty list.
  - 5. Basic URL parse the given value, with url as url and path start state as state override.
  - 6. Update href.

- [x] [HyperlinkElementUtils.search](https://html.spec.whatwg.org/#dom-hyperlink-search) — The search getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, or url's query is either null or the empty string, return the empty string.
  - 4. Return "?", followed by url's query.

- [x] [The search setter steps](https://html.spec.whatwg.org/#dom-hyperlink-search) — The search setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, terminate these steps.
  - 4. If the given value is the empty string, set url's query to null.
  - 5. Otherwise:
    - 5a. Let input be the given value with a single leading "?" removed, if any.
    - 5b. Set url's query to the empty string.
    - 5c. Basic URL parse input, with url as url and query state as state override.
  - 6. Update href.

- [x] [HyperlinkElementUtils.hash](https://html.spec.whatwg.org/#dom-hyperlink-hash) — The hash getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, or url's fragment is either null or the empty string, return the empty string.
  - 4. Return "#", followed by url's fragment.

- [x] [The hash setter steps](https://html.spec.whatwg.org/#dom-hyperlink-hash) — The hash setter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null, then return.
  - 4. If the given value is the empty string, set url's fragment to null.
  - 5. Otherwise:
    - 5a. Let input be the given value with a single leading "#" removed, if any.
    - 5b. Set url's fragment to the empty string.
    - 5c. Basic URL parse input, with url as url and fragment state as state override.
  - 6. Update href.

#### [4.6.4 API for a and area elements](https://html.spec.whatwg.org/#api-for-a-and-area-elements)

Status: reviewed (2026-11-01)
> `href` is reflected as a URL (`ReflectURL`), and the url is computed when it's needed. There are no speculative loads.

- [x] [HTMLHyperlinkElementUtils.href](https://html.spec.whatwg.org/#dom-hyperlink-href) — The href getter steps are:
  - 1. Reinitialize url.
  - 2. Let url be this's url.
  - 3. If url is null and this has no href content attribute, return the empty string.
  - 4. Otherwise, if url is null, return this's href content attribute's value.
  - 5. Return url, serialized.

- [x] [The HTML element insertion steps for a and area elements, given insertedNode,](https://html.spec.whatwg.org/#html-element-insertion-steps) — The HTML element insertion steps for a and area elements, given insertedNode, are:
  - 1. If insertedNode is not connected, then return.
  - 2. Consider speculative loads given insertedNode's node document.
  > n/a: no speculative loads.

- [x] [The HTML element removing steps for a and area elements, given removedNode, isS…](https://html.spec.whatwg.org/#html-element-removing-steps) — The HTML element removing steps for a and area elements, given removedNode, isSubtreeRoot, and oldAncestor are:
  - 1. If oldAncestor is not connected, then return.
  - 2. Consider speculative loads given oldAncestor's node document.
  > n/a: no speculative loads.

- [x] [The HTML element moving steps for a and area elements, given movedNode,](https://html.spec.whatwg.org/#html-element-moving-steps) — The HTML element moving steps for a and area elements, given movedNode, are:
  - 1. Consider speculative loads given movedNode's node document.
  > n/a: no speculative loads.

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#dom-hyperlink-href) — The following attribute change steps, given element, localName, oldValue, value, and namespace, are used for all a and area elements:
  - 1. If namespace is not null, then return.
  - 2. If oldValue equals value, then return.
  - 3. If localName is href, then set the url given element.
  - 4. If localName is href, referrerpolicy, or rel, then consider speculative loads given element's node document.

- [x] [The set the url algorithm for HTMLAnchorElement and HTMLAreaElement elements](https://html.spec.whatwg.org/#concept-hyperlink-url-set) — The set the url algorithm for HTMLAnchorElement and HTMLAreaElement elements is as follows:
  - 1. Set this element's url to null.
  - 2. If this element's href content attribute is absent, then return.
  - 3. Let url be the result of encoding-parsing a URL given this element's href content attribute's value, relative to this element's node document.
  - 4. If url is not failure, then set this element's url to url.
  > `HyperlinkAlgorithms::Href`, computed when it's needed.

#### [4.6.5 Following hyperlinks](https://html.spec.whatwg.org/#following-hyperlinks)

Status: reviewed (2026-11-01)
> `HyperlinkAlgorithms::FollowTheHyperlink`, `GetNoopener` and `HyperlinkReferrerPolicy`; the loader (`FrameLoader::ChangeLocation`) chooses the navigable.
> Fixed: a URL that doesn't parse was still navigated to.
<!-- definitions: following-hyperlinksuffix following-userinvolvement -->

- [x] [cannot navigate](https://html.spec.whatwg.org/#cannot-navigate) — An element element cannot navigate if any of the following are true:
  > Step 1 of `HyperlinkAlgorithms::FollowTheHyperlink`: no frame (not fully active), or an `<area>` that isn't connected.

- [x] [get an element's noopener](https://html.spec.whatwg.org/#get-an-element's-noopener) — To get an element's noopener, given an a, area, or form element element, a URL record url, and a string target, perform the following steps. They return a boolean.
  - 1. If element's link types include the noopener or noreferrer keyword, then return true.
  - 2. If element's link types do not include the opener keyword and target is an ASCII case-insensitive match for "_blank", then return true.
  - 3. If url's blob URL entry is not null:
    - 3a. Let blobOrigin be url's blob URL entry's environment's origin.
    - 3b. Let topLevelOrigin be element's relevant settings object's top-level origin.
    - 3c. If blobOrigin is not same site with topLevelOrigin, then return true.
  - 4. Return false.

- [x] [follow the hyperlink](https://html.spec.whatwg.org/#following-hyperlinks-2) — To follow the hyperlink created by an element subject, given an optional hyperlinkSuffix (default null) and an optional userInvolvement (default "none"):
  - 1. If subject cannot navigate, then return.
  - 2. Let targetAttributeValue be the empty string.
  - 3. If subject is an a or area element, then set targetAttributeValue to the result of getting an element's target given subject.
  - 4. Let urlRecord be the result of encoding-parsing a URL given subject's href attribute value, relative to subject's node document.
  - 5. If urlRecord is failure, then return.
  - 6. Let noopener be the result of getting an element's noopener with subject, urlRecord, and targetAttributeValue.
  - 7. Let targetNavigable be the first return value of applying the rules for choosing a navigable given targetAttributeValue, subject's node navigable, and noopener.
  - 8. If targetNavigable is null, then return.
  - 9. Let urlString be the result of applying the URL serializer to urlRecord.
  - 10. If hyperlinkSuffix is non-null, then append it to urlString.
  - 11. Navigate targetNavigable to urlString using subject's node document, with referrerPolicy set to subject's hyperlink referrer policy, userInvolvement set to userInvolvement, and sourceElement set to subject.

- [x] [hyperlink referrer policy](https://html.spec.whatwg.org/#hyperlink-referrer-policy) — The hyperlink referrer policy for an element subject is the value returned by the following steps:
  - 1. If subject's link types includes the noreferrer keyword, then return "no-referrer".
  - 2. Return the current state of subject's referrerpolicy content attribute.

#### [4.6.6 Downloading resources](https://html.spec.whatwg.org/#downloading-resources)

Status: n/a (downloads are the host's: the `download` attribute is passed to it; see `KNOWN-ISSUES.md`)
<!-- definitions: downloading-hyperlinksuffix downloading-userinvolvement concept-extension -->

- [x] [download the hyperlink](https://html.spec.whatwg.org/#downloading-hyperlinks) — To download the hyperlink created by an element subject, given an optional hyperlinkSuffix (default null) and an optional userInvolvement (default "none"):
  - 1. If subject cannot navigate, then return.
  - 2. If subject's node document's active sandboxing flag set has the sandboxed downloads browsing context flag set, then return.
  - 3. Let urlString be the result of encoding-parsing-and-serializing a URL given subject's href attribute value, relative to subject's node document.
  - 4. If urlString is failure, then return.
  - 5. If hyperlinkSuffix is non-null, then append it to urlString.
  - 6. If userInvolvement is not "browser UI":
    - 6a. Assert: subject has a download attribute.
    - 6b. Let navigation be subject's relevant global object's navigation API.
    - 6c. Let filename be the value of subject's download attribute.
    - 6d. Let continue be the result of firing a download request navigate event at navigation with destinationURL set to urlString, userInvolvement set to userInvolvement, sourceElement set to subject, and filename set to filename.
    - 6e. If continue is false, then return.
    - 6f. Inform the navigation API about aborting navigation given subject's node navigable.
  - 7. Optionally, the user agent may return, if it believes doing so would safeguard the user from a potentially hostile download.
  - 8. Let request be a new request whose URL is urlString, client is entry settings object, initiator is "download", destination is the empty string, and whose use-URL-credentials flag is set.
  - 9. Fetch request, with processResponse set to the following steps given response response:
    - 9a. Handle as a download response with subject's node navigable and null.
  > n/a: the host downloads.

- [x] [handle as a download](https://html.spec.whatwg.org/#handle-as-a-download) — To handle as a download a response response with a navigable navigable and a navigation ID or null navigationId:
  - 1. Let suggestedFilename be the result of getting the suggested filename for response.
  - 2. Let download behavior be the result of WebDriver BiDi download will begin with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "pending", url is response's URL, suggestedFilename is suggestedFilename, and downloadResponse is response.
  - 3. If download behavior is not null and download behavior's allowed is false:
    - 3a. Invoke WebDriver BiDi download end with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "canceled", url is response's URL, and downloadResponse is response.
    - 3b. Return.
  - 4. If download behavior is not null, let destinationFolder be download behavior's destinationFolder.
  - 5. Run these steps in parallel:
    - 5a. Run implementation-defined steps to save response for later use. If destinationFolder is not null, the user agent should save the file to that path. If the user agent needs a filename, the user agent should use the suggestedFilename.
    - 5b. If any of the following are true: the download is canceled by the user;; the download is canceled by the user agent;; an error occurs (for example, a network error, not enough storage, an unavailable destination folder); then:
      - 5b.i. Invoke WebDriver BiDi download end with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "canceled", url is response's URL.
      - 5b.ii. Return.
    - 5c. When the download completes successfully, invoke WebDriver BiDi download end with navigable and a new WebDriver BiDi navigation status whose id is navigationId, status is "complete", downloadedFilepath is an absolute path of the downloaded file if available, otherwise null, url is response's URL, and downloadResponse is response.
  > n/a: the host downloads.

- [x] [get the suggested filename](https://html.spec.whatwg.org/#getting-the-suggested-filename) — To get the suggested filename for a response response:
  - 1. Let filename be the undefined value.
  - 2. If response has a `Content-Disposition` header, that header specifies the attachment disposition type, and the header includes filename information, then let filename have the value specified by the header, and jump to the step labeled sanitize below. [RFC6266]
  - 3. Let interface origin be the origin of the Document in which the download or navigate action resulting in the download was initiated, if any.
  - 4. Let response origin be the origin of the URL of response, unless that URL's scheme component is data, in which case let response origin be the same as the interface origin, if any.
  - 5. If there is no interface origin, then let trusted operation be true. Otherwise, let trusted operation be true if response origin is the same origin as interface origin, and false otherwise.
  - 6. If trusted operation is true and response has a `Content-Disposition` header and that header includes filename information, then let filename have the value specified by the header, and jump to the step labeled sanitize below. [RFC6266]
  - 7. If the download was not initiated from a hyperlink created by an a or area element, or if the element of the hyperlink from which it was initiated did not have a download attribute when the download was initiated, or if there was such an attribute but its value when the download was initiated was the empty string, then jump to the step labeled no proposed filename.
  - 8. Let proposed filename have the value of the download attribute of the element of the hyperlink that initiated the download at the time the download was initiated.
  - 9. If trusted operation is true, let filename have the value of proposed filename, and jump to the step labeled sanitize below.
  - 10. If response has a `Content-Disposition` header and that header specifies the attachment disposition type, let filename have the value of proposed filename, and jump to the step labeled sanitize below. [RFC6266]
  - 11. No proposed filename: If trusted operation is true, or if the user indicated a preference for having the response in question downloaded, let filename have a value derived from the URL of response in an implementation-defined manner, and jump to the step labeled sanitize below.
  - 12. Let filename be set to the user's preferred filename or to a filename selected by the user agent, and jump to the step labeled sanitize below.
  - 13. Sanitize: Optionally, allow the user to influence filename. For example, a user agent could prompt the user for a filename, potentially providing the value of filename as determined above as a default value.
  - 14. Adjust filename to be suitable for the local file system.
  - 15. If the platform conventions do not in any way use extensions to determine the types of file on the file system, then return filename as the filename.
  - 16. Let claimed type be the type given by response's Content-Type metadata, if any is known. Let named type be the type given by filename's extension, if any is known. For the purposes of this step, a type is a mapping of a MIME type to an extension.
  - 17. If named type is consistent with the user's preferences (e.g., because the value of filename was determined by prompting the user), then return filename as the filename.
  - 18. If claimed type and named type are the same type (i.e., the type given by response's Content-Type metadata is consistent with the type given by filename's extension), then return filename as the filename.
  - 19. If the claimed type is known, then alter filename to add an extension corresponding to claimed type. Otherwise, if named type is known to be potentially dangerous (e.g. it will be treated by the platform conventions as a native executable, shell script, HTML application, or executable-macro-capable document), then optionally alter filename to add a known-safe extension (e.g. ".txt").
  - 20. Return filename as the filename.
  > n/a: the host downloads.

#### [4.6.7 Hyperlink auditing](https://html.spec.whatwg.org/#hyperlink-auditing)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)

- [x] [If a hyperlink created by an a or area element has a ping attribute, and the us…](https://html.spec.whatwg.org/#hyperlink-auditing) — If a hyperlink created by an a or area element has a ping attribute, and the user follows the hyperlink, and the value of the element's href attribute can be parsed, relative to the element's node document, without failure, then the user agent must take the ping attribute's value, split that string…
  - 1. If ping URL's scheme is not an HTTP(S) scheme, then return.
  - 2. Optionally, return. (For example, the user agent might wish to ignore any or all ping URLs in accordance with the user's expressed preferences.)
  - 3. Let settingsObject be the element's node document's relevant settings object.
  - 4. Let request be a new request whose URL is ping URL, method is `POST`, header list is « (`Content-Type`, `text/ping`) », body is `PING`, client is settingsObject, destination is the empty string, credentials mode is "include", referrer is "no-referrer", and whose use-URL-credentials flag is set, and whose initiator type is "ping".
  - 5. Let target URL be the result of encoding-parsing-and-serializing a URL given the element's href attribute's value, relative to the element's node document, and then: If the URL of the Document object containing the hyperlink being audited and ping URL have the same originIf the origins are different, but the scheme of the URL of the Document containing the hyperlink being audited is not "https"re…
  - 6. Fetch request.

##### [4.6.7.1 The `Ping-From` and `Ping-To` headers](https://html.spec.whatwg.org/#the-ping-headers)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)
<!-- definitions: ping-from ping-to -->

#### [4.6.8 Link types](https://html.spec.whatwg.org/#linkTypes)

Status: reviewed (2026-11-01)
> `HTMLLinkElement` recognises only `stylesheet` and `alternate` (see `KNOWN-ISSUES.md`); `<a>` and `<area>` read `noopener`, `noreferrer` and `opener`.
<!-- definitions: body-ok -->

##### [4.6.8.1 Link type "alternate"](https://html.spec.whatwg.org/#rel-alternate)

Status: reviewed (2026-11-01)
> `HTMLLinkElement::IsAlternate`: an alternative stylesheet, applied when enabled. Other alternates are only for authors.

##### [4.6.8.2 Link type "author"](https://html.spec.whatwg.org/#link-type-author)

Status: n/a (author conformance)

##### [4.6.8.3 Link type "bookmark"](https://html.spec.whatwg.org/#link-type-bookmark)

Status: n/a (author conformance)

##### [4.6.8.4 Link type "canonical"](https://html.spec.whatwg.org/#link-type-canonical)

Status: n/a (author conformance)

##### [4.6.8.5 Link type "dns-prefetch"](https://html.spec.whatwg.org/#link-type-dns-prefetch)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)

- [x] [The fetch and process the linked resource steps for this type of linked resourc…](https://html.spec.whatwg.org/#fetch-and-process-the-linked-resource) — The fetch and process the linked resource steps for this type of linked resource, given a link element el, are:
  - 1. Let url be the result of encoding-parsing a URL given el's href attribute's value, relative to el's node document.
  - 2. If url is failure, then return.
  - 3. Let partitionKey be the result of determining the network partition key given el's node document's relevant settings object.
  - 4. The user agent should resolve an origin given partitionKey and url's origin.

##### [4.6.8.6 Link type "expect"](https://html.spec.whatwg.org/#link-type-expect)

Status: n/a (not implemented; see `KNOWN-ISSUES.md`)

- [x] [process internal resource link](https://html.spec.whatwg.org/#process-internal-resource-link) — To process internal resource link given a link element el, run these steps:
  - 1. Let doc be el's node document.
  - 2. Let url be the result of encoding-parsing a URL given el's href attribute's value, relative to doc.
  - 3. If this fails, or if url does not equal doc's URL with exclude fragments set to true, then unblock rendering on el and return.
  - 4. Let indicatedElement be the result of selecting the indicated part given doc and url.
  - 5. If all of the following are true: doc's current document readiness is "loading";; el creates an internal resource link;; el is browsing-context connected;; el's rel attribute contains expect;; el is potentially render-blocking;; el's media attribute matches the environment; and; indicatedElement is not an element, or is on a stack of open elements of an HTML parser whose associated Document is do…
  - 6. Otherwise, unblock rendering on el.

- [x] [process internal resource links](https://html.spec.whatwg.org/#process-internal-resource-links) — To process internal resource links given a Document doc:
  - 1. For each expect link element link in doc's render-blocking element set, process link.

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#process-internal-resource-links) — The following attribute change steps, given element, localName, oldValue, value, and namespace, are used to ensure expect link elements respond to dynamic id and name changes:
  - 1. If namespace is not null, then return.
  - 2. If element is in a stack of open elements of an HTML parser, then return.
  - 3. If any of the following is true: localName is id; or; localName is name and element is an a element, then process internal resource links given element's node document.

##### [4.6.8.7 Link type "external"](https://html.spec.whatwg.org/#link-type-external)

Status: n/a (author conformance)

##### [4.6.8.8 Link type "help"](https://html.spec.whatwg.org/#link-type-help)

Status: n/a (author conformance)

##### [4.6.8.9 Link type "icon"](https://html.spec.whatwg.org/#rel-icon)

Status: n/a (Booey has no browser user interface)

- [x] [Other keywords must be further parsed as follows to determine what they represe…](https://html.spec.whatwg.org/#attr-link-sizes-any) — Other keywords must be further parsed as follows to determine what they represent:
  - 1. If the keyword doesn't contain exactly one U+0078 LATIN SMALL LETTER X or U+0058 LATIN CAPITAL LETTER X character, then this keyword doesn't represent anything. Return for that keyword.
  - 2. Let width string be the string before the "x" or "X".
  - 3. Let height string be the string after the "x" or "X".
  - 4. If either width string or height string start with a U+0030 DIGIT ZERO (0) character or contain any characters other than ASCII digits, then this keyword doesn't represent anything. Return for that keyword.
  - 5. Apply the rules for parsing non-negative integers to width string to obtain width.
  - 6. Apply the rules for parsing non-negative integers to height string to obtain height.
  - 7. The keyword represents that the resource contains a bitmap icon with a width of width device pixels and a height of height device pixels.

- [x] [The linked resource fetch setup steps for this type of linked resource, given a…](https://html.spec.whatwg.org/#linked-resource-fetch-setup-steps) — The linked resource fetch setup steps for this type of linked resource, given a link element el and request request, are:
  - 1. Set request's destination to "image".
  - 2. Return true.

- [x] [In the absence of a link with the icon keyword, for Document objects whose URL'…](https://html.spec.whatwg.org/#attr-link-sizes-any) — In the absence of a link with the icon keyword, for Document objects whose URL's scheme is an HTTP(S) scheme, user agents may instead run these steps:
  - 1. Let request be a new request whose URL is the URL record obtained by resolving the URL "/favicon.ico" against the Document object's URL, client is the Document object's relevant settings object, destination is "image", credentials mode is "include", and whose use-URL-credentials flag is set.
  - 2. Fetch request, with processResponse set to the following steps given response response:
    - 2a. Use response's unsafe response as an icon as if it had been declared using the icon keyword.

##### [4.6.8.10 Link type "license"](https://html.spec.whatwg.org/#link-type-license)

Status: n/a (author conformance)

##### [4.6.8.11 Link type "manifest"](https://html.spec.whatwg.org/#link-type-manifest)

Status: n/a (no application manifests)

- [x] [The linked resource fetch setup steps for this type of linked resource, given a…](https://html.spec.whatwg.org/#linked-resource-fetch-setup-steps) — The linked resource fetch setup steps for this type of linked resource, given a link element el and request request, are:
  - 1. Let navigable be el's node document's node navigable.
  - 2. If navigable is null, then return false.
  - 3. If navigable is not a top-level traversable, then return false.
  - 4. Set request's initiator to "manifest".
  - 5. Set request's destination to "manifest".
  - 6. Set request's mode to "cors".
  - 7. Set request's credentials mode to the CORS settings attribute credentials mode for el's crossorigin content attribute.
  - 8. Return true.

- [x] [To process this type of linked resource given a link element el, boolean succes…](https://html.spec.whatwg.org/#link-type-manifest) — To process this type of linked resource given a link element el, boolean success, response response, and byte sequence bodyBytes:
  - 1. If response's Content-Type metadata is not a JSON MIME type, then set success to false.
  - 2. If success is true:
    - 2a. Let document URL be el's node document's URL.
    - 2b. Let manifest URL be response's URL.
    - 2c. Let client be el's node document's relevant settings object.
    - 2d. Process the manifest given document URL, manifest URL, bodyBytes, and client. [MANIFEST]

##### [4.6.8.12 Link type "modulepreload"](https://html.spec.whatwg.org/#link-type-modulepreload)

Status: n/a (no scripting)
<!-- definitions: module-preload-destination -->

- [x] [The fetch and process the linked resource algorithm for modulepreload links, gi…](https://html.spec.whatwg.org/#fetch-and-process-the-linked-resource) — The fetch and process the linked resource algorithm for modulepreload links, given a link element el, is as follows:
  - 1. If el's href attribute's value is the empty string, then return.
  - 2. Let destination be the current state of el's as attribute (a destination), or "script" if it is in no state.
  - 3. If destination is not a module preload destination, then queue an element task on the networking task source given el to fire an event named error at el, and return.
  - 4. Let url be the result of encoding-parsing a URL given el's href attribute's value, relative to el's node document.
  - 5. If url is failure, then return.
  - 6. Let settings object be el's node document's relevant settings object.
  - 7. Let credentials mode be the CORS settings attribute credentials mode for el's crossorigin attribute.
  - 8. Let cryptographic nonce be el.[[CryptographicNonce]].
  - 9. Let integrity metadata be the value of el's integrity attribute, if it is specified, or the empty string otherwise.
  - 10. If el does not have an integrity attribute, then set integrity metadata to the result of resolving a module integrity metadata with url and settings object.
  - 11. Let referrer policy be the current state of el's referrerpolicy attribute.
  - 12. Let fetch priority be the current state of el's fetchpriority attribute.
  - 13. Let options be a script fetch options whose cryptographic nonce is cryptographic nonce, integrity metadata is integrity metadata, parser metadata is "not-parser-inserted", credentials mode is credentials mode, referrer policy is referrer policy, and fetch priority is fetch priority.
  - 14. Fetch a modulepreload module script graph given url, destination, settings object, options, and with the following steps given result:
    - 14a. If result is null, then fire an event named error at el, and return.
    - 14b. Fire an event named load at el.

##### [4.6.8.13 Link type "nofollow"](https://html.spec.whatwg.org/#link-type-nofollow)

Status: n/a (author conformance)

##### [4.6.8.14 Link type "noopener"](https://html.spec.whatwg.org/#link-type-noopener)

Status: reviewed (2026-11-01)
> `HyperlinkAlgorithms::GetNoopener`.

##### [4.6.8.15 Link type "noreferrer"](https://html.spec.whatwg.org/#link-type-noreferrer)

Status: reviewed (2026-11-01)
> `HyperlinkAlgorithms::GetNoopener` and `HyperlinkReferrerPolicy`.

##### [4.6.8.16 Link type "opener"](https://html.spec.whatwg.org/#link-type-opener)

Status: reviewed (2026-11-01)
> `HyperlinkAlgorithms::GetNoopener`.

##### [4.6.8.17 Link type "pingback"](https://html.spec.whatwg.org/#link-type-pingback)

Status: n/a (author conformance)

##### [4.6.8.18 Link type "preconnect"](https://html.spec.whatwg.org/#link-type-preconnect)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)

- [x] [preconnect](https://html.spec.whatwg.org/#preconnect) — To preconnect given a link processing options options:
  - 1. If options's href is an empty string, return.
  - 2. Let url be the result of encoding-parsing a URL given options's href, relative to options's base URL.
  - 3. If url is failure, then return.
  - 4. If url's scheme is not an HTTP(S) scheme, then return.
  - 5. Let partitionKey be the result of determining the network partition key given options's environment.
  - 6. Let useCredentials be true.
  - 7. If options's crossorigin is Anonymous and options's origin does not have the same origin as url's origin, then set useCredentials to false.
  - 8. The user agent should obtain a connection given partitionKey, url's origin, and useCredentials. The user agent should attempt to initiate a preconnect and perform the full connection handshake (DNS+TCP for HTTP, and DNS+TCP+TLS for HTTPS origins) whenever possible, but is allowed to elect to perform a partial handshake (DNS only for HTTP, and DNS or DNS+TCP for HTTPS origins), or skip it entirely…

##### [4.6.8.19 Link type "prefetch"](https://html.spec.whatwg.org/#link-type-prefetch)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)

- [x] [The fetch and process the linked resource algorithm for prefetch links, given a…](https://html.spec.whatwg.org/#fetch-and-process-the-linked-resource) — The fetch and process the linked resource algorithm for prefetch links, given a link element el, is as follows:
  - 1. If el's href attribute's value is the empty string, then return.
  - 2. Let options be the result of creating link options from el.
  - 3. Let request be the result of creating a link request given options.
  - 4. If request is null, then return.
  - 5. Set request's initiator to "prefetch".
  - 6. Let processPrefetchResponse be the following steps given a response response and null, failure, or a byte sequence bytesOrNull:
    - 6a. If response is a network error, fire an event named error at el.
    - 6b. Otherwise, fire an event named load at el.
  - 7. The user agent should fetch request, with processResponseConsumeBody set to processPrefetchResponse. User agents may delay the fetching of request to prioritize other requests that are necessary for the current document.

##### [4.6.8.20 Link type "preload"](https://html.spec.whatwg.org/#link-type-preload)

Status: n/a (no network loads; see `KNOWN-ISSUES.md`)
<!-- definitions: map-of-preloaded-resources preload-key preload-url preload-key-destination preload-mode preload-credentials-mode preload-entry preload-integrity-metadata preload-response preload-on-response-available preload-destination -->

- [x] [consume a preloaded resource](https://html.spec.whatwg.org/#consume-a-preloaded-resource) — To consume a preloaded resource for Window window, given a URL url, a string destination, a string mode, a string credentialsMode, a string integrityMetadata, and onResponseAvailable, which is an algorithm accepting a response:
  - 1. Let key be a preload key whose URL is url, destination is destination, mode is mode, and credentials mode is credentialsMode.
  - 2. Let preloads be window's associated Document's map of preloaded resources.
  - 3. If key does not exist in preloads, then return false.
  - 4. Let entry be preloads[key].
  - 5. Let consumerIntegrityMetadata be the result of parsing integrityMetadata.
  - 6. Let preloadIntegrityMetadata be the result of parsing entry's integrity metadata.
  - 7. If none of the following conditions apply: consumerIntegrityMetadata is no metadata;; consumerIntegrityMetadata is equal to preloadIntegrityMetadata; or then return false.
  - 8. Remove preloads[key].
  - 9. If entry's response is null, then set entry's on response available to onResponseAvailable.
  - 10. Otherwise, call onResponseAvailable with entry's response.
  - 11. Return true.

- [x] [matches](https://html.spec.whatwg.org/#match-preload-type) — For the purposes of this section, a string type matches a preload destination destination if the following algorithm returns true:
  - 1. If type is an empty string, then return true.
  - 2. If destination is "fetch", then return true.
  - 3. Let mimeTypeRecord be the result of parsing type.
  - 4. If mimeTypeRecord is failure, then return false.
  - 5. If mimeTypeRecord is not supported by the user agent, then return false.
  - 6. If any of the following are true: destination is "script" and mimeTypeRecord is a JavaScript MIME type;; destination is "image" and mimeTypeRecord is an image MIME type;; destination is "font" and mimeTypeRecord is a font MIME type;; destination is "style" and mimeTypeRecord's essence is text/css; or; destination is "track" and mimeTypeRecord's essence is text/vtt, then return true.
  - 7. Return false.

- [x] [create a preload key](https://html.spec.whatwg.org/#create-a-preload-key) — To create a preload key for a request request, return a new preload key whose URL is request's URL, destination is request's destination, mode is request's mode, and credentials mode is request's credentials mode.

- [x] [translate a preload destination](https://html.spec.whatwg.org/#translate-a-preload-destination) — To translate a preload destination given a string destination:
  - 1. If destination is not a preload destination, then return null.
  - 2. Return the result of translating destination.

- [x] [preload](https://html.spec.whatwg.org/#preload) — To preload given a link processing options options and an optional processResponse, which is an algorithm accepting a response:
  - 1. If options's type doesn't match options's destination, then return.
  - 2. If options's destination is "image" and options's source set is not null, then set options's href to the result of selecting an image source from options's source set.
  - 3. Let request be the result of creating a link request given options.
  - 4. If request is null, then return.
  - 5. Let unsafeEndTime be 0.
  - 6. Let entry be a new preload entry whose integrity metadata is options's integrity.
  - 7. Let key be the result of creating a preload key given request.
  - 8. If options's document is null, then set request's initiator type to "early hint".
  - 9. Let controller be null.
  - 10. Let reportTiming given a Document document be to report timing for controller given document's relevant global object.
  - 11. Set controller to the result of fetching request, with processResponseConsumeBody set to the following steps given a response response and null, failure, or a byte sequence bodyBytes:
    - 11a. If bodyBytes is a byte sequence, then set response's body to bodyBytes as a body.
    - 11b. Otherwise, set response to a network error.
    - 11c. Set unsafeEndTime to the unsafe shared current time.
    - 11d. If options's document is not null, then call reportTiming given options's document.
    - 11e. If entry's on response available is null, then set entry's response to response; otherwise call entry's on response available given response.
    - 11f. If processResponse is given, then call processResponse with response.
  - 12. Let commit be the following steps given a Document document:
    - 12a. If entry's response is not null, then call reportTiming given document.
    - 12b. Set document's map of preloaded resources[key] to entry.
  - 13. If options's document is null, then set options's on document ready to commit. Otherwise, call commit with options's document.

- [x] [The fetch and process the linked resource steps for this type of linked resourc…](https://html.spec.whatwg.org/#fetch-and-process-the-linked-resource) — The fetch and process the linked resource steps for this type of linked resource, given a link element el, are:
  - 1. Update the source set for el.
  - 2. Let options be the result of creating link options from el.
  - 3. Let destination be the result of translating the keyword representing the state of el's as attribute.
  - 4. If destination is null, then return.
  - 5. Set options's destination to destination.
  - 6. Preload options, with the following steps given a response response:
    - 6a. If response is a network error, fire an event named error at el. Otherwise, fire an event named load at el.

##### [4.6.8.21 Link type "privacy-policy"](https://html.spec.whatwg.org/#link-type-privacy-policy)

Status: n/a (author conformance)

##### [4.6.8.22 Link type "search"](https://html.spec.whatwg.org/#link-type-search)

Status: n/a (author conformance)

##### [4.6.8.23 Link type "stylesheet"](https://html.spec.whatwg.org/#link-type-stylesheet)

Status: reviewed (2026-11-01)
> `HTMLLinkElement::Process` (the fetch setup steps) and `SetCSSStyleSheet` with `NotifyLoadedSheetAndAllCriticalSubresources` (processing it). There's no Content-Type to check, and no scripts for script-blocking style sheets to block.

- [x] [The linked resource fetch setup steps for this type of linked resource, given a…](https://html.spec.whatwg.org/#linked-resource-fetch-setup-steps) — The linked resource fetch setup steps for this type of linked resource, given a link element el and request request, are:
  - 1. If el's disabled attribute is set, then return false.
  - 2. If el contributes a script-blocking style sheet, append el to its node document's script-blocking style sheet set.
  - 3. If el's media attribute's value matches the environment and el is potentially render-blocking, then block rendering on el.
  - 4. If el is currently render-blocking, then set request's render-blocking to true.
  - 5. Return true.

- [x] [To process this type of linked resource given a link element el, boolean succes…](https://html.spec.whatwg.org/#the-link-is-an-alternative-stylesheet) — To process this type of linked resource given a link element el, boolean success, response response, and byte sequence bodyBytes:
  - 1. If the resource's Content-Type metadata is not text/css, then set success to false.
  - 2. If el no longer creates an external resource link that contributes to the styling processing model, or if, since the resource in question was fetched, it has become appropriate to fetch it again:
    - 2a. Remove el from el's node document's script-blocking style sheet set.
    - 2b. Return.
  - 3. If el has an associated CSS style sheet, remove the CSS style sheet.
  - 4. If success is true:
    - 4a. Create a CSS style sheet with the following properties: typetext/csslocation response's URL list[0] owner nodeelmedia The media attribute of el. title The title attribute of el, if el is in a document tree, or the empty string otherwise. alternate flagSet if the link is an alternative style sheet and el's explicitly enabled is false; unset otherwise.origin-clean flagSet if the resource is CORS-sa…
      - 4a.i. If el has a charset attribute, get an encoding from that attribute's value. If that succeeds, return the resulting encoding. [ENCODING]
      - 4a.ii. Otherwise, return the document's character encoding. [DOM]
    - 4b. Fire an event named load at el.
  - 5. Otherwise, fire an event named error at el.
  - 6. If el contributes a script-blocking style sheet:
    - 6a. Assert: el's node document's script-blocking style sheet set contains el.
    - 6b. Remove el from its node document's script-blocking style sheet set.
  - 7. Unblock rendering on el.

##### [4.6.8.24 Link type "tag"](https://html.spec.whatwg.org/#link-type-tag)

Status: n/a (author conformance)

##### [4.6.8.25 Link Type "terms-of-service"](https://html.spec.whatwg.org/#link-type-terms-of-service)

Status: n/a (author conformance)

##### [4.6.8.26 Sequential link types](https://html.spec.whatwg.org/#sequential-link-types)

Status: n/a (author conformance)

###### [4.6.8.26.1 Link type "next"](https://html.spec.whatwg.org/#link-type-next)

Status: n/a (author conformance)

###### [4.6.8.26.2 Link type "prev"](https://html.spec.whatwg.org/#link-type-prev)

Status: n/a (author conformance)

##### [4.6.8.27 Other link types](https://html.spec.whatwg.org/#other-link-types)

Status: n/a (author conformance)
<!-- definitions: concept-rel-extensions -->
