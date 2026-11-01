# HTML review checklist: 2.3 Common microsyntaxes

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 54 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html common-microsyntaxes` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [2.3 Common microsyntaxes](https://html.spec.whatwg.org/#common-microsyntaxes)

Status: unreviewed

#### [2.3.1 Common parser idioms](https://html.spec.whatwg.org/#common-parser-idioms)

Status: unreviewed

#### [2.3.2 Boolean attributes](https://html.spec.whatwg.org/#boolean-attributes)

Status: unreviewed
<!-- definitions: boolean-attribute -->

#### [2.3.3 Keywords and enumerated attributes](https://html.spec.whatwg.org/#keywords-and-enumerated-attributes)

Status: unreviewed
<!-- definitions: enumerated-attribute invalid-value-default missing-value-default -->

- [ ] [To determine the state of an attribute, use the following steps](https://html.spec.whatwg.org/#empty-value-default) — To determine the state of an attribute, use the following steps:
  - 1. If the attribute is not specified:
    - 1a. If the attribute has a missing value default state defined, then return that missing value default state.
    - 1b. Otherwise, return no state.
  - 2. If the attribute's value is an ASCII case-insensitive match for one of the keywords defined for the attribute, then return the state represented by that keyword.
  - 3. If the attribute has an empty value default state defined and the attribute's value is the empty string, then return that empty value default state.
  - 4. If the attribute has an invalid value default state defined, then return that invalid value default state.
  - 5. Return no state.

- [ ] [canonical keyword](https://html.spec.whatwg.org/#canonical-keyword) — For reflection purposes, states which have any keywords mapping to them are said to have a canonical keyword. This is determined as follows:
  - 1. If there is only one keyword mapping to the given state, then it is that keyword.
  - 2. If there is only one conforming keyword mapping to the given state, then it is that conforming keyword.
  - 3. If there are two conforming keywords mapping to the given state, and one is the empty string, then the canonical keyword will be the conforming keyword that is not the empty string.
  - 4. Otherwise, the canonical keyword for the state will be explicitly given in the specification for the attribute.

#### [2.3.4 Numbers](https://html.spec.whatwg.org/#numbers)

Status: unreviewed

##### [2.3.4.1 Signed integers](https://html.spec.whatwg.org/#signed-integers)

Status: unreviewed

- [ ] [valid integer](https://html.spec.whatwg.org/#valid-integer) — A string is a valid integer if it consists of one or more ASCII digits, optionally prefixed with a U+002D HYPHEN-MINUS character (-).

- [ ] [rules for parsing integers](https://html.spec.whatwg.org/#rules-for-parsing-integers) — The rules for parsing integers are as given in the following algorithm. When invoked, the steps must be followed in the order given, aborting at the first step that returns a value. This algorithm will return either an integer or an error.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Let sign have the value "positive".
  - 4. Skip ASCII whitespace within input given position.
  - 5. If position is past the end of input, return an error.
  - 6. If the character indicated by position (the first character) is a U+002D HYPHEN-MINUS character (-): Otherwise, if the character indicated by position (the first character) is a U+002B PLUS SIGN character (+):
    - 6a. Let sign be "negative".
    - 6b. Advance position to the next character.
    - 6c. If position is past the end of input, return an error.
    - 6a. Advance position to the next character. (The "+" is ignored, but it is not conforming.)
    - 6b. If position is past the end of input, return an error.
  - 7. If the character indicated by position is not an ASCII digit, then return an error.
  - 8. Collect a sequence of code points that are ASCII digits from input given position, and interpret the resulting sequence as a base-ten integer. Let value be that integer.
  - 9. If sign is "positive", return value, otherwise return the result of subtracting value from zero.

##### [2.3.4.2 Non-negative integers](https://html.spec.whatwg.org/#non-negative-integers)

Status: unreviewed

- [ ] [valid non-negative integer](https://html.spec.whatwg.org/#valid-non-negative-integer) — A string is a valid non-negative integer if it consists of one or more ASCII digits.

- [ ] [rules for parsing non-negative integers](https://html.spec.whatwg.org/#rules-for-parsing-non-negative-integers) — The rules for parsing non-negative integers are as given in the following algorithm. When invoked, the steps must be followed in the order given, aborting at the first step that returns a value. This algorithm will return either zero, a positive integer, or an error.
  - 1. Let input be the string being parsed.
  - 2. Let value be the result of parsing input using the rules for parsing integers.
  - 3. If value is an error, return an error.
  - 4. If value is less than zero, return an error.
  - 5. Return value.

##### [2.3.4.3 Floating-point numbers](https://html.spec.whatwg.org/#floating-point-numbers)

Status: unreviewed

- [ ] [valid floating-point number](https://html.spec.whatwg.org/#valid-floating-point-number) — A string is a valid floating-point number if it consists of:
  - 1. Optionally, a U+002D HYPHEN-MINUS character (-).
  - 2. One or both of the following, in the given order:
    - 2a. A series of one or more ASCII digits.
    - 2b. Both of the following, in the given order:
      - 2b.i. A single U+002E FULL STOP character (.).
      - 2b.ii. A series of one or more ASCII digits.
  - 3. Optionally:
    - 3a. Either a U+0065 LATIN SMALL LETTER E character (e) or a U+0045 LATIN CAPITAL LETTER E character (E).
    - 3b. Optionally, a U+002D HYPHEN-MINUS character (-) or U+002B PLUS SIGN character (+).
    - 3c. A series of one or more ASCII digits.

- [ ] [best representation of the number n as a floating-point number](https://html.spec.whatwg.org/#best-representation-of-the-number-as-a-floating-point-number) — The best representation of the number n as a floating-point number is the string obtained from running ToString(n). The abstract operation ToString is not uniquely determined. When there are multiple possible strings that could be obtained from ToString for a particular value, the user agent must always return the same string for that value (though it may differ from the value used by other user agents).

- [ ] [rules for parsing floating-point number values](https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values) — The rules for parsing floating-point number values are as given in the following algorithm. This algorithm must be aborted at the first step that returns something. This algorithm will return either a number or an error.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Let value have the value 1.
  - 4. Let divisor have the value 1.
  - 5. Let exponent have the value 1.
  - 6. Skip ASCII whitespace within input given position.
  - 7. If position is past the end of input, return an error.
  - 8. If the character indicated by position is a U+002D HYPHEN-MINUS character (-): Otherwise, if the character indicated by position (the first character) is a U+002B PLUS SIGN character (+):
    - 8a. Change value and divisor to −1.
    - 8b. Advance position to the next character.
    - 8c. If position is past the end of input, return an error.
    - 8a. Advance position to the next character. (The "+" is ignored, but it is not conforming.)
    - 8b. If position is past the end of input, return an error.
  - 9. If the character indicated by position is a U+002E FULL STOP (.), and that is not the last character in input, and the character after the character indicated by position is an ASCII digit, then set value to zero and jump to the step labeled fraction.
  - 10. If the character indicated by position is not an ASCII digit, then return an error.
  - 11. Collect a sequence of code points that are ASCII digits from input given position, and interpret the resulting sequence as a base-ten integer. Multiply value by that integer.
  - 12. If position is past the end of input, jump to the step labeled conversion.
  - 13. Fraction: If the character indicated by position is a U+002E FULL STOP (.), run these substeps:
    - 13a. Advance position to the next character.
    - 13b. If position is past the end of input, or if the character indicated by position is not an ASCII digit, U+0065 LATIN SMALL LETTER E (e), or U+0045 LATIN CAPITAL LETTER E (E), then jump to the step labeled conversion.
    - 13c. If the character indicated by position is a U+0065 LATIN SMALL LETTER E character (e) or a U+0045 LATIN CAPITAL LETTER E character (E), skip the remainder of these substeps.
    - 13d. Fraction loop: Multiply divisor by ten.
    - 13e. Add the value of the character indicated by position, interpreted as a base-ten digit (0..9) and divided by divisor, to value.
    - 13f. Advance position to the next character.
    - 13g. If position is past the end of input, then jump to the step labeled conversion.
    - 13h. If the character indicated by position is an ASCII digit, jump back to the step labeled fraction loop in these substeps.
  - 14. If the character indicated by position is U+0065 (e) or a U+0045 (E):
    - 14a. Advance position to the next character.
    - 14b. If position is past the end of input, then jump to the step labeled conversion.
    - 14c. If the character indicated by position is a U+002D HYPHEN-MINUS character (-): Otherwise, if the character indicated by position is a U+002B PLUS SIGN character (+):
      - 14c.i. Change exponent to −1.
      - 14c.ii. Advance position to the next character.
      - 14c.iii. If position is past the end of input, then jump to the step labeled conversion.
      - 14c.i. Advance position to the next character.
      - 14c.ii. If position is past the end of input, then jump to the step labeled conversion.
    - 14d. If the character indicated by position is not an ASCII digit, then jump to the step labeled conversion.
    - 14e. Collect a sequence of code points that are ASCII digits from input given position, and interpret the resulting sequence as a base-ten integer. Multiply exponent by that integer.
    - 14f. Multiply value by ten raised to the exponentth power.
  - 15. Conversion: Let S be the set of finite IEEE 754 double-precision floating-point values except −0, but with two special values added: 21024 and −21024.
  - 16. Let rounded-value be the number in S that is closest to value, selecting the number with an even significand if there are two equally close values. (The two special values 21024 and −21024 are considered to have even significands for this purpose.)
  - 17. If rounded-value is 21024 or −21024, return an error.
  - 18. Return rounded-value.

##### [2.3.4.4 Percentages and lengths](https://html.spec.whatwg.org/#percentages-and-dimensions)

Status: unreviewed

- [ ] [rules for parsing dimension values](https://html.spec.whatwg.org/#rules-for-parsing-dimension-values) — The rules for parsing dimension values are as given in the following algorithm. When invoked, the steps must be followed in the order given, aborting at the first step that returns a value. This algorithm will return either a number greater than or equal to 0.0, or failure; if a number is returned,…
  - 1. Let input be the string being parsed.
  - 2. Let position be a position variable for input, initially pointing at the start of input.
  - 3. Skip ASCII whitespace within input given position.
  - 4. If position is past the end of input or the code point at position within input is not an ASCII digit, then return failure.
  - 5. Collect a sequence of code points that are ASCII digits from input given position, and interpret the resulting sequence as a base-ten integer. Let value be that number.
  - 6. If position is past the end of input, then return value as a length.
  - 7. If the code point at position within input is U+002E (.):
    - 7a. Advance position by 1.
    - 7b. If position is past the end of input or the code point at position within input is not an ASCII digit, then return the current dimension value with value, input, and position.
    - 7c. Let divisor have the value 1.
    - 7d. While true:
      - 7d.i. Multiply divisor by ten.
      - 7d.ii. Add the value of the code point at position within input, interpreted as a base-ten digit (0..9) and divided by divisor, to value.
      - 7d.iii. Advance position by 1.
      - 7d.iv. If position is past the end of input, then return value as a length.
      - 7d.v. If the code point at position within input is not an ASCII digit, then break.
  - 8. Return the current dimension value with value, input, and position.

- [ ] [current dimension value](https://html.spec.whatwg.org/#current-dimension-value) — The current dimension value, given value, input, and position, is determined as follows:
  - 1. If position is past the end of input, then return value as a length.
  - 2. If the code point at position within input is U+0025 (%), then return value as a percentage.
  - 3. Return value as a length.

##### [2.3.4.5 Nonzero percentages and lengths](https://html.spec.whatwg.org/#nonzero-percentages-and-lengths)

Status: unreviewed

- [ ] [rules for parsing nonzero dimension values](https://html.spec.whatwg.org/#rules-for-parsing-non-zero-dimension-values) — The rules for parsing nonzero dimension values are as given in the following algorithm. When invoked, the steps must be followed in the order given, aborting at the first step that returns a value. This algorithm will return either a number greater than 0.0, or an error; if a number is returned, th…
  - 1. Let input be the string being parsed.
  - 2. Let value be the result of parsing input using the rules for parsing dimension values.
  - 3. If value is an error, return an error.
  - 4. If value is zero, return an error.
  - 5. If value is a percentage, return value as a percentage.
  - 6. Return value as a length.

##### [2.3.4.6 Lists of floating-point numbers](https://html.spec.whatwg.org/#lists-of-floating-point-numbers)

Status: unreviewed

- [ ] [valid list of floating-point numbers](https://html.spec.whatwg.org/#valid-list-of-floating-point-numbers) — A valid list of floating-point numbers is a number of valid floating-point numbers separated by U+002C COMMA characters, with no other characters (e.g. no ASCII whitespace). In addition, there might be restrictions on the number of floating-point numbers that can be given, or on the range of values allowed.

- [ ] [rules for parsing a list of floating-point numbers](https://html.spec.whatwg.org/#rules-for-parsing-a-list-of-floating-point-numbers) — The rules for parsing a list of floating-point numbers are as follows:
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Let numbers be an initially empty list of floating-point numbers. This list will be the result of this algorithm.
  - 4. Collect a sequence of code points that are ASCII whitespace, U+002C COMMA, or U+003B SEMICOLON characters from input given position. This skips past any leading delimiters.
  - 5. While position is not past the end of input:
    - 5a. Collect a sequence of code points that are not ASCII whitespace, U+002C COMMA, U+003B SEMICOLON, ASCII digits, U+002E FULL STOP, or U+002D HYPHEN-MINUS characters from input given position. This skips past leading garbage.
    - 5b. Collect a sequence of code points that are not ASCII whitespace, U+002C COMMA, or U+003B SEMICOLON characters from input given position, and let unparsed number be the result.
    - 5c. Let number be the result of parsing unparsed number using the rules for parsing floating-point number values.
    - 5d. If number is an error, set number to zero.
    - 5e. Append number to numbers.
    - 5f. Collect a sequence of code points that are ASCII whitespace, U+002C COMMA, or U+003B SEMICOLON characters from input given position. This skips past the delimiter.
  - 6. Return numbers.

##### [2.3.4.7 Lists of dimensions](https://html.spec.whatwg.org/#lists-of-dimensions)

Status: unreviewed

- [ ] [rules for parsing a list of dimensions](https://html.spec.whatwg.org/#rules-for-parsing-a-list-of-dimensions) — The rules for parsing a list of dimensions are as follows. These rules return a list of zero or more pairs consisting of a number and a unit, the unit being one of percentage, relative, and absolute.
  - 1. Let raw input be the string being parsed.
  - 2. If the last character in raw input is a U+002C COMMA character (,), then remove that character from raw input.
  - 3. Split the string raw input on commas. Let raw tokens be the resulting list of tokens.
  - 4. Let result be an empty list of number/unit pairs.
  - 5. For each token in raw tokens, run the following substeps:
    - 5a. Let input be the token.
    - 5b. Let position be a pointer into input, initially pointing at the start of the string.
    - 5c. Let value be the number 0.
    - 5d. Let unit be absolute.
    - 5e. If position is past the end of input, set unit to relative and jump to the last substep.
    - 5f. If the character at position is an ASCII digit, collect a sequence of code points that are ASCII digits from input given position, interpret the resulting sequence as an integer in base ten, and increment value by that integer.
    - 5g. If the character at position is U+002E (.):
      - 5g.i. Collect a sequence of code points consisting of ASCII whitespace and ASCII digits from input given position. Let s be the resulting sequence.
      - 5g.ii. Remove all ASCII whitespace in s.
      - 5g.iii. If s is not the empty string:
        - 5g.iii.A. Let length be the number of characters in s (after the spaces were removed).
        - 5g.iii.B. Let fraction be the result of interpreting s as a base-ten integer, and then dividing that number by 10length.
        - 5g.iii.C. Increment value by fraction.
    - 5h. Skip ASCII whitespace within input given position.
    - 5i. If the character at position is a U+0025 PERCENT SIGN character (%), then set unit to percentage. Otherwise, if the character at position is a U+002A ASTERISK character (*), then set unit to relative.
    - 5j. Add an entry to result consisting of the number given by value and the unit given by unit.
  - 6. Return the list result.

#### [2.3.5 Dates and times](https://html.spec.whatwg.org/#dates-and-times)

Status: unreviewed
<!-- definitions: proleptic-gregorian-calendar proleptic-gregorian-date -->

- [ ] [number of days in month month of year year](https://html.spec.whatwg.org/#number-of-days-in-month-month-of-year-year) — In the algorithms below, the number of days in month month of year year is: 31 if month is 1, 3, 5, 7, 8, 10, or 12; 30 if month is 4, 6, 9, or 11; 29 if month is 2 and year is a number divisible by 400, or if year is a number divisible by 4 but not by 100; and 28 otherwise. This takes into account leap years in the Gregorian calendar. [GREGORIAN]

##### [2.3.5.1 Months](https://html.spec.whatwg.org/#months)

Status: unreviewed
<!-- definitions: concept-month -->

- [ ] [valid month string](https://html.spec.whatwg.org/#valid-month-string) — A string is a valid month string representing a year year and month month if it consists of the following components in the given order:
  - 1. Four or more ASCII digits, representing year, where year > 0
  - 2. A U+002D HYPHEN-MINUS character (-)
  - 3. Two ASCII digits, representing the month month, in the range 1 ≤ month ≤ 12

- [ ] [parse a month string](https://html.spec.whatwg.org/#parse-a-month-string) — The rules to parse a month string are as follows. This will return either a year and month, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a month component to obtain year and month. If this returns nothing, then fail.
  - 4. If position is not beyond the end of input, then fail.
  - 5. Return year and month.

- [ ] [parse a month component](https://html.spec.whatwg.org/#parse-a-month-component) — The rules to parse a month component, given an input string and a position, are as follows. This will return either a year and a month, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not at least four characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let year be that number.
  - 2. If year is not a number greater than zero, then fail.
  - 3. If position is beyond the end of input or if the character at position is not a U+002D HYPHEN-MINUS character, then fail. Otherwise, move position forwards one character.
  - 4. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let month be that number.
  - 5. If month is not a number in the range 1 ≤ month ≤ 12, then fail.
  - 6. Return year and month.

##### [2.3.5.2 Dates](https://html.spec.whatwg.org/#dates)

Status: unreviewed
<!-- definitions: concept-date -->

- [ ] [valid date string](https://html.spec.whatwg.org/#valid-date-string) — A string is a valid date string representing a year year, month month, and day day if it consists of the following components in the given order:
  - 1. A valid month string, representing year and month
  - 2. A U+002D HYPHEN-MINUS character (-)
  - 3. Two ASCII digits, representing day, in the range 1 ≤ day ≤ maxday where maxday is the number of days in the month month and year year

- [ ] [parse a date string](https://html.spec.whatwg.org/#parse-a-date-string) — The rules to parse a date string are as follows. This will return either a date, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a date component to obtain year, month, and day. If this returns nothing, then fail.
  - 4. If position is not beyond the end of input, then fail.
  - 5. Let date be the date with year year, month month, and day day.
  - 6. Return date.

- [ ] [parse a date component](https://html.spec.whatwg.org/#parse-a-date-component) — The rules to parse a date component, given an input string and a position, are as follows. This will return either a year, a month, and a day, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Parse a month component to obtain year and month. If this returns nothing, then fail.
  - 2. Let maxday be the number of days in month month of year year.
  - 3. If position is beyond the end of input or if the character at position is not a U+002D HYPHEN-MINUS character, then fail. Otherwise, move position forwards one character.
  - 4. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let day be that number.
  - 5. If day is not a number in the range 1 ≤ day ≤ maxday, then fail.
  - 6. Return year, month, and day.

##### [2.3.5.3 Yearless dates](https://html.spec.whatwg.org/#yearless-dates)

Status: unreviewed
<!-- definitions: concept-yearless-date -->

- [ ] [valid yearless date string](https://html.spec.whatwg.org/#valid-yearless-date-string) — A string is a valid yearless date string representing a month month and a day day if it consists of the following components in the given order:
  - 1. Optionally, two U+002D HYPHEN-MINUS characters (-)
  - 2. Two ASCII digits, representing the month month, in the range 1 ≤ month ≤ 12
  - 3. A U+002D HYPHEN-MINUS character (-)
  - 4. Two ASCII digits, representing day, in the range 1 ≤ day ≤ maxday where maxday is the number of days in the month month and any arbitrary leap year (e.g. 4 or 2000)

- [ ] [parse a yearless date string](https://html.spec.whatwg.org/#parse-a-yearless-date-string) — The rules to parse a yearless date string are as follows. This will return either a month and a day, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a yearless date component to obtain month and day. If this returns nothing, then fail.
  - 4. If position is not beyond the end of input, then fail.
  - 5. Return month and day.

- [ ] [parse a yearless date component](https://html.spec.whatwg.org/#parse-a-yearless-date-component) — The rules to parse a yearless date component, given an input string and a position, are as follows. This will return either a month and a day, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Collect a sequence of code points that are U+002D HYPHEN-MINUS characters (-) from input given position. If the collected sequence is not exactly zero or two characters long, then fail.
  - 2. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let month be that number.
  - 3. If month is not a number in the range 1 ≤ month ≤ 12, then fail.
  - 4. Let maxday be the number of days in month month of any arbitrary leap year (e.g. 4 or 2000).
  - 5. If position is beyond the end of input or if the character at position is not a U+002D HYPHEN-MINUS character, then fail. Otherwise, move position forwards one character.
  - 6. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let day be that number.
  - 7. If day is not a number in the range 1 ≤ day ≤ maxday, then fail.
  - 8. Return month and day.

##### [2.3.5.4 Times](https://html.spec.whatwg.org/#times)

Status: unreviewed
<!-- definitions: concept-time -->

- [ ] [valid time string](https://html.spec.whatwg.org/#valid-time-string) — A string is a valid time string representing an hour hour, a minute minute, and a second second if it consists of the following components in the given order:
  - 1. Two ASCII digits, representing hour, in the range 0 ≤ hour ≤ 23
  - 2. A U+003A COLON character (:)
  - 3. Two ASCII digits, representing minute, in the range 0 ≤ minute ≤ 59
  - 4. If second is nonzero, or optionally if second is zero:
    - 4a. A U+003A COLON character (:)
    - 4b. Two ASCII digits, representing the integer part of second, in the range 0 ≤ s ≤ 59
    - 4c. If second is not an integer, or optionally if second is an integer:
      - 4c.i. A U+002E FULL STOP character (.)
      - 4c.ii. One, two, or three ASCII digits, representing the fractional part of second

- [ ] [parse a time string](https://html.spec.whatwg.org/#parse-a-time-string) — The rules to parse a time string are as follows. This will return either a time, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a time component to obtain hour, minute, and second. If this returns nothing, then fail.
  - 4. If position is not beyond the end of input, then fail.
  - 5. Let time be the time with hour hour, minute minute, and second second.
  - 6. Return time.

- [ ] [parse a time component](https://html.spec.whatwg.org/#parse-a-time-component) — The rules to parse a time component, given an input string and a position, are as follows. This will return either an hour, a minute, and a second, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let hour be that number.
  - 2. If hour is not a number in the range 0 ≤ hour ≤ 23, then fail.
  - 3. If position is beyond the end of input or if the character at position is not a U+003A COLON character, then fail. Otherwise, move position forwards one character.
  - 4. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let minute be that number.
  - 5. If minute is not a number in the range 0 ≤ minute ≤ 59, then fail.
  - 6. Let second be 0.
  - 7. If position is not beyond the end of input and the character at position is U+003A (:):
    - 7a. Advance position to the next character in input.
    - 7b. If position is beyond the end of input, or at the last character in input, or if the next two characters in input starting at position are not both ASCII digits, then fail.
    - 7c. Collect a sequence of code points that are either ASCII digits or U+002E FULL STOP characters from input given position. If the collected sequence is three characters long, or if it is longer than three characters long and the third character is not a U+002E FULL STOP character, or if it has more than one U+002E FULL STOP character, then fail. Otherwise, interpret the resulting sequence as a base…
    - 7d. If second is not a number in the range 0 ≤ second < 60, then fail.
  - 8. Return hour, minute, and second.

##### [2.3.5.5 Local dates and times](https://html.spec.whatwg.org/#local-dates-and-times)

Status: unreviewed
<!-- definitions: concept-datetime-local -->

- [ ] [valid local date and time string](https://html.spec.whatwg.org/#valid-local-date-and-time-string) — A string is a valid local date and time string representing a date and time if it consists of the following components in the given order:
  - 1. A valid date string representing the date
  - 2. A U+0054 LATIN CAPITAL LETTER T character (T) or a U+0020 SPACE character
  - 3. A valid time string representing the time

- [ ] [valid normalized local date and time string](https://html.spec.whatwg.org/#valid-normalised-local-date-and-time-string) — A string is a valid normalized local date and time string representing a date and time if it consists of the following components in the given order:
  - 1. A valid date string representing the date
  - 2. A U+0054 LATIN CAPITAL LETTER T character (T)
  - 3. A valid time string representing the time, expressed as the shortest possible string for the given time (e.g. omitting the seconds component entirely if the given time is zero seconds past the minute)

- [ ] [parse a local date and time string](https://html.spec.whatwg.org/#parse-a-local-date-and-time-string) — The rules to parse a local date and time string are as follows. This will return either a date and time, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a date component to obtain year, month, and day. If this returns nothing, then fail.
  - 4. If position is beyond the end of input or if the character at position is neither a U+0054 LATIN CAPITAL LETTER T character (T) nor a U+0020 SPACE character, then fail. Otherwise, move position forwards one character.
  - 5. Parse a time component to obtain hour, minute, and second. If this returns nothing, then fail.
  - 6. If position is not beyond the end of input, then fail.
  - 7. Let date be the date with year year, month month, and day day.
  - 8. Let time be the time with hour hour, minute minute, and second second.
  - 9. Return date and time.

##### [2.3.5.6 Time zones](https://html.spec.whatwg.org/#time-zones)

Status: unreviewed
<!-- definitions: concept-timezone -->

- [ ] [valid time-zone offset string](https://html.spec.whatwg.org/#valid-time-zone-offset-string) — A string is a valid time-zone offset string representing a time-zone offset if it consists of either:

- [ ] [parse a time-zone offset string](https://html.spec.whatwg.org/#parse-a-time-zone-offset-string) — The rules to parse a time-zone offset string are as follows. This will return either a time-zone offset, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a time-zone offset component to obtain timezonehours and timezoneminutes. If this returns nothing, then fail.
  - 4. If position is not beyond the end of input, then fail.
  - 5. Return the time-zone offset that is timezonehours hours and timezoneminutes minutes from UTC.

- [ ] [parse a time-zone offset component](https://html.spec.whatwg.org/#parse-a-time-zone-offset-component) — The rules to parse a time-zone offset component, given an input string and a position, are as follows. This will return either time-zone hours and time-zone minutes, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. If the character at position is a U+005A LATIN CAPITAL LETTER Z character (Z): Otherwise, if the character at position is either a U+002B PLUS SIGN (+) or a U+002D HYPHEN-MINUS (-): Otherwise, fail.
    - 1a. Let timezonehours be 0.
    - 1b. Let timezoneminutes be 0.
    - 1c. Advance position to the next character in input.
    - 1a. If the character at position is a U+002B PLUS SIGN (+), let sign be "positive". Otherwise, it's a U+002D HYPHEN-MINUS (-); let sign be "negative".
    - 1b. Advance position to the next character in input.
    - 1c. Collect a sequence of code points that are ASCII digits from input given position. Let s be the collected sequence.
    - 1d. If s is exactly two characters long: If s is exactly four characters long: Otherwise, fail.
      - 1d.i. Interpret s as a base-ten integer. Let timezonehours be that number.
      - 1d.ii. If position is beyond the end of input or if the character at position is not a U+003A COLON character, then fail. Otherwise, move position forwards one character.
      - 1d.iii. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let timezoneminutes be that number.
      - 1d.i. Interpret the first two characters of s as a base-ten integer. Let timezonehours be that number.
      - 1d.ii. Interpret the last two characters of s as a base-ten integer. Let timezoneminutes be that number.
    - 1e. If timezonehours is not a number in the range 0 ≤ timezonehours ≤ 23, then fail.
    - 1f. If sign is "negative", then negate timezonehours.
    - 1g. If timezoneminutes is not a number in the range 0 ≤ timezoneminutes ≤ 59, then fail.
    - 1h. If sign is "negative", then negate timezoneminutes.
  - 2. Return timezonehours and timezoneminutes.

##### [2.3.5.7 Global dates and times](https://html.spec.whatwg.org/#global-dates-and-times)

Status: unreviewed
<!-- definitions: concept-datetime -->

- [ ] [valid global date and time string](https://html.spec.whatwg.org/#valid-global-date-and-time-string) — A string is a valid global date and time string representing a date, time, and a time-zone offset if it consists of the following components in the given order:
  - 1. A valid date string representing the date
  - 2. A U+0054 LATIN CAPITAL LETTER T character (T) or a U+0020 SPACE character
  - 3. A valid time string representing the time
  - 4. A valid time-zone offset string representing the time-zone offset

- [ ] [parse a global date and time string](https://html.spec.whatwg.org/#parse-a-global-date-and-time-string) — The rules to parse a global date and time string are as follows. This will return either a time in UTC, with associated time-zone offset information for roundtripping or display purposes, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and…
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Parse a date component to obtain year, month, and day. If this returns nothing, then fail.
  - 4. If position is beyond the end of input or if the character at position is neither a U+0054 LATIN CAPITAL LETTER T character (T) nor a U+0020 SPACE character, then fail. Otherwise, move position forwards one character.
  - 5. Parse a time component to obtain hour, minute, and second. If this returns nothing, then fail.
  - 6. If position is beyond the end of input, then fail.
  - 7. Parse a time-zone offset component to obtain timezonehours and timezoneminutes. If this returns nothing, then fail.
  - 8. If position is not beyond the end of input, then fail.
  - 9. Let time be the moment in time at year year, month month, day day, hours hour, minute minute, second second, subtracting timezonehours hours and timezoneminutes minutes. That moment in time is a moment in the UTC time zone.
  - 10. Let timezone be timezonehours hours and timezoneminutes minutes from UTC.
  - 11. Return time and timezone.

##### [2.3.5.8 Weeks](https://html.spec.whatwg.org/#weeks)

Status: unreviewed
<!-- definitions: concept-week -->

- [ ] [week number of the last day](https://html.spec.whatwg.org/#week-number-of-the-last-day) — The week number of the last day of a week-year with 53 weeks is 53; the week number of the last day of a week-year with 52 weeks is 52.

- [ ] [valid week string](https://html.spec.whatwg.org/#valid-week-string) — A string is a valid week string representing a week-year year and week week if it consists of the following components in the given order:
  - 1. Four or more ASCII digits, representing year, where year > 0
  - 2. A U+002D HYPHEN-MINUS character (-)
  - 3. A U+0057 LATIN CAPITAL LETTER W character (W)
  - 4. Two ASCII digits, representing the week week, in the range 1 ≤ week ≤ maxweek, where maxweek is the week number of the last day of week-year year

- [ ] [parse a week string](https://html.spec.whatwg.org/#parse-a-week-string) — The rules to parse a week string are as follows. This will return either a week-year number and week number, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not at least four characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let year be that number.
  - 4. If year is not a number greater than zero, then fail.
  - 5. If position is beyond the end of input or if the character at position is not a U+002D HYPHEN-MINUS character, then fail. Otherwise, move position forwards one character.
  - 6. If position is beyond the end of input or if the character at position is not a U+0057 LATIN CAPITAL LETTER W character (W), then fail. Otherwise, move position forwards one character.
  - 7. Collect a sequence of code points that are ASCII digits from input given position. If the collected sequence is not exactly two characters long, then fail. Otherwise, interpret the resulting sequence as a base-ten integer. Let week be that number.
  - 8. Let maxweek be the week number of the last day of year year.
  - 9. If week is not a number in the range 1 ≤ week ≤ maxweek, then fail.
  - 10. If position is not beyond the end of input, then fail.
  - 11. Return the week-year number year and the week number week.

##### [2.3.5.9 Durations](https://html.spec.whatwg.org/#durations)

Status: unreviewed
<!-- definitions: concept-duration duration-time-component-scale -->

- [ ] [valid duration string](https://html.spec.whatwg.org/#valid-duration-string) — A string is a valid duration string representing a duration t if it consists of either of the following:

- [ ] [duration time component](https://html.spec.whatwg.org/#duration-time-component) — A duration time component is a string consisting of the following components:
  - 1. Zero or more ASCII whitespace.
  - 2. One or more ASCII digits, representing a number of time units, scaled by the duration time component scale specified (see below) to represent a number of seconds.
  - 3. If the duration time component scale specified is 1 (i.e. the units are seconds), then, optionally, a U+002E FULL STOP character (.) followed by one, two, or three ASCII digits, representing a fraction of a second.
  - 4. Zero or more ASCII whitespace.
  - 5. One of the following characters, representing the duration time component scale of the time unit used in the numeric part of the duration time component: U+0057 LATIN CAPITAL LETTER W characterU+0077 LATIN SMALL LETTER W characterWeeks. The scale is 604800.U+0044 LATIN CAPITAL LETTER D characterU+0064 LATIN SMALL LETTER D characterDays. The scale is 86400.U+0048 LATIN CAPITAL LETTER H characterU+…
  - 6. Zero or more ASCII whitespace.

- [ ] [parse a duration string](https://html.spec.whatwg.org/#parse-a-duration-string) — The rules to parse a duration string are as follows. This will return either a duration or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Let months, seconds, and component count all be zero.
  - 4. Let M-disambiguator be minutes.
  - 5. Skip ASCII whitespace within input given position.
  - 6. If position is past the end of input, then fail.
  - 7. If the character in input pointed to by position is a U+0050 LATIN CAPITAL LETTER P character, then advance position to the next character, set M-disambiguator to months, and skip ASCII whitespace within input given position.
  - 8. While true:
    - 8a. Let units be undefined. It will be assigned one of the following values: years, months, weeks, days, hours, minutes, and seconds.
    - 8b. Let next character be undefined. It is used to process characters from the input.
    - 8c. If position is past the end of input, then break.
    - 8d. If the character in input pointed to by position is a U+0054 LATIN CAPITAL LETTER T character, then advance position to the next character, set M-disambiguator to minutes, skip ASCII whitespace within input given position, and continue.
    - 8e. Set next character to the character in input pointed to by position.
    - 8f. If next character is a U+002E FULL STOP character (.), then let N be 0. (Do not advance position. That is taken care of below.) Otherwise, if next character is an ASCII digit, then collect a sequence of code points that are ASCII digits from input given position, interpret the resulting sequence as a base-ten integer, and let N be that number. Otherwise, next character is not part of a number; fa…
    - 8g. If position is past the end of input, then fail.
    - 8h. Set next character to the character in input pointed to by position, and this time advance position to the next character. (If next character was a U+002E FULL STOP character (.) before, it will still be that character this time.)
    - 8i. If next character is U+002E (.): Otherwise:
      - 8i.i. Collect a sequence of code points that are ASCII digits from input given position. Let s be the resulting sequence.
      - 8i.ii. If s is the empty string, then fail.
      - 8i.iii. Let length be the number of characters in s.
      - 8i.iv. Let fraction be the result of interpreting s as a base-ten integer, and then dividing that number by 10length.
      - 8i.v. Increment N by fraction.
      - 8i.vi. Skip ASCII whitespace within input given position.
      - 8i.vii. If position is past the end of input, then fail.
      - 8i.viii. Set next character to the character in input pointed to by position, and advance position to the next character.
      - 8i.ix. If next character is neither a U+0053 LATIN CAPITAL LETTER S character nor a U+0073 LATIN SMALL LETTER S character, then fail.
      - 8i.x. Set units to seconds.
      - 8i.i. If next character is ASCII whitespace, then skip ASCII whitespace within input given position, set next character to the character in input pointed to by position, and advance position to the next character.
      - 8i.ii. If next character is a U+0059 LATIN CAPITAL LETTER Y character, or a U+0079 LATIN SMALL LETTER Y character, set units to years and set M-disambiguator to months. If next character is a U+004D LATIN CAPITAL LETTER M character or a U+006D LATIN SMALL LETTER M character, and M-disambiguator is months, then set units to months. If next character is a U+0057 LATIN CAPITAL LETTER W character or a U+007…
    - 8j. Increment component count.
    - 8k. Let multiplier be 1.
    - 8l. If units is years, multiply multiplier by 12 and set units to months.
    - 8m. If units is months, add the product of N and multiplier to months. Otherwise:
      - 8m.i. If units is weeks, multiply multiplier by 7 and set units to days.
      - 8m.ii. If units is days, multiply multiplier by 24 and set units to hours.
      - 8m.iii. If units is hours, multiply multiplier by 60 and set units to minutes.
      - 8m.iv. If units is minutes, multiply multiplier by 60 and set units to seconds.
      - 8m.v. Forcibly, units is now seconds. Add the product of N and multiplier to seconds.
    - 8n. Skip ASCII whitespace within input given position.
  - 9. If component count is zero, fail.
  - 10. If months is not zero, fail.
  - 11. Return the duration consisting of seconds seconds.

##### [2.3.5.10 Vaguer moments in time](https://html.spec.whatwg.org/#vaguer-moments-in-time)

Status: unreviewed

- [ ] [valid date string with optional time](https://html.spec.whatwg.org/#valid-date-string-with-optional-time) — A string is a valid date string with optional time if it is also one of the following:

- [ ] [parse a date or time string](https://html.spec.whatwg.org/#parse-a-date-or-time-string) — The rules to parse a date or time string are as follows. The algorithm will return either a date, a time, a global date and time, or nothing. If at any point the algorithm says that it "fails", this means that it is aborted at that point and returns nothing.
  - 1. Let input be the string being parsed.
  - 2. Let position be a pointer into input, initially pointing at the start of the string.
  - 3. Set start position to the same position as position.
  - 4. Set the date present and time present flags to true.
  - 5. Parse a date component to obtain year, month, and day. If this fails, then set the date present flag to false.
  - 6. If date present is true, and position is not beyond the end of input, and the character at position is either a U+0054 LATIN CAPITAL LETTER T character (T) or a U+0020 SPACE character, then advance position to the next character in input. Otherwise, if date present is true, and either position is beyond the end of input or the character at position is neither a U+0054 LATIN CAPITAL LETTER T chara…
  - 7. If the time present flag is true, then parse a time component to obtain hour, minute, and second. If this returns nothing, then fail.
  - 8. If the date present and time present flags are both true, but position is beyond the end of input, then fail.
  - 9. If the date present and time present flags are both true, parse a time-zone offset component to obtain timezonehours and timezoneminutes. If this returns nothing, then fail.
  - 10. If position is not beyond the end of input, then fail.
  - 11. If the date present flag is true and the time present flag is false, then let date be the date with year year, month month, and day day, and return date. Otherwise, if the time present flag is true and the date present flag is false, then let time be the time with hour hour, minute minute, and second second, and return time. Otherwise, let time be the moment in time at year year, month month, day…

#### [2.3.6 Legacy colors](https://html.spec.whatwg.org/#colours)

Status: unreviewed

- [ ] [rules for parsing a legacy color value](https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-colour-value) — Some obsolete legacy attributes parse colors using the rules for parsing a legacy color value, given a string input. They will return either a CSS color or failure.
  - 1. If input is the empty string, then return failure.
  - 2. Strip leading and trailing ASCII whitespace from input.
  - 3. If input is an ASCII case-insensitive match for "transparent", then return failure.
  - 4. If input is an ASCII case-insensitive match for one of the named colors, then return the CSS color corresponding to that keyword. [CSSCOLOR]
  - 5. If input's code point length is four, and the first character in input is U+0023 (#), and the last three characters of input are all ASCII hex digits:
    - 5a. Let result be a CSS color.
    - 5b. Interpret the second character of input as a hexadecimal digit; let the red component of result be the resulting number multiplied by 17.
    - 5c. Interpret the third character of input as a hexadecimal digit; let the green component of result be the resulting number multiplied by 17.
    - 5d. Interpret the fourth character of input as a hexadecimal digit; let the blue component of result be the resulting number multiplied by 17.
    - 5e. Return result.
  - 6. Replace any code points greater than U+FFFF in input (i.e., any characters that are not in the basic multilingual plane) with "00".
  - 7. If input's code point length is greater than 128, truncate input, leaving only the first 128 characters.
  - 8. If the first character in input is U+0023 (#), then remove it.
  - 9. Replace any character in input that is not an ASCII hex digit with U+0030 (0).
  - 10. While input's code point length is zero or not a multiple of three, append U+0030 (0) to input.
  - 11. Split input into three strings of equal code point length, to obtain three components. Let length be the code point length that all of those components have (one third the code point length of input).
  - 12. If length is greater than 8, then remove the leading length-8 characters in each component, and let length be 8.
  - 13. While length is greater than two and the first character in each component is U+0030 (0), remove that character and reduce length by one.
  - 14. If length is still greater than two, truncate each component, leaving only the first two characters in each.
  - 15. Let result be a CSS color.
  - 16. Interpret the first component as a hexadecimal number; let the red component of result be the resulting number.
  - 17. Interpret the second component as a hexadecimal number; let the green component of result be the resulting number.
  - 18. Interpret the third component as a hexadecimal number; let the blue component of result be the resulting number.
  - 19. Return result.

#### [2.3.7 Space-separated tokens](https://html.spec.whatwg.org/#space-separated-tokens)

Status: unreviewed

- [ ] [set of space-separated tokens](https://html.spec.whatwg.org/#set-of-space-separated-tokens) — A set of space-separated tokens is a string containing zero or more words (known as tokens) separated by one or more ASCII whitespace, where words consist of any string of one or more characters, none of which are ASCII whitespace.

- [ ] [unordered set of unique space-separated tokens](https://html.spec.whatwg.org/#unordered-set-of-unique-space-separated-tokens) — An unordered set of unique space-separated tokens is a set of space-separated tokens where none of the tokens are duplicated.

- [ ] [ordered set of unique space-separated tokens](https://html.spec.whatwg.org/#ordered-set-of-unique-space-separated-tokens) — An ordered set of unique space-separated tokens is a set of space-separated tokens where none of the tokens are duplicated but where the order of the tokens is meaningful.

#### [2.3.8 Comma-separated tokens](https://html.spec.whatwg.org/#comma-separated-tokens)

Status: unreviewed

- [ ] [set of comma-separated tokens](https://html.spec.whatwg.org/#set-of-comma-separated-tokens) — A set of comma-separated tokens is a string containing zero or more tokens each separated from the next by a single U+002C COMMA character (,), where tokens consist of any string of zero or more characters, neither beginning nor ending with ASCII whitespace, nor containing any U+002C COMMA characters (,), and optionally surrounded by ASCII whitespace.

#### [2.3.9 References](https://html.spec.whatwg.org/#syntax-references)

Status: unreviewed

- [ ] [valid hash-name reference](https://html.spec.whatwg.org/#valid-hash-name-reference) — A valid hash-name reference to an element of type type is a string consisting of a U+0023 NUMBER SIGN character (#) followed by a string which exactly matches the value of the name attribute of an element with type type in the same tree.

- [ ] [rules for parsing a hash-name reference](https://html.spec.whatwg.org/#rules-for-parsing-a-hash-name-reference) — The rules for parsing a hash-name reference to an element of type type, given a context node scope, are as follows:
  - 1. If the string being parsed does not contain a U+0023 NUMBER SIGN character, or if the first such character in the string is the last character in the string, then return null.
  - 2. Let s be the string from the character immediately after the first U+0023 NUMBER SIGN character in the string being parsed up to the end of that string.
  - 3. Return the first element of type type in scope's tree, in tree order, that has an id or name attribute whose value is s, or null if there is no such element.

#### [2.3.10 Media queries](https://html.spec.whatwg.org/#mq)

Status: unreviewed

- [ ] [valid media query list](https://html.spec.whatwg.org/#valid-media-query-list) — A string is a valid media query list if it matches the <media-query-list> production of Media Queries. [MQ]

- [ ] [matches the environment](https://html.spec.whatwg.org/#matches-the-environment) — A string matches the environment of the user if it is the empty string, a string consisting of only ASCII whitespace, or is a media query list that matches the user's environment according to the definitions given in Media Queries. [MQ]

#### [2.3.11 Unique internal values](https://html.spec.whatwg.org/#unique-values)

Status: unreviewed
<!-- definitions: unique-internal-value -->

- [ ] [new unique internal value](https://html.spec.whatwg.org/#new-unique-internal-value) — To create a new unique internal value, return a unique internal value that has never previously been returned by this algorithm.
