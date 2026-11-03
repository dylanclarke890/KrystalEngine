# HTML review checklist: 5 Microdata

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 21 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html microdata` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [5 Microdata](https://html.spec.whatwg.org/#microdata)

Status: n/a (microdata isn't processed, as in browsers)

### [5.1 Introduction](https://html.spec.whatwg.org/#introduction-7)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.1.1 Overview](https://html.spec.whatwg.org/#overview)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.1.2 The basic syntax](https://html.spec.whatwg.org/#the-basic-syntax)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.1.3 Typed items](https://html.spec.whatwg.org/#typed-items)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.1.4 Global identifiers for items](https://html.spec.whatwg.org/#global-identifiers-for-items)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.1.5 Selecting names when defining vocabularies](https://html.spec.whatwg.org/#selecting-names-when-defining-vocabularies)

Status: n/a (microdata isn't processed, as in browsers)

### [5.2 Encoding microdata](https://html.spec.whatwg.org/#encoding-microdata)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.2.1 The microdata model](https://html.spec.whatwg.org/#the-microdata-model)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.2.2 Items](https://html.spec.whatwg.org/#items)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: attr-itemscope concept-item attr-itemtype relevant-types support-global-identifiers-for-items attr-itemid attr-itemref -->

- [x] [item types](https://html.spec.whatwg.org/#item-types) — The item types of an item are the tokens obtained by splitting the element's itemtype attribute's value on ASCII whitespace. If the itemtype attribute is missing or parsing it in this way finds no tokens, the item is said to have no item types.

- [x] [typed item](https://html.spec.whatwg.org/#typed-item) — An item is said to be a typed item when either it has an item type, or it is the value of a property of a typed item. The relevant types for a typed item is the item's item types, if it has any, or else is the relevant types of the item for which it is a property's value.

- [x] [global identifier](https://html.spec.whatwg.org/#global-identifier) — The global identifier of an item is the value of its element's itemid attribute, if it has one, parsed relative to the node document of the element on which the attribute is specified. If the itemid attribute is missing or if parsing it returns failure, it is said to have no global identifier.

#### [5.2.3 Names: the itemprop attribute](https://html.spec.whatwg.org/#names:-the-itemprop-attribute)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: defined-property-name -->

- [x] [property names](https://html.spec.whatwg.org/#property-names) — The property names of an element are the tokens that the element's itemprop attribute is found to contain when its value is split on ASCII whitespace, with the order preserved but with duplicates removed (leaving only the first occurrence of each name).

#### [5.2.4 Values](https://html.spec.whatwg.org/#values)

Status: n/a (microdata isn't processed, as in browsers)

- [x] [property value](https://html.spec.whatwg.org/#concept-property-value) — The property value of a name-value pair added by an element with an itemprop attribute is as given for the first matching case in the following list:
  - • If the element also has an itemscope attribute: The value is the item created by the element.
  - • If the element is a meta element: The value is the value of the element's content attribute, if any, or the empty string if there is no such attribute.
  - • If the element is an audio, embed, iframe, img, source, track, or video element: The value is the result of encoding-parsing-and-serializing a URL given the element's src attribute's value, relative to the element's node document, at the time the attribute is set, or the empty string if there is no such attribute or the result is failure.
  - • If the element is an a, area, or link element: The value is the result of encoding-parsing-and-serializing a URL given the element's href attribute's value, relative to the element's node document, at the time the attribute is set, or the empty string if there is no such attribute or the result is failure.
  - • If the element is an object element: The value is the result of encoding-parsing-and-serializing a URL given the element's data attribute's value, relative to the element's node document, at the time the attribute is set, or the empty string if there is no such attribute or the result is failure.
  - • If the element is a data element: The value is the value of the element's value attribute, if it has one, or the empty string otherwise.
  - • If the element is a meter element: The value is the value of the element's value attribute, if it has one, or the empty string otherwise.
  - • If the element is a time element: The value is the element's datetime value.
  - • Otherwise: The value is the element's descendant text content.

- [x] [URL property elements](https://html.spec.whatwg.org/#url-property-elements) — The URL property elements are the a, area, audio, embed, iframe, img, link, object, source, track, and video elements.

#### [5.2.5 Associating names with items](https://html.spec.whatwg.org/#associating-names-with-items)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: microdata-error -->

- [x] [the properties of an item](https://html.spec.whatwg.org/#the-properties-of-an-item) — To find the properties of an item defined by the element root, the user agent must run the following steps. These steps are also used to flag microdata errors.
  - 1. Let results, memory, and pending be empty lists of elements.
  - 2. Add the element root to memory.
  - 3. Add the child elements of root, if any, to pending.
  - 4. If root has an itemref attribute, split the value of that itemref attribute on ASCII whitespace. For each resulting token ID, if there is an element in the tree of root with the ID ID, then add the first such element to pending.
  - 5. While pending is not empty:
    - 5a. Remove an element from pending and let current be that element.
    - 5b. If current is already in memory, there is a microdata error; continue.
    - 5c. Add current to memory.
    - 5d. If current does not have an itemscope attribute, then add all the child elements of current to pending.
    - 5e. If current has an itemprop attribute specified and has one or more property names, then add current to results.
  - 6. Sort results in tree order.
  - 7. Return results.

- [x] [top-level microdata item](https://html.spec.whatwg.org/#top-level-microdata-items) — An item is a top-level microdata item if its element does not have an itemprop attribute.

#### [5.2.6 Microdata and other namespaces](https://html.spec.whatwg.org/#microdata-and-other-namespaces)

Status: n/a (microdata isn't processed, as in browsers)

### [5.3 Sample microdata vocabularies](https://html.spec.whatwg.org/#mdvocabs)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.3.1 vCard](https://html.spec.whatwg.org/#vcard)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: md-vcard md-vcard-kind md-vcard-fn md-vcard-n md-vcard-n-family-name md-vcard-n-given-name md-vcard-n-additional-name md-vcard-n-honorific-prefix md-vcard-n-honorific-suffix md-vcard-nickname md-vcard-photo md-vcard-bday md-vcard-anniversary md-vcard-sex md-vcard-gender-identity md-vcard-adr md-vcard-adr-type md-vcard-adr-post-office-box md-vcard-adr-extended-address md-vcard-adr-street-address md-vcard-adr-locality md-vcard-adr-region md-vcard-adr-postal-code md-vcard-adr-country-name md-vcard-tel md-vcard-tel-type md-vcard-tel-value md-vcard-email md-vcard-impp md-vcard-lang md-vcard-tz md-vcard-geo md-vcard-title md-vcard-role md-vcard-logo md-vcard-agent md-vcard-org md-vcard-org-organization-name md-vcard-org-organization-unit md-vcard-member md-vcard-related md-vcard-related-url md-vcard-related-rel md-vcard-categories md-vcard-note md-vcard-rev md-vcard-sound md-vcard-uid md-vcard-url kind-strings md-vcard-kind-individual md-vcard-kind-group md-vcard-kind-org md-vcard-kind-location address-type-strings md-vcard-type-adr-home md-vcard-type-adr-work telephone-type-strings md-vcard-type-tel-home md-vcard-type-tel-work md-vcard-type-tel-text md-vcard-type-tel-voice md-vcard-type-tel-fax md-vcard-type-tel-cell md-vcard-type-tel-video md-vcard-type-tel-pager md-vcard-type-tel-textphone relationship-strings md-vcard-rel-emergency md-vcard-rel-agent md-vcard-rel-contact md-vcard-rel-acquaintance md-vcard-rel-friend md-vcard-rel-met md-vcard-rel-co-worker md-vcard-rel-colleague md-vcard-rel-co-resident md-vcard-rel-neighbor md-vcard-rel-child md-vcard-rel-parent md-vcard-rel-sibling md-vcard-rel-spouse md-vcard-rel-kin md-vcard-rel-muse md-vcard-rel-crush md-vcard-rel-date md-vcard-rel-sweetheart md-vcard-rel-me -->

- [x] [The value must be text and must match the following syntax](https://html.spec.whatwg.org/#concept-property-value) — The value must be text and must match the following syntax:
  - 1. Either a U+002B PLUS SIGN character (+) or a U+002D HYPHEN-MINUS character (-).
  - 2. A valid non-negative integer that is exactly two digits long and that represents a number in the range 00..23.
  - 3. A U+003A COLON character (:).
  - 4. A valid non-negative integer that is exactly two digits long and that represents a number in the range 00..59.

- [x] [The value must be text and must match the following syntax](https://html.spec.whatwg.org/#concept-property-value) — The value must be text and must match the following syntax:
  - 1. Optionally, either a U+002B PLUS SIGN character (+) or a U+002D HYPHEN-MINUS character (-).
  - 2. One or more ASCII digits.
  - 3. Optionally*, a U+002E FULL STOP character (.) followed by one or more ASCII digits.
  - 4. A U+003B SEMICOLON character (;).
  - 5. Optionally, either a U+002B PLUS SIGN character (+) or a U+002D HYPHEN-MINUS character (-).
  - 6. One or more ASCII digits.
  - 7. Optionally*, a U+002E FULL STOP character (.) followed by one or more ASCII digits.

##### [5.3.1.1 Conversion to vCard](https://html.spec.whatwg.org/#conversion-to-vcard)

Status: n/a (microdata isn't processed, as in browsers)

- [x] [extract any vCard data represented by those nodes](https://html.spec.whatwg.org/#extracting-a-vcard) — Given a list of nodes nodes in a Document, a user agent must run the following algorithm to extract any vCard data represented by those nodes (only the first vCard is returned):
  - 1. If none of the nodes in nodes are items with the item type http://microformats.org/profile/hcard, then there is no vCard. Abort the algorithm, returning nothing.
  - 2. Let node be the first node in nodes that is an item with the item type http://microformats.org/profile/hcard.
  - 3. Let output be an empty string.
  - 4. Add a vCard line with the type "BEGIN" and the value "VCARD" to output.
  - 5. Add a vCard line with the type "PROFILE" and the value "VCARD" to output.
  - 6. Add a vCard line with the type "VERSION" and the value "4.0" to output.
  - 7. Add a vCard line with the type "SOURCE" and the result of escaping the vCard text string that is the document's URL as the value to output.
  - 8. If the title element is not null, add a vCard line with the type "NAME" and with the result of escaping the vCard text string obtained from the title element's descendant text content as the value to output.
  - 9. Let sex be the empty string.
  - 10. Let gender-identity be the empty string.
  - 11. For each element element that is a property of the item node: for each name name in element's property names, run the following substeps:
    - 11a. Let parameters be an empty set of name-value pairs.
    - 11b. Run the appropriate set of substeps from the following list. The steps will set a variable value, which is used in the next step. If the property's value is an item subitem and name is n If the property's value is an item subitem and name is adr If the property's value is an item subitem and name is org If the property's value is an item subitem with the item type http://microformats.org/profile/…
      - 11b.i. Let value be the empty string.
      - 11b.ii. Append to value the result of collecting the first vCard subproperty named family-name in subitem.
      - 11b.iii. Append a U+003B SEMICOLON character (;) to value.
      - 11b.iv. Append to value the result of collecting the first vCard subproperty named given-name in subitem.
      - 11b.v. Append a U+003B SEMICOLON character (;) to value.
      - 11b.vi. Append to value the result of collecting the first vCard subproperty named additional-name in subitem.
      - 11b.vii. Append a U+003B SEMICOLON character (;) to value.
      - 11b.viii. Append to value the result of collecting the first vCard subproperty named honorific-prefix in subitem.
      - 11b.ix. Append a U+003B SEMICOLON character (;) to value.
      - 11b.x. Append to value the result of collecting the first vCard subproperty named honorific-suffix in subitem.
      - 11b.i. Let value be the empty string.
      - 11b.ii. Append to value the result of collecting vCard subproperties named post-office-box in subitem.
      - 11b.iii. Append a U+003B SEMICOLON character (;) to value.
      - 11b.iv. Append to value the result of collecting vCard subproperties named extended-address in subitem.
      - 11b.v. Append a U+003B SEMICOLON character (;) to value.
      - 11b.vi. Append to value the result of collecting vCard subproperties named street-address in subitem.
      - 11b.vii. Append a U+003B SEMICOLON character (;) to value.
      - 11b.viii. Append to value the result of collecting the first vCard subproperty named locality in subitem.
      - 11b.ix. Append a U+003B SEMICOLON character (;) to value.
      - 11b.x. Append to value the result of collecting the first vCard subproperty named region in subitem.
      - 11b.xi. Append a U+003B SEMICOLON character (;) to value.
      - 11b.xii. Append to value the result of collecting the first vCard subproperty named postal-code in subitem.
      - 11b.xiii. Append a U+003B SEMICOLON character (;) to value.
      - 11b.xiv. Append to value the result of collecting the first vCard subproperty named country-name in subitem.
      - 11b.xv. If there is a property named type in subitem, and the first such property has a value that is not an item and whose value consists only of ASCII alphanumerics, then add a parameter named "TYPE" whose value is the value of that property to parameters.
      - 11b.i. Let value be the empty string.
      - 11b.ii. Append to value the result of collecting the first vCard subproperty named organization-name in subitem.
      - 11b.iii. For each property named organization-unit in subitem, run the following steps:
        - 11b.iii.A. If the value of the property is an item, then skip this property.
        - 11b.iii.B. Append a U+003B SEMICOLON character (;) to value.
        - 11b.iii.C. Append the result of escaping the vCard text string given by the value of the property to value.
      - 11b.i. Let value be the empty string.
      - 11b.ii. If there is a property named url in subitem, and its element is a URL property element, then append the result of escaping the vCard text string given by the value of the first such property to value, and add a parameter with the name "VALUE" and the value "URI" to parameters.
      - 11b.iii. If there is a property named rel in subitem, and the first such property has a value that is not an item and whose value consists only of ASCII alphanumerics, then add a parameter named "RELATION" whose value is the value of that property to parameters.
      - 11b.i. Let value be the result of collecting the first vCard subproperty named value in subitem.
      - 11b.ii. If there is a property named type in subitem, and the first such property has a value that is not an item and whose value consists only of ASCII alphanumerics, then add a parameter named "TYPE" whose value is the value of that property to parameters.
      - 11b.i. Let value be the property's value.
      - 11b.ii. If element is one of the URL property elements, add a parameter with the name "VALUE" and the value "URI" to parameters.
      - 11b.iii. Otherwise, if name is bday or anniversary and the value is a valid date string, add a parameter with the name "VALUE" and the value "DATE" to parameters.
      - 11b.iv. Otherwise, if name is rev and the value is a valid global date and time string, add a parameter with the name "VALUE" and the value "DATE-TIME" to parameters.
      - 11b.v. Prefix every U+005C REVERSE SOLIDUS character (\) in value with another U+005C REVERSE SOLIDUS character (\).
      - 11b.vi. Prefix every U+002C COMMA character (,) in value with a U+005C REVERSE SOLIDUS character (\).
      - 11b.vii. Unless name is geo, prefix every U+003B SEMICOLON character (;) in value with a U+005C REVERSE SOLIDUS character (\).
      - 11b.viii. Replace every U+000D CARRIAGE RETURN U+000A LINE FEED character pair (CRLF) in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
      - 11b.ix. Replace every remaining U+000D CARRIAGE RETURN (CR) or U+000A LINE FEED (LF) character in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
    - 11c. Add a vCard line with the type name, the parameters parameters, and the value value to output.
  - 12. If either sex or gender-identity has a value that is not the empty string, add a vCard line with the type "GENDER" and the value consisting of the concatenation of sex, a U+003B SEMICOLON character (;), and gender-identity to output.
  - 13. Add a vCard line with the type "END" and the value "VCARD" to output.

- [x] [add a vCard line](https://html.spec.whatwg.org/#add-a-vcard-line) — When the above algorithm says that the user agent is to add a vCard line consisting of a type type, optionally some parameters, and a value value to a string output, it must run the following steps:
  - 1. Let line be an empty string.
  - 2. Append type, converted to ASCII uppercase, to line.
  - 3. If there are any parameters, then for each parameter, in the order that they were added, run these substeps:
    - 3a. Append a U+003B SEMICOLON character (;) to line.
    - 3b. Append the parameter's name to line.
    - 3c. Append a U+003D EQUALS SIGN character (=) to line.
    - 3d. Append the parameter's value to line.
  - 4. Append a U+003A COLON character (:) to line.
  - 5. Append value to line.
  - 6. Let maximum length be 75.
  - 7. While line's code point length is greater than maximum length:
    - 7a. Append the first maximum length code points of line to output.
    - 7b. Remove the first maximum length code points from line.
    - 7c. Append a U+000D CARRIAGE RETURN character (CR) to output.
    - 7d. Append a U+000A LINE FEED character (LF) to output.
    - 7e. Append a U+0020 SPACE character to output.
    - 7f. Let maximum length be 74.
  - 8. Append (what remains of) line to output.
  - 9. Append a U+000D CARRIAGE RETURN character (CR) to output.
  - 10. Append a U+000A LINE FEED character (LF) to output.

- [x] [collecting vCard subproperties](https://html.spec.whatwg.org/#collecting-vcard-subproperties) — When the steps above require the user agent to obtain the result of collecting vCard subproperties named subname in subitem, the user agent must run the following steps:
  - 1. Let value be the empty string.
  - 2. For each property named subname in the item subitem, run the following substeps:
    - 2a. If the value of the property is itself an item, then skip this property.
    - 2b. If this is not the first property named subname in subitem (ignoring any that were skipped by the previous step), then append a U+002C COMMA character (,) to value.
    - 2c. Append the result of escaping the vCard text string given by the value of the property to value.
  - 3. Return value.

- [x] [collecting the first vCard subproperty](https://html.spec.whatwg.org/#collecting-the-first-vcard-subproperty) — When the steps above require the user agent to obtain the result of collecting the first vCard subproperty named subname in subitem, the user agent must run the following steps:
  - 1. If there are no properties named subname in subitem, then return the empty string.
  - 2. If the value of the first property named subname in subitem is an item, then return the empty string.
  - 3. Return the result of escaping the vCard text string given by the value of the first property named subname in subitem.

- [x] [escape the vCard text string](https://html.spec.whatwg.org/#escaping-the-vcard-text-string) — When the above algorithms say the user agent is to escape the vCard text string value, the user agent must use the following steps:
  - 1. Prefix every U+005C REVERSE SOLIDUS character (\) in value with another U+005C REVERSE SOLIDUS character (\).
  - 2. Prefix every U+002C COMMA character (,) in value with a U+005C REVERSE SOLIDUS character (\).
  - 3. Prefix every U+003B SEMICOLON character (;) in value with a U+005C REVERSE SOLIDUS character (\).
  - 4. Replace every U+000D CARRIAGE RETURN U+000A LINE FEED character pair (CRLF) in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
  - 5. Replace every remaining U+000D CARRIAGE RETURN (CR) or U+000A LINE FEED (LF) character in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
  - 6. Return the mutated value.

##### [5.3.1.2 Examples](https://html.spec.whatwg.org/#examples-2)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.3.2 vEvent](https://html.spec.whatwg.org/#vevent)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: md-vevent md-vevent-attach md-vevent-categories md-vevent-class md-vevent-comment md-vevent-description md-vevent-geo md-vevent-location md-vevent-resources md-vevent-status md-vevent-summary md-vevent-dtend md-vevent-dtstart md-vevent-duration md-vevent-transp md-vevent-contact md-vevent-url md-vevent-uid md-vevent-exdate md-vevent-rdate md-vevent-rrule md-vevent-created md-vevent-last-modified md-vevent-sequence -->

- [x] [The value must be text and must match the following syntax](https://html.spec.whatwg.org/#concept-property-value) — The value must be text and must match the following syntax:
  - 1. Optionally, either a U+002B PLUS SIGN character (+) or a U+002D HYPHEN-MINUS character (-).
  - 2. One or more ASCII digits.
  - 3. Optionally*, a U+002E FULL STOP character (.) followed by one or more ASCII digits.
  - 4. A U+003B SEMICOLON character (;).
  - 5. Optionally, either a U+002B PLUS SIGN character (+) or a U+002D HYPHEN-MINUS character (-).
  - 6. One or more ASCII digits.
  - 7. Optionally*, a U+002E FULL STOP character (.) followed by one or more ASCII digits.

- [x] [valid vevent duration string](https://html.spec.whatwg.org/#valid-vevent-duration-string) — A string is a valid vevent duration string if it matches the following pattern:
  - 1. A U+0050 LATIN CAPITAL LETTER P character (P).
  - 2. One of the following: A valid non-negative integer followed by a U+0057 LATIN CAPITAL LETTER W character (W). The integer represents a duration of that number of weeks.; At least one, and possible both in this order, of the following:
    - 2a. A valid non-negative integer followed by a U+0044 LATIN CAPITAL LETTER D character (D). The integer represents a duration of that number of days.
    - 2b. A U+0054 LATIN CAPITAL LETTER T character (T) followed by any one of the following, or the first and second of the following in that order, or the second and third of the following in that order, or all three of the following in this order:
      - 2b.i. A valid non-negative integer followed by a U+0048 LATIN CAPITAL LETTER H character (H). The integer represents a duration of that number of hours.
      - 2b.ii. A valid non-negative integer followed by a U+004D LATIN CAPITAL LETTER M character (M). The integer represents a duration of that number of minutes.
      - 2b.iii. A valid non-negative integer followed by a U+0053 LATIN CAPITAL LETTER S character (S). The integer represents a duration of that number of seconds.

##### [5.3.2.1 Conversion to iCalendar](https://html.spec.whatwg.org/#conversion-to-icalendar)

Status: n/a (microdata isn't processed, as in browsers)

- [x] [extract any vEvent data represented by those nodes](https://html.spec.whatwg.org/#extracting-vevent-data) — Given a list of nodes nodes in a Document, a user agent must run the following algorithm to extract any vEvent data represented by those nodes:
  - 1. If none of the nodes in nodes are items with the type http://microformats.org/profile/hcalendar#vevent, then there is no vEvent data. Abort the algorithm, returning nothing.
  - 2. Let output be an empty string.
  - 3. Add an iCalendar line with the type "BEGIN" and the value "VCALENDAR" to output.
  - 4. Add an iCalendar line with the type "PRODID" and the value equal to a user-agent-specific string representing the user agent to output.
  - 5. Add an iCalendar line with the type "VERSION" and the value "2.0" to output.
  - 6. For each node node in nodes that is an item with the type http://microformats.org/profile/hcalendar#vevent, run the following steps:
    - 6a. Add an iCalendar line with the type "BEGIN" and the value "VEVENT" to output.
    - 6b. Add an iCalendar line with the type "DTSTAMP" and a value consisting of an iCalendar DATE-TIME string representing the current date and time, with the annotation "VALUE=DATE-TIME", to output. [RFC5545]
    - 6c. For each element element that is a property of the item node: for each name name in element's property names, run the appropriate set of substeps from the following list: If the property's value is an item Skip the property. If the property is dtendIf the property is dtstartIf the property is exdateIf the property is rdateIf the property is createdIf the property is last-modified Let value be the…
    - 6d. Add an iCalendar line with the type "END" and the value "VEVENT" to output.
  - 7. Add an iCalendar line with the type "END" and the value "VCALENDAR" to output.

- [x] [add an iCalendar line](https://html.spec.whatwg.org/#add-an-icalendar-line) — When the above algorithm says that the user agent is to add an iCalendar line consisting of a type type, a value value, and optionally an annotation, to a string output, it must run the following steps:
  - 1. Let line be an empty string.
  - 2. Append type, converted to ASCII uppercase, to line.
  - 3. If there is an annotation:
    - 3a. Append a U+003B SEMICOLON character (;) to line.
    - 3b. Append the annotation to line.
  - 4. Append a U+003A COLON character (:) to line.
  - 5. Prefix every U+005C REVERSE SOLIDUS character (\) in value with another U+005C REVERSE SOLIDUS character (\).
  - 6. Prefix every U+002C COMMA character (,) in value with a U+005C REVERSE SOLIDUS character (\).
  - 7. Prefix every U+003B SEMICOLON character (;) in value with a U+005C REVERSE SOLIDUS character (\).
  - 8. Replace every U+000D CARRIAGE RETURN U+000A LINE FEED character pair (CRLF) in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
  - 9. Replace every remaining U+000D CARRIAGE RETURN (CR) or U+000A LINE FEED (LF) character in value with a U+005C REVERSE SOLIDUS character (\) followed by a U+006E LATIN SMALL LETTER N character (n).
  - 10. Append value to line.
  - 11. Let maximum length be 75.
  - 12. While line's code point length is greater than maximum length:
    - 12a. Append the first maximum length code points of line to output.
    - 12b. Remove the first maximum length code points from line.
    - 12c. Append a U+000D CARRIAGE RETURN character (CR) to output.
    - 12d. Append a U+000A LINE FEED character (LF) to output.
    - 12e. Append a U+0020 SPACE character to output.
    - 12f. Let maximum length be 74.
  - 13. Append (what remains of) line to output.
  - 14. Append a U+000D CARRIAGE RETURN character (CR) to output.
  - 15. Append a U+000A LINE FEED character (LF) to output.

##### [5.3.2.2 Examples](https://html.spec.whatwg.org/#examples-3)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.3.3 Licensing works](https://html.spec.whatwg.org/#licensing-works)

Status: n/a (microdata isn't processed, as in browsers)
<!-- definitions: md-work md-work-work md-work-title md-work-author md-work-license -->

##### [5.3.3.1 Examples](https://html.spec.whatwg.org/#examples-4)

Status: n/a (microdata isn't processed, as in browsers)

### [5.4 Converting HTML to other formats](https://html.spec.whatwg.org/#converting-html-to-other-formats)

Status: n/a (microdata isn't processed, as in browsers)

#### [5.4.1 JSON](https://html.spec.whatwg.org/#json)

Status: n/a (microdata isn't processed, as in browsers)

- [x] [extract the microdata from those nodes into a JSON form](https://html.spec.whatwg.org/#extracting-json) — Given a list of nodes nodes in a Document, a user agent must run the following algorithm to extract the microdata from those nodes into a JSON form:
  - 1. Let result be an empty object.
  - 2. Let items be an empty array.
  - 3. For each node in nodes, check if the element is a top-level microdata item, and if it is then get the object for that element and add it to items.
  - 4. Add an entry to result called "items" whose value is the array items.
  - 5. Return the result of serializing result to JSON in the shortest possible way (meaning no whitespace between tokens, no unnecessary zero digits in numbers, and only using Unicode escapes in strings for characters that do not have a dedicated escape sequence), and with a lowercase "e" used, when appropriate, in the representation of any numbers. [JSON]

- [x] [get the object](https://html.spec.whatwg.org/#get-the-object) — When the user agent is to get the object for an item item, optionally with a list of elements memory, it must run the following substeps:
  - 1. Let result be an empty object.
  - 2. If no memory was passed to the algorithm, let memory be an empty list.
  - 3. Add item to memory.
  - 4. If the item has any item types, add an entry to result called "type" whose value is an array listing the item types of item, in the order they were specified on the itemtype attribute.
  - 5. If the item has a global identifier, add an entry to result called "id" whose value is the global identifier of item.
  - 6. Let properties be an empty object.
  - 7. For each element element that has one or more property names and is one of the properties of the item item, in the order those elements are given by the algorithm that returns the properties of an item, run the following substeps:
    - 7a. Let value be the property value of element.
    - 7b. If value is an item, then: if value is in memory, then let value be the string "ERROR". Otherwise, get the object for value, passing a copy of memory, and then replace value with the object returned from those steps.
    - 7c. For each name name in element's property names, run the following substeps:
      - 7c.i. If there is no entry named name in properties, then add an entry named name to properties whose value is an empty array.
      - 7c.ii. Append value to the entry named name in properties.
  - 8. Add an entry to result called "properties" whose value is the object properties.
  - 9. Return result.
