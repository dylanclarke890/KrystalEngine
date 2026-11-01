# HTML review checklist: 7.6 Speculative loading

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 14 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html speculative-loading` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [7.6 Speculative loading](https://html.spec.whatwg.org/#speculative-loading)

Status: n/a (no speculative loading: later, with the network loader)

#### [7.6.1 Speculation rules](https://html.spec.whatwg.org/#speculation-rules)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: speculation-rule-set-authoring-requirements valid-speculation-rule valid-document-rule-predicate valid-url-pattern-input -->

##### [7.6.1.1 Data model](https://html.spec.whatwg.org/#speculation-rules-data-model)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: speculation-rule-set sr-set-prefetch speculation-rule sr-urls sr-predicate sr-eagerness sr-referrer-policy sr-tags sr-requirements sr-nvs-hint document-rule-predicate document-rule-conjunction sr-dr-c-clauses document-rule-disjunction sr-dr-d-clauses document-rule-negation sr-dr-n-clause document-rule-url-pattern-predicate sr-dr-urlpattern-patterns document-rule-selector-predicate sr-dr-cssselector-selectors speculation-rule-eagerness sr-eagerness-immediate sr-eagerness-eager sr-eagerness-moderate sr-eagerness-conservative speculation-rule-tag speculation-rule-requirement -->

- [x] [less eager](https://html.spec.whatwg.org/#sr-eagerness-less-eager) — A speculation rule eagerness A is less eager than another speculation rule eagerness B if A follows B in the above list.

- [x] [at least as eager](https://html.spec.whatwg.org/#sr-eagerness-at-least-as-eager) — A speculation rule eagerness A is at least as eager as another speculation rule eagerness B if A is not less eager than B.

##### [7.6.1.2 Parsing](https://html.spec.whatwg.org/#speculation-rules-parsing)

Status: n/a (no speculative loading: later, with the network loader)

- [x] [parse a speculation rule set string](https://html.spec.whatwg.org/#parse-a-speculation-rule-set-string) — To parse a speculation rule set string given a string input, a Document document, and a URL baseURL:
  - 1. Let parsed be the result of parsing a JSON string to an Infra value given input.
  - 2. If parsed is not a map, then throw a TypeError indicating that the top-level value needs to be a JSON object.
  - 3. Let result be a new speculation rule set.
  - 4. Let tag be null.
  - 5. If parsed["tag"] exists:
    - 5a. If parsed["tag"] is not a speculation rule tag, then throw a TypeError indicating that the speculation rule tag is invalid.
    - 5b. Set tag to parsed["tag"].
  - 6. Let typesToTreatAsPrefetch be « "prefetch" ».
  - 7. The user agent may append "prerender" to typesToTreatAsPrefetch.
  - 8. For each type of typesToTreatAsPrefetch:
    - 8a. If parsed[type] exists:
      - 8a.i. If parsed[type] is a list, then for each rule of parsed[type]:
        - 8a.i.A. Let rule be the result of parsing a speculation rule given rule, tag, document, and baseURL.
        - 8a.i.B. If rule is null, then continue.
        - 8a.i.C. Append rule to result's prefetch rules.
      - 8a.ii. Otherwise, the user agent may report a warning to the console indicating that the rules list for type needs to be a JSON array.
  - 9. Return result.

- [x] [parse a speculation rule](https://html.spec.whatwg.org/#parse-a-speculation-rule) — To parse a speculation rule given a map input, a speculation rule tag rulesetLevelTag, a Document document, and a URL baseURL:
  - 1. If input is not a map:
    - 1a. The user agent may report a warning to the console indicating that the rule needs to be a JSON object.
    - 1b. Return null.
  - 2. If input has any key other than "source", "urls", "where", "relative_to", "eagerness", "referrer_policy", "tag", "requires", "expects_no_vary_search", or "target_hint":
    - 2a. The user agent may report a warning to the console indicating that the rule has unrecognized keys.
    - 2b. Return null.
  - 3. Let source be null.
  - 4. If input["source"] exists, then set source to input["source"].
  - 5. Otherwise, if input["urls"] exists and input["where"] does not exist, then set source to "list".
  - 6. Otherwise, if input["where"] exists and input["urls"] does not exist, then set source to "document".
  - 7. If source is neither "list" nor "document":
    - 7a. The user agent may report a warning to the console indicating that a source could not be inferred or an invalid source was specified.
    - 7b. Return null.
  - 8. Let urls be an empty list.
  - 9. Let predicate be null.
  - 10. If source is "list":
    - 10a. If input["where"] exists:
      - 10a.i. The user agent may report a warning to the console indicating that there were conflicting sources for this rule.
      - 10a.ii. Return null.
    - 10b. If input["relative_to"] exists:
      - 10b.i. If input["relative_to"] is neither "ruleset" nor "document":
        - 10b.i.A. The user agent may report a warning to the console indicating that the supplied relative-to value was invalid.
        - 10b.i.B. Return null.
      - 10b.ii. If input["relative_to"] is "document", then set baseURL to document's document base URL.
    - 10c. If input["urls"] does not exist or is not a list:
      - 10c.i. The user agent may report a warning to the console indicating that the supplied URL list was invalid.
      - 10c.ii. Return null.
    - 10d. For each urlString of input["urls"]:
      - 10d.i. If urlString is not a string:
        - 10d.i.A. The user agent may report a warning to the console indicating that the supplied URL must be a string.
        - 10d.i.B. Return null.
      - 10d.ii. Let parsedURL be the result of URL parsing urlString with baseURL.
      - 10d.iii. If parsedURL is failure, or parsedURL's scheme is not an HTTP(S) scheme:
        - 10d.iii.A. The user agent may report a warning to the console indicating that the supplied URL string was unparseable.
        - 10d.iii.B. Continue.
      - 10d.iv. Append parsedURL to urls.
  - 11. If source is "document":
    - 11a. If input["urls"] or input["relative_to"] exists:
      - 11a.i. The user agent may report a warning to the console indicating that there were conflicting sources for this rule.
      - 11a.ii. Return null.
    - 11b. If input["where"] does not exist, then set predicate to a document rule conjunction whose clauses is an empty list.
    - 11c. Otherwise, set predicate to the result of parsing a document rule predicate given input["where"], document, and baseURL.
    - 11d. If predicate is null, then return null.
  - 12. Let eagerness be "immediate" if source is "list"; otherwise, "conservative".
  - 13. If input["eagerness"] exists:
    - 13a. If input["eagerness"] is not a speculation rule eagerness:
      - 13a.i. The user agent may report a warning to the console indicating that the eagerness was invalid.
      - 13a.ii. Return null.
    - 13b. Set eagerness to input["eagerness"].
  - 14. Let referrerPolicy be the empty string.
  - 15. If input["referrer_policy"] exists:
    - 15a. If input["referrer_policy"] is not a referrer policy:
      - 15a.i. The user agent may report a warning to the console indicating that the referrer policy was invalid.
      - 15a.ii. Return null.
    - 15b. Set referrerPolicy to input["referrer_policy"].
  - 16. Let tags be an empty ordered set.
  - 17. If rulesetLevelTag is not null, then append rulesetLevelTag to tags.
  - 18. If input["tag"] exists:
    - 18a. If input["tag"] is not a speculation rule tag:
      - 18a.i. The user agent may report a warning to the console indicating that the tag was invalid.
      - 18a.ii. Return null.
    - 18b. Append input["tag"] to tags.
  - 19. If tags is empty, then append null to tags.
  - 20. Assert: tags's size is either 1 or 2.
  - 21. Let requirements be an empty ordered set.
  - 22. If input["requires"] exists:
    - 22a. If input["requires"] is not a list:
      - 22a.i. The user agent may report a warning to the console indicating that the requirements were not understood.
      - 22a.ii. Return null.
    - 22b. For each requirement of input["requires"]:
      - 22b.i. If requirement is not a speculation rule requirement:
        - 22b.i.A. The user agent may report a warning to the console indicating that the requirement was not understood.
        - 22b.i.B. Return null.
      - 22b.ii. Append requirement to requirements.
  - 23. Let noVarySearchHint be the default URL search variance.
  - 24. If input["expects_no_vary_search"] exists:
    - 24a. If input["expects_no_vary_search"] is not a string:
      - 24a.i. The user agent may report a warning to the console indicating that the `No-Vary-Search` hint was invalid.
      - 24a.ii. Return null.
    - 24b. Set noVarySearchHint to the result of parsing a URL search variance given input["expects_no_vary_search"].
  - 25. Return a speculation rule with: URLsurlspredicatepredicateeagernesseagernessreferrer policyreferrerPolicytagstagsrequirementsrequirementsNo-Vary-Search hintnoVarySearchHint

- [x] [parse a document rule predicate](https://html.spec.whatwg.org/#parse-a-document-rule-predicate) — To parse a document rule predicate given a value input, a Document document, and a URL baseURL:
  - 1. If input is not a map:
    - 1a. The user agent may report a warning to the console indicating that the document rule predicate was invalid.
    - 1b. Return null.
  - 2. If input does not contain exactly one of "and", "or", "not", "href_matches", or "selector_matches":
    - 2a. The user agent may report a warning to the console indicating that the document rule predicate was empty or ambiguous.
    - 2b. Return null.
  - 3. Let predicateType be the single key found in the previous step.
  - 4. If predicateType is "and" or "or":
    - 4a. If input has any key other than predicateType:
      - 4a.i. The user agent may report a warning to the console indicating that the document rule predicate had unexpected extra options.
      - 4a.ii. Return null.
    - 4b. If input[predicateType] is not a list:
      - 4b.i. The user agent may report a warning to the console indicating that the document rule predicate had an invalid clause list.
      - 4b.ii. Return null.
    - 4c. Let clauses be an empty list.
    - 4d. For each rawClause of input[predicateType]:
      - 4d.i. Let clause be the result of parsing a document rule predicate given rawClause, document, and baseURL.
      - 4d.ii. If clause is null, then return null.
      - 4d.iii. Append clause to clauses.
    - 4e. If predicateType is "and", then return a document rule conjunction whose clauses is clauses.
    - 4f. Return a document rule disjunction whose clauses is clauses.
  - 5. If predicateType is "not":
    - 5a. If input has any key other than "not":
      - 5a.i. The user agent may report a warning to the console indicating that the document rule predicate had unexpected extra options.
      - 5a.ii. Return null.
    - 5b. Let clause be the result of parsing a document rule predicate given input[predicateType], document, and baseURL.
    - 5c. If clause is null, then return null.
    - 5d. Return a document rule negation whose clause is clause.
  - 6. If predicateType is "href_matches":
    - 6a. If input has any key other than "href_matches" or "relative_to":
      - 6a.i. The user agent may report a warning to the console indicating that the document rule predicate had unexpected extra options.
      - 6a.ii. Return null.
    - 6b. If input["relative_to"] exists:
      - 6b.i. If input["relative_to"] is neither "ruleset" nor "document":
        - 6b.i.A. The user agent may report a warning to the console indicating that the supplied relative-to value was invalid.
        - 6b.i.B. Return null.
      - 6b.ii. If input["relative_to"] is "document", then set baseURL to document's document base URL.
    - 6c. Let rawPatterns be input["href_matches"].
    - 6d. If rawPatterns is not a list, then set rawPatterns to « rawPatterns ».
    - 6e. Let patterns be an empty list.
    - 6f. For each rawPattern of rawPatterns:
      - 6f.i. Let pattern be the result of building a URL pattern from an Infra value given rawPattern and baseURL. If this step throws an exception, catch the exception and set pattern to null.
      - 6f.ii. If pattern is null:
        - 6f.ii.A. The user agent may report a warning to the console indicating that the supplied URL pattern was invalid.
        - 6f.ii.B. Return null.
      - 6f.iii. Append pattern to patterns.
    - 6g. Return a document rule URL pattern predicate whose patterns is patterns.
  - 7. If predicateType is "selector_matches":
    - 7a. If input has any key other than "selector_matches":
      - 7a.i. The user agent may report a warning to the console indicating that the document rule predicate had unexpected extra options.
      - 7a.ii. Return null.
    - 7b. Let rawSelectors be input["selector_matches"].
    - 7c. If rawSelectors is not a list, then set rawSelectors to « rawSelectors ».
    - 7d. Let selectors be an empty list.
    - 7e. For each rawSelector of rawSelectors:
      - 7e.i. Let parsedSelectorList be failure.
      - 7e.ii. If rawSelector is a string, then set parsedSelectorList to the result of parsing a selector given rawSelector.
      - 7e.iii. If parsedSelectorList is failure:
        - 7e.iii.A. The user agent may report a warning to the console indicating that the supplied selector list was invalid.
        - 7e.iii.B. Return null.
      - 7e.iv. For each selector of parsedSelectorList, append selector to selectors.
    - 7f. Return a document rule selector predicate whose selectors is selectors.
  - 8. Assert: this step is never reached, as one of the previous branches was taken.

##### [7.6.1.3 Processing model](https://html.spec.whatwg.org/#speculation-rules-processing-model)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: speculative-load-candidate sl-candidate-url sl-candidate-nvs-hint sl-candidate-eagerness sl-candidate-referrer-policy sl-candidate-tags prefetch-candidate sl-candidate-anonymization-policy prefetch-ip-anonymization-policy cross-origin-prefetch-ip-anonymization-policy copiap-origin document-sr-sets consider-speculative-loads-microtask-queued speculation-rules-task-source -->

- [x] [redundant with](https://html.spec.whatwg.org/#sl-candidate-redundant-with) — A speculative load candidate candidateA is redundant with another speculative load candidate candidateB if the following steps return true:
  - 1. If candidateA's No-Vary-Search hint is not equal to candidateB's No-Vary-Search hint, then return false.
  - 2. If candidateA's URL is not equivalent modulo search variance to candidateB's URL given candidateA's No-Vary-Search hint, then return false.
  - 3. Return true.

- [x] [consider speculative loads](https://html.spec.whatwg.org/#consider-speculative-loads) — To consider speculative loads for a Document document:
  - 1. If document's node navigable is not a top-level traversable, then return.
  - 2. If document's consider speculative loads microtask queued is true, then return.
  - 3. Set document's consider speculative loads microtask queued to true.
  - 4. Queue a microtask given document to run the following steps:
    - 4a. Set document's consider speculative loads microtask queued to false.
    - 4b. Run the inner consider speculative loads steps for document.

- [x] [inner consider speculative loads steps](https://html.spec.whatwg.org/#inner-consider-speculative-loads-steps) — The inner consider speculative loads steps for a Document document are:
  - 1. If document is not fully active, then return.
  - 2. Let prefetchCandidates be an empty list.
  - 3. For each ruleSet of document's speculation rule sets:
    - 3a. For each rule of ruleSet's prefetch rules:
      - 3a.i. Let anonymizationPolicy be null.
      - 3a.ii. If rule's requirements contains "anonymous-client-ip-when-cross-origin", then set anonymizationPolicy to a cross-origin prefetch IP anonymization policy whose origin is document's origin.
      - 3a.iii. For each url of rule's URLs:
        - 3a.iii.A. Let referrerPolicy be the result of computing a speculative load referrer policy given rule and null.
        - 3a.iii.B. Append a new prefetch candidate with URLurlNo-Vary-Search hintrule's No-Vary-Search hinteagernessrule's eagernessreferrer policyreferrerPolicytagsrule's tagsanonymization policyanonymizationPolicy to prefetchCandidates.
      - 3a.iv. If rule's predicate is not null:
        - 3a.iv.A. Let links be the result of finding matching links given document and rule's predicate.
        - 3a.iv.B. For each link of links:
          - 3a.iv.B.1. Let referrerPolicy be the result of computing a speculative load referrer policy given rule and link.
          - 3a.iv.B.2. Append a new prefetch candidate with URLlink's urlNo-Vary-Search hintrule's No-Vary-Search hinteagernessrule's eagernessreferrer policyreferrerPolicytagsrule's tagsanonymization policyanonymizationPolicy to prefetchCandidates.
  - 4. For each prefetchRecord of document's prefetch records:
    - 4a. If prefetchRecord's source is not "speculation rules", then continue.
    - 4b. Assert: prefetchRecord's state is not "canceled".
    - 4c. If prefetchRecord is not still being speculated given prefetchCandidates, then cancel and discard prefetchRecord given document.
  - 5. Let prefetchCandidateGroups be an empty list.
  - 6. For each candidate of prefetchCandidates:
    - 6a. Let group be « candidate ».
    - 6b. Extend group with all items in prefetchCandidates, apart from candidate itself, which are redundant with candidate and whose eagerness is at least as eager as candidate's eagerness.
    - 6c. If prefetchCandidateGroups contains another group whose items are the same as group, ignoring order, then continue.
    - 6d. Append group to prefetchCandidateGroups.
  - 7. For each group of prefetchCandidateGroups:
    - 7a. The user agent may run the following steps: When deciding whether to execute this "may" step, user agents should consider prefetchCandidate's eagerness, in accordance to the current behavior of the user and the definitions of speculation rule eagerness. prefetchCandidate's No-Vary-Search hint can also be useful in implementing the heuristics defined for the speculation rule eagerness values. For…
      - 7a.i. Let prefetchCandidate be group[0].
      - 7a.ii. Let tagsToSend be the result of collecting tags from speculative load candidates given group.
      - 7a.iii. Let prefetchRecord be a new prefetch record with source"speculation rules"URLprefetchCandidate's URLNo-Vary-Search hintprefetchCandidate's No-Vary-Search hintreferrer policyprefetchCandidate's referrer policyanonymization policyprefetchCandidate's anonymization policytags tagsToSend
      - 7a.iv. Start a referrer-initiated navigational prefetch given document and prefetchRecord.

- [x] [compute a speculative load referrer policy](https://html.spec.whatwg.org/#compute-a-speculative-load-referrer-policy) — To compute a speculative load referrer policy given a speculation rule rule and an a element, area element, or null link:
  - 1. If rule's referrer policy is not the empty string, then return rule's referrer policy.
  - 2. If link is null, then return the empty string.
  - 3. Return link's hyperlink referrer policy.

- [x] [collect tags from speculative load candidates](https://html.spec.whatwg.org/#collect-tags-from-speculative-load-candidates) — To collect tags from speculative load candidates given a list of speculative load candidates candidates:
  - 1. Let tags be an empty ordered set.
  - 2. For each candidate of candidates:
    - 2a. For each tag of candidate's tags: append tag to tags.
  - 3. Sort in ascending order tags, with tagA being less than tagB if tagA is null, or if tagA is code unit less than tagB.
  - 4. Return tags.

- [x] [find matching links](https://html.spec.whatwg.org/#find-matching-links) — To find matching links given a Document document and a document rule predicate predicate:
  - 1. Let links be an empty list.
  - 2. For each shadow-including descendant descendant of document, in shadow-including tree order:
    - 2a. If descendant is not an a or area element with an href attribute, then continue.
    - 2b. If descendant is not being rendered or is part of skipped contents, then continue.
    - 2c. If descendant's url is null, or its scheme is not an HTTP(S) scheme, then continue.
    - 2d. If predicate matches descendant, then append descendant to links.
  - 3. Return links.

- [x] [matches](https://html.spec.whatwg.org/#dr-predicate-matches) — A document rule predicate predicate matches an a or area element el if the following steps return true, switching on predicate's type:
  - • document rule conjunction:
    - 1. For each clause of predicate's clauses:
      - 1a. If clause does not match el, then return false.
    - 2. Return true.
  - • document rule disjunction:
    - 1. For each clause of predicate's clauses:
      - 1a. If clause matches el, then return true.
    - 2. Return false.
  - • document rule negation:
    - 1. If predicate's clause matches el, then return false.
    - 2. Return true.
  - • document rule URL pattern predicate:
    - 1. For each pattern of predicate's patterns:
      - 1a. If performing a match given pattern and el's url gives a non-null value, then return true.
    - 2. Return false.
  - • document rule selector predicate:
    - 1. For each selector of predicate's selectors:
      - 1a. If performing a match given selector and el with the scoping root set to el's root returns success, then return true.
    - 2. Return false.

#### [7.6.2 Navigational prefetching](https://html.spec.whatwg.org/#navigational-prefetching)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: prefetch-record prefetch-record-source prefetch-record-url prefetch-record-nvs-hint prefetch-record-referrer-policy prefetch-record-anonymization-policy prefetch-record-tags prefetch-record-state prefetch-record-cancel-and-discard still-being-speculated document-prefetch-records start-a-referrer-initiated-navigational-prefetch -->

#### [7.6.3 The `Speculation-Rules` header](https://html.spec.whatwg.org/#the-speculation-rules-header)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: speculation-rules-2 -->

- [x] [process the `Speculation-Rules` header](https://html.spec.whatwg.org/#process-the-speculation-rules-header) — To process the `Speculation-Rules` header given a Document document and a response response:
  - 1. Let parsedList be the result of getting a structured field value given `Speculation-Rules` and "list" from response's header list.
  - 2. If parsedList is null, then return.
  - 3. For each item of parsedList:
    - 3a. If item is not a string, then continue.
    - 3b. Let url be the result of URL parsing item with document's document base URL.
    - 3c. If url is failure, then continue.
    - 3d. In parallel:
      - 3d.i. Optionally, wait for an implementation-defined amount of time.
      - 3d.ii. Queue a global task on the speculation rules task source given document's relevant global object to perform the following steps:
        - 3d.ii.A. Let request be a new request whose URL is url, destination is "speculationrules", and mode is "cors".
        - 3d.ii.B. Fetch request with the following processResponseConsumeBody steps given response response and null, failure, or a byte sequence bodyBytes:
          - 3d.ii.B.1. If bodyBytes is null or failure, then abort these steps.
          - 3d.ii.B.2. If response's status is not an ok status, then abort these steps.
          - 3d.ii.B.3. If the result of extracting a MIME type from response's header list does not have an essence of "application/speculationrules+json", then abort these steps.
          - 3d.ii.B.4. Let bodyText be the result of UTF-8 decoding bodyBytes.
          - 3d.ii.B.5. Let ruleSet be the result of parsing a speculation rule set string given bodyText, document, and response's URL. If this throws an exception, then abort these steps.
          - 3d.ii.B.6. Append ruleSet to document's speculation rule sets.
          - 3d.ii.B.7. Consider speculative loads for document.

#### [7.6.4 The `Sec-Speculation-Tags` header](https://html.spec.whatwg.org/#the-sec-speculation-tags-header)

Status: n/a (no speculative loading: later, with the network loader)
<!-- definitions: sec-speculation-tags -->

#### [7.6.5 Security considerations](https://html.spec.whatwg.org/#speculative-loading-security)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.5.1 Cross-site requests](https://html.spec.whatwg.org/#speculative-loading-cross-site-requests)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.5.2 Injected content](https://html.spec.whatwg.org/#speculative-loading-injected-content)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.5.3 IP anonymization](https://html.spec.whatwg.org/#speculative-loading-ip-anonymization)

Status: n/a (no speculative loading: later, with the network loader)

#### [7.6.6 Privacy considerations](https://html.spec.whatwg.org/#speculative-loading-privacy)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.6.1 Heuristics and optionality](https://html.spec.whatwg.org/#speculative-loading-heuristics)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.6.2 State partitioning](https://html.spec.whatwg.org/#speculative-loading-state-partitioning)

Status: n/a (no speculative loading: later, with the network loader)

##### [7.6.6.3 Identity joining](https://html.spec.whatwg.org/#speculative-loading-identity-joining)

Status: n/a (no speculative loading: later, with the network loader)

### [7.7 The `X-Frame-Options` header](https://html.spec.whatwg.org/#the-x-frame-options-header)

Status: n/a (no frame tree)
<!-- definitions: x-frame-options -->

- [x] [check a navigation response's adherence to `X-Frame-Options`](https://html.spec.whatwg.org/#check-a-navigation-response's-adherence-to-x-frame-options) — To check a navigation response's adherence to `X-Frame-Options`, given a response response, a navigable navigable, a CSP list cspList, and an origin destinationOrigin:
  - 1. If navigable is not a child navigable, then return true.
  - 2. For each policy of cspList:
    - 2a. If policy's disposition is not "enforce", then continue.
    - 2b. If policy's directive set contains a frame-ancestors directive, then return true.
  - 3. Let rawXFrameOptions be the result of getting, decoding, and splitting `X-Frame-Options` from response's header list.
  - 4. Let xFrameOptions be a new set.
  - 5. For each value of rawXFrameOptions, append value, converted to ASCII lowercase, to xFrameOptions.
  - 6. If xFrameOptions's size is greater than 1, and xFrameOptions contains any of "deny", "allowall", or "sameorigin", then return false.
  - 7. If xFrameOptions's size is greater than 1, then return true.
  - 8. If xFrameOptions[0] is "deny", then return false.
  - 9. If xFrameOptions[0] is "sameorigin":
    - 9a. Let containerDocument be navigable's container document.
    - 9b. While containerDocument is not null:
      - 9b.i. If containerDocument's origin is not same origin with destinationOrigin, then return false.
      - 9b.ii. Set containerDocument to containerDocument's container document.
  - 10. Return true.

### [7.8 The `Refresh` header](https://html.spec.whatwg.org/#the-refresh-header)

Status: unreviewed
<!-- definitions: refresh -->

### [7.9 Browser user interface considerations](https://html.spec.whatwg.org/#nav-traversal-ui)

Status: n/a (guidance)
