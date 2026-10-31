# Infra review checklist

Generated from <https://infra.spec.whatwg.org/> on 2026-10-30 by `tools/spec_checklist.py`: 79 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[infra:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note.

## [Abstract](https://infra.spec.whatwg.org/#abstract)

Status: unreviewed

## [Goals](https://infra.spec.whatwg.org/#goals)

Status: unreviewed

## [1. Usage](https://infra.spec.whatwg.org/#usage)

Status: unreviewed

## [2. Conventions](https://infra.spec.whatwg.org/#conventions)

Status: unreviewed

### [2.1. Conformance](https://infra.spec.whatwg.org/#conformance)

Status: unreviewed

### [2.2. Compliance with other specifications](https://infra.spec.whatwg.org/#other-specs)

Status: unreviewed
<!-- definitions: willful-violation -->

### [2.3. Terminology](https://infra.spec.whatwg.org/#terminology)

Status: unreviewed
<!-- definitions: user-agent implementation implementation-defined -->

### [2.4. Privacy concerns](https://infra.spec.whatwg.org/#privacy)

Status: unreviewed
<!-- definitions: tracking-vector -->

## [3. Algorithms](https://infra.spec.whatwg.org/#algorithms)

Status: unreviewed

### [3.1. Conformance](https://infra.spec.whatwg.org/#algorithm-conformance)

Status: unreviewed

### [3.2. Avoid limits on algorithm inputs](https://infra.spec.whatwg.org/#algorithm-limits)

Status: unreviewed

### [3.3. Declaration](https://infra.spec.whatwg.org/#algorithm-declaration)

Status: unreviewed

### [3.4. Parameters](https://infra.spec.whatwg.org/#algorithm-params)

Status: unreviewed

### [3.5. Variables](https://infra.spec.whatwg.org/#variables)

Status: unreviewed

### [3.6. Control flow](https://infra.spec.whatwg.org/#algorithm-control-flow)

Status: unreviewed

### [3.7. Conditional abort](https://infra.spec.whatwg.org/#algorithm-conditional-abort)

Status: unreviewed
<!-- definitions: abort-when if-aborted -->

### [3.8. Conditional statements](https://infra.spec.whatwg.org/#algorithm-conditional-statements)

Status: unreviewed

### [3.9. Iteration](https://infra.spec.whatwg.org/#algorithm-iteration)

Status: unreviewed
<!-- definitions: iteration-while iteration-continue iteration-break -->

### [3.10. Assertions](https://infra.spec.whatwg.org/#assertions)

Status: unreviewed
<!-- definitions: assert -->

### [3.11. Identity and equality](https://infra.spec.whatwg.org/#identity-and-equality)

Status: unreviewed
<!-- definitions: is equality equal -->

## [4. Primitive data types](https://infra.spec.whatwg.org/#primitive-data-types)

Status: unreviewed

### [4.1. Nulls](https://infra.spec.whatwg.org/#nulls)

Status: unreviewed

### [4.2. Booleans](https://infra.spec.whatwg.org/#booleans)

Status: unreviewed
<!-- definitions: boolean -->

### [4.3. Numbers](https://infra.spec.whatwg.org/#numbers)

Status: unreviewed
<!-- definitions: 8-bit-unsigned-integer 16-bit-unsigned-integer 32-bit-unsigned-integer 64-bit-unsigned-integer 128-bit-unsigned-integer 8-bit-signed-integer 16-bit-signed-integer 32-bit-signed-integer 64-bit-signed-integer -->

### [4.4. Bytes](https://infra.spec.whatwg.org/#bytes)

Status: unreviewed
<!-- definitions: byte byte-value ascii-byte -->

### [4.5. Byte sequences](https://infra.spec.whatwg.org/#byte-sequences)

Status: unreviewed
<!-- definitions: byte-sequence byte-sequence-length byte-case-insensitive byte-sequence-starts-with -->

- [ ] [byte-lowercase](https://infra.spec.whatwg.org/#byte-lowercase) — To byte-lowercase a byte sequence, increase each byte it contains, in the range 0x41 (A) to 0x5A (Z), inclusive, by 0x20.

- [ ] [byte-uppercase](https://infra.spec.whatwg.org/#byte-uppercase) — To byte-uppercase a byte sequence, subtract each byte it contains, in the range 0x61 (a) to 0x7A (z), inclusive, by 0x20.

- [ ] [prefix (byte sequence)](https://infra.spec.whatwg.org/#byte-sequence-prefix) — A byte sequence potentialPrefix is a prefix of a byte sequence input if the following steps return true:
  - 1. Let i be 0.
  - 2. While true:
    - 2a. If i is greater than or equal to potentialPrefix’s length, then return true.
    - 2b. If i is greater than or equal to input’s length, then return false.
    - 2c. Let potentialPrefixByte be the ith byte of potentialPrefix.
    - 2d. Let inputByte be the ith byte of input.
    - 2e. Return false if potentialPrefixByte is not inputByte.
    - 2f. Set i to i + 1.

- [ ] [byte less than](https://infra.spec.whatwg.org/#byte-less-than) — A byte sequence a is byte less than a byte sequence b if the following steps return true:
  - 1. If b is a prefix of a, then return false.
  - 2. If a is a prefix of b, then return true.
  - 3. Let n be the smallest index such that the nth byte of a is different from the nth byte of b. (There has to be such an index, since neither byte sequence is a prefix of the other.)
  - 4. If the nth byte of a is less than the nth byte of b, then return true.
  - 5. Return false.

- [ ] [isomorphic decode](https://infra.spec.whatwg.org/#isomorphic-decode) — To isomorphic decode a byte sequence input, return a string whose code point length is equal to input’s length and whose code points have the same values as the values of input’s bytes, in the same order.

### [4.6. Code points](https://infra.spec.whatwg.org/#code-points)

Status: unreviewed
<!-- definitions: code-point code-point-value leading-surrogate trailing-surrogate surrogate scalar-value noncharacter ascii-code-point ascii-tab-or-newline ascii-whitespace c0-control c0-control-or-space control ascii-digit ascii-upper-hex-digit ascii-lower-hex-digit ascii-hex-digit ascii-upper-alpha ascii-lower-alpha ascii-alpha ascii-alphanumeric -->

### [4.7. Strings](https://infra.spec.whatwg.org/#strings)

Status: unreviewed
<!-- definitions: string code-unit string-length string-code-point-length ascii-string isomorphic-string scalar-value-string string-is string-starts-with string-ends-with code-unit-substring-by-positions code-unit-substring-to-the-end-of-the-string code-point-substring-by-positions code-point-substring-to-the-end-of-the-string ascii-case-insensitive string-position-variable -->

- [ ] [convert (string,JavaScript string)](https://infra.spec.whatwg.org/#javascript-string-convert) — To convert a string into a scalar value string, replace any surrogates with U+FFFD (�).

- [ ] [code unit prefix](https://infra.spec.whatwg.org/#code-unit-prefix) — A string potentialPrefix is a code unit prefix of a string input if the following steps return true:
  - 1. Let i be 0.
  - 2. While true:
    - 2a. If i is greater than or equal to potentialPrefix’s length, then return true.
    - 2b. If i is greater than or equal to input’s length, then return false.
    - 2c. Let potentialPrefixCodeUnit be the ith code unit of potentialPrefix.
    - 2d. Let inputCodeUnit be the ith code unit of input.
    - 2e. Return false if potentialPrefixCodeUnit is not inputCodeUnit.
    - 2f. Set i to i + 1.

- [ ] [code unit suffix](https://infra.spec.whatwg.org/#code-unit-suffix) — A string potentialSuffix is a code unit suffix of a string input if the following steps return true:
  - 1. Let i be 1.
  - 2. While true:
    - 2a. Let potentialSuffixIndex be potentialSuffix’s length − i.
    - 2b. Let inputIndex be input’s length − i.
    - 2c. If potentialSuffixIndex is less than 0, then return true.
    - 2d. If inputIndex is less than 0, then return false.
    - 2e. Let potentialSuffixCodeUnit be the potentialSuffixIndexth code unit of potentialSuffix.
    - 2f. Let inputCodeUnit be the inputIndexth code unit of input.
    - 2g. Return false if potentialSuffixCodeUnit is not inputCodeUnit.
    - 2h. Set i to i + 1.

- [ ] [code unit less than](https://infra.spec.whatwg.org/#code-unit-less-than) — A string a is code unit less than a string b if the following steps return true:
  - 1. If b is a code unit prefix of a, then return false.
  - 2. If a is a code unit prefix of b, then return true.
  - 3. Let n be the smallest index such that the nth code unit of a is different from the nth code unit of b. (There has to be such an index, since neither string is a prefix of the other.)
  - 4. If the nth code unit of a is less than the nth code unit of b, then return true.
  - 5. Return false.

- [ ] [code unit substring](https://infra.spec.whatwg.org/#code-unit-substring) — The code unit substring from start with length length within a string string is determined as follows:
  - 1. Assert: start and length are nonnegative.
  - 2. Assert: start + length is less than or equal to string’s length.
  - 3. Let result be the empty string.
  - 4. For each i of the range from start to start + length, exclusive: append the ith code unit of string to result.
  - 5. Return result.

- [ ] [code point substring](https://infra.spec.whatwg.org/#code-point-substring) — The code point substring within a string string from start with length length is determined as follows:
  - 1. Assert: start and length are nonnegative.
  - 2. Assert: start + length is less than or equal to string’s code point length.
  - 3. Let result be the empty string.
  - 4. For each i of the range from start to start + length, exclusive: append the ith code point of string to result.
  - 5. Return result.

- [ ] [isomorphic encode](https://infra.spec.whatwg.org/#isomorphic-encode) — To isomorphic encode an isomorphic string input: return a byte sequence whose length is equal to input’s code point length and whose bytes have the same values as the values of input’s code points, in the same order.

- [ ] [ASCII lowercase](https://infra.spec.whatwg.org/#ascii-lowercase) — To ASCII lowercase a string, replace all ASCII upper alphas in the string with their corresponding code point in ASCII lower alpha.

- [ ] [ASCII uppercase](https://infra.spec.whatwg.org/#ascii-uppercase) — To ASCII uppercase a string, replace all ASCII lower alphas in the string with their corresponding code point in ASCII upper alpha.

- [ ] [ASCII encode](https://infra.spec.whatwg.org/#ascii-encode) — To ASCII encode an ASCII string input: return the isomorphic encoding of input.

- [ ] [ASCII decode](https://infra.spec.whatwg.org/#ascii-decode) — To ASCII decode a byte sequence input, run these steps:
  - 1. Assert: all bytes in input are ASCII bytes.
  - 2. Return the isomorphic decoding of input.

- [ ] [strip newlines](https://infra.spec.whatwg.org/#strip-newlines) — To strip newlines from a string, remove any U+000A LF and U+000D CR code points from the string.

- [ ] [normalize newlines](https://infra.spec.whatwg.org/#normalize-newlines) — To normalize newlines in a string, replace every U+000D CR U+000A LF code point pair with a single U+000A LF code point, and then replace every remaining U+000D CR code point with a U+000A LF code point.

- [ ] [strip leading and trailing ASCII whitespace](https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace) — To strip leading and trailing ASCII whitespace from a string, remove all ASCII whitespace that are at the start or the end of the string.

- [ ] [strip and collapse ASCII whitespace](https://infra.spec.whatwg.org/#strip-and-collapse-ascii-whitespace) — To strip and collapse ASCII whitespace in a string, replace any sequence of one or more consecutive code points that are ASCII whitespace in the string with a single U+0020 SPACE code point, and then remove any leading and trailing ASCII whitespace from that string.

- [ ] [collect a sequence of code points](https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points) — To collect a sequence of code points meeting a condition condition from a string input, given a position variable position tracking the position of the calling algorithm within input:
  - 1. Let result be the empty string.
  - 2. While position doesn’t point past the end of input and the code point at position within input meets the condition condition:
    - 2a. Append that code point to the end of result.
    - 2b. Advance position by 1.
  - 3. Return result.

- [ ] [skip ASCII whitespace](https://infra.spec.whatwg.org/#skip-ascii-whitespace) — To skip ASCII whitespace within a string input given a position variable position, collect a sequence of code points that are ASCII whitespace from input given position. The collected code points are not used, but position is still updated.

- [ ] [strictly split a string](https://infra.spec.whatwg.org/#strictly-split) — To strictly split a string input on a particular delimiter code point delimiter:
  - 1. Let position be a position variable for input, initially pointing at the start of input.
  - 2. Let tokens be a list of strings, initially empty.
  - 3. Let token be the result of collecting a sequence of code points that are not equal to delimiter from input, given position.
  - 4. Append token to tokens.
  - 5. While position is not past the end of input:
    - 5a. Assert: the code point at position within input is delimiter.
    - 5b. Advance position by 1.
    - 5c. Let token be the result of collecting a sequence of code points that are not equal to delimiter from input, given position.
    - 5d. Append token to tokens.
  - 6. Return tokens.

- [ ] [split a string input on ASCII whitespace](https://infra.spec.whatwg.org/#split-on-ascii-whitespace) — To split a string input on ASCII whitespace:
  - 1. Let position be a position variable for input, initially pointing at the start of input.
  - 2. Let tokens be a list of strings, initially empty.
  - 3. Skip ASCII whitespace within input given position.
  - 4. While position is not past the end of input:
    - 4a. Let token be the result of collecting a sequence of code points that are not ASCII whitespace from input, given position.
    - 4b. Append token to tokens.
    - 4c. Skip ASCII whitespace within input given position.
  - 5. Return tokens.

- [ ] [split a string input on commas](https://infra.spec.whatwg.org/#split-on-commas) — To split a string input on commas:
  - 1. Let position be a position variable for input, initially pointing at the start of input.
  - 2. Let tokens be a list of strings, initially empty.
  - 3. While position is not past the end of input:
    - 3a. Let token be the result of collecting a sequence of code points that are not U+002C (,) from input, given position.
    - 3b. Strip leading and trailing ASCII whitespace from token.
    - 3c. Append token to tokens.
    - 3d. If position is not past the end of input:
      - 3d.i. Assert: the code point at position within input is U+002C (,).
      - 3d.ii. Advance position by 1.
  - 4. Return tokens.

- [ ] [concatenate (string)](https://infra.spec.whatwg.org/#string-concatenate) — To concatenate a list of strings list, using an optional separator string separator, run these steps:
  - 1. If list is empty, then return the empty string.
  - 2. If separator is not given, then set separator to the empty string.
  - 3. Return a string whose contents are list’s items, in order, separated from each other by separator.

### [4.8. Time](https://infra.spec.whatwg.org/#time)

Status: unreviewed

### [4.9. Singletons](https://infra.spec.whatwg.org/#singleton)

Status: unreviewed
<!-- definitions: allowed blocked failure success -->

## [5. Data structures](https://infra.spec.whatwg.org/#data-structures)

Status: unreviewed

### [5.1. Lists](https://infra.spec.whatwg.org/#lists)

Status: unreviewed
<!-- definitions: list list-item list-contain list-size list-is-empty list-slice-from list-slice-to -->

- [ ] [append (list)](https://infra.spec.whatwg.org/#list-append) — To append to a list that is not an ordered set is to add the given item to the end of the list.

- [ ] [extend (list)](https://infra.spec.whatwg.org/#list-extend) — To extend a list that is not an ordered set A with a list B, for each item of B, append item to A.

- [ ] [prepend (list)](https://infra.spec.whatwg.org/#list-prepend) — To prepend to a list that is not an ordered set is to add the given item to the beginning of the list.

- [ ] [replace (list)](https://infra.spec.whatwg.org/#list-replace) — To replace within a list that is not an ordered set is to replace all items from the list that match a given condition with the given item, or do nothing if none do.

- [ ] [insert (list,set)](https://infra.spec.whatwg.org/#list-insert) — To insert an item into a list before an index is to add the given item to the list between the given index − 1 and the given index. If the given index is 0, then prepend the given item to the list.

- [ ] [remove (list,set)](https://infra.spec.whatwg.org/#list-remove) — To remove zero or more items from a list:
  - 1. Let sizeBefore be list’s size.
  - 2. Remove all items from the list that match a given condition, or do nothing if none do.
  - 3. Return true if list’s size is less than sizeBefore; otherwise false.

- [ ] [empty (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-empty) — To empty a list is to remove all of its items.

- [ ] [get the indices (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-get-the-indices) — To get the indices of a list, return the range from 0 to the list’s size, exclusive.

- [ ] [iterate (list,set)](https://infra.spec.whatwg.org/#list-iterate) — To iterate over a list, performing a set of steps on each item in order, use phrasing of the form "For each item of list", and then operate on item in the subsequent prose.

- [ ] [slice (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-slice) — To slice a list list, with an optional integer from (default 0) and an optional integer to (default list’s size), perform the following steps. They return a list of the same designation as list.
  - 1. Assert: from is greater than or equal to 0.
  - 2. Assert: to is less than or equal to list’s size.
  - 3. Assert: to is greater than or equal to from.
  - 4. Let slice be a new list of the same designation as list.
  - 5. For each i of the range from to to, exclusive: append list[i] to slice.
  - 6. Return slice.

- [ ] [has duplicates (list)](https://infra.spec.whatwg.org/#list-has-duplicates) — To check if a list list has duplicates:
  - 1. Let seenItems be « ».
  - 2. For each item of list:
    - 2a. If seenItems contains item, then return true.
    - 2b. Append item to seenItems.
  - 3. Return false.

- [ ] [clone (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-clone) — To clone a list list is to return a slice of list.

- [ ] [reverse (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-reverse) — To reverse a list list is to create a new list reversed, of the same designation, and, for each item of list, prepend item to reversed.

- [ ] [sort in ascending order (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-sort-in-ascending-order) — To sort in ascending order a list list, with a less than algorithm lessThanAlgo, is to create a new list sorted, containing the same items as list but sorted so that according to lessThanAlgo, each item is less than the one following it, if any. For items that sort the same (i.e., for which lessThanAlgo returns false for both comparisons), their relative order in sorted must be the same as it was in list.

- [ ] [sort in descending order (list,stack,queue,set)](https://infra.spec.whatwg.org/#list-sort-in-descending-order) — To sort in descending order a list list, with a less than algorithm lessThanAlgo, is to create a new list sorted, containing the same items as list but sorted so that according to lessThanAlgo, each item is less than the one preceding it, if any. For items that sort the same (i.e., for which lessThanAlgo returns false for both comparisons), their relative order in sorted must be the same as it was in list.

#### [5.1.1. Stacks](https://infra.spec.whatwg.org/#stacks)

Status: unreviewed
<!-- definitions: stack -->

- [ ] [push (stack)](https://infra.spec.whatwg.org/#stack-push) — To push onto a stack is to append to it.

- [ ] [pop (stack)](https://infra.spec.whatwg.org/#stack-pop) — To pop from a stack: if the stack is not empty, then remove its last item and return it; otherwise, return nothing.

- [ ] [peek (stack)](https://infra.spec.whatwg.org/#stack-peek) — To peek into a stack: if the stack is not empty, then return its last item; otherwise, return nothing.

#### [5.1.2. Queues](https://infra.spec.whatwg.org/#queues)

Status: unreviewed
<!-- definitions: queue -->

- [ ] [enqueue (queue)](https://infra.spec.whatwg.org/#queue-enqueue) — To enqueue in a queue is to append to it.

- [ ] [dequeue (queue)](https://infra.spec.whatwg.org/#queue-dequeue) — To dequeue from a queue is to remove its first item and return it, if the queue is not empty, or to return nothing if it is.

#### [5.1.3. Sets](https://infra.spec.whatwg.org/#sets)

Status: unreviewed
<!-- definitions: ordered-set set-subset set-superset set-unordered-equal -->

- [ ] [create (set)](https://infra.spec.whatwg.org/#set-create) — To create a set, given a list input:
  - 1. Let result be an empty set.
  - 2. For each item of input, append item to result.
  - 3. Return result.

- [ ] [append (set)](https://infra.spec.whatwg.org/#set-append) — To append to an ordered set: if the set contains the given item, then do nothing; otherwise, perform the normal list append operation.

- [ ] [extend (set)](https://infra.spec.whatwg.org/#set-extend) — To extend an ordered set A with a list B, for each item of B, append item to A.

- [ ] [prepend (set)](https://infra.spec.whatwg.org/#set-prepend) — To prepend to an ordered set: if the set contains the given item, then do nothing; otherwise, perform the normal list prepend operation.

- [ ] [replace (set)](https://infra.spec.whatwg.org/#set-replace) — To replace within an ordered set set, given item and replacement: if set contains item or replacement, then replace the first instance of either with replacement and remove all other instances.

- [ ] [intersection (set)](https://infra.spec.whatwg.org/#set-intersection) — The intersection of ordered sets A and B, is the result of creating a new ordered set set and, for each item of A, if B contains item, appending item to set.

- [ ] [union (set)](https://infra.spec.whatwg.org/#set-union) — The union of ordered sets A and B, is the result of cloning A as set and, for each item of B, appending item to set.

- [ ] [difference (set)](https://infra.spec.whatwg.org/#set-difference) — The difference of ordered sets A and B, is the result of creating a new ordered set set and, for each item of A, if B does not contain item, appending item to set.

- [ ] [The range](https://infra.spec.whatwg.org/#the-range) — The range n to m, inclusive, creates a new ordered set containing all of the integers from n up to and including m in consecutively increasing order, as long as m is greater than or equal to n.

- [ ] [The range](https://infra.spec.whatwg.org/#the-exclusive-range) — The range n to m, exclusive, creates a new ordered set containing all of the integers from n up to and including m − 1 in consecutively increasing order, as long as m is greater than n. If m equals n, then it creates an empty ordered set.

### [5.2. Maps](https://infra.spec.whatwg.org/#maps)

Status: unreviewed
<!-- definitions: ordered-map map-key map-value map-entry map-with-default map-exists map-size map-is-empty -->

- [ ] [get the value of an entry (map)](https://infra.spec.whatwg.org/#map-get) — To get the value of an entry in an ordered map map given a key key and an optional default:
  - 1. If map does not contain key and default is given, then return default.
  - 2. Assert: map contains key.
  - 3. Return the value of the entry in map whose key equals key.

- [ ] [set the value of an entry (map)](https://infra.spec.whatwg.org/#map-set) — To set the value of an entry in an ordered map to a given value is to update the value of any existing entry if the map contains an entry with the given key, or if none such exists, to add a new entry with the given key/value to the end of the map. We can also denote this by saying, for an ordered map map, key key, and value value, "set map[key] to value".

- [ ] [remove an entry (map)](https://infra.spec.whatwg.org/#map-remove) — To remove an entry from an ordered map is to remove all entries from the map that match a given condition, or do nothing if none do. If the condition is having a certain key, then we can also denote this by saying, for an ordered map map and key key, "remove map[key]".

- [ ] [clear (map)](https://infra.spec.whatwg.org/#map-clear) — To clear an ordered map is to remove all entries from the map.

- [ ] [get the keys (map)](https://infra.spec.whatwg.org/#map-getting-the-keys) — To get the keys of an ordered map, return a new ordered set whose items are each of the keys in the map’s entries.

- [ ] [get the values (map)](https://infra.spec.whatwg.org/#map-getting-the-values) — To get the values of an ordered map, return a new list whose items are each of the values in the map’s entries.

- [ ] [iterate (map)](https://infra.spec.whatwg.org/#map-iterate) — To iterate over an ordered map, performing a set of steps on each entry in order, use phrasing of the form "For each key → value of map", and then operate on key and value in the subsequent prose.

- [ ] [clone (map)](https://infra.spec.whatwg.org/#map-clone) — To clone an ordered map map is to create a new ordered map clone, and, for each key → value of map, set clone[key] to value.

- [ ] [sort in ascending order (map)](https://infra.spec.whatwg.org/#map-sort-in-ascending-order) — To sort in ascending order a map map, with a less than algorithm lessThanAlgo, is to create a new map sorted, containing the same entries as map but sorted so that according to lessThanAlgo, each entry is less than the one following it, if any. For entries that sort the same (i.e., for which lessThanAlgo returns false for both comparisons), their relative order in sorted must be the same as it was in map.

- [ ] [sort in descending order (map)](https://infra.spec.whatwg.org/#map-sort-in-descending-order) — To sort in descending order a map map, with a less than algorithm lessThanAlgo, is to create a new map sorted, containing the same entries as map but sorted so that according to lessThanAlgo, each entry is less than the one preceding it, if any. For entries that sort the same (i.e., for which lessThanAlgo returns false for both comparisons), their relative order in sorted must be the same as it was in map.

### [5.3. Structs](https://infra.spec.whatwg.org/#structs)

Status: unreviewed
<!-- definitions: struct struct-item struct-name -->

- [ ] [itemwise equality (struct)](https://infra.spec.whatwg.org/#struct-itemwise-equality) — The itemwise equality, given structs a and b, returns true if each item of a equals the item of b with the same name; otherwise false. A struct’s definition can specify that its equality is itemwise equality.

#### [5.3.1. Tuples](https://infra.spec.whatwg.org/#tuples)

Status: unreviewed
<!-- definitions: tuple -->

## [6. JSON](https://infra.spec.whatwg.org/#json)

Status: unreviewed

- [ ] [parse a JSON string to a JavaScript value](https://infra.spec.whatwg.org/#parse-a-json-string-to-a-javascript-value) — To parse a JSON string to a JavaScript value, given a string string:
  - 1. Return ? Call(%JSON.parse%, undefined, « string »).

- [ ] [parse JSON bytes to a JavaScript value](https://infra.spec.whatwg.org/#parse-json-bytes-to-a-javascript-value) — To parse JSON bytes to a JavaScript value, given a byte sequence bytes:
  - 1. Let string be the result of running UTF-8 decode on bytes. [ENCODING]
  - 2. Return the result of parsing a JSON string to a JavaScript value given string.

- [ ] [serialize a JavaScript value to a JSON string](https://infra.spec.whatwg.org/#serialize-a-javascript-value-to-a-json-string) — To serialize a JavaScript value to a JSON string, given a JavaScript value value:
  - 1. Let result be ? Call(%JSON.stringify%, undefined, « value »).
  - 2. If result is undefined, then throw a TypeError.
  - 3. Assert: result is a string.
  - 4. Return result.

- [ ] [serialize a JavaScript value to JSON bytes](https://infra.spec.whatwg.org/#serialize-a-javascript-value-to-json-bytes) — To serialize a JavaScript value to JSON bytes, given a JavaScript value value:
  - 1. Let string be the result of serializing a JavaScript value to a JSON string given value.
  - 2. Return the result of running UTF-8 encode on string. [ENCODING]

- [ ] [parse a JSON string to an Infra value](https://infra.spec.whatwg.org/#parse-a-json-string-to-an-infra-value) — To parse a JSON string to an Infra value, given a string string:
  - 1. Let jsValue be ? Call(%JSON.parse%, undefined, « string »).
  - 2. Return the result of converting a JSON-derived JavaScript value to an Infra value, given jsValue.

- [ ] [parse JSON bytes to an Infra value](https://infra.spec.whatwg.org/#parse-json-bytes-to-an-infra-value) — To parse JSON bytes to an Infra value, given a byte sequence bytes:
  - 1. Let string be the result of running UTF-8 decode on bytes. [ENCODING]
  - 2. Return the result of parsing a JSON string to an Infra value given string.

- [ ] [convert a JSON-derived JavaScript value to an Infra value](https://infra.spec.whatwg.org/#convert-a-json-derived-javascript-value-to-an-infra-value) — To convert a JSON-derived JavaScript value to an Infra value, given a JavaScript value jsValue:
  - 1. If jsValue is null, jsValue is a Boolean, jsValue is a String, or jsValue is a Number, then return jsValue.
  - 2. If IsArray(jsValue) is true:
    - 2a. Let result be an empty list.
    - 2b. Let length be ! ToLength(! Get(jsValue, "length")).
    - 2c. For each index of the range 0 to length − 1, inclusive:
      - 2c.i. Let indexName be ! ToString(index).
      - 2c.ii. Let jsValueAtIndex be ! Get(jsValue, indexName).
      - 2c.iii. Let infraValueAtIndex be the result of converting a JSON-derived JavaScript value to an Infra value, given jsValueAtIndex.
      - 2c.iv. Append infraValueAtIndex to result.
    - 2d. Return result.
  - 3. Let result be an empty ordered map.
  - 4. For each key of ! jsValue.[[OwnPropertyKeys]]():
    - 4a. Let jsValueAtKey be ! Get(jsValue, key).
    - 4b. Let infraValueAtKey be the result of converting a JSON-derived JavaScript value to an Infra value, given jsValueAtKey.
    - 4c. Set result[key] to infraValueAtKey.
  - 5. Return result.

- [ ] [serialize an Infra value to a JSON string](https://infra.spec.whatwg.org/#serialize-an-infra-value-to-a-json-string) — To serialize an Infra value to a JSON string, given a string, boolean, number, null, list, or string-keyed map value:
  - 1. Let jsValue be the result of converting an Infra value to a JSON-compatible JavaScript value, given value.
  - 2. Return ! Call(%JSON.stringify%, undefined, « jsValue »).

- [ ] [serialize an Infra value to JSON bytes](https://infra.spec.whatwg.org/#serialize-an-infra-value-to-json-bytes) — To serialize an Infra value to JSON bytes, given a string, boolean, number, null, list, or string-keyed map value:
  - 1. Let string be the result of serializing an Infra value to a JSON string, given value.
  - 2. Return the result of running UTF-8 encode on string. [ENCODING]

- [ ] [convert an Infra value to a JSON-compatible JavaScript value](https://infra.spec.whatwg.org/#convert-an-infra-value-to-a-json-compatible-javascript-value) — To convert an Infra value to a JSON-compatible JavaScript value, given value:
  - 1. If value is a string, boolean, number, or null, then return value.
  - 2. If value is a list:
    - 2a. Let jsValue be ! ArrayCreate(0).
    - 2b. Let i be 0.
    - 2c. For each listItem of value:
      - 2c.i. Let listItemJSValue be the result of converting an Infra value to a JSON-compatible JavaScript value, given listItem.
      - 2c.ii. Perform ! CreateDataPropertyOrThrow(jsValue, ! ToString(i), listItemJSValue).
      - 2c.iii. Set i to i + 1.
    - 2d. Return jsValue.
  - 3. Assert: value is a map.
  - 4. Let jsValue be ! OrdinaryObjectCreate(null).
  - 5. For each mapKey → mapValue of value:
    - 5a. Assert: mapKey is a string.
    - 5b. Let mapValueJSValue be the result of converting an Infra value to a JSON-compatible JavaScript value, given mapValue.
    - 5c. Perform ! CreateDataPropertyOrThrow(jsValue, mapKey, mapValueJSValue).
  - 6. Return jsValue.

## [7. Forgiving base64](https://infra.spec.whatwg.org/#forgiving-base64)

Status: unreviewed

- [ ] [forgiving-base64 encode](https://infra.spec.whatwg.org/#forgiving-base64-encode) — To forgiving-base64 encode given a byte sequence data, apply the base64 algorithm defined in section 4 of RFC 4648 to data and return the result. [RFC4648]

- [ ] [forgiving-base64 decode](https://infra.spec.whatwg.org/#forgiving-base64-decode) — To forgiving-base64 decode given a string data, run these steps:
  - 1. Remove all ASCII whitespace from data.
  - 2. If data’s code point length divides by 4 leaving no remainder:
    - 2a. If data ends with one or two U+003D (=) code points, then remove them from data.
  - 3. If data’s code point length divides by 4 leaving a remainder of 1, then return failure.
  - 4. If data contains a code point that is not one of U+002B (+); U+002F (/); ASCII alphanumeric then return failure.
  - 5. Let output be an empty byte sequence.
  - 6. Let buffer be an empty buffer that can have bits appended to it.
  - 7. Let position be a position variable for data, initially pointing at the start of data.
  - 8. While position does not point past the end of data:
    - 8a. Find the code point pointed to by position in the second column of Table 1: The Base 64 Alphabet of RFC 4648. Let n be the number given in the first cell of the same row. [RFC4648]
    - 8b. Append the six bits corresponding to n, most significant bit first, to buffer.
    - 8c. If buffer has accumulated 24 bits, interpret them as three 8-bit big-endian numbers. Append three bytes with values equal to those numbers to output, in the same order, and then empty buffer.
    - 8d. Advance position by 1.
  - 9. If buffer is not empty, it contains either 12 or 18 bits. If it contains 12 bits, then discard the last four and interpret the remaining eight as an 8-bit big-endian number. If it contains 18 bits, then discard the last two and interpret the remaining 16 as two 8-bit big-endian numbers. Append the one or two bytes with values equal to those one or two numbers to output, in the same order.
  - 10. Return output.

## [8. Namespaces](https://infra.spec.whatwg.org/#namespaces)

Status: unreviewed
<!-- definitions: html-namespace mathml-namespace svg-namespace xlink-namespace xml-namespace xmlns-namespace -->

## [Acknowledgments](https://infra.spec.whatwg.org/#acknowledgments)

Status: unreviewed

## [Intellectual property rights](https://infra.spec.whatwg.org/#ipr)

Status: unreviewed

## [Index](https://infra.spec.whatwg.org/#index)

Status: unreviewed

### [Terms defined by this specification](https://infra.spec.whatwg.org/#index-defined-here)

Status: unreviewed

### [Terms defined by reference](https://infra.spec.whatwg.org/#index-defined-elsewhere)

Status: unreviewed

## [References](https://infra.spec.whatwg.org/#references)

Status: unreviewed

### [Normative References](https://infra.spec.whatwg.org/#normative)

Status: unreviewed

### [Non-Normative References](https://infra.spec.whatwg.org/#informative)

Status: unreviewed
