# HTML review checklist: 9.3 Cross-document messaging

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 21 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html web-messaging` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [9.3 Cross-document messaging](https://html.spec.whatwg.org/#web-messaging)

Status: unreviewed

#### [9.3.1 Introduction](https://html.spec.whatwg.org/#introduction-13)

Status: unreviewed

#### [9.3.2 Security](https://html.spec.whatwg.org/#security-postmsg)

Status: unreviewed

##### [9.3.2.1 Authors](https://html.spec.whatwg.org/#authors)

Status: unreviewed

##### [9.3.2.2 User agents](https://html.spec.whatwg.org/#user-agents)

Status: unreviewed

#### [9.3.3 Posting messages](https://html.spec.whatwg.org/#posting-messages)

Status: unreviewed
<!-- definitions: posted-message-task-source -->

- [ ] [window post message steps](https://html.spec.whatwg.org/#window-post-message-steps) — The window post message steps, given a targetWindow, message, and options, are as follows:
  - 1. Let targetRealm be targetWindow's realm.
  - 2. Let incumbentSettings be the incumbent settings object.
  - 3. Let targetOrigin be options["targetOrigin"].
  - 4. If targetOrigin is a single U+002F SOLIDUS character (/), then set targetOrigin to incumbentSettings's origin.
  - 5. Otherwise, if targetOrigin is not a single U+002A ASTERISK character (*):
    - 5a. Let parsedURL be the result of running the URL parser on targetOrigin.
    - 5b. If parsedURL is failure, then throw a "SyntaxError" DOMException.
    - 5c. Set targetOrigin to parsedURL's origin.
  - 6. Let transfer be options["transfer"].
  - 7. Let serializeWithTransferResult be StructuredSerializeWithTransfer(message, transfer). Rethrow any exceptions.
  - 8. Queue a global task on the posted message task source given targetWindow to run the following steps:
    - 8a. If the targetOrigin argument is not a single literal U+002A ASTERISK character (*) and targetWindow's associated Document's origin is not same origin with targetOrigin, then return.
    - 8b. Let origin be incumbentSettings's origin.
    - 8c. Let source be the WindowProxy object corresponding to incumbentSettings's global object (a Window object).
    - 8d. Let deserializeRecord be StructuredDeserializeWithTransfer(serializeWithTransferResult, targetRealm). If this throws an exception, catch it, fire an event named messageerror at targetWindow, using MessageEvent, with its origin initialized to origin and the source attribute initialized to source, and then return.
    - 8e. Let messageClone be deserializeRecord.[[Deserialized]].
    - 8f. Let newPorts be a new frozen array consisting of all MessagePort objects in deserializeRecord.[[TransferredValues]], if any, maintaining their relative order.
    - 8g. Fire an event named message at targetWindow, using MessageEvent, with its origin initialized to origin, the source attribute initialized to source, the data attribute initialized to messageClone, and the ports attribute initialized to newPorts.

- [ ] [Window.postMessage(message, options)](https://html.spec.whatwg.org/#dom-window-postmessage-options) — The Window interface's postMessage(message, options) method steps are to run the window post message steps given this, message, and options.

- [ ] [Window.postMessage(message, targetOrigin, transfer)](https://html.spec.whatwg.org/#dom-window-postmessage) — The Window interface's postMessage(message, targetOrigin, transfer) method steps are to run the window post message steps given this, message, and «[ "targetOrigin" → targetOrigin, "transfer" → transfer ]».

### [9.4 Channel messaging](https://html.spec.whatwg.org/#channel-messaging)

Status: n/a (not planned: no message channels or broadcast channels)

#### [9.4.1 Introduction](https://html.spec.whatwg.org/#introduction-14)

Status: n/a (not planned: no message channels or broadcast channels)

##### [9.4.1.1 Examples](https://html.spec.whatwg.org/#examples-5)

Status: n/a (not planned: no message channels or broadcast channels)

##### [9.4.1.2 Ports as the basis of an object-capability model on the web](https://html.spec.whatwg.org/#ports-as-the-basis-of-an-object-capability-model-on-the-web)

Status: n/a (not planned: no message channels or broadcast channels)

##### [9.4.1.3 Ports as the basis of abstracting out service implementations](https://html.spec.whatwg.org/#ports-as-the-basis-of-abstracting-out-service-implementations)

Status: n/a (not planned: no message channels or broadcast channels)

#### [9.4.2 Message channels](https://html.spec.whatwg.org/#message-channels)

Status: n/a (not planned: no message channels or broadcast channels)
<!-- definitions: port-1 port-2 -->

- [x] [new MessageChannel()](https://html.spec.whatwg.org/#dom-messagechannel) — The new MessageChannel() constructor steps are:
  - 1. Set this's port 1 to a new MessagePort in this's relevant realm.
  - 2. Set this's port 2 to a new MessagePort in this's relevant realm.
  - 3. Entangle this's port 1 and this's port 2.

- [x] [MessageChannel.port1](https://html.spec.whatwg.org/#dom-messagechannel-port1) — The port1 getter steps are to return this's port 1.

- [x] [MessageChannel.port2](https://html.spec.whatwg.org/#dom-messagechannel-port2) — The port2 getter steps are to return this's port 2.

#### [9.4.3 The MessageEventTarget mixin](https://html.spec.whatwg.org/#the-messageeventtarget-mixin)

Status: n/a (not planned: no message channels or broadcast channels)

#### [9.4.4 Message ports](https://html.spec.whatwg.org/#message-ports)

Status: n/a (not planned: no message channels or broadcast channels)
<!-- definitions: message-event-target port-message-queue has-been-shipped -->

- [x] [unshipped port message queue](https://html.spec.whatwg.org/#unshipped-port-message-queue) — Each event loop has a task source called the unshipped port message queue. This is a virtual task source: it must act as if it contained the tasks of each port message queue of each MessagePort whose has been shipped flag is false, whose port message queue is enabled, and whose relevant agent's event loop is that event loop, in the order in which they were added to their respective task source. When a task would be removed from the unshipped port message queue, it must instead be removed from its port message queue.

- [x] [entangle](https://html.spec.whatwg.org/#entangle) — When the user agent is to entangle two MessagePort objects, it must run the following steps:
  - 1. If one of the ports is already entangled, then disentangle it and the port that it was entangled with.
  - 2. Associate the two ports to be entangled, so that they form the two parts of a new channel. (There is no MessageChannel object that represents this channel.) Two ports A and B that have gone through this step are now said to be entangled; one is entangled to the other, and vice versa.

- [x] [disentangle](https://html.spec.whatwg.org/#disentangle) — The disentangle steps, given a MessagePort initiatorPort which initiates disentangling, are as follows:
  - 1. Let otherPort be the MessagePort which initiatorPort was entangled with.
  - 2. Assert: otherPort exists.
  - 3. Disentangle initiatorPort and otherPort, so that they are no longer entangled or associated with each other.

- [x] [MessagePort objects are transferable objects. Their transfer steps, given value…](https://html.spec.whatwg.org/#disentangle) — MessagePort objects are transferable objects. Their transfer steps, given value and dataHolder, are:
  - 1. Set value's has been shipped flag to true.
  - 2. Set dataHolder.[[PortMessageQueue]] to value's port message queue.
  - 3. If value is entangled with another port remotePort:
    - 3a. Set remotePort's has been shipped flag to true.
    - 3b. Set dataHolder.[[RemotePort]] to remotePort.
  - 4. Otherwise, set dataHolder.[[RemotePort]] to null.

- [x] [Their transfer-receiving steps, given dataHolder and value,](https://html.spec.whatwg.org/#disentangle) — Their transfer-receiving steps, given dataHolder and value, are:
  - 1. Set value's has been shipped flag to true.
  - 2. Move all the tasks that are to fire message events in dataHolder.[[PortMessageQueue]] to the port message queue of value, if any, leaving value's port message queue in its initial disabled state, and, if value's relevant global object is a Window, associating the moved tasks with value's relevant global object's associated Document.
  - 3. If dataHolder.[[RemotePort]] is not null, then entangle dataHolder.[[RemotePort]] and value. (This will disentangle dataHolder.[[RemotePort]] from the original port that was transferred.)

- [x] [message port post message steps](https://html.spec.whatwg.org/#message-port-post-message-steps) — The message port post message steps, given sourcePort, targetPort, message, and options are as follows:
  - 1. Let transfer be options["transfer"].
  - 2. If transfer contains sourcePort, then throw a "DataCloneError" DOMException.
  - 3. Let doomed be false.
  - 4. If targetPort is not null and transfer contains targetPort, then set doomed to true and optionally report to a developer console that the target port was posted to itself, causing the communication channel to be lost.
  - 5. Let serializeWithTransferResult be StructuredSerializeWithTransfer(message, transfer). Rethrow any exceptions.
  - 6. If targetPort is null, or if doomed is true, then return.
  - 7. Add a task that runs the following steps to the port message queue of targetPort:
    - 7a. Let finalTargetPort be the MessagePort in whose port message queue the task now finds itself.
    - 7b. Let messageEventTarget be finalTargetPort's message event target.
    - 7c. Let targetRealm be finalTargetPort's relevant realm.
    - 7d. Let deserializeRecord be StructuredDeserializeWithTransfer(serializeWithTransferResult, targetRealm). If this throws an exception, catch it, fire an event named messageerror at messageEventTarget, using MessageEvent, and then return.
    - 7e. Let messageClone be deserializeRecord.[[Deserialized]].
    - 7f. Let newPorts be a new frozen array consisting of all MessagePort objects in deserializeRecord.[[TransferredValues]], if any, maintaining their relative order.
    - 7g. Fire an event named message at messageEventTarget, using MessageEvent, with the data attribute initialized to messageClone and the ports attribute initialized to newPorts.

- [x] [MessagePort.postMessage(message, options)](https://html.spec.whatwg.org/#dom-messageport-postmessage-options) — The postMessage(message, options) method steps are:
  - 1. Let targetPort be the port with which this is entangled, if any; otherwise let it be null.
  - 2. Run the message port post message steps providing this, targetPort, message and options.

- [x] [MessagePort.postMessage(message, transfer)](https://html.spec.whatwg.org/#dom-messageport-postmessage) — The postMessage(message, transfer) method steps are:
  - 1. Let targetPort be the port with which this is entangled, if any; otherwise let it be null.
  - 2. Let options be «[ "transfer" → transfer ]».
  - 3. Run the message port post message steps providing this, targetPort, message and options.

- [x] [MessagePort.start()](https://html.spec.whatwg.org/#dom-messageport-start) — The start() method steps are to enable this's port message queue, if it is not already enabled.

- [x] [MessagePort.close()](https://html.spec.whatwg.org/#dom-messageport-close) — The close() method steps are:
  - 1. Set this's [[Detached]] internal slot value to true.
  - 2. If this is entangled, disentangle it.

#### [9.4.5 Broadcasting to many ports](https://html.spec.whatwg.org/#broadcasting-to-many-ports)

Status: n/a (not planned: no message channels or broadcast channels)

#### [9.4.6 Ports and garbage collection](https://html.spec.whatwg.org/#ports-and-garbage-collection)

Status: n/a (not planned: no message channels or broadcast channels)

### [9.5 Broadcasting to other browsing contexts](https://html.spec.whatwg.org/#broadcasting-to-other-browsing-contexts)

Status: n/a (not planned: no message channels or broadcast channels)
<!-- definitions: channel-name concept-broadcastchannel-closed -->

- [x] [new BroadcastChannel(name)](https://html.spec.whatwg.org/#dom-broadcastchannel) — The new BroadcastChannel(name) constructor steps are:
  - 1. Set this's channel name to name.
  - 2. Set this's closed flag to false.

- [x] [BroadcastChannel.name](https://html.spec.whatwg.org/#dom-broadcastchannel-name) — The name getter steps are to return this's channel name.

- [x] [eligible for messaging](https://html.spec.whatwg.org/#eligible-for-messaging) — A BroadcastChannel object is said to be eligible for messaging when its relevant global object is either:

- [x] [BroadcastChannel.postMessage(message)](https://html.spec.whatwg.org/#dom-broadcastchannel-postmessage) — The postMessage(message) method steps are:
  - 1. If this is not eligible for messaging, then return.
  - 2. If this's closed flag is true, then throw an "InvalidStateError" DOMException.
  - 3. Let serialized be StructuredSerialize(message). Rethrow any exceptions.
  - 4. Let sourceOrigin be this's relevant settings object's origin.
  - 5. Let sourceStorageKey be the result of running obtain a storage key for non-storage purposes with this's relevant settings object.
  - 6. Let destinations be a list of BroadcastChannel objects that match the following criteria: They are eligible for messaging.; The result of running obtain a storage key for non-storage purposes with their relevant settings object equals sourceStorageKey.; Their channel name is this's channel name.
  - 7. Remove source from destinations.
  - 8. Sort destinations such that all BroadcastChannel objects whose relevant agents are the same are sorted in creation order, oldest first. (This does not define a complete ordering. Within this constraint, user agents may sort the list in any implementation-defined manner.)
  - 9. For each destination in destinations, queue a global task on the DOM manipulation task source given destination's relevant global object to perform the following steps:
    - 9a. If destination's closed flag is true, then abort these steps.
    - 9b. Let targetRealm be destination's relevant realm.
    - 9c. Let data be StructuredDeserialize(serialized, targetRealm). If this throws an exception, catch it, fire an event named messageerror at destination, using MessageEvent, with its origin initialized to sourceOrigin, and then abort these steps.
    - 9d. Fire an event named message at destination, using MessageEvent, with the data attribute initialized to data and its origin initialized to sourceOrigin.

- [x] [BroadcastChannel.close()](https://html.spec.whatwg.org/#dom-broadcastchannel-close) — The close() method steps are to set this's closed flag to true.
