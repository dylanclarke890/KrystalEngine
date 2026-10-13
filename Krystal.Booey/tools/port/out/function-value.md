# Port graph

Entries: `css/CSSFunctionValue.cpp`

- Files to port: **4**
- Boundary `external`: 1
- Boundary `wtf`: 11
- Unresolved: 1

## Files to port

Deepest first (rough leaves-first order).

| Hops | File | Cond | Included by |
|---|---|---|---|
| 3 | `css/CSSValue.h` |  | css/CSSValueList.h |
| 2 | `css/CSSValueList.h` |  | css/CSSFunctionValue.h |
| 1 | `css/CSSFunctionValue.h` |  | css/CSSFunctionValue.cpp |
| 0 | `css/CSSFunctionValue.cpp` |  | (entry) |

## Boundaries

### external (1)

- `unicode/umachine.h` ← 1 file(s)

### wtf (11)

- `wtf/Hasher.h` ← 1 file(s)
- `wtf/IterationStatus.h` ← 1 file(s)
- `wtf/MallocSpan.h` ← 1 file(s)
- `wtf/NoVirtualDestructorBase.h` ← 1 file(s)
- `wtf/Noncopyable.h` ← 1 file(s)
- `wtf/RefPtr.h` ← 1 file(s)
- `wtf/TypeCasts.h` ← 1 file(s)
- `wtf/Vector.h` ← 1 file(s)
- `wtf/text/ASCIILiteral.h` ← 1 file(s)
- `wtf/text/StringBuilder.h` ← 1 file(s)
- `wtf/text/WTFString.h` ← 1 file(s)

## Problems

### unresolved (1)

- `CSSValueKeywords.h` ← 1 file(s)
