# WebKit CSS port ledger

- **Upstream:** `C:\Users\dylan\repos\WebKit` @ `300e80c70fd3` (2025-12-26). Never update the clone mid-port.
- **Paths** are relative to `Source/WebCore` (WebKit) and `Krystal.Booey/include/Krystal.Booey` (Krystal) unless stated.
- **Regenerate a slice:** `python tools/port/graph.py <entries...> --headers-only --out tools/port/out/<slice>.md` (to traverse outside `css/` and `style/`, pass all prefixes: `--follow css/ style/ platform/graphics/`; `--follow` replaces the defaults)

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

## Where we are (2026-10-13)

**Process per file/slice:** run `tools/port/graph.py` from the WebKit entry (`--headers-only`; also check the `.cpp`'s own includes, which headers-only misses), compare the Krystal file with upstream, add missing methods, fix bugs, record differences here, add tests where something can be instantiated, build, run tests. The user reviews each step; don't port ahead without being asked.

**Build:** only files listed in `KRYSTAL_BOOEY_CSS_HEADERS`/`KRYSTAL_BOOEY_CSS_SOURCES` (`Krystal.Booey/CMakeLists.txt`) are built; everything else under `CSS/`/`Style/` is on disk but unreviewed. Code generation runs in the build (`codegen/`). Build with Visual Studio's bundled CMake and the `windows-x64` preset (`cmake --build --preset windows-x64-debug --target Krystal.Booey.Tests`); run `build/Krystal.Booey.Tests/Debug/Krystal.Booey.Tests.exe` from that folder. If `CMakeLists.txt` is changed and restored, re-run `cmake --preset windows-x64` or the generated project keeps stale sources.

**Done:**
- Slice 1, Tokenizer: `Token`, `Tokenizer`, `Markup`, `KeywordLookup`, generated `PropertyId`/`ValueId` build and are tested.
- Slice 2, Values: `Value` and `PrimitiveValue` reviewed (headers build; `.cpp`s not built), `Units`/`UnitType`/`UnitCategory`, `NumericTypesSerialisation`, `Aggregates` reviewed and tested.
- Slice 3, Colour, layer 1 (2026-10-12): Graphics/Color (`platform/graphics/Color*`) reviewed, completed and tested; see the Slice 3 section.
- Slice 3, layer 2 (2026-10-12): calc-free CSS numeric primitives and value types reviewed, built and tested.
- Slice 3, layer 3a (2026-10-12): calc tree, types, serialisation and the calc-dependent numeric primitives reviewed, built and tested; see the Slice 3 section.
- Slice 3, layer 3b (2026-10-12): calc evaluation and simplification, Style calc, `ToLengthConversionData` and length resolution, built against the Style seam (`Style/Seam.hpp`) and tested.
- Slice 3, layer 3c (2026-10-12): calc parsing (`TreeParser.cpp`, `ValueParsing.cpp`) and the property-parser pieces it needs (`PropertyParserState`, `MetaConsumer`, Ident/Primitives/Number consumers, `TokenRangeGuard`, `ParseUnitType`) built and tested; calc is parsed end-to-end from strings.
- Slice 3, layer 4a (2026-10-12): `LayoutUnit`, `IntSize`/`IntPoint`, `FloatPoint`, `LayoutSize`/`LayoutPoint` ported and tested (upstream's API tests); see the layer 4a section.
- Slice 3, layer 4b (2026-10-12): Style numerics (`Types`, `PrimitiveNumeric`, `NumericTypes` and its Calc/Conversions/Evaluation/Serialisation/Blending headers, `CalculationValueMap`, `LengthWrapper*`) built and tested, on top of a Core port of `CompactVariant`/`FlatteningVariant`; see the layer 4b section.
- Slice 3, layer 5a (2026-10-12): CSS colour types (`css::Color` and all 43 alternatives, keyword/system/named colours, `color-mix()`, `contrast-color()`, `light-dark()`, `color-layers()`, `DynamicRangeLimit`) built and tested; see the layer 5a section.
- Slice 3, layer 5b (2026-10-12): the colour parser (`+Color`, `+ColorAdjust`, `+ColorInterpolationMethod`), the colour half of `ParserFastPaths`, `ColorScheme` and the Angle/Percentage/Keyword/Symbol consumer definitions built and tested; colours parse end-to-end from strings. The Value-returning consumers are written but wait for 5c; see the layer 5b section.
- Slice 3, layer 5c (2026-10-12): `ValuePool`/`StaticValuePool`, `ColorValue`, `DynamicRangeLimitValue`, `ColorSchemeValue` and the Value-returning colour/ident consumers compile into the library (compile-only: nothing calls them until `Value.cpp`/`PrimitiveValue.cpp` build); see the layer 5c section.
- Slice 3, layer 6 (2026-10-12): Style colour (`style::Color` and its currentcolor-dependent kinds, conversion from `css::Color`, serialisation, blending), Style `DynamicRangeLimit` and `PlatformDynamicRangeLimit`, and the `Opacity` struct built and tested, against a testable seam (`RenderStyle` can be constructed); see the layer 6 section.
- Slice 3 is committed (up to `225dbc61`).
- Slice 4, Value subclasses (2026-10-12): `ValuePair`, `ValueList`/`ValueContainingList`, `FunctionValue`, `QuadValue`, `RectValue` and `RatioValue` reviewed and compile (compile-only; `Ratio` itself is tested); see the Slice 4 section.
- Slice 4, tier 1 done (2026-10-12): `BackgroundRepeatValue`, `BorderImageSliceValue`, `BorderImageWidthValue`, `ReflectValue`, `UnicodeRangeValue` and the five font values (`FontVariationValue`, `FontFeatureValue`, `FontStyleWithAngleValue`, `FontStyleRangeValue`, `FontValue`), `GridLineNamesValue`, `GridLineValue`, `SubgridValue`, `TransformListValue`, `GridAutoRepeatValue`, `GridIntegerRepeatValue`, `CounterValue`, `OffsetRotateValue` and `AttrValue` reviewed and compile (compile-only), so tier 1 is done; `Graphics/FontTaggedSettings.hpp` ported and tested.
- Slice 4, tier 2 started (2026-10-12): `Position` (CSS primitive) built and tested; `PositionValue`/`PositionXValue`/`PositionYValue` compile (compile-only).
- URL (2026-10-12): WTF's `URL`/`URLParser`/`URLHash` ported as `url::URL` (`Krystal.Booey/URL/`), with ICU for IDNA, and tested against the WPT URL data; `CSSURL`, `URLModifiers` and `UrlValue` built and tested (`UrlValue` compile-only). See the `UrlValue` entry.
- Slice 4, tier 2 done (2026-10-13): `GridTemplateAreasValue` (with `GridTemplateAreas`, `GridNamedAreaMap`, `GridArea`/`GridSpan` moved to `Rendering/Style/`, and the Style `GridPosition`/`GridTrackSizingDirection` moved to `Style/Values/Grid/`) and `EasingFunctionValue` (with the five easing types) built and tested (the value classes compile-only). See their entries.
- Slice 4, tier 3 started (2026-10-13): `BoxShadowPropertyValue` and `TextShadowPropertyValue` (with `BoxShadow`/`BoxShadowProperty` moved to `CSS/Values/Borders/`, and `TextShadow`/`TextShadowProperty`) built and tested (the value classes compile-only). `Markable` is now optional-like in `Aggregates.hpp`. See their entry.
- Slice 4, tier 3 item 26 (2026-10-13): `PathValue` and `PathFunction`, with the geometry types (`IntRect`/`FloatRect`/`LayoutRect`, `FloatQuad`, `Region`, `GeometryUtilities`, `AffineTransform`, `TransformationMatrix` and their leaves) and the SVG path parse/serialise pipeline (`SVGPathByteStream` and friends, `FormatNumberFixedPrecision`), built and tested (`PathValue` compile-only). See its entry.
- Slice 4, tier 3 item 27 (2026-10-13): `BasicShapeValue`, with `BasicShape`, the eight shape functions (`circle()`/`ellipse()`/`inset()`/`polygon()`/`rect()`/`shape()`/`xywh()` completed, `path()` from item 26) and `BorderRadius`, built and tested (`BasicShapeValue` compile-only). The `rect()` shape is `RectShape` (clashes with `css::Rect`). See its entry.
- Slice 4, tier 3 item 28 (2026-10-13): `FilterValue`, with `Filter`, `FilterReference` and the ten filter functions (the folder renamed `CSS/Values/FilterEffects/`), built and tested (`FilterValue` compile-only). See its entry.
- Slice 4, tier 3 item 29 (2026-10-13): `AppleColorFilterValue`, with `AppleColorFilter` and `AppleInvertLightness`, built and tested (`AppleColorFilterValue` compile-only). See its entry.

**Open / next candidates:**
- Slice 4 (Value subclasses): the generic types are done; next are the specific value types, one at a time, in the order under Slice 4's *Order* (tiers 1 and 2 done; tier 3 items 24–29 done; next `RayValue`).
- Not yet ported, needed later: the graphics `Path` (cached by `SVGPathByteStream` upstream), the rest of `SVGPathUtilities` (needs `Path` and `SVGPathSegList`), `parseKerningUnicodeString`/`parseGlyphName`.
- Slice 3 (colour), bottom-up: (1) Graphics/Color, (2) calc-free numeric primitives, (3a) Style-free calc, (3b) calc evaluation and Style calc and (3c) calc parsing are done. (4a) `LayoutUnit` and the point/size types and (4b) Style numerics are done. (5a) the CSS colour types, (5b) the colour parser and (5c) `ColorValue`/`ValuePool` (compile-only) are done. (6) Style colour is done (its CSSValue half is deferred, see below). Slice 3 is complete apart from the Value-dependent pieces.
- Deferred from layer 6 (needs `Style/BuilderChecking.hpp` and the raw-WebKit `ValueList`/`ValuePair`/`FunctionValue`/`BuilderState` it includes): `CSSValueConversion<Color>`, `CSSValueCreation<Color>`, `CSSValueConversion<DynamicRangeLimit>`, `CSSValueCreation<DynamicRangeLimit>` and `CSSValueConversion<Opacity>`. They come with the other `+CSSValueConversion`/`+CSSValueCreation` headers.
- Waiting on `PrimitiveValue.cpp`/`Value.cpp` linking: `PropertyParserState::Pool` is a pointer (TODO: back to a reference defaulting to `MainThreadPool()`), `ColorValueConsumers.cpp` and `ColorAdjustValueConsumers.cpp` compile but should merge back into `Color.cpp`/`ColorAdjust.cpp`, `ValuePool::CreateFontFaceValue` (also needs `ValueList`/`PropertyParser`) and `ParserFastPaths::MaybeParseValue` aren't written, nothing in 5c is tested, and the Style `+CSSValueConversion`/`+CSSValueCreation` headers (numerics, `LengthWrapper`, `PrimitiveKeyword`) are still raw WebKit.
- Style `ToCSS`/serialisation can't be tested until there's a real `RenderStyle` (the seam's can't be constructed).
- Only the `Number` consumer definitions are reviewed; the other `Consumers/*Definitions.hpp` (and `IntegerDefinitions.hpp`, which still uses `CalcAllowedSymbols`) come with the slices that need them.
- The Style seam (`Style/Seam.hpp`, `src/Style/Seam.cpp`) stands in for `RenderStyle`, `RenderView`, `RenderBox`, the font types, `Style::BuilderState`, `AnchorPositionEvaluator`, `ContainerQueryEvaluator` and the `Document` colour API. Each placeholder is replaced when its slice is ported; see the layer 3b and layer 6 sections. Since layer 6, `RenderStyle` can be constructed (colour state only); its other members are still unreachable.
- `Value.cpp` and `PrimitiveValue.cpp` can't build until every `Value` subclass compiles (`VisitDerived`). The `AbsoluteColorResolver.hpp` blocker found by a trial compile of `PrimitiveValue.cpp` is fixed (5a), and `ValuePool.hpp`/`ColorValue.hpp` build (5c).
- `CSS/Values/Concepts.hpp` (`CSSValueConcepts.h`) compiles but isn't reviewed.
- URL, not yet done: the DOM `URL` interface and `URLSearchParams` (WebKit's `DOMURL`/`URLDecomposition`, tested by WPT `setters_tests.json`), `Document`'s URL/base URL (`Document::URL()` is a TODO), `CompleteURL(string, Document)`, and `applyModifiersToLoaderOptions` (needs the resource loader).
- Krystal.Core: the `IntrusivePtr` `Downcast`/`DynamicDowncast` overloads can't be called with an explicit target and `Downcast` rejects null (see the `CounterValue`/`OffsetRotateValue` entry); `OffsetRotateValue.cpp` has a FIXME workaround.
- `CSS/Properties/Property.hpp` (`CSSProperty.h`) isn't reviewed, but compiles (the generated `PropertyId.cpp` defines its static functions); the colour parser and `AngleValidator` use `AcceptsQuirkyColor`/`AcceptsQuirkyAngle` from it.
- Callers still using the case-sensitive raw lookups: `src/CSS/Parser.cpp:477`, `Consumers/Animations.cpp:154`, `Consumers/WillChange.cpp:48` (switch to `PropertyIdFromName`/`ValueIdFromName`); delete the commented-out originals in `src/CSS/Properties/PropertyParser.cpp`.
- Tokenizer doesn't keep numeric original text or whitespace counts, so `TokenSerialisationMode::CustomProperty` behaves like `Normal` (FIXMEs in `src/CSS/Parser/Token.cpp`).
- `LayoutUnit` now exists: `PrimitiveValue`'s `ResolveAsLength<LayoutUnit>` and `maxValueForCssLength`/`minValueForCssLength` (FIXME in `PrimitiveValue.hpp`) can be added when `PrimitiveValue.cpp` is built.
- Not yet ported, needed later: `StringBuilder`, `RefCountedFixedVector`, `DefaultHash<StringAtom>` (Krystal.Core; for `add(Hasher&, const Style::GridPosition&)`).
- Style `GridPosition`'s `CSSValueConversion` is deferred with the other `+CSSValueConversion` headers.
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
| `wtf/Brigand.h` | vcpkg `brigand` | Already linked by Booey's CMake. Keep `brigand::list` and brigand algorithms wherever upstream uses them. Core doesn't link brigand, so its `VariantOrSingle` takes a pack: upstream's `VariantOrSingle<List>` is `brigand::wrap<List, VariantOrSingle>`. |

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
| `wtf/URL.h`, `URLParser.h`, `URLHash.h` | `have` → `url::URL`, `url::URLParser` in `Krystal.Booey/URL/` | WHATWG URL, UTF-8 only; ICU (vcpkg `icu`) for IDNA. See the `UrlValue` entry. |
| `wtf/text/CodePointIterator.h` | `have*` → `text::CodePointIterator` in `Text/CodePointIterator.hpp` | UTF-8 only (one iterator instead of the Latin-1/UTF-16 templates). |

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
- `Value::ValueId()` called `PrimitiveValue::ValueId()`, which didn't exist, so it resolved back to `Value::ValueId()` (infinite recursion). Added `PrimitiveValue::ValueId()`.
- Type checks used the stored unit instead of `PrimitiveType()`, so a `calc()` resolving to a number/percentage/length wasn't one (`IsNumber`, `IsInteger`, `IsPercentage`, `IsLength`, `IsFlex`, …). Now upstream's split of `PrimitiveType()` vs `PrimitiveUnitType()`.
- `PropertyId()` asserted for non-property values; upstream returns `Invalid`.
- `ValueDeprecated(UnitType)` called a `DoubleValueDeprecated(UnitType)` that didn't exist.
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
- `ResolveAsLength` is specialised for `int64/uint64/int32/uint32/int16/uint16/float/double`; no `LayoutUnit` specialisation and no `maxValueForCssLength`/`minValueForCssLength` yet (`LayoutUnit` exists since layer 4a of slice 3).
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

Entry: `css/CSSColorValue.cpp` (headers-only graph: 42 files, `tools/port/out/color-value.md`; with every `css/values/color` file as an entry: 158 files, `tools/port/out/color.md`, which reaches the style builder through `CSSLightDarkColor.cpp`/`CSSKeywordColor.cpp`). `ColorValue` itself is a thin wrapper; the work is in `css::Color` (Krystal has 4 of upstream's 43 alternatives). Decision (2026-10-12): full colour slice, bottom-up, building and testing each layer; layer 1 (Graphics/Color) first, then re-plan.

### Layer 1: Graphics/Color (2026-10-12)

Graph from every `platform/graphics/Color*`, `DestinationColorSpace` and `AlphaPremultiplication` file with `--follow platform/graphics/` (headers-only): 31 files, all entries except `PlatformColorSpace.h` and the CG/Skia colour-space headers (`tools/port/out/graphics-color.md`). Boundaries: `platform/animation/AnimationUtilities.h`, `wtf/text/MakeString.h`/`StringBuilder.h`/`HexNumber.h`, `wtf/MathExtras.h`, `wtf/Hasher.h`/`HashTraits.h`, `TextStream`. These files were already built (outside the CSS lists) but never reviewed or tested.

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/graphics/AlphaPremultiplication.h` | `have` | `Graphics/Color/AlphaPremultiplication.hpp` | `operator<<(TextStream&)` not ported. |
| `platform/graphics/Color.h`, `.cpp` | `have` | `Graphics/Color/Color.hpp`, `ColorFlags.hpp`, `src/.../Color.cpp` | tested. `Flags`/`FlagsIncludingPrivate` are the namespace-level `ColorFlags`/`ColorFlagsIncludingPrivate` (flags enums, not `OptionSet`). `colorSpace()` is `Space()` (a member named `ColorSpace` would hide the type). |
| `platform/graphics/ColorHash.h` | `port` | `Graphics/Color/ColorHash.hpp` | tested. `HashTraits<Color>`; the `add(Hasher&, Color)` equivalent is `DefaultHash<Color>` in `Color.hpp`. |
| `platform/graphics/ColorBlending.h`, `.cpp` | `port` | `Graphics/Color/ColorBlending.hpp`, `src/.../ColorBlending.cpp` | tested. New. |
| `platform/animation/AnimationUtilities.h` | `port` (partial) | `Graphics/Animation/AnimationUtilities.hpp` | tested. `BlendingContext` and the `int`/`unsigned`/`float`/`double` `Blend` overloads; the `IntPoint`/`FloatPoint`/`LayoutPoint`/`LayoutUnit` overloads were added in layer 4a. Members are PascalCase (`Progress`, `CompositeOperation`, …). |
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
| `css/values/primitives/CSSSymbol.h`, `.cpp` | `have` | `CSS/Values/Primitives/Symbol.hpp`, `src/.../Symbol.cpp` | on | tested. |
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
| `css/calc/CSSCalcValue.h`, `.cpp` | `have` | `CSS/Calc/Value.hpp`, `src/.../Value.cpp`, `src/.../ValueEvaluation.cpp` | on (not `ValueParsing.cpp`) | tested (creation, type, equality, serialisation, dependencies). `category()`/`range()`/`tree()` → `GetCategory()`/`GetRange()`/`GetTree()`, because the plain names would hide the types. The simplify/evaluate half is in `ValueEvaluation.cpp` (built in 3b); `Parse` is in `ValueParsing.cpp` (3c). `dump`/`operator<<` not ported. |
| `css/values/primitives/CSSUnevaluatedCalc.h`, `.cpp` | `have` | `CSS/Values/Primitives/UnevaluatedCalc.hpp`, `src/.../UnevaluatedCalc.cpp`, `UnevaluatedCalcEvaluation.cpp` | on | tested through the numeric types. Was a stub; now complete. `SimplifyBase`/`Evaluate` are in `UnevaluatedCalcEvaluation.cpp` (built in 3b). |
| `css/values/primitives/CSSPrimitiveData.h` | `have` | `CSS/Values/Primitives/PrimitiveData.hpp` | on | tested. `payload`/`index`/`storage` are now private (`_payload`, `_index`, `_storage`) with friends: public PascalCase names would clash with the `Payload`/`Index`/`Storage` aliases. The union fields are `Number`/`Calc`. `raw()`/`calc()` → `GetRaw()`/`GetCalc()`. |
| `css/values/primitives/CSSPrimitiveNumeric.h` | `have` | `CSS/Values/Primitives/PrimitiveNumeric.hpp` | on | tested. Same accessor renames. |
| `css/values/primitives/CSSPrimitiveNumericOrKeyword.h` | `have` | `CSS/Values/Primitives/PrimitiveNumericOrKeyword.hpp` | on | tested. Was unconverted; rewritten. |
| `css/values/primitives/CSSPrimitiveNumericTypes.h` | `have` | `CSS/Values/Primitives/NumericTypes.hpp` | on | tested. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h`, `.cpp` | `have` | `NumericTypesSerialisation.hpp`, `.cpp` | on | tested. Added the `SerialisableNumber`, raw and `NumberOrPercentageResolvedToNumber` serialisers. |
| `css/values/primitives/CSSPrimitiveNumericTypes+ComputedStyleDependencies.h`, `.cpp` | `have` | `NumericTypesComputedStyleDependencies.hpp`, `.cpp` | on | tested. The `.cpp` (the `LengthUnit` collector) was missing; now ported. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Canonicalization.h`, `.cpp` | `have` | `NumericTypesCanonicalization.hpp` | on | tested. Defined inline rather than in a `.cpp`. |
| `css/values/primitives/CSSPrimitiveNumericTypes+SymbolReplacement.h`, `.cpp` | `have` | `NumericTypesSymbolReplacement.hpp`, `.cpp` | on | tested. The `.cpp` was missing; now ported. |
| `css/values/primitives/CSSPrimitiveNumericTypes+EvaluateCalc.h` | `have` | `NumericTypesEvaluateCalc.hpp` | on | `RequiresConversionData`/`IsUnevaluatedCalc` tested; evaluation tested in 3b. |
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

### Layer 3b: calc evaluation and Style calc, against the Style seam (2026-10-12)

Graph from `css/calc/CSSCalcTree+{Evaluation,Simplification,Parser}.cpp` and `CSSCalcValue.cpp` (headers-only): 123 files (`tools/port/out/calc-3b.md`). The explosion is `StyleBuilderState.h` (cascade, rule sets, selectors, DOM), `RenderStyle`/`RenderView`/fonts through `CSSToLengthConversionData` and `StyleLengthResolution`, and the property parser through `CSSCalcTree+Parser`. Decisions (2026-10-12): (1) a narrow seam instead of porting the style resolver: the rendering, font and resolver types calc touches are reduced to declarations of only the members it calls, in `Style/Seam.hpp`; (2) calc parsing is its own step, 3c.

**The seam.** `Style/Seam.hpp` declares `FontMetrics`, `FontCascadeDescription`, `FontCascade`, `RenderStyle`, `RenderView`, `RenderBox` (`krys::boo`) and `style::BuilderState`, `style::AnchorPositionEvaluator`, `style::ContainerQueryEvaluator`, `style::EvaluationTimeZoomEnabled`, under their upstream names (PascalCase). None of the classes can be constructed (deleted constructors), so no caller can reach the members; every definition in `src/Style/Seam.cpp` is `krys_unreachable()`. Calc works fully with no conversion data, `NoConversionDataRequiredTag`, or a default-constructed `ToLengthConversionData` (no style, view or builder), which is what the tests use. A few members collapse an upstream call chain whose types aren't ported:

| Seam member | Upstream |
|---|---|
| `RenderStyle::HasPseudoElementType()` | `!style.pseudoElementType()` |
| `RenderStyle::EvaluateFixedLineHeight(ZoomFactor)` | `style.lineHeight().tryFixed()` + `Style::evaluate<LayoutUnit>(…).toFloat()` |
| `RenderStyle::ComputeSpecifiedLineHeight()` | `style.computeLineHeight(style.specifiedLineHeight())` |
| `RenderViewOf(document)`, `RenderStyleOf(element)`, `RenderBoxOf(element)` | `document.renderView()`, `element.renderStyle()`, `dynamicDowncast<RenderBox>(element.renderer())` |
| `AdjustFloatForAbsoluteZoom(value, style)` | `adjustFloatForAbsoluteZoom` (`RenderStyle+GettersInlines.h`) |
| `BuilderState::EvaluationTimeZoomEnabled()` | `state.document().settings().evaluationTimeZoomEnabled()` (added in 4b) |

The unreviewed `Style/BuilderState.hpp` and `Style/AnchorPositionEvaluator.hpp` (raw WebKit) still declare the full classes; nothing built includes them. When the style resolver is ported, the real classes replace the seam's and the collapsed members go back to upstream's call chains.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/calc/CSSCalcTree+Evaluation.h`, `.cpp` | `port` | `CSS/Calc/TreeEvaluation.hpp`, `src/.../TreeEvaluation.cpp` | on | tested. |
| `css/calc/CSSCalcTree+Simplification.h`, `.cpp` | `port` | `CSS/Calc/TreeSimplification.hpp`, `src/.../TreeSimplification.cpp` | on | tested (every operation). |
| `css/calc/CSSCalcTree+Mappings.h` | `have` | `CSS/Calc/TreeMappings.hpp` | on | All 58 mappings compared mechanically with upstream. |
| `css/calc/CSSCalcValue.cpp` (evaluation half) | `have` | `src/CSS/Calc/ValueEvaluation.cpp` | on | tested. `Parse` moved to `ValueParsing.cpp` (3c, not built). |
| `css/values/primitives/CSSUnevaluatedCalc.cpp` (evaluation half) | `have` | `src/.../UnevaluatedCalcEvaluation.cpp` | on | tested through `EvaluateCalc`. |
| `css/CSSToLengthConversionData.h`, `.cpp` | `have` | `CSS/ToLengthConversionData.hpp`, `src/CSS/ToLengthConversionData.cpp` | on | tested (default state and copies). |
| `style/values/primitives/StyleLengthResolution.h`, `.cpp` | `port` | `Style/Values/Primitives/LengthResolution.hpp`, `src/.../LengthResolution.cpp` | on | tested for absolute, viewport (no view) and container (no element) units. |
| `style/calc/StyleCalculationTree.h`, `.cpp` | `have` | `Style/Calc/Tree.hpp`, `src/Style/Calc/Tree.cpp` | on | tested. `operator<<` not ported. |
| `style/calc/StyleCalculationTree+Traversal.h` | `port` | `Style/Calc/TreeTraversal.hpp` | on | tested. |
| `style/calc/StyleCalculationTree+Copy.h`, `.cpp` | `port` | `Style/Calc/TreeCopy.hpp`, `src/.../TreeCopy.cpp` | on | tested. |
| `style/calc/StyleCalculationTree+Evaluation.h`, `.cpp` | `port` | `Style/Calc/TreeEvaluation.hpp`, `src/.../TreeEvaluation.cpp` | on | tested. |
| `style/calc/StyleCalculationTree+Conversion.h`, `.cpp` | `port` | `Style/Calc/TreeConversion.hpp`, `src/.../TreeConversion.cpp` | on | `ToStyle` tested through `CreateCalculationValue`; `ToCSS` needs a `RenderStyle`. |
| `style/calc/StyleCalculationValue.h`, `.cpp` | `port` | `Style/Calc/Value.hpp`, `src/.../Value.cpp` | on | tested. `operator<<` not ported. |
| `style/calc/StyleCalculationValueMap.h`, `.cpp` | | | | Done in layer 4b. |
| `style/values/viewport/StyleZoomPrimitives.h` | `port` | `Style/Values/Viewport/ZoomPrimitives.hpp` | on | |
| `style/StyleScopeOrdinal.h` | `port` | `Style/ScopeOrdinal.hpp` | on | tested. `operator<<` not ported. |
| `style/ScopedName.h` | `have*` | `Style/ScopedName.hpp` | on | tested. The struct only: `switchOn` (needs `Style::CustomIdentifier`), `CSSValueConversion`, `MarkableTraits` and `operator<<` not ported. |
| `css/query/ContainerQuery.h` (`CQ::Axis` only) | `have` | `CSS/Queries/ContainerQueryAxis.hpp` | on | The enum moved out of the unreviewed `ContainerQuery.hpp` (which now includes it; also removed a stray `}` at its end). |
| `platform/graphics/FloatSize.h`, `.cpp` | `have*` | `Graphics/FloatSize.hpp` | on | tested. Everything except `IntSize`/`DoubleSize` conversions, CG, JSON, `isExpressibleAsIntSize` and `operator<<`. |
| rendering, font and resolver types | seam | `Style/Seam.hpp`, `src/Style/Seam.cpp` | on | See above. |

**Bugs fixed:**
- `Style/Calc/Tree.hpp`: `Child` and `ChildOrNone` constructors were declared without `noexcept` but defined with it (wouldn't compile); `Children Children` members hid their type; `IndirectNode ==` dereferenced null nodes; `Blend`'s progress member was `CurrentProgress` (now `Progress`, like upstream); comments had capitalised words ("To", "Value") from a bad rename; `SmallList` → `List`, like the CSS tree.
- `ToLengthConversionData.hpp` forward-declared `RenderStyle`/`RenderView` in `style` instead of `krys::boo`; `RenderView()` hid its type (now `GetRenderView()`); `ProtectedStyleBuilderState` was missing; the `.cpp` didn't exist. `CSS/Calc/Value.hpp` had the same `style::RenderStyle` mistake.
- `LengthResolution.hpp` was raw WebKit.
- `TreeEvaluation.hpp`/`TreeSimplification.hpp`: `SymbolTable` members hid their type.
- `UnevaluatedCalcEvaluation.cpp` didn't include its own header first and called a misspelt `CSSToLengthConversionData()`.
- Upstream bugs fixed (behaviour differs from upstream):
  - `random()` evaluation checked `min` twice and never checked `max`.
  - `random()` simplification required an element when the value *was* element-shared, the reverse of evaluation and of `ToStyle` (a non-element-shared value is keyed on the element, so it's the one that needs it).
  - Negating a product of numerics negated every factor, so `-(a * b)` became `(-a) * (-b)`; now only the first factor is negated.
  - `ComputeUnzoomedNonCalcLengthDouble`'s comment says every viewport unit is 0 without a render view; the `*min`/`*max` units return the value unchanged. Comment corrected (behaviour kept), and its misspelt unit list (`Rca, Rc, Re, Re, Ri`) fixed.

**Differences:**
- `ToLengthConversionData` declares its assignment operators and `StyleBuilderState()` out of line, because `CheckedPtr<BuilderState>` needs the complete type.
- `Style::calc::Random::fixed` is `FixedValue` and `Progress::progress` is `CurrentProgress`: a member can't share the name of its class or of a nested type it's declared alongside.
- `Style::Calculation::number`/`percentage`/`dimension`/`blend` → `CreateNumber`/`CreatePercentage`/`CreateDimension`/`BlendChildren` (the plain names are types).
- Known upstream behaviour kept, with `[!shouldfail]` tests: trigonometric functions evaluate their argument as radians; simplification converts degrees first but evaluation doesn't, so an angle that reaches evaluation unsimplified (e.g. `sin(sibling-index() * 10deg)` through `DoubleValue`) is wrong.
- Upstream quirks kept: Style calc `Number` leaves are multiplied by the zoom like dimensions; `CreateCalculationValue(NoConversionDataRequiredTag)` on a length dereferences the empty conversion data (asserts in debug); `ComputeCanonicalNonCalcLengthDouble` with conversion data that has no style dereferences the null style unless computing font-size.

Tests: `tests/CSS/Calc/{TreeEvaluation,TreeSimplification,Value}.cpp`, `tests/Style/Calc/Tree.cpp`, `tests/Style/ScopedName.cpp`, `tests/Style/Values/Primitives/LengthResolution.cpp`, `tests/Graphics/FloatSize.cpp`, and `EvaluateCalc` in `tests/CSS/Values/Primitives/PrimitiveNumeric.cpp`. Full Booey suite: 687 cases / 11116 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

---

### Layer 3c: calc parsing (2026-10-12)

Graph from `css/calc/CSSCalcTree+Parser.cpp` (headers-only): 75 files (`tools/port/out/calc-3c.md`). Most of the graph comes in through headers the parser includes but barely uses: `CSSPropertyParserState.h` → `CSSValuePool.h` → `CSSColorValue.h` → `css/values/color` (layer 5), `CSSPropertyParserConsumer+Ident.h` → `CSSPrimitiveValue.h`, and `CSSPropertyParserConsumer+MetaConsumer.h` → `StylePrimitiveNumericTypes.h` (layer 4). What the parser actually calls: the token range and guard, `consumeFunction`/`consumeCommaIncludingWhitespace`, `consumeIdentRaw<…>`/`consumeDashedIdentRaw`, `isValidCustomIdentifier`, five `CSSParserContext` flags, four `PropertyParserState` fields, and `MetaConsumer<CSS::Number<ClosedUnitRange>>` for `random(fixed …)`. Decision (2026-10-12): `PropertyParserState` forward-declares `ValuePool` and holds a pointer, so none of the pool/colour chain is pulled in.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/calc/CSSCalcTree+Parser.h`, `.cpp` | `port` | `CSS/Calc/TreeParser.hpp`, `src/.../TreeParser.cpp` | on | tested. The `.cpp` was missing; now ported. `isCalcFunction` stays `constexpr` in the header. The repeated "simplify if options were given, else make the child" tail of every consumer is one helper, `SimplifyOrCreateChild`. `LOG_WITH_STREAM` not ported. |
| `css/calc/CSSCalcValue.cpp` (`parse`) | `have` | `src/CSS/Calc/ValueParsing.cpp` | on | tested end-to-end from strings. |
| `css/parser/CSSPropertyParserState.h` | `have*` | `CSS/Properties/PropertyParserState.hpp` | on | See bugs and differences. |
| `css/parser/CSSPropertyParserOptions.h` | `have*` | `CSS/Properties/PropertyParserOptions.hpp` | on | `AnchorPolicy`/`AnchorSizePolicy`/`UnitlessZeroQuirk` enums are `StronglyTypedBool`s (`AllowAnchor`, `AllowAnchorSize`, `AllowUnitlessZero`); `OverrideParserMode` is the last member instead of the first. Same defaults. |
| `css/parser/CSSParserTokenRangeGuard.h` | `have` | `CSS/Parser/TokenRangeGuard.hpp` | on | tested. |
| `css/parser/CSSParserTokenRange.h` (`span()`) | `have*` | `CSS/Parser/TokenRange.hpp` | on | Added `Tokens()` (upstream `span()`; the plain name would hide `Span`). |
| `css/parser/CSSParserContext.h` | `have*` | `CSS/Parser/ParserContext.hpp` | on | Only the fields the parser reads were checked. No `baseURL`, `charset`, `enclosingRuleType`, `hasDocumentSecurityOrigin`, `loadedFromOpaqueSource`, constructors, hashing or `strictCSSParserContext()`. Flag members keep upstream's camelCase (as does the generated `PropertySettings`). |
| `css/parser/CSSPropertyParserConsumer+Primitives.h`, `.cpp` | `have` | `Consumers/Primitives.hpp`, `src/.../Primitives.cpp` | on | tested. `consumeCommaIncludingWhitespace`/`consumeSlashIncludingWhitespace` → `ConsumeComma`/`ConsumeSlash` (upstream FIXME's names). |
| `css/parser/CSSPropertyParserConsumer+Ident.h`, `.cpp` | `have` | `Consumers/Ident.hpp`, `src/.../Ident.cpp` | header on, `.cpp` off | Raw consumers tested. The value-returning `ConsumeIdent`/`ConsumeIdentRange`/`ConsumeCustomIdent`/`ConsumeDashedIdent` moved to `Ident.cpp` (as upstream): `PrimitiveValue::Create(ValueId)` needs `ValuePool`, so it waits for layer 5. `consumeIdentUsingMapping`/`peekIdentUsingMapping` not ported (no `SortedArrayMap`). Extra: `DiscardIdent`. Raw custom/dashed idents return `Maybe<CSSOMString>` (`null` for upstream's null `String`). |
| `css/parser/CSSPropertyParserConsumer+MetaConsumer.h` | `have` | `Consumers/MetaConsumer.hpp` | on | tested (`Number`, calc, functor overload). |
| `css/parser/CSSPropertyParserConsumer+MetaConsumerDefinitions.h` | `have*` | `Consumers/MetaConsumerDefinitions.hpp` | on | tested through `Number`. `PercentageConsumer` takes the percentage unit as a third template argument. |
| `css/parser/CSSPropertyParserConsumer+NumberDefinitions.h` | `have` | `Consumers/NumberDefinitions.hpp` | on | tested. |
| `css/parser/CSSParserToken.cpp` (`stringToUnitType`) | `port` | `ParseUnitType` in `CSS/Values/UnitType.hpp` | on | tested. Was a `krys_not_implemented()` stub. A table with ASCII case-insensitive matching instead of upstream's generated trie; same 63 names including `__qem`. |
| `css/StyleRuleType.h` | `have*` | `CSS/Rules/RuleType.hpp` | on | Different order and two extra values (`Unknown`, `Viewport`); upstream's values are the CSSOM `CSSRule.type` numbers, so they'll need matching when `CSSRule` is ported. |
| `css/IsImportant.h` | `have*` | `CSS/Properties/Types/IsImportant.hpp` | on | `StronglyTypedBool` instead of `enum class : bool`. |
| `css/CSSValuePool.h`, `CSSPrimitiveValue.h`, `CSSColorValue.h`, `StylePrimitiveNumericTypes.h` | | | | Not needed by the parser; see the decision above. |
| `platform/Logging.h`, `wtf/SortedArrayMap.h` | `wont` | | | Logging dropped; the constant table is a `switch`. |

**Bugs fixed:**
- `MetaConsumerDefinitions.hpp`:
  - `FunctionConsumerForCalcValues` returned `null` even when `calc()` parsed (a TODO), so no primitive could be written with `calc()`.
  - `KeywordConsumer` compared against `T::value`; the generated keyword constants name it `Value`.
  - It and `MetaConsumer.hpp` included the non-existent `Calc/CalcAllowedSymbols.hpp` and used `CalcAllowedSymbols` (now `AllowedSymbols`).
- `ConsumeDashedIdentRaw` dereferenced an empty `Maybe` when the next token wasn't a valid custom identifier (undefined behaviour). `ConsumeCustomIdent`/`ConsumeDashedIdent` created string values instead of custom idents.
- `PropertyParserState`: no `CSSRandomFunctionCount`; `CurrentRule`, `CurrentProperty` and `Important` had no defaults (upstream: `Style`, `Invalid`, `No`); the `ValuePool ValuePool` member hid its type.
- `ParserContext::Mode` was uninitialised (upstream `HTMLStandardMode`).
- `RuleType`'s enum traits declared 22 values; it has 30.
- `TreeParser.hpp` used `ValueId` enumerators with only a forward declaration of the enum.
- `ValueParsing.cpp` didn't include its own header first.
- Upstream bug fixed (behaviour differs from upstream): `anchor-size()` with no element or size ignored a failed fallback, but the failed parse had already consumed a token. So any single token that wasn't a value (`anchor-size(top)`, `anchor-size(foo)`) parsed as `anchor-size()`. A fallback that fails to parse now rejects the function.

**Differences:**
- `PropertyParserState::Pool` is a `ValuePool *` defaulting to `nullptr` instead of `CSSValuePool& pool { CSSValuePool::singleton() }` (TODO in the code); it goes back to a reference once `ValuePool` builds (layer 5).
- Tokens have no `unitType()`; dimensions are mapped with `ParseUnitType(token.Unit())` where they're consumed.
- `ConsumeSlash` checks `Delimiter()` rather than the token's text (both work; this matches upstream). Asserts in `Primitives.cpp` are `krys_debug_assert`.

Tests: `tests/CSS/Calc/TreeParser.cpp` (arithmetic, whitespace rules, type checking, depth limit, constants, symbols, every math function, the feature-flag/rule/property gates on `progress()`/`random()`/sibling functions, `anchor()`/`anchor-size()`), `tests/CSS/Properties/Consumers/{Ident,MetaConsumer,Primitives}.cpp` (also `TokenRangeGuard` and `TokenRange::Tokens`), `ParseUnitType` in `tests/CSS/Values/Units.cpp`. Full Booey suite: 724 cases / 11606 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

---

### Layer 4a: LayoutUnit and geometry (2026-10-12)

Graph from `platform/LayoutUnit.h`, `platform/graphics/LayoutPoint.h`, `LayoutSize.h` and `FloatPoint.h` with `--follow platform/` (headers-only): 20 files (`tools/port/out/layout-unit.md`); the rest comes in through `FloatPoint.h` (`FloatRect`, `AffineTransform`, `TransformationMatrix`) and `DoublePoint`/`DoubleSize`. Layer 4 needs these for `StylePrimitiveNumericTypes+Evaluation.h` and `StyleLengthWrapper.h` (graph from the Style numerics: 35 files, `tools/port/out/style-numerics.md`). Decisions (2026-10-12): port `LayoutUnit` and the point/size types first, as 4a; the Style numerics are 4b; the `+CSSValueConversion`/`+CSSValueCreation` headers (CSS-side `PrimitiveValue`/`ValuePool`) wait for layer 5.

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/LayoutUnit.h`, `.cpp` | `port` | `Core/LayoutUnit.hpp` | tested (upstream's `LayoutUnitTests.cpp`, all of it, plus the rest of the API). The `.cpp` is only `operator<<`: not ported. `kFixedPointDenominator` etc. → `FixedPointDenominator`, `LayoutUnitFractionalBits`, `IntMaxForLayoutUnit`, `IntMinForLayoutUnit`. `DefaultHash`, `HashTraits`, `MarkableTraits` and the `_lu` literal ported. |
| `platform/graphics/IntSize.h`, `.cpp` | `port` | `Graphics/IntSize.hpp`, `src/Graphics/IntSize.cpp` | tested. No `area()` (needs `Checked<unsigned>`), JSON, CG/Win conversions or `operator<<`. |
| `platform/graphics/IntPoint.h`, `.cpp` | `port` | `Graphics/IntPoint.hpp`, `src/Graphics/IntPoint.cpp` | tested (upstream's `IntPointTests.cpp`). No `constrainedWithin(IntRect)`, CG/Win conversions or `operator<<`. |
| `platform/graphics/FloatPoint.h`, `.cpp` | `port` | `Graphics/FloatPoint.hpp` | tested. No `constrainedWithin(FloatRect)`, `matrixTransform` (transforms), `DoublePoint` conversions, JSON, CG/Win or `operator<<`. `normalize()` → `Normalise()`. `add(Hasher&, FloatPoint)` → `DefaultHash<FloatPoint>`. |
| `platform/graphics/LayoutSize.h`, `.cpp` | `port` | `Graphics/LayoutSize.hpp` | tested. `AspectRatioFitShrink`/`Grow` → `enum class AspectRatioFit { Shrink, Grow }`; `LayoutSizeLimits::m_min`/`m_max` → `Min`/`Max`. `constrainedBetween` is inline. |
| `platform/graphics/LayoutPoint.h`, `.cpp` | `port` | `Graphics/LayoutPoint.hpp` | tested. No `DoublePoint` conversions or `operator<<`. `LayoutPointLimits` members as for `LayoutSizeLimits`. |
| `platform/graphics/FloatSize.h` (IntSize half) | `have` | `Graphics/FloatSize.hpp` | tested. Added `FloatSize(const IntSize&)`, `IsExpressibleAsIntSize`, `RoundedIntSize`, `FlooredIntSize`, `ExpandedIntSize`, `FlooredIntPoint`. |
| `platform/animation/AnimationUtilities.h` (rest) | `port` | `Graphics/Animation/AnimationUtilities.hpp` | tested. The `LayoutUnit`, `IntPoint`, `FloatPoint` and `LayoutPoint` `Blend` overloads (the TODO is gone). |
| `platform/graphics/DoublePoint.h`, `DoubleSize.h`, `FloatRect.h`, `IntRect.h`, `transforms/*` | | | Ported with `PathValue` (Slice 4). |
| `wtf/SaturatedArithmetic.h` | `port` | `SaturatedSum`/`SaturatedDifference` in `Krystal.Core/Maths/Base.hpp` | tested through `LayoutUnit`. Portable versions only (no builtins/asm). |
| `wtf/MathExtras.h` (`clampToInteger`, `isWithinIntRange`) | `port` | `ClampToInteger`, `IsWithinIntRange` in `Krystal.Core/Maths/Base.hpp` | The `double`/`float` overloads only. |

The Style seam's `RenderBox::ContentBoxWidth/Height()` now return `LayoutUnit`, like upstream (they returned `float`).

**Bugs fixed (upstream):**
- `LayoutUnit(unsigned long long)` multiplied by 64 before clamping, so values from 2^58 up wrapped instead of saturating. It now clamps first (same results otherwise).
- `LayoutUnit::abs()` of `min()` was `::abs(INT_MIN)` (undefined behaviour); it now saturates to `Max()`, like unary minus.

**Differences:**
- `REPORT_OVERFLOW` (logging only) isn't ported: `ToUnsigned()` of a negative value and `SetRawValue(int64)` out of range don't log.
- No `unsigned long` constructor or operator overloads (`uint16`/`uint32`/`uint64` only; `unsigned long` is a distinct 32-bit type on Windows that nothing uses). No private `isInBounds` helpers (unused upstream).
- Added `LayoutUnit::operator=(int32)`: upstream has only `operator=(const float&)`, which overload resolution prefers over the implicit `int` constructor, so assigning an integer goes through `float` (and loses precision above 2^24).
- Most of `LayoutUnit` is `constexpr` (not `FromFloatCeil`/`FromFloatFloor`/`CeilToFloat` or the device-pixel functions, which use `<cmath>`).

Tests: `tests/Core/LayoutUnit.cpp`, `tests/Graphics/{IntSize,IntPoint,FloatPoint,LayoutSize,LayoutPoint}.cpp` (also the new `Blend` overloads): 34 cases / 640k assertions (upstream's pixel-snapping loops). Full Booey suite: 758 cases / 652100 assertions; all pass except 2 expected failures (`[!shouldfail]`).


---

### Layer 4b: Style numerics (2026-10-12)

Graph from `style/values/primitives/StylePrimitiveNumericTypes.h`, `StyleLengthWrapper.h`, `StyleLengthWrapperData.cpp` and `style/calc/StyleCalculationValueMap.cpp` (headers-only): 35 files (`tools/port/out/style-numerics.md`); the `.cpp`s add `StyleLengthResolution`, the `RenderStyle` getters and `Settings`. Decisions (2026-10-12): `CompactVariant`/`FlatteningVariantAdaptor` are ported into Krystal.Core first, so `LengthPercentage` keeps upstream's 8-byte layout and `PrimitiveNumericOrKeyword` flattens like upstream; the `+CSSValueConversion`/`+CSSValueCreation` headers (and `StyleLengthWrapper+CSSValueConversion`/`+DeprecatedCSSValueConversion`) wait for layer 5.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `wtf/CompactVariant.h`, `CompactVariantOperations.h` | `port` | `Krystal.Core/Types/CompactVariant.hpp` | Core | tested (upstream's `TestWebKitAPI/Tests/WTF/CompactVariant.cpp`). One header. `valueless_by_move` → `IsValuelessByMove`; `holdsAlternative`/`switchOn` → `HoldsAlternative`/`SwitchOn`; the `CompactVariantTraits` functions are PascalCase. |
| `wtf/FlatteningVariantAdaptor.h` | `port` | `Krystal.Core/Types/FlatteningVariant.hpp` | Core | tested. Its own type list (`FlatteningTypeList`) instead of `brigand`, which Core doesn't link. |
| `wtf/VariantExtras.h`, `wtf/StdLibExtras.h` (`typeForIndex`, `VariantBestMatch`, `alternativeIndexV`, `IsStdInPlace*`) | `port` | `TypeForIndex`, `VariantBestMatch`, `AlternativeIndexV`, `IsInPlaceType`/`IsInPlaceIndex` in `Krystal.Core/Types/Variant.hpp` | Core | `TypeForIndex` is a recursive `if constexpr` chain instead of the unrolled switch. |
| `style/values/StyleValueTypes.h` | `have` | `Style/Values/Types.hpp` | on | tested (`tests/Style/Values/Types.cpp`). See bugs. Invokers: `toCSS`/`toStyle`/`toStyleNoConversionDataRequired` → `ConvertToCSS`/`ConvertToStyle`/`ConvertToStyleNoConversionDataRequired`, `createCSSValue` → `CreateCSSValue`, `toStyleFromCSSValue` → `ConvertCSSValueToStyle`, `toPlatform` → `ConvertToPlatform`, `serializationForCSS` → `Serialise`, `evaluate` → `Evaluate`, `isZero`/`isKnownZero`/`isEmpty`/`isKnownEmpty` → `IsValueZero`/`IsValueKnownZero`/`IsValueEmpty`/`IsValueKnownEmpty` (the customisation structs keep the plain names), `Interpolation::Context` → `lerp::Context`. Logging not ported. |
| `style/values/primitives/StylePrimitiveNumericConcepts.h` | `have` | `Style/Values/Primitives/Concepts.hpp` | on | `UnevaluatedCalculation` → `UnevaluatedCalc` (the namespace tells it apart from `css::UnevaluatedCalc`). |
| `style/values/primitives/StylePrimitiveNumeric+Forward.h` | `port` | `Style/Values/Primitives/PrimitiveNumericForward.hpp` | on | |
| `style/values/primitives/StyleUnevaluatedCalculation.h`, `.cpp` | `have` | `Style/Values/Primitives/UnevaluatedCalc.hpp`, `src/.../UnevaluatedCalc.cpp` | on | tested through `PrimitiveNumeric`. Was raw WebKit. `protectedCalculation` → `ProtectedCalculation`. |
| `style/values/primitives/StylePrimitiveNumeric.h` | `have` | `Style/Values/Primitives/PrimitiveNumeric.hpp` | on | tested. Was raw WebKit. Public `value` → `Value`, `unit` → `Unit`, `range`/`category` → `Range`/`Category`; `Length` keeps its value private (`_value`) like upstream. The composite `get<T>()` keeps its lowercase name. No IPC (`IPCData`). |
| `style/values/primitives/StylePrimitiveNumericOrKeyword.h` | `have` | `Style/Values/Primitives/PrimitiveNumericOrKeyword.hpp` | on | tested. |
| `style/values/primitives/StylePrimitiveNumericTypes.h` | `have` | `Style/Values/Primitives/NumericTypes.hpp` | on | tested. `Nonnegative` aliases → `NonNegative`, `MinimallySerializing…` → `MinimallySerialising…`. |
| `…+Calculation.h` | `have` | `NumericTypesCalc.hpp` | on | tested. `copyCalculation` → `CopyCalculation`. |
| `…+Conversions.h`, `.cpp` | `have` | `NumericTypesConversions.hpp`, `src/.../NumericTypesConversions.cpp` | on | tested (raw canonicalization, CSS → Style including calc). `ToCSS` needs a `RenderStyle`, so it's untested. The two calc `ToStyle` specializations share one helper (`detail_conversions::DimensionPercentageCalcToStyle`). `canonicalize*` → `Canonicalize*` (the CSS side kept the American spelling). |
| `…+Evaluation.h` | `have` | `NumericTypesEvaluation.hpp` | on | tested (lengths, percentages, calc, points and sizes in float and layout units, `Reflect`/`ReflectSum`). |
| `…+Serialization.h` | `have` | `NumericTypesSerialisation.hpp` | on | Compiles; untested (every specialization goes through `ToCSS`, which needs a `RenderStyle`). |
| `…+Blending.h` | `have` | `NumericTypesBlending.hpp` | on | tested. |
| `StylePrimitiveNumericOrKeyword+Conversions.h` | `have` | `PrimitiveNumericOrKeywordConversions.hpp` | on | `ToStyle` tested; `ToCSS` needs a `RenderStyle`. |
| `style/calc/StyleCalculationValueMap.h`, `.cpp` | `port` | `Style/Calc/ValueMap.hpp`, `src/Style/Calc/ValueMap.cpp` | on | tested. `HashMap` is `std::unordered_map`, so handles 0 and the maximum are skipped explicitly (WTF's empty/deleted keys). `ref`/`deref` → `AddRef`/`SubRef`. |
| `style/values/primitives/StyleLengthWrapperData.h`, `.cpp` | `have` | `Style/Values/Primitives/LengthWrapperData.hpp`, `src/.../LengthWrapperData.cpp` | on | tested. No IPC. |
| `style/values/primitives/StyleLengthWrapper.h` | `have` | `Style/Values/Primitives/LengthWrapper.hpp` | on | tested with a test-only wrapper (upstream's concrete wrappers live in the property files). `evaluateMinimum` → `EvaluateMinimum`. No IPC or logging. `LengthWrapperBaseDerived` uses an overload test instead of `IsBaseOfTemplate` (a `template <typename...>` parameter doesn't match the constrained keyword pack). |
| `style/values/primitives/StyleLengthWrapper+Blending.h` | `have` | `Style/Values/Primitives/LengthWrapperBlending.hpp` | on | tested. |
| `css/ComputedStyleDependencies.cpp` | `have` | `src/CSS/ComputedStyleDependencies.cpp` | on | Now built (`ToLengthConversionData` builds since 3b); `RenderView()` → `GetRenderView()`. |
| `css/CSSPrimitiveValue.h` (`maxValueForCssLength`/`minValueForCssLength`) | `have` | `MaxValueForCssLength`/`MinValueForCssLength` in `CSS/Values/PrimitiveValue.hpp` | header | Replaces the FIXME. |
| `…+CSSValueConversion.h`, `…+CSSValueCreation.h`, `StyleLengthWrapper+CSSValueConversion.h`, `+DeprecatedCSSValueConversion.h`, `StylePrimitiveKeyword+*` | | | off | Layer 5 (they need `PrimitiveValue`/`ValuePool`). Still raw WebKit. |

**Seam changes:** `style::EvaluationTimeZoomEnabled(const RenderStyle&)` was a seam placeholder for this layer's `+Conversions` function; it's now defined in `NumericTypesConversions.cpp` (`LengthResolution.cpp` includes that header, as upstream does). Added `BuilderState::UseSVGZoomRulesForLength()` and `BuilderState::EvaluationTimeZoomEnabled()` (collapses `document().settings().evaluationTimeZoomEnabled()`).

**Bugs fixed:**
- `Types.hpp` didn't compile when instantiated:
  - the `KeywordValueConstant` `Blending` specialization's first `Blend` had lost its last parameter (syntax error);
  - separators were `utf8_string`s `push_back`'d into the builder (and `""` assigned to a `u8string`); now `CSSOMStringView` and `append`, as on the CSS side;
  - `FunctionNotation` used `value.name`/`value.parameters` and `css::ToString`, from before the Aggregates renames; now `Name`/`Parameters` and `ToLower`, as on the CSS side;
  - the list conversions called a non-existent `List::map` (and mixed `.value`/`.Value`); now the wrappers' static `map`;
  - `IsKnownEmpty` called lowercase `isKnownEmpty()`/`isEmpty()`/`width()`, and the `HasIs*` concepts weren't qualified with `css::`;
  - the string non-converting types were `StringAtom`/`utf8_string` instead of `CSSOMStringAtom`/`CSSOMString`, and `url::Url` was commented out.
- Upstream bug fixed (behaviour differs): `IsKnownZero` fell back to `!value.isZero()` for types without `isKnownZero`, reporting non-zero values as known zero. It now returns `IsZero()`.
- Upstream bug fixed: `FlatteningVariantAdaptor::holdsAlternative<T>()` called itself with an argument it doesn't take, so any instantiation failed; now `krys::HoldsAlternative<T>(_value)`.
- Upstream bug fixed: the `MarkableTraits` of Style `NumberOrPercentage` called its private `isEmpty` without being a friend.
- Upstream bug fixed: the `SpaceSeparatedSize` `ZoomFactor` evaluations were constrained on a `float` third parameter while passing a `ZoomFactor`; now constrained on `ZoomFactor`, like the other evaluations.
- Upstream typo fixed: the calc lambdas of `ToCSS<AnglePercentage>`/`ToCSS<LengthPercentage>` returned `CSS::AnglePercentage<R>`/`CSS::LengthPercentage<R>`, dropping `V`.
- All the Style primitives (`UnevaluatedCalc`, `PrimitiveNumeric`, `PrimitiveNumericOrKeyword`, `NumericTypes` and every `+` header, `LengthWrapper*`) were raw WebKit (`<WebCore/...>` includes); rewritten against upstream.

**Differences:**
- `CompactVariant` and `CompactVariantOperations` share one header; the payload helpers aren't `constexpr` (they use placement new).
- `Canonicalize(LengthRaw, NoConversionDataRequiredTag)`'s debug check uses `ConversionToCanonicalUnitRequiresConversionData(raw.Unit)` directly: the CSS side has no raw overload of `RequiresConversionData`.
- Known upstream behaviour (from 3b): converting a calc length-percentage with `NoConversionDataRequiredTag` asserts, because `CreateCalculationValue` dereferences the missing conversion data. The tests pass a default `ToLengthConversionData`.

Tests: `tests/Style/Values/Types.cpp`, `tests/Style/Values/Primitives/{PrimitiveNumeric,NumericTypesConversions,NumericTypesEvaluation,LengthWrapper}.cpp`, Core `tests/Types/CompactVariant.cpp` (also `FlatteningVariant`). Full Booey suite: 784 cases / 652384 assertions; all pass except 2 expected failures (`[!shouldfail]`). Core suite: 542 cases, all pass. Whole solution builds with no warnings.

---

### Layer 5a: CSS colour types (2026-10-12)

Graph from every `css/values/color` file (headers-only): 156 files (`tools/port/out/color-5a.md`). Almost all of it comes in through three includes the colour code barely uses: `CSSPrimitiveValueMappings.h` (for `toCSSValueID(BlendMode)` in `CSSColorLayers.cpp`), `StyleColor.h` (included by `CSSPlatformColorResolutionState.h`, which only needs `Style::ForVisitedLink` and `StyleColorOptions`) and `Document.h`/`StyleBuilderState.h`/`StyleColorResolutionState.h` (included by `CSSLightDarkColor.cpp`/`CSSColor.cpp`, unused). None of them is pulled in. Decisions (2026-10-12): layer 5 is split into 5a (the `css::Color` types), 5b (the colour parser, `CSSPropertyParserConsumer+Color`) and 5c (`ColorValue` and `ValuePool`, compile-only until `Value.cpp` builds); system colours use a port of `RenderTheme`'s default table; `DynamicRangeLimit` is included.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/color/CSSColor.h`, `.cpp` | `have` | `CSS/Values/Color/Color.hpp`, `src/.../Color.cpp` | on | tested. All 43 alternatives (Krystal had 4). `UniqueRef<T>` → `UniquePtr<T>` (never null; Core has no `UniqueRef`). `absoluteColor()` → `GetAbsoluteColor()` (a member named `AbsoluteColor` hides the `AbsoluteColor<D>` template inside the class); `resolved()`/`keyword()`/`hex()` → `Resolved()`/`Keyword()`/`Hex()`. Value member is private `_value`. |
| `css/values/color/CSSColorDescriptors.h` | `have` | `CSS/Values/Color/ColorDescriptors.hpp` | on | tested through every descriptor's resolution. `CSSColorComponent`/`CSSColorFunctionSyntax`/`CSSColorFunctionForm` → `ColorComponent`/`ColorFunctionSyntax`/`ColorFunctionForm`. Descriptor traits are PascalCase (`Components`, `FlagsForAbsolute`, `SerialisationFunctionName`, …). |
| `css/values/color/CSSColorConversion+Normalize.h` | `have` | `CSS/Values/Color/Convert/Normalise.hpp` | on | |
| `css/values/color/CSSColorConversion+ToColor.h` | `have` | `CSS/Values/Color/Convert/ToColor.hpp` | on | The `ConvertToColor<D, Form>` struct is `ColorConverter<D, Form>::Convert` (it shared its name with the `ConvertToColor` functions once both were PascalCase). |
| `css/values/color/CSSColorConversion+ToTypedColor.h` | `have` | `CSS/Values/Color/Convert/ToTypedColor.hpp` | on | Was raw WebKit. |
| `css/values/color/CSSAbsoluteColor.h`, `CSSAbsoluteColorResolver.h` | `have` | `AbsoluteColor.hpp`, `AbsoluteColorResolver.hpp` | on | tested. The resolver was raw WebKit. |
| `css/values/color/CSSRelativeColor.h`, `CSSRelativeColorResolver.h` | `have` | `RelativeColor.hpp`, `RelativeColorResolver.hpp` | on | tested. Both were raw WebKit. The symbol table and symbol replacement shared by both resolve functions are helpers in `detail_relative_color`. |
| `css/values/color/CSSKeywordColor.h`, `.cpp` | `have` | `KeywordColor.hpp`, `src/.../KeywordColor.cpp` | on | tested. The `.cpp` was missing. `valueID` → `Value`. `nameLiteral` → `ToString(ValueId)`, `nameLiteralForSerialization` → `ToLower(ValueId)`. |
| `css/values/color/CSSHexColor.h`, `.cpp`, `CSSResolvedColor.h`, `.cpp` | `have` | `HexColor.hpp`, `ResolvedColor.hpp`, `src/...` | on | tested. The `.cpp`s were missing. `value` → `Value`. |
| `css/values/color/CSSColorType.h` | `have` | `ColorType.hpp` | on | tested. Flags enum (`None` added) instead of `OptionSet`. |
| `css/values/color/CSSPlatformColorResolutionState.h`, `.cpp` | `have` | `PlatformColorResolutionState.hpp`, `src/...` | on | tested. Fields PascalCase (`Delegate`, `NestingLevel`, `ConversionData`, `ForVisitedLink`, `KeywordOptions`, `Appearance`). Delegate defaults moved to the `.cpp` like upstream. |
| `css/values/color/CSSColorLayers.h`, `.cpp`, `CSSColorLayersResolver.h`, `.cpp` | `have` | `ColorLayers.hpp`, `ColorLayersResolver.hpp`, `src/...` | on | tested. `blendSourceOver` → `BlendSourceOver`. `toCSSValueID(BlendMode)` is a local `switch` in `ColorLayers.cpp` (FIXME: use `PrimitiveValueMappings.hpp` once reviewed). |
| `css/values/color/CSSColorMix.h`, `.cpp`, `CSSColorMixResolver.h`, `.cpp` | `have` | `ColorMix.hpp`, `ColorMixResolver.hpp`, `src/...` | on | tested. `Component::Percentage` (alias) → `PercentageType`: the member is `Percentage`. `mix` → `Mix`, `defaultInterpolationMethodForColorMix` → `DefaultInterpolationMethodForColorMix`. |
| `css/values/color/CSSContrastColor.h`, `.cpp`, `CSSContrastColorResolver.h`, `.cpp` | `have` | `ContrastColor.hpp`, `ContrastColorResolver.hpp`, `src/...` | on | tested. |
| `css/values/color/CSSLightDarkColor.h`, `.cpp` | `have` | `LightDarkColor.hpp`, `src/...` | on | tested. Upstream's unused `Document.h`/`StyleBuilderState.h` includes dropped. |
| `css/values/color/CSSDynamicRangeLimit.h`, `.cpp`, `CSSDynamicRangeLimitMix.h` | `have` | `DynamicRangeLimit.hpp`, `DynamicRangeLimitMix.hpp`, `src/...` | on | tested. Now a `CompactVariant` like upstream (was `Variant`). `DynamicRangeLimitValue.hpp` (the `Value` subclass) is not part of this layer. |
| `platform/ColorData.gperf`, `platform/HashTools.h` | `port` | `Graphics/Color/NamedColors.hpp`, `src/Graphics/Color/NamedColors.cpp` | Graphics | tested. `findColor` → `FindColor(u8string_view)`: a sorted `constexpr` table with binary search instead of gperf (same 152 entries, same exact lowercase matching). |
| `rendering/RenderTheme.cpp` (`systemColor`) | `port` | `Style/Values/Color/SystemColor.hpp`, `src/.../SystemColor.cpp` | on | tested. The base `RenderTheme::systemColor` as a free function `style::SystemColor`; no platform themes. |
| `style/values/color/StyleColorOptions.h` | `port` | `Style/Values/Color/ColorOptions.hpp` | on | `style::ColorOptions`, a flags enum. `operator<<` not ported. |
| `style/StyleForVisitedLink.h` | `have` | `Style/ForVisitedLink.hpp` | on | |
| `platform/graphics/GraphicsTypes.h` (`BlendMode` only) | `port` | `Graphics/GraphicsTypes.hpp` | on | |

**Bugs fixed:**
- `ColorType::Absolute` was `0`, so `IsColorKeyword(id, types)` never accepted absolute colour keywords; the other flags were off by one bit from upstream.
- `css::Color` had 4 of 43 alternatives, no copy/move/destructor, no `SwitchOn`, and its `AbsoluteColor()` member hid the `AbsoluteColor<D>` template.
- `AbsoluteColorResolver.hpp`, `RelativeColor.hpp`, `RelativeColorResolver.hpp`, `ColorMix.hpp`, `ColorMixResolver.hpp`, `LightDarkColor.hpp` and `Convert/ToTypedColor.hpp` were raw or half-converted WebKit (`WebCore::`, `WTF::switchOn`, `StringBuilder`).
- `ColorDescriptors.hpp`: `flagsForRelative` was lowercase (no descriptor had a `FlagsForRelative`, so relative colours couldn't resolve); the component results were `VariantOrSingle<brigand::list<…>>`, which with Core's pack-based `VariantOrSingle` made each component a `brigand::list` instead of a variant (now `brigand::wrap<List, VariantOrSingle>`, as in 3a).
- `Convert/ToColor.hpp`: the struct and function templates both named `ConvertToColor` (ill-formed); `Color` named `css::Color` instead of the platform colour; member names were lowercase (`saturation`, `whiteness`, …).
- `Convert/Normalise.hpp` called `NormalizeHue` from `ColorUtilities.hpp`; it's in `ColorNormalization.hpp` since layer 1.
- `DynamicRangeLimit::SwitchOn` had no parameter name for the visitors and passed the `UniquePtr`'s raw pointer instead of the function; the mix function's `UniquePtr` equality dereferenced null (moved-from) pointers.
- `PlatformColorResolutionStateNester` wasn't `explicit` and was copyable.
- Upstream bug fixed (behaviour differs): `containsCurrentColor(RelativeColor)` returned `containsColorSchemeDependentColor(origin)`, so `rgb(from currentcolor r g b)` didn't report containing `currentcolor` and `rgb(from Canvas r g b)` did.
- Upstream: `blendSourceOver(ColorLayersResolver)` dereferences an empty `optional` when there are no layers; now a debug assertion (the parser requires at least one).

**Differences:**
- `Color`'s indirect alternatives are `UniquePtr` (never null except after a move) instead of `UniqueRef`.
- Keyword and colour-type sets are flags enums (`ColorType`, `style::ColorOptions`) instead of `OptionSet`.
- `CurrentColor.hpp` (Krystal-only, not upstream) is still included by the unbuilt `Filter/FilterFunctionDescriptor.hpp`; it isn't built.

Tests: `tests/CSS/Values/Color/Color.cpp` (keyword classification and resolution, the delegate and its cache, every composite colour, absolute and relative resolution and serialisation, `Markable`, copies, dependencies, `DynamicRangeLimit`), `tests/Graphics/Color/NamedColors.cpp`, `tests/Style/Values/Color/SystemColor.cpp`: 13 cases / 199 assertions. Full Booey suite: 797 cases / 652583 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

### Layer 5b: colour parser (2026-10-12)

Graph from `css/parser/CSSPropertyParserConsumer+Color.cpp`, `+ColorAdjust.cpp` and `+ColorInterpolationMethod.cpp` (headers-only): 165 files (`tools/port/out/color-5b.md`). Almost all of it comes in through `CSSValuePool.h`, `CSSParser.h`, `CSSPropertyParsing.h` and `CSSPrimitiveValueMappings.h`, which only the Value-returning consumers (or nothing) use. What the parser actually calls: the 5a colour types, the meta consumer with the Number/Percentage/Angle/Keyword/Symbol definitions, `consumeFunction`/`consumeComma`/`consumeSlash`, `evaluateCalcIfNoConversionDataRequired`, `CSSParserFastPaths::parseHexColor`/`parseSimpleColor`, `CSSProperty::acceptsQuirkyColor`, four `CSSParserContext` flags and the tokenizer. Decisions (2026-10-12): the Value-returning consumers (`consumeColor`, `consumeDynamicRangeLimit`, `consumeColorScheme`) go in separate sources that stay out of CMake until 5c; the colour half of `CSSParserFastPaths.cpp` is ported now; `CSSColorScheme` comes with `+ColorAdjust`; the raw parse functions drop their `ScriptExecutionContext` parameter (Booey won't have one).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParserConsumer+Color.h`, `.cpp` | `have` | `Consumers/Color.hpp`, `src/.../Color.cpp` | on | tested. The `.cpp` was a stub; now ported. `CSSColorParsingOptions` → `ColorParsingOptions`, `allowedColorTypes` → `AllowedColorTypes`. The internal `ColorParserState`/nester and helpers are in an anonymous namespace; `normalize*` → `Normalise*`. `consumeColor` (Value) and `consumeDynamicRangeLimit` (Value) are in `src/.../ColorValueConsumers.cpp` (off). |
| `css/parser/CSSPropertyParserConsumer+ColorInlines.h` | `port` | `Consumers/ColorInlines.hpp` | on | tested. |
| `css/parser/CSSPropertyParserConsumer+ColorInterpolationMethod.h`, `.cpp` | `have` | `Consumers/ColorInterpolationMethod.hpp`, `src/.../ColorInterpolationMethod.cpp` | on | tested. The `.cpp` was missing. The hue method table is a `switch` (no `SortedArrayMap`, as in 3c). |
| `css/parser/CSSPropertyParserConsumer+ColorAdjust.h`, `.cpp` | `have` | `Consumers/ColorAdjust.hpp`, `src/.../ColorAdjust.cpp` | on | tested. `consumeColorScheme` (Value) is in `src/.../ColorAdjustValueConsumers.cpp` (off). No `DARK_MODE_CSS` guard. |
| `css/values/color-adjust/CSSColorScheme.h`, `.cpp` | `have` | `CSS/Values/ColorAdjust/ColorScheme.hpp`, `src/.../ColorScheme.cpp` | on | tested. The `.cpp` (serialiser) was missing. |
| `css/parser/CSSParserFastPaths.h`, `.cpp` (colour half, `lengthValueRangeForPropertiesSupportingSimpleLengths`) | `have` | `CSS/Parser/ParserFastPaths.hpp`, `src/CSS/Parser/ParserFastPaths.cpp` | on | tested. The `.cpp` was missing. UTF-8 only, so no 8-/16-bit split; the WTF span helpers (`skipWhile`, `skipExactly`, …) are local helpers on `CSSOMStringView`. `charactersToDouble` → `std::from_chars` after skipping leading whitespace (same grammar as WTF's fast_float: no leading `+`, accepts `inf`/`nan`). `maybeParseValue` and the length/transform/display/opacity fast paths wait for the Values slice (`MaybeParseValue` is declared, not defined; TODO). |
| `css/parser/CSSPropertyParserConsumer+SymbolDefinitions.h`, `+Symbol.cpp` | `port` | `Consumers/SymbolDefinitions.hpp`, `src/.../Symbol.cpp` | on | tested. `validatedRange` → `ValidatedRange`. |
| `css/parser/CSSPropertyParserConsumer+AngleDefinitions.h` | `have` | `Consumers/AngleDefinitions.hpp` | on | tested. See bugs. |
| `css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h`, `+KeywordDefinitions.h` | `have` | `Consumers/PercentageDefinitions.hpp`, `KeywordDefinitions.hpp` | on | tested through the colour parser. Match upstream. |
| `css/parser/CSSParserContext.cpp` (`strictCSSParserContext`) | `port` | `StrictParserContext()` in `CSS/Parser/ParserContext.hpp` | on | A function-local static instead of `MainThreadNeverDestroyed`. |
| `css/CSSProperty.h` (`acceptsQuirkyColor`, `acceptsQuirkyAngle`) | `have?` | `CSS/Properties/Property.hpp` | header (via codegen) | Used, not reviewed; see "Where we are". |
| `css/CSSValuePool.h`, `CSSColorValue.h`, `CSSPrimitiveValue.h`, `CSSDynamicRangeLimitValue.h`, `CSSColorSchemeValue.h` | | | off | Only the Value-returning consumers use them; layer 5c. |
| `wtf/SortedArrayMap.h`, `wtf/text/MakeString.h` | `wont` | | | `switch`; `std::to_string` + `append`. |

**Bugs fixed:**
- `Color.hpp`: `ConsumeColorRaw` returned `Color`, which inside `css` is `css::Color`, not the platform colour; `DynamicRangeLimit` was forward-declared as a `class` (it's a `struct`; C4099 is an error under `/WX`); `ParseColorRaw`/`ParseColorRawSimple`/`ParseColorRawGeneral` were missing.
- `ColorInterpolationMethod.hpp` forward-declared `css::ColorInterpolationMethod`; the type is `boo::ColorInterpolationMethod`, so the declaration named a type that doesn't exist.
- `ColorAdjust`: the functions weren't in `PropertyParserHelpers`; the designated initialisers and member accesses used `schemes`/`only` (the members are `Schemes`/`Only`); `ParseUnresolvedColorScheme` never pumped the tokenizer, so it always parsed an empty range and returned null; it included `ColorSchemeValue.hpp` (layer 5c).
- `ColorScheme` had no serialiser.
- `AngleValidator::ShouldAcceptUnitlessValue` was missing the quirks-mode case, so `glyph-orientation-horizontal`/`-vertical` rejected unitless angles in quirks mode.
- `ParserFastPaths::MaybeParseValue` returned `Maybe<Value>` (`Value` is ref-counted) instead of `RefPtr<Value>`; no function in the header was defined.
- There was no `Symbol` consumer definition, so relative colour channel keywords (`r`, `h`, `alpha`, …) couldn't be consumed.
- Tokenizer slice: `InputStream` rejected empty input (its constructor asserted non-empty data, and Core's `Decode` reports `EmptyInput`), so every string entry point asserted on `""`. An empty stream is valid (it only produces EOF), as upstream; `FilterCodePoints` now returns early for empty input. Test: `Tokenizer(empty input)` in `tests/CSS/Parser/Tokenizer.cpp`.

**Added (upstream API):** `ParseColorRaw`, `ParseColorRawSimple`, `ParseColorRawGeneral`, `StrictParserContext`, `ParserFastPaths::ParseNamedColor`/`LengthValueRangeForPropertiesSupportingSimpleLengths`, `Serialiser<ColorScheme>`, `ValidatedRange(SymbolRaw)`.

**Differences:**
- The Value-returning consumers live in `ColorValueConsumers.cpp`/`ColorAdjustValueConsumers.cpp` (not built) and are written against upstream's API (`ValuePool::CreateColorValue`, `DynamicRangeLimitValue::Create`), which 5c has to add. `ConsumeColor` (Value) calls the public `ConsumeUnresolvedColor` instead of the internal overload; the behaviour is the same.
- `ParseColorRaw`/`ParseColorRawGeneral` take no `ScriptExecutionContext`; the state's pool is the default.
- `IsColorKeywordAllowed` has no Apple system colours (no platform-specific keywords).

Tests: `tests/CSS/Properties/Consumers/Color.cpp` (keywords, allowed types, mode-specific keywords, hex and quirky colours, modern and legacy `rgb()`/`hsl()`, eager and preserved calc, `lab()`/`lch()`/`oklab()`/`oklch()`/`hwb()`, every `color()` space, relative colours and symbols, `color-mix()` and `<color-interpolation-method>`, `light-dark()`, the setting-gated functions, the raw entry points, `<'dynamic-range-limit'>`), `tests/CSS/Properties/Consumers/ColorAdjust.cpp`, `tests/CSS/Parser/ParserFastPaths.cpp`, two cases in `tests/CSS/Properties/Consumers/MetaConsumer.cpp` (unitless quirky angles, symbols): 15 cases / 303 assertions (with the empty-input tokenizer case). Full Booey suite: 812 cases / 652886 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

### Layer 5c: colour values and the value pool (2026-10-12)

Graph from `css/CSSValuePool.cpp`, `CSSColorValue.cpp`, `CSSDynamicRangeLimitValue.cpp` and `CSSColorSchemeValue.cpp` (headers-only): 128 files (`tools/port/out/color-5c.md`). Almost all of it comes in through `CSSPrimitiveValueMappings.h`, `CSSPropertyParser.h` and `CSSValueList.h`, which only `CSSValuePool::createFontFaceValue` uses; the rest is layers 2–5a. Compile-only: the sources build into the library, but nothing can call them from a test until `Value.cpp`/`PrimitiveValue.cpp` build (the static pool constructs `PrimitiveValue`s). Since Booey is a static library, unreferenced objects aren't linked, so the colour parser tests still link.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSValuePool.h`, `.cpp` | `have` | `CSS/Values/ValuePool.hpp`, `src/CSS/Values/ValuePool.cpp` | on (compile-only) | `StaticCSSValuePool` → `StaticValuePool`, `staticCSSValuePool` → `CommonValuePool`, `singleton()` → `MainThreadPool()` (per upstream's FIXME). `createFontFaceValue` not ported (TODO: needs `ValueList` and `PropertyParser::ParseStylePropertyLonghand`). |
| `css/CSSColorValue.h`, `.cpp` | `have` | `CSS/Values/ColorValue.hpp`, `src/CSS/Values/ColorValue.cpp` | on (compile-only) | `color()` → `GetColor()`, `absoluteColor()` → `GetAbsoluteColor()` (members named after types would hide `css::Color`/`AbsoluteColor<D>` in the class; same as `css::Color::GetAbsoluteColor`). |
| `css/CSSDynamicRangeLimitValue.h`, `.cpp` | `have` | `CSS/Values/DynamicRangeLimitValue.hpp`, `src/.../DynamicRangeLimitValue.cpp` | on (compile-only) | `dynamicRangeLimit()` → `GetDynamicRangeLimit()`. |
| `css/CSSColorSchemeValue.h`, `.cpp` | `have` | `CSS/Values/ColorSchemeValue.hpp`, `src/.../ColorSchemeValue.cpp` | on (compile-only) | `colorScheme()` → `GetColorScheme()`. No `DARK_MODE_CSS` guard. |
| `css/parser/CSSPropertyParserConsumer+Color.cpp`, `+ColorAdjust.cpp` (Value-returning consumers) | `have` | `src/.../ColorValueConsumers.cpp`, `ColorAdjustValueConsumers.cpp` | on (compile-only) | Written in 5b. Stay separate from `Color.cpp`/`ColorAdjust.cpp` until `Value.cpp` builds, or the colour parser tests would pull in the pool. |
| `css/parser/CSSPropertyParserConsumer+Ident.cpp` (Value-returning consumers) | `have` | `src/CSS/Properties/Consumers/Ident.cpp` | on (compile-only) | Written in 3c. |
| `css/CSSPrimitiveValueMappings.h`, `CSSValueList.h`, `parser/CSSPropertyParser.h` | | | off | Only `createFontFaceValue`. |

**Bugs fixed:**
- `ColorValue::CustomVisitChildren` returned `Continue` without visiting the colour, so values nested in a colour (e.g. `calc()` in `rgb()`) were skipped by `CollectComputedStyleDependencies` and other child walks.
- `ColorValue::AbsoluteColor` called `css::Color::AbsoluteColor()`, renamed `GetAbsoluteColor()` in 5a; `ColorValue.cpp` was missing the `KeywordColor.hpp` include for `IsAbsoluteColorKeyword`/`ColorFromAbsoluteKeyword`.
- `ColorValue`'s single-argument constructors weren't `explicit`.
- `DynamicRangeLimitValue` wasn't `final`.
- `ColorSchemeValue` wasn't `final`, its constructor wasn't `explicit`, and its `.cpp` had no BOM.
- `PrimitiveValue::Create(ValueId)` only debug-asserted the keyword range; upstream release-asserts (`krys_assert`).

**Added (upstream API):** `ValuePool::CreateColorValue`, `CreateFontFamilyValue`, `Drain`, the colour and font-family caches (with upstream's random eviction at 512/128 entries); `DynamicRangeLimitValue::Create`/`GetDynamicRangeLimit`; `ColorSchemeValue::GetColorScheme`.

**Differences:**
- The caches are `HashMap` (`std::unordered_map`): `boo::Color` is hashed through a private `ColorHash` adaptor over `DefaultHash<boo::Color>`; random eviction picks an index with `krys::Random` and walks to it (upstream: `HashMap::random()`).
- Static values (`StaticValuePool`'s colours and primitives) still use the extra-`AddRef` workaround (TODO in the constructors).
- `PropertyParserState::Pool` stays a pointer: defaulting it to `MainThreadPool()` would link `ValuePool.cpp` (and with it `PrimitiveValue`'s constructors) into every parser test. It becomes a reference when `PrimitiveValue.cpp` builds.

Tests: none (nothing can be instantiated until `Value.cpp` builds). Full Booey suite: 812 cases / 652886 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

### Layer 6: Style colour (2026-10-12)

Graph from every `style/values/color` file except `StyleOpacity` (headers-only): 176 files (`tools/port/out/color-6.md`). Almost all of it comes in through `StyleBuilderState.h` (cascade, rule sets, selectors, DOM), `Document.h`/`RenderStyle.h`/`RenderTheme.h` (in `StyleColor.cpp`, `StyleKeywordColor.cpp`, `StyleLightDarkColor.cpp`) and `CSSPrimitiveValueMappings.h`/`CSSValuePool.h`/`CSSColorValue.h`, which only the CSSValue half uses. What the code calls: the 5a colour types and resolvers, the Style numerics, `Document::textColor`/`linkColor`/`visitedLinkColor`/`activeLinkColor`/`styleColorOptions`/`useDarkAppearance`, `RenderStyle::colorResolvingCurrentColor`, `RenderTheme::focusRingColor` and four `BuilderState` accessors. Decisions (2026-10-12): (1) a testable seam: `RenderStyle` becomes constructible with only its colour state, and the `Document` colour API becomes seam free functions with upstream's defaults for a document with no page and default settings; (2) the CSSValue half is deferred; (3) `PlatformDynamicRangeLimit` and the `Opacity` struct are included.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/values/color/StyleColor.h`, `.cpp` | `port` | `Style/Values/Color/Color.hpp`, `src/.../Color.cpp` | on | tested. `UniqueRef<T>` → `UniquePtr<T>` (never null except after a move). `currentColor()` → `GetCurrentColor()`, `resolvedColor()` → `GetResolvedColor()` (the plain names would hide `CurrentColor`/`ResolvedColor` in the class). `toStyleColor`/`resolveColor`/`containsCurrentColor`/`serializationForCSSTokenization` → `ToStyleColor`/`ResolveColor`/`ContainsCurrentColor`/`SerialisationForCSSTokenization`. `debugDescription`/`operator<<` not ported. CSSValue half deferred. |
| `style/values/color/StyleCurrentColor.h`, `.cpp` | `port` | `CurrentColor.hpp`, `src/.../CurrentColor.cpp` | on | tested. |
| `style/values/color/StyleResolvedColor.h`, `.cpp` | `port` | `ResolvedColor.hpp`, `src/.../ResolvedColor.cpp` | on | tested. `color` → `Value` (as `css::ResolvedColor`). |
| `style/values/color/StyleColorResolutionState.h` | `port` | `ColorResolutionState.hpp` | on | tested. Fields PascalCase (`Document`, `Style`, `ConversionData`, `ForVisitedLink`, `NestingLevel`). The nester is `explicit` and non-copyable. |
| `style/values/color/StyleAbsoluteColor.h` | `port` | `AbsoluteColor.hpp` | on | tested. |
| `style/values/color/StyleHexColor.h`, `.cpp`, `StyleKeywordColor.h`, `.cpp`, `StyleLightDarkColor.h`, `.cpp` | `port` | `HexColor.hpp`, `KeywordColor.hpp`, `LightDarkColor.hpp`, `src/...` | on | tested. Conversion functions only (no Style types), as upstream. |
| `style/values/color/StyleColorMix.h`, `.cpp` | `port` | `ColorMix.hpp`, `src/.../ColorMix.cpp` | on | tested. `Component::Percentage` (alias) → `PercentageType`, as on the CSS side. |
| `style/values/color/StyleContrastColor.h`, `.cpp` | `port` | `ContrastColor.hpp`, `src/.../ContrastColor.cpp` | on | tested. |
| `style/values/color/StyleColorLayers.h`, `.cpp` | `port` | `ColorLayers.hpp`, `src/.../ColorLayers.cpp` | on | tested. See bugs. |
| `style/values/color/StyleRelativeColor.h` | `port` | `RelativeColor.hpp` | on | tested. |
| `style/values/color/StyleDynamicRangeLimit.h`, `.cpp` | `port` | `DynamicRangeLimit.hpp`, `src/.../DynamicRangeLimit.cpp` | on | tested. `toPlatformDynamicRangeLimit` → `ToPlatformDynamicRangeLimit`. CSSValue half deferred. |
| `style/values/color/StyleDynamicRangeLimitMix.h`, `.cpp` | `port` | `DynamicRangeLimitMix.hpp`, `src/.../DynamicRangeLimitMix.cpp` | on | tested. `addWeightedLimitTo` → `AddWeightedLimitTo`; parameters `Standard`/`Constrained`/`NoLimit`. Logging not ported. |
| `style/values/color/StyleOpacity.h`, `.cpp` | `port` (partial) | `Opacity.hpp` | on | tested. The struct only: `CSSValueConversion<Opacity>` (the whole `.cpp`) is deferred; `Evaluation<Opacity, AcceleratedEffectOpacity>` (threaded animations) not ported. |
| `style/values/color/StyleColorOptions.h`, `.cpp` | | | | Done in 5a; the `.cpp` is only `operator<<`: not ported. |
| `platform/graphics/PlatformDynamicRangeLimit.h`, `.cpp` | `port` | `Graphics/PlatformDynamicRangeLimit.hpp`, `src/Graphics/PlatformDynamicRangeLimit.cpp` | on | tested. Statics PascalCase (`Standard()`, `NoLimit()`, `InitialValue()`, …); `value()` → `Value()`. No IPC or `operator<<`. |
| `rendering/RenderTheme.cpp` (`focusRingColor`) | `port` | `style::FocusRingColor` in `Style/Values/Color/SystemColor.hpp` | on | tested. The base theme's black (`platformFocusRingColor`); no per-options cache. |
| `css/CSSPrimitiveValueMappings.h` (`toCSSValueID(BlendMode)`) | `have*` | `css::ToValueId(BlendMode)` in `CSS/Values/Color/ColorLayers.hpp` | on | Was private to `ColorLayers.cpp` (5a); exposed for the Style serialiser. FIXME to use `PrimitiveValueMappings.hpp` once reviewed. |
| `Document`, `RenderStyle`, `BuilderState` colour members | seam | `Style/Seam.hpp`, `src/Style/Seam.cpp` | on | See below. |

**Seam changes.** `RenderStyle` can now be constructed: it holds `color`, `visitedLinkColor` (both initially black, upstream's initial values) and a dark-only colour-scheme flag, with `GetColor`/`SetColor`, `VisitedLinkColor`/`SetVisitedLinkColor` and `ColorResolvingCurrentColor(const style::Color &, bool visitedLink)` (upstream's `Style::Color` overload). Its other members are still unreachable. Added `BuilderState::Style()`. Collapsed members:

| Seam member | Upstream |
|---|---|
| `RenderStyle::HasDarkOnlyColorScheme()` | `style.colorScheme()` resolving to `dark` without `light` (used by `Document::useDarkAppearance`) |
| `DocumentTextColor(document)` | `document.textColor()`; nothing sets it (`<body text>`), so black |
| `DocumentLinkColor`/`DocumentVisitedLinkColor`/`DocumentActiveLinkColor(document, style)` | `document.linkColor(style)` etc.; nothing sets them (`<body link>`), so the keyword colours |
| `DocumentUseDarkAppearance(document, style)` | `document.useDarkAppearance(style)` with no page and default settings: dark only for a dark-only colour scheme |
| `DocumentStyleColorOptions(document, style)` | `document.styleColorOptions(style)` with no page and default settings: `UseDarkAppearance` or nothing |

**Bugs fixed:**
- Upstream bug fixed (behaviour differs): `toStyleColor(CSS::ColorLayers)` checked `any_of(isResolvedColor)` instead of "any not resolved". A `color-layers()` with a resolved layer was kept unresolved until use time, and one with no resolved layer (e.g. all `currentcolor`) called `resolvedColor()` on unresolved colours (an assertion, or a bad variant access in release). Now `!all_of(isResolvedColor)`.

**Added:** `style::FocusRingColor`; `css::ToValueId(BlendMode)` made public; `ToStyle<css::DynamicRangeLimit>` and `ToStyle<css::DynamicRangeLimitMixFunction>` take a `ToLengthConversionData` as well as a `BuilderState`. The `BuilderState` overloads forward to them with `CssToLengthConversionData()`, which is all upstream's percentage conversion uses, so tests can reach the mix normalisation.

**Differences:**
- `style::Color`'s constructors from a platform colour, `SRGBA<uint8>`, the `currentcolor`/`transparent`/`black`/`white` keywords, `ResolvedColor` and `CurrentColor` are implicit, like upstream (callers rely on `return {color};`); the indirect kinds are `explicit`, like `css::Color`.
- `style::Color` equality compares the pointees of the indirect kinds in `operator==` (like `css::Color`) instead of per-kind `operator==(UniqueRef, UniqueRef)` overloads. `DynamicRangeLimit` keeps the overload, because `CompactVariant`'s defaulted `==` needs it.
- Upstream behaviour kept: Style `color-mix()` serialisation omits the second percentage unless both are given (the CSS side also serialises a lone calc second percentage); relative colour components are simplified during conversion, so `calc(alpha / 2)` serialises as `calc(0.5 * alpha)`.
- The parser doesn't take a `color-layers()` blend mode yet (upstream FIXME), so the multiply serialisation test builds the Style value directly.

Tests: `tests/Style/Values/Color/{Color,DynamicRangeLimit}.cpp` (construction, `Markable`, conversion of every colour kind including keywords, document colours, system colours and `light-dark()` under the dark colour scheme, every currentcolor-dependent kind and its use-time resolution and serialisation, copies, computed serialisation, `ToCSS`, `RenderStyle`, blending; dynamic-range-limit keywords, mixes, normalisation, flattening, copies, blending, the platform value; `Opacity`): 13 cases / 303 assertions. Full Booey suite: 825 cases / 653189 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

## Slice 4: Value subclasses

Every `Value` subclass, so `Value.cpp`/`PrimitiveValue.cpp` can build (`VisitDerived`). Decision (2026-10-12): the generic containers first (`ValuePair`, `ValueList`/`ValueContainingVector`, `FunctionValue`, `QuadValue`, `RectValue`), then the specific value types, one type per step (graph, port, build, test). Compile-only until `Value.cpp` builds: nothing can be created in a test, because `Ref` releases through `Value`'s `operator delete` and `CssText`/`Equals` dispatch through `Value.cpp`.

### Order (decided 2026-10-12)

Remaining subclasses, loosely by complexity. Measured from each upstream `.cpp`'s headers-only graph: "new" = graph files the ledger doesn't cover yet. `DeprecatedCSSOM*` is won't-port (no `createDeprecatedCSSOMWrapper`), so it's ignored. Every Krystal header exists except `CanvasValue.hpp`; only `AppleColorFilter`, `Attr`, `BackgroundRepeat`, `BasicShape`, `BorderImageSlice`/`Width` and `BoxShadowProperty` have a `.cpp`.

**Tier 1: self-contained** (only the class's own files are new).
1. `BackgroundRepeatValue`
2. `BorderImageSliceValue`, 3. `BorderImageWidthValue` (on `Quad`)
4. `ReflectValue`
5. `UnicodeRangeValue`
6. `FontVariationValue`, 7. `FontFeatureValue`, 8. `FontStyleWithAngleValue`, 9. `FontStyleRangeValue`, 10. `FontValue` (needs 8)
11. `GridLineNamesValue`, 12. `GridLineValue`
13. `SubgridValue`, 14. `TransformListValue`, 15. `GridAutoRepeatValue`, 16. `GridIntegerRepeatValue` (on `ValueContainingList`)
17. `CounterValue`
18. `OffsetRotateValue`
19. `AttrValue`

**Tier 2: one or two new CSS types.**
20. `PositionValue`, `PositionXValue`, `PositionYValue` (`CSSPosition.h`)
21. `UrlValue` (`CSSURL.h`, `CSSURLModifiers.h`, and WTF's URL)
22. `GridTemplateAreasValue` (`CSSGridNamedAreaMap.h`, `CSSGridTemplateAreas.h`)
23. `EasingFunctionValue` (the four easing functions; `Values/Easing/` exists)

**Tier 3: CSS type families.**
24. `BoxShadowPropertyValue`, 25. `TextShadowPropertyValue` (`CSSBoxShadow`/`CSSTextShadow`)
26. `PathValue` (`CSSFillRule`, `CSSPathFunction`; needs `SVGPathByteStream`)
27. `BasicShapeValue` (shape functions, `CSSBorderRadius`; needs 20, 26)
28. `FilterValue`, 29. `AppleColorFilterValue` (filter functions; `FilterValue` needs 21)
30. `RayValue`, 31. `ScrollValue`, 32. `ViewValue`: only new through `CSSPrimitiveValueMappings.h`, which reaches `StyleBuilderState`. Port just the enum mappings they use.

**Tier 4: need the parser, Style builder or resource loading.**
33. `CustomPropertyValue`, 34. `VariableReferenceValue`, 35. `PendingSubstitutionValue` (`CSSVariableData`, `CSSPropertyParser`, `StyleBuilder`, custom property registry)
36. `FontFaceSrcLocalValue`/`FontFaceSrcResourceValue` (`CachedFont`, resource loader)
37. Images, which create `StyleImage`s through `BuilderState`: `ImageValue`, `ImageSetOptionValue`, `ImageSetValue`, `NamedImageValue`, `CanvasValue` (new file), `PaintImageValue`, `CrossFadeValue`, `FilterImageValue` (needs 28), `CursorImageValue`, `GradientValue` (largest)

Tier 4 may be stubbed (declarations plus minimal definitions) so `Value.cpp`/`PrimitiveValue.cpp` can build before its dependencies are ported; decide when tiers 1–3 are done.

### `ValuePair` (2026-10-12)

Graph from `css/CSSValuePair.cpp` (headers-only): 3 files (`CSSValuePair.cpp`, `CSSValuePair.h`, `CSSValue.h`), plus WTF boundaries (`tools/port/out/value-pair.md`). The `.cpp` only adds `Hasher.h`/`MakeString.h`/`WTFString.h`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSValuePair.h`, `.cpp` | `have` | `CSS/Values/ValuePair.hpp`, `src/CSS/Values/ValuePair.cpp` | on (compile-only) | `createNoncoalescing` → `CreateNonCoalescing` (existing callers). Defines `Value::First()`/`Second()`. |

**Bugs fixed:**
- No `const` `First()`/`Second()`.
- `Create*` took `Ref<Value> &&`; now by value like upstream, so callers can pass a `Ref<Derived>` lvalue copy.

**Krystal.Core bugs fixed (found here):**
- `Hash::Combine(seed, x)` with an lvalue `size_t seed` (the `AddDerivedHash` pattern) didn't compile: it picked the private `Combine(size_t &, ...)` helper. Latent in `Value.cpp` and `PrimitiveValue.cpp`, which aren't built. The helper is renamed `CombineInto`. Tested in `Krystal.Core.Tests/tests/Hash.cpp`.

**Added (upstream API):** `CanBeCoalesced`, `AddDerivedHash` (separator, coalesce flag, then both values; false if either value hashes by pointer), the `IdenticalValueSerialisation` constructor.

**Differences:** none beyond the translation rules. `friend class Value` (for `AddDerivedHash` via `VisitDerived`) instead of upstream's `friend CSSValue::addHash`, like `PrimitiveValue`.

Tests: none for `ValuePair` (compile-only). `Krystal.Core.Tests/tests/Hash.cpp`: 2 cases / 6 assertions. Full Booey suite: 825 cases / 653189 assertions; all pass except 2 expected failures. Core suite: 544 cases / 2214 assertions, all pass. Whole solution builds with no warnings.

### `ValueList` (2026-10-12)

Graph from `css/CSSValueList.cpp` (headers-only): 22 files (`tools/port/out/value-list.md`). Everything past `CSSValueList.h` comes in through `CSSPrimitiveValue.h` (the `.cpp` uses `isValueID`), all reviewed in slices 2–3.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSValueList.h`, `.cpp` (`CSSValueContainingVector`) | `have` | `ValueContainingList` in `CSS/Values/ValueList.hpp`, `src/CSS/Values/ValueList.cpp` | on (compile-only) | Renamed from `ValueWithSmallList` (decided 2026-10-12; `FunctionValue`, `SubgridValue`, `TransformListValue`, `Value.cpp`, `PropertyParser.cpp` updated). Container protocol keeps upstream's names (`size`, `operator[]`, `begin`/`end`); the rest PascalCase (`Length`, `Item`, `ItemWithoutBoundsCheck`, `ProtectedItem`, `HasValue`, `SerialiseItems`, `ItemsEqual`, `ContainsSingleEqualItem`, `CopyValues`). |
| `css/CSSValueList.h`, `.cpp` (`CSSValueList`) | `have` | `ValueList` in the same files | on (compile-only) | `create(char16_t, ...)` takes a `char32`. |
| `CSSValueListBuilder` (in `CSSValueList.h`) | `have` | `CSS/Values/ValueListBuilder.hpp` | on | `SmallList<Ref<Value>, 4>`. |

**Bugs fixed:**
- No `AddDerivedHash`, so lists hashed by pointer.
- `Get` used raw `assert`; `CustomTraverseSubresources` wasn't `noexcept`; `ValueList(ValueSeparator)` wasn't `noexcept`.
- `Create*` took `ValueListBuilder &&`; now by value like upstream.

**Krystal callers updated:** `Consumers/CounterStyles.cpp` (`->Size()` → `->size()`), `Consumers/Grid.cpp` (`function.Get(i)` → `function[i]`). Both unreviewed and not built.

**Added (upstream API):** `CreateCommaSeparated(Ref<Value>)`, `CreateSpaceSeparated()`, `CreateSlashSeparated(Ref<Value>)`, `Length`, `Item`, `ItemWithoutBoundsCheck`, `ProtectedItem`, `SeparatorCSSText` (public), `AddDerivedHash`.

**Differences:**
- Items are held in a `SmallList<const Value *, 4>` (leaked references, released in the destructor) instead of an inline `std::array` plus a `MallocSpan`. Same ownership and inline capacity.
- Removed `Get`/`TryGet`/`Size` (use `operator[]`/`Item`/`size`).
- No free `add(Hasher&, const CSSValueContainingVector&)` (declared upstream, never defined); see `Value`.

Tests: none (compile-only). Full Booey suite: 825 cases / 653189 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### `FunctionValue` (2026-10-12)

Graph from `css/CSSFunctionValue.cpp` (headers-only): 4 files (`CSSFunctionValue.cpp`, `.h`, `CSSValueList.h`, `CSSValue.h`; `tools/port/out/function-value.md`). The `.cpp` also includes `CSSValueKeywords.h` (generated `ValueId.hpp`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSFunctionValue.h`, `.cpp` | `have` | `CSS/Values/FunctionValue.hpp`, `src/CSS/Values/FunctionValue.cpp` | on (compile-only) | `nameLiteral(m_name)` → `ToString(_name)` (keeps the keyword's case, e.g. `translateX`). |
| `CSSValueKeywords.h`, `CSSPropertyNames.h` (`DefaultHash`) | `have` | `codegen/css_value_keywords.py`, `codegen/css_properties.py` | | See bugs. |

**Bugs fixed:**
- It declared its own `CustomVisitChildren` and `Length` (never defined), hiding `ValueContainingList`'s.
- Generated code: `DefaultHash<ValueId>` and `DefaultHash<PropertyId>` derived from `IntegerHash<uint16>`, whose `Hash(uint16)` doesn't accept the enum, so hashing either (`Hash::Combine(seed, valueId)`, also in the unbuilt `PrimitiveValue::AddDerivedHash`) failed to compile. They now define `Hash`/`Equal` on the enum, forwarding to `IntegerHash<uint16>`. Tested in `tests/CSS/Parser/KeywordLookup.cpp`.

**Added (upstream API):** `AddDerivedHash` (name, then the list's hash).

**Differences:** none beyond the translation rules.

Tests: none for `FunctionValue` (compile-only); `KeywordLookup.cpp` gained a hash case (6 assertions). Full Booey suite: 826 cases / 653195 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### `QuadValue` (2026-10-12)

Graph from `css/CSSQuadValue.cpp` (headers-only): 5 files (`CSSQuadValue.cpp`, `.h`, `Quad.h`, `RectBase.h`, `CSSValue.h`; `tools/port/out/quad-value.md`). The `.cpp` adds nothing.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSQuadValue.h`, `.cpp` | `have` | `CSS/Values/QuadValue.hpp`, `src/CSS/Values/QuadValue.cpp` | on (compile-only) | `quad()` → `GetQuad()`. Defines `Value::GetQuad()`. |
| `css/Quad.h` | `have` | `CSS/Values/Quad.hpp` | on | Moved from `Primitives/` to match upstream's location (`Rect.hpp`/`RectBase.hpp` too). |
| `css/RectBase.h` | `have` | `CSS/Values/RectBase.hpp` | on | Shared with `Rect` (reviewed with `RectValue`). |

**Bugs fixed:**
- It declared its own `CustomVisitChildren` (never defined; upstream has none), hiding `Value`'s default.

**Added (upstream API):** `Create(Quad)`, `GetQuad` (and `Value::GetQuad()`), `CanBeCoalesced`.

**Differences:**
- `Quad(Ref<Value>)` is `explicit` (upstream's is implicit, but nothing upstream converts implicitly; `create({ ... })` uses the four-argument constructor).
- `RectBase::Equals` uses `CompareValue`, like upstream's `compareCSSValue` (was `operator==`, same behaviour).
- No `AddDerivedHash`, like upstream: quads hash by pointer.

Tests: `tests/CSS/Values/Quad.cpp` covers `Quad::Serialise` (1 case / 6 assertions); `QuadValue` itself is compile-only. Full Booey suite: 827 cases / 653201 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### `RectValue` (2026-10-12)

Graph from `css/CSSRectValue.cpp` (headers-only): 5 files (`CSSRectValue.cpp`, `.h`, `Rect.h`, `RectBase.h`, `CSSValue.h`; `tools/port/out/rect-value.md`). The `.cpp` adds nothing.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSRectValue.h`, `.cpp` | `have` | `CSS/Values/RectValue.hpp`, `src/CSS/Values/RectValue.cpp` | on (compile-only) | `rect()` → `GetRect()`. Defines `Value::GetRect()`. |
| `css/Rect.h` | `have` | `CSS/Values/Rect.hpp` | on | `generateCSSString` → `GenerateCSSString`. |

**Bugs fixed:**
- It declared its own `CustomVisitChildren` (never defined; upstream has none), hiding `Value`'s default.
- `RectValue` wasn't `final`.

**Added (upstream API):** `GetRect` (and `Value::GetRect()`).

**Differences:**
- `Rect`'s serialiser was public (`Serialise`); now private `GenerateCSSString` like upstream (nothing else called it).
- No `AddDerivedHash`, like upstream: rects hash by pointer.

Tests: none (compile-only; `Rect`'s serialiser is private, and `CssText` dispatches through `Value.cpp`). Full Booey suite: 827 cases / 653201 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### `RatioValue` (2026-10-12)

Graph from `css/CSSRatioValue.cpp` (headers-only): 23 files (`tools/port/out/ratio-value.md`). Only `CSSRatioValue.h`/`.cpp` and `CSSRatio.h` are new; the rest (numeric primitives and the `+CSSValueVisitation`/`+ComputedStyleDependencies`/`+Serialization` headers) was reviewed in slices 2–3. `CSSRatio.cpp` (not in the headers-only graph) defines `CSSValueCreation<Ratio>`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSRatioValue.h`, `.cpp` | `have` | `CSS/Values/RatioValue.hpp`, `src/CSS/Values/RatioValue.cpp` | on (compile-only) | `ratio()` → `GetRatio()`. |
| `css/values/primitives/CSSRatio.h`, `.cpp` | `have` | `CSS/Values/Primitives/Ratio.hpp`, `src/CSS/Values/Primitives/Ratio.cpp` | on | tested. `numerator`/`denominator` → `Numerator`/`Denominator`. |

**Bugs fixed:**
- `RatioValue` wasn't `final`.

**Added (upstream API):** `GetRatio`.

**Differences:**
- One `Create(Ratio)` by value instead of upstream's `Ratio&&`/`const Ratio&` overloads (like `DynamicRangeLimitValue`).
- No `AddDerivedHash`, like upstream: ratios hash by pointer.

Tests: `tests/CSS/Values/Primitives/Ratio.cpp` (default denominator, `get`, equality, serialisation including `2 / 1` for an omitted denominator, visitation): 4 cases / 10 assertions. `RatioValue` and `ValueCreation<Ratio>` are compile-only. Full Booey suite: 831 cases / 653211 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### `BackgroundRepeatValue` (2026-10-12)

Graph from `css/CSSBackgroundRepeatValue.cpp` (headers-only): 3 files (`CSSBackgroundRepeatValue.cpp`, `.h`, `CSSValue.h`; `tools/port/out/background-repeat-value.md`). The `.cpp` also includes `CSSValueKeywords.h` (generated `ValueId.hpp`). First of tier 1.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSBackgroundRepeatValue.h`, `.cpp` | `have` | `CSS/Values/BackgroundRepeatValue.hpp`, `src/CSS/Values/BackgroundRepeatValue.cpp` | on (compile-only) | `xValue()`/`yValue()` → `XValue()`/`YValue()`. |

**Bugs fixed:**
- The accessors were `x()`/`y()` (lowercase, not upstream's names); now `XValue()`/`YValue()`. No Krystal callers.
- The `.cpp` had no BOM; the header included `Quad.hpp` for nothing.

**Added (upstream API):** none; the existing port was complete.

**Differences:** none beyond the translation rules. No `AddDerivedHash`, like upstream: hashes by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 831 cases / 653211 assertions; all pass except 2 expected failures.

### `BorderImageSliceValue`, `BorderImageWidthValue` (2026-10-12)

Graphs from `css/CSSBorderImageSliceValue.cpp` and `css/CSSBorderImageWidthValue.cpp` (headers-only): 5 files each (the class's `.h`/`.cpp`, `Quad.h`, `RectBase.h`, `CSSValue.h`; `tools/port/out/border-image-slice-value.md`, `border-image-width-value.md`). `Quad`/`RectBase` were reviewed with `QuadValue`; the `.cpp`s add only WTF string headers.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSBorderImageSliceValue.h`, `.cpp` | `have` | `CSS/Values/BorderImageSliceValue.hpp`, `src/CSS/Values/BorderImageSliceValue.cpp` | on (compile-only) | `slices()`/`fill()` → `Slices()`/`Fill()`. |
| `css/CSSBorderImageWidthValue.h`, `.cpp` | `have` | `CSS/Values/BorderImageWidthValue.hpp`, `src/CSS/Values/BorderImageWidthValue.cpp` | on (compile-only) | `widths()`/`overridesBorderWidths()` → `Widths()`/`OverridesBorderWidths()`. |

**Bugs fixed:** none; the existing port was complete and matched upstream. Tidied: constructors moved to the `.cpp`s, the `.cpp`s gained a BOM, and `BorderImageWidthValue::Equals` compares in upstream's order.

**Added (upstream API):** none.

**Differences:** no out-of-line `= default` destructors (the implicit ones do the same). No `AddDerivedHash`, like upstream: both hash by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 831 cases / 653211 assertions; all pass except 2 expected failures.

### `ReflectValue` (2026-10-12)

Graph from `css/CSSReflectValue.cpp` (headers-only): 22 files (`tools/port/out/reflect-value.md`). Only `CSSReflectValue.h`/`.cpp` are new; everything else comes in through `CSSPrimitiveValue.h`, reviewed in slices 2–3 (`CSSAttrValue.h` is tier 1 item 19, and already compiles through `PrimitiveValue.hpp`). The `.cpp` adds only `MakeString.h`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSReflectValue.h`, `.cpp` | `have` | `CSS/Values/ReflectValue.hpp`, `src/CSS/Values/ReflectValue.cpp` | on (compile-only) | `direction()`/`offset()`/`mask()` → `Direction()`/`Offset()`/`Mask()`. |

**Bugs fixed:**
- `CustomVisitChildren` was missing `KRYS_NOESCAPE`; it's now defined inline, like upstream.
- The header included `Quad.hpp` for nothing.

**Added (upstream API):** `Direction`, `Offset`, `Mask`.

**Differences:** includes `Value.hpp` and forward-declares `ValueId` instead of including `PrimitiveValue.hpp` (only the keyword type is used). No `AddDerivedHash`, like upstream: hashes by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 831 cases / 653211 assertions; all pass except 2 expected failures.

### `UnicodeRangeValue` (2026-10-12)

Graph from `css/CSSUnicodeRangeValue.cpp` (headers-only): 3 files (`CSSUnicodeRangeValue.cpp`, `.h`, `CSSValue.h`; `tools/port/out/unicode-range-value.md`). The `.cpp` adds only `HexNumber.h`/`MakeString.h`; the header's `<unicode/umachine.h>` (ICU, for `char32_t`) isn't needed.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSUnicodeRangeValue.h`, `.cpp` | `have` | `CSS/Values/UnicodeRangeValue.hpp`, `src/CSS/Values/UnicodeRangeValue.cpp` | on (compile-only) | `from()`/`to()` → `From()`/`To()`. |

**Bugs fixed:**
- It declared its own `CustomVisitChildren` (never defined; upstream has none), hiding `Value`'s default.

**Added (upstream API):** `From`, `To`.

**Differences:** `hex(x, Lowercase)` is a local `std::to_chars` helper in the `.cpp` (lowercase, no padding, same output); there's no general hex-number helper yet, so add one to `Text/` if another slice needs it. No `AddDerivedHash`, like upstream: hashes by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 831 cases / 653211 assertions; all pass except 2 expected failures.

### Font values (2026-10-12)

`FontVariationValue`, `FontFeatureValue`, `FontStyleWithAngleValue`, `FontStyleRangeValue`, `FontValue` (tier 1 items 6–10), as one step. Graphs from each `css/CSS<name>.cpp` (headers-only, `--follow css/ style/ platform/graphics/`; `tools/port/out/font-*.md`). The only file not already reviewed is `platform/graphics/FontTaggedSettings.h` (from the variation and feature values); the rest comes in through `CSSPrimitiveValue.h`/`CSSValueList.h`/`CSSPrimitiveNumericTypes.h`, reviewed in slices 2–4.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/FontTaggedSettings.h` | `port` | `Graphics/FontTaggedSettings.hpp` | on | tested. Header-only. `fontFeatureTag` → `FontFeatureTag`; `tag()`/`value()`/`enabled()` → `Tag()`/`Value()`/`Enabled()`; the list keeps container-protocol names (`insert`, `isEmpty`, `size`, `at`, `begin`/`end`). |
| `platform/graphics/FontTaggedSettings.cpp` | `wont` | | | Only `TextStream` dumping. |
| `css/CSSFontVariationValue.h`, `.cpp` | `have` | `CSS/Values/FontVariationValue.hpp`, `src/CSS/Values/FontVariationValue.cpp` | on (compile-only) | `tag()`/`value()` → `Tag()`/`GetValue()`. |
| `css/CSSFontFeatureValue.h`, `.cpp` | `have` | `CSS/Values/FontFeatureValue.hpp`, `src/CSS/Values/FontFeatureValue.cpp` | on (compile-only) | Same accessors. |
| `css/CSSFontStyleWithAngleValue.h`, `.cpp` | `have` | `CSS/Values/FontStyleWithAngleValue.hpp`, `src/CSS/Values/FontStyleWithAngleValue.cpp` | on (compile-only) | `obliqueAngle()` → `GetObliqueAngle()`; `nameLiteralForSerialization` → `ToLower(ValueId)`. |
| `css/CSSFontStyleRangeValue.h`, `.cpp` | `have` | `CSS/Values/FontStyleRangeValue.hpp`, `src/CSS/Values/FontStyleRangeValue.cpp` | on (compile-only) | Public members `fontStyleValue`/`obliqueValues` → `FontStyleValue`/`ObliqueValues`. |
| `css/CSSFontValue.h`, `.cpp` | `have` | `CSS/Values/FontValue.hpp`, `src/CSS/Values/FontValue.cpp` | on (compile-only) | Public members `style`, `variant`, ... → `Style`, `Variant`, `Weight`, `Width`, `Size`, `LineHeight`, `Family`. |

**Bugs fixed:**
- `FontStyleRangeValue` and `FontValue` declared a `CustomVisitChildren` without `KRYS_NOESCAPE`; none of the five were `final`. `FontValue.hpp` had no BOM.

**Added (upstream API):** every member of the five classes (above); `FontTag`, `FontFeatureTag`, `FourCharacterTagHash`, `FourCharacterTagHashTraits`, `FontTaggedSetting`, `FontTaggedSettings`, `FontFeature`, `FontFeatureSettings`, `FontVariationSettings`.

**Differences:**
- `FontTaggedSetting::operator<` and `FontTaggedSettings::hash()` aren't ported: upstream declares them but never defines them.
- `add(Hasher&, FontTag)` → `FourCharacterTagHash::Hash` (same big-endian integer); `add(Hasher&, const FontTaggedSetting<T>&)` → `DefaultHash<FontTaggedSetting<T>>`. No `DefaultHash<FontTag>` (it would cover every `Array<char, 4>`).
- `FontVariationValue::Create`/`FontFeatureValue::Create` take `FontTag` and `Ref<Value>` by value (upstream: `Ref&&`, and `FontTag&&` for features); `FontStyleWithAngleValue::Create` takes the angle by value.
- `FontFeatureValue` calls `ResolveAsIntegerIfNotCalculated<int>()` explicitly: Krystal's default is `int64`, upstream's `int`.
- `FontStyleRangeValue::Equals` uses `CompareValue`/`CompareValuePtr` instead of `arePointingToEqualData` (same result; only the same-pointer shortcut is missing). `wtf/PointerComparison.h` isn't ported; `Style/Values/Primitives/CoordinatedValueList.hpp` (unreviewed) still calls it.
- `FontValue.hpp` includes `PrimitiveValue.hpp` instead of forward-declaring it, so its `RefPtr<PrimitiveValue>` members are complete wherever the class is destroyed.
- No `AddDerivedHash` on any of them, like upstream: all hash by pointer.
- `CSS/Font/FontFace.hpp` and `Style/BuilderState.hpp` (unreviewed) still include the raw `FontTaggedSettings.h`; switch them when their slices come.

Tests: `tests/Graphics/FontTaggedSettings.cpp` (tag creation, settings, sorted insert/replace, iteration, equality, tag and setting hashes, deleted-value traits): 5 cases / 31 assertions. The five value classes are compile-only. Full Booey suite: 836 cases / 653242 assertions; all pass except 2 expected failures.

### Grid line values (2026-10-12)

`GridLineNamesValue`, `GridLineValue` (tier 1 items 11–12). Graphs from `css/CSSGridLineNamesValue.cpp` (headers-only): 4 files, only `CSSMarkup.h` beyond the class (reviewed in slice 1); `css/CSSGridLineValue.cpp`: 22 files, all through `CSSPrimitiveValue.h` (reviewed). `tools/port/out/grid-line-names-value.md`, `grid-line-value.md`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSGridLineNamesValue.h`, `.cpp` | `have` | `CSS/Values/GridLineNamesValue.hpp`, `src/CSS/Values/GridLineNamesValue.cpp` | on (compile-only) | `names()` → `Names()`; `serializeIdentifier` → `SerialiseIdentifier`. |
| `css/CSSGridLineValue.h`, `.cpp` | `have` | `CSS/Values/GridLineValue.hpp`, `src/CSS/Values/GridLineValue.cpp` | on (compile-only) | `spanValue()`/`numericValue()`/`gridLineName()` and their `protected*` versions → PascalCase. |

**Bugs fixed:**
- Both declared their own `CustomVisitChildren` (never defined; upstream has none), hiding `Value`'s default.
- `GridLineNamesValue::Create` returned `RefPtr`; now `Ref` like upstream (the unbuilt callers assign it to a `RefPtr`).

**Added (upstream API):** `GridLineValue::SpanValue`, `ProtectedSpanValue`, `NumericValue`, `ProtectedNumericValue`, `GridLineName`, `ProtectedGridLineName`.

**Differences:**
- `GridLineValue`'s constructor and `Create` take the three `RefPtr`s by value (upstream: `RefPtr&&`).
- `makeStringByJoining(parts, " ")` is a local append lambda (no intermediate list).
- No `AddDerivedHash` on either, like upstream: both hash by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 836 cases / 653242 assertions; all pass except 2 expected failures.

### `ValueContainingList` subclasses (2026-10-12)

`SubgridValue`, `TransformListValue`, `GridAutoRepeatValue`, `GridIntegerRepeatValue` (tier 1 items 13–16). Graphs from each `css/CSS<name>.cpp` (headers-only): 4 files each (class, `CSSValueList.h`, `CSSValue.h`), plus `CSSPrimitiveValue.h` for `GridIntegerRepeatValue` (23 files, all reviewed). `tools/port/out/subgrid-value.md`, `transform-list-value.md`, `grid-auto-repeat-value.md`, `grid-integer-repeat-value.md`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSSubgridValue.h`, `.cpp` | `have` | `CSS/Values/SubgridValue.hpp`, `src/CSS/Values/SubgridValue.cpp` | on (compile-only) | |
| `css/CSSTransformListValue.h`, `.cpp` | `have` | `CSS/Values/TransformListValue.hpp`, `src/CSS/Values/TransformListValue.cpp` | on (compile-only) | |
| `css/CSSGridAutoRepeatValue.h`, `.cpp` | `have` | `CSS/Values/GridAutoRepeatValue.hpp`, `src/CSS/Values/GridAutoRepeatValue.cpp` | on (compile-only) | `autoRepeatID()` → `AutoRepeatID()`. |
| `css/CSSGridIntegerRepeatValue.h`, `.cpp` | `have` | `CSS/Values/GridIntegerRepeatValue.hpp`, `src/CSS/Values/GridIntegerRepeatValue.cpp` | on (compile-only) | `repetitions()` → `Repetitions()`. |

**Bugs fixed:**
- `GridAutoRepeatValue` and `GridIntegerRepeatValue` derived from `Value` instead of `ValueContainingList`, although their `ValueType`s sit in the list range (`ContainsValueList()`), so `Downcast<ValueContainingList>` on them would have been undefined behaviour.
- Each declared its own `CustomVisitChildren` (never defined), hiding `ValueContainingList`'s. Upstream only overrides it in `GridIntegerRepeatValue` (list items, then the repetitions), which is now defined inline.
- `SubgridValue`, `GridAutoRepeatValue` and `GridIntegerRepeatValue` included `Grid/GridTemplateAreas.hpp` (tier 2, unreviewed) for nothing.

**Added (upstream API):** `GridAutoRepeatValue::AutoRepeatID`, `GridIntegerRepeatValue::Repetitions`, the `TransformListValue::Create(Ref<Value>)` definition.

**Differences:** `GridIntegerRepeatValue::Create` takes `Ref<PrimitiveValue>` by value (upstream: `Ref&&`). No `AddDerivedHash` overrides, like upstream: the list hash from `ValueContainingList` applies (the grid repeats' extra state isn't hashed).

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 836 cases / 653242 assertions; all pass except 2 expected failures.

### `CounterValue`, `OffsetRotateValue`, `AttrValue` (2026-10-12)

Tier 1 items 17–19; tier 1 is done. Graphs from each `css/CSS<name>.cpp` (headers-only; `tools/port/out/counter-value.md`, `offset-rotate-value.md`, `attr-value.md`). `CounterValue` adds only `CSSMarkup.h` (slice 1) beyond `CSSPrimitiveValue.h`; `OffsetRotateValue`'s `.cpp` includes `CSSValuePool.h` (layer 5c; 44 files, all reviewed) but uses nothing from it; `AttrValue` adds nothing.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSCounterValue.h`, `.cpp` | `have` | `CSS/Values/CounterValue.hpp`, `src/CSS/Values/CounterValue.cpp` | on (compile-only) | `identifier()`/`separator()`/`counterStyle()`/`counterStyleCSSText()` → PascalCase. |
| `css/CSSOffsetRotateValue.h`, `.cpp` | `have` | `CSS/Values/OffsetRotateValue.hpp`, `src/CSS/Values/OffsetRotateValue.cpp` | on (compile-only) | `modifier()` → `Modifier()`, `angle()` → `GetAngle()` (would hide `css::Angle`), `isInitialValue()` → `IsInitialValue()`. |
| `css/CSSAttrValue.h`, `.cpp` | `have` | `CSS/Values/AttrValue.hpp`, `src/CSS/Values/AttrValue.cpp` | on (compile-only) | Header was already built through `PrimitiveValue.hpp`. |

**Bugs fixed:**
- `CounterValue`'s and `OffsetRotateValue`'s `CustomVisitChildren` declarations lacked `KRYS_NOESCAPE`; they're now defined inline like upstream. `CounterValue` wasn't `final`.
- `AttrValue` matched upstream; tidied only (`_attr` → `_attributeName`, the `.cpp` gained a BOM and lost an unused `Types.hpp` include, string building no longer allocates temporaries).

**Added (upstream API):** `CounterValue::Identifier`, `Separator`, `CounterStyle`, `CounterStyleCSSText`; `OffsetRotateValue::Modifier`, `GetAngle`, `IsInitialValue` and the constructor's asserts (modifier is a keyword, angle is an angle, at least one is set).

**Krystal.Core bug found (not fixed, outside the slice):** the `IntrusivePtr` overloads of `Downcast`/`DynamicDowncast` (`Types/IntrusivePtr.hpp`) can't be called as `Downcast<Target>(ptr)`: their first template parameter is both the explicit target and the source's pointee, so deduction fails. `Downcast` also asserts the pointer is non-null, where WTF's `RefPtr` downcast allows null. Nothing calls them yet. `OffsetRotateValue::Create` works around it (FIXME) by downcasting the raw pointer and re-adopting it.

**Differences:**
- `CounterValue::Equals` uses `CompareValue` instead of `arePointingToEqualData` (same result; see the font values).
- `ASSERT_NOT_REACHED` in `CounterStyleCSSText` → `krys_debug_assert_msg(false, ...)`, still returning an empty string (not `krys_unreachable()`, which would be undefined behaviour in release).
- `OffsetRotateValue.cpp` doesn't include `ValuePool.hpp` (upstream includes `CSSValuePool.h` for `StringBuilder` only).
- No `AddDerivedHash` on any of them, like upstream: all hash by pointer.

Tests: none (compile-only: `CssText` dispatches through `Value.cpp`). Full Booey suite: 836 cases / 653242 assertions; all pass except 2 expected failures.

### `PositionValue`, `PositionXValue`, `PositionYValue` (2026-10-12)

Tier 2 item 20. Graphs from `css/CSSPositionValue.cpp` (headers-only): 23 files, and `css/values/primitives/CSSPosition.cpp`: 19 files (`tools/port/out/position-csspositionvalue.md`, `position-cssposition.md`). Only `CSSPositionValue.h`/`.cpp` and `CSSPosition.h`/`.cpp` are new; the rest is the numeric primitives (slices 2–3). `CSSPosition.h` lives in `css/values/primitives/` upstream, which matches Krystal's `CSS/Values/Primitives/Position.hpp`, so it isn't moved.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/primitives/CSSPosition.h`, `.cpp` | `have` | `CSS/Values/Primitives/Position.hpp`, `src/CSS/Values/Primitives/Position.cpp` | on | tested. `isCenterPosition`/`split` → `IsCenterPosition`/`Split`; `switchOn` → `SwitchOn`; struct members `offset`/`value` → `Offset`/`Value`; `CSS::Keyword::X` → generated `keywords::X`. |
| `css/CSSPositionValue.h`, `.cpp` | `have` | `CSS/Values/PositionValue.hpp`, `src/CSS/Values/PositionValue.cpp` | on (compile-only) | All three classes in one file, like upstream. `position()` → `GetPosition()` (would hide `Position`). |

**Bugs fixed:**
- The value classes' accessor was named `Position()`, which changes the meaning of `Position` inside the class (ill-formed; MSVC accepted it).
- Their constructors weren't `noexcept`, and `CustomVisitChildren` lacked `KRYS_NOESCAPE`.
- `Position.hpp`'s public struct members were lowercase (`offset`, `value`).

**Differences:**
- `Create` takes the position by value (upstream: `&&`), like `RatioValue`.
- The single-argument value constructors are `explicit`.
- No `AddDerivedHash`, like upstream: all three hash by pointer.
- `Style/Values/Primitives/Position.hpp` (unreviewed, raw WebKit) still reads `x.offset`/`y.offset`; it changes to `Offset` when the Style position is ported.

Tests: `tests/CSS/Values/Primitives/Position.cpp` (`IsCenterPosition` including `50%` and the non-two-component forms, `Split` for all four forms, equality, serialisation, computed-style dependencies, visitation): 5 cases / 25 assertions. The value classes are compile-only. Full Booey suite: 841 cases / 653267 assertions; all pass except 2 expected failures.

### `UrlValue` and WTF's URL (2026-10-12)

Tier 2 item 21. Krystal had no URL implementation (`URL/Url.hpp` was an empty struct), so this ports WTF's URL as the base for CSS and the DOM. Decisions (2026-10-12): ICU through vcpkg for IDNA (like upstream), UTF-8-only query encoding (no `URLTextEncoding`), and the main type is `url::URL` in `URL/URL.hpp` with CSS's `CSSURL` holding one.

Graphs: `css/CSSURLValue.cpp` (headers-only): 24 files, new ones `CSSURLValue.h`, `CSSURL.h`, `CSSURLModifiers.h`; `css/values/primitives/CSSURL.cpp`: also `CSSParserContext.h` (have*, layer 3c), `CSSMarkup.h`, `Document.h`; `CSSURLModifiers.cpp`: `loader/LoadedFromOpaqueSource.h`, `loader/ResourceLoaderOptions.h`. WTF's URL is outside the graph tool's prefixes; its files were read directly (`URL.h`/`.cpp`, `URLParser.h`/`.cpp`, `URLHash.h`, `text/CodePointIterator.h`). `tools/port/out/cssurl.md`, `cssurlmodifiers.md`, `cssurlvalue.md`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `wtf/URL.h`, `.cpp` | `port` | `URL/URL.hpp`, `src/URL/URL.cpp` | on | tested. `url::URL`; methods PascalCase (`string()` → `String()`, `stringCenterEllipsizedToLength` → `StringCenterEllipsisedToLength`). Free functions too (`AboutBlankURL`, `ProtocolIs`, `PortAllowed`, …). |
| `wtf/URLParser.h`, `.cpp` | `port` | `URL/URLParser.hpp`, `src/URL/URLParser.cpp` | on | tested. Every state and helper ported; one UTF-8 `CodePointIterator` instead of the Latin-1/UTF-16 templates. `serialize` → `Serialise`, `maybeCanonicalizeScheme` → `MaybeCanonicaliseScheme`. |
| `wtf/URLHash.h` | `port` | `URL/URLHash.hpp` | on | tested. `DefaultHash<url::URL>`/`HashTraits<url::URL>`. |
| `wtf/text/CodePointIterator.h` | `port` | Krystal.Core `Text/CodePointIterator.hpp` | on | tested. UTF-8 only; adds `DecodeUTF8`/`IsValidUTF8`. |
| `wtf/URLHelpers.h`, `.cpp` | `wont` | | | Display-only (decoding punycode for UI). |
| `css/values/primitives/CSSURL.h`, `.cpp` | `have` | `CSS/Values/Primitives/URL.hpp`, `src/CSS/Values/Primitives/URL.cpp` | on | tested. Upstream's `CSS::URL` is `CSSURL` (no clash with `url::URL`); members `specified`/`resolved`/`modifiers` → `Specified`/`Resolved`/`Modifiers`. |
| `css/values/primitives/CSSURLModifiers.h`, `.cpp` | `have` | `CSS/Values/Primitives/URLModifiers.hpp` | on | tested. Members PascalCase. `applyModifiersToLoaderOptions` not ported (FIXME): no resource loader. |
| `css/CSSURLValue.h`, `.cpp` | `have` | `CSS/Values/UrlValue.hpp`, `src/CSS/Values/UrlValue.cpp` | on (compile-only) | Krystal's existing name `UrlValue` kept. `url()` → `URL()`. |
| `loader/LoadedFromOpaqueSource.h` | `port` | `Loader/LoadedFromOpaqueSource.hpp` | on | |
| `css/parser/CSSParserContext.h` | `have*` | `CSS/Parser/ParserContext.hpp` | on | Added `BaseURL` and `LoadedFromOpaqueSource`. Still no `charset`, `enclosingRuleType`, `hasDocumentSecurityOrigin`, constructors or hashing. |
| `loader/ResourceLoaderOptions.h`, `pal/text/TextEncoding.h`, `dom/Document.h` | | | off | See differences. |

**Bugs fixed:**
- `CSSURL::IsNone()` tested `specified.empty()`, so `url("")` counted as `none`; upstream tests for a null string. It now also requires a null resolved URL, which only `None()` has (`url("")` always resolves to a non-null, invalid URL).
- `URLModifiers` was missing `loadedFromOpaqueSource` (commented out), so it was never compared or propagated from the context.
- `Consumers/URL.cpp` (unbuilt) used `.parameters`, but `FunctionNotation`'s member is `Parameters`; renamed along with the modifier members.

**Krystal.Core changes (found here):**
- `Text/Unicode.hpp` used `MaxASCIIChar` without including `Text/ASCII.hpp`, so it didn't compile on its own.
- Added `text::IsASCIIAlphaCaselessEqual` (single characters; WTF's `isASCIIAlphaCaselessEqual`); Core's `IsASCIICaselessEqual` compares whole strings.
- Added `Text/CodePointIterator.hpp` (above). Core's text tests weren't built (`tests/Text/*` weren't in `Krystal.Core.Tests/CMakeLists.txt`); only the new `CodePointIterator.cpp` is added.

**Differences:**
- Strings are UTF-8 (`utf8_string`/`utf8_stringview`). WTF's null `String` is a separate `_isNull` bit on `URL`; null string views (`data() == nullptr`) stand in for WTF's null `StringView` in the getters (absent component) and in `SetQuery` (null removes the query) and `URL(base, relative)` (null relative gives the base without its fragment). Invalid UTF-8 in parser input reads as U+FFFD, one per invalid byte.
- `URLTextEncoding` (non-UTF-8 query encoding) isn't supported: queries are always UTF-8 percent-encoded. `parseAllowingC0AtEnd`'s sentinel encoding pointer is a `bool`.
- IDNA uses `uidna_nameToASCII_UTF8` (upstream converts to UTF-16 for `uidna_nameToASCII`). vcpkg's ICU is 78.3 (Unicode 17).
- The scheme length field is 24 bits (upstream 26) to make room for the null and deleted-value bits.
- Not ported: `isolatedCopy` (strings are values), `fakeURLWithRelativePart` (needs a UUID), `registerDefaultPortForProtocolForTesting`/`clearDefaultPortForProtocolMapForTesting`, `print`/`dump`/`TextStream`, the CF/NS/GLib conversions.
- `User()`/`Password()` return an empty string for invalid UTF-8 after decoding (upstream a null string).
- `RemoveQueryParameters` passes an empty remaining query as a null view, as upstream's empty `StringBuilder` does, so the `?` is removed.
- `CSSURL` serialisation always uses the specified URL: Krystal's `SerialisationContext` has no replacement URLs or `shouldUseResolvedURLInCSSText` (upstream's default is the specified URL too).
- `CompleteURL` has no WebVTT check (Krystal's `ParserMode` has no WebVTT mode), so it never returns null. The `Document` overload isn't ported: Krystal's `Document` has no base URL yet.

Tests:
- `tests/URL/WPT.cpp` runs the WPT URL data (`data/url/urltestdata.json`, `toascii.json`, `IdnaTestV2.json`, copied from WebKit's `LayoutTests/imported/w3c/web-platform-tests/url/resources`), mapping each field to the getter the DOM `URL` interface uses, plus `toascii.json`'s host/hostname setter cases: 3 cases / 13161 assertions, all pass. Three `IdnaTestV2.json` cases expect a V7 failure for U+32931/U+32B9A, which Unicode 17 (ICU 78) assigned (CJK Extension J); the test expects them to be valid. WebKit passes all of these upstream (no FAIL lines in its expectations).
- `tests/URL/URL.cpp`: WebKit's `TestWebKitAPI/Tests/WTF/URL.cpp` (except `URLDataURIStringSharing`, `PrintStream`, `IsolatedCopy`), plus setters, hashing and the string helpers: 19 cases. WebKit's `URLParser.cpp` API tests aren't ported; `urltestdata.json` covers their parsing cases.
- `tests/CSS/Values/Primitives/URL.cpp` (`CompleteURL`, `None`, `Resolve`, `MayDependOnBaseURL`, serialisation with modifiers, equality, dependencies, visitation): 6 cases / 36 assertions. `UrlValue` is compile-only.
- Krystal.Core `tests/Text/CodePointIterator.cpp`: 4 cases / 20 assertions.
- Full Booey suite: 869 cases / 666730 assertions; all pass except 2 expected failures. Core suite: 548 cases / 2234 assertions, all pass. Whole solution builds with no warnings.

### `GridTemplateAreasValue` (2026-10-13)

Tier 2 item 22. Graph from `css/CSSGridTemplateAreasValue.cpp` (headers-only): 9 files (`tools/port/out/grid-template-areas-value.md`). New: `CSSGridTemplateAreasValue.h`/`.cpp`, `values/grid/CSSGridTemplateAreas.h`, `values/grid/CSSGridNamedAreaMap.h`/`.cpp`, and the boundary `rendering/style/GridArea.h`, which needs `rendering/style/GridSpan.h`/`.cpp` and `style/values/grid/StyleGridTrackSizingDirection.h`. `GridSpan.cpp` clamps through `Style::GridPosition::max()`/`min()`, so `style/values/grid/StyleGridPosition.h`/`.cpp` (and `StyleGridPositionSide.h`, which it includes) came too, minus its CSSValue conversion. The rest is reviewed (`CSSValueTypes.h` and the aggregates).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSGridTemplateAreasValue.h`, `.cpp` | `have` | `CSS/Values/GridTemplateAreasValue.hpp`, `src/CSS/Values/GridTemplateAreasValue.cpp` | on (compile-only) | `areas()` → `Areas()`, `stringForRow` → `StringForRow`. |
| `css/values/grid/CSSGridTemplateAreas.h` | `have` | `CSS/Values/Grid/GridTemplateAreas.hpp` | on | tested. `map` → `Map`. |
| `css/values/grid/CSSGridNamedAreaMap.h`, `.cpp` | `have` | `CSS/Values/Grid/GridNamedAreaMap.hpp`, `src/CSS/Values/Grid/GridNamedAreaMap.cpp` | on | tested. `map`/`rowCount`/`columnCount` → `Map`/`RowCount`/`ColumnCount`; `addRow` → `AddRow`. |
| `rendering/style/GridArea.h` | `have` | `Rendering/Style/GridArea.hpp` | on | tested. Moved from `CSS/Values/Grid/` to match upstream's location (like `Loader/`), namespace `boo`. `columns`/`rows` → `Columns`/`Rows`, `span()` → `Span()`. |
| `rendering/style/GridArea.cpp` | `wont` | | | Only `TextStream` dumping. |
| `rendering/style/GridSpan.h`, `.cpp` | `have` | `Rendering/Style/GridSpan.hpp`, `src/Rendering/Style/GridSpan.cpp` | on | tested. Moved likewise. Methods PascalCase (`translatedDefiniteGridSpan` → `TranslatedDefiniteGridSpan`, `startLine` → `StartLine`, `clamp` → `Clamp`, ...); `begin`/`end` keep their names. |
| `style/values/grid/StyleGridTrackSizingDirection.h` | `have` | `Style/Values/Grid/GridTrackSizingDirection.hpp` | on | tested. Moved from `CSS/Values/Grid/` into `boo::style`. |
| `style/values/grid/StyleGridPosition.h`, `.cpp` | `have` | `Style/Values/Grid/GridPosition.hpp`, `src/Style/Values/Grid/GridPosition.cpp` | on | tested. Was `CSS/Values/Grid/GridPosition.hpp` in `boo::css` (unreviewed, nothing defined); moved into `boo::style`. Methods PascalCase (`max` → `Max`, `setMaxPositionForTesting` → `SetMaxPositionForTesting`, ...). `CSSValueConversion<GridPosition>` and the logging aren't ported (see differences). |
| `style/values/grid/StyleGridPositionSide.h` | `port` | `Style/Values/Grid/GridPositionSide.hpp` | on | |

**Bugs fixed:**
- `GridTemplateAreasValue` declared a `CustomVisitChildren` (never defined; upstream has none), hiding `Value`'s default, and neither it nor `GridSpan`, `GridNamedAreaMap`'s `AddRow` or `GridPosition` had any definitions (no `.cpp`s).
- `GridTemplateAreas`/`GridNamedAreaMap`/`GridArea`/`GridPosition`'s public members were lowercase; `GridPosition`'s accessors and statics were camelCase.
- `GridPosition` (a Style type) was in `boo::css`, so `Consumers/Grid.cpp`'s `GridPosition::max()` would have resolved to it.

**Krystal callers updated (unbuilt):** `Consumers/Grid.cpp` (`style::GridPosition::Max()`, `map.RowCount`, the `GridPosition.hpp` include).

**Added (upstream API):** everything in the table above; `GridArea::Span`, `GridSpan::MasonryAxisTranslatedDefiniteGridSpan`, `GridPosition::SetMaxPositionForTesting`, `Serialiser<GridNamedAreaMap>`.

**Differences:**
- One `GridTemplateAreasValue::Create(GridTemplateAreas)` by value instead of upstream's `&&`/`const &` overloads (like `RatioValue`).
- `GridNamedAreaMap::Map` (upstream's `Map` alias) is `MapType`, because the member is now `Map`; likewise `GridPositionExplicit::Position`/`GridPositionSpan::Position` (aliases) are `PositionType`, with the members `Position`/`Name`.
- `GridNamedAreaMap::Map` is a `std::unordered_map`, so iteration order differs from WTF's `HashMap`. Serialisation and `StringForRow` don't depend on it (areas don't overlap). `HashMap::ensure` → `try_emplace`.
- `StringForRow` fills a `List<const CSSOMString *>` (null for an unnamed cell) instead of a `FixedVector<String>` of null strings.
- `GridPosition`'s constructors take their argument by value (upstream `&&`).
- Not ported: `add(Hasher&, const GridPosition&)` (Krystal.Core has no `DefaultHash<StringAtom>`; nothing hashes grid positions yet), `CSSValueConversion<GridPosition>` (needs `BuilderState`/`BuilderChecking`, deferred like the Style colour conversions), and the `TextStream` logging.
- `GridPosition::SetMaxPositionForTesting` can't be reset, like upstream; tests set it back to 1000000.
- `Consumers/Grid.hpp` (unreviewed) still declares its own `GridNamedAreaMapRow` alias (same type).

Tests: `tests/Rendering/Style/GridSpan.cpp` (factories, iteration, `Translate`/`TranslateTo` both ways, both `Clamp`s, clamping to the grid position limits, `GridArea`, `OrthogonalDirection`): 5 cases; `tests/Style/Values/Grid/GridPosition.cpp` (the four kinds, limits and clamping, `SwitchOn` including the minimal `Explicit`/`Span` forms): 3 cases; `tests/CSS/Values/Grid/GridTemplateAreas.cpp` (`AddRow` including every rejection, serialisation, `none`, equality): 2 cases. 10 cases / 90 assertions. `GridTemplateAreasValue` is compile-only.

### `EasingFunctionValue` (2026-10-13)

Tier 2 item 23; tier 2 is done. Graph from `css/CSSEasingFunctionValue.cpp` (headers-only): 26 files (`tools/port/out/easing-function-value.md`). New: `CSSEasingFunctionValue.h`/`.cpp` and the five `css/values/easing/` headers (no `.cpp`s); the rest is the numeric primitives (slices 2–3).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSEasingFunctionValue.h`, `.cpp` | `have` | `CSS/Values/EasingFunctionValue.hpp`, `src/CSS/Values/EasingFunctionValue.cpp` | on (compile-only) | `easingFunction()` → `GetEasingFunction()` (would hide `EasingFunction`). |
| `css/values/easing/CSSEasingFunction.h` | `have` | `CSS/Values/Easing/EasingFunction.hpp` | on | tested. `value` → `Value`, `switchOn` → `SwitchOn`. |
| `css/values/easing/CSSCubicBezierEasingFunction.h` | `have` | `CSS/Values/Easing/CubicBezierEasingFunction.hpp` | on | tested. `value` → `Value`. |
| `css/values/easing/CSSLinearEasingFunction.h` | `have` | `CSS/Values/Easing/LinearEasingFunction.hpp` | on | tested. `stops`/`output`/`input`/`extra` → PascalCase. |
| `css/values/easing/CSSSpringEasingFunction.h` | `have` | `CSS/Values/Easing/SpringEasingFunction.hpp` | on | tested. `mass`/`stiffness`/`damping`/`initialVelocity` → PascalCase. |
| `css/values/easing/CSSStepsEasingFunction.h` | `have` | `CSS/Values/Easing/StepsEasingFunction.hpp` | on | tested. `Kind`'s `steps`/`keyword` → `Steps`/`Keyword` (template parameter `Keyword` → `TKeyword`). |

**Bugs fixed:**
- No `Serialiser` for `StepsEasingParameters::Kind`: as a one-element tuple it serialised only the step count, so `steps(4, jump-start)` came out as `steps(4)`.
- `EasingFunctionValue` had no constructor, member, accessor or `.cpp`, and its `CustomVisitChildren` lacked `KRYS_NOESCAPE`.
- `LinearEasingParameters::Stop::Length::extra` was a `Markable<Percentage<>>` (upstream `std::optional`); now `Maybe`, like `Stop::input`.
- Public members were lowercase; `EasingFunction::switchOn` was camelCase; `Stop::operator==` wasn't `noexcept`.

**Krystal callers updated (unbuilt):** `Consumers/Easing.cpp` (designated initialisers: `.Parameters`, `.Stops`, `.Value`, `.Input`, `.Extra`, `.Output`, `.Mass`, ...).

**Added (upstream API):** `EasingFunctionValue::Create`, `GetEasingFunction`, `CustomCSSText`, `Equals`, `CustomVisitChildren`; `Serialiser<StepsEasingParameters::Kind>`.

**Differences:** `std::numeric_limits` → `Limits`. No `AddDerivedHash`, like upstream: hashes by pointer.

Tests: `tests/CSS/Values/Easing/EasingFunction.cpp` (serialisation of every keyword and function, including each steps position and the omitted `end`/`jump-end` keyword; equality; `SwitchOn`; visitation and dependencies): 6 cases / 30 assertions. `EasingFunctionValue` is compile-only. Full Booey suite: 885 cases / 666850 assertions; all pass except 2 expected failures.

### `BoxShadowPropertyValue`, `TextShadowPropertyValue` (2026-10-13)

Tier 3 items 24–25. Graphs from `css/CSSBoxShadowPropertyValue.cpp` and `css/CSSTextShadowPropertyValue.cpp` (headers-only): 62 files each (`tools/port/out/box-shadow-property-value.md`, `text-shadow-property-value.md`). New: the value classes, `values/borders/CSSBoxShadow.h`/`CSSBoxShadowProperty.h` and `values/text-decoration/CSSTextShadow.h`/`CSSTextShadowProperty.h` (no `.cpp`s); the rest is reviewed (colour, numerics, `ValuePool`). The `DeprecatedCSSOM*` includes are won't-port.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/borders/CSSBoxShadow.h` | `have` | `CSS/Values/Borders/BoxShadow.hpp` | on | tested. Moved from `CSS/Values/Background/` to match upstream (the folder is gone). Members `color`/`location`/`blur`/`spread`/`inset`/`isWebkitBoxShadow` → PascalCase; `Color`'s type is written `boo::css::Color`. |
| `css/values/borders/CSSBoxShadowProperty.h` | `have` | `CSS/Values/Borders/BoxShadowProperty.hpp` | on | tested. Moved likewise. |
| `css/CSSBoxShadowPropertyValue.h`, `.cpp` | `have` | `CSS/Values/BoxShadowPropertyValue.hpp`, `src/CSS/Values/BoxShadowPropertyValue.cpp` | on (compile-only) | `shadow()` → `Shadow()`. |
| `css/values/text-decoration/CSSTextShadow.h` | `have` | `CSS/Values/TextDecoration/TextShadow.hpp` | on | tested. Members PascalCase, as for `BoxShadow`. |
| `css/values/text-decoration/CSSTextShadowProperty.h` | `have` | `CSS/Values/TextDecoration/TextShadowProperty.hpp` | on | tested. New file: the alias was in `TextShadow.hpp`. |
| `css/CSSTextShadowPropertyValue.h`, `.cpp` | `have` | `CSS/Values/TextShadowPropertyValue.hpp`, `src/CSS/Values/TextShadowPropertyValue.cpp` | on (compile-only) | `shadow()` → `Shadow()`. |

**Bugs fixed:**
- `Aggregates.hpp` didn't specialise `TreatAsOptionalLike` for `Markable` (upstream `CSSValueAggregates.h` does), so a `Markable` inside a tuple-like type couldn't be serialised, visited or have its dependencies collected (compile error).
- `TextShadowPropertyValue`'s accessor was named `Value()`, which hides the base class `Value` inside the class (ill-formed). It had no `.cpp`, so the constructor and every method were declared but never defined.
- `BoxShadow`/`TextShadow` used `Maybe` where upstream uses `Markable` (colour, blur, spread), doubling the flag storage per member.
- Both value classes' `CustomVisitChildren` lacked `KRYS_NOESCAPE`; `BoxShadowPropertyValue`'s constructor wasn't `explicit`, and its `.cpp` had no BOM.
- Public members were lowercase; `TextShadow`'s `operator==` and `get` weren't `noexcept`/`KRYS_NODISCARD`.

**Krystal callers updated (unbuilt):** `Consumers/Background.cpp` (includes, `.Color`/`.Location`/`.Blur`/`.Spread`/`.Inset`/`.IsWebkitBoxShadow`), `Consumers/TextDecoration.cpp` (includes `TextShadowProperty.hpp`, designated initialisers). Their `Maybe` locals convert to `Markable`, as upstream's `std::optional` locals do.

**Added (upstream API):** `TextShadowPropertyValue::Create`, `Shadow`, `CustomCSSText`, `Equals`, `CustomVisitChildren` (definitions); `TextShadowProperty.hpp`.

**Differences:**
- `BoxShadow::IsWebkitBoxShadow` defaults to `false` (upstream leaves it uninitialised).
- No `createDeprecatedCSSOMWrapper` (won't-port). No `AddDerivedHash`, like upstream: both hash by pointer.

Tests: `tests/CSS/Values/Borders/BoxShadow.cpp` (serialisation of each optional part and `inset`, `IsWebkitBoxShadow` not serialised but compared, the property's `none` and lists, equality, dependencies, visitation): 4 cases; `tests/CSS/Values/TextDecoration/TextShadow.cpp` (the same for `TextShadow`): 3 cases. 7 cases / 37 assertions. The value classes are compile-only. Full Booey suite: 892 cases / 666887 assertions; all pass except 2 expected failures.

### `PathValue`, with the geometry types and the SVG path pipeline (2026-10-13)

Tier 3 item 26. Graph from `css/CSSPathValue.cpp` with `--follow css/ style/ svg/ platform/graphics/` (headers-only; `tools/port/out/path-value.md`). `CSSPathValue` and `CSSPathFunction` are small; the work is `CSS::Path`'s `SVGPathByteStream` (an empty class in Krystal), whose serialisation and parsing need SVG's path parser, which needs `AffineTransform` for arcs. Decisions (2026-10-13): port the byte stream with the parse/serialise pipeline and `AffineTransform`, and all of `AffineTransform` including the geometry its `mapRect`/`mapQuad`/`mapRegion`/`toTransformationMatrix` need. Geometry graph: `tools/port/out/affine-transform.md` (from `AffineTransform.cpp`, `TransformationMatrix.cpp`, `Region.cpp`, `FloatQuad.cpp`, `FloatRect.cpp`, `IntRect.cpp`, `GeometryUtilities.cpp`, `--follow platform/graphics/ platform/`). Not ported: the graphics `Path` (and `PathImpl`/`PathSegment`/platform backends) that the byte stream caches.

**Geometry** (`Graphics/`, `Core/`):

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/FloatConversion.h` | `port` | `Core/FloatConversion.hpp` | on | `NarrowPrecisionToFloat(double)`; other types are a deleted template (upstream: a declared-only primary template). No CGFloat variants. |
| `platform/BoxExtents.h` | `port` | `Core/BoxExtents.hpp` | on | `toLayoutBoxExtent` → `ToLayoutBoxExtent`. |
| `platform/graphics/DoubleSize.h`, `.cpp` | `port` | `Graphics/DoubleSize.hpp`, `src/Graphics/DoubleSize.cpp` | on | tested. Constructor from `FloatSize` takes `const &` (upstream `const FloatSize&&`). |
| `platform/graphics/DoublePoint.h`, `.cpp` | `port` | `Graphics/DoublePoint.hpp` | on | tested. No JSON, `LogArgument`, CG/Win. `FloatSize`/`FloatPoint`/`LayoutPoint` gain their `Double*` conversions (missing since layer 4a). |
| `platform/graphics/FloatPoint3D.h`, `.cpp` | `port` | `Graphics/FloatPoint3D.hpp`, `src/Graphics/FloatPoint3D.cpp` | on | tested. `normalize` → `Normalise`. |
| `platform/graphics/transforms/Quaternion.h`, `.cpp` | `port` | `Graphics/Transforms/Quaternion.hpp`, `src/Graphics/Transforms/Quaternion.cpp` | on | tested. Members `X`/`Y`/`Z`/`W`; `Slerp`/`Accumulate`/`Interpolate` are `const` (upstream non-const, but they don't modify). |
| `platform/graphics/IntRect.h`, `.cpp`, `IntRectHash.h` | `port` | `Graphics/IntRect.hpp`, `IntRectHash.hpp`, `src/Graphics/IntRect.cpp` | on | tested (upstream's `IntRectTests`). No `area()` (no `Checked`). `IsValid`/`ToRectWithExtentsClippedToNumericLimits` use `int64` sums. The hash combines the four fields. `IntPoint::ConstrainedWithin` added. |
| `platform/graphics/FloatRect.h`, `.cpp` | `port` | `Graphics/FloatRect.hpp`, `src/Graphics/FloatRect.cpp` | on | tested (upstream's `FloatRectTests`). `ContainsMode` is an enum class (`FloatRect::ContainsMode::InsideOrOnStroke`). `normalizeRect` → `NormaliseRect`. `MarkableTraits`. `FloatPoint::ConstrainedWithin` added (`src/Graphics/FloatPoint.cpp`). |
| `platform/graphics/LayoutRect.h`, `.cpp` | `port` | `Graphics/LayoutRect.hpp`, `src/Graphics/LayoutRect.cpp` | on | tested. `unionRect(const Vector<LayoutRect>&)` → `UnionRect(Span<const LayoutRect>)`. |
| `platform/graphics/FloatQuad.h`, `.cpp` | `port` | `Graphics/FloatQuad.hpp`, `src/Graphics/FloatQuad.cpp` | on | tested (upstream's `FloatQuadTests`). `p1()`..`p4()` → `P1()`..`P4()`; the list helpers take `Span`s. |
| `platform/graphics/GeometryUtilities.h`, `.cpp` | `port` | `Graphics/GeometryUtilities.hpp`, `src/Graphics/GeometryUtilities.cpp` | on | tested. `normalizeAngleInRadians` → `NormaliseAngleInRadians`; `euclidianDistance` keeps its spelling; `min4` etc. → `Min4` (constexpr); `Vector` parameters → `Span`. |
| `platform/graphics/Region.h`, `.cpp` | `port` | `Graphics/Region.hpp`, `src/Graphics/Region.cpp` | on | tested (upstream's `RegionTests`). `SmallList` for the segments/spans; `Span` members `Y`/`SegmentIndex`; inside `Region`, `krys::Span` is written qualified (the nested `Region::Span` hides it). `Bounds()` uses `int64` differences. |
| `platform/graphics/transforms/AffineTransform.h`, `.cpp` | `port` | `Graphics/Transforms/AffineTransform.hpp`, `src/…/AffineTransform.cpp` | on | tested (upstream's `AffineTransform` tests). `span()` → `AsSpan()` (would hide `Span`); the namespace-level `identity` → `AffineTransform::Identity`. `FloatPoint::MatrixTransform` added. |
| `platform/graphics/transforms/TransformationMatrix.h`, `.cpp` | `port` | `Graphics/Transforms/TransformationMatrix.hpp`, `src/…/TransformationMatrix.cpp` | on | tested (upstream's `TransformationMatrix` tests). Portable path only (no NEON/SSE2 multiply or inverse). `m11()` → `M11()`, `type()` → `GetType()`, `identity` → `Identity`; decomposition members PascalCase. |

**SVG path pipeline** (`SVG/Internals/`, `boo::svg`, beside the existing byte stream stub):

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `svg/SVGPathSeg.h` (`SVGPathSegType`) | `port` (partial) | `SVGPathSegType.hpp` | on | The enum only; `SVGPathSeg` is SVG DOM. |
| `svg/SVGPathConsumer.h`, `SVGPathSource.h` | `port` | `SVGPathConsumer.hpp`, `SVGPathSource.hpp` | on | `PathCoordinateMode`/`PathParsingMode` are enum classes (`NormalizedParsing` → `NormalisedParsing`). Segment members PascalCase. |
| `svg/SVGParserUtilities.h`, `.cpp` | `port` (partial) | `SVGParserUtilities.hpp`, `src/…` | on | tested. Parsers consume a `dom::DOMStringView&` (UTF-8) instead of `StringParsingBuffer`. `parseNumber(StringView)` → `ParseNumberFromString`. Not ported: `parseKerningUnicodeString`, `parseGlyphName` (SVG fonts). |
| `svg/SVGPathByteStream.h` | `port` | `SVGPathByteStream.hpp`, `src/…` | on | tested. Replaces the stub. |
| `svg/SVGPathByteStreamSource.h`, `.cpp`, `SVGPathByteStreamBuilder.h`, `.cpp` | `port` | same names | on | tested. |
| `svg/SVGPathStringViewSource.h`, `.cpp`, `SVGPathStringBuilder.h`, `.cpp` | `port` | same names | on | tested. |
| `svg/SVGPathParser.h`, `.cpp` | `port` | `SVGPathParser.hpp`, `src/…` | on | tested, including arc decomposition (normalised parsing). |
| `svg/SVGPathUtilities.h`, `.cpp` | `port` (partial) | `SVGPathUtilities.hpp`, `src/…` | on | `BuildSVGPathByteStreamFromString`, `BuildStringFromByteStream` only; the rest need `Path`/`SVGPathSegList` (FIXME). |
| `wtf/text/StringConcatenateNumbers.h` (`FormattedNumber::fixedPrecision`), `wtf/dtoa.h` (`numberToFixedPrecisionString`) | `port` | `FormatNumberFixedPrecision` in `Krystal.Booey/Core/NumberFormatting.hpp` | on | tested with WTF's `StringNumberFixedPrecision`/`FormattedDoubleFixedPrecision` expectations. ECMAScript `toPrecision` rules on `std::to_chars` (upstream double-conversion). |
| `wtf/MathExtras.h` (`piOverTwoFloat`) | `port` | Krystal.Core `Maths/Base.hpp` | on | |

**CSS:**

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/shapes/CSSFillRule.h` | `have` | `CSS/Values/Shapes/FillRule.hpp` | on | Matches upstream. |
| `css/values/shapes/CSSPathFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/PathFunction.hpp`, `src/CSS/Values/Shapes/PathFunction.cpp` | on | tested. `Path::Data` stays `PathData` (the member is `Data`); members `FillRule`/`Data`/`ByteStream`. |
| `css/CSSPathValue.h`, `.cpp` | `have` | `CSS/Values/PathValue.hpp`, `src/CSS/Values/PathValue.cpp` | on (compile-only) | `path()` → `GetPath()` (would hide `Path`). |

**Bugs fixed:**
- `PathValue` was an empty shell: no `final`, constructor, `Create`, accessor, member or `.cpp`, and its `CustomVisitChildren` lacked `KRYS_NOESCAPE`. `PathFunction.hpp`'s serialiser, dependency collector and visitor were declared but never defined.
- `SVGPathByteStream` was an empty class, so every `path()` compared equal and serialised as `""`.
- `Path`'s member `Maybe<FillRule> FillRule` changed the meaning of `FillRule` inside the struct (ill-formed; MSVC accepted it); the type is now written `boo::css::FillRule`.
- Upstream: `genericParseNumber` (SVG number parsing) did `return false;` in a function returning `std::optional<float>` when the exponent was too large, which returns an engaged 0, so `1e39` parsed as 0 instead of failing. Krystal fails.
- Upstream test bug: `FloatQuadTests`' `ExpectRotatedRect` computed its comparisons and dropped them, so the rotated-rect tests checked nothing. Krystal checks them; the trapezoid's expected size (26.83, 13.42) is corrected to (26.833, 13.416) (12√5, 6√5) for the 0.001 tolerance.

**Differences:**
- `SVGPathByteStream` holds its bytes by value; upstream shares them between copies (copy-on-write) and caches the platform `Path` built from them (FIXME until `Path` is ported). `bytes()` → `GetBytes()` (would hide `Bytes`).
- The SVG consumer/source/byte stream have no weak-pointer bases; the parser holds references. The byte stream reader asserts the remaining size (upstream's `consumeSpan` bounds-checks).
- Region tests: upstream compares `TextStream` dumps (won't-port); Krystal compares the same data structurally (`Rects()` and the segment/span lists). The fuzz test uses a fixed seed.
- No `operator<<`/`TextStream`, JSON, IPC coders, TZone allocation or platform (CG/Skia/Cairo/Win/simd) conversions anywhere in the geometry types.
- Upstream's `AffineTransform::Blend` doubles the blended components for non-`Replace` composite operations; kept as is.

Tests: geometry: `tests/Graphics/{DoublePoint,FloatPoint3D,IntRect,FloatRect,LayoutRect,FloatQuad,GeometryUtilities,Region}.cpp`, `tests/Graphics/Transforms/{Quaternion,AffineTransform,TransformationMatrix,TransformExtras}.cpp` (upstream's `IntRectTests`, `FloatRectTests`, `FloatQuadTests`, `RegionTests`, `AffineTransform` and `TransformationMatrix` tests, converted with a gtest→Catch2 script, minus the platform-only `Casting`/`EnclosingIntRect` cases and `area()`; `FloatPointTests`' `Transforms`; plus Krystal tests for everything upstream leaves untested, e.g. `LayoutRect` and `GeometryUtilities` have no upstream tests, and 3D rotation, projection, perspective and decomposition round trips). SVG: `tests/SVG/Internals/SVGPath.cpp` (number/flag/point/rect parsing, unaltered round trips of every command and syntax variant, parse errors, normalised parsing including arcs) and `tests/Core/NumberFormatting.cpp` (`FormatNumberFixedPrecision`). CSS: `tests/CSS/Values/Shapes/PathFunction.cpp` (serialisation with and without a fill rule, canonical path data, equality, dependencies, visitation). `PathValue` is compile-only. Full Booey suite: 1077 cases / 669775 assertions; all pass except 2 expected failures. Core suite: 548 cases, all pass. Whole solution builds with no warnings.

### `BasicShapeValue`, with the shape functions and `BorderRadius` (2026-10-13)

Tier 3 item 27. Graph from `css/CSSBasicShapeValue.cpp` (headers-only): 34 files (`tools/port/out/basic-shape-value.md`). New: the value class, `values/shapes/CSSBasicShape.h` and the seven shape functions other than `path()` (`CSSCircleFunction`, `CSSEllipseFunction`, `CSSInsetFunction`, `CSSPolygonFunction`, `CSSRectFunction`, `CSSShapeFunction`, `CSSXywhFunction`, each with a `.cpp`), and `values/borders/CSSBorderRadius.h`/`.cpp`; the rest is reviewed (numerics, `Position`, `PathFunction`, `FillRule`). Decision (2026-10-13): the `rect()` shape is `RectShape`, because upstream's `WebCore::Rect` (`css/Rect.h`, behind `RectValue`) and `WebCore::CSS::Rect` are both `css::Rect` here.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/borders/CSSBorderRadius.h`, `.cpp` | `have` | `CSS/Values/Borders/BorderRadius.hpp`, `src/CSS/Values/Borders/BorderRadius.cpp` | on | tested. `defaultValue()`/`topLeft()`.. → `DefaultValue()`/`TopLeft()`..; `hasDefaultValue` → `HasDefaultValue`. |
| `css/values/shapes/CSSCircleFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/CircleFunction.hpp`, `src/…/CircleFunction.cpp` | on | tested. `Position`'s type is written `boo::css::Position`. |
| `css/values/shapes/CSSEllipseFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/EllipseFunction.hpp`, `src/…/EllipseFunction.cpp` | on | tested. As `Circle`. |
| `css/values/shapes/CSSInsetFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/InsetFunction.hpp`, `src/…/InsetFunction.cpp` | on | tested. `Inset::Insets` (alias) → `InsetsType`: the member is `Insets`. |
| `css/values/shapes/CSSPolygonFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/PolygonFunction.hpp`, `src/…/PolygonFunction.cpp` | on | tested. `Polygon::Vertices` (alias) → `VerticesType`; `FillRule`'s type is written `boo::css::FillRule`. |
| `css/values/shapes/CSSRectFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/RectFunction.hpp`, `src/…/RectFunction.cpp` | on | tested. `CSS::Rect` → `RectShape` (see above). |
| `css/values/shapes/CSSXywhFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/XywhFunction.hpp`, `src/…/XywhFunction.cpp` | on | tested. `Xywh::Location`/`Size` (aliases) → `LocationType`/`SizeType`. |
| `css/values/shapes/CSSShapeFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/ShapeFunction.hpp`, `src/…/ShapeFunction.cpp` | on | tested. `affinity`/`name` → `Affinity`/`Name`, members PascalCase. `RelativeControlPoint::Anchor`/`AbsoluteControlPoint::Anchor` (aliases) → `AnchorType`; `Shape::Commands` → `CommandsType`; `ArcSweep`/`ArcSize`/`FillRule`'s types are written `boo::css::X`. |
| `css/values/shapes/CSSBasicShape.h` | `have` | `CSS/Values/Shapes/BasicShape.hpp` | on | tested. |
| `css/CSSBasicShapeValue.h`, `.cpp` | `have` | `CSS/Values/BasicShapeValue.hpp`, `src/CSS/Values/BasicShapeValue.cpp` | on (compile-only) | `shape()` → `GetShape()` (would hide `Shape`). |

**Bugs fixed:**
- No shape function or `BorderRadius` had a `.cpp`, so every `Serialiser` (and `HasDefaultValue`) was declared but never defined.
- Members named after their own types, which changes the name's meaning inside the struct (ill-formed; MSVC accepts it): `Maybe<Position> Position` (`Circle`, `Ellipse`), `Maybe<FillRule> FillRule` (`Polygon`, `Shape`), `Insets Insets`, `Vertices Vertices`, `Location Location`/`Size Size` (`Xywh`), `Maybe<Anchor> Anchor` (both control points), `ArcSweep ArcSweep`/`ArcSize ArcSize` (`ArcCommand`), `Commands Commands` (`Shape`).
- `BorderRadius`'s corner accessors used `.value[i]`; the aggregates' member is `Value` (compile error once used).
- `InsetFunction.hpp` didn't include `BorderRadius.hpp`, and the shape headers included `Types.hpp` instead of `NumericTypes.hpp` for `LengthPercentage`.
- `BasicShapeValue` wasn't `final`, its constructor wasn't `explicit` and was inline, its `CustomVisitChildren` lacked `KRYS_NOESCAPE`, and `Shape()` hid the `Shape` struct.
- clang-format had merged grammar comments with their spec links; `<control-point-anchor>` had been capitalised to `<control-point-Anchor>`. Restored.
- Missing `KRYS_NODISCARD` on the `operator==`s and the control points' `get`; the eight shape headers and `BasicShapeValue.cpp` had no BOM.

**Krystal callers updated (unbuilt):** none; `Consumers/Shapes.cpp` (shape parsing) is still a stub.

**Differences:**
- `ShapeFunction.cpp` shares the identical `to`/`by`, control point, curve and smooth serialisation through local templates; upstream repeats them per type. Output is the same.

Tests: `tests/CSS/Values/Borders/BorderRadius.cpp` (`HasDefaultValue`, minimal serialisation of each axis and the `/` form, corners, dependencies, visitation): 4 cases; `tests/CSS/Values/Shapes/BasicShape.cpp` (serialisation of `circle()`, `ellipse()`, `inset()`, `polygon()`, `rect()`, `xywh()` including their defaults and `round`, the `BasicShape` variant, equality, dependencies, visitation): 9 cases; `tests/CSS/Values/Shapes/ShapeFunction.cpp` (every command in both `to`/`by` forms, control point anchors, arc defaults, `shape()` with fill rules, equality, dependencies, visitation): 7 cases. 20 cases / 96 assertions. `BasicShapeValue` is compile-only. Full Booey suite: 1097 cases / 669871 assertions; all pass except 2 expected failures.

### `FilterValue`, with the filter functions (2026-10-13)

Tier 3 item 28. Graph from `css/CSSFilterValue.cpp` (headers-only): 76 files (`tools/port/out/filter-value.md`). New: the value class, `values/filter-effects/CSSFilter.h`, `CSSFilterReference.h` and the ten filter function headers (no `.cpp`s); the rest is reviewed (colour, numerics, `CSSURL`, `ValuePool`). `CSSFilterFunction.h` (with `CSSAppleInvertLightnessFunction.h`) is only reached through `DeprecatedCSSOMFilterFunctionValue.h` (won't-port), so it isn't ported; `AppleInvertLightness` comes with item 29. `CSSFilterFunctionDescriptor.h` is the parser's and isn't in the graph. Decision: the folder `CSS/Values/Filter/` is renamed `CSS/Values/FilterEffects/` to match upstream's `filter-effects/` (as `text-decoration/` → `TextDecoration/`); the unreviewed files in it (`AppleColorFilter`, `AppleInvertLightnessFunction`, `FilterFunctionDescriptor`, `FilterOperationType`) moved with it, includes only.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/filter-effects/CSSBlurFunction.h`, `CSSBrightnessFunction.h`, `CSSContrastFunction.h`, `CSSGrayscaleFunction.h`, `CSSHueRotateFunction.h`, `CSSInvertFunction.h`, `CSSOpacityFunction.h`, `CSSSaturateFunction.h`, `CSSSepiaFunction.h` | `have` | `CSS/Values/FilterEffects/<Name>Function.hpp` | on | tested. `value` → `Value`. |
| `css/values/filter-effects/CSSDropShadowFunction.h` | `have` | `CSS/Values/FilterEffects/DropShadowFunction.hpp` | on | tested. `color`/`location`/`stdDeviation` → PascalCase; `Color`'s type is written `boo::css::Color`. |
| `css/values/filter-effects/CSSFilterReference.h` | `have` | `CSS/Values/FilterEffects/FilterReference.hpp` | on | tested. `url` → `Url` (`CSSURL`). |
| `css/values/filter-effects/CSSFilter.h` | `have` | `CSS/Values/FilterEffects/Filter.hpp` | on | tested. `CSS::FilterValue` → `FilterData` (clashes with the value class `css::FilterValue`); member `Value`. |
| `css/CSSFilterValue.h`, `.cpp` | `have` | `CSS/Values/FilterValue.hpp`, `src/CSS/Values/FilterValue.cpp` | on (compile-only) | `filter()` → `GetFilter()` (would hide `Filter`). |

**Bugs fixed:**
- `FilterValue` had no `.cpp`, so its constructor and every method were declared but never defined. Its accessor `Filter()` hid the `Filter` struct (its own return type, ill-formed), its constructor wasn't `explicit`, and its `CustomVisitChildren` lacked `KRYS_NOESCAPE`.
- Public members were lowercase; the `operator==`s weren't `KRYS_NODISCARD`/`noexcept`.

**Krystal callers updated (unbuilt):** `Consumers/Filter.cpp` (includes, `.Parameters` for every filter function, the drop-shadow's `.Color`/`.Location`/`.StdDeviation`, and `list.Value`, which the list wrappers had already renamed).

**Differences:** No `createDeprecatedCSSOMWrapper` (won't-port). No `AddDerivedHash`, like upstream: hashes by pointer.

Tests: `tests/CSS/Values/FilterEffects/Filter.cpp` (serialisation of every function with and without its optional parameter, `drop-shadow()` with each optional part, references, `none` and lists; equality, including same parameter/different function and list order; `SwitchOn`; dependencies; visitation): 6 cases / 43 assertions. `FilterValue` is compile-only. Full Booey suite: 1103 cases / 669914 assertions; all pass except 2 expected failures.

### `AppleColorFilterValue` (2026-10-13)

Tier 3 item 29, so tier 3's filter items are done. Graph from `css/CSSAppleColorFilterValue.cpp` (headers-only): 91 files (`tools/port/out/apple-color-filter-value.md`). New: the value class, `values/filter-effects/CSSAppleColorFilter.h` and `CSSAppleInvertLightnessFunction.h` (no `.cpp`s); the filter functions came with item 28. `CSSFilterFunction.h` is again only reached through the won't-port `DeprecatedCSSOMFilterFunctionValue.h`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/values/filter-effects/CSSAppleInvertLightnessFunction.h` | `have` | `CSS/Values/FilterEffects/AppleInvertLightnessFunction.hpp` | on | tested. Serialises as `apple-invert-lightness()` (the keyword has no leading dash, as upstream). |
| `css/values/filter-effects/CSSAppleColorFilter.h` | `have` | `CSS/Values/FilterEffects/AppleColorFilter.hpp` | on | tested. `CSS::AppleColorFilterValue` → `AppleColorFilterData` (clashes with the value class), member `Value`. The upstream comment's `-apple-invert-lightness-filter()` is kept as is (the function is `apple-invert-lightness()`). |
| `css/CSSAppleColorFilterValue.h`, `.cpp` | `have` | `CSS/Values/AppleColorFilterValue.hpp`, `src/CSS/Values/AppleColorFilterValue.cpp` | on (compile-only) | `filter()` → `GetFilter()`, as `FilterValue`. |

**Bugs fixed:**
- `AppleColorFilterValue.cpp` didn't include the serialisation/visitation headers its `Serialise`/`VisitValueChildren` calls need, used `std::move`, and had no BOM.
- `AppleColorFilterValue::Filter()` was renamed `GetFilter()` for consistency with `FilterValue` (it didn't hide anything here, but upstream's names match). The constructor took its argument by value; `CustomVisitChildren` lacked `KRYS_NOESCAPE`.
- Public members were lowercase; the `operator==`s weren't `KRYS_NODISCARD`/`noexcept`.

**Krystal callers updated (unbuilt):** none needed (`Consumers/Filter.cpp` was updated in item 28).

**Differences:** No `createDeprecatedCSSOMWrapper` (won't-port). No `AddDerivedHash`, like upstream.

Tests: `tests/CSS/Values/FilterEffects/AppleColorFilter.cpp` (serialisation of `apple-invert-lightness()`, `none` and lists; equality; `SwitchOn`; dependencies; visitation): 4 cases / 17 assertions. `AppleColorFilterValue` is compile-only. Full Booey suite: 1107 cases / 669931 assertions; all pass except 2 expected failures.

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
| `platform/LayoutUnit.h` | `port` | `Core/LayoutUnit.hpp` | Done in slice 3, layer 4a. |
| `platform/RectCorners.h` | `have` | `Core/RectCorners.hpp` | |
| `platform/RectEdges.h` | `have` | `Core/RectEdges.hpp` | |
| `loader/LoadedFromOpaqueSource.h` | `port` | | Single enum. |
| `pal/text/TextEncoding.h` | `have*` | `Krystal.Core/Text/Encodings/*` | The Core encodings replace it; `EncodingRegistry` may not be the final API. |
