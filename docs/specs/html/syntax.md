# HTML review checklist: 13 The HTML syntax

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 11 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html syntax` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [13 The HTML syntax](https://html.spec.whatwg.org/#syntax)

Status: unreviewed

### [13.1 Writing HTML documents](https://html.spec.whatwg.org/#writing)

Status: n/a (author conformance: the parser is 13.2)

- [x] [Documents must consist of the following parts, in the given order](https://html.spec.whatwg.org/#writing) — Documents must consist of the following parts, in the given order:
  - 1. Optionally, a single U+FEFF BYTE ORDER MARK (BOM) character.
  - 2. Any number of comments and ASCII whitespace.
  - 3. A DOCTYPE.
  - 4. Any number of comments and ASCII whitespace.
  - 5. The document element, in the form of an html element.
  - 6. Any number of comments and ASCII whitespace.

#### [13.1.1 The DOCTYPE](https://html.spec.whatwg.org/#the-doctype)

Status: n/a (author conformance: the parser is 13.2)

- [x] [A DOCTYPE must consist of the following components, in this order](https://html.spec.whatwg.org/#syntax-doctype) — A DOCTYPE must consist of the following components, in this order:
  - 1. A string that is an ASCII case-insensitive match for the string "<!DOCTYPE".
  - 2. One or more ASCII whitespace.
  - 3. A string that is an ASCII case-insensitive match for the string "html".
  - 4. Optionally, a DOCTYPE legacy string.
  - 5. Zero or more ASCII whitespace.
  - 6. A U+003E GREATER-THAN SIGN character (>).

- [x] [DOCTYPE legacy string](https://html.spec.whatwg.org/#doctype-legacy-string) — For the purposes of HTML generators that cannot output HTML markup with the short DOCTYPE "<!DOCTYPE html>", a DOCTYPE legacy string may be inserted into the DOCTYPE (in the position defined above). This string must consist of:
  - 1. One or more ASCII whitespace.
  - 2. A string that is an ASCII case-insensitive match for the string "SYSTEM".
  - 3. One or more ASCII whitespace.
  - 4. A U+0022 QUOTATION MARK or U+0027 APOSTROPHE character (the quote mark).
  - 5. The literal string "about:legacy-compat".
  - 6. A matching U+0022 QUOTATION MARK or U+0027 APOSTROPHE character (i.e. the same character as in the earlier step labeled quote mark).

#### [13.1.2 Elements](https://html.spec.whatwg.org/#elements-2)

Status: unreviewed
<!-- definitions: syntax-elements void-elements the-template-element-2 raw-text-elements escapable-raw-text-elements foreign-elements normal-elements syntax-tags syntax-tag-name -->

##### [13.1.2.1 Start tags](https://html.spec.whatwg.org/#start-tags)

Status: n/a (author conformance: the parser is 13.2)

- [x] [Start tags](https://html.spec.whatwg.org/#syntax-start-tag) — Start tags must have the following format:
  - 1. The first character of a start tag must be a U+003C LESS-THAN SIGN character (<).
  - 2. The next few characters of a start tag must be the element's tag name.
  - 3. If there are to be any attributes in the next step, there must first be one or more ASCII whitespace.
  - 4. Then, the start tag may have a number of attributes, the syntax for which is described below. Attributes must be separated from each other by one or more ASCII whitespace.
  - 5. After the attributes, or after the tag name if there are no attributes, there may be one or more ASCII whitespace. (Some attributes are required to be followed by a space. See the attributes section below.)
  - 6. Then, if the element is one of the void elements, or if the element is a foreign element, then there may be a single U+002F SOLIDUS character (/), which on foreign elements marks the start tag as self-closing. On void elements, it does not mark the start tag as self-closing but instead is unnecessary and has no effect of any kind. For such void elements, it should be used only with caution — espe…
  - 7. Finally, start tags must be closed by a U+003E GREATER-THAN SIGN character (>).

##### [13.1.2.2 End tags](https://html.spec.whatwg.org/#end-tags)

Status: n/a (author conformance: the parser is 13.2)

- [x] [End tags](https://html.spec.whatwg.org/#syntax-end-tag) — End tags must have the following format:
  - 1. The first character of an end tag must be a U+003C LESS-THAN SIGN character (<).
  - 2. The second character of an end tag must be a U+002F SOLIDUS character (/).
  - 3. The next few characters of an end tag must be the element's tag name.
  - 4. After the tag name, there may be one or more ASCII whitespace.
  - 5. Finally, end tags must be closed by a U+003E GREATER-THAN SIGN character (>).

##### [13.1.2.3 Attributes](https://html.spec.whatwg.org/#attributes-2)

Status: n/a (author conformance: the parser is 13.2)
<!-- definitions: syntax-attributes syntax-attribute-name syntax-attribute-value -->

##### [13.1.2.4 Optional tags](https://html.spec.whatwg.org/#optional-tags)

Status: n/a (author conformance: the parser is 13.2)
<!-- definitions: syntax-tag-omission -->

##### [13.1.2.5 Restrictions on content models](https://html.spec.whatwg.org/#element-restrictions)

Status: n/a (author conformance: the parser is 13.2)

##### [13.1.2.6 Restrictions on the contents of raw text and escapable raw text elements](https://html.spec.whatwg.org/#cdata-rcdata-restrictions)

Status: n/a (author conformance: the parser is 13.2)

#### [13.1.3 Text](https://html.spec.whatwg.org/#text-2)

Status: n/a (author conformance: the parser is 13.2)
<!-- definitions: syntax-text -->

##### [13.1.3.1 Newlines](https://html.spec.whatwg.org/#newlines)

Status: n/a (author conformance: the parser is 13.2)
<!-- definitions: syntax-newlines -->

#### [13.1.4 Character references](https://html.spec.whatwg.org/#character-references)

Status: n/a (author conformance: the parser is 13.2)
<!-- definitions: syntax-charref -->

- [x] [ambiguous ampersand](https://html.spec.whatwg.org/#syntax-ambiguous-ampersand) — An ambiguous ampersand is a U+0026 AMPERSAND character (&) that is followed by one or more ASCII alphanumerics, followed by a U+003B SEMICOLON character (;), where these characters do not match any of the names given in the named character references section.

#### [13.1.5 CDATA sections](https://html.spec.whatwg.org/#cdata-sections)

Status: n/a (author conformance: the parser is 13.2)

- [x] [CDATA sections](https://html.spec.whatwg.org/#syntax-cdata) — CDATA sections must consist of the following components, in this order:
  - 1. The string "<![CDATA[".
  - 2. Optionally, text, with the additional restriction that the text must not contain the string "]]>".
  - 3. The string "]]>".

#### [13.1.6 Comments](https://html.spec.whatwg.org/#comments)

Status: n/a (author conformance: the parser is 13.2)

- [x] [Comments](https://html.spec.whatwg.org/#syntax-comments) — Comments must have the following format:
  - 1. The string "<!--".
  - 2. Optionally, text, with the additional restriction that the text must not start with the string ">", nor start with the string "->", nor contain the strings "<!--", "-->", or "--!>", nor end with the string "<!-".
  - 3. The string "-->".

#### [13.1.7 Processing instructions](https://html.spec.whatwg.org/#processing-instructions)

Status: n/a (author conformance: the parser is 13.2)

- [x] [Processing instructions](https://html.spec.whatwg.org/#syntax-processing-instruction) — Processing instructions must have the following format:
  - 1. "<?".
  - 2. A target.
  - 3. Either of the following two options: Zero or more ASCII whitespace.; One or more ASCII whitespace, followed by data.
  - 4. Optionally, a U+003F (?).
  - 5. A U+003E (>).

- [x] [target](https://html.spec.whatwg.org/#syntax-pi-target) — The target consists of ASCII alphanumeric, U+002D (-), and U+005F (_) code points. It must start with an ASCII alpha or U+005F (_) and must not be an ASCII case-insensitive match for "xml" or "xml-stylesheet".

- [x] [data](https://html.spec.whatwg.org/#syntax-pi-data) — The data is text, with the additional restriction that the text must not contain U+003E (>), nor end with U+003F (?).
