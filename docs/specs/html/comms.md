# HTML review checklist: 9 Communication

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 8 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html comms` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [9 Communication](https://html.spec.whatwg.org/#comms)

Status: unreviewed

### [9.1 The MessageEvent interface](https://html.spec.whatwg.org/#the-messageevent-interface)

Status: unreviewed
<!-- definitions: concept-messageevent-origin -->

- [ ] [MessageEvent.data](https://html.spec.whatwg.org/#dom-messageevent-data) — The data attribute must return the value it was initialized to. It represents the message being sent.

- [ ] [MessageEvent.origin](https://html.spec.whatwg.org/#dom-messageevent-origin) — The origin attribute represents, in server-sent events and cross-document messaging, the origin of the document that sent the message (typically the scheme, hostname, and port of the document, but not its path or fragment).

- [ ] [The origin getter steps](https://html.spec.whatwg.org/#dom-messageevent-origin) — The origin getter steps are:
  - 1. If this's origin is an origin, then return the serialization of this's origin.
  - 2. If this's origin is null, then return the empty string.
  - 3. Return this's origin.

- [ ] [Objects implementing the MessageEvent interface's extract an origin steps](https://html.spec.whatwg.org/#dom-messageevent-origin) — Objects implementing the MessageEvent interface's extract an origin steps are to return this's origin if it is an origin; otherwise null.

- [ ] [MessageEvent.lastEventId](https://html.spec.whatwg.org/#dom-messageevent-lasteventid) — The lastEventId attribute must return the value it was initialized to. It represents, in server-sent events, the last event ID string of the event source.

- [ ] [MessageEvent.source](https://html.spec.whatwg.org/#dom-messageevent-source) — The source attribute must return the value it was initialized to. It represents, in cross-document messaging, the WindowProxy of the browsing context of the Window object from which the message came; and in the connect events used by shared workers, the newly connecting MessagePort.

- [ ] [MessageEvent.ports](https://html.spec.whatwg.org/#dom-messageevent-ports) — The ports attribute must return the value it was initialized to. It represents, in cross-document messaging and channel messaging, the MessagePort array being sent.

- [ ] [MessageEvent.initMessageEvent(type, bubbles, cancelable, data, origin, lastEventId, source, ports)](https://html.spec.whatwg.org/#dom-messageevent-initmessageevent) — The initMessageEvent(type, bubbles, cancelable, data, origin, lastEventId, source, ports) method must initialize the event in a manner analogous to the similarly-named initEvent() method. [DOM]
