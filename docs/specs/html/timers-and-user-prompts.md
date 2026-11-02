# HTML review checklist: 8.7 Timers

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 13 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html timers-and-user-prompts` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [8.7 Timers](https://html.spec.whatwg.org/#timers)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::SetTimeout`/`SetInterval`/`ClearTimeout`/`ClearInterval` and `DOMTimer` (the timer initialization steps); handlers are C++ functions. Clamping and repeating follow upstream (see `DIFFERENCES.md`).
> `EventLoop::ScheduleTask` runs steps after a timeout, with no map of active timers.
<!-- definitions: map-of-settimeout-and-setinterval-ids dom-clearinterval timer-nesting-level timer-task-source map-of-active-timers -->

- [x] [WindowOrWorkerGlobalScope.setTimeout(handler, timeout, ...arguments)](https://html.spec.whatwg.org/#dom-settimeout) — The setTimeout(handler, timeout, ...arguments) method steps are to return the result of running the timer initialization steps given this, handler, timeout, arguments, and false.

- [x] [WindowOrWorkerGlobalScope.setInterval(handler, timeout, ...arguments)](https://html.spec.whatwg.org/#dom-setinterval) — The setInterval(handler, timeout, ...arguments) method steps are to return the result of running the timer initialization steps given this, handler, timeout, arguments, and true.

- [x] [WindowOrWorkerGlobalScope.clearTimeout(id)](https://html.spec.whatwg.org/#dom-cleartimeout) — The clearTimeout(id) and clearInterval(id) method steps are to remove this's map of setTimeout and setInterval IDs[id].

- [x] [timer initialization steps](https://html.spec.whatwg.org/#timer-initialisation-steps) — To perform the timer initialization steps, given a WindowOrWorkerGlobalScope global, a string or Function or TrustedScript handler, a number timeout, a list arguments, a boolean repeat, and optionally (and only if repeat is true) a number previousId, perform the following steps. They return a numbe…
  - 1. If handler is not a Function and previousId was not given:
    - 1a. Let globalName be "Window" if global is a Window object; "WorkerGlobalScope" otherwise.
    - 1b. Let methodName be "setInterval" if repeat is true; "setTimeout" otherwise.
    - 1c. Let sink be a concatenation of globalName, U+0020 SPACE, and methodName.
    - 1d. Set handler to the result of invoking the get trusted type compliant string algorithm with TrustedScript, global, handler, sink, and "script".
    - 1e. Perform EnsureCSPDoesNotBlockStringCompilation(global's relevant realm, « », handler, handler, timer, « », handler). If this throws an exception, catch it and return 0.
  - 2. Let thisArg be global if that is a WorkerGlobalScope object; otherwise let thisArg be the WindowProxy that corresponds to global.
  - 3. If previousId was given, let id be previousId; otherwise, let id be an implementation-defined integer that is greater than zero and does not already exist in global's map of setTimeout and setInterval IDs.
  - 4. If the surrounding agent's event loop's currently running task is a task that was created by this algorithm, then let nestingLevel be the task's timer nesting level. Otherwise, let nestingLevel be 0.
  - 5. If timeout is less than 0, then set timeout to 0.
  - 6. If nestingLevel is greater than 5, and timeout is less than 4, then set timeout to 4.
  - 7. Let initiating script be the active script.
  - 8. Let uniqueHandle be null.
  - 9. Let task be a task that runs the following substeps:
    - 9a. Assert: uniqueHandle is a unique internal value, not null.
    - 9b. If id does not exist in global's map of setTimeout and setInterval IDs, then abort these steps.
    - 9c. If global's map of setTimeout and setInterval IDs[id] does not equal uniqueHandle, then abort these steps.
    - 9d. Let settings object be global's relevant settings object.
    - 9e. If scripting is disabled for settings object, then abort these steps.
    - 9f. Record timing info for timer handler given handler, settings object, and repeat.
    - 9g. If handler is a Function, then invoke handler given arguments and "report", and with callback this value set to thisArg.
    - 9h. Otherwise:
      - 9h.i. Assert: handler is a string.
      - 9h.ii. Let fetch options be the default script fetch options.
      - 9h.iii. Let base URL be settings object's API base URL.
      - 9h.iv. If initiating script is not null:
        - 9h.iv.A. Set fetch options to a script fetch options whose cryptographic nonce is initiating script's fetch options's cryptographic nonce, integrity metadata is the empty string, parser metadata is "not-parser-inserted", credentials mode is initiating script's fetch options's credentials mode, referrer policy is initiating script's fetch options's referrer policy, and fetch priority is "auto".
        - 9h.iv.B. Set base URL to initiating script's base URL.
      - 9h.v. Let script be the result of creating a classic script given handler, settings object, base URL, and fetch options.
      - 9h.vi. Run the classic script script.
    - 9i. If id does not exist in global's map of setTimeout and setInterval IDs, then abort these steps.
    - 9j. If global's map of setTimeout and setInterval IDs[id] does not equal uniqueHandle, then abort these steps.
    - 9k. If repeat is true, then perform the timer initialization steps again, given global, handler, timeout, arguments, true, and id.
    - 9l. Otherwise, remove global's map of setTimeout and setInterval IDs[id].
  - 10. Increment nestingLevel by one.
  - 11. Set task's timer nesting level to nestingLevel.
  - 12. Let completionStep be an algorithm step which queues a global task on the timer task source given global to run task.
  - 13. Set uniqueHandle to the result of running steps after a timeout given global, "setTimeout/setInterval", timeout, and completionStep.
  - 14. Set global's map of setTimeout and setInterval IDs[id] to uniqueHandle.
  - 15. Return id.

- [x] [run steps after a timeout](https://html.spec.whatwg.org/#run-steps-after-a-timeout) — To run steps after a timeout, given a WindowOrWorkerGlobalScope global, a string orderingIdentifier, a number milliseconds, and a set of steps completionSteps, perform the following steps. They return a unique internal value.
  - 1. Let timerKey be a new unique internal value.
  - 2. Let startTime be the current high resolution time given global.
  - 3. Set global's map of active timers[timerKey] to startTime plus milliseconds.
  - 4. Run the following steps in parallel:
    - 4a. If global is a Window object, wait until global's associated Document has been fully active for a further milliseconds milliseconds (not necessarily consecutively). Otherwise, global is a WorkerGlobalScope object; wait until milliseconds milliseconds have passed with the worker not suspended (not necessarily consecutively).
    - 4b. Wait until any invocations of this algorithm that had the same global and orderingIdentifier, that started before this one, and whose milliseconds is less than or equal to this one's, have completed.
    - 4c. Optionally, wait a further implementation-defined length of time.
    - 4d. Perform completionSteps.
    - 4e. Remove global's map of active timers[timerKey].
  - 5. Return timerKey.

### [8.8 Microtask queuing](https://html.spec.whatwg.org/#microtask-queuing)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::QueueMicrotask`.

- [x] [WindowOrWorkerGlobalScope.queueMicrotask(callback)](https://html.spec.whatwg.org/#dom-queuemicrotask) — The queueMicrotask(callback) method must queue a microtask to invoke callback with « » and "report".

### [8.9 User prompts](https://html.spec.whatwg.org/#user-prompts)

Status: reviewed (2026-11-01)
> The sections below.

#### [8.9.1 Simple dialogs](https://html.spec.whatwg.org/#simple-dialogs)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::Alert`/`Confirm`/`Prompt` ask the host's `ChromeClient`, which may truncate; there's no WebDriver BiDi or sandboxed modals flag (FIXME).
<!-- definitions: dom-alert -->

- [x] [Window.alert()](https://html.spec.whatwg.org/#dom-alert-noargs) — The alert() and alert(message) method steps are:
  - 1. If we cannot show simple dialogs for this, then return.
  - 2. If the method was invoked with no arguments, then let message be the empty string; otherwise, let message be the method's first argument.
  - 3. Set message to the result of normalizing newlines given message.
  - 4. Set message to the result of optionally truncating message.
  - 5. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "alert", and message.
  - 6. If userPromptHandler is "none":
    - 6a. Show message to the user, treating U+000A LF as a line break.
    - 6b. Optionally, pause while waiting for the user to acknowledge the message.
  - 7. Invoke WebDriver BiDi user prompt closed with this, "alert", and true.

- [x] [Window.confirm(message)](https://html.spec.whatwg.org/#dom-confirm) — The confirm(message) method steps are:
  - 1. If we cannot show simple dialogs for this, then return false.
  - 2. Set message to the result of normalizing newlines given message.
  - 3. Set message to the result of optionally truncating message.
  - 4. Show message to the user, treating U+000A LF as a line break, and ask the user to respond with a positive or negative response.
  - 5. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "confirm", and message.
  - 6. Let accepted be false.
  - 7. If userPromptHandler is "none":
    - 7a. Pause until the user responds either positively or negatively.
    - 7b. If the user responded positively, then set accepted to true.
  - 8. If userPromptHandler is "accept", then set accepted to true.
  - 9. Invoke WebDriver BiDi user prompt closed with this, "confirm", and accepted.
  - 10. Return accepted.

- [x] [Window.prompt(message, default)](https://html.spec.whatwg.org/#dom-prompt) — The prompt(message, default) method steps are:
  - 1. If we cannot show simple dialogs for this, then return null.
  - 2. Set message to the result of normalizing newlines given message.
  - 3. Set message to the result of optionally truncating message.
  - 4. Set default to the result of optionally truncating default.
  - 5. Show message to the user, treating U+000A LF as a line break, and ask the user to either respond with a string value or abort. The response must be defaulted to the value given by default.
  - 6. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "prompt", and message.
  - 7. Let result be null.
  - 8. If userPromptHandler is "none":
    - 8a. Pause while waiting for the user's response.
    - 8b. If the user did not abort, then set result to the string that the user responded with.
  - 9. Otherwise, if userPromptHandler is "accept", then set result to the empty string.
  - 10. Invoke WebDriver BiDi user prompt closed with this, "prompt", false if result is null or true otherwise, and result.
  - 11. Return result.

- [x] [optionally truncate a simple dialog string](https://html.spec.whatwg.org/#optionally-truncate-a-simple-dialog-string) — To optionally truncate a simple dialog string s, return either s itself or some string derived from s that is shorter. User agents should not provide UI for displaying the elided portion of s, as this makes it too easy for abusers to create dialogs of the form "Important security alert! Click 'Show More' for full details!".

- [x] [cannot show simple dialogs](https://html.spec.whatwg.org/#cannot-show-simple-dialogs) — We cannot show simple dialogs for a Window window when the following algorithm returns true:
  - 1. If the active sandboxing flag set of window's associated Document has the sandboxed modals flag set, then return true.
  - 2. If window's relevant agent's event loop's termination nesting level is nonzero, then optionally return true.
  - 3. Optionally, return true. (For example, the user agent might give the user the option to ignore all modal dialogs, and would thus abort at this step whenever the method was invoked.)
  - 4. Return false.

#### [8.9.2 Printing](https://html.spec.whatwg.org/#printing)

Status: reviewed (2026-11-01)
> Fixed: `print()` while the document is loading prints once its load event has fired (`LocalDOMWindow::FinishedLoading`, from `Document::ImplicitClose`).
> `LocalDOMWindow::Print` and `RunPrintingSteps`; there are no child navigables.
<!-- definitions: print-when-loaded -->

- [x] [Window.print()](https://html.spec.whatwg.org/#dom-print) — The print() method steps are:
  - 1. Let document be this's associated Document.
  - 2. If document is not fully active, then return.
  - 3. If document's unload counter is greater than 0, then return.
  - 4. If document is ready for post-load tasks, then run the printing steps for document.
  - 5. Otherwise, set document's print when loaded flag.

- [x] [printing steps](https://html.spec.whatwg.org/#printing-steps) — The printing steps for a Document document are:
  - 1. The user agent may display a message to the user or return (or both).
  - 2. If the active sandboxing flag set of document has the sandboxed modals flag set, then return.
  - 3. The user agent must fire an event named beforeprint at the relevant global object of document, as well as any child navigable in it.
  - 4. The user agent should offer the user the opportunity to obtain a physical form (or the representation of a physical form) of document. The user agent may wait for the user to either accept or decline before returning; if so, the user agent must pause while the method is waiting. Even if the user agent doesn't wait at this point, the user agent must use the state of the relevant documents as they…
  - 5. The user agent must fire an event named afterprint at the relevant global object of document, as well as any child navigables in it.
