# HTML review checklist: 8.10 System state and capabilities

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 45 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html system-state` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [8.10 System state and capabilities](https://html.spec.whatwg.org/#system-state-and-capabilities)

Status: reviewed (2026-11-01)
> The sections below.

#### [8.10.1 The Navigator object](https://html.spec.whatwg.org/#the-navigator-object)

Status: reviewed (2026-11-01)
> `Navigator`, made when `LocalDOMWindow::GetNavigator` is first called; `ClientInformation` is the same object.
<!-- definitions: dom-clientinformation -->

- [x] [associated Navigator](https://html.spec.whatwg.org/#associated-navigator) — Each Window has an associated Navigator, which is a Navigator object. Upon creation of the Window object, its associated Navigator must be set to a new Navigator object created in the Window object's relevant realm.

- [x] [Window.navigator](https://html.spec.whatwg.org/#dom-navigator) — The navigator and clientInformation getter steps are to return this's associated Navigator.

##### [8.10.1.1 Client identification](https://html.spec.whatwg.org/#client-identification)

Status: reviewed (2026-11-01)
> `Navigator`'s NavigatorID members, with the host's user agent string (`LocalFrameLoaderClient::UserAgent`). `platform` and `vendor` don't follow a compatibility mode (see `DIFFERENCES.md`); there's no `oscpu`, which is Gecko's.
<!-- definitions: concept-navigator-compatibility-mode -->

- [x] [NavigatorID.appCodeName](https://html.spec.whatwg.org/#dom-navigator-appcodename) — The appCodeName getter steps are to return "Mozilla".

- [x] [NavigatorID.appName](https://html.spec.whatwg.org/#dom-navigator-appname) — The appName getter steps are to return "Netscape".

- [x] [NavigatorID.appVersion](https://html.spec.whatwg.org/#dom-navigator-appversion) — The appVersion getter steps are:
  - 1. Let userAgent be this's relevant settings object's environment default `User-Agent` value.
  - 2. If userAgent does not start with `Mozilla/5.0 (`, then return the empty string.
  - 3. Let trail be the substring of userAgent, isomorphic decoded, that follows the "Mozilla/" prefix.
  - 4. If the navigator compatibility mode is Chrome or WebKitReturn trail.If the navigator compatibility mode is Gecko If trail starts with "5.0 (Windows", then return "5.0 (Windows)". Otherwise, return the prefix of trail up to but not including the first U+003B (;), concatenated with the character U+0029 RIGHT PARENTHESIS. For example, "5.0 (Macintosh)", "5.0 (Android 10)", or "5.0 (X11)".

- [x] [NavigatorID.platform](https://html.spec.whatwg.org/#dom-navigator-platform) — The platform getter steps are to return a string representing the platform on which the browser is executing (e.g. "MacIntel", "Win32", "Linux x86_64", "Linux armv81") or, for privacy and compatibility, a string that is commonly returned on another platform.

- [x] [NavigatorID.product](https://html.spec.whatwg.org/#dom-navigator-product) — The product getter steps are to return "Gecko".

- [x] [NavigatorID.productSub](https://html.spec.whatwg.org/#dom-navigator-productsub) — The productSub getter steps are to return the appropriate string from the following list:
  - • If the navigator compatibility mode is Chrome or WebKit: The string "20030107".
  - • If the navigator compatibility mode is Gecko: The string "20100101".

- [x] [NavigatorID.userAgent](https://html.spec.whatwg.org/#dom-navigator-useragent) — The userAgent getter steps are to return this's relevant settings object's environment default `User-Agent` value.

- [x] [NavigatorID.vendor](https://html.spec.whatwg.org/#dom-navigator-vendor) — The vendor getter steps are to return the appropriate string from the following list:
  - • If the navigator compatibility mode is Chrome: The string "Google Inc.".
  - • If the navigator compatibility mode is Gecko: The empty string.
  - • If the navigator compatibility mode is WebKit: The string "Apple Computer, Inc.".

- [x] [NavigatorID.vendorSub](https://html.spec.whatwg.org/#dom-navigator-vendorsub) — The vendorSub getter steps are to return the empty string.

- [x] [NavigatorID.taintEnabled()](https://html.spec.whatwg.org/#dom-navigator-taintenabled) — The taintEnabled() method must return false.

- [x] [NavigatorID.oscpu](https://html.spec.whatwg.org/#dom-navigator-oscpu) — The oscpu attribute's getter must return either the empty string or a string representing the platform on which the browser is executing, e.g. "Windows NT 10.0; Win64; x64", "Linux x86_64".

##### [8.10.1.2 Language preferences](https://html.spec.whatwg.org/#language-preferences)

Status: reviewed (2026-11-01)
> `Navigator::Language` and `Languages` (only the most preferred language, see `DIFFERENCES.md`); no WebDriver BiDi emulation.

- [x] [NavigatorLanguage.language](https://html.spec.whatwg.org/#dom-navigator-language) — The language getter steps are:
  - 1. Let emulatedLanguage be the WebDriver BiDi emulated language for this's relevant settings object.
  - 2. If emulatedLanguage is not null, return emulatedLanguage.
  - 3. Return a valid BCP 47 language tag representing either a plausible language or the user's most preferred language. [BCP47]

- [x] [NavigatorLanguage.languages](https://html.spec.whatwg.org/#dom-navigator-languages) — The languages getter steps are:
  - 1. Let emulatedLanguage be the WebDriver BiDi emulated language for this's relevant settings object.
  - 2. If emulatedLanguage is not null, return a frozen array containing emulatedLanguage.
  - 3. Return a frozen array of valid BCP 47 language tags representing either one or more plausible languages, or the user's preferred languages, ordered by preference with the most preferred language first. [BCP47]

- [x] [a plausible language](https://html.spec.whatwg.org/#a-plausible-language) — To determine a plausible language, the user agent should bear in mind the following:

##### [8.10.1.3 Browser state](https://html.spec.whatwg.org/#navigator.online)

Status: reviewed (2026-11-01)
> `Navigator::OnLine` asks the loader strategy; `Page::NetworkStateChanged` fires `online`/`offline`.

- [x] [NavigatorOnLine.onLine](https://html.spec.whatwg.org/#dom-navigator-online) — The onLine getter steps are to return true if this's relevant settings object is offline is false; otherwise false.

##### [8.10.1.4 Custom scheme handlers: the registerProtocolHandler() method](https://html.spec.whatwg.org/#custom-handlers)

Status: reviewed (2026-11-01)
> `Navigator::RegisterProtocolHandler`/`UnregisterProtocolHandler` pass the normalized parameters to the host's `ChromeClient`, which registers and uses handlers. Available outside secure contexts (see `DIFFERENCES.md`).
<!-- definitions: protocol-handler-registration rph-automation-mode-none rph-automation-mode-auto-accept rph-automation-mode-auto-reject protocol-handler-invocation safelisted-scheme -->

- [x] [NavigatorContentUtils.registerProtocolHandler(scheme, url)](https://html.spec.whatwg.org/#dom-navigator-registerprotocolhandler) — The registerProtocolHandler(scheme, url) method steps are:
  - 1. Let (normalizedScheme, normalizedURLString) be the result of running normalize protocol handler parameters with scheme, url, and this's relevant settings object.
  - 2. In parallel: register a protocol handler for normalizedScheme and normalizedURLString. User agents may, within the constraints described, do whatever they like. A user agent could, for instance, prompt the user and offer the user the opportunity to add the site to a shortlist of handlers, or make the handlers their default, or cancel the request. User agents could also silently collect the inform…
    - 2a. Assert: inputURL's scheme is normalizedScheme.
    - 2b. Set the username given inputURL and the empty string.
    - 2c. Set the password given inputURL and the empty string.
    - 2d. Let inputURLString be the serialization of inputURL.
    - 2e. Let encodedURL be the result of running UTF-8 percent-encode on inputURLString using the component percent-encode set.
    - 2f. Let handlerURLString be normalizedURLString.
    - 2g. Replace the first instance of "%s" in handlerURLString with encodedURL.
    - 2h. Let resultURL be the result of parsing handlerURLString.
    - 2i. Navigate an appropriate navigable to resultURL.

- [x] [NavigatorContentUtils.unregisterProtocolHandler(scheme, url)](https://html.spec.whatwg.org/#dom-navigator-unregisterprotocolhandler) — The unregisterProtocolHandler(scheme, url) method steps are:
  - 1. Let (normalizedScheme, normalizedURLString) be the result of running normalize protocol handler parameters with scheme, url, and this's relevant settings object.
  - 2. In parallel: unregister the handler described by normalizedScheme and normalizedURLString.

- [x] [normalize protocol handler parameters](https://html.spec.whatwg.org/#normalize-protocol-handler-parameters) — To normalize protocol handler parameters, given a string scheme, a string url, and an environment settings object environment, run these steps:
  - 1. Set scheme to scheme, converted to ASCII lowercase.
  - 2. If scheme is neither a safelisted scheme nor a string starting with "web+" followed by one or more ASCII lower alphas, then throw a "SecurityError" DOMException. The following schemes are the safelisted schemes: bitcoin; ftp; ftps; geo; im; irc; ircs; magnet; mailto; matrix; mms; news; nntp; openpgp4fpr; sftp; sip; sms; smsto; ssh; tel; urn; webcal; wtai; xmpp
  - 3. If url does not contain "%s", then throw a "SyntaxError" DOMException.
  - 4. Let urlRecord be the result of encoding-parsing a URL given url, relative to environment.
  - 5. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 6. If urlRecord's scheme is not an HTTP(S) scheme or urlRecord's origin is not same origin with environment's origin, then throw a "SecurityError" DOMException.
  - 7. Assert: the result of Is url potentially trustworthy? given urlRecord is "Potentially Trustworthy".
  - 8. Return (scheme, urlRecord).

###### [8.10.1.4.1 Security and privacy](https://html.spec.whatwg.org/#security-and-privacy)

Status: reviewed (2026-11-01)
> No requirements beyond the host's.

###### [8.10.1.4.2 User agent automation](https://html.spec.whatwg.org/#user-agent-automation)

Status: n/a (no WebDriver)
<!-- definitions: registerprotocolhandler()-automation-mode -->

- [x] [The remote end steps](https://html.spec.whatwg.org/#set-rph-registration-mode) — The remote end steps are:
  - 1. If parameters is not a JSON Object, return a WebDriver error with WebDriver error code invalid argument.
  - 2. Let mode be the result of getting a property named "mode" from parameters.
  - 3. If mode is not "autoAccept", "autoReject", or "none", return a WebDriver error with WebDriver error code invalid argument.
  - 4. Let document be the current browsing context's active document.
  - 5. Set document's registerProtocolHandler() automation mode to mode.
  - 6. Return success with data null.

##### [8.10.1.5 Cookies](https://html.spec.whatwg.org/#cookies)

Status: reviewed (2026-11-01)
> `Navigator::CookieEnabled` is false: there are no cookies.

- [x] [NavigatorCookies.cookieEnabled](https://html.spec.whatwg.org/#dom-navigator-cookieenabled) — The cookieEnabled attribute must return true if the user agent attempts to handle cookies according to HTTP State Management Mechanism, and false if it ignores cookie change requests. [COOKIES]

##### [8.10.1.6 PDF viewing support](https://html.spec.whatwg.org/#pdf-viewing-support)

Status: reviewed (2026-11-01)
> There's no PDF viewer, so `pdfViewerEnabled` is false and `DOMPluginArray`/`DOMMimeTypeArray` are empty; no `Plugin` or `MimeType` objects are ever made.
<!-- definitions: pdf-viewer-supported pdf-viewer-plugin-names pdf-viewer-mime-types plugins-array mime-types-array concept-plugin-name concept-mimetype-type -->

- [x] [PDF viewer plugin objects](https://html.spec.whatwg.org/#pdf-viewer-plugin-objects) — Each Window object has a PDF viewer plugin objects list. If the user agent's PDF viewer supported is false, then it is the empty list. Otherwise, it is a list containing five Plugin objects, whose names are, respectively:
  - 0. "PDF Viewer"
  - 1. "Chrome PDF Viewer"
  - 2. "Chromium PDF Viewer"
  - 3. "Microsoft Edge PDF Viewer"
  - 4. "WebKit built-in PDF"

- [x] [PDF viewer mime type objects](https://html.spec.whatwg.org/#pdf-viewer-mime-type-objects) — Each Window object has a PDF viewer mime type objects list. If the user agent's PDF viewer supported is false, then it is the empty list. Otherwise, it is a list containing two MimeType objects, whose types are, respectively:
  - 0. "application/pdf"
  - 1. "text/pdf"

- [x] [NavigatorPlugins.plugins](https://html.spec.whatwg.org/#dom-navigator-plugins) — The NavigatorPlugins mixin's plugins getter steps are to return this's plugins array.

- [x] [NavigatorPlugins.mimeTypes](https://html.spec.whatwg.org/#dom-navigator-mimetypes) — The NavigatorPlugins mixin's mimeTypes getter steps are to return this's mime types array.

- [x] [NavigatorPlugins.javaEnabled()](https://html.spec.whatwg.org/#dom-navigator-javaenabled) — The NavigatorPlugins mixin's javaEnabled() method steps are to return false.

- [x] [NavigatorPlugins.pdfViewerEnabled](https://html.spec.whatwg.org/#dom-navigator-pdfviewerenabled) — The NavigatorPlugins mixin's pdfViewerEnabled getter steps are to return the user agent's PDF viewer supported.

- [x] [PluginArray.namedItem(name)](https://html.spec.whatwg.org/#dom-pluginarray-nameditem) — The PluginArray interface's namedItem(name) method steps are:
  - 1. For each Plugin plugin of this's relevant global object's PDF viewer plugin objects: if plugin's name is name, then return plugin.
  - 2. Return null.

- [x] [PluginArray.item(index)](https://html.spec.whatwg.org/#dom-pluginarray-item) — The PluginArray interface's item(index) method steps are:
  - 1. Let plugins be this's relevant global object's PDF viewer plugin objects.
  - 2. If index < plugins's size, then return plugins[index].
  - 3. Return null.

- [x] [PluginArray.length](https://html.spec.whatwg.org/#dom-pluginarray-length) — The PluginArray interface's length getter steps are to return this's relevant global object's PDF viewer plugin objects's size.

- [x] [PluginArray.refresh()](https://html.spec.whatwg.org/#dom-pluginarray-refresh) — The PluginArray interface's refresh() method steps are to do nothing.

- [x] [MimeTypeArray.namedItem(name)](https://html.spec.whatwg.org/#dom-mimetypearray-nameditem) — The MimeTypeArray interface's namedItem(name) method steps are:
  - 1. For each MimeType mimeType of this's relevant global object's PDF viewer mime type objects: if mimeType's type is name, then return mimeType.
  - 2. Return null.

- [x] [MimeTypeArray.item(index)](https://html.spec.whatwg.org/#dom-mimetypearray-item) — The MimeTypeArray interface's item(index) method steps are:
  - 1. Let mimeTypes be this's relevant global object's PDF viewer mime type objects.
  - 2. If index < mimeTypes's size, then return mimeTypes[index].
  - 3. Return null.

- [x] [MimeTypeArray.length](https://html.spec.whatwg.org/#dom-mimetypearray-length) — The MimeTypeArray interface's length getter steps are to return this's relevant global object's PDF viewer mime type objects's size.

- [x] [Plugin.name](https://html.spec.whatwg.org/#dom-plugin-name) — The Plugin interface's name getter steps are to return this's name.

- [x] [Plugin.description](https://html.spec.whatwg.org/#dom-plugin-description) — The Plugin interface's description getter steps are to return "Portable Document Format".

- [x] [Plugin.filename](https://html.spec.whatwg.org/#dom-plugin-filename) — The Plugin interface's filename getter steps are to return "internal-pdf-viewer".

- [x] [Plugin.namedItem(name)](https://html.spec.whatwg.org/#dom-plugin-nameditem) — The Plugin interface's namedItem(name) method steps are:
  - 1. For each MimeType mimeType of this's relevant global object's PDF viewer mime type objects: if mimeType's type is name, then return mimeType.
  - 2. Return null.

- [x] [Plugin.item(index)](https://html.spec.whatwg.org/#dom-plugin-item) — The Plugin interface's item(index) method steps are:
  - 1. Let mimeTypes be this's relevant global object's PDF viewer mime type objects.
  - 2. If index < mimeTypes's size, then return mimeTypes[index].
  - 3. Return null.

- [x] [Plugin.length](https://html.spec.whatwg.org/#dom-plugin-length) — The Plugin interface's length getter steps are to return this's relevant global object's PDF viewer mime type objects's size.

- [x] [MimeType.type](https://html.spec.whatwg.org/#dom-mimetype-type) — The MimeType interface's type getter steps are to return this's type.

- [x] [MimeType.description](https://html.spec.whatwg.org/#dom-mimetype-description) — The MimeType interface's description getter steps are to return "Portable Document Format".

- [x] [MimeType.suffixes](https://html.spec.whatwg.org/#dom-mimetype-suffixes) — The MimeType interface's suffixes getter steps are to return "pdf".

- [x] [MimeType.enabledPlugin](https://html.spec.whatwg.org/#dom-mimetype-enabledplugin) — The MimeType interface's enabledPlugin getter steps are to return this's relevant global object's PDF viewer plugin objects[0] (i.e., the generic "PDF Viewer" one).
