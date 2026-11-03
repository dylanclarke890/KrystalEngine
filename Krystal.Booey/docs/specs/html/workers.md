# HTML review checklist: 10 Web workers

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 36 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html workers` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [10 Web workers](https://html.spec.whatwg.org/#workers)

Status: reviewed (2026-11-01)
> Only `navigator.hardwareConcurrency` applies: there are no workers.

### [10.1 Introduction](https://html.spec.whatwg.org/#introduction-15)

Status: n/a (no workers)

#### [10.1.1 Scope](https://html.spec.whatwg.org/#scope-2)

Status: n/a (no workers)

#### [10.1.2 Examples](https://html.spec.whatwg.org/#examples-6)

Status: n/a (no workers)

##### [10.1.2.1 A background number-crunching worker](https://html.spec.whatwg.org/#a-background-number-crunching-worker)

Status: n/a (no workers)

##### [10.1.2.2 Using a JavaScript module as a worker](https://html.spec.whatwg.org/#module-worker-example)

Status: n/a (no workers)

##### [10.1.2.3 Shared workers introduction](https://html.spec.whatwg.org/#shared-workers-introduction)

Status: n/a (no workers)

##### [10.1.2.4 Shared state using a shared worker](https://html.spec.whatwg.org/#shared-state-using-a-shared-worker)

Status: n/a (no workers)

##### [10.1.2.5 Delegation](https://html.spec.whatwg.org/#delegation)

Status: n/a (no workers)

##### [10.1.2.6 Providing libraries](https://html.spec.whatwg.org/#providing-libraries)

Status: n/a (no workers)

#### [10.1.3 Tutorials](https://html.spec.whatwg.org/#tutorials)

Status: n/a (no workers)

##### [10.1.3.1 Creating a dedicated worker](https://html.spec.whatwg.org/#creating-a-dedicated-worker)

Status: n/a (no workers)

##### [10.1.3.2 Communicating with a dedicated worker](https://html.spec.whatwg.org/#communicating-with-a-dedicated-worker)

Status: n/a (no workers)

##### [10.1.3.3 Shared workers](https://html.spec.whatwg.org/#shared-workers)

Status: n/a (no workers)

### [10.2 Infrastructure](https://html.spec.whatwg.org/#infrastructure-2)

Status: reviewed (2026-11-01)
> Only `navigator.hardwareConcurrency` applies.

#### [10.2.1 The global scope](https://html.spec.whatwg.org/#the-global-scope)

Status: n/a (no workers)

##### [10.2.1.1 The WorkerGlobalScope common interface](https://html.spec.whatwg.org/#the-workerglobalscope-common-interface)

Status: n/a (no workers)
<!-- definitions: concept-WorkerGlobalScope-owner-set concept-workerglobalscope-type concept-workerglobalscope-url concept-workerglobalscope-name concept-workerglobalscope-policy-container concept-workerglobalscope-embedder-policy concept-workerglobalscope-module-map concept-workerglobalscope-cross-origin-isolated-capability -->

- [x] [WorkerGlobalScope.self](https://html.spec.whatwg.org/#dom-workerglobalscope-self) — The self attribute must return the WorkerGlobalScope object itself.

- [x] [WorkerGlobalScope.location](https://html.spec.whatwg.org/#dom-workerglobalscope-location) — The location attribute must return the WorkerLocation object whose associated WorkerGlobalScope object is the WorkerGlobalScope object.

##### [10.2.1.2 Dedicated workers and the DedicatedWorkerGlobalScope interface](https://html.spec.whatwg.org/#dedicated-workers-and-the-dedicatedworkerglobalscope-interface)

Status: n/a (no workers)
<!-- definitions: inside-port dom-dedicatedworkerglobalscope-postmessage-options -->

- [x] [DedicatedWorkerGlobalScope.name](https://html.spec.whatwg.org/#dom-dedicatedworkerglobalscope-name) — The name getter steps are to return this's name. Its value represents the name given to the worker using the Worker constructor, used primarily for debugging purposes.

- [x] [DedicatedWorkerGlobalScope.postMessage(message, transfer)](https://html.spec.whatwg.org/#dom-dedicatedworkerglobalscope-postmessage) — The postMessage(message, transfer) and postMessage(message, options) methods on DedicatedWorkerGlobalScope objects act as if, when invoked, it immediately invoked the respective postMessage(message, transfer) and postMessage(message, options) on the port, with the same arguments, and returned the same return value.

- [x] [close a worker](https://html.spec.whatwg.org/#close-a-worker) — To close a worker, given a workerGlobal, run these steps:
  - 1. Discard any tasks that have been added to workerGlobal's relevant agent's event loop's task queues.
  - 2. Set workerGlobal's closing flag to true. (This prevents any further tasks from being queued.)

- [x] [DedicatedWorkerGlobalScope.close()](https://html.spec.whatwg.org/#dom-dedicatedworkerglobalscope-close) — The close() method steps are to close a worker given this.

##### [10.2.1.3 Shared workers and the SharedWorkerGlobalScope interface](https://html.spec.whatwg.org/#shared-workers-and-the-sharedworkerglobalscope-interface)

Status: n/a (no workers)
<!-- definitions: concept-sharedworkerglobalscope-constructor-storage-key concept-sharedworkerglobalscope-constructor-url concept-sharedworkerglobalscope-credentials concept-sharedworkerglobalscope-extended-lifetime -->

- [x] [SharedWorkerGlobalScope.name](https://html.spec.whatwg.org/#dom-sharedworkerglobalscope-name) — The name getter steps are to return this's name. Its value represents the name that can be used to obtain a reference to the worker using the SharedWorker constructor.

- [x] [SharedWorkerGlobalScope.close()](https://html.spec.whatwg.org/#dom-sharedworkerglobalscope-close) — The close() method steps are to close a worker given this.

#### [10.2.2 The event loop](https://html.spec.whatwg.org/#worker-event-loop)

Status: n/a (no workers)
<!-- definitions: dom-workerglobalscope-closing -->

#### [10.2.3 The worker's lifetime](https://html.spec.whatwg.org/#the-worker's-lifetime)

Status: n/a (no workers)
<!-- definitions: between-loads-shared-worker-timeout extended-lifetime-shared-worker-timeout -->

- [x] [the worker's ports](https://html.spec.whatwg.org/#the-worker's-ports) — Each WorkerGlobalScope object worker global scope has a list of the worker's ports, which consists of all the MessagePort objects that are entangled with another port and that have one (but only one) port owned by worker global scope. This list includes the implicit MessagePort in the case of dedicated workers.

- [x] [relevant owner to add](https://html.spec.whatwg.org/#relevant-owner-to-add) — Given an environment settings object o when creating or obtaining a worker, the relevant owner to add depends on the type of global object specified by o. If o's global object is a WorkerGlobalScope object (i.e., if we are creating a nested dedicated worker), then the relevant owner is that global object. Otherwise, o's global object is a Window object, and the relevant owner is that Window's associated Document.

- [x] [in extended lifetime](https://html.spec.whatwg.org/#in-extended-lifetime-worker) — A WorkerGlobalScope global is in extended lifetime if the following algorithm returns true:
  - 1. If global is not a SharedWorkerGlobalScope, then return false.
  - 2. If global's extended lifetime is false, then return false.
  - 3. If global's owner set is empty, but has been empty for less than the user agent's extended lifetime shared worker timeout, then return true.
  - 4. For each owner of global's owner set:
    - 4a. If owner is a Document that is fully active, then return false.
  - 5. For each owner of global's owner set:
    - 5a. If owner is a Document and the amount of time that owner has been non-fully active is less than the user agent's extended lifetime shared worker timeout, then return true.
    - 5b. If owner is a WorkerGlobalScope that is in extended lifetime, then return true.
  - 6. Return false.

- [x] [actively needed](https://html.spec.whatwg.org/#active-needed-worker) — A WorkerGlobalScope global is actively needed if the following algorithm returns true:
  - 1. If global is in extended lifetime, then return true.
  - 2. For each owner of global's owner set:
    - 2a. If owner is a Document, and owner is fully active, then return true.
    - 2b. If owner is a WorkerGlobalScope that is actively needed, then return true.
  - 3. Return false.

- [x] [protected](https://html.spec.whatwg.org/#protected-worker) — A WorkerGlobalScope global is protected if the following algorithm returns true:
  - 1. If global is not actively needed, then return false.
  - 2. If global is in extended lifetime, then return true.
  - 3. If global is a SharedWorkerGlobalScope, then return true.
  - 4. If global's the worker's ports is not empty, then return true.
  - 5. If global has outstanding timers, database transactions, or network connections, then return true.
  - 6. Return false.

- [x] [permissible](https://html.spec.whatwg.org/#permissible-worker) — A WorkerGlobalScope global is permissible if the following algorithm returns true:
  - 1. If global's owner set is not empty, then return true.
  - 2. If global is in extended lifetime, then return true.
  - 3. If all of the following are true: global is a SharedWorkerGlobalScope;; global's owner set has been empty for no more than the user agent's between-loads shared worker timeout; and; the user agent has a navigable whose active document is not completely loaded, then return true.
  - 4. Return false.

- [x] [suspendable](https://html.spec.whatwg.org/#suspendable-worker) — A WorkerGlobalScope global is suspendable if the following algorithm returns true:
  - 1. If global is actively needed, then return false.
  - 2. If global is permissible, then return true.
  - 3. Return false.

#### [10.2.4 Processing model](https://html.spec.whatwg.org/#worker-processing-model)

Status: n/a (no workers)

- [x] [run a worker](https://html.spec.whatwg.org/#run-a-worker) — When a user agent is to run a worker for a script with Worker or SharedWorker object worker, URL url, environment settings object outside settings, MessagePort outside port, and a WorkerOptions dictionary options, it must run the following steps:
  - 1. Let is shared be true if worker is a SharedWorker object, and false otherwise.
  - 2. Let owner be the relevant owner to add given outside settings.
  - 3. Let unsafeWorkerCreationTime be the unsafe shared current time.
  - 4. Let agent be the result of obtaining a dedicated/shared worker agent given outside settings and is shared. Run the rest of these steps in that agent.
  - 5. Let realm execution context be the result of creating a new realm given agent and the following customizations: For the global object, if is shared is true, create a new SharedWorkerGlobalScope object. Otherwise, create a new DedicatedWorkerGlobalScope object.
  - 6. Let worker global scope be the global object of realm execution context's Realm component.
  - 7. Set up a worker environment settings object with realm execution context, outside settings, and unsafeWorkerCreationTime, and let inside settings be the result.
  - 8. Set worker global scope's name to options["name"].
  - 9. Append owner to worker global scope's owner set.
  - 10. If is shared is true:
    - 10a. Set worker global scope's constructor storage key to the result of running obtain a storage key for non-storage purposes given outside settings.
    - 10b. Set worker global scope's constructor URL to url.
    - 10c. Set worker global scope's type to options["type"].
    - 10d. Set worker global scope's credentials to options["credentials"].
    - 10e. Set worker global scope's extended lifetime to options["extendedLifetime"].
  - 11. Let destination be "sharedworker" if is shared is true, and "worker" otherwise.
  - 12. Obtain script by switching on options["type"]: "classic"Fetch a classic worker script given url, outside settings, destination, inside settings, and with onComplete and performFetch as defined below."module"Fetch a module worker script graph given url, outside settings, destination, the value of the credentials member of options, inside settings, and with onComplete and performFetch as defined be…
    - 12a. If isTopLevel is false, fetch request with processResponseConsumeBody set to processCustomFetchResponse, and abort these steps.
    - 12b. Set request's reserved client to inside settings.
    - 12c. Fetch request with processResponseConsumeBody set to the following steps given response response and null, failure, or a byte sequence bodyBytes:
      - 12c.i. Set worker global scope's url to response's URL.
      - 12c.ii. Set inside settings's creation URL to response's URL.
      - 12c.iii. Initialize worker global scope's policy container given worker global scope, response, and inside settings.
      - 12c.iv. If the Run CSP initialization for a global object algorithm returns "Blocked" when executed upon worker global scope, set response to a network error. [CSP]
      - 12c.v. If worker global scope's embedder policy's value is compatible with cross-origin isolation and is shared is true, then set agent's agent cluster's cross-origin isolation mode to "logical" or "concrete". The one chosen is implementation-defined.
      - 12c.vi. If the result of checking a global object's embedder policy with worker global scope, outside settings, and response is false, then set response to a network error.
      - 12c.vii. Set worker global scope's cross-origin isolated capability to true if agent's agent cluster's cross-origin isolation mode is "concrete".
      - 12c.viii. If is shared is false and owner's cross-origin isolated capability is false, then set worker global scope's cross-origin isolated capability to false.
      - 12c.ix. If is shared is false and response's URL's scheme is "data", then set worker global scope's cross-origin isolated capability to false.
      - 12c.x. Run processCustomFetchResponse with response and bodyBytes.
    - 12a. If script is null or if script's error to rethrow is non-null:
      - 12a.i. Queue a global task on the DOM manipulation task source given worker's relevant global object to fire an event named error at worker.
      - 12a.ii. Run the environment discarding steps for inside settings.
      - 12a.iii. Abort these steps.
    - 12b. Associate worker with worker global scope.
    - 12c. Let inside port be a new MessagePort object in inside settings's realm.
    - 12d. If is shared is false:
      - 12d.i. Set inside port's message event target to worker global scope.
      - 12d.ii. Set worker global scope's inside port to inside port.
    - 12e. Entangle outside port and inside port.
    - 12f. Create a new WorkerLocation object and associate it with worker global scope.
    - 12g. Closing orphan workers: Start monitoring worker global scope such that no sooner than it stops being protected, and no later than it stops being permissible, worker global scope's closing flag is set to true.
    - 12h. Suspending workers: Start monitoring worker global scope, such that whenever worker global scope's closing flag is false and it is suspendable, the user agent suspends execution of script in worker global scope until such time as either the closing flag switches to true or worker global scope stops being suspendable.
    - 12i. Set inside settings's execution ready flag.
    - 12j. If script is a classic script, then run the classic script script. Otherwise, it is a module script; run the module script script.
    - 12k. Enable outside port's port message queue.
    - 12l. If is shared is false, enable the port message queue of the worker's implicit port.
    - 12m. If is shared is true, then queue a global task on the DOM manipulation task source given worker global scope to fire an event named connect at worker global scope, using MessageEvent, with the data attribute initialized to the empty string, the ports attribute initialized to a new frozen array containing inside port, and the source attribute initialized to inside port.
    - 12n. Enable the client message queue of the ServiceWorkerContainer object whose associated service worker client is worker global scope's relevant settings object.
    - 12o. Event loop: Run the responsible event loop specified by inside settings until it is destroyed.
    - 12p. Clear the worker global scope's map of active timers.
    - 12q. Disentangle all the ports in the list of the worker's ports.
    - 12r. Empty worker global scope's owner set.

- [x] [terminate a worker](https://html.spec.whatwg.org/#terminate-a-worker) — When a user agent is to terminate a worker, it must run the following steps in parallel with the worker's main loop (the "run a worker" processing model defined above):
  - 1. Set the worker's WorkerGlobalScope object's closing flag to true.
  - 2. If there are any tasks queued in the WorkerGlobalScope object's relevant agent's event loop's task queues, discard them without processing them.
  - 3. Abort the script currently running in the worker.
  - 4. If the worker's WorkerGlobalScope object is actually a DedicatedWorkerGlobalScope object (i.e. the worker is a dedicated worker), then empty the port message queue of the port that the worker's implicit port is entangled with.

#### [10.2.5 Runtime script errors](https://html.spec.whatwg.org/#runtime-script-errors-2)

Status: n/a (no workers)

#### [10.2.6 Creating workers](https://html.spec.whatwg.org/#creating-workers)

Status: n/a (no workers)

##### [10.2.6.1 The AbstractWorker mixin](https://html.spec.whatwg.org/#the-abstractworker-mixin)

Status: n/a (no workers)

##### [10.2.6.2 Script settings for workers](https://html.spec.whatwg.org/#script-settings-for-workers)

Status: n/a (no workers)

- [x] [set up a worker environment settings object](https://html.spec.whatwg.org/#set-up-a-worker-environment-settings-object) — To set up a worker environment settings object, given a JavaScript execution context execution context, an environment settings object outside settings, and a number unsafeWorkerCreationTime:
  - 1. Let realm be the value of execution context's Realm component.
  - 2. Let worker global scope be realm's global object.
  - 3. Let origin be a unique opaque origin if worker global scope's url's scheme is "data"; otherwise outside settings's origin.
  - 4. Let settings object be a new environment settings object whose algorithms are defined as follows: The realm execution context Return execution context. The module map Return worker global scope's module map. The API base URL Return worker global scope's url. The originReturn origin.The has cross-site ancestor The policy container Return worker global scope's policy container. The cross-origin iso…
    - 4a. If outside settings's has cross-site ancestor is true, then return true.
    - 4b. If worker global scope's url's scheme is "data", then return true.
    - 4c. Return false.
  - 5. Set settings object's id to a new unique opaque string, creation URL to worker global scope's url, top-level creation URL to null, target browsing context to null, and active service worker to null.
  - 6. If worker global scope is a DedicatedWorkerGlobalScope object, then set settings object's top-level origin to outside settings's top-level origin.
  - 7. Otherwise, set settings object's top-level origin to an implementation-defined value.
  - 8. Set realm's [[HostDefined]] field to settings object.
  - 9. Return settings object.

##### [10.2.6.3 Dedicated workers and the Worker interface](https://html.spec.whatwg.org/#dedicated-workers-and-the-worker-interface)

Status: n/a (no workers)
<!-- definitions: outside-port dom-worker-postmessage-options -->

- [x] [Worker.terminate()](https://html.spec.whatwg.org/#dom-worker-terminate) — The terminate() method steps are to terminate a worker given this's worker.

- [x] [Worker.postMessage(message, transfer)](https://html.spec.whatwg.org/#dom-worker-postmessage) — The postMessage(message, transfer) and postMessage(message, options) methods on Worker objects act as if, when invoked, they immediately invoked the respective postMessage(message, transfer) and postMessage(message, options) on this's outside port, with the same arguments, and returned the same return value.

- [x] [Worker.new Worker(scriptURL, options)](https://html.spec.whatwg.org/#dom-worker) — The new Worker(scriptURL, options) constructor steps are:
  - 1. Let compliantScriptURL be the result of invoking the get trusted type compliant string algorithm with TrustedScriptURL, this's relevant global object, scriptURL, "Worker constructor", and "script".
  - 2. Let outsideSettings be this's relevant settings object.
  - 3. Let workerURL be the result of encoding-parsing a URL given compliantScriptURL, relative to outsideSettings.
  - 4. If workerURL is failure, then throw a "SyntaxError" DOMException.
  - 5. Let outsidePort be a new MessagePort in outsideSettings's realm.
  - 6. Set outsidePort's message event target to this.
  - 7. Set this's outside port to outsidePort.
  - 8. Let worker be this.
  - 9. Run this step in parallel:
    - 9a. Run a worker given worker, workerURL, outsideSettings, outsidePort, and options.

##### [10.2.6.4 Shared workers and the SharedWorker interface](https://html.spec.whatwg.org/#shared-workers-and-the-sharedworker-interface)

Status: n/a (no workers)
<!-- definitions: shared-worker-manager concept-sharedworker-port -->

- [x] [SharedWorker.port](https://html.spec.whatwg.org/#dom-sharedworker-port) — The port getter steps are to return this's port.

- [x] [SharedWorker.new SharedWorker(scriptURL, options)](https://html.spec.whatwg.org/#dom-sharedworker) — The new SharedWorker(scriptURL, options) constructor steps are:
  - 1. Let compliantScriptURL be the result of invoking the get trusted type compliant string algorithm with TrustedScriptURL, this's relevant global object, scriptURL, "SharedWorker constructor", and "script".
  - 2. If options is a DOMString, set options to a new WorkerOptions dictionary whose name member is set to the value of options and whose other members are set to their default values.
  - 3. Let outsideSettings be this's relevant settings object.
  - 4. Let urlRecord be the result of encoding-parsing a URL given compliantScriptURL, relative to outsideSettings.
  - 5. If urlRecord is failure, then throw a "SyntaxError" DOMException.
  - 6. Let outsidePort be a new MessagePort in outsideSettings's realm.
  - 7. Set this's port to outsidePort.
  - 8. Let callerIsSecureContext be true if outsideSettings is a secure context; otherwise, false.
  - 9. Let outsideStorageKey be the result of running obtain a storage key for non-storage purposes given outsideSettings.
  - 10. Let worker be this.
  - 11. Enqueue the following steps to the shared worker manager:
    - 11a. Let workerGlobalScope be null.
    - 11b. For each scope in the list of all SharedWorkerGlobalScope objects:
      - 11b.i. If all of the following are true: scope's constructor storage key equals outsideStorageKey;; scope's closing flag is false;; scope's constructor URL equals urlRecord; and; scope's name equals options["name"], then:
        - 11b.i.A. Set workerGlobalScope to scope.
        - 11b.i.B. Break.
    - 11c. If workerGlobalScope is not null, but the user agent has been configured to disallow communication between the worker represented by the workerGlobalScope and the scripts whose settings object is outsideSettings, then set workerGlobalScope to null.
    - 11d. If workerGlobalScope is not null, and any of the following are true: workerGlobalScope's type is not equal to options["type"];; workerGlobalScope's credentials is not equal to options["credentials"]; or; workerGlobalScope's extended lifetime is not equal to options["extendedLifetime"], then:
      - 11d.i. Queue a global task on the DOM manipulation task source given worker's relevant global object to fire an event named error at worker.
      - 11d.ii. Abort these steps.
    - 11e. If workerGlobalScope is not null:
      - 11e.i. Let insideSettings be workerGlobalScope's relevant settings object.
      - 11e.ii. Let workerIsSecureContext be true if insideSettings is a secure context; otherwise, false.
      - 11e.iii. If workerIsSecureContext is not callerIsSecureContext:
        - 11e.iii.A. Queue a global task on the DOM manipulation task source given worker's relevant global object to fire an event named error at worker.
        - 11e.iii.B. Abort these steps.
      - 11e.iv. Associate worker with workerGlobalScope.
      - 11e.v. Let insidePort be a new MessagePort in insideSettings's realm.
      - 11e.vi. Entangle outsidePort and insidePort.
      - 11e.vii. Queue a global task on the DOM manipulation task source given workerGlobalScope to fire an event named connect at workerGlobalScope, using MessageEvent, with the data attribute initialized to the empty string, the ports attribute initialized to a new frozen array containing only insidePort, and the source attribute initialized to insidePort.
      - 11e.viii. Append the relevant owner to add given outsideSettings to workerGlobalScope's owner set.
    - 11f. Otherwise, in parallel, run a worker given worker, urlRecord, outsideSettings, outsidePort, and options.

#### [10.2.7 Concurrent hardware capabilities](https://html.spec.whatwg.org/#navigator.hardwareconcurrency)

Status: reviewed (2026-11-01)
> `Navigator::HardwareConcurrency`: 4 or 8, as upstream (see `DIFFERENCES.md`).

- [x] [NavigatorConcurrentHardware.navigator.hardwareConcurrency](https://html.spec.whatwg.org/#dom-navigator-hardwareconcurrency) — The navigator.hardwareConcurrency attribute's getter must return a number between 1 and the number of logical processors potentially available to the user agent. If this cannot be determined, the getter must return 1.

### [10.3 APIs available to workers](https://html.spec.whatwg.org/#apis-available-to-workers)

Status: n/a (no workers)

#### [10.3.1 Importing scripts and libraries](https://html.spec.whatwg.org/#importing-scripts-and-libraries)

Status: n/a (no workers)

- [x] [WorkerGlobalScope.importScripts(...urls)](https://html.spec.whatwg.org/#dom-workerglobalscope-importscripts) — The importScripts(...urls) method steps are:
  - 1. Let urlStrings be « ».
  - 2. For each url of urls:
    - 2a. Append the result of invoking the get trusted type compliant string algorithm with TrustedScriptURL, this's relevant global object, url, "WorkerGlobalScope importScripts", and "script" to urlStrings.
  - 3. Import scripts into worker global scope given this and urlStrings.

- [x] [import scripts into worker global scope](https://html.spec.whatwg.org/#import-scripts-into-worker-global-scope) — To import scripts into worker global scope, given a WorkerGlobalScope object worker global scope, a list of scalar value strings urls, and an optional perform the fetch hook performFetch:
  - 1. If worker global scope's type is "module", throw a TypeError exception.
  - 2. Let settings object be the current settings object.
  - 3. If urls is empty, return.
  - 4. Let urlRecords be « ».
  - 5. For each url of urls:
    - 5a. Let urlRecord be the result of encoding-parsing a URL given url, relative to settings object.
    - 5b. If urlRecord is failure, then throw a "SyntaxError" DOMException.
    - 5c. Append urlRecord to urlRecords.
  - 6. For each urlRecord of urlRecords:
    - 6a. Fetch a classic worker-imported script given urlRecord and settings object, passing along performFetch if provided. If this succeeds, let script be the result. Otherwise, rethrow the exception.
    - 6b. Run the classic script script, with rethrow errors set to true. If an exception was thrown or if the script was prematurely aborted, then abort all these steps, letting the exception or aborting continue to be processed by the calling script.

#### [10.3.2 The WorkerNavigator interface](https://html.spec.whatwg.org/#the-workernavigator-object)

Status: n/a (no workers)

- [x] [WorkerGlobalScope.navigator](https://html.spec.whatwg.org/#dom-worker-navigator) — The navigator attribute of the WorkerGlobalScope interface must return an instance of the WorkerNavigator interface, which represents the identity and state of the user agent (the client):

#### [10.3.3 The WorkerLocation interface](https://html.spec.whatwg.org/#worker-locations)

Status: n/a (no workers)
<!-- definitions: concept-workerlocation-workerglobalscope -->

- [x] [WorkerLocation.href](https://html.spec.whatwg.org/#dom-workerlocation-href) — The href getter steps are to return this's WorkerGlobalScope object's url, serialized.

- [x] [WorkerLocation.origin](https://html.spec.whatwg.org/#dom-workerlocation-origin) — The origin getter steps are to return the serialization of this's WorkerGlobalScope object's url's origin.

- [x] [WorkerLocation.protocol](https://html.spec.whatwg.org/#dom-workerlocation-protocol) — The protocol getter steps are to return this's WorkerGlobalScope object's url's scheme, followed by ":".

- [x] [WorkerLocation.host](https://html.spec.whatwg.org/#dom-workerlocation-host) — The host getter steps are:
  - 1. Let url be this's WorkerGlobalScope object's url.
  - 2. If url's host is null, return the empty string.
  - 3. If url's port is null, return url's host, serialized.
  - 4. Return url's host, serialized, followed by ":" and url's port, serialized.

- [x] [WorkerLocation.hostname](https://html.spec.whatwg.org/#dom-workerlocation-hostname) — The hostname getter steps are:
  - 1. Let host be this's WorkerGlobalScope object's url's host.
  - 2. If host is null, return the empty string.
  - 3. Return host, serialized.

- [x] [WorkerLocation.port](https://html.spec.whatwg.org/#dom-workerlocation-port) — The port getter steps are:
  - 1. Let port be this's WorkerGlobalScope object's url's port.
  - 2. If port is null, return the empty string.
  - 3. Return port, serialized.

- [x] [WorkerLocation.pathname](https://html.spec.whatwg.org/#dom-workerlocation-pathname) — The pathname getter steps are to return the result of URL path serializing this's WorkerGlobalScope object's url.

- [x] [WorkerLocation.search](https://html.spec.whatwg.org/#dom-workerlocation-search) — The search getter steps are:
  - 1. Let query be this's WorkerGlobalScope object's url's query.
  - 2. If query is either null or the empty string, return the empty string.
  - 3. Return "?", followed by query.

- [x] [WorkerLocation.hash](https://html.spec.whatwg.org/#dom-workerlocation-hash) — The hash getter steps are:
  - 1. Let fragment be this's WorkerGlobalScope object's url's fragment.
  - 2. If fragment is either null or the empty string, return the empty string.
  - 3. Return "#", followed by fragment.
