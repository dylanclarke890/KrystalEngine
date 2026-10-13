# Port graph

Entries: `css/CSSFontFaceSrcValue.cpp`

- Files to port: **11**
- Boundary `dom`: 1
- Boundary `loader`: 8
- Boundary `platform`: 3
- Boundary `svg`: 1
- Boundary `wtf`: 20
- Unresolved: 2

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 6 | `css/values/CSSValueConcepts.h` |  | css/values/CSSValueAggregates.h |
| 5 | `css/ComputedStyleDependencies.h` |  | css/values/CSSValueTypes.h |
| 5 | `css/values/CSSValueAggregates.h` |  | css/values/CSSValueTypes.h |
| 4 | `css/values/CSSValueTypes.h` |  | css/values/primitives/CSSURLModifiers.h |
| 3 | `css/values/primitives/CSSURLModifiers.h` |  | css/values/primitives/CSSURL.h |
| 2 | `css/CSSValue.h` |  | css/CSSFontFaceSrcValue.h, css/values/CSSValueTypes.h |
| 2 | `css/values/primitives/CSSURL.h` |  | css/CSSFontFaceSrcValue.h |
| 1 | `css/CSSFontFaceSrcValue.h` |  | css/CSSFontFaceSrcValue.cpp |
| 1 | `css/CSSMarkup.h` |  | css/CSSFontFaceSrcValue.cpp |
| 1 | `css/values/CSSSerializationContext.h` |  | css/CSSFontFaceSrcValue.cpp |
| 0 | `css/CSSFontFaceSrcValue.cpp` |  | (entry) |

## Boundaries

### dom (1)

- `dom/ScriptExecutionContext.h` ← 1 file(s)

### loader (8)

- `loader/LoadedFromOpaqueSource.h` ← 1 file(s)
- `loader/ResourceLoaderOptions.h` ← 1 file(s)
- `loader/cache/CachedFont.h` ← 1 file(s)
- `loader/cache/CachedFontLoadRequest.h` ← 1 file(s)
- `loader/cache/CachedResourceHandle.h` ← 1 file(s)
- `loader/cache/CachedResourceLoader.h` ← 1 file(s)
- `loader/cache/CachedResourceRequest.h` ← 1 file(s)
- `loader/cache/CachedResourceRequestInitiatorTypes.h` ← 1 file(s)

### platform (3)

- `platform/RectCorners.h` ← 1 file(s)
- `platform/RectEdges.h` ← 1 file(s)
- `platform/graphics/FontCustomPlatformData.h` ← 1 file(s)

### svg (1)

- `svg/SVGFontFaceElement.h` ← 1 file(s)

### wtf (20)

- `wtf/EnumSet.h` ← 1 file(s)
- `wtf/FixedVector.h` ← 1 file(s)
- `wtf/HashMap.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/ListHashSet.h` ← 1 file(s)
- `wtf/Markable.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefCountedFixedVector.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/StdLibExtras.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/URL.h` ← 1 file(s)
- `wtf/Vector.h` ← 4 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/AtomString.h` ← 1 file(s)
- `wtf/text/MakeString.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 2 file(s)
- `wtf/text/TextStream.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 3 file(s)

## Problems

### unresolved (2)

- `CSSPropertyNames.h` ← 1 file(s)
- `CSSValueKeywords.h` ← 1 file(s)
