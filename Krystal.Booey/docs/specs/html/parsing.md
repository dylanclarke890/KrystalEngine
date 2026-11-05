# HTML review checklist: 13.2 Parsing HTML documents

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 84 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html parsing` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [13.2 Parsing HTML documents](https://html.spec.whatwg.org/#parsing)

Status: reviewed (2026-11-02)
> `DocumentParser`, which owns the `InputStream`, `Tokenizer` and `TreeBuilder`.
<!-- definitions: html-parser -->

#### [13.2.1 Overview of the parsing model](https://html.spec.whatwg.org/#overview-of-the-parsing-model)

Status: n/a (overview)
<!-- definitions: script-nesting-level parser-pause-flag -->

#### [13.2.2 Parse errors](https://html.spec.whatwg.org/#parse-errors)

Status: reviewed (2026-11-02)
> `ParseError` names the tokenizer's errors; parse errors aren't reported (see `KNOWN-ISSUES.md`).

#### [13.2.3 The input byte stream](https://html.spec.whatwg.org/#the-input-byte-stream)

Status: reviewed (2026-11-02)
> Documents decode UTF-8 only (`DocumentWriter`, see `DIFFERENCES.md`).
<!-- definitions: concept-encoding-confidence -->

##### [13.2.3.1 Parsing with a known character encoding](https://html.spec.whatwg.org/#parsing-with-a-known-character-encoding)

Status: reviewed (2026-11-02)
> Documents decode UTF-8 only (`DocumentWriter`, see `DIFFERENCES.md`).
<!-- definitions: a-known-definite-encoding -->

##### [13.2.3.2 Determining the character encoding](https://html.spec.whatwg.org/#determining-the-character-encoding)

Status: reviewed (2026-11-02)
> `DocumentWriter` removes a UTF-8 BOM and decodes UTF-8 only; there's no prescan (see `DIFFERENCES.md`).
<!-- definitions: prescan-end-condition -->

- [x] [encoding sniffing algorithm](https://html.spec.whatwg.org/#encoding-sniffing-algorithm) — User agents must use the following algorithm, called the encoding sniffing algorithm, to determine the character encoding to use when decoding a document in the first pass. This algorithm takes as input any out-of-band metadata available to the user agent (e.g. the Content-Type metadata of the docu…
  - 1. If the result of BOM sniffing is an encoding, return that encoding with confidence certain.
  - 2. If the user has explicitly instructed the user agent to override the document's character encoding with a specific encoding, optionally return that encoding with the confidence certain.
  - 3. The user agent may wait for more bytes of the resource to be available, either in this step or at any later step in this algorithm. For instance, a user agent might wait 500ms or 1024 bytes, whichever came first. In general preparsing the source to find the encoding improves performance, as it reduces the need to throw away the data structures used when parsing upon finding the encoding informati…
  - 4. If the transport layer specifies a character encoding, and it is supported, return that encoding with the confidence certain.
  - 5. Optionally, prescan the byte stream to determine its encoding, with the end condition being when the user agent decides that scanning further bytes would not be efficient. User agents are encouraged to only prescan the first 1024 bytes. User agents may decide that scanning any bytes is not efficient, in which case these substeps are entirely skipped. The aforementioned algorithm returns either a…
  - 6. If the HTML parser for which this algorithm is being run is associated with a Document d whose container document is non-null:
    - 6a. Let parentDocument be d's container document.
    - 6b. If parentDocument's origin is same origin with d's origin and parentDocument's character encoding is not UTF-16BE/LE, then return parentDocument's character encoding, with the confidence tentative.
  - 7. Otherwise, if the user agent has information on the likely encoding for this page, e.g. based on the encoding of the page when it was last visited, then return that encoding, with the confidence tentative.
  - 8. The user agent may attempt to autodetect the character encoding from applying frequency analysis or other algorithms to the data stream. Such algorithms may use information about the resource other than the resource's contents, including the address of the resource. If autodetection succeeds in determining a character encoding, and that encoding is a supported encoding, then return that encoding,…
  - 9. Otherwise, return an implementation-defined or user-specified default character encoding, with the confidence tentative. In controlled environments or in environments where the encoding of documents can be prescribed (for example, for user agents intended for dedicated use in new networks), the comprehensive UTF-8 encoding is suggested. In other environments, the default encoding is typically dep…

- [x] [prescan a byte stream to determine its encoding](https://html.spec.whatwg.org/#prescan-a-byte-stream-to-determine-its-encoding) — When an algorithm requires a user agent to prescan a byte stream to determine its encoding, given some defined end condition, then it must run the following steps. If at any point during these steps (including during instances of the get an attribute algorithm invoked by this one) the user agent ei…
  - 1. Let position be a pointer to a byte in the input byte stream, initially pointing at the first byte.
  - 2. Prescan for UTF-16 XML declarations: If position points to: A sequence of bytes starting with: 0x3C, 0x0, 0x3F, 0x0, 0x78, 0x0 (case-sensitive UTF-16 little-endian '<?x')Return UTF-16LE.A sequence of bytes starting with: 0x0, 0x3C, 0x0, 0x3F, 0x0, 0x78 (case-sensitive UTF-16 big-endian '<?x')Return UTF-16BE.
  - 3. Loop: If position points to: A sequence of bytes starting with: 0x3C 0x21 0x2D 0x2D (`<!--`) Advance the position pointer so that it points at the first 0x3E byte which is preceded by two 0x2D bytes (i.e. at the end of an ASCII '-->' sequence) and comes after the 0x3C byte that was found. (The two 0x2D bytes can be the same as those in the '<!--' sequence.) A sequence of bytes starting with: 0x3C…
    - 3a. Advance the position pointer so that it points at the next 0x09, 0x0A, 0x0C, 0x0D, 0x20, or 0x2F byte (the one in sequence of characters matched above).
    - 3b. Let attribute list be an empty list of strings.
    - 3c. Let got pragma be false.
    - 3d. Let need pragma be null.
    - 3e. Let charset be the null value (which, for the purposes of this algorithm, is distinct from an unrecognized encoding or the empty string).
    - 3f. Attributes: Get an attribute and its value. If no attribute was sniffed, then jump to the processing step below.
    - 3g. If the attribute's name is already in attribute list, then return to the step labeled attributes.
    - 3h. Add the attribute's name to attribute list.
    - 3i. Run the appropriate step from the following list, if one applies: If the attribute's name is "http-equiv"If the attribute's value is "content-type", then set got pragma to true.If the attribute's name is "content"Apply the algorithm for extracting a character encoding from a meta element, giving the attribute's value as the string to parse. If a character encoding is returned, and if charset is s…
    - 3j. Return to the step labeled attributes.
    - 3k. Processing: If need pragma is null, then jump to the step below labeled next byte.
    - 3l. If need pragma is true but got pragma is false, then jump to the step below labeled next byte.
    - 3m. If charset is failure, then jump to the step below labeled next byte.
    - 3n. If charset is UTF-16BE/LE, then set charset to UTF-8.
    - 3o. If charset is x-user-defined, then set charset to windows-1252.
    - 3p. Return charset.
    - 3a. Advance the position pointer so that it points at the next 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), 0x20 (SP), or 0x3E (>) byte.
    - 3b. Repeatedly get an attribute until no further attributes can be found, then jump to the step below labeled next byte.
  - 4. Next byte: Move position so it points at the next byte in the input byte stream, and return to the step above labeled loop.

- [x] [get an attribute](https://html.spec.whatwg.org/#concept-get-attributes-when-sniffing) — When the prescan a byte stream to determine its encoding algorithm says to get an attribute, it means doing this:
  - 1. If the byte at position is one of 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), 0x20 (SP), or 0x2F (/), then advance position to the next byte and redo this step.
  - 2. If the byte at position is 0x3E (>), then abort the get an attribute algorithm. There isn't one.
  - 3. Otherwise, the byte at position is the start of the attribute name. Let attribute name and attribute value be the empty string.
  - 4. Process the byte at position as follows: If it is 0x3D (=), and the attribute name is longer than the empty stringAdvance position to the next byte and jump to the step below labeled value.If it is 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), or 0x20 (SP)Jump to the step below labeled spaces.If it is 0x2F (/) or 0x3E (>)Abort the get an attribute algorithm. The attribute's name is the value of att…
  - 5. Advance position to the next byte and return to the previous step.
  - 6. Spaces: If the byte at position is one of 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), or 0x20 (SP), then advance position to the next byte, then, repeat this step.
  - 7. If the byte at position is not 0x3D (=), abort the get an attribute algorithm. The attribute's name is the value of attribute name, its value is the empty string.
  - 8. Advance position past the 0x3D (=) byte.
  - 9. Value: If the byte at position is one of 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), or 0x20 (SP), then advance position to the next byte, then, repeat this step.
  - 10. Process the byte at position as follows: If it is 0x22 (") or 0x27 (') If it is 0x3E (>)Abort the get an attribute algorithm. The attribute's name is the value of attribute name, its value is the empty string.If it is in the range 0x41 (A) to 0x5A (Z)Append a code point b+0x20 to attribute value (where b is the value of the byte at position). Advance position to the next byte.Anything elseAppend…
    - 10a. Let b be the value of the byte at position.
    - 10b. Quote loop: Advance position to the next byte.
    - 10c. If the value of the byte at position is the value of b, then advance position to the next byte and abort the "get an attribute" algorithm. The attribute's name is the value of attribute name, and its value is the value of attribute value.
    - 10d. Otherwise, if the value of the byte at position is in the range 0x41 (A) to 0x5A (Z), then append a code point to attribute value whose value is 0x20 more than the value of the byte at position.
    - 10e. Otherwise, append a code point to attribute value whose value is the same as the value of the byte at position.
    - 10f. Return to the step above labeled quote loop.
  - 11. Process the byte at position as follows: If it is 0x09 (HT), 0x0A (LF), 0x0C (FF), 0x0D (CR), 0x20 (SP), or 0x3E (>)Abort the get an attribute algorithm. The attribute's name is the value of attribute name and its value is the value of attribute value.If it is in the range 0x41 (A) to 0x5A (Z)Append a code point b+0x20 to attribute value (where b is the value of the byte at position).Anything els…
  - 12. Advance position to the next byte and return to the previous step.

- [x] [get an XML encoding](https://html.spec.whatwg.org/#concept-get-xml-encoding-when-sniffing) — When the prescan a byte stream to determine its encoding algorithm is aborted without returning an encoding, get an XML encoding means doing this.
  - 1. Let encodingPosition be a pointer to the start of the stream.
  - 2. If encodingPosition does not point to the start of a byte sequence 0x3C, 0x3F, 0x78, 0x6D, 0x6C (`<?xml`), then return failure.
  - 3. Let xmlDeclarationEnd be a pointer to the next byte in the input byte stream which is 0x3E (>). If there is no such byte, then return failure.
  - 4. Set encodingPosition to the position of the first occurrence of the subsequence of bytes 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67 (`encoding`) at or after the current encodingPosition and before xmlDeclarationEnd. If there is no such sequence, then return failure.
  - 5. Advance encodingPosition past the 0x67 (g) byte.
  - 6. While the byte at encodingPosition is less than or equal to 0x20 (i.e., it is either an ASCII space or control character), advance encodingPosition to the next byte.
  - 7. If the byte at encodingPosition is not 0x3D (=), then return failure.
  - 8. Advance encodingPosition to the next byte.
  - 9. While the byte at encodingPosition is less than or equal to 0x20 (i.e., it is either an ASCII space or control character), advance encodingPosition to the next byte.
  - 10. Let quoteMark be the byte at encodingPosition.
  - 11. If quoteMark is not either 0x22 (") or 0x27 ('), then return failure.
  - 12. Advance encodingPosition to the next byte.
  - 13. Let encodingEndPosition be the position of the next occurrence of quoteMark at or after encodingPosition. If quoteMark does not occur again, then return failure.
  - 14. Let potentialEncoding be the sequence of the bytes between encodingPosition (inclusive) and encodingEndPosition (exclusive).
  - 15. If potentialEncoding contains one or more bytes whose byte value is 0x20 or below, then return failure.
  - 16. Let encoding be the result of getting an encoding given potentialEncoding isomorphic decoded.
  - 17. If the encoding is UTF-16BE/LE, then change it to UTF-8.
  - 18. Return encoding.

##### [13.2.3.3 Character encodings](https://html.spec.whatwg.org/#character-encodings)

Status: reviewed (2026-11-02)
> Documents decode UTF-8 only (`DocumentWriter`, see `DIFFERENCES.md`).
<!-- definitions: utf-8 iso-8859-2 iso-8859-7 iso-8859-8 windows-874 windows-1250 windows-1251 windows-1252 windows-1254 windows-1255 windows-1256 windows-1257 windows-1258 gbk big5 iso-2022-jp shift_jis euc-kr utf-16be utf-16le utf-16-encoding x-user-defined -->

##### [13.2.3.4 Changing the encoding while parsing](https://html.spec.whatwg.org/#changing-the-encoding-while-parsing)

Status: reviewed (2026-11-02)
> Skipped: documents are UTF-8 only (see `DIFFERENCES.md`).

- [x] [change the encoding](https://html.spec.whatwg.org/#change-the-encoding) — When the parser requires the user agent to change the encoding, it must run the following steps. This might happen if the encoding sniffing algorithm described above failed to find a character encoding, or if it found a character encoding that was not the actual encoding of the file.
  - 1. If the encoding that is already being used to interpret the input stream is UTF-16BE/LE, then set the confidence to certain and return. The new encoding is ignored; if it was anything but the same encoding, then it would be clearly incorrect.
  - 2. If the new encoding is UTF-16BE/LE, then change it to UTF-8.
  - 3. If the new encoding is x-user-defined, then change it to windows-1252.
  - 4. If the new encoding is identical or equivalent to the encoding that is already being used to interpret the input stream, then set the confidence to certain and return. This happens when the encoding information found in the file matches what the encoding sniffing algorithm determined to be the encoding, and in the second pass through the parser if the first pass found that the encoding sniffing a…
  - 5. If all the bytes up to the last byte converted by the current decoder have the same Unicode interpretations in both the current encoding and the new encoding, and if the user agent supports changing the converter on the fly, then the user agent may change to the new converter for the encoding on the fly. Set the document's character encoding and the encoding used to convert the input stream to th…
  - 6. Otherwise, restart the navigate algorithm, with historyHandling set to "replace" and other inputs kept the same, but this time skip the encoding sniffing algorithm and instead just set the encoding to the new encoding and the confidence to certain. Whenever possible, this should be done without actually contacting the network layer (the bytes should be re-parsed from memory), even if, e.g., the d…

##### [13.2.3.5 Preprocessing the input stream](https://html.spec.whatwg.org/#preprocessing-the-input-stream)

Status: reviewed (2026-11-02)
> `InputStream` normalizes newlines; the tokenizer replaces nulls in each state.
<!-- definitions: input-stream current-input-character insertion-point explicit-eof-character -->

- [x] [next input character](https://html.spec.whatwg.org/#next-input-character) — The next input character is the first character in the input stream that has not yet been consumed or explicitly ignored by the requirements in this section. Initially, the next input character is the first character in the input. The current input character is the last character to have been consumed.

#### [13.2.4 Parse state](https://html.spec.whatwg.org/#parse-state)

Status: reviewed (2026-11-02)

##### [13.2.4.1 The insertion mode](https://html.spec.whatwg.org/#the-insertion-mode)

Status: reviewed (2026-11-02)
> `InsertionMode`, the template insertion modes and `ResetInsertionModeAppropriately` in `TreeBuilder`.
<!-- definitions: insertion-mode original-insertion-mode stack-of-template-insertion-modes current-template-insertion-mode -->

- [x] [using the rules for](https://html.spec.whatwg.org/#using-the-rules-for) — Several of these modes, namely "in head", "in body", and "in table", are special, in that the other modes defer to them at various times. When the algorithm below says that the user agent is to do something "using the rules for the m insertion mode", where m is one of these modes, the user agent must use the rules described under the m insertion mode's section, but must leave the insertion mode unchanged unless the rules in m themselves switch the insertion mode to a new value.

- [x] [reset the insertion mode appropriately](https://html.spec.whatwg.org/#reset-the-insertion-mode-appropriately) — When the steps below require the UA to reset the insertion mode appropriately, it means the UA must follow these steps:
  - 1. Let last be false.
  - 2. Let node be the last node in the stack of open elements.
  - 3. Loop: If node is the first node in the stack of open elements, then set last to true, and, if the parser's fragment context element is non-null, then set node to that element (fragment case).
  - 4. If node is a td or th element and last is false, then switch the insertion mode to "in cell" and return.
  - 5. If node is a tr element, then switch the insertion mode to "in row" and return.
  - 6. If node is a tbody, thead, or tfoot element, then switch the insertion mode to "in table body" and return.
  - 7. If node is a caption element, then switch the insertion mode to "in caption" and return.
  - 8. If node is a colgroup element, then switch the insertion mode to "in column group" and return.
  - 9. If node is a table element, then switch the insertion mode to "in table" and return.
  - 10. If node is a template element, then switch the insertion mode to the current template insertion mode and return.
  - 11. If node is a head element and last is false, then switch the insertion mode to "in head" and return.
  - 12. If node is a body element, then switch the insertion mode to "in body" and return.
  - 13. If node is a frameset element, then switch the insertion mode to "in frameset" and return. (fragment case)
  - 14. If node is an html element, run these substeps:
    - 14a. If the head element pointer is null, switch the insertion mode to "before head" and return. (fragment case)
    - 14b. Otherwise, the head element pointer is not null, switch the insertion mode to "after head" and return.
  - 15. If last is true, then switch the insertion mode to "in body" and return. (fragment case)
  - 16. Let node now be the node before node in the stack of open elements.
  - 17. Return to the step labeled loop.

##### [13.2.4.2 The stack of open elements](https://html.spec.whatwg.org/#the-stack-of-open-elements)

Status: reviewed (2026-11-02)
> `OpenElementStack`, with its scopes and `StackItem`.
<!-- definitions: stack-of-open-elements special formatting ordinary -->

- [x] [current node](https://html.spec.whatwg.org/#current-node) — The current node is the bottommost node in this stack of open elements.

- [x] [adjusted current node](https://html.spec.whatwg.org/#adjusted-current-node) — The adjusted current node is the parser's fragment context element if that element is non-null and the stack of open elements has only one element in it (fragment case); otherwise, the adjusted current node is the current node.

- [x] [have an element target node in a specific scope](https://html.spec.whatwg.org/#has-an-element-in-the-specific-scope) — The stack of open elements is said to have an element target node in a specific scope consisting of a list of element types list when the following algorithm terminates in a match state:
  - 1. Initialize node to be the current node (the bottommost node of the stack).
  - 2. If node is target node, terminate in a match state.
  - 3. Otherwise, if node is one of the element types in list, terminate in a failure state.
  - 4. Otherwise, set node to the previous entry in the stack of open elements and return to step 2. (This will never fail, since the loop will always terminate in the previous step if the top of the stack — an html element — is reached.)

- [x] [have a particular element in scope](https://html.spec.whatwg.org/#has-an-element-in-scope) — The stack of open elements is said to have a particular element in scope when it has that element in the specific scope consisting of the following element types:

- [x] [have a particular element in list item scope](https://html.spec.whatwg.org/#has-an-element-in-list-item-scope) — The stack of open elements is said to have a particular element in list item scope when it has that element in the specific scope consisting of the following element types:

- [x] [have a particular element in button scope](https://html.spec.whatwg.org/#has-an-element-in-button-scope) — The stack of open elements is said to have a particular element in button scope when it has that element in the specific scope consisting of the following element types:

- [x] [have a particular element in table scope](https://html.spec.whatwg.org/#has-an-element-in-table-scope) — The stack of open elements is said to have a particular element in table scope when it has that element in the specific scope consisting of the following element types:

##### [13.2.4.3 The list of active formatting elements](https://html.spec.whatwg.org/#the-list-of-active-formatting-elements)

Status: reviewed (2026-11-02)
> `FormattingElementList`; reconstruction is `TreeBuilder::ReconstructActiveFormattingElements`.
<!-- definitions: list-of-active-formatting-elements concept-parser-marker -->

- [x] [push onto the list of active formatting elements](https://html.spec.whatwg.org/#push-onto-the-list-of-active-formatting-elements) — When the steps below require the UA to push onto the list of active formatting elements an element element, the UA must perform the following steps:
  - 1. If there are already three elements in the list of active formatting elements after the last marker, if any, or anywhere in the list if there are no markers, that have the same tag name, namespace, and attributes as element, then remove the earliest such element from the list of active formatting elements. For these purposes, the attributes must be compared as they were on the tokens for which th…
  - 2. Add element to the list of active formatting elements.

- [x] [reconstruct the active formatting elements](https://html.spec.whatwg.org/#reconstruct-the-active-formatting-elements) — When the steps below require the UA to reconstruct the active formatting elements, the UA must perform the following steps:
  - 1. If there are no entries in the list of active formatting elements, then there is nothing to reconstruct; stop this algorithm.
  - 2. If the last (most recently added) entry in the list of active formatting elements is a marker, or if it is an element that is in the stack of open elements, then there is nothing to reconstruct; stop this algorithm.
  - 3. Let entry be the last (most recently added) element in the list of active formatting elements.
  - 4. Rewind: If there are no entries before entry in the list of active formatting elements, then jump to the step labeled create.
  - 5. Let entry be the entry one earlier than entry in the list of active formatting elements.
  - 6. If entry is neither a marker nor an element that is also in the stack of open elements, go to the step labeled rewind.
  - 7. Advance: Let entry be the element one later than entry in the list of active formatting elements.
  - 8. Create: Insert an HTML element for the token for which the element entry was created, to obtain new element.
  - 9. Replace the entry for entry in the list with an entry for new element.
  - 10. If the entry for new element in the list of active formatting elements is not the last entry in the list, return to the step labeled advance.

- [x] [clear the list of active formatting elements up to the last marker](https://html.spec.whatwg.org/#clear-the-list-of-active-formatting-elements-up-to-the-last-marker) — When the steps below require the UA to clear the list of active formatting elements up to the last marker, the UA must perform the following steps:
  - 1. Let entry be the last (most recently added) entry in the list of active formatting elements.
  - 2. Remove entry from the list of active formatting elements.
  - 3. If entry was a marker, then stop the algorithm at this point. The list has been cleared up to the last marker.
  - 4. Go to step 1.

##### [13.2.4.4 The element pointers](https://html.spec.whatwg.org/#the-element-pointers)

Status: reviewed (2026-11-02)
> `_head` and `_form` in `TreeBuilder`.
<!-- definitions: head-element-pointer form-element-pointer parsing-template-contents -->

##### [13.2.4.5 Other parsing state flags](https://html.spec.whatwg.org/#other-parsing-state-flags)

Status: reviewed (2026-11-02)
> The scripting mode (`ParserScriptingMode`) and `_framesetOk`; there's no scripting.
<!-- definitions: fragment-context-element allow-declarative-shadow-roots scripting-mode parser-scripting-mode scripting-mode-normal scripting-mode-disabled scripting-mode-inert scripting-mode-fragment frameset-ok-flag parser-sanitizer-configuration sanitize insertion-target-redirection-map -->

#### [13.2.5 Tokenization](https://html.spec.whatwg.org/#tokenization)

Status: reviewed (2026-11-02)
> `Tokenizer`, one state per spec state, tested state by state (`tests/HTML/Parser/Tokenizer.cpp`).
> Fixed: a near-miss end tag in RCDATA, RAWTEXT or script data followed by whitespace or a solidus (`</scrip `) asserted, as the state switched before checking for an appropriate end tag; and a numeric character reference too large for 64 bits overflowed back into the Unicode range.
<!-- definitions: reconsume temporary-buffer return-state force-quirks-flag self-closing-flag -->

- [x] [acknowledged](https://html.spec.whatwg.org/#acknowledge-self-closing-flag) — When a start tag token is emitted with its self-closing flag set, if the flag is not acknowledged when it is processed by the tree construction stage, that is a non-void-html-element-start-tag-with-trailing-solidus parse error.

- [x] [appropriate end tag token](https://html.spec.whatwg.org/#appropriate-end-tag-token) — An appropriate end tag token is an end tag token whose tag name matches the tag name of the last start tag to have been emitted from this tokenizer, if any. If no start tag has been emitted from this tokenizer, then no end tag token is appropriate.

- [x] [consumed as part of an attribute](https://html.spec.whatwg.org/#charref-in-attribute) — A character reference is said to be consumed as part of an attribute if the return state is either attribute value (double-quoted) state, attribute value (single-quoted) state, or attribute value (unquoted) state.

- [x] [flush code points consumed as a character reference](https://html.spec.whatwg.org/#flush-code-points-consumed-as-a-character-reference) — When a state says to flush code points consumed as a character reference, it means that for each code point in the temporary buffer (in the order they were added to the buffer), the user agent must append the code point from the buffer to the current attribute's value if the character reference was consumed as part of an attribute, or emit the code point as a character token otherwise.

- [x] [convert the temporary buffer to a comment](https://html.spec.whatwg.org/#convert-the-temporary-buffer-to-a-comment) — To convert the temporary buffer to a comment, create a comment token whose data is the concatenation of "?" and the code points in the temporary buffer, in the order they were added to the buffer.

##### [13.2.5.1 Data state](https://html.spec.whatwg.org/#data-state)

Status: reviewed (2026-11-02)

##### [13.2.5.2 RCDATA state](https://html.spec.whatwg.org/#rcdata-state)

Status: reviewed (2026-11-02)

##### [13.2.5.3 RAWTEXT state](https://html.spec.whatwg.org/#rawtext-state)

Status: reviewed (2026-11-02)

##### [13.2.5.4 Script data state](https://html.spec.whatwg.org/#script-data-state)

Status: reviewed (2026-11-02)

##### [13.2.5.5 PLAINTEXT state](https://html.spec.whatwg.org/#plaintext-state)

Status: reviewed (2026-11-02)

##### [13.2.5.6 Tag open state](https://html.spec.whatwg.org/#tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.7 End tag open state](https://html.spec.whatwg.org/#end-tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.8 Tag name state](https://html.spec.whatwg.org/#tag-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.9 RCDATA less-than sign state](https://html.spec.whatwg.org/#rcdata-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.10 RCDATA end tag open state](https://html.spec.whatwg.org/#rcdata-end-tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.11 RCDATA end tag name state](https://html.spec.whatwg.org/#rcdata-end-tag-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.12 RAWTEXT less-than sign state](https://html.spec.whatwg.org/#rawtext-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.13 RAWTEXT end tag open state](https://html.spec.whatwg.org/#rawtext-end-tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.14 RAWTEXT end tag name state](https://html.spec.whatwg.org/#rawtext-end-tag-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.15 Script data less-than sign state](https://html.spec.whatwg.org/#script-data-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.16 Script data end tag open state](https://html.spec.whatwg.org/#script-data-end-tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.17 Script data end tag name state](https://html.spec.whatwg.org/#script-data-end-tag-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.18 Script data escape start state](https://html.spec.whatwg.org/#script-data-escape-start-state)

Status: reviewed (2026-11-02)

##### [13.2.5.19 Script data escape start dash state](https://html.spec.whatwg.org/#script-data-escape-start-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.20 Script data escaped state](https://html.spec.whatwg.org/#script-data-escaped-state)

Status: reviewed (2026-11-02)

##### [13.2.5.21 Script data escaped dash state](https://html.spec.whatwg.org/#script-data-escaped-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.22 Script data escaped dash dash state](https://html.spec.whatwg.org/#script-data-escaped-dash-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.23 Script data escaped less-than sign state](https://html.spec.whatwg.org/#script-data-escaped-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.24 Script data escaped end tag open state](https://html.spec.whatwg.org/#script-data-escaped-end-tag-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.25 Script data escaped end tag name state](https://html.spec.whatwg.org/#script-data-escaped-end-tag-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.26 Script data double escape start state](https://html.spec.whatwg.org/#script-data-double-escape-start-state)

Status: reviewed (2026-11-02)

##### [13.2.5.27 Script data double escaped state](https://html.spec.whatwg.org/#script-data-double-escaped-state)

Status: reviewed (2026-11-02)

##### [13.2.5.28 Script data double escaped dash state](https://html.spec.whatwg.org/#script-data-double-escaped-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.29 Script data double escaped dash dash state](https://html.spec.whatwg.org/#script-data-double-escaped-dash-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.30 Script data double escaped less-than sign state](https://html.spec.whatwg.org/#script-data-double-escaped-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.31 Script data double escape end state](https://html.spec.whatwg.org/#script-data-double-escape-end-state)

Status: reviewed (2026-11-02)

##### [13.2.5.32 Before attribute name state](https://html.spec.whatwg.org/#before-attribute-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.33 Attribute name state](https://html.spec.whatwg.org/#attribute-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.34 After attribute name state](https://html.spec.whatwg.org/#after-attribute-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.35 Before attribute value state](https://html.spec.whatwg.org/#before-attribute-value-state)

Status: reviewed (2026-11-02)

##### [13.2.5.36 Attribute value (double-quoted) state](https://html.spec.whatwg.org/#attribute-value-%28double-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.37 Attribute value (single-quoted) state](https://html.spec.whatwg.org/#attribute-value-%28single-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.38 Attribute value (unquoted) state](https://html.spec.whatwg.org/#attribute-value-%28unquoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.39 After attribute value (quoted) state](https://html.spec.whatwg.org/#after-attribute-value-%28quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.40 Self-closing start tag state](https://html.spec.whatwg.org/#self-closing-start-tag-state)

Status: reviewed (2026-11-02)

##### [13.2.5.41 Bogus comment state](https://html.spec.whatwg.org/#bogus-comment-state)

Status: reviewed (2026-11-02)

##### [13.2.5.42 Markup declaration open state](https://html.spec.whatwg.org/#markup-declaration-open-state)

Status: reviewed (2026-11-02)

##### [13.2.5.43 Comment start state](https://html.spec.whatwg.org/#comment-start-state)

Status: reviewed (2026-11-02)

##### [13.2.5.44 Comment start dash state](https://html.spec.whatwg.org/#comment-start-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.45 Comment state](https://html.spec.whatwg.org/#comment-state)

Status: reviewed (2026-11-02)

##### [13.2.5.46 Comment less-than sign state](https://html.spec.whatwg.org/#comment-less-than-sign-state)

Status: reviewed (2026-11-02)

##### [13.2.5.47 Comment less-than sign bang state](https://html.spec.whatwg.org/#comment-less-than-sign-bang-state)

Status: reviewed (2026-11-02)

##### [13.2.5.48 Comment less-than sign bang dash state](https://html.spec.whatwg.org/#comment-less-than-sign-bang-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.49 Comment less-than sign bang dash dash state](https://html.spec.whatwg.org/#comment-less-than-sign-bang-dash-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.50 Comment end dash state](https://html.spec.whatwg.org/#comment-end-dash-state)

Status: reviewed (2026-11-02)

##### [13.2.5.51 Comment end state](https://html.spec.whatwg.org/#comment-end-state)

Status: reviewed (2026-11-02)

##### [13.2.5.52 Comment end bang state](https://html.spec.whatwg.org/#comment-end-bang-state)

Status: reviewed (2026-11-02)

##### [13.2.5.53 DOCTYPE state](https://html.spec.whatwg.org/#doctype-state)

Status: reviewed (2026-11-02)

##### [13.2.5.54 Before DOCTYPE name state](https://html.spec.whatwg.org/#before-doctype-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.55 DOCTYPE name state](https://html.spec.whatwg.org/#doctype-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.56 After DOCTYPE name state](https://html.spec.whatwg.org/#after-doctype-name-state)

Status: reviewed (2026-11-02)

##### [13.2.5.57 After DOCTYPE public keyword state](https://html.spec.whatwg.org/#after-doctype-public-keyword-state)

Status: reviewed (2026-11-02)

##### [13.2.5.58 Before DOCTYPE public identifier state](https://html.spec.whatwg.org/#before-doctype-public-identifier-state)

Status: reviewed (2026-11-02)

##### [13.2.5.59 DOCTYPE public identifier (double-quoted) state](https://html.spec.whatwg.org/#doctype-public-identifier-%28double-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.60 DOCTYPE public identifier (single-quoted) state](https://html.spec.whatwg.org/#doctype-public-identifier-%28single-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.61 After DOCTYPE public identifier state](https://html.spec.whatwg.org/#after-doctype-public-identifier-state)

Status: reviewed (2026-11-02)

##### [13.2.5.62 Between DOCTYPE public and system identifiers state](https://html.spec.whatwg.org/#between-doctype-public-and-system-identifiers-state)

Status: reviewed (2026-11-02)

##### [13.2.5.63 After DOCTYPE system keyword state](https://html.spec.whatwg.org/#after-doctype-system-keyword-state)

Status: reviewed (2026-11-02)

##### [13.2.5.64 Before DOCTYPE system identifier state](https://html.spec.whatwg.org/#before-doctype-system-identifier-state)

Status: reviewed (2026-11-02)

##### [13.2.5.65 DOCTYPE system identifier (double-quoted) state](https://html.spec.whatwg.org/#doctype-system-identifier-%28double-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.66 DOCTYPE system identifier (single-quoted) state](https://html.spec.whatwg.org/#doctype-system-identifier-%28single-quoted%29-state)

Status: reviewed (2026-11-02)

##### [13.2.5.67 After DOCTYPE system identifier state](https://html.spec.whatwg.org/#after-doctype-system-identifier-state)

Status: reviewed (2026-11-02)

##### [13.2.5.68 Bogus DOCTYPE state](https://html.spec.whatwg.org/#bogus-doctype-state)

Status: reviewed (2026-11-02)

##### [13.2.5.69 CDATA section state](https://html.spec.whatwg.org/#cdata-section-state)

Status: reviewed (2026-11-02)

##### [13.2.5.70 CDATA section bracket state](https://html.spec.whatwg.org/#cdata-section-bracket-state)

Status: reviewed (2026-11-02)

##### [13.2.5.71 CDATA section end state](https://html.spec.whatwg.org/#cdata-section-end-state)

Status: reviewed (2026-11-02)

##### [13.2.5.72 Processing instruction open state](https://html.spec.whatwg.org/#processing-instruction-open-state)

Status: reviewed (2026-11-02)
> `Tokenizer.hpp`.

##### [13.2.5.73 Processing instruction target state](https://html.spec.whatwg.org/#processing-instruction-target-state)

Status: reviewed (2026-11-02)
> `Tokenizer.hpp`.

- [x] [If target is an ASCII case-insensitive match for "xml" or "xml-stylesheet"](https://html.spec.whatwg.org/#processing-instruction-target-state) — If target is an ASCII case-insensitive match for "xml" or "xml-stylesheet":
  - 1. This is a disallowed-processing-instruction-target parse error.
  - 2. Convert the temporary buffer to a comment.
  - 3. Reconsume in the bogus comment state.

##### [13.2.5.74 After processing instruction target state](https://html.spec.whatwg.org/#after-processing-instruction-target-state)

Status: reviewed (2026-11-02)
> `Tokenizer.hpp`.

##### [13.2.5.75 Processing instruction data state](https://html.spec.whatwg.org/#processing-instruction-data-state)

Status: reviewed (2026-11-02)
> `Tokenizer.hpp`.

##### [13.2.5.76 Processing instruction questionable state](https://html.spec.whatwg.org/#processing-instruction-questionable-state)

Status: reviewed (2026-11-02)
> `Tokenizer.hpp`.

##### [13.2.5.77 Character reference state](https://html.spec.whatwg.org/#character-reference-state)

Status: reviewed (2026-11-02)

##### [13.2.5.78 Named character reference state](https://html.spec.whatwg.org/#named-character-reference-state)

Status: reviewed (2026-11-02)

- [x] [Otherwise](https://html.spec.whatwg.org/#named-character-reference-state) — Otherwise:
  - 1. If the last character matched is not a U+003B SEMICOLON character (;), then this is a missing-semicolon-after-character-reference parse error.
  - 2. Set the temporary buffer to the empty string. Append one or two characters corresponding to the character reference name (as given by the second column of the named character references table) to the temporary buffer.
  - 3. Flush code points consumed as a character reference. Switch to the return state.

##### [13.2.5.79 Ambiguous ampersand state](https://html.spec.whatwg.org/#ambiguous-ampersand-state)

Status: reviewed (2026-11-02)

##### [13.2.5.80 Numeric character reference state](https://html.spec.whatwg.org/#numeric-character-reference-state)

Status: reviewed (2026-11-02)

- [x] [character reference code](https://html.spec.whatwg.org/#character-reference-code) — Set the character reference code to zero (0).

##### [13.2.5.81 Hexadecimal character reference start state](https://html.spec.whatwg.org/#hexadecimal-character-reference-start-state)

Status: reviewed (2026-11-02)

##### [13.2.5.82 Hexadecimal character reference state](https://html.spec.whatwg.org/#hexadecimal-character-reference-state)

Status: reviewed (2026-11-02)

##### [13.2.5.83 Decimal character reference state](https://html.spec.whatwg.org/#decimal-character-reference-state)

Status: reviewed (2026-11-02)

##### [13.2.5.84 Numeric character reference end state](https://html.spec.whatwg.org/#numeric-character-reference-end-state)

Status: reviewed (2026-11-02)

#### [13.2.6 Tree construction](https://html.spec.whatwg.org/#tree-construction)

Status: reviewed (2026-11-02)
> `TreeBuilder::ProcessToken` is the dispatcher.

- [x] [tree construction dispatcher](https://html.spec.whatwg.org/#tree-construction-dispatcher) — As each token is emitted from the tokenizer, the user agent must follow the appropriate steps from the following list, known as the tree construction dispatcher:
  - • If the stack of open elements is empty / If the adjusted current node is an element in the HTML namespace / If the adjusted current node is a MathML text integration point and the token is a start tag whose tag name is neither "mglyph" nor "malignmark" / If the adjusted current node is a MathML text integration point and the token is a character token / If the adjusted current node is a MathML an…
  - • Otherwise: Process the token according to the rules given in the section for parsing tokens in foreign content.

- [x] [next token](https://html.spec.whatwg.org/#next-token) — The next token is the token that is about to be processed by the tree construction dispatcher (even if the token is subsequently just ignored).

- [x] [MathML text integration point](https://html.spec.whatwg.org/#mathml-text-integration-point) — A node is a MathML text integration point if it is one of the following elements:

- [x] [HTML integration point](https://html.spec.whatwg.org/#html-integration-point) — A node is an HTML integration point if it is one of the following elements:

##### [13.2.6.1 Creating and inserting nodes](https://html.spec.whatwg.org/#creating-and-inserting-nodes)

Status: reviewed (2026-11-02)
> `AppropriatePlaceToInsertNode`, `CreateElement`, `InsertForeignElement`, `InsertCharacters`, `InsertComment`, `InsertProcessingInstruction` and the attribute adjusters; content patching is `InsertionLocationInTemplate`. The xmlns parse errors are skipped (see `DIFFERENCES.md`), a resettable element isn't reset again after its attributes have put it in its reset state, and the parser form owner is missing (see `KNOWN-ISSUES.md`).
> Fixed: a plain `xmlns` attribute on a foreign element got the `xmlns` prefix.
<!-- definitions: insertion-location insertion-location-target-parent insertion-location-reference-child foster-parent -->

- [x] [appropriate place for inserting a node](https://html.spec.whatwg.org/#appropriate-place-for-inserting-a-node) — To find the appropriate place for inserting a node, given an optional Node overrideTarget (default null), perform the following steps. They return an insertion location.
  - 1. Let targetParent be overrideTarget if it is not null; otherwise the current node.
  - 2. Let referenceChild be null.
  - 3. If foster parenting is enabled and targetParent is a table, tbody, tfoot, thead, or tr element:
    - 3a. Let lastTemplateOrTable be the last template or table element in the stack of open elements if there is one; otherwise null.
    - 3b. If lastTemplateOrTable is null, then return (the first element in the stack of open elements (the html element), null). (fragment case)
    - 3c. If lastTemplateOrTable is a template element, then set targetParent to lastTemplateOrTable.
    - 3d. Otherwise, if lastTemplateOrTable's parent is not null, then set targetParent to lastTemplateOrTable's parent and set referenceChild to lastTemplateOrTable.
    - 3e. Otherwise, set targetParent to the element immediately above lastTemplateOrTable in the stack of open elements.
  - 4. If targetParent is not a template element, then return (targetParent, referenceChild).
  - 5. Let patchInsertionTarget be targetParent's insertion target.
  - 6. If patchInsertionTarget is null, then return (targetParent's template contents, null).
  - 7. If targetParent's insertion end marker is not null and its parent is patchInsertionTarget, then return (patchInsertionTarget, targetParent's insertion end marker).
  - 8. Return (patchInsertionTarget, null).

- [x] [create an element for a token](https://html.spec.whatwg.org/#create-an-element-for-the-token) — To create an element for a token, given a token token, a string namespace, a Node object intendedParent, and an optional string-or-null prefix (default null):
  - 1. If the active speculative HTML parser is not null, then return the result of creating a speculative mock element given namespace, token's tag name, and token's attributes.
  - 2. Otherwise, if the parser's parser sanitizer configuration is null, optionally create a speculative mock element given namespace, token's tag name, and token's attributes.
  - 3. Let document be intendedParent's node document.
  - 4. Let localName be token's tag name.
  - 5. Let sanitizerConfig be the parser's parser sanitizer configuration.
  - 6. Let sanitizerAction be "Keep".
  - 7. If sanitizerConfig is not null, then set sanitizerAction to the result of getting the sanitizer action for an element name given localName, namespace, and sanitizerConfig.
  - 8. Let is be the value of the "is" attribute in token, if such an attribute exists; otherwise null.
  - 9. If is is non-null, sanitizerConfig is non-null, and the result of checking if a sanitizer config allows an attribute given "is", null, is, localName, namespace, and sanitizerConfig is false, then set is to null.
  - 10. Let registry be the result of looking up a custom element registry given intendedParent.
  - 11. Let creationRegistry be registry if sanitizerAction is "Keep"; otherwise null.
  - 12. Let definition be null if sanitizerAction is not "Keep"; otherwise the result of looking up a custom element definition given registry, namespace, localName, and is.
  - 13. Let willExecuteScript be true if definition is non-null and the parser's fragment context element is null; otherwise false.
  - 14. If willExecuteScript is true:
    - 14a. Increment document's throw-on-dynamic-markup-insertion counter.
    - 14b. If the JavaScript execution context stack is empty, then perform a microtask checkpoint.
    - 14c. Push a new element queue onto document's relevant agent's custom element reactions stack.
  - 15. Let element be the result of creating an element given document, localName, namespace, prefix, is, willExecuteScript, and creationRegistry.
  - 16. If sanitizerAction is "Replace with children", then set element's custom element registry to registry.
  - 17. For each attribute of token's attributes:
    - 17a. If sanitizerConfig is not null:
      - 17a.i. Let attrLocalName be attribute's local name if attribute is a namespaced attribute; otherwise attribute's name.
      - 17a.ii. Let attrNamespace be attribute's namespace if attribute is a namespaced attribute; otherwise null.
      - 17a.iii. If the result of checking if a sanitizer config allows an attribute given attrLocalName, attrNamespace, attribute's value, localName, namespace, and sanitizerConfig is false, then continue.
    - 17b. Append attribute to element.
  - 18. If willExecuteScript is true:
    - 18a. Let queue be the result of popping from document's relevant agent's custom element reactions stack. (This will be the same element queue as was pushed above.)
    - 18b. Invoke custom element reactions in queue.
    - 18c. Decrement document's throw-on-dynamic-markup-insertion counter.
  - 19. If element has an xmlns attribute in the XMLNS namespace whose value is not exactly the same as the element's namespace, that is a parse error. Similarly, if element has an xmlns:xlink attribute in the XMLNS namespace whose value is not the XLink Namespace, that is a parse error.
  - 20. If element is a resettable element and not a form-associated custom element, then invoke its reset algorithm. (This initializes the element's value and checkedness based on the element's attributes.)
  - 21. If element is a form-associated element and not a form-associated custom element, the form element pointer is not null, the parser is not parsing template contents, the parser's fragment context element is null, element is either not listed or doesn't have a form attribute, the intendedParent is in the same tree as the element pointed to by the form element pointer, and the parser's insertion tar…
  - 22. Return element.

- [x] [adjusted insertion location](https://html.spec.whatwg.org/#adjusted-insertion-location) — To compute the adjusted insertion location with an optional insertion location insertionLocation (default null):
  - 1. Let location be insertionLocation if insertionLocation is non-null; otherwise the appropriate place for inserting a node.
  - 2. Let (targetParent, refChild) be location.
  - 3. If the parser's insertion target redirection map[targetParent] does not exist, then return location.
  - 4. Assert: refChild is null.
  - 5. Return the parser's insertion target redirection map[targetParent].

- [x] [insert an element at the adjusted insertion location](https://html.spec.whatwg.org/#insert-an-element-at-the-adjusted-insertion-location) — To insert an element at the adjusted insertion location with an element element, an optional insertion location overrideLocation (default null), and an optional boolean alreadySanitized (default false):
  - 1. Let (targetParent, referenceChild) be the adjusted insertion location given overrideLocation.
  - 2. If any of the following are true: element's parent is non-null;; element is a host-including inclusive ancestor of targetParent; or; targetParent is a Document node that already has an element child, then return.
  - 3. If alreadySanitized is false and the parser's parser sanitizer configuration is not null:
    - 3a. Let action be the result of getting the sanitizer action for an element name given element's local name, element's namespace, and the parser's parser sanitizer configuration.
    - 3b. For each attribute of element's attribute list:
      - 3b.i. Assert: the result of checking if a sanitizer config allows an attribute given attribute's local name, attribute's namespace, attribute's value, element's local name, element's namespace, and the parser's parser sanitizer configuration is true.
    - 3c. Assert: if targetParent is a Document, then action is not "Replace with children".
    - 3d. Switch on action: "Replace with children"Set the parser's insertion target redirection map[element] to (targetParent, referenceChild), and return."Remove"Return."Keep"Do nothing.
  - 4. Assert: ensure pre-insert validity given element, targetParent, referenceChild, and « » does not throw.
  - 5. If the parser's fragment context element is null, then push a new element queue onto element's relevant agent's custom element reactions stack.
  - 6. Insert element into targetParent before referenceChild.
  - 7. If the parser's fragment context element is null, then pop the element queue from element's relevant agent's custom element reactions stack, and invoke custom element reactions in that queue.

- [x] [insert a foreign element](https://html.spec.whatwg.org/#insert-a-foreign-element) — To insert a foreign element, given a token token and a string namespace:
  - 1. Let element be the result of creating an element for the token given token, namespace, and the target parent of the appropriate place for inserting a node.
  - 2. Insert an element at the adjusted insertion location with element.
  - 3. Push element onto the stack of open elements so that it is the new current node.
  - 4. Return element.

- [x] [insert an HTML element](https://html.spec.whatwg.org/#insert-an-html-element) — To insert an HTML element given a token token: insert a foreign element given token and the HTML namespace.

- [x] [adjust MathML attributes](https://html.spec.whatwg.org/#adjust-mathml-attributes) — When the steps below require the user agent to adjust MathML attributes for a token, then, if the token has an attribute named definitionurl, change its name to definitionURL (note the case difference).

- [x] [adjust SVG attributes](https://html.spec.whatwg.org/#adjust-svg-attributes) — When the steps below require the user agent to adjust SVG attributes for a token, then, for each attribute on the token whose attribute name is one of the ones in the first column of the following table, change the attribute's name to the name given in the corresponding cell in the second column. (This fixes the case of SVG attributes that are not all lowercase.)

- [x] [adjust foreign attributes](https://html.spec.whatwg.org/#adjust-foreign-attributes) — When the steps below require the user agent to adjust foreign attributes for a token, then, if any of the attributes on the token match the strings given in the first column of the following table, let the attribute be a namespaced attribute, with the prefix being the string given in the corresponding cell in the second column, the local name being the string given in the corresponding cell in the third column, and the namespace being the namespace given in the corresponding cell in the fourth column. (This fixes the use of namespaced attributes, in particular lang attributes in the XML namespace.)

- [x] [insert a character](https://html.spec.whatwg.org/#insert-a-character) — When the steps below require the user agent to insert a character while processing a token, the user agent must run the following steps:
  - 1. Let data be the characters passed to the algorithm, or, if no characters were explicitly specified, the character of the character token being processed.
  - 2. Let (targetParent, referenceChild) be the adjusted insertion location.
  - 3. If targetParent is a Document node, then return.
  - 4. Let previousSibling be referenceChild's previous sibling if referenceChild is non-null; otherwise targetParent's last child.
  - 5. If previousSibling is a Text node, then append data to previousSibling's data. Otherwise, let text be the result of creating a text node given targetParent's node document and data, and insert text into targetParent before referenceChild.

- [x] [insert a comment](https://html.spec.whatwg.org/#insert-a-comment) — When the steps below require the user agent to insert a comment while processing a comment token, with an optional insertion location insertionLocation (default null), the user agent must run the following steps:
  - 1. If the parser's parser sanitizer configuration is non-null and the parser's parser sanitizer configuration does not allow comments, then return.
  - 2. Let data be the data given in the comment token being processed.
  - 3. Let (targetParent, referenceChild) be the adjusted insertion location given insertionLocation.
  - 4. Let comment be the result of creating a comment node given targetParent's node document and data.
  - 5. Insert comment into targetParent before referenceChild.

- [x] [insert a processing instruction](https://html.spec.whatwg.org/#insert-a-processing-instruction) — When the steps below require the user agent to insert a processing instruction while processing a processing instruction token, with an optional insertion location insertionLocation (default null), the user agent must run the following steps:
  - 1. Let piTarget be the target given in the processing instruction token being processed.
  - 2. If the parser's parser sanitizer configuration is non-null and the parser's parser sanitizer configuration does not allow processing instruction target piTarget, then return.
  - 3. Let data be the data given in the processing instruction token being processed.
  - 4. Let (targetParent, referenceChild) be the adjusted insertion location given insertionLocation.
  - 5. Let pi be the result of creating a processing instruction node given targetParent's node document, piTarget, and data.
  - 6. Insert pi into targetParent before referenceChild.

##### [13.2.6.2 Parsing elements that contain only text](https://html.spec.whatwg.org/#parsing-elements-that-contain-only-text)

Status: reviewed (2026-11-02)
> `ParseGenericRawTextElement` and `ParseGenericRCDATATextElement`.
<!-- definitions: generic-rcdata-element-parsing-algorithm -->

- [x] [generic raw text element parsing algorithm](https://html.spec.whatwg.org/#generic-raw-text-element-parsing-algorithm) — The generic raw text element parsing algorithm and the generic RCDATA element parsing algorithm consist of the following steps. These algorithms are always invoked in response to a start tag token.
  - 1. Insert an HTML element for the token.
  - 2. If the algorithm that was invoked is the generic raw text element parsing algorithm, switch the tokenizer to the RAWTEXT state; otherwise the algorithm invoked was the generic RCDATA element parsing algorithm, switch the tokenizer to the RCDATA state.
  - 3. Set the original insertion mode to the current insertion mode.
  - 4. Then, switch the insertion mode to "text".

##### [13.2.6.3 Closing elements that have implied end tags](https://html.spec.whatwg.org/#closing-elements-that-have-implied-end-tags)

Status: reviewed (2026-11-02)
> `OpenElementStack::GenerateImpliedEndTags` and its thoroughly variant.

- [x] [generate implied end tags](https://html.spec.whatwg.org/#generate-implied-end-tags) — When the steps below require the UA to generate implied end tags, then, while the current node is a dd element, a dt element, an li element, an optgroup element, an option element, a p element, an rb element, an rp element, an rt element, or an rtc element, the UA must pop the current node off the stack of open elements.

- [x] [generate all implied end tags thoroughly](https://html.spec.whatwg.org/#generate-all-implied-end-tags-thoroughly) — When the steps below require the UA to generate all implied end tags thoroughly, then, while the current node is a caption element, a colgroup element, a dd element, a dt element, an li element, an optgroup element, an option element, a p element, an rb element, an rp element, an rt element, an rtc element, a tbody element, a td element, a tfoot element, a th element, a thead element, or a tr element, the UA must pop the current node off the stack of open elements.

##### [13.2.6.4 The rules for parsing tokens in HTML content](https://html.spec.whatwg.org/#parsing-main-inhtml)

Status: reviewed (2026-11-02)

###### [13.2.6.4.1 The "initial" insertion mode](https://html.spec.whatwg.org/#the-initial-insertion-mode)

Status: reviewed (2026-11-02)
<!-- definitions: parser-cannot-change-the-mode-flag -->

###### [13.2.6.4.2 The "before html" insertion mode](https://html.spec.whatwg.org/#the-before-html-insertion-mode)

Status: reviewed (2026-11-02)

###### [13.2.6.4.3 The "before head" insertion mode](https://html.spec.whatwg.org/#the-before-head-insertion-mode)

Status: reviewed (2026-11-02)

###### [13.2.6.4.4 The "in head" insertion mode](https://html.spec.whatwg.org/#parsing-main-inhead)

Status: reviewed (2026-11-02)
> A meta element doesn't change the encoding, as documents are UTF-8 only (see `DIFFERENCES.md`).

- [x] [If the active speculative HTML parser is null](https://html.spec.whatwg.org/#parsing-main-inhead) — If the active speculative HTML parser is null:
  - 1. If the element has a charset attribute, and getting an encoding from its value results in an encoding, and the confidence is currently tentative, then change the encoding to the resulting encoding.
  - 2. Otherwise, if the element has an http-equiv attribute whose value is an ASCII case-insensitive match for "Content-Type", and the element has a content attribute, and applying the algorithm for extracting a character encoding from a meta element to that attribute's value returns an encoding, and the confidence is currently tentative, then change the encoding to the extracted encoding.

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inhead) — Run these steps:
  - 1. Create an element for the token in the HTML namespace, with the intended parent being the target parent of the appropriate place for inserting a node.
  - 2. If the scripting mode is not Fragment, then set the element's parser document to the Document and set its force async to false.
  - 3. If the parser's scripting mode is Inert, then set the script element's already started to true. (fragment case)
  - 4. If the parser was invoked via the document.write() or document.writeln() methods, then optionally set the script element's already started to true. (For example, the user agent might use this clause to prevent execution of cross-origin scripts inserted via document.write() under slow network conditions, or when the page has already taken a long time to load.)
  - 5. Insert an element at the adjusted insertion location with the newly created element.
  - 6. Push the newly created element onto the stack of open elements so that it is the new current node.
  - 7. Switch the tokenizer to the script data state.
  - 8. Set the original insertion mode to the current insertion mode.
  - 9. Switch the insertion mode to "text".

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inhead) — Run these steps:
  - 1. Let templateStartTag be the start tag.
  - 2. Insert a marker at the end of the list of active formatting elements.
  - 3. Set the frameset-ok flag to "not ok".
  - 4. Switch the insertion mode to "in template".
  - 5. Push "in template" onto the stack of template insertion modes so that it is the new current template insertion mode.
  - 6. Let declarativeShadowHostElement be the current node.
  - 7. Let hostIsRoot be true if declarativeShadowHostElement is the topmost element in the stack of open elements; otherwise false.
  - 8. Let template be the result of creating an element for the token given templateStartTag, the HTML namespace, and the target parent of the appropriate place for inserting a node.
  - 9. Let insertionLocation be the appropriate place for inserting a node.
  - 10. Push template onto the stack of open elements so that it is the new current node.
  - 11. Let sanitizerAction be "Keep".
  - 12. If the parser's parser sanitizer configuration is not null, then set sanitizerAction to the result of getting the sanitizer action for an element name given template's local name, template's namespace, and the parser's parser sanitizer configuration.
  - 13. Switch on sanitizerAction: "Remove""Replace with children" Return. "Keep"
    - 13a. If template's shadowrootmode attribute is not in the None state:
      - 13a.i. If any of the following are true: the parser's allow declarative shadow roots is false;; hostIsRoot is true; or; the parser's insertion target redirection map[declarativeShadowHostElement] exists, then insert an element at the adjusted insertion location with template, insertionLocation, and true, and return.
      - 13a.ii. Let mode be template's shadowrootmode attribute's value.
      - 13a.iii. Let delegatesFocus be true if template has a shadowrootdelegatesfocus attribute; otherwise false.
      - 13a.iv. Let serializable be true if template has a shadowrootserializable attribute; otherwise false.
      - 13a.v. Let slotAssignment be "named".
      - 13a.vi. If template's shadowrootslotassignment attribute is in the Manual state, then set slotAssignment to "manual".
      - 13a.vii. Let clonable be true if template has a shadowrootclonable attribute; otherwise false.
      - 13a.viii. If declarativeShadowHostElement is a shadow host, then insert an element at the adjusted insertion location with template, insertionLocation, and true.
      - 13a.ix. Otherwise:
        - 13a.ix.A. Let registry be null if template has a shadowrootcustomelementregistry attribute; otherwise declarativeShadowHostElement's node document's custom element registry.
        - 13a.ix.B. Attach a shadow root with declarativeShadowHostElement, mode, delegatesFocus, serializable, slotAssignment, clonable, and registry. If an exception is thrown, then catch it and:
          - 13a.ix.B.1. Insert an element at the adjusted insertion location with template, insertionLocation, and true.
          - 13a.ix.B.2. The user agent may report an error to the developer console.
          - 13a.ix.B.3. Return.
        - 13a.ix.C. Let shadow be declarativeShadowHostElement's shadow root.
        - 13a.ix.D. Set shadow's available to element internals to true.
        - 13a.ix.E. Set shadow's declarative to true.
        - 13a.ix.F. Set template's template contents to shadow.
        - 13a.ix.G. If template has a shadowrootcustomelementregistry attribute, then set shadow's keep custom element registry null to true.
    - 13b. Otherwise, if template has a for attribute:
      - 13b.i. Let scope be the target parent of the adjusted insertion location given insertionLocation.
      - 13b.ii. If scope is the body element of its node document, then set scope to its parent.
      - 13b.iii. Let success be the result of preparing content patching given scope and template.
      - 13b.iv. If success is false, then insert an element at the adjusted insertion location with template, insertionLocation, and true.
    - 13c. Otherwise, insert an element at the adjusted insertion location with template, insertionLocation, and true.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inhead) — Otherwise, run these steps:
  - 1. Generate all implied end tags thoroughly.
  - 2. If the current node is not a template element, then this is a parse error.
  - 3. Let template be the last template element in the stack of open elements.
  - 4. If template's insertion target is not null:
    - 4a. Let start be template's insertion start marker.
    - 4b. Let end be template's insertion end marker.
    - 4c. Assert: start is not null.
    - 4d. If start's parent is not null, then remove start.
    - 4e. If end is not null and end's parent is not null, then remove end.
    - 4f. Set template's insertion target to null.
    - 4g. Set template's insertion start marker to null.
    - 4h. Set template's insertion end marker to null.
  - 5. Pop elements from the stack of open elements until a template element has been popped from the stack.
  - 6. Clear the list of active formatting elements up to the last marker.
  - 7. Pop the current template insertion mode off the stack of template insertion modes.
  - 8. Reset the insertion mode appropriately.

###### [13.2.6.4.5 The "in head noscript" insertion mode](https://html.spec.whatwg.org/#parsing-main-inheadnoscript)

Status: reviewed (2026-11-02)

###### [13.2.6.4.6 The "after head" insertion mode](https://html.spec.whatwg.org/#the-after-head-insertion-mode)

Status: reviewed (2026-11-02)
> Fixed: anything else didn't set the frameset-ok flag back to "ok".

###### [13.2.6.4.7 The "in body" insertion mode](https://html.spec.whatwg.org/#parsing-main-inbody)

Status: reviewed (2026-11-02)
> Fixed: an `input` or `select` start tag looked for a select fragment context by the parser's scripting mode rather than the context element.

- [x] [Otherwise](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise:
  - 1. Let htmlElement be the top element of the stack of open elements.
  - 2. For each attribute on the token:
    - 2a. If the parser's parser sanitizer configuration is not null and the result of checking if a sanitizer config allows an attribute given attribute's name, null, attribute's value, htmlElement's local name, htmlElement's namespace, and the parser's parser sanitizer configuration is false, then continue.
    - 2b. If htmlElement does not have an attribute whose name is attribute's name, then add attribute's name and its corresponding value to htmlElement.

- [x] [Otherwise, run the following steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run the following steps:
  - 1. Remove the second element on the stack of open elements from its parent node, if it has one.
  - 2. Pop all the nodes from the bottom of the stack of open elements, from the current node up to, but not including, the root html element.
  - 3. Insert an HTML element for the token.
  - 4. Switch the insertion mode to "in frameset".

- [x] [Otherwise, follow these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, follow these steps:
  - 1. If there is a node in the stack of open elements that is not either a dd element, a dt element, an li element, an optgroup element, an option element, a p element, an rb element, an rp element, an rt element, an rtc element, a tbody element, a td element, a tfoot element, a th element, a thead element, a tr element, the body element, or the html element, then this is a parse error.
  - 2. Stop parsing.

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Run these steps:
  - 1. Set the frameset-ok flag to "not ok".
  - 2. Initialize node to be the current node (the bottommost node of the stack).
  - 3. Loop: If node is an li element, then run these substeps:
    - 3a. Generate implied end tags, except for li elements.
    - 3b. If the current node is not an li element, then this is a parse error.
    - 3c. Pop elements from the stack of open elements until an li element has been popped from the stack.
    - 3d. Jump to the step labeled done below.
  - 4. If node is in the special category, but is not an address, div, or p element, then jump to the step labeled done below.
  - 5. Otherwise, set node to the previous entry in the stack of open elements and return to the step labeled loop.
  - 6. Done: If the stack of open elements has a p element in button scope, then close a p element.
  - 7. Finally, insert an HTML element for the token.

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Run these steps:
  - 1. Set the frameset-ok flag to "not ok".
  - 2. Initialize node to be the current node (the bottommost node of the stack).
  - 3. Loop: If node is a dd element, then run these substeps:
    - 3a. Generate implied end tags, except for dd elements.
    - 3b. If the current node is not a dd element, then this is a parse error.
    - 3c. Pop elements from the stack of open elements until a dd element has been popped from the stack.
    - 3d. Jump to the step labeled done below.
  - 4. If node is a dt element, then run these substeps:
    - 4a. Generate implied end tags, except for dt elements.
    - 4b. If the current node is not a dt element, then this is a parse error.
    - 4c. Pop elements from the stack of open elements until a dt element has been popped from the stack.
    - 4d. Jump to the step labeled done below.
  - 5. If node is in the special category, but is not an address, div, or p element, then jump to the step labeled done below.
  - 6. Otherwise, set node to the previous entry in the stack of open elements and return to the step labeled loop.
  - 7. Done: If the stack of open elements has a p element in button scope, then close a p element.
  - 8. Finally, insert an HTML element for the token.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run these steps:
  - 1. Generate implied end tags.
  - 2. If the current node is not an HTML element with the same tag name as that of the token, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an HTML element with the same tag name as the token has been popped from the stack.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run these steps:
  - 1. Generate implied end tags, except for li elements.
  - 2. If the current node is not an li element, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an li element has been popped from the stack.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run these steps:
  - 1. Generate implied end tags, except for HTML elements with the same tag name as the token.
  - 2. If the current node is not an HTML element with the same tag name as that of the token, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an HTML element with the same tag name as the token has been popped from the stack.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run these steps:
  - 1. Generate implied end tags.
  - 2. If the current node is not an HTML element with the same tag name as that of the token, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an HTML element whose tag name is one of "h1", "h2", "h3", "h4", "h5", or "h6" has been popped from the stack.

- [x] [Otherwise, run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Otherwise, run these steps:
  - 1. Generate implied end tags.
  - 2. If the current node is not an HTML element with the same tag name as that of the token, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an HTML element with the same tag name as the token has been popped from the stack.
  - 4. Clear the list of active formatting elements up to the last marker.

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Run these steps:
  - 1. Insert an HTML element for the token.
  - 2. If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to the next one. (Newlines at the start of textarea elements are ignored as an authoring convenience.)
  - 3. Switch the tokenizer to the RCDATA state.
  - 4. Set the original insertion mode to the current insertion mode.
  - 5. Set the frameset-ok flag to "not ok".
  - 6. Switch the insertion mode to "text".

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inbody) — Run these steps:
  - 1. Initialize node to be the current node (the bottommost node of the stack).
  - 2. Loop: If node is an HTML element with the same tag name as the token:
    - 2a. Generate implied end tags, except for HTML elements with the same tag name as the token.
    - 2b. If node is not the current node, then this is a parse error.
    - 2c. Pop all the nodes from the current node up to node, including node, then stop these steps.
  - 3. Otherwise, if node is in the special category, then this is a parse error; ignore the token, and return.
  - 4. Set node to the previous entry in the stack of open elements.
  - 5. Return to the step labeled loop.

- [x] [close a p element](https://html.spec.whatwg.org/#close-a-p-element) — When the steps above say the user agent is to close a p element, it means that the user agent must run the following steps:
  - 1. Generate implied end tags, except for p elements.
  - 2. If the current node is not a p element, then this is a parse error.
  - 3. Pop elements from the stack of open elements until a p element has been popped from the stack.

- [x] [adoption agency algorithm](https://html.spec.whatwg.org/#adoption-agency-algorithm) — The adoption agency algorithm, which takes as its only argument a token token for which the algorithm is being run, consists of the following steps:
  - 1. Let subject be token's tag name.
  - 2. If the current node is an HTML element whose tag name is subject, and the current node is not in the list of active formatting elements, then pop the current node off the stack of open elements and return.
  - 3. Let outerLoopCounter be 0.
  - 4. While true:
    - 4a. If outerLoopCounter is greater than or equal to 8, then return.
    - 4b. Increment outerLoopCounter by 1.
    - 4c. Let formattingElement be the last element in the list of active formatting elements that: is between the end of the list and the last marker in the list, if any, or the start of the list otherwise, and; has the tag name subject. If there is no such element, then act as described in the "any other end tag" entry above and return.
    - 4d. If formattingElement is not in the stack of open elements, then this is a parse error; remove the element from the list, and return.
    - 4e. If formattingElement is in the stack of open elements, but the element is not in scope, then this is a parse error; return.
    - 4f. If formattingElement is not the current node, this is a parse error. (But do not return.)
    - 4g. Let furthestBlock be the topmost node in the stack of open elements that is lower in the stack than formattingElement, and is an element in the special category. There might not be one.
    - 4h. If there is no furthestBlock, then the UA must first pop all the nodes from the bottom of the stack of open elements, from the current node up to and including formattingElement, then remove formattingElement from the list of active formatting elements, and finally return.
    - 4i. Let commonAncestor be the element immediately above formattingElement in the stack of open elements.
    - 4j. Let a bookmark note the position of formattingElement in the list of active formatting elements relative to the elements on either side of it in the list.
    - 4k. Let node and lastNode be furthestBlock.
    - 4l. Let pendingRedirects be an empty list.
    - 4m. If the parser's parser sanitizer configuration is not null:
      - 4m.i. Let action be the result of getting the sanitizer action for an element name given lastNode's local name, lastNode's namespace, and the parser's parser sanitizer configuration.
      - 4m.ii. If action is not "Keep", then set lastNode to null.
      - 4m.iii. If action is "Replace with children", then append furthestBlock to pendingRedirects.
    - 4n. Let innerLoopCounter be 0.
    - 4o. Let bookmarkMoved be false.
    - 4p. While true:
      - 4p.i. Increment innerLoopCounter by 1.
      - 4p.ii. Let node be the element immediately above node in the stack of open elements, or if node is no longer in the stack of open elements (e.g. because it got removed by this algorithm), the element that was immediately above node in the stack of open elements before node was removed.
      - 4p.iii. If node is formattingElement, then break.
      - 4p.iv. If innerLoopCounter is greater than 3 and node is in the list of active formatting elements, then remove node from the list of active formatting elements.
      - 4p.v. If node is not in the list of active formatting elements, then remove node from the stack of open elements and continue.
      - 4p.vi. Create an element for the token for which the element node was created, in the HTML namespace, with commonAncestor as the intended parent; replace the entry for node in the list of active formatting elements with an entry for the new element, replace the entry for node in the stack of open elements with an entry for the new element, and set node to the new element.
      - 4p.vii. If bookmarkMoved is false, then move the aforementioned bookmark to be immediately after the new node in the list of active formatting elements, and set bookmarkMoved to true.
      - 4p.viii. Let nodeAction be "Keep".
      - 4p.ix. If the parser's parser sanitizer configuration is not null, then set nodeAction to the result of getting the sanitizer action for an element name given node's local name, node's namespace, and the parser's parser sanitizer configuration.
      - 4p.x. If nodeAction is not "Replace with children":
        - 4p.x.A. For each replacedElement of pendingRedirects: set the parser's insertion target redirection map[replacedElement] to (node, null).
        - 4p.x.B. Empty pendingRedirects.
      - 4p.xi. Switch on nodeAction: "Keep"If lastNode is non-null, then append lastNode to node. Set lastNode to node."Remove"If lastNode is non-null and lastNode's parent is non-null, then remove lastNode. Set lastNode to null."Replace with children"Append node to pendingRedirects.
    - 4q. Let insertionLocation be the appropriate place for inserting a node given commonAncestor.
    - 4r. Let (target, refNode) be the adjusted insertion location given insertionLocation.
    - 4s. For each replacedElement of pendingRedirects: set the parser's insertion target redirection map[replacedElement] to (target, refNode).
    - 4t. If lastNode is non-null and lastNode's parent is non-null, then remove lastNode.
    - 4u. If all of the following are true: lastNode is non-null;; lastNode's parent is null;; lastNode is not a host-including inclusive ancestor of target;; target is not a Document node, or it does not have an element child; and; refNode is null or its parent is target, then:
      - 4u.i. Assert: ensure pre-insert validity given lastNode, target, refNode, and « » does not throw.
      - 4u.ii. Insert lastNode into target before refNode.
    - 4v. Let newElement be the result of creating an element for the token for which formattingElement was created, in the HTML namespace, with furthestBlock as the intended parent.
    - 4w. Remove formattingElement from the list of active formatting elements, and insert newElement into the list of active formatting elements at the position of the aforementioned bookmark.
    - 4x. Remove formattingElement from the stack of open elements, and insert newElement into the stack of open elements immediately below the position of furthestBlock in that stack.
    - 4y. Let sanitizerAction be "Keep".
    - 4z. If the parser's parser sanitizer configuration is not null, then set sanitizerAction to the result of getting the sanitizer action for an element name given newElement's local name, newElement's namespace, and the parser's parser sanitizer configuration.
    - 4aa. Switch on sanitizerAction: "Replace with children"Set the parser's insertion target redirection map[newElement] to the adjusted insertion location given (furthestBlock, null)."Remove"Take all of the child nodes of furthestBlock and append them to newElement."Keep"
      - 4aa.i. Take all of the child nodes of furthestBlock and append them to newElement.
      - 4aa.ii. Let (newElementParent, newElementRefNode) be the adjusted insertion location given (furthestBlock, null).
      - 4aa.iii. Insert newElement into newElementParent before newElementRefNode.
    - 4ab. Let belowNode be the element immediately below newElement in the stack of open elements, if any; otherwise null.
    - 4ac. While belowNode is non-null and the parser's insertion target redirection map[belowNode] exists:
      - 4ac.i. Set the parser's insertion target redirection map[belowNode] to the adjusted insertion location given (newElement, null).
      - 4ac.ii. Set belowNode to the element immediately below belowNode in the stack of open elements, if any; otherwise null.

###### [13.2.6.4.8 The "text" insertion mode](https://html.spec.whatwg.org/#parsing-main-incdata)

Status: reviewed (2026-11-02)
> A script end tag only pops the script, as there's no scripting (see `DIFFERENCES.md`).

- [x] [While the pending parsing-blocking script is not null](https://html.spec.whatwg.org/#parsing-main-incdata) — While the pending parsing-blocking script is not null:
  - 1. Let the script be the pending parsing-blocking script.
  - 2. Set the pending parsing-blocking script to null.
  - 3. Start the speculative HTML parser for this instance of the HTML parser.
  - 4. Block the tokenizer for this instance of the HTML parser, such that the event loop will not run tasks that invoke the tokenizer.
  - 5. If the parser's Document has a style sheet that is blocking scripts or the script's ready to be parser-executed is false: spin the event loop until the parser's Document has no style sheet that is blocking scripts and the script's ready to be parser-executed becomes true.
  - 6. If this parser has been aborted in the meantime, return.
  - 7. Stop the speculative HTML parser for this instance of the HTML parser.
  - 8. Unblock the tokenizer for this instance of the HTML parser, such that tasks that invoke the tokenizer can again be run.
  - 9. Let the insertion point be just before the next input character.
  - 10. Increment the parser's script nesting level by one (it should be zero before this step, so this sets it to one).
  - 11. Execute the script element the script.
  - 12. Decrement the parser's script nesting level by one. If the parser's script nesting level is zero (which it always should be at this point), then set the parser pause flag to false.
  - 13. Let the insertion point be undefined again.

###### [13.2.6.4.9 The "in table" insertion mode](https://html.spec.whatwg.org/#parsing-main-intable)

Status: reviewed (2026-11-02)
> Fixed: a form start tag inside a template was ignored rather than inserted.

- [x] [pending table character tokens](https://html.spec.whatwg.org/#concept-pending-table-char-tokens) — Let the pending table character tokens be an empty list of tokens. Set the original insertion mode to the current insertion mode. Switch the insertion mode to "in table text" and reprocess the token.

- [x] [clear the stack back to a table context](https://html.spec.whatwg.org/#clear-the-stack-back-to-a-table-context) — When the steps above require the UA to clear the stack back to a table context, it means that the UA must, while the current node is not a table, template, or html element, pop elements from the stack of open elements.

###### [13.2.6.4.10 The "in table text" insertion mode](https://html.spec.whatwg.org/#parsing-main-intabletext)

Status: reviewed (2026-11-02)

###### [13.2.6.4.11 The "in caption" insertion mode](https://html.spec.whatwg.org/#parsing-main-incaption)

Status: reviewed (2026-11-02)

- [x] [Otherwise](https://html.spec.whatwg.org/#parsing-main-incaption) — Otherwise:
  - 1. Generate implied end tags.
  - 2. Now, if the current node is not a caption element, then this is a parse error.
  - 3. Pop elements from this stack until a caption element has been popped from the stack.
  - 4. Clear the list of active formatting elements up to the last marker.
  - 5. Switch the insertion mode to "in table".

###### [13.2.6.4.12 The "in column group" insertion mode](https://html.spec.whatwg.org/#parsing-main-incolgroup)

Status: reviewed (2026-11-02)

###### [13.2.6.4.13 The "in table body" insertion mode](https://html.spec.whatwg.org/#parsing-main-intbody)

Status: reviewed (2026-11-02)

- [x] [Otherwise](https://html.spec.whatwg.org/#parsing-main-intbody) — Otherwise:
  - 1. Clear the stack back to a table body context. (See below.)
  - 2. Pop the current node from the stack of open elements. Switch the insertion mode to "in table".

- [x] [clear the stack back to a table body context](https://html.spec.whatwg.org/#clear-the-stack-back-to-a-table-body-context) — When the steps above require the UA to clear the stack back to a table body context, it means that the UA must, while the current node is not a tbody, tfoot, thead, template, or html element, pop elements from the stack of open elements.

###### [13.2.6.4.14 The "in row" insertion mode](https://html.spec.whatwg.org/#parsing-main-intr)

Status: reviewed (2026-11-02)

- [x] [Otherwise](https://html.spec.whatwg.org/#parsing-main-intr) — Otherwise:
  - 1. Clear the stack back to a table row context. (See below.)
  - 2. Pop the current node (which will be a tr element) from the stack of open elements. Switch the insertion mode to "in table body".

- [x] [clear the stack back to a table row context](https://html.spec.whatwg.org/#clear-the-stack-back-to-a-table-row-context) — When the steps above require the UA to clear the stack back to a table row context, it means that the UA must, while the current node is not a tr, template, or html element, pop elements from the stack of open elements.

###### [13.2.6.4.15 The "in cell" insertion mode](https://html.spec.whatwg.org/#parsing-main-intd)

Status: reviewed (2026-11-02)

- [x] [Otherwise](https://html.spec.whatwg.org/#parsing-main-intd) — Otherwise:
  - 1. Generate implied end tags.
  - 2. Now, if the current node is not an HTML element with the same tag name as the token, then this is a parse error.
  - 3. Pop elements from the stack of open elements until an HTML element with the same tag name as the token has been popped from the stack.
  - 4. Clear the list of active formatting elements up to the last marker.
  - 5. Switch the insertion mode to "in row".

- [x] [close the cell](https://html.spec.whatwg.org/#close-the-cell) — Where the steps above say to close the cell, they mean to run the following algorithm:
  - 1. Generate implied end tags.
  - 2. If the current node is not now a td element or a th element, then this is a parse error.
  - 3. Pop elements from the stack of open elements until a td element or a th element has been popped from the stack.
  - 4. Clear the list of active formatting elements up to the last marker.
  - 5. Switch the insertion mode to "in row".

###### [13.2.6.4.16 The "in template" insertion mode](https://html.spec.whatwg.org/#parsing-main-intemplate)

Status: reviewed (2026-11-02)

###### [13.2.6.4.17 The "after body" insertion mode](https://html.spec.whatwg.org/#parsing-main-afterbody)

Status: reviewed (2026-11-02)

###### [13.2.6.4.18 The "in frameset" insertion mode](https://html.spec.whatwg.org/#parsing-main-inframeset)

Status: reviewed (2026-11-02)

###### [13.2.6.4.19 The "after frameset" insertion mode](https://html.spec.whatwg.org/#parsing-main-afterframeset)

Status: reviewed (2026-11-02)

###### [13.2.6.4.20 The "after after body" insertion mode](https://html.spec.whatwg.org/#the-after-after-body-insertion-mode)

Status: reviewed (2026-11-02)

###### [13.2.6.4.21 The "after after frameset" insertion mode](https://html.spec.whatwg.org/#the-after-after-frameset-insertion-mode)

Status: reviewed (2026-11-02)

##### [13.2.6.5 The rules for parsing tokens in foreign content](https://html.spec.whatwg.org/#parsing-main-inforeign)

Status: reviewed (2026-11-02)
> An SVG script end tag only pops the script (see `DIFFERENCES.md`).
> Fixed: any other end tag checked the stack entry's tag name before stepping to the previous entry, so it compared the wrong element, and reported its parse error the wrong way round.

- [x] [Run these steps](https://html.spec.whatwg.org/#parsing-main-inforeign) — Run these steps:
  - 1. Initialize node to be the current node (the bottommost node of the stack).
  - 2. If node's tag name, converted to ASCII lowercase, is not the same as the tag name of the token, then this is a parse error.
  - 3. Loop: If node is the topmost element in the stack of open elements, then return. (fragment case)
  - 4. If node's tag name, converted to ASCII lowercase, is the same as the tag name of the token, pop elements from the stack of open elements until node has been popped from the stack, and then return.
  - 5. Set node to the previous entry in the stack of open elements.
  - 6. If node is not an element in the HTML namespace, return to the step labeled loop.
  - 7. Otherwise, process the token according to the rules given in the section corresponding to the current insertion mode in HTML content.

#### [13.2.7 The end](https://html.spec.whatwg.org/#the-end)

Status: reviewed (2026-11-02)
> `TreeBuilder::StopParsing` pops the stack and `Document::FinishedParsing` runs the rest; `DOMContentLoaded` fires synchronously (see `DIFFERENCES.md`).
<!-- definitions: delay-the-load-event ready-for-post-load-tasks -->

- [x] [stops parsing](https://html.spec.whatwg.org/#stop-parsing) — Once the user agent stops parsing the document, the user agent must run the following steps:
  - 1. If the active speculative HTML parser is not null, then stop the speculative HTML parser and return.
  - 2. Set the insertion point to undefined.
  - 3. Update the current document readiness to "interactive".
  - 4. Pop all the nodes off the stack of open elements.
  - 5. While the list of scripts that will execute when the document has finished parsing is not empty:
    - 5a. Spin the event loop until the first script in the list of scripts that will execute when the document has finished parsing has its ready to be parser-executed set to true and the parser's Document has no style sheet that is blocking scripts.
    - 5b. Execute the script element given by the first script in the list of scripts that will execute when the document has finished parsing.
    - 5c. Remove the first script element from the list of scripts that will execute when the document has finished parsing (i.e. shift out the first entry in the list).
  - 6. Queue a global task on the DOM manipulation task source given the Document's relevant global object to run the following substeps:
    - 6a. Set the Document's load timing info's DOM content loaded event start time to the current high resolution time given the Document's relevant global object.
    - 6b. Fire an event named DOMContentLoaded at the Document object, with its bubbles attribute initialized to true.
    - 6c. Set the Document's load timing info's DOM content loaded event end time to the current high resolution time given the Document's relevant global object.
    - 6d. Enable the client message queue of the ServiceWorkerContainer object whose associated service worker client is the Document object's relevant settings object.
    - 6e. Invoke WebDriver BiDi DOM content loaded with the Document's browsing context, and a new WebDriver BiDi navigation status whose id is the Document object's during-loading navigation ID for WebDriver BiDi, status is "pending", and url is the Document object's URL.
  - 7. Spin the event loop until the set of scripts that will execute as soon as possible and the list of scripts that will execute in order as soon as possible are empty.
  - 8. Spin the event loop until there is nothing that delays the load event in the Document.
  - 9. Queue a global task on the DOM manipulation task source given the Document's relevant global object to run the following steps:
    - 9a. Update the current document readiness to "complete".
    - 9b. If the Document object's browsing context is null, then abort these steps.
    - 9c. Let window be the Document's relevant global object.
    - 9d. Set the Document's load timing info's load event start time to the current high resolution time given window.
    - 9e. Fire an event named load at window, with legacy target override flag set.
    - 9f. Invoke WebDriver BiDi load complete with the Document's browsing context, and a new WebDriver BiDi navigation status whose id is the Document object's during-loading navigation ID for WebDriver BiDi, status is "complete", and url is the Document object's URL.
    - 9g. Set the Document object's during-loading navigation ID for WebDriver BiDi to null.
    - 9h. Set the Document's load timing info's load event end time to the current high resolution time given window.
    - 9i. Assert: Document's page showing is false.
    - 9j. Set the Document's page showing to true.
    - 9k. Fire a page transition event named pageshow at window with false.
    - 9l. Completely finish loading the Document.
    - 9m. Queue the navigation timing entry for the Document.
  - 10. If the Document's print when loaded flag is set, then run the printing steps.
  - 11. The Document is now ready for post-load tasks.

- [x] [abort a parser](https://html.spec.whatwg.org/#abort-a-parser) — When the user agent is to abort a parser, it must run the following steps:
  - 1. Throw away any pending content in the input stream, and discard any future content that would have been added to it.
  - 2. Stop the speculative HTML parser for this HTML parser.
  - 3. Update the current document readiness to "interactive".
  - 4. Pop all the nodes off the stack of open elements.
  - 5. Update the current document readiness to "complete".

#### [13.2.8 Speculative HTML parsing](https://html.spec.whatwg.org/#speculative-html-parsing)

Status: n/a (optional; no speculative parser)
<!-- definitions: active-speculative-html-parser speculative-html-parser list-of-speculative-fetch-urls speculative-mock-element concept-mock-namespace concept-mock-local-name concept-mock-attribute-list concept-mock-children -->

- [x] [speculative fetch](https://html.spec.whatwg.org/#speculative-fetch) — A speculative fetch for a speculative mock element element must follow these rules:

- [x] [start the speculative HTML parser](https://html.spec.whatwg.org/#start-the-speculative-html-parser) — To start the speculative HTML parser for an instance of an HTML parser parser:
  - 1. Optionally, return.
  - 2. If parser's active speculative HTML parser is not null, then stop the speculative HTML parser for parser.
  - 3. Let speculativeParser be a new speculative HTML parser, with the same state as parser.
  - 4. Let speculativeDoc be a new isomorphic representation of parser's Document, where all elements are instead speculative mock elements. Let speculativeParser parse into speculativeDoc.
  - 5. Set parser's active speculative HTML parser to speculativeParser.
  - 6. In parallel, run speculativeParser until it is stopped or until it reaches the end of its input stream.

- [x] [stop the speculative HTML parser](https://html.spec.whatwg.org/#stop-the-speculative-html-parser) — To stop the speculative HTML parser for an instance of an HTML parser parser:
  - 1. Let speculativeParser be parser's active speculative HTML parser.
  - 2. If speculativeParser is null, then return.
  - 3. Throw away any pending content in speculativeParser's input stream, and discard any future content that would have been added to it.
  - 4. Set parser's active speculative HTML parser to null.

- [x] [create a speculative mock element](https://html.spec.whatwg.org/#create-a-speculative-mock-element) — To create a speculative mock element given a namespace, tagName, and attributes:
  - 1. Let element be a new speculative mock element.
  - 2. Set element's namespace to namespace.
  - 3. Set element's local name to tagName.
  - 4. Set element's attribute list to attributes.
  - 5. Set element's children to a new empty list.
  - 6. Optionally, perform a speculative fetch for element.
  - 7. Return element.

#### [13.2.9 Coercing an HTML DOM into an infoset](https://html.spec.whatwg.org/#coercing-an-html-dom-into-an-infoset)

Status: n/a (no XML infoset output)

#### [13.2.10 An introduction to error handling and strange cases in the parser](https://html.spec.whatwg.org/#an-introduction-to-error-handling-and-strange-cases-in-the-parser)

Status: n/a (non-normative)

##### [13.2.10.1 Misnested tags: <b><i></b></i>](https://html.spec.whatwg.org/#misnested-tags:-b-i-/b-/i)

Status: n/a (non-normative)

##### [13.2.10.2 Misnested tags: <b><p></b></p>](https://html.spec.whatwg.org/#misnested-tags:-b-p-/b-/p)

Status: n/a (non-normative)

##### [13.2.10.3 Unexpected markup in tables](https://html.spec.whatwg.org/#unexpected-markup-in-tables)

Status: n/a (non-normative)

##### [13.2.10.4 Scripts that modify the page as it is being parsed](https://html.spec.whatwg.org/#scripts-that-modify-the-page-as-it-is-being-parsed)

Status: n/a (non-normative)

##### [13.2.10.5 The execution of scripts that are moving across multiple documents](https://html.spec.whatwg.org/#the-execution-of-scripts-that-are-moving-across-multiple-documents)

Status: n/a (non-normative)

##### [13.2.10.6 Unclosed formatting elements](https://html.spec.whatwg.org/#unclosed-formatting-elements)

Status: n/a (non-normative)

### [13.3 Serializing HTML fragments](https://html.spec.whatwg.org/#serialising-html-fragments)

Status: reviewed (2026-11-02)
> `MarkupAccumulator` (through `SerializeFragment` in `Markup.hpp`); the shadow root's custom element registry attribute is a FIXME.
> Fixed: HTML, SVG and MathML elements were written with their qualified names, the `is` value and manual slot assignment weren't written, and processing instructions ended with `?>` (see `DIFFERENCES.md`).
<!-- definitions: attribute's-serialised-name -->

- [x] [serializes as void](https://html.spec.whatwg.org/#serializes-as-void) — For the purposes of the following algorithm, an element serializes as void if its element type is one of the void elements, or is basefont, bgsound, frame, keygen, or param.

- [x] [HTML fragment serialization algorithm](https://html.spec.whatwg.org/#html-fragment-serialisation-algorithm) — The following steps form the HTML fragment serialization algorithm. The algorithm takes as input a DOM Element, Document, or DocumentFragment referred to as the node, a boolean serializableShadowRoots, and a sequence<ShadowRoot> shadowRoots, and returns a string.
  - 1. If the node serializes as void, then return the empty string.
  - 2. Let s be a string, and initialize it to the empty string.
  - 3. If the node is a template element, then let the node instead be the template element's template contents (a DocumentFragment node).
  - 4. If current node is a shadow host:
    - 4a. Let shadow be current node's shadow root.
    - 4b. If one of the following is true: serializableShadowRoots is true and shadow's serializable is true; or; shadowRoots contains shadow, then:
      - 4b.i. Append "<template shadowrootmode="".
      - 4b.ii. If shadow's mode is "open", then append "open". Otherwise, append "closed".
      - 4b.iii. Append U+0022 (").
      - 4b.iv. If shadow's delegates focus is set, then append " shadowrootdelegatesfocus=""".
      - 4b.v. If shadow's serializable is set, then append " shadowrootserializable=""".
      - 4b.vi. If shadow's slot assignment is "manual", then append " shadowrootslotassignment="manual"".
      - 4b.vii. If shadow's clonable is set, then append " shadowrootclonable=""".
      - 4b.viii. Let shouldAppendRegistryAttribute be the result of running these steps:
        - 4b.viii.A. Let documentRegistry be shadow's node document's custom element registry.
        - 4b.viii.B. Let shadowRegistry be shadow's custom element registry.
        - 4b.viii.C. If documentRegistry is null and shadowRegistry is null, then return false.
        - 4b.viii.D. If documentRegistry is a global custom element registry and shadowRegistry is a global custom element registry, then return false.
        - 4b.viii.E. Return true.
      - 4b.ix. If shouldAppendRegistryAttribute is true, then append " shadowrootcustomelementregistry=""".
      - 4b.x. Append U+003E (>).
      - 4b.xi. Append the value of running the HTML fragment serialization algorithm with shadow, serializableShadowRoots, and shadowRoots (thus recursing into this algorithm for that element).
      - 4b.xii. Append "</template>".
  - 5. For each child node of the node, in tree order, run the following steps:
    - 5a. Let current node be the child node being processed.
    - 5b. Append the appropriate string from the following list to s: If current node is an Element If current node is an element in the HTML namespace, the MathML namespace, or the SVG namespace, then let tagname be current node's local name. Otherwise, let tagname be current node's qualified name. Append U+003C (<), followed by tagname. If current node's is value is not null, and the element does not hav…
  - 6. Return s.

- [x] [Escaping a string](https://html.spec.whatwg.org/#escapingString) — Escaping a string (for the purposes of the algorithm above) consists of running the following steps:
  - 1. Replace any occurrence of "&" character by "&amp;".
  - 2. Replace any occurrences of the U+00A0 NO-BREAK SPACE character by "&nbsp;".
  - 3. Replace any occurrences of the "<" character by "&lt;".
  - 4. Replace any occurrences of the ">" character by "&gt;".
  - 5. If the algorithm was invoked in the attribute mode, then replace any occurrences of the """ character by "&quot;".

### [13.4 Parsing HTML fragments](https://html.spec.whatwg.org/#parsing-html-fragments)

Status: reviewed (2026-11-02)
> `DocumentParser::ParseFragment`.
<!-- definitions: concept-frag-parse-context -->

- [x] [HTML fragment parsing algorithm](https://html.spec.whatwg.org/#html-fragment-parsing-algorithm) — The HTML fragment parsing algorithm, given an Element or DocumentFragment target, a string input, an optional boolean allowDeclarativeShadowRoots (default false), an optional parser scripting mode scriptingMode (default Inert), and an optional SanitizerConfig or null configuration (default null), i…
  - 1. Assert: scriptingMode is either Inert or Fragment.
  - 2. Let context be target if target is an Element; otherwise target's host.
  - 3. Assert: context is non-null.
  - 4. Let document be a Document node whose type is "html".
  - 5. Let contextDocument be context's node document.
  - 6. If contextDocument is in quirks mode, then set document's mode to "quirks".
  - 7. Otherwise, if contextDocument is in limited-quirks mode, then set document's mode to "limited-quirks".
  - 8. If contextDocument's scripting is disabled, then set scriptingMode to Disabled.
  - 9. Create a new HTML parser whose parser sanitizer configuration is configuration, allow declarative shadow roots is allowDeclarativeShadowRoots, scripting mode is scriptingMode, and fragment context element is context, and associate it with document.
  - 10. Set the state of the HTML parser's tokenization stage as follows, switching on context: titletextareaSwitch the tokenizer to the RCDATA state.stylexmpiframenoembednoframesSwitch the tokenizer to the RAWTEXT state.scriptSwitch the tokenizer to the script data state.noscriptIf scripting mode is not Disabled, switch the tokenizer to the RAWTEXT state. Otherwise, leave the tokenizer in the data state…
  - 11. Let root be the result of creating an element given document, "html", the HTML namespace, null, null, false, and the result of looking up a custom element registry given target.
  - 12. Append root to document.
  - 13. Set up the HTML parser's stack of open elements so that it contains just the single element root.
  - 14. Let fragment be the result of creating a document fragment given document if configuration is non-null; otherwise target's node document.
  - 15. Set the parser's insertion target redirection map[root] to (fragment, null).
  - 16. If context is a template element, then push "in template" onto the stack of template insertion modes so that it is the new current template insertion mode.
  - 17. Create a start tag token whose name is the local name of context and whose attributes are the attributes of context. Let this start tag token be the start tag token of context; e.g. for the purposes of determining if it is an HTML integration point.
  - 18. Reset the parser's insertion mode appropriately.
  - 19. Set the HTML parser's form element pointer to the nearest node to context that is a form element (going straight up the ancestor chain, and including the element itself, if it is a form element), if any. (If there is no such form element, the form element pointer keeps its initial value, null.)
  - 20. Place the input into the input stream for the HTML parser just created. The encoding confidence is irrelevant.
  - 21. Start the HTML parser and let it run until it has consumed all the characters just inserted into the input stream.
  - 22. Adopt fragment into target's node document.
  - 23. Return fragment.
