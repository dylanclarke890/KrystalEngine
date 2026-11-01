# HTML review checklist: 4.8.8 The video element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 125 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html media` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.8.8 The video element](https://html.spec.whatwg.org/#the-video-element)

Status: unreviewed
<!-- definitions: attr-video-poster attr-video-playsinline concept-video-intrinsic-width concept-video-intrinsic-height -->

- [ ] [poster frame](https://html.spec.whatwg.org/#poster-frame) — If the specified resource is to be used, then, when the element is created or when the poster attribute is set, changed, or removed, the user agent must run the following steps to determine the element's poster frame (regardless of the value of the element's show poster flag):
  - 1. If there is an existing instance of this algorithm running for this video element, abort that instance of this algorithm without changing the poster frame.
  - 2. If the poster attribute's value is the empty string or if the attribute is absent:
    - 2a. Set the video element's poster lazy load resumption steps to null.
    - 2b. There is no poster frame; return.
  - 3. Let url be the result of encoding-parsing a URL given the poster attribute's value, relative to the element's node document.
  - 4. If url is failure, then return.
  - 5. Let request be a new request whose URL is url, client is the element's node document's relevant settings object, destination is "image", initiator type is "video", credentials mode is "include", and whose use-URL-credentials flag is set.
  - 6. If the will lazy load element steps given the video return true:
    - 6a. Let posterResumptionSteps be the rest of this algorithm starting with the step labeled Fetch.
    - 6b. Set the video element's poster lazy load resumption steps to posterResumptionSteps.
    - 6c. Return.
  - 7. Fetch request. This must delay the load event of the element's node document.
  - 8. If an image is thus obtained, the poster frame is that image. Otherwise, there is no poster frame.

- [ ] [HTMLVideoElement.videoWidth](https://html.spec.whatwg.org/#dom-video-videowidth) — The videoWidth getter steps are:
  - 1. If this's readyState attribute is HAVE_NOTHING, then return 0.
  - 2. Return the natural width of the video in CSS pixels.

- [ ] [HTMLVideoElement.videoHeight](https://html.spec.whatwg.org/#dom-video-videoheight) — The videoHeight getter steps are:
  - 1. If this's readyState attribute is HAVE_NOTHING, then return 0.
  - 2. Return the natural height of the video in CSS pixels.

#### [4.8.9 The audio element](https://html.spec.whatwg.org/#the-audio-element)

Status: unreviewed

- [ ] [Audio(src)](https://html.spec.whatwg.org/#dom-audio) — A legacy factory function is provided for creating HTMLAudioElement objects (in addition to the factory methods from DOM such as createElement()): Audio(src). When invoked, the legacy factory function must perform the following steps:
  - 1. Let document be the current global object's associated Document.
  - 2. Let audio be the result of creating an element given document, "audio", and the HTML namespace.
  - 3. Set an attribute value for audio using "preload" and "auto".
  - 4. If src is given, then set an attribute value for audio using "src" and src. (This will cause the user agent to invoke the object's resource selection algorithm before returning.)
  - 5. Return audio.

#### [4.8.10 The track element](https://html.spec.whatwg.org/#the-track-element)

Status: unreviewed
<!-- definitions: attr-track-kind attr-track-src attr-track-srclang attr-track-label attr-track-default dom-track-none dom-track-loading dom-track-loaded dom-track-error -->

- [ ] [When the element's src attribute is set, run these steps](https://html.spec.whatwg.org/#track-url) — When the element's src attribute is set, run these steps:
  - 1. Let trackURL be failure.
  - 2. Let value be the element's src attribute value.
  - 3. If value is not the empty string, then set trackURL to the result of encoding-parsing-and-serializing a URL given value, relative to the element's node document.
  - 4. Set the element's track URL to trackURL if it is not failure; otherwise to the empty string.

- [ ] [track language](https://html.spec.whatwg.org/#track-language) — If the element has a srclang attribute whose value is not the empty string, then the element's track language is the value of the attribute. Otherwise, the element has no track language.

- [ ] [track label](https://html.spec.whatwg.org/#track-label) — If the element has a label attribute whose value is not the empty string, then the element's track label is the value of the attribute. Otherwise, the element's track label is an empty string.

- [ ] [HTMLTrackElement.readyState](https://html.spec.whatwg.org/#dom-track-readystate) — The readyState attribute must return the numeric value corresponding to the text track readiness state of the track element's text track, as defined by the following list:
  - • NONE (numeric value 0): The text track not loaded state.
  - • LOADING (numeric value 1): The text track loading state.
  - • LOADED (numeric value 2): The text track loaded state.
  - • ERROR (numeric value 3): The text track failed to load state.

- [ ] [HTMLTrackElement.track](https://html.spec.whatwg.org/#dom-track-track) — The track getter steps are to return this's text track.

- [ ] [HTMLTrackElement.kind](https://html.spec.whatwg.org/#dom-track-kind) — The kind IDL attribute must reflect the content attribute of the same name, limited to only known values.

#### [4.8.11 Media elements](https://html.spec.whatwg.org/#media-elements)

Status: n/a (later: media playback)
<!-- definitions: media-element media-element-attributes media-data media-resource media-resource-origin media-element-event-task-source -->

- [x] [When the loading attribute's state is changed to the Eager state, the user agen…](https://html.spec.whatwg.org/#attr-media-loading) — When the loading attribute's state is changed to the Eager state, the user agent must run these steps:
  - 1. Let resumptionSteps be the media element's lazy load resumption steps.
  - 2. Let posterResumptionSteps be null.
  - 3. If the media element is a video element, then set posterResumptionSteps to the video element's poster lazy load resumption steps.
  - 4. If resumptionSteps is null and posterResumptionSteps is null, then return.
  - 5. If resumptionSteps is not null:
    - 5a. Set the media element's lazy load resumption steps to null.
    - 5b. Invoke resumptionSteps.
  - 6. If posterResumptionSteps is not null:
    - 6a. Set the video element's poster lazy load resumption steps to null.
    - 6b. Invoke posterResumptionSteps.

- [x] [HTMLMediaElement.loading](https://html.spec.whatwg.org/#dom-media-loading) — The loading IDL attribute must reflect the loading content attribute, limited to only known values.

- [x] [queue a media element task](https://html.spec.whatwg.org/#queue-a-media-element-task) — To queue a media element task with a media element element and a series of steps steps, queue an element task on the media element's media element event task source given element and steps.

##### [4.8.11.1 Error codes](https://html.spec.whatwg.org/#error-codes)

Status: n/a (later: media playback)
<!-- definitions: concept-mediaerror-message concept-mediaerror-code dom-mediaerror-media_err_aborted dom-mediaerror-media_err_network dom-mediaerror-media_err_decode dom-mediaerror-media_err_src_not_supported -->

- [x] [HTMLMediaElement.error](https://html.spec.whatwg.org/#dom-media-error) — All media elements have an associated error status, which records the last error the element encountered since its resource selection algorithm was last invoked. The error attribute, on getting, must return the MediaError object created for this last error, or null if there has not been an error.

- [x] [create a MediaError](https://html.spec.whatwg.org/#creating-a-mediaerror) — To create a MediaError, given an error code which is one of the above values, return a new MediaError object whose code is the given error code and whose message is a string containing any details the user agent is able to supply about the cause of the error condition, or the empty string if the user agent is unable to supply such details. This message string must not contain only the information already available via the supplied error code; for example, it must not simply be a translation of the code into a string format. If no additional information is available beyond that provided by the error code, the message must be set to the empty string.

- [x] [MediaError.code](https://html.spec.whatwg.org/#dom-mediaerror-code) — The code getter steps are to return this's code.

- [x] [MediaError.message](https://html.spec.whatwg.org/#dom-mediaerror-message) — The message getter steps are to return this's message.

##### [4.8.11.2 Location of the media resource](https://html.spec.whatwg.org/#location-of-the-media-resource)

Status: n/a (later: media playback)
<!-- definitions: attr-media-src attr-media-crossorigin media-provider-object assigned-media-provider-object -->

- [x] [HTMLMediaElement.crossOrigin](https://html.spec.whatwg.org/#dom-media-crossorigin) — The crossOrigin IDL attribute must reflect the crossorigin content attribute, limited to only known values.

- [x] [HTMLMediaElement.currentSrc](https://html.spec.whatwg.org/#dom-media-currentsrc) — The currentSrc IDL attribute must initially be set to the empty string. Its value is changed by the resource selection algorithm defined below.

- [x] [HTMLMediaElement.srcObject](https://html.spec.whatwg.org/#dom-media-srcobject) — The srcObject getter steps are to return this's assigned media provider object.

- [x] [The srcObject setter steps](https://html.spec.whatwg.org/#dom-media-srcobject) — The srcObject setter steps are:
  - 1. Set this's assigned media provider object to the given value.
  - 2. Invoke this's media element load algorithm.

##### [4.8.11.3 MIME types](https://html.spec.whatwg.org/#mime-types)

Status: n/a (later: media playback)
<!-- definitions: a-type-that-the-user-agent-knows-it-cannot-render dom-canplaytyperesult-nil dom-canplaytyperesult-probably dom-canplaytyperesult-maybe -->

- [x] [HTMLMediaElement.canPlayType(type)](https://html.spec.whatwg.org/#dom-navigator-canplaytype) — The canPlayType(type) method must return the empty string if type is a type that the user agent knows it cannot render or is the type "application/octet-stream"; it must return "probably" if the user agent is confident that the type represents a media resource that it can render if used with this audio or video element; and it must return "maybe" otherwise. Implementers are encouraged to return "maybe" unless the type can be confidently established as being supported or not. Generally, a user agent should never return "probably" for a type that allows the codecs parameter if that parameter is not present.

##### [4.8.11.4 Network states](https://html.spec.whatwg.org/#network-states)

Status: n/a (later: media playback)
<!-- definitions: dom-media-network_empty dom-media-network_idle dom-media-network_loading dom-media-network_no_source -->

- [x] [HTMLMediaElement.networkState](https://html.spec.whatwg.org/#dom-media-networkstate) — As media elements interact with the network, their current network activity is represented by the networkState attribute. On getting, it must return the current network state of the element, which must be one of the following values:

##### [4.8.11.5 Loading the media resource](https://html.spec.whatwg.org/#loading-the-media-resource)

Status: n/a (later: media playback)
<!-- definitions: can-autoplay-flag delaying-the-load-event-flag is-currently-stalled dedicated-media-source-failure-steps stall-timeout media-data-processing-steps-list attr-media-preload -->

- [x] [HTMLMediaElement.load()](https://html.spec.whatwg.org/#dom-media-load) — When the load() method on a media element is invoked, the user agent must run the following steps:
  - 1. Let resumptionSteps be the media element's lazy load resumption steps.
  - 2. If resumptionSteps is not null:
    - 2a. Set the media element's lazy load resumption steps to null.
    - 2b. Invoke resumptionSteps.
  - 3. Run the media element load algorithm.

- [x] [media element load algorithm](https://html.spec.whatwg.org/#media-element-load-algorithm) — The media element load algorithm consists of the following steps:
  - 1. Set this element's is currently stalled to false.
  - 2. Abort any already-running instance of the resource selection algorithm for this element.
  - 3. Let pending tasks be a list of all tasks from the media element's media element event task source in one of the task queues.
  - 4. For each task in pending tasks that would resolve pending play promises or reject pending play promises, immediately resolve or reject those promises in the order the corresponding tasks were queued.
  - 5. Remove each task in pending tasks from its task queue.
  - 6. If the media element's networkState is set to NETWORK_LOADING or NETWORK_IDLE, queue a media element task given the media element to fire an event named abort at the media element.
  - 7. If the media element's networkState is not set to NETWORK_EMPTY:
    - 7a. Queue a media element task given the media element to fire an event named emptied at the media element.
    - 7b. If a fetching process is in progress for the media element, the user agent should stop it.
    - 7c. If the media element's assigned media provider object is a MediaSource object, then detach it.
    - 7d. Forget the media element's media-resource-specific tracks.
    - 7e. If readyState is not set to HAVE_NOTHING, then set it to that state.
    - 7f. If the paused attribute is false:
      - 7f.i. Set the paused attribute to true.
      - 7f.ii. Take pending play promises and reject pending play promises with the result and an "AbortError" DOMException.
    - 7g. If seeking is true, set it to false.
    - 7h. Set the current playback position to 0. Set the official playback position to 0. If this changed the official playback position, then queue a media element task given the media element to fire an event named timeupdate at the media element.
    - 7i. Set the timeline offset to Not-a-Number (NaN).
    - 7j. Update the duration attribute to Not-a-Number (NaN).
  - 8. Set the playbackRate attribute to the value of the defaultPlaybackRate attribute.
  - 9. Set the error attribute to null and the can autoplay flag to true.
  - 10. Invoke the media element's resource selection algorithm.
  - 11.

- [x] [resource selection algorithm](https://html.spec.whatwg.org/#concept-media-load-algorithm) — The resource selection algorithm for a media element is as follows. This algorithm is always invoked as part of a task, but one of the first steps in the algorithm is to return and continue running the remaining steps in parallel. In addition, this algorithm interacts closely with the event loop me…
  - 1. Set the element's networkState attribute to the NETWORK_NO_SOURCE value.
  - 2. Set the element's show poster flag to true.
  - 3. Await a stable state, allowing the task that invoked this algorithm to continue. The synchronous section consists of all the remaining steps of this algorithm until the algorithm says the synchronous section has ended. (Steps in synchronous sections are marked with ⌛.)
  - 4. ⌛ If the media element's lazy loading attribute is in the Eager state or scripting is disabled, then set the media element's delaying-the-load-event flag to true (this delays the load event).
  - 5. ⌛ If the media element's blocked-on-parser flag is false, then populate the list of pending text tracks.
  - 6. ⌛ Let mode be null.
  - 7. ⌛ Let candidate be null.
  - 8. ⌛ If the media element's assigned media provider object is not null, then set mode to object.
  - 9. ⌛ Otherwise, if the media element has a src attribute, then set mode to attribute.
  - 10. ⌛ Otherwise, if the media element has a source element child, then set mode to children and set candidate to the first source element child in tree order.
  - 11. ⌛ Otherwise:
    - 11a. ⌛ Set the networkState to NETWORK_EMPTY.
    - 11b. ⌛ Set the element's delaying-the-load-event flag to false. This stops delaying the load event.
    - 11c. End the synchronous section and return.
  - 12. ⌛ Set the media element's networkState to NETWORK_LOADING.
  - 13. ⌛ Queue a media element task given the media element to fire an event named loadstart at the media element.
  - 14. Run the appropriate steps from the following list: If mode is object If mode is attribute Otherwise (mode is children) The dedicated media source failure steps with a list of promises promises are the following steps:
    - 14a. ⌛ Set the currentSrc attribute to the empty string.
    - 14b. End the synchronous section, continuing the remaining steps in parallel.
    - 14c. Run the resource fetch algorithm with the assigned media provider object. If that algorithm returns without aborting this one, then the load failed.
    - 14d. Failed with media provider: Reaching this step indicates that the media resource failed to load. Take pending play promises and queue a media element task given the media element to run the dedicated media source failure steps with the result.
    - 14e. Wait for the task queued by the previous step to have executed.
    - 14f. Return. The element won't attempt to load another resource until this algorithm is triggered again.
    - 14a. ⌛ If the src attribute's value is the empty string, then end the synchronous section, and jump down to the failed with attribute step below.
    - 14b. ⌛ Let urlRecord be the result of encoding-parsing a URL given the src attribute's value, relative to the media element's node document when the src attribute was last changed.
    - 14c. ⌛ If urlRecord is not failure, then set the currentSrc attribute to the result of applying the URL serializer to urlRecord.
    - 14d. End the synchronous section, continuing the remaining steps in parallel.
    - 14e. If urlRecord is not failure, then run the resource fetch algorithm with urlRecord. If that algorithm returns without aborting this one, then the load failed.
    - 14f. Failed with attribute: Reaching this step indicates that the media resource failed to load or that urlRecord is failure. Take pending play promises and queue a media element task given the media element to run the dedicated media source failure steps with the result.
    - 14g. Wait for the task queued by the previous step to have executed.
    - 14h. Return. The element won't attempt to load another resource until this algorithm is triggered again.
    - 14a. ⌛ Let pointer be a position defined by two adjacent nodes in the media element's child list, treating the start of the list (before the first child in the list, if any) and end of the list (after the last child in the list, if any) as nodes in their own right. One node is the node before pointer, and the other node is the node after pointer. Initially, let pointer be the position between the cand…
    - 14b. ⌛ Process candidate: If candidate does not have a src attribute, or if its src attribute's value is the empty string, then end the synchronous section, and jump down to the failed with elements step below.
    - 14c. ⌛ If candidate has a media attribute whose value does not match the environment, then end the synchronous section, and jump down to the failed with elements step below.
    - 14d. ⌛ Let urlRecord be the result of encoding-parsing a URL given candidate's src attribute's value, relative to candidate's node document when the src attribute was last changed.
    - 14e. ⌛ If urlRecord is failure, then end the synchronous section, and jump down to the failed with elements step below.
    - 14f. ⌛ If candidate has a type attribute whose value, when parsed as a MIME type (including any codecs described by the codecs parameter, for types that define that parameter), represents a type that the user agent knows it cannot render, then end the synchronous section, and jump down to the failed with elements step below.
    - 14g. ⌛ Set the currentSrc attribute to the result of applying the URL serializer to urlRecord.
    - 14h. End the synchronous section, continuing the remaining steps in parallel.
    - 14i. Run the resource fetch algorithm with urlRecord. If that algorithm returns without aborting this one, then the load failed.
    - 14j. Failed with elements: If candidate is not null, then queue a media element task given the media element to fire an event named error at candidate.
    - 14k. Await a stable state. The synchronous section consists of all the remaining steps of this algorithm until the algorithm says the synchronous section has ended. (Steps in synchronous sections are marked with ⌛.)
    - 14l. ⌛ Forget the media element's media-resource-specific tracks.
    - 14m. ⌛ Find next candidate: Let candidate be null.
    - 14n. ⌛ Search loop: If the node after pointer is the end of the list, then jump to the waiting step below.
    - 14o. ⌛ If the node after pointer is a source element, let candidate be that element.
    - 14p. ⌛ Advance pointer so that the node before pointer is now the node that was after pointer, and the node after pointer is the node after the node that used to be after pointer, if any.
    - 14q. ⌛ If candidate is null, jump back to the search loop step. Otherwise, jump back to the process candidate step.
    - 14r. ⌛ Waiting: Set the element's networkState attribute to the NETWORK_NO_SOURCE value.
    - 14s. ⌛ Set the element's show poster flag to true.
    - 14t. ⌛ Queue a media element task given the media element to set the element's delaying-the-load-event flag to false. This stops delaying the load event.
    - 14u. End the synchronous section, continuing the remaining steps in parallel.
    - 14v. Wait until the node after pointer is a node other than the end of the list. (This step might wait forever.)
    - 14w. Await a stable state. The synchronous section consists of all the remaining steps of this algorithm until the algorithm says the synchronous section has ended. (Steps in synchronous sections are marked with ⌛.)
    - 14x. ⌛ If the media element's lazy loading attribute is in the Eager state or scripting is disabled, then set the element's delaying-the-load-event flag back to true (this delays the load event again, in case it hasn't been fired yet).
    - 14y. ⌛ Set the networkState back to NETWORK_LOADING.
    - 14z. ⌛ Jump back to the find next candidate step above.
    - 14a. Set the error attribute to the result of creating a MediaError with MEDIA_ERR_SRC_NOT_SUPPORTED.
    - 14b. Forget the media element's media-resource-specific tracks.
    - 14c. Set the element's networkState attribute to the NETWORK_NO_SOURCE value.
    - 14d. Set the element's show poster flag to true.
    - 14e. Fire an event named error at the media element.
    - 14f. Reject pending play promises with promises and a "NotSupportedError" DOMException.
    - 14g. Set the element's delaying-the-load-event flag to false. This stops delaying the load event.

- [x] [verify a media response](https://html.spec.whatwg.org/#verify-a-media-response) — To verify a media response given a response response, a media resource resource, and "entire resource" or a (number, number or "until end") tuple byteRange:
  - 1. If response is a network error, then return false.
  - 2. If byteRange is "entire resource", then return true.
  - 3. Let internalResponse be response's unsafe response.
  - 4. If internalResponse's status is 200, then return true.
  - 5. If internalResponse's status is not 206, then return false.
  - 6. If the result of extracting content-range values from internalResponse is failure, then return false.
  - 7. Let origin be "rewritten" if internalResponse's URL is null; otherwise internalResponse's URL's origin.
  - 8. Let previousOrigin be resource's origin.
  - 9. If any of the following are true: previousOrigin is "none";; origin and previousOrigin are "rewritten"; or; origin and previousOrigin are origins, and origin is same origin with previousOrigin, then set resource's origin to origin. Otherwise, if response is CORS-cross-origin, then return false. Otherwise, set resource's origin to "multiple".
  - 10. Return true.

- [x] [resource fetch algorithm](https://html.spec.whatwg.org/#concept-media-load-resource) — The resource fetch algorithm for a media element and a given URL record or media provider object is as follows:
  - 1. If the will lazy load element steps given the media element return true:
    - 1a. Let resumptionSteps be the rest of this algorithm starting with the step labeled Let mode be remote.
    - 1b. Set the media element's lazy load resumption steps to resumptionSteps.
    - 1c. Start intersection-observing a lazy loading element for the media element.
    - 1d. Return.
  - 2. Let mode be remote.
  - 3. If the algorithm was invoked with media provider object, then set mode to local. Otherwise:
    - 3a. Let isTopLevelSelfFetch be false.
    - 3b. Let settingsObject be the media element's node document's relevant settings object.
    - 3c. Let global be the media element's node document's relevant global object.
    - 3d. If all of the following conditions are true: global is a Window object;; global's navigable is not null;; global's navigable's parent is null; and; settingsObject's creation URL equals the URL record, then set isTopLevelSelfFetch to true.
    - 3e. Let stringOrEnvironment be "top-level-self-fetch" if isTopLevelSelfFetch is true; otherwise settingsObject.
    - 3f. Let object be the result of obtaining a blob object using the URL record's blob URL entry and stringOrEnvironment.
    - 3g. If object is a media provider object, then set mode to local.
  - 4. If mode is remote, then let the current media resource be the resource given by the URL record passed to this algorithm; otherwise, let the current media resource be the resource given by the media provider object. Either way, the current media resource is now the element's media resource.
  - 5. Remove all media-resource-specific text tracks from the media element's list of pending text tracks, if any.
  - 6. Run the appropriate steps from the following list: If mode is remote Otherwise (mode is local) The resource described by the current media resource, if any, contains the media data. It is CORS-same-origin. If the current media resource is a raw data stream (e.g. from a File object), then to determine the format of the media resource, the user agent must use the rules for sniffing audio and video…
    - 6a. Optionally, run the following substeps. This is the expected behavior if the user agent intends to not attempt to fetch the resource until the user requests it explicitly (e.g. as a way to implement the preload attribute's none keyword).
      - 6a.i. Set the networkState to NETWORK_IDLE.
      - 6a.ii. Queue a media element task given the media element to fire an event named suspend at the element.
      - 6a.iii. Queue a media element task given the media element to set the element's delaying-the-load-event flag to false. This stops delaying the load event.
      - 6a.iv. Wait for the task to be run.
      - 6a.v. Wait for an implementation-defined event (e.g., the user requesting that the media element begin playback).
      - 6a.vi. Set the element's delaying-the-load-event flag back to true (this delays the load event again, in case it hasn't been fired yet).
      - 6a.vii. Set the networkState to NETWORK_LOADING.
    - 6b. Let destination be "audio" if the media element is an audio element, or "video" otherwise.
    - 6c. Let request be the result of creating a potential-CORS request given current media resource's URL record, destination, and the current state of the media element's crossorigin content attribute.
    - 6d. Set request's client to the media element's node document's relevant settings object.
    - 6e. Set request's initiator type to destination.
    - 6f. Let byteRange, which is "entire resource" or a (number, number or "until end") tuple, be the byte range required to satisfy missing data in media data. This value is implementation-defined and may rely on codec, network conditions or other heuristics. The user-agent may determine to fetch the resource in full, in which case byteRange would be "entire resource", to fetch from a byte offset until t…
    - 6g. If byteRange is not "entire resource":
      - 6g.i. If byteRange[1] is "until end", then add a range header to request given byteRange[0].
      - 6g.ii. Otherwise, add a range header to request given byteRange[0] and byteRange[1].
    - 6h. Fetch request, with processResponse set to the following steps given response response: The media element stall timeout is an implementation-defined length of time, which should be about three seconds. When a media element that is actively attempting to obtain media data has failed to receive any data for a duration equal to the media element stall timeout, the user agent must queue a media eleme…
      - 6h.i. Let global be the media element's node document's relevant global object.
      - 6h.ii. Let updateMedia be to queue a media element task given the media element to run the first appropriate steps from the media data processing steps list below. (A new task is used for this so that the work described below occurs relative to the appropriate media element event task source rather than using the networking task source.)
      - 6h.iii. Let processEndOfMedia be the following step: If the fetching process has completed without errors, including decoding the media data, and if all of the data is available to the user agent without network access, then, the user agent must move on to the final step below. This might never happen, e.g. when streaming an infinite resource such as web radio, or if the resource is longer than the user…
      - 6h.iv. If the result of verifying response given the current media resource and byteRange is false, then abort these steps.
      - 6h.v. Otherwise, incrementally read response's body given updateMedia, processEndOfMedia, an empty algorithm, and global.
      - 6h.vi. Update the media data with the contents of response's unsafe response obtained in this fashion. response can be CORS-same-origin or CORS-cross-origin; this affects whether subtitles referenced in the media data are exposed in the API and, for video elements, whether a canvas gets tainted when the video is drawn on it.
      - 6h.i. Set the element's is currently stalled to true.
      - 6h.ii. Fire an event named stalled at the element.
      - 6h.i. Set the element's is currently stalled to false.
      - 6h.ii. Fire an event named progress at the element.
    - 6a. The user agent should cancel the fetching process.
    - 6b. Abort this subalgorithm, returning to the resource selection algorithm.
    - 6a. Create an AudioTrack object to represent the audio track.
    - 6b. Update the media element's audioTracks attribute's AudioTrackList object with the new AudioTrack object.
    - 6c. Let enable be unknown.
    - 6d. If either the media resource or the URL of the current media resource indicate a particular set of audio tracks to enable, or if the user agent has information that would facilitate the selection of specific audio tracks to improve the user's experience: if this audio track is one of the ones to enable, then set enable to true, otherwise, set enable to false.
    - 6e. If enable is still unknown, then, if the media element does not yet have an enabled audio track, then set enable to true, otherwise, set enable to false.
    - 6f. If enable is true, then enable this audio track, otherwise, do not enable this audio track.
    - 6g. Fire an event named addtrack at this AudioTrackList object, using TrackEvent, with the track attribute initialized to the new AudioTrack object.
    - 6a. Create a VideoTrack object to represent the video track.
    - 6b. Update the media element's videoTracks attribute's VideoTrackList object with the new VideoTrack object.
    - 6c. Let enable be unknown.
    - 6d. If either the media resource or the URL of the current media resource indicate a particular set of video tracks to enable, or if the user agent has information that would facilitate the selection of specific video tracks to improve the user's experience: if this video track is the first such video track, then set enable to true, otherwise, set enable to false.
    - 6e. If enable is still unknown, then, if the media element does not yet have a selected video track, then set enable to true, otherwise, set enable to false.
    - 6f. If enable is true, then select this track and unselect any previously selected video tracks, otherwise, do not select this video track. If other tracks are unselected, then a change event will be fired.
    - 6g. Fire an event named addtrack at this VideoTrackList object, using TrackEvent, with the track attribute initialized to the new VideoTrack object.
    - 6a. Establish the media timeline for the purposes of the current playback position and the earliest possible position, based on the media data.
    - 6b. Update the timeline offset to the date and time that corresponds to the zero time in the media timeline established in the previous step, if any. If no explicit time and date is given by the media resource, the timeline offset must be set to Not-a-Number (NaN).
    - 6c. Set the current playback position and the official playback position to the earliest possible position.
    - 6d. Update the duration attribute with the time of the last frame of the resource, if known, on the media timeline established above. If it is not known (e.g. a stream that is in principle infinite), update the duration attribute to the value positive Infinity.
    - 6e. For video elements, set the videoWidth and videoHeight attributes, and queue a media element task given the media element to fire an event named resize at the media element.
    - 6f. Set the readyState attribute to HAVE_METADATA.
    - 6g. Let jumped be false.
    - 6h. If the media element's default playback start position is greater than zero, then seek to that time, and let jumped be true.
    - 6i. Set the media element's default playback start position to zero.
    - 6j. Let the initial playback position be 0.
    - 6k. If either the media resource or the URL of the current media resource indicate a particular start time, then set the initial playback position to that time and, if jumped is still false, seek to that time.
    - 6l. If there is no enabled audio track, then enable an audio track. This will cause a change event to be fired.
    - 6m. If there is no selected video track, then select a video track. This will cause a change event to be fired.
    - 6a. The user agent should cancel the fetching process.
    - 6b. Set the error attribute to the result of creating a MediaError with MEDIA_ERR_NETWORK.
    - 6c. Set the element's networkState attribute to the NETWORK_IDLE value.
    - 6d. Set the element's delaying-the-load-event flag to false. This stops delaying the load event.
    - 6e. Fire an event named error at the media element.
    - 6f. Abort the overall resource selection algorithm.
    - 6a. The user agent should cancel the fetching process.
    - 6b. Set the error attribute to the result of creating a MediaError with MEDIA_ERR_DECODE.
    - 6c. Set the element's networkState attribute to the NETWORK_IDLE value.
    - 6d. Set the element's delaying-the-load-event flag to false. This stops delaying the load event.
    - 6e. Fire an event named error at the media element.
    - 6f. Abort the overall resource selection algorithm.
    - 6a. The user agent should cancel the fetching process.
    - 6b. Set the error attribute to the result of creating a MediaError with MEDIA_ERR_ABORTED.
    - 6c. Fire an event named abort at the media element.
    - 6d. If the media element's readyState attribute has a value equal to HAVE_NOTHING, set the element's networkState attribute to the NETWORK_EMPTY value, set the element's show poster flag to true, and fire an event named emptied at the element. Otherwise, set the element's networkState attribute to the NETWORK_IDLE value.
    - 6e. Set the element's delaying-the-load-event flag to false. This stops delaying the load event.
    - 6f. Abort the overall resource selection algorithm.
  - 7. Final step: If the user agent ever reaches this step (which can only happen if the entire resource gets loaded and kept available): abort the overall resource selection algorithm.

- [x] [forget the media element's media-resource-specific tracks](https://html.spec.whatwg.org/#forget-the-media-element's-media-resource-specific-tracks) — When a media element is to forget the media element's media-resource-specific tracks, the user agent must remove from the media element's list of text tracks all the media-resource-specific text tracks, then empty the media element's audioTracks attribute's AudioTrackList object, then empty the media element's videoTracks attribute's VideoTrackList object. No events (in particular, no removetrack events) are fired as part of this; the error and emptied events, fired by the algorithms that invoke this one, can be used instead.

- [x] [HTMLMediaElement.preload](https://html.spec.whatwg.org/#dom-media-preload) — The preload IDL attribute must reflect the content attribute of the same name, limited to only known values.

- [x] [HTMLMediaElement.buffered](https://html.spec.whatwg.org/#dom-media-buffered) — The buffered getter steps are to return a new normalized TimeRanges object that represents the ranges of this's media resource, if any, that the user agent has buffered. User agents must accurately determine the ranges available, even for media streams where this can only be determined by tedious inspection.

##### [4.8.11.6 Offsets into the media resource](https://html.spec.whatwg.org/#offsets-into-the-media-resource)

Status: n/a (later: media playback)
<!-- definitions: media-timeline defineTimeline current-playback-position official-playback-position default-playback-start-position show-poster-flag timeline-offset attr-media-loop -->

- [x] [HTMLMediaElement.currentTime](https://html.spec.whatwg.org/#dom-media-currenttime) — The currentTime attribute must, on getting, return the media element's default playback start position, unless that is zero, in which case it must return the element's official playback position. The returned value must be expressed in seconds. On setting, if the media element's readyState is HAVE_NOTHING, then it must set the media element's default playback start position to the new value; otherwise, it must set the official playback position to the new value and then seek to the new value. The new value must be interpreted as being in seconds.

- [x] [If at any time the user agent learns that an audio or video track has ended and…](https://html.spec.whatwg.org/#earliest-possible-position) — If at any time the user agent learns that an audio or video track has ended and all media data relating to that track corresponds to parts of the media timeline that are before the earliest possible position, the user agent may queue a media element task given the media element to run these steps:
  - 1. Remove the track from the audioTracks attribute's AudioTrackList object or the videoTracks attribute's VideoTrackList object as appropriate.
  - 2. Fire an event named removetrack at the media element's aforementioned AudioTrackList or VideoTrackList object, using TrackEvent, with the track attribute initialized to the AudioTrack or VideoTrack object representing the track.

- [x] [HTMLMediaElement.duration](https://html.spec.whatwg.org/#dom-media-duration) — The duration attribute must return the time of the end of the media resource, in seconds, on the media timeline. If no media data is available, then the attributes must return the Not-a-Number (NaN) value. If the media resource is not known to be bounded (e.g. streaming radio, or a live event with no announced end time), then the attribute must return the positive Infinity value.

- [x] [HTMLMediaElement.getStartDate()](https://html.spec.whatwg.org/#dom-media-getstartdate) — The getStartDate() method must return a new Date object representing the current timeline offset.

##### [4.8.11.7 Ready states](https://html.spec.whatwg.org/#ready-states)

Status: n/a (later: media playback)
<!-- definitions: dom-media-have_nothing dom-media-have_metadata dom-media-have_current_data dom-media-have_future_data attr-media-autoplay -->

- [x] [When the ready state of a media element whose networkState is not NETWORK_EMPTY…](https://html.spec.whatwg.org/#dom-media-have_enough_data) — When the ready state of a media element whose networkState is not NETWORK_EMPTY changes, the user agent must follow the steps given below:
  - 1. Apply the first applicable set of substeps from the following list: If the previous ready state was HAVE_NOTHING, and the new ready state is HAVE_METADATA Queue a media element task given the media element to fire an event named loadedmetadata at the element. If the previous ready state was HAVE_METADATA and the new ready state is HAVE_CURRENT_DATA or greater If this is the first time this occurs…
    - 1a. Set the paused attribute to false.
    - 1b. If the element's show poster flag is true, set it to false and run the time marches on steps.
    - 1c. Queue a media element task given the element to fire an event named play at the element.
    - 1d. Notify about playing for the element.
    - 1a. Run the internal pause steps and set the can autoplay flag to true.
    - 1b. Queue a media element task given the element to fire an event named pause at the element.

- [x] [HTMLMediaElement.readyState](https://html.spec.whatwg.org/#dom-media-readystate) — The readyState IDL attribute must, on getting, return the value described above that describes the current ready state of the media element.

##### [4.8.11.8 Playing the media resource](https://html.spec.whatwg.org/#playing-the-media-resource)

Status: n/a (later: media playback)
<!-- definitions: list-of-pending-play-promises -->

- [x] [HTMLMediaElement.paused](https://html.spec.whatwg.org/#dom-media-paused) — The paused attribute represents whether the media element is paused or not. The attribute must initially be true.

- [x] [blocked media element](https://html.spec.whatwg.org/#blocked-media-element) — A media element is a blocked media element if its readyState attribute is in the HAVE_NOTHING state, the HAVE_METADATA state, or the HAVE_CURRENT_DATA state, or if the element has paused for user interaction or paused for in-band content.

- [x] [potentially playing (media element)](https://html.spec.whatwg.org/#potentially-playing) — A media element is said to be potentially playing when its paused attribute is false, the element has not ended playback, playback has not stopped due to errors, and the element is not a blocked media element.

- [x] [eligible for autoplay](https://html.spec.whatwg.org/#eligible-for-autoplay) — A media element is said to be eligible for autoplay when all of the following are true:

- [x] [allowed to play](https://html.spec.whatwg.org/#allowed-to-play) — A media element is said to be allowed to play if the user agent and the system allow media playback in the current context.

- [x] [ended playback](https://html.spec.whatwg.org/#ended-playback) — A media element is said to have ended playback when:

- [x] [HTMLMediaElement.ended](https://html.spec.whatwg.org/#dom-media-ended) — The ended attribute must return true if, the last time the event loop reached step 1, the media element had ended playback and the direction of playback was forwards, and false otherwise.

- [x] [stopped due to errors](https://html.spec.whatwg.org/#stopped-due-to-errors) — A media element is said to have stopped due to errors when the element's readyState attribute is HAVE_METADATA or greater, and the user agent encounters a non-fatal error during the processing of the media data, and due to that error, is not able to play the content at the current playback position.

- [x] [paused for user interaction](https://html.spec.whatwg.org/#paused-for-user-interaction) — A media element is said to have paused for user interaction when its paused attribute is false, the readyState attribute is either HAVE_FUTURE_DATA or HAVE_ENOUGH_DATA, and the user agent has reached a point in the media resource where the user has to make a selection for the resource to continue.

- [x] [paused for in-band content](https://html.spec.whatwg.org/#paused-for-in-band-content) — A media element is said to have paused for in-band content when its paused attribute is false, the readyState attribute is either HAVE_FUTURE_DATA or HAVE_ENOUGH_DATA, and the user agent has suspended playback of the media resource in order to play content that is temporally anchored to the media resource and has a nonzero length, or to play content that is temporally anchored to a segment of the media resource but has a length longer than that segment.

- [x] [When the current playback position reaches the end of the media resource when t…](https://html.spec.whatwg.org/#paused-for-in-band-content) — When the current playback position reaches the end of the media resource when the direction of playback is forwards, then the user agent must follow these steps:
  - 1. If the media element has a loop attribute specified, then seek to the earliest possible position of the media resource and return.
  - 2. As defined above, the ended IDL attribute starts returning true once the event loop returns to step 1.
  - 3. Queue a media element task given the media element and the following steps:
    - 3a. Fire an event named timeupdate at the media element.
    - 3b. If the media element has ended playback, the direction of playback is forwards, and paused is false:
      - 3b.i. Set the paused attribute to true.
      - 3b.ii. Fire an event named pause at the media element.
      - 3b.iii. Take pending play promises and reject pending play promises with the result and an "AbortError" DOMException.
    - 3c. Fire an event named ended at the media element.

- [x] [HTMLMediaElement.defaultPlaybackRate](https://html.spec.whatwg.org/#dom-media-defaultplaybackrate) — The defaultPlaybackRate attribute gives the desired speed at which the media resource is to play, as a multiple of its intrinsic speed. The attribute is mutable: on getting it must return the last value it was set to, or 1.0 if it hasn't yet been set; on setting the attribute must be set to the new value.

- [x] [HTMLMediaElement.playbackRate](https://html.spec.whatwg.org/#dom-media-playbackrate) — The playbackRate attribute gives the effective playback rate, which is the speed at which the media resource plays, as a multiple of its intrinsic speed. If it is not equal to the defaultPlaybackRate, then the implication is that the user is using a feature such as fast forward or slow motion playb…
  - 1. If the given value is not supported by the user agent, then throw a "NotSupportedError" DOMException.
  - 2. Set playbackRate to the new value, and if the element is potentially playing, change the playback speed.

- [x] [HTMLMediaElement.preservesPitch](https://html.spec.whatwg.org/#dom-media-preservespitch) — The preservesPitch getter steps are to return true if a pitch-preserving algorithm is in effect during playback. The setter steps are to correspondingly switch the pitch-preserving algorithm on or off, without any perceivable gaps or muting of playback. By default, such a pitch-preserving algorithm must be in effect (i.e., the getter will initially return true).

- [x] [HTMLMediaElement.played](https://html.spec.whatwg.org/#dom-media-played) — The played getter steps are to return a new normalized TimeRanges object that represents the ranges of points on the media timeline of this's media resource reached through the usual monotonic increase of this's current playback position during normal playback, if any.

- [x] [take pending play promises](https://html.spec.whatwg.org/#take-pending-play-promises) — To take pending play promises for a media element, the user agent must run the following steps:
  - 1. Let promises be an empty list of promises.
  - 2. Copy the media element's list of pending play promises to promises.
  - 3. Clear the media element's list of pending play promises.
  - 4. Return promises.

- [x] [resolve pending play promises](https://html.spec.whatwg.org/#resolve-pending-play-promises) — To resolve pending play promises for a media element with a list of promises promises, the user agent must resolve each promise in promises with undefined.

- [x] [reject pending play promises](https://html.spec.whatwg.org/#reject-pending-play-promises) — To reject pending play promises for a media element with a list of promises promises and an exception name error, the user agent must reject each promise in promises with error.

- [x] [notify about playing](https://html.spec.whatwg.org/#notify-about-playing) — To notify about playing for a media element, the user agent must run the following steps:
  - 1. Take pending play promises and let promises be the result.
  - 2. Queue a media element task given the element and the following steps:
    - 2a. Fire an event named playing at the element.
    - 2b. Resolve pending play promises with promises.

- [x] [HTMLMediaElement.play()](https://html.spec.whatwg.org/#dom-media-play) — When the play() method on a media element is invoked, the user agent must run the following steps:
  - 1. If the media element is not allowed to play, then return a promise rejected with a "NotAllowedError" DOMException.
  - 2. If the media element's error attribute is not null and its code is MEDIA_ERR_SRC_NOT_SUPPORTED, then return a promise rejected with a "NotSupportedError" DOMException.
  - 3. Let resumptionSteps be the media element's lazy load resumption steps.
  - 4. If resumptionSteps is not null:
    - 4a. Set the media element's lazy load resumption steps to null.
    - 4b. Invoke resumptionSteps.
  - 5. Let promise be a new promise and append promise to the list of pending play promises.
  - 6. Run the internal play steps for the media element.
  - 7. Return promise.

- [x] [internal play steps](https://html.spec.whatwg.org/#internal-play-steps) — The internal play steps for a media element are as follows:
  - 1. If the media element's networkState attribute has the value NETWORK_EMPTY, invoke the media element's resource selection algorithm.
  - 2. If the playback has ended and the direction of playback is forwards, seek to the earliest possible position of the media resource.
  - 3. If the media element's paused attribute is true:
    - 3a. Change the value of paused to false.
    - 3b. If the show poster flag is true, set the element's show poster flag to false and run the time marches on steps.
    - 3c. Queue a media element task given the media element to fire an event named play at the element.
    - 3d. If the media element's readyState attribute has the value HAVE_NOTHING, HAVE_METADATA, or HAVE_CURRENT_DATA, queue a media element task given the media element to fire an event named waiting at the element. Otherwise, the media element's readyState attribute has the value HAVE_FUTURE_DATA or HAVE_ENOUGH_DATA: notify about playing for the element.
  - 4. Otherwise, if the media element's readyState attribute has the value HAVE_FUTURE_DATA or HAVE_ENOUGH_DATA, take pending play promises and queue a media element task given the media element to resolve pending play promises with the result.
  - 5. Set the media element's can autoplay flag to false.

- [x] [HTMLMediaElement.pause()](https://html.spec.whatwg.org/#dom-media-pause) — When the pause() method is invoked, and when the user agent is required to pause the media element, the user agent must run the following steps:
  - 1. If the media element's networkState attribute has the value NETWORK_EMPTY, invoke the media element's resource selection algorithm.
  - 2. Run the internal pause steps for the media element.

- [x] [internal pause steps](https://html.spec.whatwg.org/#internal-pause-steps) — The internal pause steps for a media element are as follows:
  - 1. Set the media element's can autoplay flag to false.
  - 2. If the media element's paused attribute is false, run the following steps:
    - 2a. Change the value of paused to true.
    - 2b. Take pending play promises and let promises be the result.
    - 2c. Queue a media element task given the media element and the following steps:
      - 2c.i. Fire an event named timeupdate at the element.
      - 2c.ii. Fire an event named pause at the element.
      - 2c.iii. Reject pending play promises with promises and an "AbortError" DOMException.
    - 2d. Set the official playback position to the current playback position.

- [x] [direction of playback](https://html.spec.whatwg.org/#direction-of-playback) — If the element's playbackRate is positive or zero, then the direction of playback is forwards. Otherwise, it is backwards.

- [x] [list of newly introduced cues](https://html.spec.whatwg.org/#list-of-newly-introduced-cues) — Each media element has a list of newly introduced cues, which must be initially empty. Whenever a text track cue is added to the list of cues of a text track that is in the list of text tracks for a media element, that cue must be added to the media element's list of newly introduced cues. Whenever a text track is added to the list of text tracks for a media element, all of the cues in that text track's list of cues must be added to the media element's list of newly introduced cues. When a media element's list of newly introduced cues has new cues added while the media element's show poster flag is not set, then the user agent must run the time marches on steps.

- [x] [time marches on](https://html.spec.whatwg.org/#time-marches-on) — The time marches on steps are as follows:
  - 1. Let current cues be a list of cues, initialized to contain all the cues of all the hidden or showing text tracks of the media element (not the disabled ones) whose start times are less than or equal to the current playback position and whose end times are greater than the current playback position.
  - 2. Let other cues be a list of cues, initialized to contain all the cues of hidden and showing text tracks of the media element that are not present in current cues.
  - 3. Let last time be the current playback position at the time this algorithm was last run for this media element, if this is not the first time it has run.
  - 4. If the current playback position has, since the last time this algorithm was run, only changed through its usual monotonic increase during normal playback, then let missed cues be the list of cues in other cues whose start times are greater than or equal to last time and whose end times are less than or equal to the current playback position. Otherwise, let missed cues be an empty list.
  - 5. Remove all the cues in missed cues that are also in the media element's list of newly introduced cues, and then empty the element's list of newly introduced cues.
  - 6. If the time was reached through the usual monotonic increase of the current playback position during normal playback, and if the user agent has not fired a timeupdate event at the element in the past 15 to 250ms and is not still running event handlers for such an event, then the user agent must queue a media element task given the media element to fire an event named timeupdate at the element. (I…
  - 7. If all of the cues in current cues have their text track cue active flag set, none of the cues in other cues have their text track cue active flag set, and missed cues is empty, then return.
  - 8. If the time was reached through the usual monotonic increase of the current playback position during normal playback, and there are cues in other cues that have their text track cue pause-on-exit flag set and that either have their text track cue active flag set or are also in missed cues, then immediately pause the media element.
  - 9. Let events be a list of tasks, initially empty. Each task in this list will be associated with a text track, a text track cue, and a time, which are used to sort the list before the tasks are queued. Let affected tracks be a list of text tracks, initially empty. When the steps below say to prepare an event named event for a text track cue target with a time time, the user agent must run these ste…
    - 9a. Let track be the text track with which the text track cue target is associated.
    - 9b. Create a task to fire an event named event at target.
    - 9c. Add the newly created task to events, associated with the time time, the text track track, and the text track cue target.
    - 9d. Add track to affected tracks.
  - 10. For each text track cue in missed cues, prepare an event named enter for the TextTrackCue object with the text track cue start time.
  - 11. For each text track cue in other cues that either has its text track cue active flag set or is in missed cues, prepare an event named exit for the TextTrackCue object with the later of the text track cue end time and the text track cue start time.
  - 12. For each text track cue in current cues that does not have its text track cue active flag set, prepare an event named enter for the TextTrackCue object with the text track cue start time.
  - 13. Sort the tasks in events in ascending time order (tasks with earlier times first). Further sort tasks in events that have the same time by the relative text track cue order of the text track cues associated with these tasks. Finally, sort tasks in events that have the same time and same text track cue order by placing tasks that fire enter events before those that fire exit events.
  - 14. Queue a media element task given the media element for each task in events, in list order.
  - 15. Sort affected tracks in the same order as the text tracks appear in the media element's list of text tracks, and remove duplicates.
  - 16. For each text track in affected tracks, in the list order, queue a media element task given the media element to fire an event named cuechange at the TextTrack object, and, if the text track has a corresponding track element, to then fire an event named cuechange at the track element as well.
  - 17. Set the text track cue active flag of all the cues in the current cues, and unset the text track cue active flag of all the cues in the other cues.
  - 18. Run the rules for updating the text track rendering of each of the text tracks in affected tracks that are showing, providing the text track's text track language as the fallback language if it is not the empty string. For example, for text tracks based on WebVTT, the rules for updating the display of WebVTT text tracks. [WEBVTT]

- [x] [When a media element is removed from a Document, the user agent must run the fo…](https://html.spec.whatwg.org/#prepare-an-event) — When a media element is removed from a Document, the user agent must run the following steps:
  - 1. Await a stable state, allowing the task that removed the media element from the Document to continue. The synchronous section consists of all the remaining steps of this algorithm. (Steps in the synchronous section are marked with ⌛.)
  - 2. ⌛ If the media element is in a document, return.
  - 3. ⌛ Run the internal pause steps for the media element.

##### [4.8.11.9 Seeking](https://html.spec.whatwg.org/#seeking)

Status: n/a (later: media playback)

- [x] [HTMLMediaElement.seeking](https://html.spec.whatwg.org/#dom-media-seeking) — The seeking attribute must initially have the value false.

- [x] [HTMLMediaElement.fastSeek(time)](https://html.spec.whatwg.org/#dom-media-fastseek) — The fastSeek(time) method must seek to the time given by time, with the approximate-for-speed flag set.

- [x] [seek](https://html.spec.whatwg.org/#dom-media-seek) — When the user agent is required to seek to a particular new playback position in the media resource, optionally with the approximate-for-speed flag set, it means that the user agent must run the following steps. This algorithm interacts closely with the event loop mechanism; in particular, it has a…
  - 1. Set the media element's show poster flag to false.
  - 2. If the media element's readyState is HAVE_NOTHING, return.
  - 3. If the element's seeking IDL attribute is true, then another instance of this algorithm is already running. Abort that other instance of the algorithm without waiting for the step that it is running to complete.
  - 4. Set the seeking IDL attribute to true.
  - 5. If the seek was in response to a DOM method call or setting of an IDL attribute, then continue the script. The remainder of these steps must be run in parallel. With the exception of the steps marked with ⌛, they could be aborted at any time by another instance of this algorithm being invoked.
  - 6. If the new playback position is later than the end of the media resource, then let it be the end of the media resource instead.
  - 7. If the new playback position is less than the earliest possible position, let it be that position instead.
  - 8. If the (possibly now changed) new playback position is not in one of the ranges given in the seekable attribute, then let it be the position in one of the ranges given in the seekable attribute that is the nearest to the new playback position. If two positions both satisfy that constraint (i.e. the new playback position is exactly in the middle between two ranges in the seekable attribute), then…
  - 9. If the approximate-for-speed flag is set, adjust the new playback position to a value that will allow for playback to resume promptly. If new playback position before this step is before current playback position, then the adjusted new playback position must also be before the current playback position. Similarly, if the new playback position before this step is after current playback position, t…
  - 10. Queue a media element task given the media element to fire an event named seeking at the element.
  - 11. Set the current playback position to the new playback position.
  - 12. Wait until the user agent has established whether or not the media data for the new playback position is available, and, if it is, until it has decoded enough data to play back that position.
  - 13. Await a stable state. The synchronous section consists of all the remaining steps of this algorithm. (Steps in the synchronous section are marked with ⌛.)
  - 14. ⌛ Set the seeking IDL attribute to false.
  - 15. ⌛ Run the time marches on steps.
  - 16. ⌛ Queue a media element task given the media element to fire an event named timeupdate at the element.
  - 17. ⌛ Queue a media element task given the media element to fire an event named seeked at the element.

- [x] [HTMLMediaElement.seekable](https://html.spec.whatwg.org/#dom-media-seekable) — The seekable getter steps are to return a new normalized TimeRanges object that represents the ranges of this's media resource, if any, that the user agent is able to seek to.

##### [4.8.11.10 Media resources with multiple media tracks](https://html.spec.whatwg.org/#media-resources-with-multiple-media-tracks)

Status: n/a (later: media playback)

- [x] [HTMLMediaElement.audioTracks](https://html.spec.whatwg.org/#dom-media-audiotracks) — The audioTracks attribute of a media element must return a live AudioTrackList object representing the audio tracks available in the media element's media resource.

- [x] [HTMLMediaElement.videoTracks](https://html.spec.whatwg.org/#dom-media-videotracks) — The videoTracks attribute of a media element must return a live VideoTrackList object representing the video tracks available in the media element's media resource.

###### [4.8.11.10.1 AudioTrackList and VideoTrackList objects](https://html.spec.whatwg.org/#audiotracklist-and-videotracklist-objects)

Status: n/a (later: media playback)
<!-- definitions: dom-videotracklist-length dom-videotracklist-gettrackbyid dom-videotrack-id dom-videotrack-kind dom-videotrack-label dom-videotrack-language -->

- [x] [AudioTrackList.length](https://html.spec.whatwg.org/#dom-audiotracklist-length) — The AudioTrackList length and VideoTrackList length attribute getters must return the number of tracks represented by their objects at the time of getting.

- [x] [AudioTrackList.getTrackById(id)](https://html.spec.whatwg.org/#dom-audiotracklist-gettrackbyid) — The AudioTrackList getTrackById(id) and VideoTrackList getTrackById(id) methods must return the first AudioTrack or VideoTrack object (respectively) in the AudioTrackList or VideoTrackList object (respectively) whose identifier is equal to the value of the id argument (in the natural order of the list, as defined above). When no tracks match the given argument, the methods must return null.

- [x] [AudioTrack.id](https://html.spec.whatwg.org/#dom-audiotrack-id) — The AudioTrack id and VideoTrack id attributes must return the identifier of the track, if it has one, or the empty string otherwise. If the media resource is in a format that supports media fragment syntax, the identifier returned for a particular track must be the same identifier that would enable the track if used as the name of a track in the track dimension of such a fragment. [INBAND]

- [x] [AudioTrack.kind](https://html.spec.whatwg.org/#dom-audiotrack-kind) — The AudioTrack kind and VideoTrack kind attributes must return the category of the track, if it has one, or the empty string otherwise.

- [x] [AudioTrack.label](https://html.spec.whatwg.org/#dom-audiotrack-label) — The AudioTrack label and VideoTrack label attributes must return the label of the track, if it has one, or the empty string otherwise. [INBAND]

- [x] [AudioTrack.language](https://html.spec.whatwg.org/#dom-audiotrack-language) — The AudioTrack language and VideoTrack language attributes must return the BCP 47 language tag of the language of the track, if it has one, or the empty string otherwise. If the user agent is not able to express that language as a BCP 47 language tag (for example because the language information in the media resource's format is a free-form string without a defined interpretation), then the method must return the empty string, as if the track had no language. [INBAND]

- [x] [AudioTrack.enabled](https://html.spec.whatwg.org/#dom-audiotrack-enabled) — The AudioTrack enabled attribute, on getting, must return true if the track is currently enabled, and false otherwise. On setting, it must enable the track if the new value is true, and disable it otherwise. (If the track is no longer in an AudioTrackList object, then the track being enabled or disabled has no effect beyond changing the value of the attribute on the AudioTrack object.)

- [x] [VideoTrackList.selectedIndex](https://html.spec.whatwg.org/#dom-videotracklist-selectedindex) — The VideoTrackList selectedIndex attribute must return the index of the currently selected track, if any. If the VideoTrackList object does not currently represent any tracks, or if none of the tracks are selected, it must instead return −1.

- [x] [VideoTrack.selected](https://html.spec.whatwg.org/#dom-videotrack-selected) — The VideoTrack selected attribute, on getting, must return true if the track is currently selected, and false otherwise. On setting, it must select the track if the new value is true, and unselect it otherwise. If the track is in a VideoTrackList, then all the other VideoTrack objects in that list must be unselected. (If the track is no longer in a VideoTrackList object, then the track being selected or unselected has no effect beyond changing the value of the attribute on the VideoTrack object.)

###### [4.8.11.10.2 Selecting specific audio and video tracks declaratively](https://html.spec.whatwg.org/#selecting-specific-audio-and-video-tracks-declaratively)

Status: n/a (later: media playback)

##### [4.8.11.11 Timed text tracks](https://html.spec.whatwg.org/#timed-text-tracks)

Status: n/a (later: media playback)

###### [4.8.11.11.1 Text track model](https://html.spec.whatwg.org/#text-track-model)

Status: n/a (later: media playback)
<!-- definitions: list-of-text-tracks text-track-kind dom-texttrack-kind-subtitles dom-texttrack-kind-captions dom-texttrack-kind-descriptions dom-texttrack-kind-chapters dom-texttrack-kind-metadata text-track-label text-track-in-band-metadata-track-dispatch-type text-track-language text-track-identifier text-track-readiness-state text-track-not-loaded text-track-loading text-track-loaded text-track-failed-to-load text-track-mode text-track-disabled text-track-hidden text-track-showing text-track-list-of-cues rules-for-updating-the-text-track-rendering list-of-pending-text-tracks blocked-on-parser did-perform-automatic-track-selection text-track-cue text-track-cue-identifier text-track-cue-start-time text-track-cue-end-time text-track-cue-pause-on-exit-flag text-track-cue-active-flag text-track-cue-display-state text-track-cue-order -->

- [x] [text tracks](https://html.spec.whatwg.org/#text-track) — A media element can have a group of associated text tracks, known as the media element's list of text tracks. The text tracks are sorted as follows:
  - 1. The text tracks corresponding to track element children of the media element, in tree order.
  - 2. Any text tracks added using the addTextTrack() method, in the order they were added, oldest first.
  - 3. Any media-resource-specific text tracks (text tracks corresponding to data in the media resource), in the order defined by the media resource's format specification.

- [x] [populate the list of pending text tracks](https://html.spec.whatwg.org/#populate-the-list-of-pending-text-tracks) — When the user agent is required to populate the list of pending text tracks of a media element, the user agent must add to the element's list of pending text tracks each text track in the element's list of text tracks whose text track mode is not disabled and whose text track readiness state is loading.

- [x] [ready](https://html.spec.whatwg.org/#the-text-tracks-are-ready) — The text tracks of a media element are ready when both the element's list of pending text tracks is empty and the element's blocked-on-parser flag is false.

- [x] [Whenever a text track that is in a media element's list of text tracks has its…](https://html.spec.whatwg.org/#pending-text-track-change-notification-flag) — Whenever a text track that is in a media element's list of text tracks has its text track mode change value, the user agent must run the following steps for the media element:
  - 1. If the media element's pending text track change notification flag is set, return.
  - 2. Set the media element's pending text track change notification flag.
  - 3. Queue a media element task given the media element to run these steps:
    - 3a. Unset the media element's pending text track change notification flag.
    - 3b. Fire an event named change at the media element's textTracks attribute's TextTrackList object.
  - 4. If the media element's show poster flag is not set, run the time marches on steps.

- [x] [unbounded text track cue](https://html.spec.whatwg.org/#unbounded-text-track-cue) — An unbounded text track cue is a text track cue with a text track cue end time set to positive Infinity. An active unbounded text track cue cannot become inactive through the usual monotonic increase of the current playback position during normal playback (e.g. a metadata cue for a chapter in a live event with no announced end time.)

###### [4.8.11.11.2 Sourcing in-band text tracks](https://html.spec.whatwg.org/#sourcing-in-band-text-tracks)

Status: n/a (later: media playback)
<!-- definitions: media-resource-specific-text-track -->

- [x] [steps to expose a media-resource-specific text track](https://html.spec.whatwg.org/#steps-to-expose-a-media-resource-specific-text-track) — When a media resource contains data that the user agent recognizes and supports as being equivalent to a text track, the user agent runs the steps to expose a media-resource-specific text track with the relevant data, as follows.
  - 1. Associate the relevant data with a new text track. The text track is a media-resource-specific text track.
  - 2. Set the new text track's kind, label, language, and identifier based on the semantics of the relevant data, as defined by the relevant specification. If there is no label in that data, then the label must be set to the empty string. If the media resource is in a format that supports media fragment syntax, then the identifier must be the same identifier that would enable the track if used as the n…
  - 3. Associate the text track list of cues with the rules for updating the text track rendering appropriate for the format in question.
  - 4. If the new text track's kind is chapters or metadata, then set the text track in-band metadata track dispatch type as follows, based on the type of the media resource: If the media resource is an Ogg fileThe text track in-band metadata track dispatch type must be set to the value of the Name header field. [OGGSKELETONHEADERS]If the media resource is a WebM fileThe text track in-band metadata trac…
  - 5. Populate the new text track's list of cues with the cues parsed so far, following the guidelines for exposing cues, and begin updating it dynamically as necessary.
  - 6. Set the new text track's readiness state to loaded.
  - 7. Set the new text track's mode to the mode consistent with the user's preferences and the requirements of the relevant specification for the data.
  - 8. Add the new text track to the media element's list of text tracks.
  - 9. Fire an event named addtrack at the media element's textTracks attribute's TextTrackList object, using TrackEvent, with the track attribute initialized to the text track.

###### [4.8.11.11.3 Sourcing out-of-band text tracks](https://html.spec.whatwg.org/#sourcing-out-of-band-text-tracks)

Status: n/a (later: media playback)

- [x] [When a text track corresponding to a track element is added to a media element'…](https://html.spec.whatwg.org/#sourcing-out-of-band-text-tracks) — When a text track corresponding to a track element is added to a media element's list of text tracks, the user agent must queue a media element task given the media element to run the following steps for the media element:
  - 1. If the element's blocked-on-parser flag is true, then return.
  - 2. If the element's did-perform-automatic-track-selection flag is true, then return.
  - 3. Honor user preferences for automatic text track selection for this element.

- [x] [honor user preferences for automatic text track selection](https://html.spec.whatwg.org/#honor-user-preferences-for-automatic-text-track-selection) — When the user agent is required to honor user preferences for automatic text track selection for a media element, the user agent must run the following steps:
  - 1. Perform automatic text track selection for subtitles and captions.
  - 2. Perform automatic text track selection for descriptions.
  - 3. If there are any text tracks in the media element's list of text tracks whose text track kind is chapters or metadata that correspond to track elements with a default attribute set whose text track mode is set to disabled, then set the text track mode of all such tracks to hidden.
  - 4. Set the element's did-perform-automatic-track-selection flag to true.

- [x] [perform automatic text track selection](https://html.spec.whatwg.org/#perform-automatic-text-track-selection) — When the steps above say to perform automatic text track selection for one or more text track kinds, it means to run the following steps:
  - 1. Let candidates be a list consisting of the text tracks in the media element's list of text tracks whose text track kind is one of the kinds that were passed to the algorithm, if any, in the order given in the list of text tracks.
  - 2. If candidates is empty, then return.
  - 3. If any of the text tracks in candidates have a text track mode set to showing, return.
  - 4. If the user has expressed an interest in having a track from candidates enabled based on its text track kind, text track language, and text track label, then set its text track mode to showing. Otherwise, if there are any text tracks in candidates that correspond to track elements with a default attribute set whose text track mode is set to disabled, then set the text track mode of the first such…

- [x] [start the track processing model](https://html.spec.whatwg.org/#start-the-track-processing-model) — When a user agent is to start the track processing model for a text track and its track element, it must run the following algorithm. This algorithm interacts closely with the event loop mechanism; in particular, it has a synchronous section (which is triggered as part of the event loop algorithm).…
  - 1. If another occurrence of this algorithm is already running for this text track and its track element, return, letting that other algorithm take care of this element.
  - 2. If the text track's text track mode is not set to one of hidden or showing, then return.
  - 3. If the text track's track element does not have a media element as a parent, return.
  - 4. Run the remainder of these steps in parallel, allowing whatever caused these steps to run to continue.
  - 5. Top: Await a stable state. The synchronous section consists of the following steps. (The steps in the synchronous section are marked with ⌛.)
  - 6. ⌛ Set the text track readiness state to loading.
  - 7. ⌛ Let URL be the track URL of the track element.
  - 8. ⌛ If the track element's parent is a media element, then let corsAttributeState be the state of the parent media element's crossorigin content attribute. Otherwise, let corsAttributeState be No CORS.
  - 9. End the synchronous section, continuing the remaining steps in parallel.
  - 10. If URL is not the empty string: The tasks queued by the fetching algorithm on the networking task source to process the data as it is being fetched must determine the type of the resource. If the type of the resource is not a supported text track format, the load will fail, as described below. Otherwise, the resource's data must be passed to the appropriate parser (e.g., the WebVTT parser) as it…
    - 10a. Let request be the result of creating a potential-CORS request given URL, "track", and corsAttributeState, and with the same-origin fallback flag set.
    - 10b. Set request's client to the track element's node document's relevant settings object.
    - 10c. Set request's initiator type to "track".
    - 10d. Fetch request.
  - 11. Wait until the text track readiness state is no longer set to loading.
  - 12. Wait until the track URL is no longer equal to URL, at the same time as the text track mode is set to hidden or showing.
  - 13. Jump to the step labeled top.

###### [4.8.11.11.4 Guidelines for exposing cues in various formats as text track cues](https://html.spec.whatwg.org/#guidelines-for-exposing-cues-in-various-formats-as-text-track-cues)

Status: n/a (later: media playback)

###### [4.8.11.11.5 Text track API](https://html.spec.whatwg.org/#text-track-api)

Status: n/a (later: media playback)
<!-- definitions: dom-texttrack-disabled dom-texttrack-hidden dom-texttrack-showing -->

- [x] [HTMLMediaElement.textTracks](https://html.spec.whatwg.org/#dom-media-texttracks) — The textTracks attribute of media elements must return a TextTrackList object representing the text tracks in the media element's list of text tracks, in the same order as in the list of text tracks.

- [x] [TextTrackList.length](https://html.spec.whatwg.org/#dom-texttracklist-length) — The length attribute of a TextTrackList object must return the number of text tracks in the list represented by the TextTrackList object.

- [x] [TextTrackList.getTrackById(id)](https://html.spec.whatwg.org/#dom-texttracklist-gettrackbyid) — The getTrackById(id) method must return the first TextTrack in the TextTrackList object whose id IDL attribute would return a value equal to the value of the id argument. When no tracks match the given argument, the method must return null.

- [x] [HTMLMediaElement.addTextTrack(kind, label, language)](https://html.spec.whatwg.org/#dom-media-addtexttrack) — The addTextTrack(kind, label, language) method of media elements, when invoked, must run the following steps:
  - 1. Create a new text track, and set its text track kind to kind, its text track label to label, its text track language to language, its text track readiness state to the text track loaded state, its text track mode to the text track hidden mode, and its text track list of cues to an empty list. Initially, the text track list of cues is not associated with any rules for updating the text track rende…
  - 2. Add the new text track to the media element's list of text tracks.
  - 3. Queue a media element task given the media element to fire an event named addtrack at the media element's textTracks attribute's TextTrackList object, using TrackEvent, with the track attribute initialized to the new text track.
  - 4. Return the new text track.

- [x] [TextTrack.kind](https://html.spec.whatwg.org/#dom-texttrack-kind) — The kind getter steps are to return this's kind.

- [x] [TextTrack.label](https://html.spec.whatwg.org/#dom-texttrack-label) — The label getter steps are to return this's label.

- [x] [TextTrack.language](https://html.spec.whatwg.org/#dom-texttrack-language) — The language getter steps are to return this's language.

- [x] [TextTrack.id](https://html.spec.whatwg.org/#dom-texttrack-id) — The id getter steps are to return this's identifier.

- [x] [TextTrack.inBandMetadataTrackDispatchType](https://html.spec.whatwg.org/#dom-texttrack-inbandmetadatatrackdispatchtype) — The inBandMetadataTrackDispatchType getter steps are to return this's in-band metadata track dispatch type.

- [x] [TextTrack.mode](https://html.spec.whatwg.org/#dom-texttrack-mode) — The mode getter steps are to return the string switching on this's mode:
  - • The text track disabled mode: "disabled"
  - • The text track hidden mode: "hidden"
  - • The text track showing mode: "showing"

- [x] [The mode setter steps](https://html.spec.whatwg.org/#dom-texttrack-mode) — The mode setter steps are to set this's mode to the mode switching on the given value:

- [x] [TextTrack.cues](https://html.spec.whatwg.org/#dom-texttrack-cues) — The cues getter steps are:
  - 1. If this's mode is the text track disabled mode, then return null.
  - 2. Return a live TextTrackCueList object that represents the subset of this's text track list of cues whose end times occur at or after the earliest possible position when the script started, in text track cue order.

- [x] [earliest possible position when the script started](https://html.spec.whatwg.org/#earliest-possible-position-when-the-script-started) — The earliest possible position when the script started is whatever the earliest possible position was the last time the event loop reached step 1.

- [x] [TextTrack.activeCues](https://html.spec.whatwg.org/#dom-texttrack-activecues) — The activeCues getter steps are:
  - 1. If this's mode is the text track disabled mode, then return null.
  - 2. Return a live TextTrackCueList object that represents the subset of this's text track list of cues whose active flag was set when the script started, in text track cue order.

- [x] [active flag was set when the script started](https://html.spec.whatwg.org/#active-flag-was-set-when-the-script-started) — A text track cue's active flag was set when the script started if its text track cue active flag was set the last time the event loop reached step 1.

- [x] [TextTrack.addCue(cue)](https://html.spec.whatwg.org/#dom-texttrack-addcue) — The addCue(cue) method steps are:
  - 1. Let list be this's text track list of cues.
  - 2. If list does not yet have any associated rules for updating the text track rendering, then associate list with the rules for updating the text track rendering appropriate to cue.
  - 3. If list's associated rules for updating the text track rendering are not the same rules for updating the text track rendering as appropriate for cue, then throw an "InvalidStateError" DOMException.
  - 4. If the given cue is in a text track list of cues, then remove cue from that text track list of cues.
  - 5. Add cue to list.

- [x] [TextTrack.removeCue(cue)](https://html.spec.whatwg.org/#dom-texttrack-removecue) — The removeCue(cue) method steps are:
  - 1. If the given cue is not in this's text track list of cues, then throw a "NotFoundError" DOMException.
  - 2. Remove cue from this's text track list of cues.

- [x] [TextTrackCueList.length](https://html.spec.whatwg.org/#dom-texttrackcuelist-length) — The length attribute must return the number of cues in the list represented by the TextTrackCueList object.

- [x] [TextTrackCueList.getCueById(id)](https://html.spec.whatwg.org/#dom-texttrackcuelist-getcuebyid) — The getCueById(id) method, when called with an argument other than the empty string, must return the first text track cue in the list represented by the TextTrackCueList object whose text track cue identifier is id, if any, or null otherwise. If the argument is the empty string, then the method must return null.

- [x] [TextTrackCue.track](https://html.spec.whatwg.org/#dom-texttrackcue-track) — The track getter steps are to return the text track in whose list of cues this's text track cue finds itself, if any; or null otherwise.

- [x] [TextTrackCue.id](https://html.spec.whatwg.org/#dom-texttrackcue-id) — The id attribute, on getting, must return the text track cue identifier of the text track cue that the TextTrackCue object represents. On setting, the text track cue identifier must be set to the new value.

- [x] [TextTrackCue.startTime](https://html.spec.whatwg.org/#dom-texttrackcue-starttime) — The startTime attribute, on getting, must return the text track cue start time of the text track cue that the TextTrackCue object represents, in seconds. On setting, the text track cue start time must be set to the new value, interpreted in seconds; then, if the TextTrackCue object's text track cue is in a text track's list of cues, and that text track is in a media element's list of text tracks, and the media element's show poster flag is not set, then run the time marches on steps for that media element.

- [x] [TextTrackCue.endTime](https://html.spec.whatwg.org/#dom-texttrackcue-endtime) — The endTime attribute, on getting, must return the text track cue end time of the text track cue that the TextTrackCue object represents, in seconds or positive Infinity. On setting, if the new value is negative Infinity or a Not-a-Number (NaN) value, then throw a TypeError exception. Otherwise, the text track cue end time must be set to the new value. Then, if the TextTrackCue object's text track cue is in a text track's list of cues, and that text track is in a media element's list of text tracks, and the media element's show poster flag is not set, then run the time marches on steps for that media element.

- [x] [TextTrackCue.pauseOnExit](https://html.spec.whatwg.org/#dom-texttrackcue-pauseonexit) — The pauseOnExit attribute, on getting, must return true if the text track cue pause-on-exit flag of the text track cue that the TextTrackCue object represents is set; or false otherwise. On setting, the text track cue pause-on-exit flag must be set if the new value is true, and must be unset otherwise.

###### [4.8.11.11.6 Event handlers for objects of the text track APIs](https://html.spec.whatwg.org/#cue-events)

Status: n/a (later: media playback)

###### [4.8.11.11.7 Best practices for metadata text tracks](https://html.spec.whatwg.org/#best-practices-for-metadata-text-tracks)

Status: n/a (later: media playback)

##### [4.8.11.12 Identifying a track kind through a URL](https://html.spec.whatwg.org/#identifying-a-track-kind-through-a-url)

Status: n/a (later: media playback)

##### [4.8.11.13 User interface](https://html.spec.whatwg.org/#user-interface)

Status: n/a (later: media playback)
<!-- definitions: attr-media-controls expose-a-user-interface-to-the-user concept-media-volume concept-media-muted concept-media-muted-state volume-locked attr-media-muted -->

- [x] [set the playback volume](https://html.spec.whatwg.org/#set-the-playback-volume) — To set the playback volume of a media element element to a number value:
  - 1. If element's playback volume equals value, then return.
  - 2. Set element's playback volume to value.
  - 3. If element is not allowed to play, then run the internal pause steps for element.
  - 4. Queue a media element task given element to fire an event named volumechange at element.

- [x] [HTMLMediaElement.volume](https://html.spec.whatwg.org/#dom-media-volume) — The volume getter steps are to return this's playback volume.

- [x] [The volume setter steps](https://html.spec.whatwg.org/#dom-media-volume) — The volume setter steps are:
  - 1. If the given value is not in the range 0.0 to 1.0 inclusive, then throw an "IndexSizeError" DOMException.
  - 2. Set the playback volume of this to the given value.

- [x] [set the muted state](https://html.spec.whatwg.org/#set-the-muted-state) — To set the muted state of a media element element to a boolean value:
  - 1. If element's muted state equals value, then return.
  - 2. Set element's muted state to value.
  - 3. If element is not allowed to play, then run the internal pause steps for element.
  - 4. Queue a media element task given element to fire an event named volumechange at element.

- [x] [HTMLMediaElement.muted](https://html.spec.whatwg.org/#dom-media-muted) — The muted getter steps are to return true if this is muted; otherwise false.

- [x] [The muted setter steps](https://html.spec.whatwg.org/#dom-media-muted) — The muted setter steps are to set the muted state of this to the given value.

- [x] [effective media volume (HTMLMediaElement)](https://html.spec.whatwg.org/#effective-media-volume) — An element's effective media volume is determined as follows:
  - 1. If the user has indicated that the user agent is to override the volume of the element, then return the volume desired by the user.
  - 2. If the user agent's volume locked is true, then return the system volume.
  - 3. If the element is muted, then return 0.
  - 4. Let volume be the playback volume of the audio portions of the media element, in range 0.0 (silent) to 1.0 (loudest).
  - 5. Return volume, interpreted relative to the range 0.0 to 1.0, with 0.0 being silent, and 1.0 being the loudest setting, values in between increasing in loudness. The range need not be linear. The loudest setting may be lower than the system's loudest possible setting; for example the user could have set a maximum volume.

##### [4.8.11.14 Time ranges](https://html.spec.whatwg.org/#time-ranges)

Status: n/a (later: media playback)
<!-- definitions: timeranges-ranges timeranges-range timeranges-range-start timeranges-range-end -->

- [x] [TimeRanges.length](https://html.spec.whatwg.org/#dom-timeranges-length) — The length getter steps are to return this's ranges's size.

- [x] [TimeRanges.start(index)](https://html.spec.whatwg.org/#dom-timeranges-start) — The start(index) method steps are:
  - 1. If index is greater than or equal to this's ranges's size, then throw an "IndexSizeError" DOMException.
  - 2. Return this's ranges[index]'s start.

- [x] [TimeRanges.end(index)](https://html.spec.whatwg.org/#dom-timeranges-end) — The end(index) method steps are:
  - 1. If index is greater than or equal to this's ranges's size, then throw an "IndexSizeError" DOMException.
  - 2. Return this's ranges[index]'s end.

- [x] [normalized TimeRanges object](https://html.spec.whatwg.org/#normalised-timeranges-object) — A TimeRanges object is a normalized TimeRanges object if the following are true for each time range in its ranges:

##### [4.8.11.15 The TrackEvent interface](https://html.spec.whatwg.org/#the-trackevent-interface)

Status: n/a (later: media playback)

- [x] [TrackEvent.track](https://html.spec.whatwg.org/#dom-trackevent-track) — The track attribute must return the value it was initialized to. It represents the context information for the event.

##### [4.8.11.16 Events summary](https://html.spec.whatwg.org/#mediaevents)

Status: n/a (later: media playback)

##### [4.8.11.17 Security and privacy considerations](https://html.spec.whatwg.org/#security-and-privacy-considerations)

Status: n/a (later: media playback)

##### [4.8.11.18 Best practices for authors using media elements](https://html.spec.whatwg.org/#best-practices-for-authors-using-media-elements)

Status: n/a (later: media playback)

##### [4.8.11.19 Best practices for implementers of media elements](https://html.spec.whatwg.org/#best-practices-for-implementers-of-media-elements)

Status: n/a (later: media playback)
