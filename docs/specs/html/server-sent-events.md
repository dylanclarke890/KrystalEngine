# HTML review checklist: 9.2 Server-sent events

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 10 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html server-sent-events` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [9.2 Server-sent events](https://html.spec.whatwg.org/#server-sent-events)

Status: n/a (later: network loader)

#### [9.2.1 Introduction](https://html.spec.whatwg.org/#server-sent-events-intro)

Status: n/a (later: network loader)

#### [9.2.2 The EventSource interface](https://html.spec.whatwg.org/#the-eventsource-interface)

Status: n/a (later: network loader)
<!-- definitions: concept-eventsource-url concept-event-stream-request concept-event-stream-reconnection-time concept-event-stream-last-event-id dom-eventsource-readystate dom-eventsource-connecting dom-eventsource-open dom-eventsource-closed -->

- [x] [EventSource.EventSource(url, eventSourceInitDict)](https://html.spec.whatwg.org/#dom-eventsource) — The EventSource(url, eventSourceInitDict) constructor, when invoked, must run these steps:
  - 1. Let ev be a new EventSource object.
  - 2. Let settings be ev's relevant settings object.
  - 3. Let urlRecord be the result of encoding-parsing a URL given url, relative to settings.
  - 4. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 5. Set ev's url to urlRecord.
  - 6. Let corsAttributeState be Anonymous.
  - 7. If the value of eventSourceInitDict's withCredentials member is true, then set corsAttributeState to Use Credentials and set ev's withCredentials attribute to true.
  - 8. Let request be the result of creating a potential-CORS request given urlRecord, the empty string, and corsAttributeState.
  - 9. Set request's client to settings.
  - 10. User agents may set (`Accept`, `text/event-stream`) in request's header list.
  - 11. Set request's cache mode to "no-store".
  - 12. Set request's initiator type to "other".
  - 13. Set ev's request to request.
  - 14. Let processEventSourceEndOfBody given response res be the following step: if res is not a network error, then reestablish the connection.
  - 15. Fetch request, with processResponseEndOfBody set to processEventSourceEndOfBody and processResponse set to the following steps given response res:
    - 15a. If res is an aborted network error, then fail the connection.
    - 15b. Otherwise, if res is a network error, then reestablish the connection, unless the user agent knows that to be futile, in which case the user agent may fail the connection.
    - 15c. Otherwise, if res's status is not 200, or if res's `Content-Type` is not `text/event-stream`, then fail the connection.
    - 15d. Otherwise, announce the connection and interpret res's body line by line.
  - 16. Return ev.

- [x] [EventSource.url](https://html.spec.whatwg.org/#dom-eventsource-url) — The url attribute's getter must return the serialization of this EventSource object's url.

- [x] [EventSource.withCredentials](https://html.spec.whatwg.org/#dom-eventsource-withcredentials) — The withCredentials attribute must return the value to which it was last initialized. When the object is created, it must be initialized to false.

- [x] [EventSource.close()](https://html.spec.whatwg.org/#dom-eventsource-close) — The close() method must abort any instances of the fetch algorithm started for this EventSource object, and must set the readyState attribute to CLOSED.

#### [9.2.3 Processing model](https://html.spec.whatwg.org/#sse-processing-model)

Status: n/a (later: network loader)
<!-- definitions: remote-event-task-source -->

- [x] [announce the connection](https://html.spec.whatwg.org/#announce-the-connection) — When a user agent is to announce the connection, the user agent must queue a task which, if the readyState attribute is set to a value other than CLOSED, sets the readyState attribute to OPEN and fires an event named open at the EventSource object.

- [x] [reestablish the connection](https://html.spec.whatwg.org/#reestablish-the-connection) — When a user agent is to reestablish the connection, the user agent must run the following steps. These steps are run in parallel, not as part of a task. (The tasks that it queues, of course, are run like normal tasks and not themselves in parallel.)
  - 1. Queue a task to run the following steps:
    - 1a. If the readyState attribute is set to CLOSED, abort the task.
    - 1b. Set the readyState attribute to CONNECTING.
    - 1c. Fire an event named error at the EventSource object.
  - 2. Wait a delay equal to the reconnection time of the event source.
  - 3. Optionally, wait some more. In particular, if the previous attempt failed, then user agents might introduce an exponential backoff delay to avoid overloading a potentially already overloaded server. Alternatively, if the operating system has reported that there is no network connectivity, user agents might wait for the operating system to announce that the network connection has returned before r…
  - 4. Wait until the aforementioned task has run, if it has not yet run.
  - 5. Queue a task to run the following steps:
    - 5a. If the EventSource object's readyState attribute is not set to CONNECTING, then return.
    - 5b. Let request be the EventSource object's request.
    - 5c. If the EventSource object's last event ID string is not the empty string:
      - 5c.i. Let lastEventIDValue be the EventSource object's last event ID string, encoded as UTF-8.
      - 5c.ii. Set (`Last-Event-ID`, lastEventIDValue) in request's header list.
    - 5d. Fetch request and process the response obtained in this fashion, if any, as described earlier in this section.

- [x] [fail the connection](https://html.spec.whatwg.org/#fail-the-connection) — When a user agent is to fail the connection, the user agent must queue a task which, if the readyState attribute is set to a value other than CLOSED, sets the readyState attribute to CLOSED and fires an event named error at the EventSource object. Once the user agent has failed the connection, it does not attempt to reconnect.

#### [9.2.4 The `Last-Event-ID` header](https://html.spec.whatwg.org/#the-last-event-id-header)

Status: n/a (later: network loader)
<!-- definitions: last-event-id -->

#### [9.2.5 Parsing an event stream](https://html.spec.whatwg.org/#parsing-an-event-stream)

Status: n/a (later: network loader)

#### [9.2.6 Interpreting an event stream](https://html.spec.whatwg.org/#event-stream-interpretation)

Status: n/a (later: network loader)

- [x] [dispatch the event](https://html.spec.whatwg.org/#dispatchMessage) — When the user agent is required to dispatch the event, the user agent must process the data buffer, the event type buffer, and the last event ID buffer using steps appropriate for the user agent.

- [x] [For web browsers, the appropriate steps to dispatch the event](https://html.spec.whatwg.org/#dispatchMessage) — For web browsers, the appropriate steps to dispatch the event are as follows:
  - 1. Set the last event ID string of the event source to the value of the last event ID buffer. The buffer does not get reset, so the last event ID string of the event source remains set to this value until the next time it is set by the server.
  - 2. If the data buffer is an empty string, set the data buffer and the event type buffer to the empty string and return.
  - 3. If the data buffer's last character is a U+000A LINE FEED (LF) character, then remove the last character from the data buffer.
  - 4. Let event be the result of creating an event using MessageEvent, in the relevant realm of the EventSource object.
  - 5. Initialize event's type attribute to "message", its data attribute to data, its origin to the origin of the event stream's final URL (i.e., the URL after redirects), and its lastEventId attribute to the last event ID string of the event source.
  - 6. If the event type buffer has a value other than the empty string, change the type of the newly created event to equal the value of the event type buffer.
  - 7. Set the data buffer and the event type buffer to the empty string.
  - 8. Queue a task which, if the readyState attribute is set to a value other than CLOSED, dispatches the newly created event at the EventSource object.

#### [9.2.7 Authoring notes](https://html.spec.whatwg.org/#authoring-notes)

Status: n/a (later: network loader)

#### [9.2.8 Connectionless push and other features](https://html.spec.whatwg.org/#eventsource-push)

Status: n/a (later: network loader)

#### [9.2.9 Garbage collection](https://html.spec.whatwg.org/#garbage-collection)

Status: n/a (later: network loader)

- [x] [forcibly close](https://html.spec.whatwg.org/#concept-eventsource-forcibly-close) — If a user agent is to forcibly close an EventSource object (this happens when a Document object goes away permanently), the user agent must abort any instances of the fetch algorithm started for this EventSource object, and must set the readyState attribute to CLOSED.

#### [9.2.10 Implementation advice](https://html.spec.whatwg.org/#implementation-advice)

Status: n/a (later: network loader)
