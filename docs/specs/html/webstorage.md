# HTML review checklist: 12 Web storage

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 12 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html webstorage` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [12 Web storage](https://html.spec.whatwg.org/#webstorage)

Status: unreviewed

### [12.1 Introduction](https://html.spec.whatwg.org/#introduction-16)

Status: unreviewed

### [12.2 The API](https://html.spec.whatwg.org/#storage)

Status: unreviewed

#### [12.2.1 The Storage interface](https://html.spec.whatwg.org/#the-storage-interface)

Status: unreviewed
<!-- definitions: concept-storage-map concept-storage-type -->

- [ ] [reorder](https://html.spec.whatwg.org/#concept-storage-reorder) — To reorder a Storage object storage, reorder storage's map's entries in an implementation-defined manner.

- [ ] [broadcast](https://html.spec.whatwg.org/#concept-storage-broadcast) — To broadcast a Storage object storage, given a key, oldValue, and newValue, run these steps:
  - 1. Let thisDocument be storage's relevant global object's associated Document.
  - 2. Let url be the serialization of thisDocument's URL.
  - 3. Let remoteStorages be all Storage objects excluding storage whose: type is storage's type; relevant settings object's origin is same origin with storage's relevant settings object's origin and, if type is "session", whose relevant settings object's associated Document's node navigable's traversable navigable is thisDocument's node navigable's traversable navigable.
  - 4. For each remoteStorage of remoteStorages: queue a global task on the DOM manipulation task source given remoteStorage's relevant global object to fire an event named storage at remoteStorage's relevant global object, using StorageEvent, with key initialized to key, oldValue initialized to oldValue, newValue initialized to newValue, url initialized to url, and storageArea initialized to remoteStor…

- [ ] [Storage.length](https://html.spec.whatwg.org/#dom-storage-length) — The length getter steps are to return this's map's size.

- [ ] [Storage.key(index)](https://html.spec.whatwg.org/#dom-storage-key) — The key(index) method steps are:
  - 1. If index is greater than or equal to this's map's size, then return null.
  - 2. Let keys be the result of running get the keys on this's map.
  - 3. Return keys[index].

- [ ] [Storage.getItem(key)](https://html.spec.whatwg.org/#dom-storage-getitem) — The getItem(key) method steps are:
  - 1. If this's map[key] does not exist, then return null.
  - 2. Return this's map[key].

- [ ] [Storage.setItem(key, value)](https://html.spec.whatwg.org/#dom-storage-setitem) — The setItem(key, value) method steps are:
  - 1. Let oldValue be null.
  - 2. Let reorder be true.
  - 3. If this's map[key] exists:
    - 3a. Set oldValue to this's map[key].
    - 3b. If oldValue is value, then return.
    - 3c. Set reorder to false.
  - 4. If value cannot be stored, then throw a QuotaExceededError.
  - 5. Set this's map[key] to value.
  - 6. If reorder is true, then reorder this.
  - 7. Broadcast this with key, oldValue, and value.

- [ ] [Storage.removeItem(key)](https://html.spec.whatwg.org/#dom-storage-removeitem) — The removeItem(key) method steps are:
  - 1. If this's map[key] does not exist, then return.
  - 2. Set oldValue to this's map[key].
  - 3. Remove this's map[key].
  - 4. Reorder this.
  - 5. Broadcast this with key, oldValue, and null.

- [ ] [Storage.clear()](https://html.spec.whatwg.org/#dom-storage-clear) — The clear() method steps are:
  - 1. If this's map is empty, then return.
  - 2. Clear this's map.
  - 3. Broadcast this with null, null, and null.

#### [12.2.2 The sessionStorage getter](https://html.spec.whatwg.org/#the-sessionstorage-attribute)

Status: unreviewed
<!-- definitions: session-storage-holder -->

- [ ] [WindowSessionStorage.sessionStorage](https://html.spec.whatwg.org/#dom-sessionstorage) — The sessionStorage getter steps are:
  - 1. If this's associated Document's session storage holder is non-null, then return this's associated Document's session storage holder.
  - 2. Let map be the result of running obtain a session storage bottle map with this's relevant settings object and "sessionStorage".
  - 3. If map is failure, then throw a "SecurityError" DOMException.
  - 4. Let storage be a new Storage object whose map is map.
  - 5. Set this's associated Document's session storage holder to storage.
  - 6. Return storage.

#### [12.2.3 The localStorage getter](https://html.spec.whatwg.org/#the-localstorage-attribute)

Status: unreviewed
<!-- definitions: local-storage-holder -->

- [ ] [WindowLocalStorage.localStorage](https://html.spec.whatwg.org/#dom-localstorage) — The localStorage getter steps are:
  - 1. If this's associated Document's local storage holder is non-null, then return this's associated Document's local storage holder.
  - 2. Let map be the result of running obtain a local storage bottle map with this's relevant settings object and "localStorage".
  - 3. If map is failure, then throw a "SecurityError" DOMException.
  - 4. Let storage be a new Storage object whose map is map.
  - 5. Set this's associated Document's local storage holder to storage.
  - 6. Return storage.

#### [12.2.4 The StorageEvent interface](https://html.spec.whatwg.org/#the-storageevent-interface)

Status: unreviewed
<!-- definitions: dom-storageevent-oldvalue dom-storageevent-newvalue dom-storageevent-url dom-storageevent-storagearea -->

- [ ] [StorageEvent.key](https://html.spec.whatwg.org/#dom-storageevent-key) — The key, oldValue, newValue, url, and storageArea attributes must return the values they were initialized to.

- [ ] [StorageEvent.initStorageEvent(type, bubbles, cancelable, key, oldValue, newValue, url, storageArea)](https://html.spec.whatwg.org/#dom-storageevent-initstorageevent) — The initStorageEvent(type, bubbles, cancelable, key, oldValue, newValue, url, storageArea) method must initialize the event in a manner analogous to the similarly-named initEvent() method. [DOM]

### [12.3 Privacy](https://html.spec.whatwg.org/#privacy)

Status: unreviewed

#### [12.3.1 User tracking](https://html.spec.whatwg.org/#user-tracking)

Status: unreviewed

#### [12.3.2 Sensitivity of data](https://html.spec.whatwg.org/#sensitivity-of-data)

Status: unreviewed

### [12.4 Security](https://html.spec.whatwg.org/#security-storage)

Status: unreviewed

#### [12.4.1 DNS spoofing attacks](https://html.spec.whatwg.org/#dns-spoofing-attacks)

Status: unreviewed

#### [12.4.2 Cross-directory attacks](https://html.spec.whatwg.org/#cross-directory-attacks)

Status: unreviewed

#### [12.4.3 Implementation risks](https://html.spec.whatwg.org/#implementation-risks)

Status: unreviewed
