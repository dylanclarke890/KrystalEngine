# WebKit CSS port ledger

- **Upstream:** `C:\Users\dylan\repos\WebKit` @ `300e80c70fd3` (2025-12-26). Never update the clone mid-port.
- **Paths** are relative to `Source/WebCore` (WebKit) and `Krystal.Booey/include/Krystal.Booey` (Krystal) unless stated.
- **Regenerate a slice:** `python tools/port/graph.py <entries...> --headers-only --out tools/port/out/<slice>.md` (add `--follow platform/graphics/` etc. to traverse outside `css/` and `style/`)

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
- Slice 3, Colour, layer 1 (2026-10-12): Graphics/Color (`platform/graphics/Color*`) reviewed, completed and tested; see the Slice 3 section.
- Slice 3, layer 2 (2026-10-12): calc-free CSS numeric primitives and value types reviewed, built and tested.
- Slice 3, layer 3a (2026-10-12): calc tree, types, serialisation and the calc-dependent numeric primitives reviewed, built and tested; see the Slice 3 section.
- Layers 1 and 2 are committed (`76e1f9cb`); layer 3a is not.

**Open / next candidates:**
- Slice 3 (colour), bottom-up: (1) Graphics/Color done, (2) calc-free numeric primitives done, (3a) Style-free calc done. Next is (3b): calc evaluation/simplification/parsing/mappings, `ValueEvaluation.cpp`, `UnevaluatedCalcEvaluation.cpp` and `ToLengthConversionData`, which need `Style::BuilderState`, `RenderStyle`, `Style::Calculation` and the property parser. Then (4) Style numerics and calc, (5) CSS colour (`css/values/color`) and `ColorValue`, (6) Style colour (`style/values/color`).
- `Value.cpp` and `PrimitiveValue.cpp` can't build until every `Value` subclass compiles (`VisitDerived`). Next blocker found by a trial compile of `PrimitiveValue.cpp`: `Values/Color/AbsoluteColorResolver.hpp` still includes raw WebKit `Color.h` (via `ValuePool.hpp` → `ColorValue.hpp`); that is layer 5 of slice 3.
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
| `wtf/MathExtras.h` | `have*` → `Maths/Base.hpp` | Most helpers already there; add the rest on demand. `areEssentiallyEqual`/`safeFPDivision` are in `Krystal.Booey/Core/MathExtras.hpp`. |
| `wtf/text/StringConcatenateNumbers.h` (`FormattedCSSNumber`), `wtf/dtoa.h` (`numberToCSSString`) | `have` → `FormatCSSNumber` in `Krystal.Booey/Core/NumberFormatting.hpp` | |
| `wtf/HexNumber.h` (`hex(x, 2)`) | `have*` → `text::*NibbleTo*ASCIIHexDigit` in `Text/ASCII.hpp` | |
| `wtf/HashTraits.h` | `have` → `HashTraits` in `Hash.hpp` | `HashTableEmptyValue`/`HashTableDeletedValue` → `HashTableEmptyValueTag`/`HashTableDeletedValueTag` (`Utils/Tags.hpp`). |
| `wtf/StdLibExtras.h` | `port` on demand | Map each helper as it comes up (e.g. `WTFMove` → `krys::move`) and record it here. |
| `wtf/TypeCasts.h` | `have` → `TypeCast.hpp` | |
| `wtf/URL.h` | `port` → URL type | WHATWG URL; port when a slice needs it. `Krystal.Booey/URL/Url.hpp` already exists, check it first. |

### Naming

- Functions and methods are PascalCase (`makeFromComponents` → `CreateFromComponents`). British spelling: `serialize`/`serialization` → `Serialise`/`Serialisation` in identifiers (decided 2026-10-12).
- Public data members of plain structs, including `static constexpr` trait members, are PascalCase (`red` → `Red`, `componentInfo` → `ComponentInfo`). When a member then shares its type's name (`CompositeOperation CompositeOperation`), write the type as `boo::X` inside the struct. Class constants (`Color::red`) and container-protocol methods (`size`, `get<I>`, `at`, `data`) keep upstream's names.
- A method named after a type would hide it in the class (`colorSpace()` → `Space()`, `quad()` → `GetQuad()`).

### Won't port

Remove these at the include site; don't stub them.

| WTF / WebCore | Reason |
|---|---|
| `wtf/text/TextStream.h` | Debug dumping. |
| `wtf/TZoneMalloc.h` | Allocator annotations (`WTF_MAKE_TZONE_ALLOCATED`); drop the macro. |
| `wtf/Forward.h` | Include the Krystal headers directly. |
| `wtf/NoVirtualDestructorBase.h` | Marker base only; drop it. |
| `css/parser/CSSParserObserver.h`, `CSSParserObserverWrapper.h` | Inspector/CSSOM source-range hooks. Drop observer parameters and code paths. |
| `platform/graphics/DestinationColorSpace.h`, `PlatformColorSpace.h` | CoreGraphics/Skia colour spaces; use `ColorSpace`. |
| IPC types (`*DataForIPC`) | No IPC. |

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

## Slice 3: Colour

Entry: `css/CSSColorValue.cpp` (headers-only graph: 42 files, `tools/port/out/color-value.md`; with every `css/values/color` file as an entry: 158 files, `tools/port/out/color.md`, which reaches the style builder through `CSSLightDarkColor.cpp`/`CSSKeywordColor.cpp`). `ColorValue` itself is a thin wrapper; the work is in `css::Color` (Krystal has 4 of upstream's 43 alternatives and no `.cpp`s under `CSS/Values/Color`). Decision (2026-10-12): full colour slice, bottom-up, building and testing each layer; layer 1 (Graphics/Color) first, then re-plan.

### Layer 1: Graphics/Color (2026-10-12)

Graph from every `platform/graphics/Color*`, `DestinationColorSpace` and `AlphaPremultiplication` file with `--follow platform/graphics/` (headers-only): 31 files, all entries except `PlatformColorSpace.h` and the CG/Skia colour-space headers (`tools/port/out/graphics-color.md`). Boundaries: `platform/animation/AnimationUtilities.h`, `wtf/text/MakeString.h`/`StringBuilder.h`/`HexNumber.h`, `wtf/MathExtras.h`, `wtf/Hasher.h`/`HashTraits.h`, `TextStream`. These files were already built (outside the CSS lists) but never reviewed or tested.

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/graphics/AlphaPremultiplication.h` | `have` | `Graphics/Color/AlphaPremultiplication.hpp` | `operator<<(TextStream&)` not ported. |
| `platform/graphics/Color.h`, `.cpp` | `have` | `Graphics/Color/Color.hpp`, `ColorFlags.hpp`, `src/.../Color.cpp` | tested. `Flags`/`FlagsIncludingPrivate` are the namespace-level `ColorFlags`/`ColorFlagsIncludingPrivate` (flags enums, not `OptionSet`). `colorSpace()` is `Space()` (a member named `ColorSpace` would hide the type). |
| `platform/graphics/ColorHash.h` | `port` | `Graphics/Color/ColorHash.hpp` | tested. `HashTraits<Color>`; the `add(Hasher&, Color)` equivalent is `DefaultHash<Color>` in `Color.hpp`. |
| `platform/graphics/ColorBlending.h`, `.cpp` | `port` | `Graphics/Color/ColorBlending.hpp`, `src/.../ColorBlending.cpp` | tested. New. |
| `platform/animation/AnimationUtilities.h` | `port` (partial) | `Graphics/Animation/AnimationUtilities.hpp` | tested. `BlendingContext` and the `int`/`unsigned`/`float`/`double` `Blend` overloads; the `IntPoint`/`FloatPoint`/`LayoutPoint`/`LayoutUnit` overloads wait for those types (TODO). Members are PascalCase (`Progress`, `CompositeOperation`, …). |
| `animation/CompositeOperation.h`, `IterationCompositeOperation.h` | `port` | `Graphics/Animation/CompositeOperation.hpp`, `IterationCompositeOperation.hpp` | `toCompositeOperation(const CSSValue&)` and `operator<<` not ported (CSS side; add with animations). |
| `platform/graphics/ColorComponents.h` | `have` | `Graphics/Color/ColorComponents.hpp` | tested. |
| `platform/graphics/ColorConversion.h`, `.cpp` | `have` | `Graphics/Color/ColorConversion.hpp`, `src/.../ColorConversion.cpp` | tested. |
| `platform/graphics/ColorInterpolation.h`, `.cpp` | `have` | `Graphics/Color/ColorInterpolation.hpp`, `src/.../ColorInterpolation.cpp` | tested. |
| `platform/graphics/ColorInterpolationMethod.h`, `.cpp` | `have` | `Graphics/Color/ColorInterpolationMethod.hpp`, `src/.../ColorInterpolationMethod.cpp` | tested. Serialisation and hash added; `operator<<` not ported. |
| `platform/graphics/ColorLuminance.h`, `.cpp` | `have` | `Graphics/Color/ColorLuminance.hpp`, `src/.../ColorLuminance.cpp` | tested. |
| `platform/graphics/ColorMatrix.h` | `have` | `Graphics/Color/ColorMatrix.hpp` | tested. `at`/`data` keep lowercase names. |
| `platform/graphics/ColorModels.h` | `have` | `Graphics/Color/ColorModels.hpp` | tested. |
| `platform/graphics/ColorNormalization.h` | `port` | `Graphics/Color/ColorNormalization.hpp` | tested. `NormalizeHue`/`CreateColorTypeByNormalizingComponents` moved here from `ColorUtilities.hpp`. |
| `platform/graphics/ColorSerialization.h`, `.cpp` | `port` | `Graphics/Color/ColorSerialisation.hpp`, `src/.../ColorSerialisation.cpp` | tested. New `.cpp` (header existed). Returns `utf8_string`; `Serialisation(ColorSpace)` returns `utf8_stringview`. |
| `platform/graphics/ColorSpace.h`, `.cpp` | `have` | `Graphics/Color/ColorSpace.hpp` | tested. `.cpp` is only `operator<<(TextStream&)`: not ported. |
| `platform/graphics/ColorTransferFunctions.h` | `have` | `Graphics/Color/ColorTransferFunctions.hpp` | tested. |
| `platform/graphics/ColorTypes.h` | `have` | `Graphics/Color/ColorTypes.hpp` | tested. Upstream's `make*` functions are `Create*`. |
| `platform/graphics/ColorUtilities.h`, `.cpp` | `have` | `Graphics/Color/ColorUtilities.hpp` | tested. The `.cpp` functions are `constexpr` in the header. |
| `platform/graphics/DestinationColorSpace.h`, `.cpp`, `PlatformColorSpace.h` | `wont` | — | CG/Skia colour spaces. Deleted the empty stub and the `DestinationColorSpace` overloads of `Color::ToResolvedColorComponentsInColorSpace` and `ConvertAndResolveColorComponents`; use the `ColorSpace` overloads. |
| `wtf/MathExtras.h` (`areEssentiallyEqual`, `safeFPDivision`) | `port` | `Krystal.Booey/Core/MathExtras.hpp` | tested. |
| `wtf/dtoa.cpp` (`numberToCSSString`) / `FormattedCSSNumber` | `port` | `Krystal.Booey/Core/NumberFormatting.hpp`, `src/Core/NumberFormatting.cpp` | tested. `FormatCSSNumber`; was a private helper in `NumericTypesSerialisation.cpp`, which now uses it. |

**Bugs fixed:**
- `ConvertByteAlphaTo<T>` had no specialisations and returned the byte unscaled, and `ConvertFloatAlphaTo<T>` divided by 255 for every `T`. As a result, `Color::AlphaAsFloat()` returned 0–255 for inline colours, `AlphaByte()` of out-of-line colours was 0, and `ColorWithAlpha(x)` on an inline colour always gave alpha 0. These are now upstream's `uint8`/`float` specialisations.
- `EqualIgnoringSemanticColor`/`OutOfLineComponentsEqualIgnoringSemanticColor` subtracted the `Semantic` flag arithmetically instead of masking it out, so a semantic colour never compared equal to its non-semantic version.
- `ClampedAlpha` converted `int` to `uint8` implicitly (a /W4 warning as soon as `CreateFromComponentsClamping<SRGBA<uint8>>` was instantiated with `int`s, as `BlendSourceOver` does).
- `ConvertAndResolveColorComponents(…, DestinationColorSpace)` was `krys_not_implemented` (removed, see above).
- `ComputeDeltaEOK` divided the OKLab lightnesses by 100 (as upstream does), but OKLab lightness is already in [0, 1], so lightness differences were 100x too small. Now uses them directly, per https://drafts.csswg.org/css-color-4/#color-difference-OK (deliberate difference from upstream).
- `Color`'s encoding helpers, `SetColor`/`SetOutOfLineComponents`, `Flags()`, `IsOutOfLine()`/`AsInline()`/`AsOutOfLine()` and the `Ref<OutOfLineComponents>` constructor were public; now private like upstream.
- `ColorConversion.hpp` used `AnalogousComponentIndex` before declaring it; clang-format had scrambled the conversion diagrams (restored inside `clang-format off`).

**Added (upstream API):** `Color(HashTableEmptyValueTag)`/`Color(HashTableDeletedValueTag)`, `ProtectedAsOutOfLine`, `DebugDescription`, `DefaultHash<Color>`, `HashTraits<Color>`; all of `ColorSerialisation.cpp`, `ColorBlending`, `ColorNormalization`; `ColorInterpolationMethod` serialisation (`SerialisationForCSS` for colour spaces, hue methods and methods) and `DefaultHash<ColorInterpolationMethod>`.

**Differences:**
- Deliberate fix: the unclamped ProPhotoRGB and Rec2020 transfer functions pass `abs(c)` to `pow` (upstream passes the signed value, so negative extended components become NaN).
- `ColorSerialisation.cpp` groups upstream's identical per-type overloads into constrained templates (`SerialisedUsingColorFunction`, `SerialisedAsLegacyRGB`, `SerialisedWithOwnFunction`); output is unchanged.
- No IPC (`ColorDataForIPC`, `Color(Maybe<ColorDataForIPC>)`, `data()`), CG/Skia conversions, `TextStream` output, or 32-bit pointer support (`static_assert` on 64-bit pointers).
- Public data members of plain structs are PascalCase (style guide), renamed from upstream's lowercase: colour type components (`Red`, `Alpha`, `Hue`, `Lightness`, `A`, `B`, `X`, …), model/descriptor traits (`ComponentInfo`, `IsInvertible`, `CoordinateSystem`, `AchromaticEpsilon`, `WhitePoint`, `LinearToXYZ`, `XYZToLinear`), `AlphaTraits::Transparent`/`Opaque`, `ColorComponentInfo::Min`/`Max`/`Type`/`Category`, `PackedColor::RGBA::Value`, `PremultipliedAlphaState`, `ColorSpaceMapping::ColorSpace`, `ChromaticAdaptation::Matrix`, the transfer-function constants (`Gamma`, `Alpha`, `Beta`), `ColorInterpolationMethod::ColorSpace`/`AlphaPremultiplication`/`InterpolationColorSpace`/`HueInterpolationMethod`, `BlendingContext`, `Color::DebugRGBAData`. Where a member shares its type's name, the type is written `boo::X` inside the struct. `Color`'s class constants (`Color::red`, …) keep upstream's names.

**Follow-ups for later layers (unbuilt files that use these):**
- `CSS/Values/Color/Convert/Normalise.hpp` calls `NormalizeHue`, now in `Graphics/Color/ColorNormalization.hpp`.
- Unbuilt `CSS/`/`Style/` colour code still uses the lowercase colour-type and model member names (`.red`, `::componentInfo`, …).
- `Style/Values/Types.hpp` forward-declares `BlendingContext`/`CompositeOperation` (now `krys::boo::BlendingContext` in `Graphics/Animation/AnimationUtilities.hpp`), and `Style/Values/Primitives/LengthWrapperBlending.hpp`/`NumericTypesBlending.hpp` use upstream's lowercase member names.

Tests: `tests/Graphics/Color/{Color,ColorBlending,ColorConversion,ColorInterpolation,ColorSerialisation,ColorUtilities}.cpp` (`Color.cpp` ports WebKit's `TestWebKitAPI/Tests/WebCore/ColorTests.cpp`), `tests/Core/NumberFormatting.cpp`: 66 cases / 1002 assertions. Full Booey suite: 596 cases / 8946 assertions, all pass.

---

### Layer 2: CSS numeric primitives and value types, calc-free half (2026-10-12)

Graph: `tools/port/out/primitives.md`. Upstream's `CSSValueTypes.h` reaches `CSSValuePool` (and colour), `CSSUnevaluatedCalc.h` reaches `StyleBuilderState`, `CSSSerializationContext.h` reaches `CSSStyleSheet`. Decision (2026-10-12): layers 2 and 3 are split; `UnevaluatedCalc`, `PrimitiveData`, `PrimitiveNumeric`(`OrKeyword`), `NumericTypes` and the `+EvaluateCalc`/`+SymbolReplacement`/`+Serialization`/`+ComputedStyleDependencies`/`+CSSValueVisitation`/`+Canonicalization` headers need `CSSCalcValue` and are part of layer 3 (calc). `Position`, `Ratio`, `URL`, `URLModifiers`, `Quad`/`Rect` aren't needed by colour and are left for later.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/CSSValueConcepts.h` | `have` | `CSS/Values/Concepts.hpp` | on | tested via `Types`. Added `EnumHasValueRepresentation`, `EnumWith(out)ValueRepresentation`, `HasIsZero`/`HasIsKnownZero`/`HasIsEmpty`/`HasIsKnownEmpty` (they check PascalCase member functions). |
| `css/values/primitives/CSSPrimitiveNumericConcepts.h` | `have` | `CSS/Values/Primitives/Concepts.hpp` | on | tested. The unit concepts moved back here from `Units.hpp`; `brigand::set` → `std::tuple` + `detail::ContainsInTuple`. |
| `css/values/primitives/CSSPrimitiveNumericCategory.h`, `.cpp` | `have` | `CSS/Values/Primitives/NumericCategory.hpp` | on | `css::Category` is the only category enum (the duplicate `NumericTypeCategory` in `Units.hpp` is gone). `operator<<` not ported. |
| `css/values/primitives/CSSPrimitiveNumericRange.h` | `have` | `CSS/Values/Primitives/Range.hpp` | on | tested. `infinity` → `Inf`, `Nonnegative` → `NonNegative`. |
| `css/values/primitives/CSSPrimitiveNumericUnits.h`, `.cpp` | `have` | `CSS/Values/Primitives/Units.hpp`, `src/.../Units.cpp` | on | tested. Switch tables and conversions compared mechanically with upstream (every case/return pair matches). |
| `css/values/primitives/CSSPrimitiveNumericRaw.h` | `have` | `CSS/Values/Primitives/NumericRaw.hpp` | on | tested. |
| `css/values/primitives/CSSPrimitiveKeywordList.h` | `have` | `CSS/Values/Primitives/PrimitiveKeywordList.hpp` | on | tested. |
| `css/values/primitives/CSSSymbol.h`, `.cpp` | `have` | `CSS/Values/Primitives/Symbol.hpp`, `src/.../Symbol.cpp` | on | tested. The `.cpp` was missing (serialisers declared, never defined). |
| `css/values/CSSNoConversionDataRequiredToken.h` | `have` | `CSS/Values/NoConversionDataRequiredTag.hpp` | on | Renamed `…Tag`. |
| `css/ComputedStyleDependencies.h`, `.cpp` | `have` | `CSS/ComputedStyleDependencies.hpp`, `src/CSS/ComputedStyleDependencies.cpp` | header | tested. The `.cpp` needs `ToLengthConversionData` (unreviewed): layer 3. |
| `css/values/CSSValueTypes.h`, `.cpp` | `have` | `CSS/Values/Types.hpp`, `src/.../Types.cpp`, `src/.../TypesValueCreation.cpp` | on (not `TypesValueCreation.cpp`) | tested. The value-creation half needs `ValueList`/`ValuePair`/`QuadValue`/`FunctionValue`: build it with the Values slice. |
| `css/values/CSSSerializationContext.h`, `.cpp` | `have*` | `CSS/Values/SerialisationContext.hpp`, `src/.../SerialisationContext.cpp` | on | Decision (2026-10-12): URL replacement is not supported (upstream fills it only when saving web archives), so the context is an empty struct kept for future options; `DefaultSerialisationContext()` stays. Unbuilt users of the old fields (`CSSURL`, `CSSImportRule`, …) drop that code when reviewed. |

**Bugs fixed:**
- `LengthUnit`/`LengthPercentageUnit` had no `QuirkyEm` (added to the enums, mappings, `IsFontRelativeLength`, `ConversionToCanonicalUnitRequiresConversionData`, `UnitString` and the `_css_quirky_em` literal; `Count` 50).
- `UnitTraits<AnglePercentageUnit>` used lowercase `isValidRangeForCategory`/`validate`/`upcast`, so `AnglePercentageUnit` didn't satisfy `UnitEnum`.
- `NumericRaw`: `using UnitTraits = UnitTraits<…>` named itself; now `css::UnitTraits`.
- `ValidKeywordForList` called `isValidKeyword` (the list has `IsValidKeyword`).
- `Range`'s clamp helpers converted the `double` bounds to `T` implicitly (warnings as errors when instantiated with `float`/`int32`/`uint32`); now explicit casts. Upstream's unsigned clamp only supports floating-point inputs (WTF has no bounded signed→unsigned `clampTo`); same here.
- `Types.hpp`: separators were `std::string` appended to the UTF-8 builder; `FunctionNotation` serialisation used `append(name, '(')` (`append(str, pos)`) and `append(')')`; `ValueCreation` forwarded the same arguments several times; unused parameter in the empty-like serialiser.
- `Types.cpp`: the four serialisation functions were `krys_not_implemented`; now `Markup`'s `SerialiseIdentifier`/`SerialiseString` and `ToString(PropertyId)`.
- `NumericCategory.hpp` had no `#pragma once`. `ComputedStyleDependencies` used `SmallList` (now `List`) and `CanResolveWith` (now upstream's `CanResolveDependenciesWithConversionData`).
- Codegen (`codegen/css_properties.py`): `PropertyNameStrings` had no commas between entries, so the literals concatenated and `ToString(PropertyId)` returned an empty name for every property but the first. Round-trip test in `tests/CSS/Parser/KeywordLookup.cpp`.

**Renames (decisions 2026-10-12, Booey-wide):**
- `Serialize`/`Serialization`/`Serializer` → `Serialise`/`Serialisation`/`Serialiser` in every identifier (and the `SERIALIZATION` macros) across Booey, its tests, codegen and one Core local; `Parser/SerializerIdioms.hpp` → `SerialiserIdioms.hpp`. Upstream file names in this ledger keep their spelling.
- `Aggregates` fields are PascalCase: the wrappers' `value` → `Value`, `FunctionNotation::parameters`/`name` → `Parameters`/`Name`; generated `KeywordValueConstant::value` → `Value`. Unbuilt users (`DEFINE_TYPE_WRAPPER_GET(X, value)` callers, filter/easing functions, …) still use the old names.

Tests: `tests/CSS/Values/Primitives/{Units,NumericRaw}.cpp`, `tests/CSS/Values/Types.cpp`, plus the `ToString(PropertyId)` round trip. Full Booey suite: 617 cases / 10484 assertions, all pass.

---

### Layer 3a: CSS calc, Style-free half (2026-10-12)

Graph from `css/calc/CSSCalcValue.h`, `CSSCalcTree.h`, the calc-dependent primitives and `CSSToLengthConversionData.h` (headers-only): 94 files (`tools/port/out/calc.md`). The headers are self-contained; the explosion into Style comes from `CSSToLengthConversionData.h`/`CSSUnevaluatedCalc.h` → `StyleBuilderState.h` and from the `.cpp`s. Split (2026-10-12): 3a is everything that doesn't need Style or the property parser; 3b is the tree's evaluation, simplification, parsing and mappings, `CSSCalcValue`'s evaluation half, `CSSUnevaluatedCalc`'s evaluation half and `CSSToLengthConversionData` (they need `Style::BuilderState`, `RenderStyle`, `Style::Calculation`, `AnchorPositionEvaluator` and the property-parser consumers).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/calc/CSSCalcType.h`, `.cpp` | `have` | `CSS/Calc/CalcType.hpp`, `src/.../CalcType.cpp` | on | tested. `make*` → `Create*`. `operator<<` not ported. |
| `css/calc/CSSCalcOperator.h`, `.cpp` | `have` | `CSS/Calc/Operator.hpp` | on | The `.cpp` is only `operator<<`: not ported. |
| `css/calc/CSSCalcExecutor.h` | `have` | `CSS/Calc/Executor.hpp` | on | tested. Renamed from `Executer.hpp`. |
| `css/calc/CSSCalcSymbolTable.h`, `.cpp` | `have` | `CSS/Calc/SymbolTable.hpp` | on | tested. Header-only `css::SymbolTable`. The nested `Value` struct is now `Entry` (a `Value` member can't share its struct's name); its fields are `Unit`/`Value`. |
| `css/calc/CSSCalcSymbolsAllowed.h`, `.cpp` | `have*` | `CSS/Calc/AllowedSymbols.hpp` | on | tested. `css::AllowedSymbols` uses a small list instead of a `HashMap` (what the upstream FIXME suggests), built from a list rather than an `initializer_list`. |
| `css/calc/CSSCalcTree.h`, `.cpp` | `have` | `CSS/Calc/Tree.hpp`, `src/.../Tree.cpp` | on | tested. `CanonicalDimension::Dimension` is now the namespace-level `calc::Dimension`. `Random::Sharing` → `SharingType` (the member is `Sharing`). `Vector<Child>` → `List<Child>`. `operator<<` not ported. |
| `css/calc/CSSCalcTree+Traversal.h` | `have` | `CSS/Calc/TreeTraversal.hpp` | on | tested. |
| `css/calc/CSSCalcTree+NumericIdentity.h` | `have` | `CSS/Calc/TreeNumericIdentity.hpp` | on | tested; mappings compared mechanically. |
| `css/calc/CSSCalcTree+Copy.h`, `.cpp` | `have` | `CSS/Calc/TreeCopy.hpp`, `src/.../TreeCopy.cpp` | on | tested. |
| `css/calc/CSSCalcTree+ComputedStyleDependencies.h`, `.cpp` | `have` | `CSS/Calc/TreeComputedStyleDependencies.hpp`, `.cpp` | on | tested. |
| `css/calc/CSSCalcTree+Serialization.h`, `.cpp` | `port` | `CSS/Calc/TreeSerialisation.hpp`, `src/.../TreeSerialisation.cpp` | on | tested. The `.cpp` was missing; now ported. |
| `css/calc/CSSCalcRandomCachingKey.h`, `CSSCalcRandomCachingKeyMap.h` | `have` | `CSS/Calc/RandomCachingKey.hpp`, `RandomCachingKeyMap.hpp` | on | tested. The base value comes from a per-thread `mt19937_64` instead of a cryptographic generator (TODO in the code). |
| `css/calc/CSSCalcValue.h`, `.cpp` | `have` | `CSS/Calc/Value.hpp`, `src/.../Value.cpp`, `src/.../ValueEvaluation.cpp` | on (not `ValueEvaluation.cpp`) | tested (creation, type, equality, serialisation, dependencies). `category()`/`range()`/`tree()` → `GetCategory()`/`GetRange()`/`GetTree()`, because the plain names would hide the types. The parse/simplify/evaluate half is in `ValueEvaluation.cpp`, which waits for 3b. `dump`/`operator<<` not ported. |
| `css/values/primitives/CSSUnevaluatedCalc.h`, `.cpp` | `have` | `CSS/Values/Primitives/UnevaluatedCalc.hpp`, `src/.../UnevaluatedCalc.cpp`, `UnevaluatedCalcEvaluation.cpp` | on (not `UnevaluatedCalcEvaluation.cpp`) | tested through the numeric types. Was a stub; now complete. `SimplifyBase`/`Evaluate` are in `UnevaluatedCalcEvaluation.cpp`, which waits for 3b. |
| `css/values/primitives/CSSPrimitiveData.h` | `have` | `CSS/Values/Primitives/PrimitiveData.hpp` | on | tested. `payload`/`index`/`storage` are now private (`_payload`, `_index`, `_storage`) with friends: public PascalCase names would clash with the `Payload`/`Index`/`Storage` aliases. The union fields are `Number`/`Calc`. `raw()`/`calc()` → `GetRaw()`/`GetCalc()`. |
| `css/values/primitives/CSSPrimitiveNumeric.h` | `have` | `CSS/Values/Primitives/PrimitiveNumeric.hpp` | on | tested. Same accessor renames. |
| `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` | `have` | `CSS/Values/Primitives/PrimitiveNumericOrKeyword.hpp` | on | tested. Was unconverted; rewritten. |
| `css/values/primitives/CSSPrimitiveNumericTypes.h` | `have` | `CSS/Values/Primitives/NumericTypes.hpp` | on | tested. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h`, `.cpp` | `have` | `NumericTypesSerialisation.hpp`, `.cpp` | on | tested. Added the `SerialisableNumber`, raw and `NumberOrPercentageResolvedToNumber` serialisers. |
| `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h`, `.cpp` | `have` | `NumericTypesComputedStyleDependencies.hpp`, `.cpp` | on | tested. The `.cpp` (the `LengthUnit` collector) was missing; now ported. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h`, `.cpp` | `have` | `NumericTypesCanonicalization.hpp` | on | tested. Defined inline rather than in a `.cpp`. |
| `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h`, `.cpp` | `have` | `NumericTypesSymbolReplacement.hpp`, `.cpp` | on | tested. The `.cpp` was missing; now ported. |
| `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` | `have` | `NumericTypesEvaluateCalc.hpp` | on | `RequiresConversionData`/`IsUnevaluatedCalc` tested; evaluation needs 3b. |
| `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueVisitation.h` | `port` | `NumericTypesValueVisitation.hpp` | on | tested. Was missing. |
| `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h` | `have` | `NumericTypesValueCreation.hpp` | off | Needs `ValuePool`: waits for the Values slice. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Logging.h` | `wont` | | | `TextStream`. |
| `style/AnchorPositionEvaluator.h` (`AnchorSizeDimension` only) | `have` | `Style/AnchorSizeDimension.hpp` | on | The enum moved out of the unreviewed `AnchorPositionEvaluator.hpp` (which now includes it) so calc serialisation can use it. |

**Bugs fixed:**
- Calc node destructuring (`Copy`, traversal, serialising function arguments) couldn't compile: the `std::tuple_element` specialisations defined `Type` instead of `type`.
- In `calc::Type` member functions, `PercentHint::Length` named the `PercentHint` member instead of the enum; it's now `calc::PercentHint::`. The same hiding affected the `Dimension`/`Type`/`Stage`/`Children` members in `Tree.hpp`, now written `calc::X`.
- `Type::Multiply` didn't check for exponent overflow, so exponents wrapped instead of failing.
- `min()`/`max()` only accepted transform views, because `FloatingPointRange` checked `*begin()`'s type, which is a reference for containers. Upstream bug; it now checks the range's value type.
- `PrimitiveData` reference counting:
  - `PrimitiveData(Calc)` was a TODO stub that stored a null payload, so calc values were lost; it now takes a reference.
  - Copy-assignment released the old calc before taking the new reference, so self-assignment freed the value (upstream order).
  - Move-assignment to self is now a no-op.
- `UnevaluatedCalc` was a stub: ref/deref did nothing and most methods were missing. It's now complete. Upstream's `Variant` overload of `SimplifyUnevaluatedCalc` returned `bool`; it now returns the variant.
- `PrimitiveNumericOrKeyword` was half-converted:
  - it used `WTF::switchOn`, lowercase upstream names, and `tokens` instead of `Range`;
  - it had no `TreatAsVariantLike` specialisation and no `MarkableTraits` friend.

  Two upstream bugs, unreachable upstream, are also fixed:
  - `isKeyword<K>()` called `PrimitiveData::isKeyword` with a template argument it doesn't take;
  - assigning a `NumericType` passed the numeric itself to `PrimitiveData` instead of its data.
- `NumberOrPercentage`/`NumberOrPercentageResolvedToNumber`:
  - `using Number = Number<…>` named itself;
  - `SwitchOn` called itself recursively instead of switching on the value;
  - the variant constructor called the member `SwitchOn`;
  - `NumberOrPercentageResolvedToNumber` had no `MarkableTraits` friend (upstream bug).
- `CanonicalizeResolution` converted to dpi instead of dppx.
- Symbol replacement and calc evaluation:
  - `SymbolReplacement`/`EvaluateCalc` referred to a non-existent `calc::SymbolTable`.
  - `TypesMinusSymbol` produced a `brigand::list`, because Core's `VariantOrSingle` takes a pack; it now uses `brigand::wrap`.
  - `EvaluateCalc` built the raw value from a bare double, which only compiles for single-unit types (upstream bug). It now passes the canonical unit.
- `ComputedStyleDependenciesCollector<LengthUnit>` and `ReplaceSymbol(Symbol)` were declared but never defined.
- `RandomCachingKey`:
  - the hasher used `SafeToCompareToEmptyOrDeleted`, but Core's name is `IsSafeToCompareToEmptyOrDeleted`;
  - the hasher had no `operator()`, so `HashMap` couldn't use it;
  - the key's defaulted `==` needed `==` on Core's hash-table tags, which have none.
- Traversal:
  - `ForAllChildren`'s leaf overload took `const auto &`, so it was never chosen;
  - the `Child` overloads dereferenced leaves.
- `IndirectNode::operator==` dereferenced null (moved-from) nodes.
- Calc `Value.hpp` forward-declared `SymbolTable`/`AllowedSymbols` in the wrong namespace, and declared `Category()`/`Range()`/`Tree()` after the class had already used those type names.
- In serialisation, `vmax` had sort priority 50, the same as `svb`. This is an upstream typo; it's now 60.

**Differences:**
- A root `Negate`/`Invert` serialises only its child (`calc(1em)`), as upstream does. Step 4 of "serialize a math function" serialises the root's children, and only Sum/Product have overloads. The test is tagged `[!shouldfail]`.
- `Value::DoubleValueDeprecated` doesn't log (FIXME in `ValueEvaluation.cpp`).

Tests: `tests/CSS/Calc/{CalcType,Executor,SymbolTable,Tree,TreeSerialisation}.cpp`, `tests/CSS/Values/Primitives/PrimitiveNumeric.cpp`. Full Booey suite: 657 cases / 10871 assertions. All pass except the one `[!shouldfail]` case, which fails as expected.

---

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
