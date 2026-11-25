# HTML review checklist: 2.7 Safe passing of structured data

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 2 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html structured-data` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [2.7 Safe passing of structured data](https://html.spec.whatwg.org/multipage/structured-data.html#safe-passing-of-structured-data)

Status: n/a (no JavaScript values)

#### [2.7.1 Serializable objects](https://html.spec.whatwg.org/multipage/structured-data.html#serializable-objects)

Status: n/a (no JavaScript values)
<!-- definitions: serializable serialization-steps deserialization-steps -->

#### [2.7.2 Transferable objects](https://html.spec.whatwg.org/multipage/structured-data.html#transferable-objects)

Status: n/a (no JavaScript values)
<!-- definitions: transferable transfer-steps transfer-receiving-steps detached -->

#### [2.7.3 Restricting to the same agent cluster](https://html.spec.whatwg.org/multipage/structured-data.html#restricting-to-the-same-agent-cluster)

Status: n/a (no JavaScript values)
<!-- definitions: sameagentcluster -->

- [x] [restrict](https://html.spec.whatwg.org/multipage/structured-data.html#restrict-to-the-same-agent-cluster) — To restrict a Record record to the same agent cluster, set record.[[AgentCluster]] to the surrounding agent's agent cluster.

#### [2.7.4 StructuredSerializeInternal ( value, forStorage [ , memory ] )](https://html.spec.whatwg.org/multipage/structured-data.html#structuredserializeinternal)

Status: n/a (no JavaScript values)
<!-- definitions: sub-serialization -->

#### [2.7.5 StructuredSerialize ( value )](https://html.spec.whatwg.org/multipage/structured-data.html#structuredserialize)

Status: n/a (no JavaScript values)

#### [2.7.6 StructuredSerializeForStorage ( value )](https://html.spec.whatwg.org/multipage/structured-data.html#structuredserializeforstorage)

Status: n/a (no JavaScript values)

#### [2.7.7 StructuredDeserialize ( serialized, targetRealm [ , memory ] )](https://html.spec.whatwg.org/multipage/structured-data.html#structureddeserialize)

Status: n/a (no JavaScript values)
<!-- definitions: sub-deserialization -->

#### [2.7.8 StructuredSerializeWithTransfer ( value, transferList )](https://html.spec.whatwg.org/multipage/structured-data.html#structuredserializewithtransfer)

Status: n/a (no JavaScript values)

#### [2.7.9 StructuredDeserializeWithTransfer ( serializeWithTransferResult, targetRealm )](https://html.spec.whatwg.org/multipage/structured-data.html#structureddeserializewithtransfer)

Status: n/a (no JavaScript values)

#### [2.7.10 Performing serialization and transferring from other specifications](https://html.spec.whatwg.org/multipage/structured-data.html#performing-structured-clones-from-other-specifications)

Status: n/a (no JavaScript values)

#### [2.7.11 Structured cloning API](https://html.spec.whatwg.org/multipage/structured-data.html#structured-cloning)

Status: n/a (no structured serialization, and no use for cloning host values)

- [x] [WindowOrWorkerGlobalScope.structuredClone(value, options)](https://html.spec.whatwg.org/multipage/structured-data.html#dom-structuredclone) — The structuredClone(value, options) method steps are:
  - 1. Let serialized be ? StructuredSerializeWithTransfer(value, options["transfer"]).
  - 2. Let deserializeRecord be ? StructuredDeserializeWithTransfer(serialized, this's relevant realm).
  - 3. Return deserializeRecord.[[Deserialized]].
