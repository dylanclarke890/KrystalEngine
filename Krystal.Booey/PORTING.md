# WebKit CSS port ledger

- **Upstream:** `C:\Users\dylan\repos\WebKit` @ `300e80c70fd3` (2025-12-26). Never update the clone mid-port.
- **Paths** are relative to `Source/WebCore` (WebKit) and `Krystal.Booey/include/Krystal.Booey` (Krystal) unless stated.
- **Regenerate a slice:** `python tools/port/graph.py <entries...> --headers-only --out tools/port/out/<slice>.md`

## Verdicts

| Verdict | Meaning |
|---|---|
| `?` | Undecided. |
| `have?` | Krystal file matched by name only, needs confirming. `have*?` = matched and looks simplified. |
| `have` | Already ported; matches upstream closely enough to diff against. |
| `have*` | Already exists, but simplified/restructured or written from the spec; don't expect a line-by-line match. |
| `port` | Will port. |
| `stub` | Needed for compilation, but only a minimal stand-in. |
| `wont` | Not supported; remove the dependency at the include site. |

Status (for `port`/`have*`) is tracked in Notes: `compiles`, `tested`.

## Where we are (2026-10-12)

**Process per file/slice:** run `tools/port/graph.py` from the WebKit entry (`--headers-only`; also check the `.cpp`'s own includes, which headers-only misses), compare the Krystal file with upstream, add missing methods, fix bugs, record differences here, add tests where something can be instantiated, build, run tests. The user reviews each step; don't port ahead without being asked.

**Build:** only files listed in `KRYSTAL_BOOEY_CSS_HEADERS`/`KRYSTAL_BOOEY_CSS_SOURCES` (`Krystal.Booey/CMakeLists.txt`) are built; everything else under `CSS/`/`Style/` is on disk but unreviewed. Code generation runs in the build (`codegen/`). Build with Visual Studio's bundled CMake and the `windows-x64` preset (`cmake --build --preset windows-x64-debug --target Krystal.Booey.Tests`); run `build/Krystal.Booey.Tests/Debug/Krystal.Booey.Tests.exe` from that folder. If `CMakeLists.txt` is changed and restored, re-run `cmake --preset windows-x64` or the generated project keeps stale sources.

**Done:**
- Slice 1, Tokenizer: `Token`, `Tokenizer`, `Markup`, `KeywordLookup`, generated `PropertyId`/`ValueId` build and are tested.
- Slice 2, Values: `Value` and `PrimitiveValue` reviewed (headers build; `.cpp`s not built), `Units`/`UnitType`/`UnitCategory`, `NumericTypesSerialisation`, `Aggregates` reviewed and tested.
- Last commit: `df981fdd` on `feat/css`. This ledger and `tools/` are not committed.

**Open / next candidates:**
- `Value.cpp` and `PrimitiveValue.cpp` can't build until every `Value` subclass compiles (`VisitDerived`). Next blocker found by a trial compile of `PrimitiveValue.cpp`: `Values/Color/AbsoluteColorResolver.hpp` still includes raw WebKit `Color.h` (via `ValuePool.hpp` → `ColorValue.hpp`).
- `CSS/Values/Concepts.hpp` (`CSSValueConcepts.h`) compiles but isn't reviewed.
- Callers still using the case-sensitive raw lookups: `src/CSS/Parser.cpp:477`, `Consumers/Animations.cpp:154`, `Consumers/WillChange.cpp:48` (switch to `PropertyIdFromName`/`ValueIdFromName`); delete the commented-out originals in `src/CSS/Properties/PropertyParser.cpp`.
- Tokenizer doesn't keep numeric original text or whitespace counts, so `TokenSerialisationMode::CustomProperty` behaves like `Normal` (FIXMEs in `src/CSS/Parser/Token.cpp`).
- Not yet ported, needed later: `LayoutUnit`, `StringBuilder`, `RefCountedFixedVector`, `Style::computeNonCalcLengthDouble`, URL type (check `Krystal.Booey/URL/Url.hpp` first).
- Undecided style question: list wrappers use container-protocol lowercase names (`isEmpty`, `contains`), matching Core's `FixedVector`/`ListHashSet`; `EnumSet` and wrappers use PascalCase.

---

## Translation rules

Applies to every slice. Krystal.Core paths are under `Krystal.Core/include/Krystal.Core/`.

### Strings (UTF-8)

Use the CSS aliases from `CSS/Types/CSSOMString.hpp`, never the Krystal.Core or `dom::` names directly.

| WTF | Krystal | Notes |
|---|---|---|
| `wtf/text/WTFString.h` (`String`) | `CSSOMString` | Alias of `dom::DOMString` (`std::u8string`). |
| `wtf/text/StringView.h` (`StringView`) | `CSSOMStringView` | Alias of `dom::DOMStringView` (`std::u8string_view`). |
| `wtf/text/AtomString.h` (`AtomString`) | `CSSOMStringAtom` | Alias of `dom::DOMStringAtom`; own port of WTF's atom string. |
| `wtf/text/StringBuilder.h` | `port` → `StringBuilder` | Krystal naming; UTF-8 only (drop WebKit's Latin-1/UTF-16 handling). |
| `wtf/text/ASCIILiteral.h` (`"foo"_s`) | `u8"foo"sv` / `CSSOMStringView` | No new type. |
| `wtf/text/ParsingUtilities.h` | `port` on demand → `Text/` | Port only the helpers a slice uses. |
| `wtf/text/StringToIntegerConversion.h` | `port` → `Text/` | `parseInteger` and friends. |
| `wtf/unicode/CharacterNames.h` | `port` on demand → `Text/Unicode.hpp` | Add the named code points as they're needed. |

### Pointers and containers

| WTF | Krystal | Notes |
|---|---|---|
| `wtf/Ref.h` (`Ref<T>`) | `Ref<T>` in `Types/RefPtr.hpp` | `IntrusivePtr` with `IsNullable(false)`. |
| `wtf/RefPtr.h` | `RefPtr<T>` in `Types/RefPtr.hpp` | |
| `wtf/RefCounted.h` | `RefCounted.hpp` | |
| `wtf/RefCountedAndCanMakeWeakPtr.h` | `RefCounted` + `CanMakeWeakPtr` | Inherit from both at each use site; no combined type. |
| `wtf/RefCountedFixedVector.h` | `port` → `Types/` | Alongside `FixedVector`. |
| `wtf/Vector.h` (`Vector<T>`) | `List<T>` in `Types/List.hpp` | `std::vector`. |
| `wtf/Vector.h` (`Vector<T, N>`) | `SmallList<T, N>` in `Types/SmallList.hpp` | Inline capacity; `llvm::SmallVector`. |
| `wtf/FixedVector.h` | `Types/FixedVector.hpp` | |
| `wtf/ListHashSet.h` | `Types/ListHashSet.hpp` | |
| `wtf/EnumSet.h` | `Types/EnumSet.hpp` | |
| `wtf/Markable.h` | `Types/Markable.hpp` | |
| `wtf/NeverDestroyed.h` | `Utils/NeverDestroyed.hpp` | |
| `wtf/Noncopyable.h` | `KRYS_NON_COPYABLE` in `Macros.hpp` | |
| `wtf/IterationStatus.h` | `Krystal.Booey/Core/IterationStatus.hpp` | Lives in Booey, not Core. |
| `wtf/Brigand.h` | vcpkg `brigand` | Already linked by Booey's CMake. |

### Utilities

| WTF | Krystal | Notes |
|---|---|---|
| `wtf/ASCIICType.h` | `have*` → `Text/ASCII.hpp` | Add missing functions there. |
| `wtf/EnumTraits.h` | `have` → `Enum.hpp` | |
| `wtf/HashFunctions.h` | `have` → `Hash.hpp` | |
| `wtf/Hasher.h` | `have*` → `Hash::Combine` in `Hash.hpp` | `computeHash(...)` → `Hash::Combine(...)`; rewrite `add(Hasher&, T)` overloads as hash specialisations. |
| `wtf/MathExtras.h` | `have*` → `Maths/Base.hpp` | Most helpers already there; add the rest on demand. |
| `wtf/StdLibExtras.h` | `port` on demand | Map each helper as it comes up (e.g. `WTFMove` → `krys::move`) and record it here. |
| `wtf/TypeCasts.h` | `have` → `TypeCast.hpp` | |
| `wtf/URL.h` | `port` → URL type | WHATWG URL; port when a slice needs it. `Krystal.Booey/URL/Url.hpp` already exists, check it first. |

### Won't port

Remove these at the include site; don't stub them.

| WTF / WebCore | Reason |
|---|---|
| `wtf/text/TextStream.h` | Debug dumping. |
| `wtf/TZoneMalloc.h` | Allocator annotations (`WTF_MAKE_TZONE_ALLOCATED`); drop the macro. |
| `wtf/Forward.h` | Include the Krystal headers directly. |
| `wtf/NoVirtualDestructorBase.h` | Marker base only; drop it. |
| `css/parser/CSSParserObserver.h`, `CSSParserObserverWrapper.h` | Inspector/CSSOM source-range hooks. Drop observer parameters and code paths. |

---|---|---|
| `wtf/ASCIICType.h` | `Text/ASCII.hpp`? | |
| `wtf/EnumTraits.h` | `Enum.hpp`? / `MagicEnum.hpp`? | |
| `wtf/HashFunctions.h`, `wtf/Hasher.h` | `Hash.hpp`? | |
| `wtf/MathExtras.h` | `Maths/*`? | |
| `wtf/NoVirtualDestructorBase.h` | | |
| `wtf/StdLibExtras.h` | | Grab-bag; port pieces on demand. |
| `wtf/TypeCasts.h` | `TypeCast.hpp`? | |
| `wtf/URL.h` | | |

---

## Slice 1: Tokenizer — done (own implementation)

Entry: `css/parser/CSSTokenizer.cpp`. The Krystal tokenizer is its own implementation, kept as is. It doesn't follow WebKit's two connections into the value types (`CSSParserToken.h` → `CSSPrimitiveValue.h`, `CSSParserIdioms.h` → `CSSParserContext.h`): the token uses `Parser/NumericValue.hpp` and the idioms use `Values/GlobalKeyword.hpp`. Everything WebKit pulls in through those two connections moves to the deferred list below.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSTokenizer.cpp` | `have*` | `src/CSS/Parser/Tokenizer.cpp` | off | Entry. |
| `css/parser/CSSTokenizer.h` | `have*` | `CSS/Parser/Tokenizer.hpp` | off | |
| `css/parser/CSSTokenizerInputStream.h` | `have*` | `CSS/Parser/InputStream.hpp` | on | |
| `css/parser/CSSParserToken.h` | `have*` | `CSS/Parser/Token.hpp`, `src/CSS/Parser/Token.cpp` | on | compiles. Reviewed against upstream 2026-10-11. Numeric data in `Parser/NumericValue.hpp` instead of `CSSPrimitiveValue`. Spec token set: no `Comment`/`*Match`/`Column` tokens (they're `Delim`s), one `Whitespace` token, plus `UnicodeRange`. Not ported: `unitType()`/`stringToUnitType` (needs `CSSUnits`, Values slice), `convertToDimensionWithUnit`/`convertToPercentage` (tokenizer builds these directly), string-literal backing (`hasStringBacking`/`tryUseStringLiteralBacking`; tokens own their text). Gap: no original text or whitespace count, so `CustomProperty` serialisation falls back to `Normal` (FIXMEs in `Token.cpp`). |
| `css/parser/CSSPropertyParser.cpp` (`cssValueKeywordID`, `cssPropertyID` only) | `port` | `CSS/Parser/KeywordLookup.hpp`, `src/CSS/Parser/KeywordLookup.cpp` | on | compiles, tested. Renamed `ValueIdFromName`/`PropertyIdFromName`. The rest of `CSSPropertyParser` belongs to the Properties slice. |
| `css/CSSMarkup.h`, `.cpp` | `port` | `CSS/Markup.hpp`, `src/CSS/Markup.cpp` | on | compiles. Needed by `Token::Serialise` (a `.cpp` dependency, so headers-only mode missed it). UTF-8 in and out. Overlaps `Parser/SerializerIdioms.hpp` (older, UTF-32, not built); one should go. |
| `css/parser/CSSParserTokenRange.h` | `have*` | `CSS/Parser/TokenRange.hpp` | on | |
| `css/parser/CSSParserIdioms.h` | `have*` | `CSS/Parser/ParserIdioms.hpp` | off | No `CSSParserContext` dependency. |
| `css/CSSWideKeyword.h` | `have*` | `CSS/Values/GlobalKeyword.hpp` | | Renamed. |
| `css/parser/CSSParserObserver.h` | `wont` | — | | See won't port. |
| `css/parser/CSSParserObserverWrapper.h` | `wont` | — | | See won't port. |

Built via `KRYSTAL_BOOEY_CSS_HEADERS`/`KRYSTAL_BOOEY_CSS_SOURCES` in `CMakeLists.txt` (codegen re-enabled). Build (2026-10-11): `Krystal.Booey` builds cleanly. `tests/CSS/Parser/Tokenizer.cpp` is enabled: 35 test cases / 380 assertions pass. `tests/CSS/Parser/Token.cpp` covers the whole `Token` API: 33 test cases, all pass. `tests/CSS/Parser/KeywordLookup.cpp` covers the name lookups (8 cases). Full Booey suite: 477 cases, all pass or fail as expected.

Bugs found by `tests/CSS/Parser/Token.cpp`:
- **Fixed (2026-10-11): keyword/property lookup was case-sensitive.** `Token` called the raw generated `FindValueKeyword`/`FindProperty`. Ported upstream's `cssValueKeywordID`/`cssPropertyID` (from `CSSPropertyParser.cpp`) as `ValueIdFromName`/`PropertyIdFromName` in `CSS/Parser/KeywordLookup.hpp`, including the `-apple-` → `-webkit-` keyword rewrite. `Token` caches the value keyword like upstream (`mutable` `uint16`, invalidated when the text changes); property lookups aren't cached, also like upstream. Still calling the raw lookups, to switch over when their slices are reviewed: `src/CSS/Parser.cpp:477`, `src/CSS/Properties/Consumers/Animations.cpp:154`, `src/CSS/Properties/Consumers/WillChange.cpp:48`; and the commented-out originals in `src/CSS/Properties/PropertyParser.cpp` can go.
- **Fixed (2026-10-11): function tokens weren't block starts.** Added `Tokenizer::BlockStart(blockType, type, name)` (upstream's `blockStart(LeftParenthesisToken, FunctionToken, name)`) and a `BlockTokenType` parameter on `Token`'s text constructor; both function-token sites in `ConsumeIdentLikeToken` use it. Note: url tokens serialise through `SerializeIdentifier` like upstream, so `url(foo.png)` → `url(foo\.png)` (tokenizes back to the same URL). `tests/CSS/Parser/Parser.cpp` and `tests/CSS/Selectors/SelectorParser.cpp` stay disabled until their slices.

Pulled into the build as headers, unreviewed: generated `PropertyId.cpp` defines the `Property::` statics and `IsKeywordValidFor*Values`, so it needs `CSS/Properties/Property.hpp` → `CSS/Values/Value.hpp` (WebKit's `CSSPropertyNames.cpp` has the same dependency). `Value.hpp` got one fix to compile: `friend class RefCounted<Value>`, because `RefCounted<Value>::SubRef` must reach `Value`'s protected destroying `operator delete` (WebKit's `CSSValue::deref` is a member, so it never needed this). Generated `ValueId.cpp` no longer includes `ParserContext.hpp`/`Property.hpp` (unused; removed from `codegen/css_value_keywords.py`).

### Generated

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `CSSPropertyNames.h` | `have` | `codegen/css_properties.py` → `CSS/Properties/PropertyId.hpp` | Generated into `build/.../codegen`; checked-in copies deleted. |
| `CSSValueKeywords.h` | `have` | `codegen/css_value_keywords.py` → `CSS/Values/ValueId.hpp` | Same. |

---

## Slice 2: Values — `Value` reviewed, not built

Entry: `css/CSSValue.cpp` (headers-only graph: 160 files, `tools/port/out/value.md`). `CSSValue.cpp`'s `visitDerived` downcasts to all ~58 subclasses, so `Value.cpp` can't compile, and no `Value` can be created or destroyed in a test, until every subclass compiles. Decision (2026-10-11): review `Value.hpp`/`Value.cpp` now; keep `Value.cpp` out of the build and add no tests until the subclasses are ported. `Value.hpp` is still compiled as a header (via `Properties/Property.hpp`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSValue.h`, `.cpp` | `have*` | `CSS/Values/Value.hpp`, `src/CSS/Values/Value.cpp` | header only | Reviewed 2026-10-11. `.cpp` not built (needs every subclass; `CanvasValue.hpp` doesn't exist yet). |
| `ClassType` (in `CSSValue.h`) | `have*` | `CSS/Values/ValueType.hpp` | | Reordered to upstream order (2026-10-11). |
| `ValueSeparator` (in `CSSValue.h`) | `have` | `CSS/Values/ValueSeparator.hpp` | | |

**Bugs fixed (2026-10-11):**
- `ValueType` had no `Canvas`, but `VisitDerived` has a `Canvas` case. Added, and `IsCanvasValue()`.
- `ValueType` order differed from upstream, which broke the range checks: `ContainsValueList()` (`>= ValueWithSmallList`, a fake enumerator second-to-last) was false for `Function`, `GridAutoRepeat`, `GridIntegerRepeat`, `ImageSet`, `Subgrid` and `TransformList`, so `CollectComputedStyleDependencies` skipped their children. Now upstream order, with `ContainsValueList()` as `>= ValueList`. Removed the unused `Invalid` and `ValueWithSmallList` enumerators (59 values, still 6 bits).
- `ValueSeparator`'s enum traits said 4 values; it has 3.
- `Value` was copyable (copying would copy the reference count). Now `KRYS_NON_COPYABLE`, like upstream.
- `Value(ValueType)` wasn't `explicit`.
- `CompareValueList` passed a `Ref<TValue>` to `Equals(const Value &)`; now dereferences it.

**Added (2026-10-11):** `IsCanvasValue`, `HasVariableReferences`, `IsImageGeneratorValue`, `IsImage`, `AddHash`/`AddDerivedHash` (`Hasher&` → `size_t &seed` with `Hash::Combine`), `IsCSSLocalURL`, `SeparatorCSSText` (static and member, returns `CSSOMStringView`), and declarations for `First()`/`Second()`/`GetQuad()`/`GetRect()` (defined by `ValuePair`/`QuadValue`/`RectValue` when ported; `GetQuad`/`GetRect` because members named `Quad`/`Rect` would hide those types in subclasses).

### `Aggregates` (2026-10-12)

Graph from `css/values/CSSValueAggregates.h` (headers-only): 2 files (`CSSValueAggregates.h`, `CSSValueConcepts.h`), plus WTF/platform boundaries (`tools/port/out/aggregates.md`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/CSSValueAggregates.h` | `have*` | `CSS/Values/Aggregates.hpp` | header | compiles, tested (`tests/CSS/Values/Aggregates.cpp`, 19 cases / 214 assertions). `*Vector` → `*List`, `ListOrNullopt` → `ListOrNull`. Not ported: `*RefCountedFixedVector` wrappers (needs `RefCountedFixedVector`), `supports_text_stream_insertion` (`TextStream`, `wont`). |
| `css/values/CSSValueConcepts.h` | `have?` | `CSS/Values/Concepts.hpp` | header | Compiles with `Aggregates.hpp`; not reviewed. |

**Bugs fixed:**
- `ListOrNull` declared both `using List = T;` and a member function `List()`, so the whole struct failed to parse (~40 errors). Accessor renamed `GetList()`.
- `SerializationSeparator<SpaceSeparatedArray<T, N>>` was specialised as `constexpr bool` (the primary is `SerializationSeparatorType`).
- `EnumSetOrKeywordBase::ContainsAny/ContainsAll/ContainsOnly` read `other.value`, which `EnumSet` doesn't have.
- The list wrappers mixed naming: `ListOrNone` called `.empty()`, `ListOrDefault`/`ListOrNull` called `IsEmpty()`/`Contains()`/`ContainsIf()`, while the wrapped lists (and Core's `FixedVector`/`ListHashSet`) use `isEmpty()`/`contains()`/`containsIf()`. Any instantiation failed. Standardised on the container protocol names (`size`, `isEmpty`, `contains`, `containsIf`, `first`, `last`) for `ListOrNone`/`ListOrDefault`/`ListOrNull`; their own API keeps PascalCase (`IsNone`, `IsDefault`, `IsNull`, `TryList`, `SwitchOn`). The `EnumSet` wrappers stay PascalCase, matching Core's `EnumSet`.
- `ListOrNone` used raw `assert`; now `krys_assert`.

**Krystal.Core bugs fixed (found here):**
- `Types/EnumSet.hpp` didn't compile when instantiated: `StorageType` used `sized_unsigned_t<StorageSize()>`, which takes a type, not a byte count. Added `unsigned_of_size_t<Bytes>` to `TypeTraits.hpp`. Storage was also sized from the value count rather than the highest value (wasted a byte at multiples of 8, and 64 values asked for a 16-byte integer); now sized like upstream with a `static_assert` for ≤ 64 values.
- `Types/EnumSet.hpp`, `Utils/AllowCompactPointers.hpp`, `Utils/GetPtrHelper.hpp`, `Utils/UnalignedAccess.hpp` and `Utils/ZipTuple.hpp` had no include guard; added `#pragma once`.

`Values/ValuePool.hpp` included `Values/Color/ColorValue.hpp`; fixed to `Values/ColorValue.hpp`. The next blocker for a trial compile of `PrimitiveValue.cpp` is `Values/Color/AbsoluteColorResolver.hpp`, which still includes raw WebKit `Color.h` (reached via `ValuePool.hpp` → `ColorValue.hpp`).

### `PrimitiveValue` (2026-10-11)

Graph from `css/CSSPrimitiveValue.cpp` (headers-only): 125 files (`tools/port/out/primitive-value.md`). Same constraint as `Value`: `PrimitiveValue.cpp` isn't built, and no instance can be created in a test until `Value.cpp` builds. A trial compile of `PrimitiveValue.cpp` stopped in its dependencies, not in the file itself: `Values/Aggregates.hpp` (~45 errors, `ListOrNull`), `Krystal.Core/Types/EnumSet.hpp` (template errors when instantiated from there), and `Values/ValuePool.hpp` (includes `Values/Color/ColorValue.hpp`, which doesn't exist; it's `Values/ColorValue.hpp`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSPrimitiveValue.h`, `.cpp` | `have*` | `CSS/Values/PrimitiveValue.hpp`, `src/CSS/Values/PrimitiveValue.cpp` | header only | Rewritten against upstream. |
| `css/CSSUnits.h`, `.cpp` | `have*` | `CSS/Values/Units.hpp`, `UnitType.hpp`, `UnitCategory.hpp` | header | Split into three headers; `operator<<(TextStream&)` not ported (`wont`). |
| `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h`, `.cpp` (number formatting only) | `port` | `CSS/Values/Primitives/NumericTypesSerialisation.hpp`, `src/.../NumericTypesSerialisation.cpp` | on | compiles, tested. `SerializableNumber`, `FormatCSSNumberValue`, `FormatNonfiniteCSSNumberValue`; WTF's `FormattedCSSNumber` (fixed, 6 decimals, trailing zeros trimmed, `-0` → `0`) done with `std::to_chars`. |

**Bugs fixed:**
- `PrimitiveValue(CSSOMString, UnitType)` stored the string's *character data* reinterpreted as a pointer (`*reinterpret_cast<const StringAtomStorage *>(atom.View().data())`). Now stores the atom's storage; added `StringAtom::Storage()` to Krystal.Core.
- No destructor: `Attr` and `Calc` references were leaked, and the serialised-text cache entry was never removed. Added `~PrimitiveValue`.
- `Value::ValueId()` called `PrimitiveValue::ValueId()`, which didn't exist, so it resolved back to `Value::ValueId()` (infinite recursion). Added `PrimitiveValue::ValueId()`; `Value`'s inline `IsValueId`/`ValueId`/`IsCustomIdent`/`CustomIdent`/`IsString`/`String`/`IsInteger`/`Integer`/`IntegerDeprecated` are now defined in `PrimitiveValue.hpp` like upstream (they were declared but never defined).
- Type checks used the stored unit instead of `PrimitiveType()`, so a `calc()` resolving to a number/percentage/length wasn't one (`IsNumber`, `IsInteger`, `IsPercentage`, `IsLength`, `IsFlex`, …). Now upstream's split of `PrimitiveType()` vs `PrimitiveUnitType()`.
- `PropertyId()` asserted for non-property values; upstream returns `Invalid`.
- `ValueDeprecated(UnitType)` called a `DoubleValueDeprecated(UnitType)` that didn't exist; `Create(ValueId)`, `Equals`, `CustomCSSText`, `CustomVisitChildren`, `DoubleValue*`… were declared but never defined.
- `ValueFromPool` cast the double to an integer before range-checking (undefined behaviour for large or NaN values). Range-checked first.
- `UnitType`'s enum traits said 77 values; it has 78 (`QuirkyEm` was outside the range).
- `UnitCategoryForType` had no case for `QuirkyEm` (fell through to `krys_unreachable`). Upstream: `Other`.
- `CanonicalUnitTypeForUnitType` did `UnitCategory(unitType)` (a cast) instead of calling `UnitCategoryForType`.
- Union members `PropertyId PropertyId` / `ValueId ValueId` changed the meaning of the type names (ill-formed); now qualified.

**Added (upstream API):** `PrimitiveType`, `IsAngle`/`IsTime`/`IsFrequency`/`IsResolution`/`IsPx`/`IsX`/`IsDotsPer*`/`IsQuirkyEms`/`IsNumberOrInteger`/`IsParentFontRelativeLength`/`IsFontFamily` and the instance forms of the length checks, `ConversionToCanonicalUnitRequiresConversionData`, `CreateFontFamily`, all `ResolveAs{Integer,Number,Percentage,Angle,Time,Resolution,Flex,Length}*` families, `ComputeAngle/Time/Frequency/Resolution`, `ValueIfNotCalculated`, `ValueDividingBy100IfPercentage*`, `IsOne`/`IsPositive`, `CSSCalcValue`/`CSSAttrValue` (+ `Protected*`), `UnitTypeString`, `CollectComputedStyleDependencies`, `AddDerivedHash`, `FloatValueDeprecated` (`dom::ExceptionOr`), `RoundForImpreciseConversion`; in `Units.hpp`: `ConversionToCanonicalUnitsScaleFactor(UnitType)`, `ConversionToCanonicalUnitRequiresConversionData(UnitType)`.

**Differences:**
- Strings are interned `StringAtom`s that are never freed, not reference-counted `StringImpl`s; string equality is atom identity.
- Static values still use the extra-`AddRef` workaround (no static flag; see `Value`).
- Non-converting accessors keep the existing Krystal name `Value<T>(…)`, which hides the base class name inside `PrimitiveValue`; refer to the base as `css::Value` there.
- `ResolveAsLength` is specialised for `int64/uint64/int32/uint32/int16/uint16/float/double`; no `LayoutUnit` specialisation and no `maxValueForCssLength`/`minValueForCssLength` until `LayoutUnit` is ported.
- `ResolveAsLengthDouble` for non-calc values is `krys_not_implemented()` until `Style::computeNonCalcLengthDouble` (`style/StyleLengthResolution`) is ported.
- `CustomCSSText` serialises keywords with `ToLower(ValueId)` for upstream's `nameStringForSerialization`.
- `FormatIntegerValue` uses shortest round-trip `std::to_chars` (same as `Token`); FIXME to share it.

Tests (constexpr/static parts only): `tests/CSS/Values/Units.cpp`, `NumericTypesSerialisation.cpp`, `PrimitiveValue.cpp` — 25 cases / 386 assertions pass.

**Differences (not fixed):**
- Reference counting uses `RefCounted<Value>` instead of upstream's own count. No `refCountFlagIsStatic`/`makeStatic()`: static values (`StaticValuePool`) rely on the pool's reference never being released. No `hasAtLeastOneRef()`.
- No `createDeprecatedCSSOMWrapper` (CSSOM; with `DeprecatedCSSOM*Value`).
- No free `add(Hasher&, const CSSValue&)`; use `AddHash`.
- `operator delete` frees with `::operator delete` (no per-type `freeAfterDestruction`).
- No size `static_assert` (upstream: refcount + one word of bitfields).

## Deferred: Values (from the tokenizer graph)

WebKit's tokenizer pulls these in, but the Krystal tokenizer doesn't need them. Kept here for a future Values/Parser slice; re-run the graph from that slice's entries when it starts. Not yet confirmed (`have?` = name match only).

| Hops | WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|---|
| 9 | `css/ComputedStyleDependencies.h` | `have?` | `CSS/ComputedStyleDependencies.hpp` | on | |
| 9 | `css/values/CSSValueAggregates.h` | `have?` | `CSS/Values/Aggregates.hpp` | on | |
| 8 | `css/values/CSSValueTypes.h` | `have?` | `CSS/Values/Types.hpp` | on | `Style/Values/Types.hpp` is the style-side counterpart. |
| 8 | `css/values/primitives/CSSPrimitiveKeywordList.h` | `have?` | `CSS/Values/Primitives/PrimitiveKeywordList.hpp` | on | |
| 7 | `css/values/primitives/CSSPrimitiveData.h` | `have?` | `CSS/Values/Primitives/PrimitiveData.hpp` | on | |
| 7 | `css/values/primitives/CSSPrimitiveNumericRaw.h` | `have?` | `CSS/Values/Primitives/NumericRaw.hpp` | on | |
| 7 | `css/values/primitives/CSSUnevaluatedCalc.h` | `have?` | `CSS/Values/Primitives/UnevaluatedCalc.hpp` | on | |
| 6 | `css/calc/CSSCalcType.h` | `have?` | `CSS/Calc/CalcType.hpp` | on | |
| 6 | `css/values/CSSValueConcepts.h` | `have?` | `CSS/Values/Concepts.hpp` | on | |
| 6 | `css/values/primitives/CSSPrimitiveNumeric.h` | `have?` | `CSS/Values/Primitives/PrimitiveNumeric.hpp` | on | |
| 5 | `css/CSSUnits.h` | `have*?` | `CSS/Values/Units.hpp` | on | Appears split into `UnitType.hpp` + `UnitCategory.hpp`. |
| 5 | `css/calc/CSSCalcTree.h` | `have?` | `CSS/Calc/Tree.hpp` | on | |
| 5 | `css/values/primitives/CSSPrimitiveNumericCategory.h` | `have?` | `CSS/Values/Primitives/NumericCategory.hpp` | — | Not in CMake. |
| 5 | `css/values/primitives/CSSPrimitiveNumericConcepts.h` | `have*?` | `CSS/Values/Primitives/Concepts.hpp` | on | No file of its own; numeric concepts seem merged here. |
| 5 | `css/values/primitives/CSSPrimitiveNumericRange.h` | `have?` | `CSS/Values/Primitives/Range.hpp` | on | |
| 4 | `css/CSSAttrValue.h` | `have?` | `CSS/Values/AttrValue.hpp` | on | |
| 4 | `css/CSSValue.h` | `have?` | `CSS/Values/Value.hpp` | on | |
| 4 | `css/calc/CSSCalcValue.h` | `have?` | `CSS/Calc/Value.hpp` | on | |
| 4 | `css/values/primitives/CSSPrimitiveNumericUnits.h` | `have?` | `CSS/Values/Primitives/Units.hpp` | on | |
| 3 | `css/CSSPrimitiveValue.h` | `have?` | `CSS/Values/PrimitiveValue.hpp` | on | |
| 3 | `css/StyleRuleType.h` | `have?` | `CSS/Rules/RuleType.hpp` | on | |
| 3 | `css/parser/CSSParserMode.h` | `have?` | `CSS/Parser/ParserMode.hpp` | on | |
| 2 | `css/parser/CSSParserContext.h` | `have?` | `CSS/Parser/ParserContext.hpp` | on | |

Boundary dependencies seen on this path (undecided):

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/LayoutUnit.h` | `port` | `Core/LayoutUnit.hpp` | Keep fixed-point semantics. |
| `platform/RectCorners.h` | `have` | `Core/RectCorners.hpp` | |
| `platform/RectEdges.h` | `have` | `Core/RectEdges.hpp` | |
| `loader/LoadedFromOpaqueSource.h` | `port` | | Single enum. |
| `pal/text/TextEncoding.h` | `have*` | `Krystal.Core/Text/Encodings/*` | The Core encodings replace it; `EncodingRegistry` may not be the final API. |
