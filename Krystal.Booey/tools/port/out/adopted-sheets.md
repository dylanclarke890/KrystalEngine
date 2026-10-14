# Port graph

Entries: `css/CSSStyleSheetObservableArray.cpp`

- Files to port: **15**
- Boundary `bindings`: 2
- Boundary `dom`: 5
- Boundary `external`: 1
- Boundary `loader`: 1
- Boundary `platform`: 1
- Boundary `wtf`: 23
- Unresolved: 3

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/StyleRuleType.h` |  | css/parser/CSSParserContext.h |
| 6 | `css/values/primitives/CSSPrimitiveNumericRange.h` |  | css/CSSToLengthConversionData.h |
| 5 | `css/CSSToLengthConversionData.h` |  | css/query/GenericMediaQueryTypes.h |
| 5 | `css/CSSValue.h` |  | css/query/GenericMediaQueryTypes.h |
| 5 | `css/parser/CSSParserContext.h` |  | css/MediaQueryParserContext.h |
| 4 | `css/MediaQueryParserContext.h` |  | css/MediaList.h |
| 4 | `css/parser/CSSParserMode.h` |  | css/StyleSheet.h, css/parser/CSSParserContext.h |
| 4 | `css/query/GenericMediaQueryTypes.h` |  | css/query/MediaQuery.h |
| 3 | `css/CSSRuleList.h` |  | css/CSSStyleSheet.h |
| 3 | `css/MediaList.h` |  | css/CSSStyleSheet.h |
| 3 | `css/StyleSheet.h` |  | css/CSSStyleSheet.h |
| 3 | `css/query/MediaQuery.h` |  | css/CSSStyleSheet.h, css/MediaList.h |
| 2 | `css/CSSStyleSheet.h` |  | css/CSSStyleSheetObservableArray.h |
| 1 | `css/CSSStyleSheetObservableArray.h` |  | css/CSSStyleSheetObservableArray.cpp |
| 0 | `css/CSSStyleSheetObservableArray.cpp` |  | (entry) |

## Boundaries

### bindings (2)

- `bindings/js/JSDOMConvert.h` ← 1 file(s)
- `bindings/js/JSObservableArray.h` ← 1 file(s)

### dom (5)

- `dom/Document.h` ← 1 file(s)
- `dom/Element.h` ← 1 file(s)
- `dom/ExceptionOr.h` ← 1 file(s)
- `dom/ShadowRoot.h` ← 1 file(s)
- `dom/TreeScope.h` ← 1 file(s)

### external (1)

- `pal/text/TextEncoding.h` ← 1 file(s)

### loader (1)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)

### platform (1)

- `platform/CommonAtomStrings.h` ← 1 file(s)

### wtf (23)

- `wtf/AbstractRefCounted.h` ← 1 file(s)
- `wtf/Assertions.h` ← 1 file(s)
- `wtf/CheckedPtr.h` ← 2 file(s)
- `wtf/Forward.h` ← 2 file(s)
- `wtf/HashFunctions.h` ← 1 file(s)
- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/MathExtras.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 2 file(s)
- `wtf/OptionSet.h` ← 1 file(s)
- `wtf/RefCounted.h` ← 2 file(s)
- `wtf/RefPtr.h` ← 2 file(s)
- `wtf/TZoneMallocInlines.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 2 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 3 file(s)
- `wtf/WeakHashSet.h` ← 1 file(s)
- `wtf/WeakPtr.h` ← 2 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/AtomStringHash.h` ← 1 file(s)
- `wtf/text/TextPosition.h` ← 1 file(s)

## Problems

### unresolved (3)

- `CSSPropertyNames.h` ← 2 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
- `JSCSSStyleSheet.h` ← 1 file(s)
