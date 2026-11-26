# cssom-1 review checklist

Generated from <https://drafts.csswg.org/cssom-1/> on 2026-11-22 by `Krystal.Booey/tools/spec_checklist.py`: 105 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py cssom-1` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[cssom-1:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py cssom-1 --tick <section anchor>` ticks a section's algorithms and sets its status.

## [Abstract](https://drafts.csswg.org/cssom-1/#abstract)

Status: unreviewed

## [Status of this document](https://drafts.csswg.org/cssom-1/#sotd)

Status: unreviewed

## [1. Introduction](https://drafts.csswg.org/cssom-1/#introduction)

Status: unreviewed

## [2. Terminology](https://drafts.csswg.org/cssom-1/#terminology)

Status: unreviewed
<!-- definitions: set unset supported-styling-language supported-css-property -->

### [2.1. Common Serializing Idioms](https://drafts.csswg.org/cssom-1/#common-serializing-idioms)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/Markup.cpp (identifiers, strings, URLs; SerialiseName for unrestricted hash names); serialize a LOCAL in Css/Values/FontFaceSrcValue.cpp; the list idioms are inline where lists are serialised.
<!-- definitions: css-serialize-a-function -->

- [x] [escape a character](https://drafts.csswg.org/cssom-1/#escape-a-character) — To escape a character means to create a string of "\" (U+005C), followed by the character.

- [x] [escape a character as code point](https://drafts.csswg.org/cssom-1/#escape-a-character-as-code-point) — To escape a character as code point means to create a string of "\" (U+005C), followed by the Unicode code point as the smallest possible number of hexadecimal digits in the range 0-9 a-f (U+0030 to U+0039 and U+0061 to U+0066) to represent the code point in base 16, followed by a single SPACE (U+0020).

- [x] [serialize an identifier](https://drafts.csswg.org/cssom-1/#serialize-an-identifier) — To serialize an identifier means to create a string represented by the concatenation of, for each character of the identifier:

- [x] [serialize a string](https://drafts.csswg.org/cssom-1/#serialize-a-string) — To serialize a string means to create a string represented by '"' (U+0022), followed by the result of applying the rules below to each character of the given string, followed by '"' (U+0022):

- [x] [serialize a URL](https://drafts.csswg.org/cssom-1/#serialize-a-url) — To serialize a URL means to create a string represented by "url(", followed by the serialization of the URL as a string, followed by ")".

- [x] [serialize a LOCAL](https://drafts.csswg.org/cssom-1/#serialize-a-local) — To serialize a LOCAL means to create a string represented by "local(", followed by the serialization of the LOCAL as a string, followed by ")".

- [x] [serialize a comma-separated list](https://drafts.csswg.org/cssom-1/#serialize-a-comma-separated-list) — To serialize a comma-separated list concatenate all items of the list in list order while separating them by ", ", i.e., COMMA (U+002C) followed by a single SPACE (U+0020).

- [x] [serialize a whitespace-separated list](https://drafts.csswg.org/cssom-1/#serialize-a-whitespace-separated-list) — To serialize a whitespace-separated list concatenate all items of the list in list order while separating them by " ", i.e., a single SPACE (U+0020).

## [3. CSSOMString](https://drafts.csswg.org/cssom-1/#cssomstring-type)

Status: unreviewed
<!-- definitions: cssomstring -->

## [4. Media Queries](https://drafts.csswg.org/cssom-1/#media-queries)

Status: unreviewed

### [4.1. Parsing Media Queries](https://drafts.csswg.org/cssom-1/#parsing-media-queries)

Status: unreviewed

- [ ] [parse a media query list](https://drafts.csswg.org/cssom-1/#parse-a-media-query-list) — To parse a media query list for a given string s into a media query list is defined in the Media Queries specification. Return the list of media queries that the algorithm defined there gives.

- [ ] [parse a media query](https://drafts.csswg.org/cssom-1/#parse-a-media-query) — To parse a media query for a given string s means to follow the parse a media query list steps and return null if more than one media query is returned or a media query if a single media query is returned.

### [4.2. Serializing Media Queries](https://drafts.csswg.org/cssom-1/#serializing-media-queries)

Status: unreviewed

- [ ] [serialize a media query list](https://drafts.csswg.org/cssom-1/#serialize-a-media-query-list) — To serialize a media query list run these steps:
  - 1. If the media query list is empty, then return the empty string.
  - 2. Serialize each media query in the list of media queries, in the same order as they appear in the media query list, and then serialize the list.

- [ ] [serialize a media query](https://drafts.csswg.org/cssom-1/#serialize-a-media-query) — To serialize a media query let s be the empty string, run the steps below:
  - 1. If the media query is negated append "not", followed by a single SPACE (U+0020), to s.
  - 2. Let type be the serialization as an identifier of the media type of the media query, converted to ASCII lowercase.
  - 3. If the media query does not contain media features append type, to s, then return s.
  - 4. If type is not "all" or if the media query is negated append type, followed by a single SPACE (U+0020), followed by "and", followed by a single SPACE (U+0020), to s.
  - 5. Then, for each media feature:
    - 5a. Append a "(" (U+0028), followed by the media feature name, converted to ASCII lowercase, to s.
    - 5b. If a value is given append a ":" (U+003A), followed by a single SPACE (U+0020), followed by the serialized media feature value, to s.
    - 5c. Append a ")" (U+0029) to s.
    - 5d. If this is not the last media feature append a single SPACE (U+0020), followed by "and", followed by a single SPACE (U+0020), to s.
  - 6. Return s.

#### [4.2.1. Serializing Media Feature Values](https://drafts.csswg.org/cssom-1/#serializing-media-feature-values)

Status: unreviewed

- [ ] [serialize a media feature value](https://drafts.csswg.org/cssom-1/#serialize-a-media-feature-value) — To serialize a media feature value named v locate v in the first column of the table below and use the serialization format described in the second column:

### [4.3. Comparing Media Queries](https://drafts.csswg.org/cssom-1/#comparing-media-queries)

Status: unreviewed

- [ ] [compare media queries](https://drafts.csswg.org/cssom-1/#compare-media-queries) — To compare media queries m1 and m2 means to serialize them both and return true if they are a case-sensitive match and false if they are not.

### [4.4. The MediaList Interface](https://drafts.csswg.org/cssom-1/#the-medialist-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/MediaList.cpp; create a MediaList object is MediaList::Create plus SetMediaText.
<!-- definitions: medialist-collection-of-media-queries -->

- [x] [create a MediaList object](https://drafts.csswg.org/cssom-1/#create-a-medialist-object) — To create a MediaList object with a string text, run the following steps:
  - 1. Create a new MediaList object.
  - 2. Set its mediaText attribute to text.
  - 3. Return the newly created MediaList object.

- [x] [MediaList.mediaText](https://drafts.csswg.org/cssom-1/#dom-medialist-mediatext) — The mediaText attribute, on getting, must return a serialization of the collection of media queries. Setting the mediaText attribute must run these steps:
  - 1. Empty the collection of media queries.
  - 2. If the given value is the empty string, then return.
  - 3. Append all the media queries as a result of parsing the given value to the collection of media queries.

- [x] [MediaList.item(index)](https://drafts.csswg.org/cssom-1/#dom-medialist-item) — The item(index) method must return a serialization of the media query in the collection of media queries given by index, or null, if index is greater than or equal to the number of media queries in the collection of media queries.

- [x] [MediaList.length](https://drafts.csswg.org/cssom-1/#dom-medialist-length) — The length attribute must return the number of media queries in the collection of media queries.

- [x] [MediaList.appendMedium(medium)](https://drafts.csswg.org/cssom-1/#dom-medialist-appendmedium) — The appendMedium(medium) method must run these steps:
  - 1. Let m be the result of parsing the given value.
  - 2. If m is null, then return.
  - 3. If comparing m with any of the media queries in the collection of media queries returns true, then return.
  - 4. Append m to the collection of media queries.

- [x] [MediaList.deleteMedium(medium)](https://drafts.csswg.org/cssom-1/#dom-medialist-deletemedium) — The deleteMedium(medium) method must run these steps:
  - 1. Let m be the result of parsing the given value.
  - 2. If m is null, then return.
  - 3. Remove any media query from the collection of media queries for which comparing the media query with m returns true. If nothing was removed, then throw a NotFoundError exception.

## [5. Selectors](https://drafts.csswg.org/cssom-1/#selectors)

Status: unreviewed

### [5.1. Parsing Selectors](https://drafts.csswg.org/cssom-1/#parsing-selectors)

Status: unreviewed

- [ ] [parse a group of selectors](https://drafts.csswg.org/cssom-1/#parse-a-group-of-selectors) — To parse a group of selectors means to parse the value using the selectors_group production defined in the Selectors specification and return either a group of selectors if parsing did not fail or null if parsing did fail.

### [5.2. Serializing Selectors](https://drafts.csswg.org/cssom-1/#serializing-selectors)

Status: unreviewed

- [ ] [serialize a group of selectors](https://drafts.csswg.org/cssom-1/#serialize-a-group-of-selectors) — To serialize a group of selectors serialize each selector in the group of selectors and then serialize a comma-separated list of these serializations.

- [ ] [serialize a selector](https://drafts.csswg.org/cssom-1/#serialize-a-selector) — To serialize a selector let s be the empty string, run the steps below for each part of the chain of the selector, and finally return s:
  - 1. If there is only one simple selector in the compound selectors which is a universal selector, append the result of serializing the universal selector to s.
  - 2. Otherwise, for each simple selector in the compound selectors that is not a universal selector of which the namespace prefix maps to a namespace that is not the default namespace serialize the simple selector and append the result to s.
  - 3. If this is not the last part of the chain of the selector append a single SPACE (U+0020), followed by the combinator ">", "+", "~", ">>", "||", as appropriate, followed by another single SPACE (U+0020) if the combinator was not whitespace, to s.
  - 4. If this is the last part of the chain of the selector and there is a pseudo-element, append "::" followed by the name of the pseudo-element, to s.

- [ ] [serialize a simple selector](https://drafts.csswg.org/cssom-1/#serialize-a-simple-selector) — To serialize a simple selector let s be the empty string, run the steps below, and finally return s:
  - • type selector / universal selector:
    - 1. If the namespace prefix maps to a namespace that is not the default namespace and is not the null namespace (not in a namespace) append the serialization of the namespace prefix as an identifier, followed by a "|" (U+007C) to s.
    - 2. If the namespace prefix maps to a namespace that is the null namespace (not in a namespace) append "|" (U+007C) to s.
    - 3. If this is a type selector append the serialization of the element name as an identifier to s.
    - 4. If this is a universal selector append "*" (U+002A) to s.
  - • attribute selector:
    - 1. Append "[" (U+005B) to s.
    - 2. If the namespace prefix maps to a namespace that is not the null namespace (not in a namespace) append the serialization of the namespace prefix as an identifier, followed by a "|" (U+007C) to s.
    - 3. Append the serialization of the attribute name as an identifier to s.
    - 4. If there is an attribute value specified, append "=", "~=", "|=", "^=", "$=", or "*=" as appropriate (depending on the type of attribute selector), followed by the serialization of the attribute value as a string, to s.
    - 5. If the attribute selector has the case-sensitivity flag present, append " i" (U+0020 U+0069) to s.
    - 6. Append "]" (U+005D) to s.
  - • class selector: Append a "." (U+002E), followed by the serialization of the class name as an identifier to s.
  - • ID selector: Append a "#" (U+0023), followed by the serialization of the ID as an identifier to s.
  - • pseudo-class: If the pseudo-class does not accept arguments append ":" (U+003A), followed by the name of the pseudo-class, to s. Otherwise, append ":" (U+003A), followed by the name of the pseudo-class, followed by "(" (U+0028), followed by the value of the pseudo-class argument(s) determined as per below, followed by ")" (U+0029), to s. :lang() The serialization of a comma-separated list of each…

## [6. CSS](https://drafts.csswg.org/cssom-1/#css-object-model)

Status: unreviewed

### [6.1. CSS Style Sheets](https://drafts.csswg.org/cssom-1/#css-style-sheets)

Status: unreviewed
<!-- definitions: css-style-sheet dom-cssstylesheet-cssstylesheet create-a-constructed-cssstylesheet concept-css-style-sheet-type concept-css-style-sheet-location concept-css-style-sheet-parent-css-style-sheet concept-css-style-sheet-owner-node concept-css-style-sheet-owner-css-rule concept-css-style-sheet-media concept-css-style-sheet-title concept-css-style-sheet-alternate-flag concept-css-style-sheet-disabled-flag concept-css-style-sheet-css-rules concept-css-style-sheet-origin-clean-flag concept-css-style-sheet-constructed-flag concept-css-style-sheet-disallow-modification-flag concept-css-style-sheet-constructor-document concept-css-style-sheet-stylesheet-base-url -->

#### [6.1.1. The StyleSheet Interface](https://drafts.csswg.org/cssom-1/#the-stylesheet-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/include/Krystal.Booey/Css/CSSOM/StyleSheet.hpp, implemented by CSSStyleSheet.

- [x] [StyleSheet.type](https://drafts.csswg.org/cssom-1/#dom-stylesheet-type) — The type attribute must return the type.

- [x] [StyleSheet.href](https://drafts.csswg.org/cssom-1/#dom-stylesheet-href) — The href attribute must return the location.

- [x] [StyleSheet.ownerNode](https://drafts.csswg.org/cssom-1/#dom-stylesheet-ownernode) — The ownerNode attribute must return the owner node.

- [x] [StyleSheet.parentStyleSheet](https://drafts.csswg.org/cssom-1/#dom-stylesheet-parentstylesheet) — The parentStyleSheet attribute must return the parent CSS style sheet.

- [x] [StyleSheet.title](https://drafts.csswg.org/cssom-1/#dom-stylesheet-title) — The title attribute must return the title or null if title is the empty string.

- [x] [StyleSheet.media](https://drafts.csswg.org/cssom-1/#dom-stylesheet-media) — The media attribute must return the media.

- [x] [StyleSheet.disabled](https://drafts.csswg.org/cssom-1/#dom-stylesheet-disabled) — The disabled attribute, on getting, must return true if the disabled flag is set, or false otherwise. On setting, the disabled attribute must set the disabled flag if the new value is true, or unset the disabled flag otherwise.

#### [6.1.2. The CSSStyleSheet Interface](https://drafts.csswg.org/cssom-1/#the-cssstylesheet-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSStyleSheet.cpp; there's no replace() (no promises), so the disallow modification flag is never set, and every sheet is origin-clean unless created otherwise (there are no origins).

- [x] [CSSStyleSheet.ownerRule](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-ownerrule) — The ownerRule attribute must return the owner CSS rule. If a value other than null is ever returned, then that same value must always be returned on each get access.

- [x] [CSSStyleSheet.cssRules](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-cssrules) — The cssRules attribute must follow these steps:
  - 1. If the origin-clean flag is unset, throw a SecurityError exception.
  - 2. Return a read-only, live CSSRuleList object representing the CSS rules.

- [x] [CSSStyleSheet.insertRule(rule, index)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-insertrule) — The insertRule(rule, index) method must run the following steps:
  - 1. If the origin-clean flag is unset, throw a SecurityError exception.
  - 2. If the disallow modification flag is set, throw a NotAllowedError DOMException.
  - 3. Let parsed rule be the return value of invoking parse a rule with rule.
  - 4. If parsed rule is a syntax error, throw a SyntaxError DOMException.
  - 5. If parsed rule is an @import rule, and the constructed flag is set, throw a SyntaxError DOMException.
  - 6. Return the result of invoking insert a CSS rule rule in the CSS rules at index.

- [x] [CSSStyleSheet.deleteRule(index)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-deleterule) — The deleteRule(index) method must run the following steps:
  - 1. If the origin-clean flag is unset, throw a SecurityError exception.
  - 2. If the disallow modification flag is set, throw a NotAllowedError DOMException.
  - 3. Remove a CSS rule in the CSS rules at index.

- [x] [CSSStyleSheet.replace(text)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-replace) — The replace(text) method must run the following steps:
  - 1. Let promise be a promise.
  - 2. If the constructed flag is not set, or the disallow modification flag is set, reject promise with a NotAllowedError DOMException and return promise.
  - 3. Set the disallow modification flag.
  - 4. In parallel, do these steps:
    - 4a. Let rules be the result of running parse a stylesheet’s contents from text.
    - 4b. If rules contains one or more @import rules, remove those rules from rules.
    - 4c. Set sheet’s CSS rules to rules.
    - 4d. Unset sheet’s disallow modification flag.
    - 4e. Resolve promise with sheet.
  - 5. Return promise.

- [x] [CSSStyleSheet.replaceSync(text)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-replacesync) — The replaceSync(text) method must run the steps to synchronously replace the rules of a CSSStyleSheet on this CSSStyleSheet given text.

- [x] [synchronously replace the rules of a CSSStyleSheet](https://drafts.csswg.org/cssom-1/#synchronously-replace-the-rules-of-a-cssstylesheet) — To synchronously replace the rules of a CSSStyleSheet on sheet given text, run these steps:
  - 1. If the constructed flag is not set, or the disallow modification flag is set, throw a NotAllowedError DOMException.
  - 2. Let rules be the result of running parse a stylesheet’s contents from text.
  - 3. If rules contains one or more @import rules, remove those rules from rules.
  - 4. Set sheet’s CSS rules to rules.

##### [6.1.2.1. Deprecated CSSStyleSheet members](https://drafts.csswg.org/cssom-1/#legacy-css-style-sheet-members)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSStyleSheet.cpp.

- [x] [CSSStyleSheet.rules](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-rules) — The rules attribute must follow the same steps as cssRules, and return the same object cssRules would return.

- [x] [CSSStyleSheet.removeRule(index)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-removerule) — The removeRule(index) method must run the same steps as deleteRule().

- [x] [CSSStyleSheet.addRule(selector, block, optionalIndex)](https://drafts.csswg.org/cssom-1/#dom-cssstylesheet-addrule) — The addRule(selector, block, optionalIndex) method must run the following steps:
  - 1. Let rule be an empty string.
  - 2. Append selector to rule.
  - 3. Append " { " to rule.
  - 4. If block is not empty, append block, followed by a space, to rule.
  - 5. Append "}" to rule
  - 6. Let index be optionalIndex if provided, or the number of CSS rules in the stylesheet otherwise.
  - 7. Call insertRule(), with rule and index as arguments.
  - 8. Return -1.

### [6.2. CSS Style Sheet Collections](https://drafts.csswg.org/cssom-1/#css-style-sheet-collections)

Status: unreviewed
<!-- definitions: persistent-css-style-sheet css-style-sheet-set css-style-sheet-set-name enabled-css-style-sheet-set last-css-style-sheet-set-name preferred-css-style-sheet-set-name -->

- [ ] [document or shadow root CSS style sheets (DocumentOrShadowRoot)](https://drafts.csswg.org/cssom-1/#documentorshadowroot-document-or-shadow-root-css-style-sheets) — Each DocumentOrShadowRoot has an associated list of zero or more CSS style sheets, named the document or shadow root CSS style sheets. This is an ordered list that contains:
  - 1. Any CSS style sheets created from HTTP Link headers, in header order
  - 2. Any CSS style sheets associated with the DocumentOrShadowRoot, in tree order

- [ ] [final CSS style sheets (DocumentOrShadowRoot)](https://drafts.csswg.org/cssom-1/#documentorshadowroot-final-css-style-sheets) — Each DocumentOrShadowRoot has an associated list of zero or more CSS style sheets, named the final CSS style sheets. This is an ordered list that contains:
  - 1. The document or shadow root CSS style sheets.
  - 2. The contents of DocumentOrShadowRoot’s adoptedStyleSheets' backing list, in array order.

- [ ] [create a CSS style sheet](https://drafts.csswg.org/cssom-1/#create-a-css-style-sheet) — To create a CSS style sheet, run these steps:
  - 1. Create a new CSS style sheet object and set its properties as specified.
  - 2. Then run the add a CSS style sheet steps for the newly created CSS style sheet.

- [ ] [add a CSS style sheet](https://drafts.csswg.org/cssom-1/#add-a-css-style-sheet) — To add a CSS style sheet, run these steps:
  - 1. Add the CSS style sheet to the list of document or shadow root CSS style sheets at the appropriate location.
  - 2. If the CSS style sheet’s owner node contributes a script-blocking style sheet, then user agents must append the owner node to its node document’s script-blocking style sheet set.
  - 3. If the disabled flag is set, then return.
  - 4. If the title is not the empty string, the alternate flag is unset, and preferred CSS style sheet set name is the empty string change the preferred CSS style sheet set name to the title.
  - 5. If any of the following is true, then unset the disabled flag and return: The title is the empty string.; The last CSS style sheet set name is null and the title is a case-sensitive match for the preferred CSS style sheet set name.; The title is a case-sensitive match for the last CSS style sheet set name.
  - 6. Set the disabled flag.

- [ ] [remove a CSS style sheet](https://drafts.csswg.org/cssom-1/#remove-a-css-style-sheet) — To remove a CSS style sheet, run these steps:
  - 1. Remove the CSS style sheet from the list of document or shadow root CSS style sheets.
  - 2. Set the CSS style sheet’s parent CSS style sheet, owner node and owner CSS rule to null.

- [ ] [enable a CSS style sheet set](https://drafts.csswg.org/cssom-1/#enable-a-css-style-sheet-set) — To enable a CSS style sheet set with name name, run these steps:
  - 1. If name is the empty string, set the disabled flag for each CSS style sheet that is in a CSS style sheet set and return.
  - 2. Unset the disabled flag for each CSS style sheet in a CSS style sheet set whose CSS style sheet set name is a case-sensitive match for name and set it for all other CSS style sheets in a CSS style sheet set.

- [ ] [select a CSS style sheet set](https://drafts.csswg.org/cssom-1/#select-a-css-style-sheet-set) — To select a CSS style sheet set with name name, run these steps:
  - 1. enable a CSS style sheet set with name name.
  - 2. Set last CSS style sheet set name to name.

- [ ] [change the preferred CSS style sheet set name](https://drafts.csswg.org/cssom-1/#change-the-preferred-css-style-sheet-set-name) — To change the preferred CSS style sheet set name with name name, run these steps:
  - 1. Let current be the preferred CSS style sheet set name.
  - 2. Set preferred CSS style sheet set name to name.
  - 3. If name is not a case-sensitive match for current and last CSS style sheet set name is null enable a CSS style sheet set with name name.

#### [6.2.1. The HTTP Default-Style Header](https://drafts.csswg.org/cssom-1/#the-http-default-style-header)

Status: unreviewed

#### [6.2.2. The StyleSheetList Interface](https://drafts.csswg.org/cssom-1/#the-stylesheetlist-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/StyleSheetList.cpp; the sheets come from style::Scope, without adopted sheets.

- [x] [StyleSheetList.item(index)](https://drafts.csswg.org/cssom-1/#dom-stylesheetlist-item) — The item(index) method must return the indexth CSS style sheet in the collection. If there is no indexth object in the collection, then the method must return null.

- [x] [StyleSheetList.length](https://drafts.csswg.org/cssom-1/#dom-stylesheetlist-length) — The length attribute must return the number of CSS style sheets represented by the collection.

#### [6.2.3. Extensions to the DocumentOrShadowRoot Interface Mixin](https://drafts.csswg.org/cssom-1/#extensions-to-the-document-or-shadow-root-interface)

Status: unreviewed

- [ ] [DocumentOrShadowRoot.styleSheets](https://drafts.csswg.org/cssom-1/#dom-documentorshadowroot-stylesheets) — The styleSheets attribute must return a StyleSheetList collection representing the document or shadow root CSS style sheets.

- [ ] [DocumentOrShadowRoot.adoptedStyleSheets](https://drafts.csswg.org/cssom-1/#dom-documentorshadowroot-adoptedstylesheets) — The set an indexed value algorithm for adoptedStyleSheets, given value and index, is the following:
  - 1. If value’s constructed flag is not set, or its constructor document is not equal to this DocumentOrShadowRoot’s node document, throw a "NotAllowedError" DOMException.

### [6.3. Style Sheet Association](https://drafts.csswg.org/cssom-1/#style-sheet-association)

Status: unreviewed

#### [6.3.1. Fetching CSS style sheets](https://drafts.csswg.org/cssom-1/#fetching-css-style-sheets)

Status: unreviewed
<!-- definitions: fetch-a-css-style-sheet -->

#### [6.3.2. The LinkStyle Interface](https://drafts.csswg.org/cssom-1/#the-linkstyle-interface)

Status: unreviewed
<!-- definitions: associated-css-style-sheet -->

- [ ] [LinkStyle.sheet](https://drafts.csswg.org/cssom-1/#dom-linkstyle-sheet) — The sheet attribute must return the associated CSS style sheet for the node or null if there is no associated CSS style sheet.

#### [6.3.3. Requirements on specifications](https://drafts.csswg.org/cssom-1/#requirements-on-specifications)

Status: unreviewed

#### [6.3.4. Requirements on user agents Implementing the xml-stylesheet processing instruction](https://drafts.csswg.org/cssom-1/#requirements-on-user-agents-implementing-the-xml-stylesheet-processing-instruction)

Status: unreviewed
<!-- definitions: prolog -->

#### [6.3.5. Requirements on user agents Implementing the HTTP Link Header](https://drafts.csswg.org/cssom-1/#requirements-on-user-agents-implementing-the-http-link-header)

Status: unreviewed

### [6.4. CSS Rules](https://drafts.csswg.org/cssom-1/#css-rules)

Status: unreviewed
<!-- definitions: css-rule concept-css-rule-type concept-css-rule-text concept-css-rule-parent-css-rule concept-css-rule-parent-css-style-sheet concept-css-rule-child-css-rules -->

- [ ] [parse a CSS rule](https://drafts.csswg.org/cssom-1/#parse-a-css-rule) — To parse a CSS rule from a string string, run the following steps:
  - 1. Let rule be the return value of invoking parse a rule with string.
  - 2. If rule is a syntax error, return rule.
  - 3. Let parsed rule be the result of parsing rule according to the appropriate CSS specifications, dropping parts that are said to be ignored. If the whole rule is dropped, return a syntax error.
  - 4. Return parsed rule.

- [ ] [serialize a CSS rule](https://drafts.csswg.org/cssom-1/#serialize-a-css-rule) — To serialize a CSS rule, perform one of the following in accordance with the CSS rule’s type:
  - • CSSStyleRule: Return the result of the following steps:
    - 1. Let s initially be the result of performing serialize a group of selectors on the rule’s associated selectors, followed by the string " {", i.e., a single SPACE (U+0020), followed by LEFT CURLY BRACKET (U+007B).
    - 2. Let decls be the result of performing serialize a CSS declaration block on the rule’s associated declarations, or null if there are no such declarations.
    - 3. Let rules be the result of performing serialize a CSS rule on each rule in the rule’s cssRules list, or null if there are no such rules.
    - 4. If decls and rules are both null, append " }" to s (i.e. a single SPACE (U+0020) followed by RIGHT CURLY BRACKET (U+007D)) and return s.
    - 5. If rules is null:
      - 5a. Append a single SPACE (U+0020) to s
      - 5b. Append decls to s
      - 5c. Append " }" to s (i.e. a single SPACE (U+0020) followed by RIGHT CURLY BRACKET (U+007D)).
      - 5d. Return s.
    - 6. Otherwise:
      - 6a. If decls is not null, prepend it to rules.
      - 6b. For each rule in rules: If rule is the empty string, do nothing.; Otherwise:
        - 6b.i. Append a newline followed by two spaces to s.
        - 6b.ii. Append rule to s.
      - 6c. Append a newline followed by RIGHT CURLY BRACKET (U+007D) to s.
      - 6d. Return s.
  - • CSSImportRule: The result of concatenating the following:
    - 1. The string "@import" followed by a single SPACE (U+0020).
    - 2. The result of performing serialize a URL on the rule’s location.
    - 3. If the rule’s associated media list is not empty, a single SPACE (U+0020) followed by the result of performing serialize a media query list on the media list.
    - 4. The string ";", i.e., SEMICOLON (U+003B).
  - • CSSMediaRule: The result of concatenating the following:
    - 1. The string "@media", followed by a single SPACE (U+0020).
    - 2. The result of performing serialize a media query list on rule’s media query list.
    - 3. A single SPACE (U+0020), followed by the string "{", i.e., LEFT CURLY BRACKET (U+007B), followed by a newline.
    - 4. The result of performing serialize a CSS rule on each rule in the rule’s cssRules list, filtering out empty strings, indenting each item with two spaces, all joined with newline.
    - 5. A newline, followed by the string "}", i.e., RIGHT CURLY BRACKET (U+007D)
  - • CSSFontFaceRule: The result of concatenating the following:
    - 1. The string "@font-face {".
    - 2. If the font-family descriptor is present:
      - 2a. A single SPACE (U+0020), followed by the string "font-family:", followed by a single SPACE (U+0020).
      - 2b. The result of performing serialize a string on the rule’s font family name.
      - 2c. The string ";", i.e., SEMICOLON (U+003B).
    - 3. If the rule’s associated source list is not empty, follow these substeps:
      - 3a. A single SPACE (U+0020), followed by the string "src:", followed by a single SPACE (U+0020).
      - 3b. The result of invoking serialize a comma-separated list on performing serialize a URL or serialize a LOCAL for each source on the source list.
      - 3c. The string ";", i.e., SEMICOLON (U+003B).
    - 4. If rule’s associated unicode-range descriptor is present, a single SPACE (U+0020), followed by the string "unicode-range:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'unicode-range'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 5. If rule’s associated font-variant descriptor is present, a single SPACE (U+0020), followed by the string "font-variant:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'font-variant'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 6. If rule’s associated font-feature-settings descriptor is present, a single SPACE (U+0020), followed by the string "font-feature-settings:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'font-feature-settings'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 7. If rule’s associated font-stretch descriptor is present, a single SPACE (U+0020), followed by the string "font-stretch:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'font-stretch'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 8. If rule’s associated font-weight descriptor is present, a single SPACE (U+0020), followed by the string "font-weight:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'font-weight'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 9. If rule’s associated font-style descriptor is present, a single SPACE (U+0020), followed by the string "font-style:", followed by a single SPACE (U+0020), followed by the result of performing serialize a <'font-style'>, followed by the string ";", i.e., SEMICOLON (U+003B).
    - 10. A single SPACE (U+0020), followed by the string "}", i.e., RIGHT CURLY BRACKET (U+007D).
  - • CSSPageRule:
  - • CSSNamespaceRule: The literal string "@namespace", followed by a single SPACE (U+0020), followed by the serialization as an identifier of the prefix attribute (if any), followed by a single SPACE (U+0020) if there is a prefix, followed by the serialization as URL of the namespaceURI attribute, followed the character ";" (U+003B).
  - • CSSKeyframesRule: The result of concatenating the following:
    - 1. The literal string "@keyframes", followed by a single SPACE (U+0020).
    - 2. The serialization of the name attribute. If the attribute is a CSS wide keyword, or the value default, or the value none, then it is serialized as a string. Otherwise, it is serialized as an identifier.
    - 3. The string " { ", i.e., a single SPACE (U+0020), followed by LEFT CURLY BRACKET (U+007B), followed by a single SPACE (U+0020).
    - 4. The result of performing serialize a CSS rule on each rule in the rule’s cssRules list, separated by a newline and indented by two spaces.
    - 5. A newline, followed by the string "}", i.e., RIGHT CURLY BRACKET (U+007D)
  - • CSSKeyframeRule: The result of concatenating the following:
    - 1. The keyText.
    - 2. The string " { ", i.e., a single SPACE (U+0020), followed by LEFT CURLY BRACKET (U+007B), followed by a single SPACE (U+0020).
    - 3. The result of performing serialize a CSS declaration block on the rule’s associated declarations.
    - 4. If the rule is associated with one or more declarations, the string " ", i.e., a single SPACE (U+0020).
    - 5. The string "}", RIGHT CURLY BRACKET (U+007D).

- [ ] [insert a CSS rule](https://drafts.csswg.org/cssom-1/#insert-a-css-rule) — To insert a CSS rule rule in a CSS rule list list at index index, with a flag nested, follow these steps:
  - 1. Set length to the number of items in list.
  - 2. If index is greater than length, then throw an IndexSizeError exception.
  - 3. Set new rule to the results of performing parse a CSS rule on argument rule.
  - 4. If new rule is a syntax error, and nested is set, perform the following substeps: Set declarations to the results of performing parse a CSS declaration block, on argument rule.; If declarations is empty, throw a SyntaxError exception.; Otherwise, set new rule to a new nested declarations rule with declarations as it contents.
  - 5. If new rule is a syntax error, throw a SyntaxError exception.
  - 6. If new rule cannot be inserted into list at the zero-indexed position index due to constraints specified by CSS, then throw a HierarchyRequestError exception. [CSS21]
  - 7. If new rule is an @namespace at-rule, and list contains anything other than @import at-rules, and @namespace at-rules, throw an InvalidStateError exception.
  - 8. Insert new rule into list at the zero-indexed position index.
  - 9. Return index.

- [ ] [remove a CSS rule](https://drafts.csswg.org/cssom-1/#remove-a-css-rule) — To remove a CSS rule from a CSS rule list list at index index, follow these steps:
  - 1. Set length to the number of items in list.
  - 2. If index is greater than or equal to length, then throw an IndexSizeError exception.
  - 3. Set old rule to the indexth item in list.
  - 4. If old rule is an @namespace at-rule, and list contains anything other than @import at-rules, and @namespace at-rules, throw an InvalidStateError exception.
  - 5. Remove rule old rule from list at the zero-indexed position index.
  - 6. Set old rule’s parent CSS rule and parent CSS style sheet to null.

#### [6.4.1. The CSSRuleList Interface](https://drafts.csswg.org/cssom-1/#the-cssrulelist-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/include/Krystal.Booey/Css/CSSOM/CSSRuleList.hpp (static and live lists).

- [x] [CSSRuleList.item(index)](https://drafts.csswg.org/cssom-1/#dom-cssrulelist-item) — The item(index) method must return the indexth CSSRule object in the collection. If there is no indexth object in the collection, then the method must return null.

- [x] [CSSRuleList.length](https://drafts.csswg.org/cssom-1/#dom-cssrulelist-length) — The length attribute must return the number of CSSRule objects represented by the collection.

#### [6.4.2. The CSSRule Interface](https://drafts.csswg.org/cssom-1/#the-cssrule-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSRule.cpp.

- [x] [CSSRule.cssText](https://drafts.csswg.org/cssom-1/#dom-cssrule-csstext) — The cssText attribute must return a serialization of the CSS rule. On setting the cssText attribute must do nothing.

- [x] [CSSRule.parentRule](https://drafts.csswg.org/cssom-1/#dom-cssrule-parentrule) — The parentRule attribute must return the parent CSS rule.

- [x] [CSSRule.parentStyleSheet](https://drafts.csswg.org/cssom-1/#dom-cssrule-parentstylesheet) — The parentStyleSheet attribute must return the parent CSS style sheet.

- [x] [CSSRule.type](https://drafts.csswg.org/cssom-1/#dom-cssrule-type) — The type attribute is deprecated. It must return an integer, as follows:
  - • If the object is a CSSStyleRule: Return 1.
  - • If the object is a CSSImportRule: Return 3.
  - • If the object is a CSSMediaRule: Return 4.
  - • If the object is a CSSFontFaceRule: Return 5.
  - • If the object is a CSSPageRule: Return 6.
  - • If the object is a CSSKeyframesRule: Return 7.
  - • If the object is a CSSKeyframeRule: Return 8.
  - • If the object is a CSSMarginRule: Return 9.
  - • If the object is a CSSNamespaceRule: Return 10.
  - • If the object is a CSSCounterStyleRule: Return 11.
  - • If the object is a CSSSupportsRule: Return 12.
  - • If the object is a CSSFontFeatureValuesRule: Return 14.
  - • Otherwise: Return 0.

#### [6.4.3. The CSSStyleRule Interface](https://drafts.csswg.org/cssom-1/#the-cssstylerule-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSStyleRule.cpp, which also implements CSSGroupingRule's members for nested rules (insert and remove a CSS rule).
<!-- definitions: concept-declarations-specified-order -->

- [x] [CSSStyleRule.selectorText](https://drafts.csswg.org/cssom-1/#dom-cssstylerule-selectortext) — The selectorText attribute, on getting, must return the result of serializing the rule’s associated selector list. On setting the selectorText attribute these steps must be run:
  - 1. Run the parse a group of selectors algorithm on the given value.
  - 2. If the algorithm returns a non-null value replace the associated selector list with the returned value.
  - 3. Otherwise, if the algorithm returns a null value, do nothing.

- [x] [CSSStyleRule.style](https://drafts.csswg.org/cssom-1/#dom-cssstylerule-style) — The style attribute must return a CSSStyleProperties object for the style rule, with the following properties:
  - • computed flag: Unset.
  - • readonly flag: Unset.
  - • declarations: The declared declarations in the rule, in specified order.
  - • parent CSS rule: this.
  - • owner node: Null.

#### [6.4.4. The CSSImportRule Interface](https://drafts.csswg.org/cssom-1/#the-cssimportrule-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSImportRule.cpp; the imported sheet's media is the rule's MediaList (CSSStyleSheet::Media).

- [x] [CSSImportRule.href](https://drafts.csswg.org/cssom-1/#dom-cssimportrule-href) — The href attribute must return the URL specified by the @import at-rule.

- [x] [CSSImportRule.media](https://drafts.csswg.org/cssom-1/#dom-cssimportrule-media) — The media attribute must return the value of the media attribute of the associated CSS style sheet.

- [x] [CSSImportRule.styleSheet](https://drafts.csswg.org/cssom-1/#dom-cssimportrule-stylesheet) — The styleSheet attribute must return the associated CSS style sheet, if any, or null otherwise.

- [x] [CSSImportRule.layerName](https://drafts.csswg.org/cssom-1/#dom-cssimportrule-layername) — The layerName attribute must return the layer name declared in the at-rule itself, or an empty string if the layer is anonymous, or null if the at-rule does not declare a layer.

- [x] [CSSImportRule.supportsText](https://drafts.csswg.org/cssom-1/#dom-cssimportrule-supportstext) — The supportsText attribute must return the <supports-condition> declared in the at-rule itself, or null if the at-rule does not declare a supports condition.

#### [6.4.5. The CSSGroupingRule Interface](https://drafts.csswg.org/cssom-1/#the-cssgroupingrule-interface)

Status: unreviewed

- [ ] [CSSGroupingRule.cssRules](https://drafts.csswg.org/cssom-1/#dom-cssgroupingrule-cssrules) — The cssRules attribute must return a CSSRuleList object for the child CSS rules.

- [ ] [CSSGroupingRule.insertRule(rule, index)](https://drafts.csswg.org/cssom-1/#dom-cssgroupingrule-insertrule) — The insertRule(rule, index) method must return the result of invoking insert a CSS rule rule into the child CSS rules at index, with the nested flag set.

- [ ] [CSSGroupingRule.deleteRule(index)](https://drafts.csswg.org/cssom-1/#dom-cssgroupingrule-deleterule) — The deleteRule(index) method must remove a CSS rule from the child CSS rules at index.

#### [6.4.6. The CSSMediaRule Interface](https://drafts.csswg.org/cssom-1/#the-cssmediarule-interface)

Status: reviewed (2026-11-22)
> Code: Krystal.Booey/src/Css/CSSOM/CSSMediaRule.cpp.

#### [6.4.7. The CSSPageRule Interface](https://drafts.csswg.org/cssom-1/#the-csspagerule-interface)

Status: unreviewed

- [ ] [CSSPageRule.selectorText](https://drafts.csswg.org/cssom-1/#dom-csspagerule-selectortext) — The selectorText attribute, on getting, must return the result of serializing the associated selector list. On setting the selectorText attribute these steps must be run:
  - 1. Run the parse a list of CSS page selectors algorithm on the given value.
  - 2. If the algorithm returns a non-null value replace the associated selector list with the returned value.
  - 3. Otherwise, if the algorithm returns a null value, do nothing.

- [ ] [CSSPageRule.style](https://drafts.csswg.org/cssom-1/#dom-csspagerule-style) — The style attribute must return a CSSPageDescriptors object for the @page at-rule, with the following properties:
  - • computed flag: Unset.
  - • readonly flag: Unset.
  - • declarations: The declared descriptors in the rule, in specified order.
  - • parent CSS rule: this.
  - • owner node: Null.

#### [6.4.8. The CSSMarginRule Interface](https://drafts.csswg.org/cssom-1/#the-cssmarginrule-interface)

Status: unreviewed

- [ ] [CSSMarginRule.name](https://drafts.csswg.org/cssom-1/#dom-cssmarginrule-name) — The name attribute must return the name of the margin at-rule. The @ character is not included in the name. [CSS3SYN]

- [ ] [CSSMarginRule.style](https://drafts.csswg.org/cssom-1/#dom-cssmarginrule-style) — The style attribute must return a CSSMarginDescriptors object for the margin at-rule, with the following properties:
  - • computed flag: Unset.
  - • readonly flag: Unset.
  - • declarations: The declared declarations in the rule, in specified order.
  - • parent CSS rule: this.
  - • owner node: Null.

#### [6.4.9. The CSSNamespaceRule Interface](https://drafts.csswg.org/cssom-1/#the-cssnamespacerule-interface)

Status: unreviewed

- [ ] [CSSNamespaceRule.namespaceURI](https://drafts.csswg.org/cssom-1/#dom-cssnamespacerule-namespaceuri) — The namespaceURI attribute must return the namespace of the @namespace at-rule.

- [ ] [CSSNamespaceRule.prefix](https://drafts.csswg.org/cssom-1/#dom-cssnamespacerule-prefix) — The prefix attribute must return the prefix of the @namespace at-rule or the empty string if there is no prefix.

### [6.5. CSS Declarations](https://drafts.csswg.org/cssom-1/#css-declarations)

Status: unreviewed
<!-- definitions: css-declaration css-declaration-property-name css-declaration-value css-declaration-important-flag css-declaration-case-sensitive-flag -->

### [6.6. CSS Declaration Blocks](https://drafts.csswg.org/cssom-1/#css-declaration-blocks)

Status: unreviewed
<!-- definitions: css-declaration-block cssstyledeclaration-computed-flag cssstyledeclaration-readonly-flag cssstyledeclaration-declarations cssstyledeclaration-parent-css-rule cssstyledeclaration-owner-node cssstyledeclaration-updating-flag serialize-a-css-declaration-block serialize-into-a-shorthand-form concept-shorthands-preferred-order -->

- [ ] [parse a CSS declaration block](https://drafts.csswg.org/cssom-1/#parse-a-css-declaration-block) — To parse a CSS declaration block from a string string, follow these steps:
  - 1. Let declarations be the returned declarations from invoking parse a list of declarations with string.
  - 2. Let parsed declarations be a new empty list.
  - 3. For each item declaration in declarations, follow these substeps:
    - 3a. Let parsed declaration be the result of parsing declaration according to the appropriate CSS specifications, dropping parts that are said to be ignored. If the whole declaration is dropped, let parsed declaration be null.
    - 3b. If parsed declaration is not null, append it to parsed declarations.
  - 4. Return parsed declarations.

- [ ] [serialize a CSS declaration](https://drafts.csswg.org/cssom-1/#serialize-a-css-declaration) — To serialize a CSS declaration with property name property, value value and optionally an important flag set, follow these steps:
  - 1. Let s be the empty string.
  - 2. Append property to s.
  - 3. Append ": " (U+003A U+0020) to s.
  - 4. If value contains any non-whitespace characters, append value to s.
  - 5. If the important flag is set, append " !important" (U+0020 U+0021 U+0069 U+006D U+0070 U+006F U+0072 U+0074 U+0061 U+006E U+0074) to s.
  - 6. Append ";" (U+003B) to s.
  - 7. Return s.

- [ ] [A CSS declaration block has these attribute change steps for its owner node wit…](https://drafts.csswg.org/cssom-1/#intermixed-properties) — A CSS declaration block has these attribute change steps for its owner node with localName, value, and namespace:
  - 1. If the computed flag is set, then return.
  - 2. If the updating flag is set, then return.
  - 3. If localName is not "style", or namespace is not null, then return.
  - 4. If value is null, empty the declarations.
  - 5. Otherwise, let the declarations be the result of parse a CSS declaration block from a string value.

- [ ] [update style attribute for](https://drafts.csswg.org/cssom-1/#update-style-attribute-for) — To update style attribute for declaration block means to run the steps below:
  - 1. Assert: declaration block’s computed flag is unset.
  - 2. Let owner node be declaration block’s owner node.
  - 3. If owner node is null, then return.
  - 4. Set declaration block’s updating flag.
  - 5. Set an attribute value for owner node using "style" and the result of serializing declaration block.
  - 6. Unset declaration block’s updating flag.

#### [6.6.1. The CSSStyleDeclaration Interface](https://drafts.csswg.org/cssom-1/#the-cssstyledeclaration-interface)

Status: unreviewed
<!-- definitions: dom-cssstyleproperties-camel-cased-attribute -->

- [ ] [CSSStyleDeclaration.cssText](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-csstext) — Getting the cssText attribute must run these steps:
  - 1. If the computed flag is set, then return the empty string.
  - 2. Return the result of serializing the declarations.

- [ ] [Setting the cssText attribute must run these steps](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-csstext) — Setting the cssText attribute must run these steps:
  - 1. If the readonly flag is set, then throw a NoModificationAllowedError exception.
  - 2. Empty the declarations.
  - 3. Parse the given value and, if the return value is not the empty list, insert the items in the list into the declarations, in specified order.
  - 4. Update style attribute for the CSS declaration block.

- [ ] [CSSStyleDeclaration.length](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-length) — The length attribute must return the number of CSS declarations in the declarations.

- [ ] [CSSStyleDeclaration.item(index)](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-item) — The item(index) method must return the property name of the CSS declaration at position index. If there is no indexth object in the collection, then the method must return the empty string.

- [ ] [CSSStyleDeclaration.getPropertyValue(property)](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-getpropertyvalue) — The getPropertyValue(property) method must run these steps:
  - 1. If property is not a custom property, follow these substeps:
    - 1a. Let property be property converted to ASCII lowercase.
    - 1b. If property is a shorthand property, then follow these substeps:
      - 1b.i. Let list be a new empty array.
      - 1b.ii. For each longhand property longhand that property maps to, in canonical order, follow these substeps:
        - 1b.ii.A. If longhand is a case-sensitive match for a property name of a CSS declaration in the declarations, let declaration be that CSS declaration, or null otherwise.
        - 1b.ii.B. If declaration is null, then return the empty string.
        - 1b.ii.C. Append the declaration to list.
      - 1b.iii. If important flags of all declarations in list are same, then return the serialization of list.
      - 1b.iv. Return the empty string.
  - 2. If property is a case-sensitive match for a property name of a CSS declaration in the declarations, then return the result of invoking serialize a CSS value of that declaration.
  - 3. Return the empty string.

- [ ] [CSSStyleDeclaration.getPropertyPriority(property)](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-getpropertypriority) — The getPropertyPriority(property) method must run these steps:
  - 1. If property is not a custom property, follow these substeps:
    - 1a. Let property be property converted to ASCII lowercase.
    - 1b. If property is a shorthand property, follow these substeps:
      - 1b.i. Let list be a new array.
      - 1b.ii. For each longhand property longhand that property maps to, append the result of invoking getPropertyPriority() with longhand as argument to list.
      - 1b.iii. If all items in list are the string "important", then return the string "important".
  - 2. If property is a case-sensitive match for a property name of a CSS declaration in the declarations that has the important flag set, return the string "important".
  - 3. Return the empty string.

- [ ] [CSSStyleDeclaration.setProperty(property, value, priority)](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-setproperty) — The setProperty(property, value, priority) method must run these steps:
  - 1. If the readonly flag is set, then throw a NoModificationAllowedError exception.
  - 2. If property is not a custom property, follow these substeps:
    - 2a. Let property be property converted to ASCII lowercase.
    - 2b. If property is not a case-sensitive match for a supported CSS property, then return.
  - 3. If value is the empty string, invoke removeProperty() with property as argument and return.
  - 4. If priority is not the empty string and is not an ASCII case-insensitive match for the string "important", then return.
  - 5. Let component value list be the result of parsing value for property property.
  - 6. If component value list is null, then return.
  - 7. Let updated be false.
  - 8. If property is a shorthand property, then for each longhand property longhand that property maps to, in canonical order, follow these substeps:
    - 8a. Let longhand result be the result of set the CSS declaration longhand with the appropriate value(s) from component value list, with the important flag set if priority is not the empty string, and unset otherwise, and with the list of declarations being the declarations.
    - 8b. If longhand result is true, let updated be true.
  - 9. Otherwise, let updated be the result of set the CSS declaration property with value component value list, with the important flag set if priority is not the empty string, and unset otherwise, and with the list of declarations being the declarations.
  - 10. If updated is true, update style attribute for the CSS declaration block.

- [ ] [set a CSS declaration](https://drafts.csswg.org/cssom-1/#set-a-css-declaration) — To set a CSS declaration property with a value component value list and optionally with an important flag set, in a list of declarations declarations, the user agent must ensure the following constraints hold after its steps:

- [ ] [CSSStyleDeclaration.removeProperty(property)](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-removeproperty) — The removeProperty(property) method must run these steps:
  - 1. If the readonly flag is set, then throw a NoModificationAllowedError exception.
  - 2. If property is not a custom property, let property be property converted to ASCII lowercase.
  - 3. Let value be the return value of invoking getPropertyValue() with property as argument.
  - 4. Let removed be false.
  - 5. If property is a shorthand property, for each longhand property longhand that property maps to:
    - 5a. If longhand is not a property name of a CSS declaration in the declarations, continue.
    - 5b. Remove that CSS declaration and let removed be true.
  - 6. Otherwise, if property is a case-sensitive match for a property name of a CSS declaration in the declarations, remove that CSS declaration and let removed be true.
  - 7. If removed is true, Update style attribute for the CSS declaration block.
  - 8. Return value.

- [ ] [CSSStyleDeclaration.parentRule](https://drafts.csswg.org/cssom-1/#dom-cssstyledeclaration-parentrule) — The parentRule attribute must return the parent CSS rule.

- [ ] [CSSStyleProperties.cssFloat](https://drafts.csswg.org/cssom-1/#dom-cssstyleproperties-cssfloat) — The cssFloat attribute, on getting, must return the result of invoking getPropertyValue() with float as argument. On setting, the attribute must invoke setProperty() with float as first argument, as second argument the given value, and no third argument. Any exceptions thrown must be re-thrown.

- [ ] [CSSStyleProperties.webkit-cased attribute](https://drafts.csswg.org/cssom-1/#dom-cssstyleproperties-webkit-cased-attribute) — The webkit-cased attribute attribute, on getting, must return the result of invoking getPropertyValue() with the argument being the result of running the IDL attribute to CSS property algorithm for webkit-cased attribute, with the dash prefix flag set.

- [ ] [CSSStyleProperties.dashed attribute](https://drafts.csswg.org/cssom-1/#dom-cssstyleproperties-dashed-attribute) — The dashed attribute attribute, on getting, must return the result of invoking getPropertyValue() with the argument being dashed attribute.

- [ ] [CSS property to IDL attribute](https://drafts.csswg.org/cssom-1/#css-property-to-idl-attribute) — The CSS property to IDL attribute algorithm for property, optionally with a lowercase first flag set, is as follows:
  - 1. Let output be the empty string.
  - 2. Let uppercase next be unset.
  - 3. If the lowercase first flag is set, remove the first character from property.
  - 4. For each character c in property:
    - 4a. If c is "-" (U+002D), let uppercase next be set.
    - 4b. Otherwise, if uppercase next is set, let uppercase next be unset and append c converted to ASCII uppercase to output.
    - 4c. Otherwise, append c to output.
  - 5. Return output.

- [ ] [IDL attribute to CSS property](https://drafts.csswg.org/cssom-1/#idl-attribute-to-css-property) — The IDL attribute to CSS property algorithm for attribute, optionally with a dash prefix flag set, is as follows:
  - 1. Let output be the empty string.
  - 2. If the dash prefix flag is set, append "-" (U+002D) to output.
  - 3. For each character c in attribute:
    - 3a. If c is in the range U+0041 to U+005A (ASCII uppercase), append "-" (U+002D) followed by c converted to ASCII lowercase to output.
    - 3b. Otherwise, append c to output.
  - 4. Return output.

### [6.7. CSS Values](https://drafts.csswg.org/cssom-1/#css-values)

Status: unreviewed

#### [6.7.1. Parsing CSS Values](https://drafts.csswg.org/cssom-1/#parsing-css-values)

Status: unreviewed

- [ ] [parse a CSS value](https://drafts.csswg.org/cssom-1/#parse-a-css-value) — To parse a CSS value value for a given property means to follow these steps:
  - 1. Let list be the value returned by invoking parse a list of component values from value.
  - 2. Match list against the grammar for the property property in the CSS specification.
  - 3. If the above step failed, return null.
  - 4. Return list.

#### [6.7.2. Serializing CSS Values](https://drafts.csswg.org/cssom-1/#serializing-css-values)

Status: unreviewed

- [ ] [serialize a CSS value](https://drafts.csswg.org/cssom-1/#serialize-a-css-value) — To serialize a CSS value of a CSS declaration declaration or a list of longhand CSS declarations list, follow these rules:
  - 1. If this algorithm is invoked with a list list:
    - 1a. Let shorthand be the first shorthand property, in preferred shorthand order, that exactly maps to all of the longhand properties in list.
    - 1b. If there is no such shorthand or shorthand cannot exactly represent the values of all the properties in list, return the empty string.
    - 1c. Otherwise, serialize a CSS value from a hypothetical declaration of the property shorthand with its value representing the combined values of the declarations in list.
  - 2. Represent the value of the declaration as a list of CSS component values components that, when parsed according to the property’s grammar, would represent that value. Additionally, unless otherwise specified: If certain component values can appear in any order without changing the meaning of the value (a pattern typically represented by a double bar || in the value syntax), reorder the component…
  - 3. Remove any <whitespace-token>s from components.
  - 4. Replace each component value in components with the result of invoking serialize a CSS component value.
  - 5. Join the items of components into a single string, inserting " " (U+0020 SPACE) between each pair of items unless the second item is a "," (U+002C COMMA) Return the result.

- [ ] [serialize a CSS component value](https://drafts.csswg.org/cssom-1/#serialize-a-css-component-value) — To serialize a CSS component value depends on the component, as follows:
  - • keyword: The keyword converted to ASCII lowercase.
  - • <angle>: The <number> component serialized as per <number> followed by the unit in canonical form as defined in its respective specification.
  - • <color>: If <color> is a component of a resolved value, see CSS Color 4 § 15. Resolving <color> Values. If <color> is a component of a computed value, see CSS Color 4 § 16. Serializing <color> Values. If <color> is a component of a declared value, then for sRGB values, see CSS Color 4 § 15.1 Resolving sRGB values. For other color functions, see CSS Color 4 § 15. Resolving <color> Values.
  - • <alpha-value>: See CSS Color 4 § 16.1 Serializing alpha values.
  - • <counter>: The return value of the following algorithm:
    - 1. Let s be the empty string.
    - 2. If <counter> has three CSS component values append the string "counters(" to s.
    - 3. If <counter> has two CSS component values append the string "counter(" to s.
    - 4. Let list be a list of CSS component values belonging to <counter>, omitting the last CSS component value if it is "decimal".
    - 5. Let each item in list be the result of invoking serialize a CSS component value on that item.
    - 6. Append the result of invoking serialize a comma-separated list on list to s.
    - 7. Append ")" (U+0029) to s.
    - 8. Return s.
  - • <frequency>: The <number> component serialized as per <number> followed by the unit in its canonical form as defined in its respective specification.
  - • <identifier>: The identifier serialized as an identifier.
  - • <integer>: A base-ten integer using digits 0-9 (U+0030 to U+0039) in the shortest form possible, preceded by "-" (U+002D) if it is negative.
  - • <length>: The <number> component serialized as per <number> followed by the unit in its canonical form as defined in its respective specification.
  - • <number>: A base-ten number using digits 0-9 (U+0030 to U+0039) in the shortest form possible, using "." to separate decimals (if any), rounding the value if necessary to not produce more than 6 decimals, preceded by "-" (U+002D) if it is negative.
  - • <percentage>: The <number> component serialized as per <number> followed by the literal string "%" (U+0025).
  - • <resolution>: The resolution in dots per CSS pixel serialized as per <number> followed by the literal string "dppx".
  - • <ratio>: The numerator serialized as per <number> followed by the literal string " / ", followed by the denominator serialized as per <number>.
  - • <shape>: The return value of the following algorithm:
    - 1. Let s be the string "rect(".
    - 2. Let list be a list of the CSS component values belonging to <shape>.
    - 3. Let each item in list be the result of invoking serialize a CSS component value of that item.
    - 4. Append the result of invoking serialize a comma-separated list on list to s.
    - 5. Append ")" (U+0029) to s.
    - 6. Return s.
  - • <string> / <font-family-name>: The string serialized as a string.
  - • <time>: The time in seconds serialized as per <number> followed by the literal string "s".
  - • <url>: The absolute-URL string serialized as URL.

##### [6.7.2.1. Examples](https://drafts.csswg.org/cssom-1/#serializing-css-values-examples)

Status: unreviewed

## [7. DOM Access to CSS Declaration Blocks](https://drafts.csswg.org/cssom-1/#dom-access-to-css-declaration-blocks)

Status: unreviewed

### [7.1. The ElementCSSInlineStyle Mixin](https://drafts.csswg.org/cssom-1/#the-elementcssinlinestyle-mixin)

Status: unreviewed

- [ ] [ElementCSSInlineStyle.style](https://drafts.csswg.org/cssom-1/#dom-elementcssinlinestyle-style) — The style attribute must return a CSSStyleProperties object whose readonly flag is unset, whose parent CSS rule is null, and whose owner node is this.

### [7.2. Extensions to the Window Interface](https://drafts.csswg.org/cssom-1/#extensions-to-the-window-interface)

Status: unreviewed

- [ ] [Window.getComputedStyle(elt, pseudoElt)](https://drafts.csswg.org/cssom-1/#dom-window-getcomputedstyle) — The getComputedStyle(elt, pseudoElt) method must run these steps:
  - 1. Let doc be elt’s node document.
  - 2. Let obj be elt.
  - 3. If pseudoElt is provided, is not the empty string, and starts with a colon, then:
    - 3a. Parse pseudoElt as a <pseudo-element-selector>, and let type be the result.
    - 3b. If type is failure, or is a ::slotted() or ::part() pseudo-element, let obj be null.
    - 3c. Otherwise let obj be the given pseudo-element of elt.
  - 4. Let decls be an empty list of CSS declarations.
  - 5. If obj is not null, and elt is connected, part of the flat tree, and its shadow-including root has a browsing context which either doesn’t have a browsing context container, or whose browsing context container is being rendered, set decls to a list of all longhand properties that are supported CSS properties, in lexicographical order, with the value being the resolved value computed for obj using…
  - 6. Return a live CSSStyleProperties object with the following properties: computed flag Set. readonly flag Set. declarations decls. parent CSS rule Null. owner node obj.

## [8. Utility APIs](https://drafts.csswg.org/cssom-1/#utility-apis)

Status: unreviewed

### [8.1. The CSS.escape() Method](https://drafts.csswg.org/cssom-1/#the-css.escape%28%29-method)

Status: unreviewed

- [ ] [CSS.escape(ident)](https://drafts.csswg.org/cssom-1/#dom-css-escape) — The escape(ident) operation must return the result of invoking serialize an identifier of ident.

## [9. Resolved Values](https://drafts.csswg.org/cssom-1/#resolved-values)

Status: unreviewed
<!-- definitions: resolved-value resolved-value-special-case-property-like-color resolved-value-special-case-property-like-height resolved-value-special-case-property-like-top resolved-value-special-case-property -->

## [10. IANA Considerations](https://drafts.csswg.org/cssom-1/#iana-considerations)

Status: unreviewed

### [10.1. Default-Style](https://drafts.csswg.org/cssom-1/#default-style)

Status: unreviewed
<!-- definitions: http-default-style -->

## [11. Change History](https://drafts.csswg.org/cssom-1/#change-history)

Status: unreviewed

### [11.1. Changes From 17 March 2016](https://drafts.csswg.org/cssom-1/#changes-from-17-march-2016)

Status: unreviewed

### [11.2. Changes From 5 December 2013](https://drafts.csswg.org/cssom-1/#changes-from-5-december-2013)

Status: unreviewed

### [11.3. Changes From 12 July 2011 To 5 December 2013](https://drafts.csswg.org/cssom-1/#changes-from-12-july-2011-to-5-december-2013)

Status: unreviewed

## [12. Security Considerations](https://drafts.csswg.org/cssom-1/#sec)

Status: unreviewed

## [13. Privacy Considerations](https://drafts.csswg.org/cssom-1/#priv)

Status: unreviewed

## [14. Acknowledgments](https://drafts.csswg.org/cssom-1/#acknowledgments)

Status: unreviewed

## [Conformance](https://drafts.csswg.org/cssom-1/#w3c-conformance)

Status: unreviewed

### [Document conventions](https://drafts.csswg.org/cssom-1/#w3c-conventions)

Status: unreviewed

### [Conformance classes](https://drafts.csswg.org/cssom-1/#w3c-conformance-classes)

Status: unreviewed

### [Partial implementations](https://drafts.csswg.org/cssom-1/#w3c-partial)

Status: unreviewed

#### [Implementations of Unstable and Proprietary Features](https://drafts.csswg.org/cssom-1/#w3c-conform-future-proofing)

Status: unreviewed

### [Non-experimental implementations](https://drafts.csswg.org/cssom-1/#w3c-testing)

Status: unreviewed

## [Index](https://drafts.csswg.org/cssom-1/#index)

Status: unreviewed

### [Terms defined by this specification](https://drafts.csswg.org/cssom-1/#index-defined-here)

Status: unreviewed

### [Terms defined by reference](https://drafts.csswg.org/cssom-1/#index-defined-elsewhere)

Status: unreviewed

## [References](https://drafts.csswg.org/cssom-1/#references)

Status: unreviewed

### [Normative References](https://drafts.csswg.org/cssom-1/#normative)

Status: unreviewed

### [Non-Normative References](https://drafts.csswg.org/cssom-1/#informative)

Status: unreviewed

## [IDL Index](https://drafts.csswg.org/cssom-1/#idl-index)

Status: unreviewed

## [Issues Index](https://drafts.csswg.org/cssom-1/#issues-index)

Status: unreviewed
