# Port graph

Entries: `css/values/primitives/CSSURL.cpp`

- Files to port: **13**
- Boundary `dom`: 1
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 2
- Boundary `wtf`: 21
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 5 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h |
| 4 | `css/CSSValue.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 3 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSURLModifiers.h |
| 2 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 2 | `css/parser/CSSParserMode.h` |  | css/parser/CSSParserContext.h |
| 2 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 1 | `css/CSSMarkup.h` |  | css/values/primitives/CSSURL.cpp |
| 1 | `css/parser/CSSParserContext.h` |  | css/values/primitives/CSSURL.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/values/primitives/CSSURL.cpp |
| 1 | `css/values/primitives/CSSURL.h` |  | css/values/primitives/CSSURL.cpp |
| 0 | `css/values/primitives/CSSURL.cpp` |  | (entry) |

## Boundaries

### dom (1)

- `dom/Document.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 2 file(s)

### platform (2)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)

### wtf (21)

- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 2 file(s)
- `wtf/Vector.h` ← 3 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 2 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
