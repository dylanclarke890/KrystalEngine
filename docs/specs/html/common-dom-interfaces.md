# HTML review checklist: 2.6 Common DOM interfaces

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 45 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html common-dom-interfaces` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [2.6 Common DOM interfaces](https://html.spec.whatwg.org/#common-dom-interfaces)

Status: reviewed (2026-11-01)
> Headings only.

#### [2.6.1 Reflecting content attributes in IDL attributes](https://html.spec.whatwg.org/#reflecting-content-attributes-in-idl-attributes)

Status: reviewed (2026-11-01)
> `HTML/Attributes/ReflectAttribute.hpp`: the getters and setters are its `detail` helpers (`ReflectDOMString`, `ReflectUSVString`, `ReflectBool`, `ReflectLong`, `ReflectUnsignedLong`, `ReflectDouble`); enumerated attributes limited to only known values are `EnumeratedAttribute::ResolveCanonicalKeyword`; `DOMTokenList` attributes are the elements' own (DOM 7.1). Element reflection is `Element::ElementForAttributeInternal` and `SetElementAttribute`.
> Fixed: the `long` getter didn't check the bottom of the range, the `unsigned long` setter wrote the given value rather than the clamped one, the `double` setter didn't write the best representation of the number (`1e-06` for 0.000001), and a clamped range didn't clamp integers too large for `int64` to its maximum.
> `ElementInternals` targets and `FrozenArray<T>?` attributes (attr-associated elements) are only for ARIA, which waits on accessibility (not planned).
<!-- definitions: reflected-target reflected-idl-attribute reflected-content-attribute-name reflect get-the-element get-the-content-attribute set-the-content-attribute delete-the-content-attribute limited-to-only-non-negative-numbers limited-to-only-non-negative-numbers-greater-than-zero limited-to-only-non-negative-numbers-greater-than-zero-with-fallback explicitly-set-attr-element explicitly-set-attr-elements cached-attr-associated-elements cached-attr-associated-elements-object -->

- [x] [The getter steps](https://html.spec.whatwg.org/#limited-to-only-known-values) — The getter steps are:
  - 1. Let element be the result of running this's get the element.
  - 2. Let contentAttributeValue be the result of running this's get the content attribute.
  - 3. Let attributeDefinition be the attribute definition of element's content attribute whose namespace is null and local name is the reflected content attribute name.
  - 4. If attributeDefinition indicates it is an enumerated attribute and the reflected IDL attribute is defined to be limited to only known values:
    - 4a. If contentAttributeValue does not correspond to any state of attributeDefinition (e.g., it is null and there is no missing value default), or if it is in a state of attributeDefinition with no associated keyword value, then return the empty string.
    - 4b. Return the canonical keyword for the state of attributeDefinition that contentAttributeValue corresponds to.
  - 5. If contentAttributeValue is null, then return the empty string.
  - 6. Return contentAttributeValue.

- [x] [The setter steps](https://html.spec.whatwg.org/#limited-to-only-known-values) — The setter steps are to run this's set the content attribute with the given value.

- [x] [The getter steps](https://html.spec.whatwg.org/#limited-to-only-known-values) — The getter steps are:
  - 1. Let element be the result of running this's get the element.
  - 2. Let contentAttributeValue be the result of running this's get the content attribute.
  - 3. Let attributeDefinition be the attribute definition of element's content attribute whose namespace is null and local name is the reflected content attribute name.
  - 4. If attributeDefinition indicates it is an enumerated attribute:
    - 4a. Assert: the reflected IDL attribute is limited to only known values.
    - 4b. Assert: contentAttributeValue corresponds to a state of attributeDefinition.
    - 4c. If contentAttributeValue corresponds to a state of attributeDefinition with no associated keyword value, then return null.
    - 4d. Return the canonical keyword for the state of attributeDefinition that contentAttributeValue corresponds to.
  - 5. Return contentAttributeValue.

- [x] [The setter steps](https://html.spec.whatwg.org/#limited-to-only-known-values) — The setter steps are:
  - 1. If the given value is null, then run this's delete the content attribute.
  - 2. Otherwise, run this's set the content attribute with the given value.

- [x] [The getter steps](https://html.spec.whatwg.org/#treated-as-a-url) — The getter steps are:
  - 1. Let element be the result of running this's get the element.
  - 2. Let contentAttributeValue be the result of running this's get the content attribute.
  - 3. If the reflected IDL attribute is treated as a URL:
    - 3a. If contentAttributeValue is null, then return the empty string.
    - 3b. Let urlString be the result of encoding-parsing-and-serializing a URL given contentAttributeValue, relative to element's node document.
    - 3c. If urlString is not failure, then return urlString.
  - 4. Return contentAttributeValue, converted to a scalar value string.

- [x] [The setter steps](https://html.spec.whatwg.org/#treated-as-a-url) — The setter steps are to run this's set the content attribute with the given value.

- [x] [The getter steps](https://html.spec.whatwg.org/#treated-as-a-url) — The getter steps are:
  - 1. Let contentAttributeValue be the result of running this's get the content attribute.
  - 2. If contentAttributeValue is null, then return false.
  - 3. Return true.

- [x] [The setter steps](https://html.spec.whatwg.org/#treated-as-a-url) — The setter steps are:
  - 1. If the given value is false, then run this's delete the content attribute.
  - 2. If the given value is true, then run this's set the content attribute with the empty string.

- [x] [The getter steps](https://html.spec.whatwg.org/#default-value) — The getter steps are:
  - 1. Let contentAttributeValue be the result of running this's get the content attribute.
  - 2. If contentAttributeValue is not null:
    - 2a. Let parsedValue be the result of integer parsing contentAttributeValue if the reflected IDL attribute is not limited to only non-negative numbers; otherwise the result of non-negative integer parsing contentAttributeValue.
    - 2b. If parsedValue is not an error and is within the long range, then return parsedValue.
  - 3. If the reflected IDL attribute has a default value, then return defaultValue.
  - 4. If the reflected IDL attribute is limited to only non-negative numbers, then return −1.
  - 5. Return 0.

- [x] [The setter steps](https://html.spec.whatwg.org/#default-value) — The setter steps are:
  - 1. If the reflected IDL attribute is limited to only non-negative numbers and the given value is negative, then throw an "IndexSizeError" DOMException.
  - 2. Run this's set the content attribute with the given value converted to the shortest possible string representing the number as a valid integer.

- [x] [The getter steps](https://html.spec.whatwg.org/#clamped-to-the-range) — The getter steps are:
  - 1. Let contentAttributeValue be the result of running this's get the content attribute.
  - 2. Let minimum be 0.
  - 3. If the reflected IDL attribute is limited to only positive numbers or limited to only positive numbers with fallback, then set minimum to 1.
  - 4. If the reflected IDL attribute is clamped to the range, then set minimum to clampedMin.
  - 5. Let maximum be 2147483647 if the reflected IDL attribute is not clamped to the range; otherwise clampedMax.
  - 6. If contentAttributeValue is not null:
    - 6a. Let parsedValue be the result of non-negative integer parsing contentAttributeValue.
    - 6b. If parsedValue is not an error and is in the range minimum to maximum, inclusive, then return parsedValue.
    - 6c. If parsedValue is not an error and the reflected IDL attribute is clamped to the range:
      - 6c.i. If parsedValue is less than minimum, then return minimum.
      - 6c.ii. Return maximum.
  - 7. If the reflected IDL attribute has a default value, then return defaultValue.
  - 8. Return minimum.

- [x] [The setter steps](https://html.spec.whatwg.org/#clamped-to-the-range) — The setter steps are:
  - 1. If the reflected IDL attribute is limited to only positive numbers and the given value is 0, then throw an "IndexSizeError" DOMException.
  - 2. Let minimum be 0.
  - 3. If the reflected IDL attribute is limited to only positive numbers or limited to only positive numbers with fallback, then set minimum to 1.
  - 4. Let newValue be minimum.
  - 5. If the reflected IDL attribute has a default value, then set newValue to defaultValue.
  - 6. If the given value is in the range minimum to 2147483647, inclusive, then set newValue to it.
  - 7. Run this's set the content attribute with newValue converted to the shortest possible string representing the number as a valid non-negative integer.

- [x] [The getter steps](https://html.spec.whatwg.org/#clamped-to-the-range) — The getter steps are:
  - 1. Let contentAttributeValue be the result of running this's get the content attribute.
  - 2. If contentAttributeValue is not null:
    - 2a. Let parsedValue be the result of floating-point number parsing contentAttributeValue.
    - 2b. If parsedValue is not an error and is greater than 0, then return parsedValue.
    - 2c. If parsedValue is not an error and the reflected IDL attribute is not limited to only positive numbers, then return parsedValue.
  - 3. If the reflected IDL attribute has a default value, then return defaultValue.
  - 4. Return 0.

- [x] [The setter steps](https://html.spec.whatwg.org/#clamped-to-the-range) — The setter steps are:
  - 1. If the reflected IDL attribute is limited to only positive numbers and the given value is not greater than 0, then return.
  - 2. Run this's set the content attribute with the given value, converted to the best representation of the number as a floating-point number.

- [x] [If a reflected IDL attribute has the type DOMTokenList, then its getter steps](https://html.spec.whatwg.org/#clamped-to-the-range) — If a reflected IDL attribute has the type DOMTokenList, then its getter steps are to return a DOMTokenList object whose associated element is this and associated attribute's local name is the reflected content attribute name. Specification authors cannot reflect IDL attributes of this type on ElementInternals.

- [x] [get the attr-associated element (Element,ElementInternals)](https://html.spec.whatwg.org/#attr-associated-element) — Its reflected target reflectedTarget has a get the attr-associated element algorithm, that runs these steps:
  - 1. Let element be the result of running reflectedTarget's get the element.
  - 2. Let contentAttributeValue be the result of running reflectedTarget's get the content attribute.
  - 3. If reflectedTarget's explicitly set attr-element is not null:
    - 3a. If reflectedTarget's explicitly set attr-element is a descendant of any of element's shadow-including ancestors, then return reflectedTarget's explicitly set attr-element.
    - 3b. Return null.
  - 4. Otherwise, if contentAttributeValue is not null, return the first element candidate, in tree order, that meets the following criteria: candidate's root is the same as element's root;; candidate's ID is contentAttributeValue; and; candidate implements T. If no such element exists, then return null.
  - 5. Return null.

- [x] [The getter steps](https://html.spec.whatwg.org/#attr-associated-element) — The getter steps are to return the result of running this's get the attr-associated element.

- [x] [The setter steps](https://html.spec.whatwg.org/#attr-associated-element) — The setter steps are:
  - 1. If the given value is null:
    - 1a. Set this's explicitly set attr-element to null.
    - 1b. Run this's delete the content attribute.
    - 1c. Return.
  - 2. Run this's set the content attribute with the empty string.
  - 3. Set this's explicitly set attr-element to a weak reference to the given value.

- [x] [For element reflected targets only](https://html.spec.whatwg.org/#attr-associated-element) — For element reflected targets only: the following attribute change steps, given element, localName, oldValue, value, and namespace, are used to synchronize between the content attribute and the IDL attribute:
  - 1. If localName is not attr or namespace is not null, then return.
  - 2. Set element's explicitly set attr-element to null.

- [x] [get the attr-associated elements (Element,ElementInternals)](https://html.spec.whatwg.org/#attr-associated-elements) — Its reflected target reflectedTarget has a get the attr-associated elements algorithm, which runs these steps:
  - 1. Let elements be an empty list.
  - 2. Let element be the result of running reflectedTarget's get the element.
  - 3. If reflectedTarget's explicitly set attr-elements is not null:
    - 3a. For each attrElement in reflectedTarget's explicitly set attr-elements:
      - 3a.i. If attrElement is not a descendant of any of element's shadow-including ancestors, then continue.
      - 3a.ii. Append attrElement to elements.
  - 4. Otherwise:
    - 4a. Let contentAttributeValue be the result of running reflectedTarget's get the content attribute.
    - 4b. If contentAttributeValue is null, then return null.
    - 4c. Let tokens be contentAttributeValue, split on ASCII whitespace.
    - 4d. For each id of tokens:
      - 4d.i. Let candidate be the first element, in tree order, that meets the following criteria: candidate's root is the same as element's root;; candidate's ID is id; and; candidate implements T. If no such element exists, then continue.
      - 4d.ii. Append candidate to elements.
  - 5. Return elements.
  > n/a: only ARIA reflects `FrozenArray<T>?` attributes, and accessibility isn't planned.

- [x] [The getter steps](https://html.spec.whatwg.org/#attr-associated-elements) — The getter steps are:
  - 1. Let elements be the result of running this's get the attr-associated elements.
  - 2. If the contents of elements is equal to the contents of this's cached attr-associated elements, then return this's cached attr-associated elements object.
  - 3. Let elementsAsFrozenArray be elements, converted to a FrozenArray<T>?.
  - 4. Set this's cached attr-associated elements to elements.
  - 5. Set this's cached attr-associated elements object to elementsAsFrozenArray.
  - 6. Return elementsAsFrozenArray.
  > n/a: only ARIA reflects `FrozenArray<T>?` attributes, and accessibility isn't planned.

- [x] [The setter steps](https://html.spec.whatwg.org/#attr-associated-elements) — The setter steps are:
  - 1. If the given value is null:
    - 1a. Set this's explicitly set attr-elements to null.
    - 1b. Run this's delete the content attribute.
    - 1c. Return.
  - 2. Run this's set the content attribute with the empty string.
  - 3. Let elements be an empty list.
  - 4. For each element in the given value:
    - 4a. Append a weak reference to element to elements.
  - 5. Set this's explicitly set attr-elements to elements.
  > n/a: only ARIA reflects `FrozenArray<T>?` attributes, and accessibility isn't planned.

- [x] [For element reflected targets only](https://html.spec.whatwg.org/#attr-associated-elements) — For element reflected targets only: the following attribute change steps, given element, localName, oldValue, value, and namespace, are used to synchronize between the content attribute and the IDL attribute:
  - 1. If localName is not attr or namespace is not null, then return.
  - 2. Set element's explicitly set attr-elements to null.
  > n/a: only ARIA reflects `FrozenArray<T>?` attributes, and accessibility isn't planned.

#### [2.6.2 Using reflect via IDL extended attributes](https://html.spec.whatwg.org/#using-reflect-via-idl-extended-attributes)

Status: reviewed (2026-11-01)
> `Reflect`, `ReflectSetter`, `ReflectURL`, `ReflectNonNegative`, `ReflectPositive`, `ReflectPositiveWithFallback`, and the `ReflectDefault`/`ReflectRange` template arguments.
<!-- definitions: xattr-reflect xattr-reflectsetter xattr-reflecturl xattr-reflectnonnegative xattr-reflectpositive xattr-reflectpositivewithfallback xattr-reflectrange xattr-reflectdefault -->

#### [2.6.3 Using reflect in specifications](https://html.spec.whatwg.org/#using-reflect-in-specifications)

Status: reviewed (2026-11-01)
> Guidance for specifications: the reflected target is the element (`ReflectTarget`).

#### [2.6.4 Collections](https://html.spec.whatwg.org/#collections)

Status: reviewed (2026-11-01)
> Headings only.

##### [2.6.4.1 The HTMLAllCollection interface](https://html.spec.whatwg.org/#the-htmlallcollection-interface)

Status: n/a (document.all is a legacy feature only useful to scripts)

- [x] [HTMLAllCollection.length](https://html.spec.whatwg.org/#dom-htmlallcollection-length) — The length getter steps are to return the number of nodes represented by the collection.

- [x] [HTMLAllCollection.namedItem(name)](https://html.spec.whatwg.org/#dom-htmlallcollection-nameditem) — The namedItem(name) method steps are to return the result of getting the "all"-named element(s) from this given name.

- [x] [HTMLAllCollection.item(nameOrIndex)](https://html.spec.whatwg.org/#dom-htmlallcollection-item) — The item(nameOrIndex) method steps are:
  - 1. If nameOrIndex was not provided, return null.
  - 2. Return the result of getting the "all"-indexed or named element(s) from this, given nameOrIndex.

- [x] ["all"-named elements](https://html.spec.whatwg.org/#all-named-elements) — The following elements are "all"-named elements: a, button, embed, form, frame, frameset, iframe, img, input, map, meta, object, select, and textarea

- [x] [get the "all"-indexed element](https://html.spec.whatwg.org/#concept-get-all-indexed) — To get the "all"-indexed element from an HTMLAllCollection collection given an index index, return the indexth element in collection, or null if there is no such indexth element.

- [x] [get the "all"-named element(s)](https://html.spec.whatwg.org/#concept-get-all-named) — To get the "all"-named element(s) from an HTMLAllCollection collection given a name name, perform the following steps:
  - 1. If name is the empty string, return null.
  - 2. Let subCollection be an HTMLCollection object rooted at the same Document as collection, whose filter matches only elements that are either: "all"-named elements with a name attribute equal to name, or; elements with an ID equal to name.
  - 3. If there is exactly one element in subCollection, then return that element.
  - 4. Otherwise, if subCollection is empty, return null.
  - 5. Otherwise, return subCollection.

- [x] [get the "all"-indexed or named element(s)](https://html.spec.whatwg.org/#concept-get-all-indexed-or-named) — To get the "all"-indexed or named element(s) from an HTMLAllCollection collection given nameOrIndex:
  - 1. If nameOrIndex, converted to a JavaScript String value, is an array index property name, return the result of getting the "all"-indexed element from collection given the number represented by nameOrIndex.
  - 2. Return the result of getting the "all"-named element(s) from collection given nameOrIndex.

###### [2.6.4.1.1 [[Call]] ( thisArgument, argumentsList )](https://html.spec.whatwg.org/#HTMLAllCollection-call)

Status: n/a (document.all is a legacy feature only useful to scripts)

##### [2.6.4.2 The HTMLFormControlsCollection interface](https://html.spec.whatwg.org/#the-htmlformcontrolscollection-interface)

Status: reviewed (2026-11-01)
> `HTMLFormControlsCollection::NamedItemOrItems` (the form keeps a `RadioNodeList` per name) and `RadioNodeList::Value`/`SetValue`.
> Fixed: `RadioNodeList`'s value skipped radio buttons with an empty `value` (DIFFERENCES.md).

- [x] [HTMLFormControlsCollection.namedItem(name)](https://html.spec.whatwg.org/#dom-htmlformcontrolscollection-nameditem) — The namedItem(name) method must act according to the following algorithm:
  - 1. If name is the empty string, return null and stop the algorithm.
  - 2. If, at the time the method is called, there is exactly one node in the collection that has either an id attribute or a name attribute equal to name, then return that node and stop the algorithm.
  - 3. Otherwise, if there are no nodes in the collection that have either an id attribute or a name attribute equal to name, then return null and stop the algorithm.
  - 4. Otherwise, create a new RadioNodeList object representing a live view of the HTMLFormControlsCollection object, further filtered so that the only nodes in the RadioNodeList object are those that have either an id attribute or a name attribute equal to name. The nodes in the RadioNodeList object must be sorted in tree order.
  - 5. Return that RadioNodeList object.

- [x] [RadioNodeList.value](https://html.spec.whatwg.org/#dom-radionodelist-value) — The value IDL attribute on the RadioNodeList object, on getting, must return the value returned by running the following steps:
  - 1. Let element be the first element in tree order represented by the RadioNodeList object that is an input element whose type attribute is in the Radio Button state and whose checkedness is true. Otherwise, let it be null.
  - 2. If element is null, return the empty string.
  - 3. If element is an element with no value attribute, return the string "on".
  - 4. Otherwise, return the value of element's value attribute.

- [x] [On setting, the value IDL attribute must run the following steps](https://html.spec.whatwg.org/#dom-radionodelist-value) — On setting, the value IDL attribute must run the following steps:
  - 1. If the new value is the string "on": let element be the first element in tree order represented by the RadioNodeList object that is an input element whose type attribute is in the Radio Button state and whose value content attribute is either absent, or present and equal to the new value, if any. If no such element exists, then instead let element be null. Otherwise: let element be the first elem…
  - 2. If element is not null, then set its checkedness to true.

##### [2.6.4.3 The HTMLOptionsCollection interface](https://html.spec.whatwg.org/#the-htmloptionscollection-interface)

Status: reviewed (2026-11-01)
> `HTMLOptionsCollection` delegates to `HTMLSelectElement`'s `Length`, `Add`, `Remove`, `SetItem`, `AppendNewOptionElements` and `SelectedIndex`.
> Fixed: `add()` didn't insert into the parent of an element it goes before, check that element is in the select, or ignore an option added before itself; the indexed setter re-inserted rather than replacing; growing the length appended options one at a time; and the length limit was 10,000 rather than 100,000 (DIFFERENCES.md).

- [x] [HTMLOptionsCollection.length](https://html.spec.whatwg.org/#dom-htmloptionscollection-length) — The length getter steps are to return the number of nodes represented by the collection.

- [x] [The length setter steps](https://html.spec.whatwg.org/#dom-htmloptionscollection-length) — The length setter steps are:
  - 1. Let current be the number of nodes represented by the collection.
  - 2. If the given value is greater than current:
    - 2a. If the given value is greater than 100,000, then return.
    - 2b. Let n be value − current.
    - 2c. Append new option elements to the select element on which this is rooted given n.
  - 3. If the given value is less than current:
    - 3a. Let n be current − value.
    - 3b. Remove the last n nodes in the collection from their parent nodes.

- [x] [append new option elements](https://html.spec.whatwg.org/#append-new-option-elements) — To append new option elements to a select element select given a non-negative integer count:
  - 1. Let fragment be the result of creating a document fragment given select's node document.
  - 2. Append count new option elements to fragment.
  - 3. Append fragment to select.

- [x] [To set the value of a new indexed property or set the value of an existing inde…](https://html.spec.whatwg.org/#append-new-option-elements) — To set the value of a new indexed property or set the value of an existing indexed property for an HTMLOptionsCollection collection, given a property index index and a new value value:
  - 1. If value is null, then remove an option from collection given index and return.
  - 2. Let length be the number of nodes represented by collection.
  - 3. Let delta be index − length.
  - 4. If delta is greater than 0, then append new option elements to the select element on which collection is rooted given delta.
  - 5. If delta is greater than or equal to 0, then append value to the select element on which collection is rooted. Otherwise, replace the indexth element in collection by value.

- [x] [HTMLOptionsCollection.add(element, before)](https://html.spec.whatwg.org/#dom-htmloptionscollection-add) — The add(element, before) method steps are:
  - 1. If element is an ancestor of the select element on which this is rooted, then throw a "HierarchyRequestError" DOMException.
  - 2. If before is an element, but that element isn't a descendant of the select element on which this is rooted, then throw a "NotFoundError" DOMException.
  - 3. If element and before are the same element, then return.
  - 4. Let reference be null.
  - 5. If before is a node, then set reference to before. Otherwise, if before is an integer and there is a beforeth node in this, then set reference to that node.
  - 6. Let parent be reference's parent node if reference is not null; otherwise the select element on which this is rooted.
  - 7. Pre-insert element into parent node before reference.

- [x] [remove an option](https://html.spec.whatwg.org/#htmloptionscollection-remove-an-option) — To remove an option from an HTMLOptionsCollection collection given an integer index:
  - 1. If the number of nodes represented by collection is 0, then return.
  - 2. If index is not a number greater than or equal to 0 and less than the number of nodes represented by collection, then return.
  - 3. Let element be the indexth element in collection.
  - 4. Remove element from its parent node.

- [x] [HTMLOptionsCollection.remove(index)](https://html.spec.whatwg.org/#dom-htmloptionscollection-remove) — The remove(index) method steps are to remove an option from this given index.

- [x] [HTMLOptionsCollection.selectedIndex](https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex) — The selectedIndex getter steps are to return the selected index of the select element on which this is rooted.

- [x] [The selectedIndex setter steps](https://html.spec.whatwg.org/#dom-htmloptionscollection-selectedindex) — The selectedIndex setter steps are to set the selected index of the select element on which this is rooted to the given value.

#### [2.6.5 The DOMStringList interface](https://html.spec.whatwg.org/#the-domstringlist-interface)

Status: n/a (a list of strings is a `List<DOMString>`, as `Location::AncestorOrigins` returns)

- [x] [DOMStringList.length](https://html.spec.whatwg.org/#dom-domstringlist-length) — The length getter steps are to return this's associated list's size.

- [x] [DOMStringList.item(index)](https://html.spec.whatwg.org/#dom-domstringlist-item) — The item(index) method steps are to return the indexth item in this's associated list, or null if index plus one is greater than this's associated list's size.

- [x] [DOMStringList.contains(string)](https://html.spec.whatwg.org/#dom-domstringlist-contains) — The contains(string) method steps are to return true if this's associated list contains string, and false otherwise.
