# HTML review checklist: 7 Loading web pages

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 51 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html browsers` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [7 Loading web pages](https://html.spec.whatwg.org/#browsers)

Status: reviewed (2026-11-01)
> The sections below.

### [7.1 Supporting concepts](https://html.spec.whatwg.org/#loading-web-pages-supporting-concepts)

Status: reviewed (2026-11-01)
> The sections below.

#### [7.1.1 Origins](https://html.spec.whatwg.org/#origin)

Status: reviewed (2026-11-01)
> Only the serialization of a URL's origin exists (`URLDecomposition::SerializedOrigin`, with a `file:` URL's origin `file://`); origins, their effective domains and the same origin checks don't (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-origin concept-origin-opaque concept-origin-tuple concept-origin-scheme concept-origin-host concept-origin-port concept-origin-domain -->

- [x] [effective domain](https://html.spec.whatwg.org/#concept-origin-effective-domain) — The effective domain of an origin origin is computed as follows:
  - 1. If origin is an opaque origin, then return null.
  - 2. If origin's domain is non-null, then return origin's domain.
  - 3. Return origin's host.

- [x] [serialization of an origin](https://html.spec.whatwg.org/#ascii-serialisation-of-an-origin) — The serialization of an origin is the string obtained by applying the following algorithm to the given origin origin:
  - 1. If origin is an opaque origin, then return "null".
  - 2. Otherwise, let result be origin's scheme.
  - 3. Append "://" to result.
  - 4. Append origin's host, serialized, to result.
  - 5. If origin's port is non-null, append a U+003A COLON character (:), and origin's port, serialized, to result.
  - 6. Return result.

- [x] [same origin](https://html.spec.whatwg.org/#same-origin) — Two origins, A and B, are said to be same origin if the following algorithm returns true:
  - 1. If A and B are the same opaque origin, then return true.
  - 2. If A and B are both tuple origins, their schemes and ports are identical, and their hosts are equal, then return true.
  - 3. Return false.

- [x] [same origin-domain](https://html.spec.whatwg.org/#same-origin-domain) — Two origins, A and B, are said to be same origin-domain if the following algorithm returns true:
  - 1. If A and B are the same opaque origin, then return true.
  - 2. If A and B are both tuple origins:
    - 2a. If A and B's schemes are identical, and their domains are identical and non-null, then return true.
    - 2b. Otherwise, if A and B are same origin and their domains are both null, return true.
  - 3. Return false.

##### [7.1.1.1 Sites](https://html.spec.whatwg.org/#sites)

Status: n/a (no origins)
<!-- definitions: scheme-and-host concept-scheme-and-host-scheme concept-scheme-and-host-host site -->

- [x] [obtain a site](https://html.spec.whatwg.org/#obtain-a-site) — To obtain a site, given an origin origin, run these steps:
  - 1. If origin is an opaque origin, then return origin.
  - 2. If origin's host's registrable domain is null, then return (origin's scheme, origin's host).
  - 3. Return (origin's scheme, origin's host's registrable domain).

- [x] [same site (site)](https://html.spec.whatwg.org/#concept-site-same-site) — Two sites, A and B, are said to be same site if the following algorithm returns true:
  - 1. If A and B are the same opaque origin, then return true.
  - 2. If A or B is an opaque origin, then return false.
  - 3. If A's and B's scheme values are different, then return false.
  - 4. If A's and B's host values are not equal, then return false.
  - 5. Return true.

- [x] [serialization of a site](https://html.spec.whatwg.org/#serialization-of-a-site) — The serialization of a site is the string obtained by applying the following algorithm to the given site site:
  - 1. If site is an opaque origin, then return "null".
  - 2. Let result be site[0].
  - 3. Append "://" to result.
  - 4. Append site[1], serialized, to result.
  - 5. Return result.

- [x] [schemelessly same site](https://html.spec.whatwg.org/#schemelessly-same-site) — Two origins, A and B, are said to be schemelessly same site if the following algorithm returns true:
  - 1. If A and B are the same opaque origin, then return true.
  - 2. If A and B are both tuple origins:
    - 2a. Let hostA be A's host, and let hostB be B's host.
    - 2b. If hostA equals hostB and hostA's registrable domain is null, then return true.
    - 2c. If hostA's registrable domain equals hostB's registrable domain and is non-null, then return true.
  - 3. Return false.

- [x] [same site](https://html.spec.whatwg.org/#same-site) — Two origins, A and B, are said to be same site if the following algorithm returns true:
  - 1. Let siteA be the result of obtaining a site given A.
  - 2. Let siteB be the result of obtaining a site given B.
  - 3. If siteA is same site with siteB, then return true.
  - 4. Return false.

##### [7.1.1.2 Relaxing the same-origin restriction](https://html.spec.whatwg.org/#relaxing-the-same-origin-restriction)

Status: reviewed (2026-11-01)
> `document.domain` isn't implemented (see `KNOWN-ISSUES.md`).

- [x] [Document.domain](https://html.spec.whatwg.org/#dom-document-domain) — The domain getter steps are:
  - 1. Let effectiveDomain be this's origin's effective domain.
  - 2. If effectiveDomain is null, then return the empty string.
  - 3. Return effectiveDomain, serialized.

- [x] [The domain setter steps](https://html.spec.whatwg.org/#dom-document-domain) — The domain setter steps are:
  - 1. If this's browsing context is null, then throw a "SecurityError" DOMException.
  - 2. If this's active sandboxing flag set has its sandboxed document.domain browsing context flag set, then throw a "SecurityError" DOMException.
  - 3. Let effectiveDomain be this's origin's effective domain.
  - 4. If effectiveDomain is null, then throw a "SecurityError" DOMException.
  - 5. If the given value is not a registrable domain suffix of and is not equal to effectiveDomain, then throw a "SecurityError" DOMException.
  - 6. If the surrounding agent's agent cluster's is origin-keyed is true, then return.
  - 7. Set this's origin's domain to the result of parsing the given value.

- [x] [is a registrable domain suffix of or is equal to](https://html.spec.whatwg.org/#is-a-registrable-domain-suffix-of-or-is-equal-to) — To determine if a scalar value string hostSuffixString is a registrable domain suffix of or is equal to a host originalHost:
  - 1. If hostSuffixString is the empty string, then return false.
  - 2. Let hostSuffix be the result of parsing hostSuffixString.
  - 3. If hostSuffix is failure, then return false.
  - 4. If hostSuffix does not equal originalHost:
    - 4a. If hostSuffix or originalHost is not a domain, then return false.
    - 4b. If hostSuffix, prefixed by U+002E (.), does not match the end of originalHost, then return false.
    - 4c. If any of the following are true: hostSuffix equals hostSuffix's public suffix; or; hostSuffix, prefixed by U+002E (.), matches the end of originalHost's public suffix, then return false. [URL]
    - 4d. Assert: originalHost's public suffix, prefixed by U+002E (.), matches the end of hostSuffix.
  - 5. Return true.

##### [7.1.1.3 The Origin interface](https://html.spec.whatwg.org/#the-origin-interface)

Status: n/a (no origins)
<!-- definitions: concept-origin-origin -->

- [x] [Objects implementing the Origin interface's extract an origin steps](https://html.spec.whatwg.org/#extract-an-origin) — Objects implementing the Origin interface's extract an origin steps are to return this's origin.

- [x] [Origin.new Origin()](https://html.spec.whatwg.org/#dom-origin-constructor) — The new Origin() constructor steps are to set this's origin to a unique opaque origin.

- [x] [Origin.from(value)](https://html.spec.whatwg.org/#dom-origin-from) — The static from(value) method steps are:
  - 1. If value is a platform object:
    - 1a. Let origin be the result of executing value's extract an origin operation.
    - 1b. If origin is not null, then return a new Origin object whose origin is origin.
  - 2. If value is a string:
    - 2a. Let parsedURL be the result of basic URL parsing value.
    - 2b. If parsedURL is not failure, then return a new Origin object whose origin is set to parsedURL's origin.
  - 3. Throw a TypeError.

- [x] [Origin.opaque](https://html.spec.whatwg.org/#dom-origin-opaque) — The opaque getter steps are to return true if this's origin is an opaque origin; otherwise false.

- [x] [Origin.isSameOrigin(other)](https://html.spec.whatwg.org/#dom-origin-issameorigin) — The isSameOrigin(other) method steps are to return true if this's origin is same origin with other's origin; otherwise false.

- [x] [Origin.isSameSite(other)](https://html.spec.whatwg.org/#dom-origin-issamesite) — The isSameSite(other) method steps are to return true if this's origin is same site with other's origin; otherwise false.

#### [7.1.2 Origin-keyed agent clusters](https://html.spec.whatwg.org/#origin-keyed-agent-clusters)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::OriginAgentCluster` is false, as no document is origin-keyed.
<!-- definitions: origin-agent-cluster -->

- [x] [Window.originAgentCluster](https://html.spec.whatwg.org/#dom-originagentcluster) — The originAgentCluster getter steps are to return the surrounding agent's agent cluster's is origin-keyed.

#### [7.1.3 Cross-origin opener policies](https://html.spec.whatwg.org/#cross-origin-opener-policies)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: cross-origin-opener-policy-value coop-unsafe-none coop-same-origin-allow-popups coop-same-origin coop-same-origin-plus-coep coop-noopener-allow-popups cross-origin-opener-policy coop-struct-value coop-struct-report-endpoint coop-struct-report-only-value coop-struct-report-only-endpoint -->

- [x] [match opener policy values](https://html.spec.whatwg.org/#matching-coop) — To match opener policy values, given an opener policy value documentCOOP, an origin documentOrigin, an opener policy value responseCOOP, and an origin responseOrigin:
  - 1. If documentCOOP is "unsafe-none" and responseCOOP is "unsafe-none", then return true.
  - 2. If documentCOOP is "unsafe-none" or responseCOOP is "unsafe-none", then return false.
  - 3. If documentCOOP is responseCOOP and documentOrigin is same origin with responseOrigin, then return true.
  - 4. Return false.

##### [7.1.3.1 The headers](https://html.spec.whatwg.org/#the-coop-headers)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: cross-origin-opener-policy-2 cross-origin-opener-policy-report-only coop-report-to -->

- [x] [obtain an opener policy](https://html.spec.whatwg.org/#obtain-coop) — To obtain an opener policy given a response response and an environment reservedEnvironment:
  - 1. Let policy be a new opener policy.
  - 2. If reservedEnvironment is a non-secure context, then return policy.
  - 3. Let parsedItem be the result of getting a structured field value given `Cross-Origin-Opener-Policy` and "item" from response's header list.
  - 4. If parsedItem is not null and parsedItem[0] is a token:
    - 4a. If parsedItem[0] is "same-origin":
      - 4a.i. Let coep be the result of obtaining a cross-origin embedder policy from response and reservedEnvironment.
      - 4a.ii. If coep's value is compatible with cross-origin isolation, then set policy's value to "same-origin-plus-COEP".
      - 4a.iii. Otherwise, set policy's value to "same-origin".
    - 4b. If parsedItem[0] is "same-origin-allow-popups", then set policy's value to "same-origin-allow-popups".
    - 4c. If parsedItem[0] is "noopener-allow-popups", then set policy's value to "noopener-allow-popups".
    - 4d. If parsedItem[1]["report-to"] exists and it is a string, then set policy's reporting endpoint to parsedItem[1]["report-to"].
  - 5. Set parsedItem to the result of getting a structured field value given `Cross-Origin-Opener-Policy-Report-Only` and "item" from response's header list.
  - 6. If parsedItem is not null and parsedItem[0] is a token:
    - 6a. If parsedItem[0] is "same-origin":
      - 6a.i. Let coep be the result of obtaining a cross-origin embedder policy from response and reservedEnvironment.
      - 6a.ii. If coep's value is compatible with cross-origin isolation or coep's report-only value is compatible with cross-origin isolation, then set policy's report-only value to "same-origin-plus-COEP".
      - 6a.iii. Otherwise, set policy's report-only value to "same-origin".
    - 6b. If parsedItem[0] is "same-origin-allow-popups", then set policy's report-only value to "same-origin-allow-popups".
    - 6c. If parsedItem[1]["report-to"] exists and it is a string, then set policy's report-only reporting endpoint to parsedItem[1]["report-to"].
  - 7. Return policy.

##### [7.1.3.2 Browsing context group switches due to opener policy](https://html.spec.whatwg.org/#browsing-context-group-switches-due-to-cross-origin-opener-policy)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: coop-enforcement-result coop-enforcement-bcg-switch coop-enforcement-bcg-switch-report-only coop-enforcement-url coop-enforcement-origin coop-enforcement-coop coop-enforcement-source -->

- [x] [check if popup COOP values require a browsing context group switch](https://html.spec.whatwg.org/#check-browsing-context-group-switch-coop-value-popup) — To check if popup COOP values require a browsing context group switch, given two origins responseOrigin and activeDocumentNavigationOrigin, and two opener policy values responseCOOPValue and activeDocumentCOOPValue:
  - 1. If responseCOOPValue is "noopener-allow-popups", then return true.
  - 2. If all of the following are true: activeDocumentCOOPValue is "same-origin-allow-popups" or "noopener-allow-popups"; and; responseCOOPValue is "unsafe-none", then return false.
  - 3. If the result of matching activeDocumentCOOPValue, activeDocumentNavigationOrigin, responseCOOPValue, and responseOrigin is true, then return false.
  - 4. Return true.

- [x] [check if COOP values require a browsing context group switch](https://html.spec.whatwg.org/#check-browsing-context-group-switch-coop-value) — To check if COOP values require a browsing context group switch, given a boolean isInitialAboutBlank, two origins responseOrigin and activeDocumentNavigationOrigin, and two opener policy values responseCOOPValue and activeDocumentCOOPValue:
  - 1. If isInitialAboutBlank is true, then return the result of checking if popup COOP values requires a browsing context group switch with responseOrigin, activeDocumentNavigationOrigin, responseCOOPValue, and activeDocumentCOOPValue.
  - 2. If the result of matching activeDocumentCOOPValue, activeDocumentNavigationOrigin, responseCOOPValue, and responseOrigin is true, then return false.
  - 3. Return true.

- [x] [check if enforcing report-only COOP would require a browsing context group switch](https://html.spec.whatwg.org/#check-bcg-switch-navigation-report-only) — To check if enforcing report-only COOP would require a browsing context group switch, given a boolean isInitialAboutBlank, two origins responseOrigin, activeDocumentNavigationOrigin, and two opener policies responseCOOP and activeDocumentCOOP:
  - 1. If the result of checking if COOP values require a browsing context group switch given isInitialAboutBlank, responseOrigin, activeDocumentNavigationOrigin, responseCOOP's report-only value, and activeDocumentCOOP's report-only value is false, then return false.
  - 2. If the result of checking if COOP values require a browsing context group switch given isInitialAboutBlank, responseOrigin, activeDocumentNavigationOrigin, responseCOOP's value, and activeDocumentCOOP's report-only value is true, then return true.
  - 3. If the result of checking if COOP values require a browsing context group switch given isInitialAboutBlank, responseOrigin, activeDocumentNavigationOrigin, responseCOOP's report-only value, and activeDocumentCOOP's value is true, then return true.
  - 4. Return false.

- [x] [enforce a response's opener policy](https://html.spec.whatwg.org/#coop-enforce) — To enforce a response's opener policy, given a browsing context browsingContext, a URL responseURL, an origin responseOrigin, an opener policy responseCOOP, an opener policy enforcement result currentCOOPEnforcementResult, and a referrer referrer:
  - 1. Let newCOOPEnforcementResult be a new opener policy enforcement result with needs a browsing context group switchcurrentCOOPEnforcementResult's needs a browsing context group switchwould need a browsing context group switch due to report-onlycurrentCOOPEnforcementResult's would need a browsing context group switch due to report-onlyurlresponseURLoriginresponseOriginopener policyresponseCOOPcurren…
  - 2. Let isInitialAboutBlank be browsingContext's active document's is initial about:blank.
  - 3. If isInitialAboutBlank is true and browsingContext's initial URL is null, set browsingContext's initial URL to responseURL.
  - 4. If the result of checking if COOP values require a browsing context group switch given isInitialAboutBlank, responseOrigin, currentCOOPEnforcementResult's origin, responseCOOP's value, and currentCOOPEnforcementResult's opener policy's value is true:
    - 4a. Set newCOOPEnforcementResult's needs a browsing context group switch to true.
    - 4b. If browsingContext's group's browsing context set's size is greater than 1:
      - 4b.i. Queue a violation report for browsing context group switch when navigating to a COOP response with responseCOOP, "enforce", responseURL, currentCOOPEnforcementResult's url, currentCOOPEnforcementResult's origin, responseOrigin, and referrer.
      - 4b.ii. Queue a violation report for browsing context group switch when navigating away from a COOP response with currentCOOPEnforcementResult's opener policy, "enforce", currentCOOPEnforcementResult's url, responseURL, currentCOOPEnforcementResult's origin, responseOrigin, and currentCOOPEnforcementResult's current context is navigation source.
  - 5. If the result of checking if enforcing report-only COOP would require a browsing context group switch given isInitialAboutBlank, responseOrigin, currentCOOPEnforcementResult's origin, responseCOOP, and currentCOOPEnforcementResult's opener policy is true:
    - 5a. Set newCOOPEnforcementResult's would need a browsing context group switch due to report-only to true.
    - 5b. If browsingContext's group's browsing context set's size is greater than 1:
      - 5b.i. Queue a violation report for browsing context group switch when navigating to a COOP response with responseCOOP, "reporting", responseURL, currentCOOPEnforcementResult's url, currentCOOPEnforcementResult's origin, responseOrigin, and referrer.
      - 5b.ii. Queue a violation report for browsing context group switch when navigating away from a COOP response with currentCOOPEnforcementResult's opener policy, "reporting", currentCOOPEnforcementResult's url, responseURL, currentCOOPEnforcementResult's origin, responseOrigin, and currentCOOPEnforcementResult's current context is navigation source.
  - 6. Return newCOOPEnforcementResult.

- [x] [obtain a browsing context to use for a navigation response](https://html.spec.whatwg.org/#obtain-browsing-context-navigation) — To obtain a browsing context to use for a navigation response, given navigation params navigationParams:
  - 1. Let browsingContext be navigationParams's navigable's active browsing context.
  - 2. If browsingContext is not a top-level browsing context, then return browsingContext.
  - 3. Let coopEnforcementResult be navigationParams's COOP enforcement result.
  - 4. Let swapGroup be coopEnforcementResult's needs a browsing context group switch.
  - 5. Let sourceOrigin be browsingContext's active document's origin.
  - 6. Let destinationOrigin be navigationParams's origin.
  - 7. If sourceOrigin is not same site with destinationOrigin:
    - 7a. If either of sourceOrigin or destinationOrigin have a scheme that is not an HTTP(S) scheme and the user agent considers it necessary for sourceOrigin and destinationOrigin to be isolated from each other (for implementation-defined reasons), optionally set swapGroup to true.
    - 7b. If navigationParams's user involvement is "browser UI", optionally set swapGroup to true.
  - 8. If browsingContext's group's browsing context set's size is 1, optionally set swapGroup to true.
  - 9. If swapGroup is false:
    - 9a. If coopEnforcementResult's would need a browsing context group switch due to report-only is true, set browsingContext's virtual browsing context group ID to a new unique identifier.
    - 9b. Return browsingContext.
  - 10. Let newBrowsingContext be the first return value of creating a new top-level browsing context and document.
  - 11. Let navigationCOOP be navigationParams's cross-origin opener policy.
  - 12. If navigationCOOP's value is "same-origin-plus-COEP", then set newBrowsingContext's group's cross-origin isolation mode to either "logical" or "concrete". The choice of which is implementation-defined.
  - 13. Let sandboxFlags be a clone of navigationParams's final sandboxing flag set.
  - 14. If sandboxFlags is not empty:
    - 14a. Assert: navigationCOOP's value is "unsafe-none".
    - 14b. Assert: newBrowsingContext's popup sandboxing flag set is empty.
    - 14c. Set newBrowsingContext's popup sandboxing flag set to sandboxFlags.
  - 15. Return newBrowsingContext.

##### [7.1.3.3 Reporting](https://html.spec.whatwg.org/#coop-reporting)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: accessor-accessed-relationship accessor-accessed-opener accessor-accessed-openee accessor-accessed-none -->

- [x] [check if an access between two browsing contexts should be reported](https://html.spec.whatwg.org/#coop-check-access-report) — To check if an access between two browsing contexts should be reported, given two browsing contexts accessor and accessed, a JavaScript property name P, and an environment settings object environment:
  - 1. If P is not a cross-origin accessible window property name, then return.
  - 2. Assert: accessor's active document and accessed's active document are both fully active.
  - 3. Let accessorTopDocument be accessor's top-level browsing context's active document.
  - 4. Let accessorInclusiveAncestorOrigins be the list obtained by taking the origin of the active document of each of accessor's active document's inclusive ancestor navigables.
  - 5. Let accessedTopDocument be accessed's top-level browsing context's active document.
  - 6. Let accessedInclusiveAncestorOrigins be the list obtained by taking the origin of the active document of each of accessed's active document's inclusive ancestor navigables.
  - 7. If any of accessorInclusiveAncestorOrigins are not same origin with accessorTopDocument's origin, or if any of accessedInclusiveAncestorOrigins are not same origin with accessedTopDocument's origin, then return.
  - 8. If accessor's top-level browsing context's virtual browsing context group ID is accessed's top-level browsing context's virtual browsing context group ID, then return.
  - 9. Let accessorAccessedRelationship be a new accessor-accessed relationship with value none.
  - 10. If accessed's top-level browsing context's opener browsing context is accessor or is an ancestor of accessor, then set accessorAccessedRelationship to accessor is opener.
  - 11. If accessor's top-level browsing context's opener browsing context is accessed or is an ancestor of accessed, then set accessorAccessedRelationship to accessor is openee.
  - 12. Queue violation reports for accesses, given accessorAccessedRelationship, accessorTopDocument's opener policy, accessedTopDocument's opener policy, accessor's active document's URL, accessed's active document's URL, accessor's top-level browsing context's initial URL, accessed's top-level browsing context's initial URL, accessor's active document's origin, accessed's active document's origin, acc…

- [x] [sanitize a URL to send in a report](https://html.spec.whatwg.org/#sanitize-url-report) — To sanitize a URL to send in a report given a URL url:
  - 1. Let sanitizedURL be a copy of url.
  - 2. Set the username given sanitizedURL and the empty string.
  - 3. Set the password given sanitizedURL and the empty string.
  - 4. Return the serialization of sanitizedURL with exclude fragment set to true.

- [x] [queue a violation report for browsing context group switch when navigating to a COOP response](https://html.spec.whatwg.org/#coop-violation-navigation-to) — To queue a violation report for browsing context group switch when navigating to a COOP response given an opener policy coop, a string disposition, a URL coopURL, a URL previousResponseURL, two origins coopOrigin and previousResponseOrigin, and a referrer referrer:
  - 1. If coop's reporting endpoint is null, return.
  - 2. Let coopValue be coop's value.
  - 3. If disposition is "reporting", then set coopValue to coop's report-only value.
  - 4. Let serializedReferrer be an empty string.
  - 5. If referrer is a URL, set serializedReferrer to the serialization of referrer.
  - 6. Let body be a new object containing the following properties:
  - 7. Queue body as "coop" for coop's reporting endpoint with coopURL.

- [x] [queue a violation report for browsing context group switch when navigating away from a COOP response](https://html.spec.whatwg.org/#coop-violation-navigation-from) — To queue a violation report for browsing context group switch when navigating away from a COOP response given an opener policy coop, a string disposition, a URL coopURL, a URL nextResponseURL, two origins coopOrigin and nextResponseOrigin, and a boolean isCOOPResponseNavigationSource:
  - 1. If coop's reporting endpoint is null, return.
  - 2. Let coopValue be coop's value.
  - 3. If disposition is "reporting", then set coopValue to coop's report-only value.
  - 4. Let body be a new object containing the following properties:
  - 5. Queue body as "coop" for coop's reporting endpoint with coopURL.

- [x] [queue violation reports for accesses](https://html.spec.whatwg.org/#coop-violation-access) — To queue violation reports for accesses, given an accessor-accessed relationship accessorAccessedRelationship, two opener policies accessorCOOP and accessedCOOP, four URLs accessorURL, accessedURL, accessorInitialURL, accessedInitialURL, four origins accessorOrigin, accessedOrigin, accessorCreatorO…
  - 1. If accessorAccessedRelationship is accessor is opener:
    - 1a. Queue a violation report for access to an opened window, given accessorCOOP, accessorURL, accessedURL, accessedInitialURL, accessorOrigin, accessedOrigin, accessedCreatorOrigin, propertyName, and environment.
    - 1b. Queue a violation report for access from the opener, given accessedCOOP, accessedURL, accessorURL, accessedOrigin, accessorOrigin, propertyName, and accessedReferrer.
  - 2. Otherwise, if accessorAccessedRelationship is accessor is openee:
    - 2a. Queue a violation report for access to the opener, given accessorCOOP, accessorURL, accessedURL, accessorOrigin, accessedOrigin, propertyName, accessorReferrer, and environment.
    - 2b. Queue a violation report for access from an opened window, given accessedCOOP, accessedURL, accessorURL, accessorInitialURL, accessedOrigin, accessorOrigin, accessorCreatorOrigin, and propertyName.
  - 3. Otherwise:
    - 3a. Queue a violation report for access to another window, given accessorCOOP, accessorURL, accessedURL, accessorOrigin, accessedOrigin, propertyName, and environment.
    - 3b. Queue a violation report for access from another window, given accessedCOOP, accessedURL, accessorURL, accessedOrigin, accessorOrigin, and propertyName.

- [x] [queue a violation report for access to the opener](https://html.spec.whatwg.org/#coop-violation-access-to-opener) — To queue a violation report for access to the opener, given an opener policy coop, two URLs coopURL and openerURL, two origins coopOrigin and openerOrigin, a string propertyName, a referrer referrer, and an environment settings object environment:
  - 1. If coop's reporting endpoint is null, then return.
  - 2. Let sourceFile, lineNumber, and columnNumber be the relevant script URL and problematic position which triggered this report.
  - 3. Let serializedReferrer be an empty string.
  - 4. If referrer is a URL, set serializedReferrer to the serialization of referrer.
  - 5. Let body be a new object containing the following properties:
  - 6. Queue body as "coop" for coop's reporting endpoint with coopURL and environment.

- [x] [queue a violation report for access to an opened window](https://html.spec.whatwg.org/#coop-violation-access-to-opened) — To queue a violation report for access to an opened window, given an opener policy coop, three URLs coopURL, openedWindowURL and initialWindowURL, three origins coopOrigin, openedWindowOrigin, and openerInitialOrigin, a string propertyName, and an environment settings object environment:
  - 1. If coop's reporting endpoint is null, then return.
  - 2. Let sourceFile, lineNumber, and columnNumber be the relevant script URL and problematic position which triggered this report.
  - 3. Let body be a new object containing the following properties:
  - 4. Queue body as "coop" for coop's reporting endpoint with coopURL and environment.

- [x] [queue a violation report for access to another window](https://html.spec.whatwg.org/#coop-violation-access-to-other) — To queue a violation report for access to another window, given an opener policy coop, two URLs coopURL and otherURL, two origins coopOrigin and otherOrigin, a string propertyName, and an environment settings object environment:
  - 1. If coop's reporting endpoint is null, then return.
  - 2. Let sourceFile, lineNumber, and columnNumber be the relevant script URL and problematic position which triggered this report.
  - 3. Let body be a new object containing the following properties:
  - 4. Queue body as "coop" for coop's reporting endpoint with coopURL and environment.

- [x] [queue a violation report for access from the opener](https://html.spec.whatwg.org/#coop-violation-access-from-opener) — To queue a violation report for access from the opener, given an opener policy coop, two URLs coopURL and openerURL, two origins coopOrigin and openerOrigin, a string propertyName, and a referrer referrer:
  - 1. If coop's reporting endpoint is null, return.
  - 2. Let serializedReferrer be an empty string.
  - 3. If referrer is a URL, set serializedReferrer to the serialization of referrer.
  - 4. Let body be a new object containing the following properties:
  - 5. Queue body as "coop" for coop's reporting endpoint with coopURL.

- [x] [queue a violation report for access from an opened window](https://html.spec.whatwg.org/#coop-violation-access-from-opened) — To queue a violation report for access from an opened window, given an opener policy coop, three URLs coopURL, openedWindowURL and initialWindowURL, three origins coopOrigin, openedWindowOrigin, and openerInitialOrigin, and a string propertyName:
  - 1. If coop's reporting endpoint is null, return.
  - 2. Let body be a new object containing the following properties:
  - 3. Queue body as "coop" for coop's reporting endpoint with coopURL.

- [x] [queue a violation report for access from another window](https://html.spec.whatwg.org/#coop-violation-access-from-other) — To queue a violation report for access from another window, given an opener policy coop, two URLs coopURL and otherURL, two origins coopOrigin and otherOrigin, and a string propertyName:
  - 1. If coop's reporting endpoint is null, return.
  - 2. Let body be a new object containing the following properties:
  - 3. Queue body as "coop" for coop's reporting endpoint with coopURL.

#### [7.1.4 Cross-origin embedder policies](https://html.spec.whatwg.org/#coep)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: embedder-policy-value coep-unsafe-none coep-require-corp coep-credentialless embedder-policy embedder-policy-value-2 embedder-policy-reporting-endpoint embedder-policy-report-only-value embedder-policy-report-only-reporting-endpoint coep-report-type -->

- [x] [compatible with cross-origin isolation](https://html.spec.whatwg.org/#compatible-with-cross-origin-isolation) — An embedder policy value is compatible with cross-origin isolation if it is "credentialless" or "require-corp".

##### [7.1.4.1 The headers](https://html.spec.whatwg.org/#the-coep-headers)

Status: n/a (no browsing context groups or HTTP)
<!-- definitions: cross-origin-embedder-policy cross-origin-embedder-policy-report-only coep-report-to -->

- [x] [obtain an embedder policy](https://html.spec.whatwg.org/#obtain-an-embedder-policy) — To obtain an embedder policy from a response response and an environment environment:
  - 1. Let policy be a new embedder policy.
  - 2. If environment is a non-secure context, then return policy.
  - 3. Let parsedItem be the result of getting a structured field value with `Cross-Origin-Embedder-Policy` and "item" from response's header list.
  - 4. If parsedItem is non-null, parsedItem[0] is a token, and parsedItem[0] is compatible with cross-origin isolation:
    - 4a. Set policy's value to parsedItem[0].
    - 4b. If parsedItem[1]["report-to"] exists and it is a string, then set policy's reporting endpoint to parsedItem[1]["report-to"].
  - 5. Set parsedItem to the result of getting a structured field value with `Cross-Origin-Embedder-Policy-Report-Only` and "item" from response's header list.
  - 6. If parsedItem is non-null, parsedItem[0] is a token, and parsedItem[0] is compatible with cross-origin isolation:
    - 6a. Set policy's report-only value to parsedItem[0].
    - 6b. If parsedItem[1]["report-to"] exists and it is a string, then set policy's report-only reporting endpoint to parsedItem[1]["report-to"].
  - 7. Return policy.

##### [7.1.4.2 Embedder policy checks](https://html.spec.whatwg.org/#embedder-policy-checks)

Status: n/a (no browsing context groups or HTTP)

- [x] [check a navigation response's adherence to its embedder policy](https://html.spec.whatwg.org/#check-a-navigation-response's-adherence-to-its-embedder-policy) — To check a navigation response's adherence to its embedder policy given a response response, a navigable navigable, and an embedder policy responsePolicy:
  - 1. If navigable is not a child navigable, then return true.
  - 2. Let parentPolicy be navigable's container document's policy container's embedder policy.
  - 3. If parentPolicy's report-only value is compatible with cross-origin isolation and responsePolicy's value is not, then queue a cross-origin embedder policy inheritance violation with response, "navigation", parentPolicy's report-only reporting endpoint, "reporting", and navigable's container document's relevant settings object.
  - 4. If parentPolicy's value is not compatible with cross-origin isolation or responsePolicy's value is compatible with cross-origin isolation, then return true.
  - 5. Queue a cross-origin embedder policy inheritance violation with response, "navigation", parentPolicy's reporting endpoint, "enforce", and navigable's container document's relevant settings object.
  - 6. Return false.

- [x] [check a global object's embedder policy](https://html.spec.whatwg.org/#check-a-global-object's-embedder-policy) — To check a global object's embedder policy given a WorkerGlobalScope workerGlobalScope, an environment settings object owner, and a response response:
  - 1. If workerGlobalScope is not a DedicatedWorkerGlobalScope object, then return true.
  - 2. Let policy be workerGlobalScope's embedder policy.
  - 3. Let ownerPolicy be owner's policy container's embedder policy.
  - 4. If ownerPolicy's report-only value is compatible with cross-origin isolation and policy's value is not, then queue a cross-origin embedder policy inheritance violation with response, "worker initialization", ownerPolicy's report-only reporting endpoint, "reporting", and owner.
  - 5. If ownerPolicy's value is not compatible with cross-origin isolation or policy's value is compatible with cross-origin isolation, then return true.
  - 6. Queue a cross-origin embedder policy inheritance violation with response, "worker initialization", ownerPolicy's reporting endpoint, "enforce", and owner.
  - 7. Return false.

- [x] [queue a cross-origin embedder policy inheritance violation](https://html.spec.whatwg.org/#queue-a-cross-origin-embedder-policy-inheritance-violation) — To queue a cross-origin embedder policy inheritance violation given a response response, a string type, a string endpoint, a string disposition, and an environment settings object settings:
  - 1. Let serialized be the result of serializing a response URL for reporting with response.
  - 2. Let body be a new object containing the following properties:
  - 3. Queue body as the "coep" report type for endpoint on settings.

#### [7.1.5 Sandboxing](https://html.spec.whatwg.org/#sandboxing)

Status: n/a (no sandboxing: no iframes or Content Security Policy)
<!-- definitions: sandboxing-flag-set sandboxed-navigation-browsing-context-flag one-permitted-sandboxed-navigator sandboxed-auxiliary-navigation-browsing-context-flag sandboxed-top-level-navigation-without-user-activation-browsing-context-flag sandboxed-top-level-navigation-with-user-activation-browsing-context-flag sandboxed-origin-browsing-context-flag sandboxed-forms-browsing-context-flag sandboxed-pointer-lock-browsing-context-flag sandboxed-scripts-browsing-context-flag sandboxed-automatic-features-browsing-context-flag sandboxed-document.domain-browsing-context-flag sandbox-propagates-to-auxiliary-browsing-contexts-flag sandboxed-modals-flag sandboxed-orientation-lock-browsing-context-flag sandboxed-presentation-browsing-context-flag sandboxed-downloads-browsing-context-flag sandboxed-custom-protocols-navigation-browsing-context-flag attr-iframe-sandbox-allow-popups attr-iframe-sandbox-allow-top-navigation attr-iframe-sandbox-allow-top-navigation-by-user-activation attr-iframe-sandbox-allow-same-origin attr-iframe-sandbox-allow-forms attr-iframe-sandbox-allow-pointer-lock attr-iframe-sandbox-allow-scripts attr-iframe-sandbox-allow-popups-to-escape-sandbox attr-iframe-sandbox-allow-modals attr-iframe-sandbox-allow-orientation-lock attr-iframe-sandbox-allow-presentation attr-iframe-sandbox-allow-downloads attr-iframe-sandbox-allow-top-navigation-to-custom-protocols popup-sandboxing-flag-set iframe-sandboxing-flag-set active-sandboxing-flag-set -->

- [x] [parse a sandboxing directive](https://html.spec.whatwg.org/#parse-a-sandboxing-directive) — When the user agent is to parse a sandboxing directive, given a string input and a sandboxing flag set output, it must run the following steps:
  - 1. Split input on ASCII whitespace, to obtain tokens.
  - 2. Let output be empty.
  - 3. Add the following flags to output: The sandboxed navigation browsing context flag.; The sandboxed auxiliary navigation browsing context flag, unless tokens contains the allow-popups keyword.; The sandboxed top-level navigation without user activation browsing context flag, unless tokens contains the allow-top-navigation keyword.; The sandboxed top-level navigation with user activation browsing co…

- [x] [CSP-derived sandboxing flags](https://html.spec.whatwg.org/#csp-derived-sandboxing-flags) — Every CSP list cspList has CSP-derived sandboxing flags, which is a sandboxing flag set. It is the return value of the following algorithm:
  - 1. Let directives be an empty ordered set.
  - 2. For each policy in cspList:
    - 2a. If policy's disposition is not "enforce", then continue.
    - 2b. If policy's directive set contains a directive whose name is "sandbox", then append that directive to directives.
  - 3. If directives is empty, then return an empty sandboxing flag set.
  - 4. Let directive be directives[directives's size − 1].
  - 5. Return the result of parsing the sandboxing directive directive.

- [x] [determine the creation sandboxing flags](https://html.spec.whatwg.org/#determining-the-creation-sandboxing-flags) — To determine the creation sandboxing flags for a browsing context browsing context, given null or an element embedder, return the union of the flags that are present in the following sandboxing flag sets:

#### [7.1.6 iframe element referrer policy](https://html.spec.whatwg.org/#iframe-element-referrer-policy)

Status: n/a (no frame tree)

- [x] [determine the iframe element referrer policy](https://html.spec.whatwg.org/#determining-the-iframe-element-referrer-policy) — To determine the iframe element referrer policy given an element-or-null embedder:
  - 1. If embedder is an iframe element, then return embedder's referrerpolicy attribute's state's corresponding keyword.
  - 2. Return the empty string.

#### [7.1.7 Policy containers](https://html.spec.whatwg.org/#policy-containers)

Status: n/a (no policy containers: no Content Security Policy or HTTP)
<!-- definitions: policy-container policy-container-csp-list policy-container-embedder-policy policy-container-referrer-policy policy-container-integrity-policy policy-container-report-only-integrity-policy -->

- [x] [clone a policy container](https://html.spec.whatwg.org/#clone-a-policy-container) — To clone a policy container given a policy container policyContainer:
  - 1. Let clone be a new policy container.
  - 2. For each policy in policyContainer's CSP list, append a copy of policy into clone's CSP list.
  - 3. Set clone's embedder policy to a copy of policyContainer's embedder policy.
  - 4. Set clone's referrer policy to policyContainer's referrer policy.
  - 5. Set clone's integrity policy to a copy of policyContainer's integrity policy.
  - 6. Return clone.

- [x] [requires storing the policy container in history](https://html.spec.whatwg.org/#requires-storing-the-policy-container-in-history) — To determine whether a URL url requires storing the policy container in history:
  - 1. If url's scheme is "blob", then return false.
  - 2. If url is local, then return true.
  - 3. Return false.

- [x] [create a policy container from a fetch response](https://html.spec.whatwg.org/#creating-a-policy-container-from-a-fetch-response) — To create a policy container from a fetch response given a response response and an environment-or-null environment:
  - 1. If response's URL's scheme is "blob", then return a clone of response's URL's blob URL entry's environment's policy container.
  - 2. Let result be a new policy container.
  - 3. Set result's CSP list to the result of parsing a response's Content Security Policies given response.
  - 4. If environment is non-null, then set result's embedder policy to the result of obtaining an embedder policy given response and environment. Otherwise, set it to "unsafe-none".
  - 5. Set result's referrer policy to the result of parsing the `Referrer-Policy` header given response. [REFERRERPOLICY]
  - 6. Parse Integrity-Policy headers with response and result.
  - 7. Return result.

- [x] [determine navigation params policy container](https://html.spec.whatwg.org/#determining-navigation-params-policy-container) — To determine navigation params policy container given a URL responseURL and four policy container-or-nulls historyPolicyContainer, initiatorPolicyContainer, parentPolicyContainer, and responsePolicyContainer:
  - 1. If historyPolicyContainer is not null:
    - 1a. Assert: responseURL requires storing the policy container in history.
    - 1b. Return a clone of historyPolicyContainer.
  - 2. If responseURL is about:srcdoc:
    - 2a. Assert: parentPolicyContainer is not null.
    - 2b. Return a clone of parentPolicyContainer.
  - 3. If responseURL is local and initiatorPolicyContainer is not null, then return a clone of initiatorPolicyContainer.
  - 4. If responsePolicyContainer is not null, then return responsePolicyContainer.
  - 5. Return a new policy container.

- [x] [initialize a worker global scope's policy container](https://html.spec.whatwg.org/#initialize-worker-policy-container) — To initialize a worker global scope's policy container given a WorkerGlobalScope workerGlobalScope, a response response, and an environment environment:
  - 1. If workerGlobalScope's url is local but its scheme is not "blob":
    - 1a. Assert: workerGlobalScope's owner set's size is 1.
    - 1b. Set workerGlobalScope's policy container to a clone of workerGlobalScope's owner set[0]'s relevant settings object's policy container.
  - 2. Otherwise, set workerGlobalScope's policy container to the result of creating a policy container from a fetch response given response and environment.
