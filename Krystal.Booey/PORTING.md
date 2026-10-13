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
- Values (slices 2 and 4, 2026-10-13): `Value`, `PrimitiveValue`, `ParserContext` and every `Value` subclass built and instance-tested, with the CSS types they hold (positions, URLs and WTF's `url::URL`, grid areas, easing, shadows, shapes, filters, `ray()`, variable data, gradients), the geometry types and the SVG path pipeline. Builder- and loader-dependent members are deferred (below). See the *Values* section.
- Property parser, layer a (2026-10-13): parser plumbing (`Property`, `ParsedPropertyList`, `PropertyParserResult`, `PropertyShorthand` and the generated shorthand functions, `CustomPropertySyntax`, `VariableParser`) built and tested; see the *Property parser* section.
- Property parser, layer b1 (2026-10-13): the shared consumer foundations (the remaining `*Definitions.hpp`, `MetaResolver`, the `PrimitiveValue` resolver, `List`, `Percentage`, `String`) built and tested.
- Property parser, layer b2 (2026-10-13): the ident/keyword-level consumers (`Align`, `Anchor`, `Attr`, `Box`, `Display`, `Inline`, `PositionTry`, `Ratio`, `ScrollSnap`, `Scrollbars`, `Syntax`, `Transitions`, `UnicodeRange`, `URL`, `ViewTransition`, `WillChange`) built and tested.

**Open / next candidates:**
- Property parser, next: layer b3 (see the group plan in the *Property parser* section); then b4–b5, then layer c (`PropertyParserCustom.hpp`, generated `PropertyParsing.cpp`, `PropertyParser.cpp`). See the *Property parser* section for the plan.
- Property parser, deferred definitions (need `style::CustomProperty`/`BuilderState`): `VariableParser::ParseInitialValueForUniversalSyntax`; layer c adds `PropertyParser::ParseTypedCustomPropertyValue`/`ParseTypedCustomPropertyInitialValue`.
- Values, deferred definitions (declared, not defined, FIXME'd; define when `style::Builder`/`BuilderState` and the resource loader are ported): `VariableReferenceValue::ResolveVariableReferences`, `ResolveSingleValue`, `ResolveTokenRange`, `ResolveVariableReference`, `ResolveVariableFallback`, `TryResolveSimpleReference` (and so `ResolveAndCacheValue`); `PendingSubstitutionValue::ResolveValue` (also needs `PropertyParser::ParseValue` and `ParsedPropertyVector`); `FontFaceSrcResourceValue::GetFontLoadRequest` and its cached font (`CustomTraverseSubresources` returns `false` until then); `FontFaceSrcLocalValue::GetSVGFontFaceElement`/`SetSVGFontFaceElement` (need SVG fonts); `CreateStyleImage` on the ten image values (needs `style::BuilderState` and `StyleImage`, and the cached style image members of `NamedImageValue`/`CanvasValue`/`GradientValue`); `ImageValue::IsPending`, `LoadImage`, `GetCachedImage`, `KnownToBeOpaque` and its cached image (`CustomTraverseSubresources` returns `false`, and `CopyForComputedStyle` doesn't copy the cache, until then).
- Not yet ported, needed later: the graphics `Path` (cached by `SVGPathByteStream` upstream), the rest of `SVGPathUtilities` (needs `Path` and `SVGPathSegList`), `parseKerningUnicodeString`/`parseGlyphName`.
- Slice 3 (colour), bottom-up: (1) Graphics/Color, (2) calc-free numeric primitives, (3a) Style-free calc, (3b) calc evaluation and Style calc and (3c) calc parsing are done. (4a) `LayoutUnit` and the point/size types and (4b) Style numerics are done. (5a) the CSS colour types, (5b) the colour parser and (5c) `ColorValue`/`ValuePool` (compile-only) are done. (6) Style colour is done (its CSSValue half is deferred, see below). Slice 3 is complete apart from the Value-dependent pieces.
- Deferred from layer 6 (needs `Style/BuilderChecking.hpp` and the raw-WebKit `ValueList`/`ValuePair`/`FunctionValue`/`BuilderState` it includes): `CSSValueConversion<Color>`, `CSSValueCreation<Color>`, `CSSValueConversion<DynamicRangeLimit>`, `CSSValueCreation<DynamicRangeLimit>` and `CSSValueConversion<Opacity>`. They come with the other `+CSSValueConversion`/`+CSSValueCreation` headers.
- Waiting on the property parser slice: `ValuePool::CreateFontFaceValue` (needs `ValueList`/`PropertyParser`) and `ParserFastPaths::MaybeParseValue` (its keyword fallback needs the generated `PropertyParsing.cpp`, which calls every consumer's `IsKeywordValidFor*`). The Style `+CSSValueConversion`/`+CSSValueCreation` headers (numerics, `LengthWrapper`, `PrimitiveKeyword`) are still raw WebKit.
- Style `ToCSS`/serialisation can't be tested until there's a real `RenderStyle` (the seam's can't be constructed).
- The Style seam (`Style/Seam.hpp`, `src/Style/Seam.cpp`) stands in for `RenderStyle`, `RenderView`, `RenderBox`, the font types, `Style::BuilderState`, `AnchorPositionEvaluator`, `ContainerQueryEvaluator` and the `Document` colour API. Each placeholder is replaced when its slice is ported; see the layer 3b and layer 6 sections. Since layer 6, `RenderStyle` can be constructed (colour state only); its other members are still unreachable.
- `ParserContext`: the `Document` and `Settings` constructors (FIXME in the header) wait for `Document::CssParserContext()` and a `Settings` port; so does the generated `PropertySettings(const Settings &)` constructor. `LocalFrame`'s `StaticValuePool::Init()` call comes with `LocalFrame`.
- `CSS/Values/Concepts.hpp` (`CSSValueConcepts.h`) compiles but isn't reviewed.
- URL, not yet done: the DOM `URL` interface and `URLSearchParams` (WebKit's `DOMURL`/`URLDecomposition`, tested by WPT `setters_tests.json`), `Document`'s URL/base URL (`Document::URL()` is a TODO), `CompleteURL(string, Document)`, and `applyModifiersToLoaderOptions` (needs the resource loader).
- Krystal.Core: the `IntrusivePtr` `Downcast`/`DynamicDowncast` overloads can't be called with an explicit target and `Downcast` rejects null (nothing calls them yet); `OffsetRotateValue.cpp` has a FIXME workaround.
- `CSS/Properties/Property.hpp` (`CSSProperty.h`) isn't reviewed, but compiles (the generated `PropertyId.cpp` defines its static functions); the colour parser and `AngleValidator` use `AcceptsQuirkyColor`/`AcceptsQuirkyAngle` from it.
- Tokenizer doesn't keep numeric original text or whitespace counts, so `TokenSerialisationMode::CustomProperty` behaves like `Normal` (FIXMEs in `src/CSS/Parser/Token.cpp`), and `ConsumeUnicodeRangeToken` rejects every range containing a number or dimension token (`U+0025-00FF`, `U+4??`; FIXME in `Consumers/UnicodeRange.cpp`, `[!shouldfail]` test). Decided 2026-10-13: fix both together once the tokenizer keeps original text.
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
| `wtf/URL.h`, `URLParser.h`, `URLHash.h` | `have` → `url::URL`, `url::URLParser` in `Krystal.Booey/URL/` | WHATWG URL, UTF-8 only; ICU (vcpkg `icu`) for IDNA. See *URL* under *Values*. |
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
- **Fixed (2026-10-11): keyword/property lookup was case-sensitive.** `Token` called the raw generated `FindValueKeyword`/`FindProperty`. Ported upstream's `cssValueKeywordID`/`cssPropertyID` (from `CSSPropertyParser.cpp`) as `ValueIdFromName`/`PropertyIdFromName` in `CSS/Parser/KeywordLookup.hpp`, including the `-apple-` → `-webkit-` keyword rewrite. `Token` caches the value keyword like upstream (`mutable` `uint16`, invalidated when the text changes); property lookups aren't cached, also like upstream. The remaining raw callers (`src/CSS/Parser.cpp`, `Consumers/Animations.cpp`, `Consumers/WillChange.cpp`) were switched over, and the commented-out originals in `PropertyParser.cpp` deleted, on 2026-10-13.
- **Fixed (2026-10-11): function tokens weren't block starts.** Added `Tokenizer::BlockStart(blockType, type, name)` (upstream's `blockStart(LeftParenthesisToken, FunctionToken, name)`) and a `BlockTokenType` parameter on `Token`'s text constructor; both function-token sites in `ConsumeIdentLikeToken` use it. Note: url tokens serialise through `SerializeIdentifier` like upstream, so `url(foo.png)` → `url(foo\.png)` (tokenizes back to the same URL). `tests/CSS/Parser/Parser.cpp` and `tests/CSS/Selectors/SelectorParser.cpp` stay disabled until their slices.

Pulled into the build as headers, unreviewed: generated `PropertyId.cpp` defines the `Property::` statics and `IsKeywordValidFor*Values`, so it needs `CSS/Properties/Property.hpp` → `CSS/Values/Value.hpp` (WebKit's `CSSPropertyNames.cpp` has the same dependency). `Value.hpp` got one fix to compile: `friend class RefCounted<Value>`, because `RefCounted<Value>::SubRef` must reach `Value`'s protected destroying `operator delete` (WebKit's `CSSValue::deref` is a member, so it never needed this). Generated `ValueId.cpp` no longer includes `ParserContext.hpp`/`Property.hpp` (unused; removed from `codegen/css_value_keywords.py`).

### Generated

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `CSSPropertyNames.h` | `have` | `codegen/css_properties.py` → `CSS/Properties/PropertyId.hpp` | Generated into `build/.../codegen`; checked-in copies deleted. |
| `CSSValueKeywords.h` | `have` | `codegen/css_value_keywords.py` → `CSS/Values/ValueId.hpp` | Same. |

---

## Values (slices 2 and 4) — done

`Value`, `PrimitiveValue` and every subclass upstream's `CSSValue::visitDerived` dispatches to are ported, built and instance-tested (2026-10-13), with the CSS types they hold. Graphs are in `tools/port/out/`, named after the class (`value.md`, `primitive-value.md`, `path-value.md`, ...). The colour values (`ColorValue`, `ColorSchemeValue`, `DynamicRangeLimitValue`) and `ValuePool` are under Slice 3, layer 5c.

**Approach (decided 2026-10-11 to 2026-10-13):** `Value.cpp` can't build until every subclass compiles (`VisitDerived`), so the subclasses were ported compile-only (generic containers first, then by complexity) and instance tests were added once `Value.cpp`/`PrimitiveValue.cpp` linked. Classes that need the Style builder or the loader are **hybrid**: everything is ported except what needs `style::Builder`/`BuilderState` or the resource loader, which is declared with a FIXME but not defined (listed under *Where we are*). `VisitDerived` only dispatches `CustomCSSText`, `Equals`, `CustomVisitChildren`, `AddHash`, `CustomMayDependOnBaseUrl`, `CustomTraverseSubresources` and `CustomSetReplacementUrlForSubresources`, none of which need the builder, so `StyleBuilderState.h` (~31 new files: selectors, `RuleSet`, `PropertyCascade`, media queries, `StyleProperties`) wasn't needed. `DeprecatedCSSOM*` is won't-port, so no class has `createDeprecatedCSSOMWrapper`.

### Base classes

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `css/CSSValue.h`, `.cpp` | `have*` | `CSS/Values/Value.hpp`, `src/CSS/Values/Value.cpp` | `First()`/`Second()`/`GetQuad()`/`GetRect()` declared here, defined by `ValuePair`/`QuadValue`/`RectValue` (`GetQuad`/`GetRect` because `Quad`/`Rect` would hide those types in subclasses). `add(Hasher&)` → `AddHash`/`AddDerivedHash` (`size_t &seed` with `Hash::Combine`). `SeparatorCSSText` returns `CSSOMStringView`. |
| `ClassType` (in `CSSValue.h`) | `have*` | `CSS/Values/ValueType.hpp` | Upstream order (the range checks depend on it); `ContainsValueList()` is `>= ValueList`. No `Invalid`/`ValueWithSmallList` enumerators (59 values, 6 bits). |
| `ValueSeparator` (in `CSSValue.h`) | `have` | `CSS/Values/ValueSeparator.hpp` | |
| `css/CSSPrimitiveValue.h`, `.cpp` | `have*` | `CSS/Values/PrimitiveValue.hpp`, `src/CSS/Values/PrimitiveValue.cpp` | Rewritten against upstream. |
| `css/CSSUnits.h`, `.cpp` | `have*` | `CSS/Values/Units.hpp`, `UnitType.hpp`, `UnitCategory.hpp` | Split into three headers; no `operator<<(TextStream&)`. |
| `css/values/primitives/CSSPrimitiveNumericTypes+Serialization.h`, `.cpp` (number formatting only) | `port` | `CSS/Values/Primitives/NumericTypesSerialisation.hpp`, `src/.../NumericTypesSerialisation.cpp` | `SerializableNumber`, `FormatCSSNumberValue`, `FormatNonfiniteCSSNumberValue`; WTF's `FormattedCSSNumber` (fixed, 6 decimals, trailing zeros trimmed, `-0` → `0`) on `std::to_chars`. |
| `css/values/CSSValueAggregates.h` | `have*` | `CSS/Values/Aggregates.hpp` | `*Vector` → `*List`, `ListOrNullopt` → `ListOrNull` (accessor `GetList()`). List wrappers use the container protocol names (`size`, `isEmpty`, `contains`, `containsIf`, `first`, `last`), their own API PascalCase (`IsNone`, `TryList`, `SwitchOn`); the `EnumSet` wrappers are PascalCase. `Markable` is optional-like (`TreatAsOptionalLike`). Not ported: `*RefCountedFixedVector` wrappers (needs `RefCountedFixedVector`), `supports_text_stream_insertion`. |
| `css/values/CSSValueConcepts.h` | `have?` | `CSS/Values/Concepts.hpp` | Compiles; not reviewed. |
| `css/parser/CSSParserContext.h`, `.cpp` | `have` | `CSS/Parser/ParserContext.hpp`, `src/CSS/Parser/ParserContext.cpp` | Upstream member order; flag members keep upstream's camelCase. `charset` → `utf8_stringview Charset`; `enclosingRuleType` → `Maybe<RuleType>`. `add(Hasher&, ...)` → `Hash()` (so `DefaultHash` comes from `MemberBasedHash`); `HashTraits` ported. The constructor calls `StaticValuePool::Init()`, like upstream. |
| `CSSPropertySettings` hasher (generated by `process-css-properties.py`) | `have` | `PropertySettings::Hash()` (`codegen/css_properties.py`) | |

**`Value` differences:**
- Reference counting uses `RefCounted<Value>` instead of upstream's own count; no `hasAtLeastOneRef()`. No `refCountFlagIsStatic`/`makeStatic()`: static values (`StaticValuePool`) rely on the pool's reference never being released.
- `operator delete` frees with `::operator delete` (no per-type `freeAfterDestruction`). No size `static_assert`.
- No free `add(Hasher&, const CSSValue&)` or `add(Hasher&, const CSSValueContainingVector&)` (the latter declared upstream, never defined); use `AddHash`.

**`PrimitiveValue` differences:**
- Strings are interned `StringAtom`s that are never freed, not reference-counted `StringImpl`s; string equality is atom identity.
- Non-converting accessors keep the Krystal name `Value<T>(…)`, which hides the base class name inside `PrimitiveValue`; refer to the base as `css::Value` there.
- `ResolveAsLength` is specialised for `int64/uint64/int32/uint32/int16/uint16/float/double/LayoutUnit` (upstream: `int`, `unsigned`, `short`, `unsigned short`, `float`, `double`, `LayoutUnit`).
- `CustomCSSText` serialises keywords with `ToLower(ValueId)` for upstream's `nameStringForSerialization`. `FormatIntegerValue` uses shortest round-trip `std::to_chars` (same as `Token`; FIXME to share it).
- `FloatValueDeprecated` returns `dom::ExceptionOr`.

**`ParserContext` differences:**
- No `Document`/`Settings` constructors (FIXME; see *Where we are*), so callers construct with a mode and set flags afterwards (designated initialisers no longer work). The `(ParserMode, URL)` constructor is `explicit` (upstream's is implicit).
- No `HAVE(CORE_ANIMATION_SEPARATED_LAYERS)`/`ENABLE(SERVICE_CONTROLS)` guards; `propertySettings.useSystemAppearance` (`HAVE(CORE_MATERIAL)`) isn't set for UA sheets (the generated `PropertySettings` has no such flag).
- `Hash()` mirrors upstream's quirks: bit 31 repeats `cssTextTransformMathAutoEnabled`, and `counterStyleAtRuleImageSymbolsEnabled`, `shouldIgnoreImportRules`, `webkitMediaTextTrackDisplayQuirkEnabled` and `enclosingRuleType` aren't hashed (still compared by `==`). Upstream hashes the fields twice; Krystal combines them once.
- `StrictParserContext()` uses `MainThreadNeverDestroyed`, like upstream.

### Subclasses

All on (in `KRYSTAL_BOOEY_CSS_SOURCES`), at `CSS/Values/<Name>.hpp` and `src/CSS/Values/<Name>.cpp`, verdict `have` unless noted. Upstream is `css/CSS<Name>.h`/`.cpp`.

| Krystal | Notes (renames and class-specific differences) |
|---|---|
| `ValuePair` | `createNoncoalescing` → `CreateNonCoalescing`. Defines `Value::First()`/`Second()`. Hashes the separator, coalesce flag, then both values (false if either hashes by pointer). |
| `ValueContainingList`, `ValueList` (`ValueList.hpp`) | `CSSValueContainingVector` → `ValueContainingList`. Items in a `SmallList<const Value *, 4>` (leaked references, released in the destructor) instead of an inline `std::array` plus `MallocSpan`; same ownership and inline capacity. Container protocol keeps upstream names (`size`, `operator[]`, `begin`/`end`), the rest PascalCase (`Length`, `Item`, `ItemWithoutBoundsCheck`, `ProtectedItem`, `HasValue`, `SerialiseItems`, `ItemsEqual`, `ContainsSingleEqualItem`, `CopyValues`). `create(char16_t, ...)` takes a `char32`. |
| `ValueListBuilder` | `CSSValueListBuilder` (in `CSSValueList.h`); `SmallList<Ref<Value>, 4>`. |
| `FunctionValue` | `nameLiteral(m_name)` → `ToString(_name)` (keeps the keyword's case, e.g. `translateX`). |
| `QuadValue` (+ `css/Quad.h` → `Quad.hpp`, `css/RectBase.h` → `RectBase.hpp`) | `quad()` → `GetQuad()`. `Quad(Ref<Value>)` is `explicit`. |
| `RectValue` (+ `css/Rect.h` → `Rect.hpp`) | `rect()` → `GetRect()`; `generateCSSString` → `GenerateCSSString` (private). |
| `RatioValue` (+ `css/values/primitives/CSSRatio.h`, `.cpp` → `Primitives/Ratio.hpp`, `src/.../Ratio.cpp`) | `ratio()` → `GetRatio()`; `numerator`/`denominator` → `Numerator`/`Denominator`. `CSSRatio.cpp` defines `ValueCreation<Ratio>`. |
| `BackgroundRepeatValue` | `xValue()`/`yValue()` → `XValue()`/`YValue()`. |
| `BorderImageSliceValue`, `BorderImageWidthValue` | `Slices()`/`Fill()`, `Widths()`/`OverridesBorderWidths()`. |
| `ReflectValue` | `Direction()`/`Offset()`/`Mask()`. Includes `Value.hpp` and forward-declares `ValueId` instead of including `PrimitiveValue.hpp`. |
| `UnicodeRangeValue` | `From()`/`To()`. `hex(x, Lowercase)` is a local `std::to_chars` helper; add a general one to `Text/` if another slice needs it. |
| `FontVariationValue`, `FontFeatureValue` | `tag()`/`value()` → `Tag()`/`GetValue()`. `Create` takes `FontTag` and `Ref<Value>` by value. `FontFeatureValue` calls `ResolveAsIntegerIfNotCalculated<int>()` explicitly (Krystal's default is `int64`). |
| `FontStyleWithAngleValue` | `obliqueAngle()` → `GetObliqueAngle()`. |
| `FontStyleRangeValue` | Public members `FontStyleValue`/`ObliqueValues`. |
| `FontValue` | Public members `Style`, `Variant`, `Weight`, `Width`, `Size`, `LineHeight`, `Family`. Includes `PrimitiveValue.hpp` (its `RefPtr<PrimitiveValue>` members must be complete where it's destroyed). |
| `GridLineNamesValue` | `names()` → `Names()`. `makeStringByJoining` is a local append lambda. |
| `GridLineValue` | `spanValue()`/`numericValue()`/`gridLineName()` and their `protected*` versions → PascalCase. |
| `SubgridValue`, `TransformListValue`, `GridAutoRepeatValue`, `GridIntegerRepeatValue` | On `ValueContainingList`. `AutoRepeatID()`, `Repetitions()`. The grid repeats' extra state isn't hashed, as upstream. |
| `CounterValue` | `Identifier`/`Separator`/`CounterStyle`/`CounterStyleCSSText`. `ASSERT_NOT_REACHED` → `krys_debug_assert_msg(false, ...)`, still returning an empty string. A null separator (`Consumers/Content.cpp` passes `CSSOMStringAtom::Null()`) serialises as empty. |
| `OffsetRotateValue` | `Modifier()`, `angle()` → `GetAngle()`, `IsInitialValue()`. `Create` downcasts the raw pointer and re-adopts it (FIXME; Core's `IntrusivePtr` `Downcast`, see *Where we are*). Doesn't include `ValuePool.hpp` (upstream includes it for `StringBuilder` only). |
| `AttrValue` | |
| `PositionValue`, `PositionXValue`, `PositionYValue` (one file) | `position()` → `GetPosition()`. Single-argument constructors `explicit`. |
| `UrlValue` | `CSSURLValue`; Krystal name kept. `url()` → `URL()`. |
| `GridTemplateAreasValue` | `areas()` → `Areas()`, `stringForRow` → `StringForRow`, which fills a `List<const CSSOMString *>` (null for an unnamed cell) instead of a `FixedVector<String>`. |
| `EasingFunctionValue` | `easingFunction()` → `GetEasingFunction()`. |
| `BoxShadowPropertyValue`, `TextShadowPropertyValue` | `shadow()` → `Shadow()`. |
| `PathValue` | `path()` → `GetPath()`. |
| `BasicShapeValue` | `shape()` → `GetShape()`. |
| `FilterValue`, `AppleColorFilterValue` | `filter()` → `GetFilter()`. |
| `RayValue` | `ray()` → `GetRay()`, `coordinateBox()` → `CoordinateBox()`. `toCSSValueID(CSSBoxType)` is a local `ToValueId` (FIXME: `PrimitiveValueMappings.hpp` once reviewed); `BoxMissing` debug-asserts and returns `None`. |
| `ScrollValue`, `ViewValue` | Headers include `Value.hpp`, the `.cpp`s `PrimitiveValue.hpp`. |
| `CustomPropertyValue` | `createWithCSSWideKeyword` → `CreateWithGlobalKeyword`, `isCSSWideKeyword`/`tryCSSWideKeyword` → `IsGlobalKeyword`/`TryGlobalKeyword`. `value()` → `Value()` (hides the base class; the class uses `css::Value`). `Tokens()` uses `get_if` (Krystal's `SwitchOn` returns by value). The string/CSS text caches use `empty()` for `isNull()`, so an empty serialisation isn't cached. |
| `VariableReferenceValue` | `have` (Builder-free half). `CacheSimpleReference` only caches an ident name (upstream: any token; the parser only creates valid `var()`/`env()`, and the general path gives the same result). |
| `PendingSubstitutionValue` | `have` (Builder-free half). Header-only like upstream; the `.cpp` holds the `ResolveValue` FIXME. `emptyString()` → `{}`. |
| `FontFaceSrcLocalValue`, `FontFaceSrcResourceValue` (`FontFaceSrcValue.hpp`) | `have` (loader-free half). `cssTextFromFontTech` → `CssTextFromFontTech` (constexpr); `FontTechnology` lives here. `isEmpty`/`fontFaceName` → `IsEmpty`/`FontFaceName`; `fontLoadRequest` → `GetFontLoadRequest`, `svgFontFaceElement` → `GetSVGFontFaceElement`. No cached font (`CustomTraverseSubresources` returns `false`) or SVG element weak pointer, so no out-of-line destructor. |
| `NamedImageValue`, `CanvasValue`, `PaintImageValue`, `ImageSetValue`, `CrossFadeValue`, `FilterImageValue`, `GradientValue` | `have` (Builder-free half). No cached `StyleImage` members, so no out-of-line destructors; `CSSGradientValue`'s private copy constructor (copies the cache) isn't ported. `ImageSetValue` is on `ValueContainingList`. `CSSCrossfadeValue` → `CrossFadeValue` (matches `ValueType::CrossFade`). `PaintImageValue` keeps upstream's FIXME (arguments aren't serialised). |
| `ImageSetOptionValue` | `type()`/`setType()` → `Type()`/`SetType()`; the MIME type is `Maybe<CSSOMString>` for the nullable `String` (`type("")` is written when empty but present; not escaped, as upstream). The `create` asserts use `IsImageValue()`. |
| `ImageValue` | `have` (loader- and Builder-free half). `url()` → `URL()`, `cachedImage()` → `GetCachedImage()`; `create(WTF::URL)` takes `url::URL`. No cached image (`CustomTraverseSubresources` returns `false`; `CopyForComputedStyle` doesn't copy it). The initiator type defaults to `CSSOMStringAtom::Null()` (`StringAtom` has no default constructor). |
| `CursorImageValue` | `have` (Builder-free half). `originalURL()` → `OriginalURL()`. Includes `ValuePair.hpp` directly (upstream gets it through `StyleBuilderState.h`). |

`StyleImage`, `StyleCursorImage`, `CachedImage`, `CachedResourceLoader`, `RenderElement` and `ResourceLoaderOptions` are forward-declared in `krys::boo` (alongside `CachedResource`); `style::BuilderState` is the seam's. `FontLoadRequest`/`SVGFontFaceElement` are forward-declared in `css`, as `Consumers/Font.hpp` does.

**Differences common to the subclasses:**
- `Create` and constructors take their arguments by value (upstream: `&&`, or `&&`/`const &` overload pairs), so callers can pass a `Ref<Derived>` copy.
- An accessor that would hide a type is `Get<Name>()` (`GetQuad`, `GetPosition`, `GetFilter`, ...).
- `AddDerivedHash` only where upstream has one (`PrimitiveValue`, `ValuePair`, `ValueContainingList`, `FunctionValue`); every other class hashes by pointer, as upstream.
- `arePointingToEqualData` → `CompareValue`/`CompareValuePtr` or inlined (`a.get() == b.get() || *a == *b`); `wtf/PointerComparison.h` isn't ported.
- `nameLiteralForSerialization` → `ToLower(ValueId)`.
- `friend class Value` (for `AddDerivedHash` via `VisitDerived`) instead of `friend CSSValue::addHash`.

### Supporting types

Ported with the values that hold them. On and tested unless noted.

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/graphics/FontTaggedSettings.h` | `port` | `Graphics/FontTaggedSettings.hpp` | `fontFeatureTag` → `FontFeatureTag`; `tag()`/`value()`/`enabled()` → `Tag()`/`Value()`/`Enabled()`; the list keeps container names. `add(Hasher&, FontTag)` → `FourCharacterTagHash::Hash` (same big-endian integer); setting hash → `DefaultHash<FontTaggedSetting<T>>`; no `DefaultHash<FontTag>` (it would cover every `Array<char, 4>`). `operator<` and `FontTaggedSettings::hash()` not ported (declared upstream, never defined). |
| `platform/graphics/FontTaggedSettings.cpp`, `rendering/style/GridArea.cpp`, `wtf/URLHelpers.h`, `.cpp` | `wont` | | `TextStream` dumping; URL display helpers. |
| `css/values/grid/CSSGridTemplateAreas.h` | `have` | `CSS/Values/Grid/GridTemplateAreas.hpp` | `map` → `Map`. |
| `css/values/grid/CSSGridNamedAreaMap.h`, `.cpp` | `have` | `CSS/Values/Grid/GridNamedAreaMap.hpp`, `src/...` | `Map`/`RowCount`/`ColumnCount`, `AddRow`; the `Map` alias is `MapType`. `std::unordered_map` (iteration order differs from WTF's `HashMap`; nothing depends on it); `ensure` → `try_emplace`. |
| `rendering/style/GridArea.h`, `GridSpan.h`, `.cpp` | `have` | `Rendering/Style/GridArea.hpp`, `GridSpan.hpp`, `src/Rendering/Style/GridSpan.cpp` | Namespace `boo`. `columns`/`rows` → `Columns`/`Rows`, `span()` → `Span()`; `GridSpan` methods PascalCase except `begin`/`end`. |
| `style/values/grid/StyleGridTrackSizingDirection.h`, `StyleGridPositionSide.h` | `have`/`port` | `Style/Values/Grid/GridTrackSizingDirection.hpp`, `GridPositionSide.hpp` | `boo::style`. |
| `style/values/grid/StyleGridPosition.h`, `.cpp` | `have` | `Style/Values/Grid/GridPosition.hpp`, `src/...` | `boo::style`. Methods PascalCase (`Max`, `SetMaxPositionForTesting`, which can't be reset, as upstream). `Position` alias → `PositionType`. Not ported: `add(Hasher&, GridPosition)` (needs `DefaultHash<StringAtom>`), `CSSValueConversion<GridPosition>` (deferred with the other conversions), logging. |
| `css/values/easing/CSS{,CubicBezier,Linear,Spring,Steps}EasingFunction.h` | `have` | `CSS/Values/Easing/*.hpp` | Members PascalCase (`Value`, `Stops`, `Mass`, ...); `switchOn` → `SwitchOn`; steps `Kind`'s template parameter `Keyword` → `TKeyword`. `Stop::Length::Extra` is `Maybe` (upstream `std::optional`). `std::numeric_limits` → `Limits`. |
| `css/values/borders/CSSBoxShadow.h`, `CSSBoxShadowProperty.h` | `have` | `CSS/Values/Borders/BoxShadow.hpp`, `BoxShadowProperty.hpp` | Members PascalCase; `IsWebkitBoxShadow` defaults to `false` (upstream uninitialised). |
| `css/values/text-decoration/CSSTextShadow.h`, `CSSTextShadowProperty.h` | `have` | `CSS/Values/TextDecoration/TextShadow.hpp`, `TextShadowProperty.hpp` | Members PascalCase. |
| `css/values/borders/CSSBorderRadius.h`, `.cpp` | `have` | `CSS/Values/Borders/BorderRadius.hpp`, `src/...` | `DefaultValue()`, `TopLeft()`.., `HasDefaultValue`. |
| `css/values/shapes/CSSFillRule.h` | `have` | `CSS/Values/Shapes/FillRule.hpp` | |
| `css/values/shapes/CSSPathFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/PathFunction.hpp`, `src/...` | `Path::Data` stays `PathData`; members `FillRule`/`Data`/`ByteStream`. |
| `css/values/shapes/CSS{Circle,Ellipse,Inset,Polygon,Xywh,Shape}Function.h`, `.cpp`, `CSSBasicShape.h` | `have` | `CSS/Values/Shapes/*.hpp`, `src/...` | Aliases that clash with a member get a `Type` suffix (`InsetsType`, `VerticesType`, `LocationType`/`SizeType`, `AnchorType`, `CommandsType`). `ShapeFunction.cpp` shares the `to`/`by`, control point and curve serialisers through local templates (same output). |
| `css/values/shapes/CSSRectFunction.h`, `.cpp` | `have` | `CSS/Values/Shapes/RectFunction.hpp`, `src/...` | `CSS::Rect` → `RectShape` (upstream's `WebCore::Rect` and `CSS::Rect` are both `css::Rect` here). |
| `css/values/filter-effects/CSS{Blur,Brightness,Contrast,Grayscale,HueRotate,Invert,Opacity,Saturate,Sepia,DropShadow,AppleInvertLightness}Function.h`, `CSSFilterReference.h` | `have` | `CSS/Values/FilterEffects/*.hpp` | Members PascalCase; `FilterReference::url` → `Url`. `apple-invert-lightness()` has no leading dash, as upstream. |
| `css/values/filter-effects/CSSFilter.h`, `CSSAppleColorFilter.h` | `have` | `CSS/Values/FilterEffects/Filter.hpp`, `AppleColorFilter.hpp` | `CSS::FilterValue` → `FilterData`, `CSS::AppleColorFilterValue` → `AppleColorFilterData` (clash with the value classes). `CSSFilterFunction.h` isn't ported (only reached through `DeprecatedCSSOMFilterFunctionValue.h`). |
| `css/values/motion/CSSRayFunction.h`, `.cpp` | `have` | `CSS/Values/Motion/RayFunction.hpp`, `src/...` | Members PascalCase. |
| `rendering/style/RenderStyleConstants.h` | `port` on demand | `Rendering/Style/RenderStyleConstants.hpp` | `CSSBoxType` only, in `krys::boo`. Add the other enums as needed. |
| `css/CSSVariableData.h`, `.cpp` | `have*` | `CSS/Values/VariableData.hpp`, `src/...` | Krystal tokens own their text, so no backing string (`m_backingString`, `updateBackingStringsInTokens`, `tryUseStringLiteralBacking`). `List<Token>`; `serialize()` → `Serialise()`. |
| `css/CSSProperty.h` | `have?` | `CSS/Properties/Property.hpp` | Not reviewed. `property.value()` is a reference (upstream: pointer), which matters when `PendingSubstitutionValue::ResolveValue` is defined. |
| `css/values/images/CSSGradient.h`, `.cpp` | `have` | `CSS/Values/Images/Gradient.hpp`, `src/...` | Members PascalCase; clashing aliases get a `Type` suffix (`GradientLineType`, `GradientBoxType`, `SizeType`, `ColorType`/`PositionType`). `Method`/`DefaultMethod`/`LegacyMethod`. `CommaSeparatedVector<Stop, 2>` → `CommaSeparatedList<Stop, 2>`. The `.cpp`'s unused Style includes aren't carried over. |
| `css/values/primitives/CSSPosition.h`, `.cpp` | `have` | `CSS/Values/Primitives/Position.hpp`, `src/...` | `IsCenterPosition`/`Split`/`SwitchOn`; members `Offset`/`Value`; `CSS::Keyword::X` → `keywords::X`. The forwarding constructors are constrained to `Kind`'s arguments other than the wrapper itself (upstream's unconstrained ones beat the copy constructor for non-const lvalues). |

**URL** (`wtf/`; decisions 2026-10-12: ICU through vcpkg for IDNA, UTF-8-only query encoding, `url::URL` in `URL/URL.hpp` with `CSSURL` holding one):

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `wtf/URL.h`, `.cpp` | `port` | `URL/URL.hpp`, `src/URL/URL.cpp` | Methods and free functions PascalCase (`string()` → `String()`, `stringCenterEllipsizedToLength` → `StringCenterEllipsisedToLength`, `AboutBlankURL`, `ProtocolIs`, ...). |
| `wtf/URLParser.h`, `.cpp` | `port` | `URL/URLParser.hpp`, `src/URL/URLParser.cpp` | Every state and helper; one UTF-8 `CodePointIterator`. `serialize` → `Serialise`, `maybeCanonicalizeScheme` → `MaybeCanonicaliseScheme`. |
| `wtf/URLHash.h` | `port` | `URL/URLHash.hpp` | |
| `wtf/text/CodePointIterator.h` | `port` | Krystal.Core `Text/CodePointIterator.hpp` | UTF-8 only; adds `DecodeUTF8`/`IsValidUTF8`. |
| `css/values/primitives/CSSURL.h`, `.cpp` | `have` | `CSS/Values/Primitives/URL.hpp`, `src/...` | `CSSURL`; members `Specified`/`Resolved`/`Modifiers`. |
| `css/values/primitives/CSSURLModifiers.h`, `.cpp` | `have` | `CSS/Values/Primitives/URLModifiers.hpp` | Members PascalCase. `applyModifiersToLoaderOptions` not ported (FIXME). |
| `loader/LoadedFromOpaqueSource.h` | `port` | `Loader/LoadedFromOpaqueSource.hpp` | |

URL differences:
- UTF-8 strings. WTF's null `String` is an `_isNull` bit on `URL`; null string views stand in for null `StringView` in the getters, `SetQuery` (null removes the query) and `URL(base, relative)` (null relative gives the base without its fragment). Invalid UTF-8 input reads as U+FFFD per invalid byte. `User()`/`Password()` return an empty string for invalid UTF-8 after decoding (upstream null).
- No `URLTextEncoding`: queries are always UTF-8 percent-encoded; `parseAllowingC0AtEnd`'s sentinel encoding pointer is a `bool`. IDNA uses `uidna_nameToASCII_UTF8` (vcpkg ICU 78.3, Unicode 17).
- The scheme length field is 24 bits (upstream 26), to make room for the null and deleted-value bits.
- `RemoveQueryParameters` passes an empty remaining query as null, so the `?` is removed (as upstream's empty `StringBuilder`).
- Not ported: `isolatedCopy`, `fakeURLWithRelativePart`, the default-port-for-testing registry, `print`/`dump`/`TextStream`, CF/NS/GLib conversions.
- `CSSURL` serialisation always uses the specified URL (no replacement URLs or `shouldUseResolvedURLInCSSText`). `CompleteURL` has no WebVTT check (no WebVTT `ParserMode`), so it never returns null; no `Document` overload.

**Geometry** (`Graphics/`, `Core/`; ported for `PathValue`'s arcs and all of `AffineTransform`). None has `operator<<`/`TextStream`, JSON, IPC coders, TZone allocation or platform (CG/Skia/Cairo/Win/SIMD) conversions.

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `platform/FloatConversion.h` | `port` | `Core/FloatConversion.hpp` | Other types are a deleted template; no CGFloat variants. |
| `platform/BoxExtents.h` | `port` | `Core/BoxExtents.hpp` | `ToLayoutBoxExtent`. |
| `platform/graphics/DoubleSize.h`, `.cpp`, `DoublePoint.h`, `.cpp` | `port` | `Graphics/DoubleSize.hpp`, `DoublePoint.hpp`, `src/...` | `FloatSize`/`FloatPoint`/`LayoutPoint` gained their `Double*` conversions. |
| `platform/graphics/FloatPoint3D.h`, `.cpp` | `port` | `Graphics/FloatPoint3D.hpp`, `src/...` | `normalize` → `Normalise`. |
| `platform/graphics/transforms/Quaternion.h`, `.cpp` | `port` | `Graphics/Transforms/Quaternion.hpp`, `src/...` | Members `X`/`Y`/`Z`/`W`; `Slerp`/`Accumulate`/`Interpolate` are `const`. |
| `platform/graphics/IntRect.h`, `.cpp`, `IntRectHash.h` | `port` | `Graphics/IntRect.hpp`, `IntRectHash.hpp`, `src/...` | No `area()` (no `Checked`); `IsValid`/`ToRectWithExtentsClippedToNumericLimits` use `int64` sums. |
| `platform/graphics/FloatRect.h`, `.cpp` | `port` | `Graphics/FloatRect.hpp`, `src/...` | `ContainsMode` is an enum class; `normalizeRect` → `NormaliseRect`. |
| `platform/graphics/LayoutRect.h`, `.cpp` | `port` | `Graphics/LayoutRect.hpp`, `src/...` | `UnionRect(Span<const LayoutRect>)`. |
| `platform/graphics/FloatQuad.h`, `.cpp` | `port` | `Graphics/FloatQuad.hpp`, `src/...` | `P1()`..`P4()`; list helpers take `Span`s. |
| `platform/graphics/GeometryUtilities.h`, `.cpp` | `port` | `Graphics/GeometryUtilities.hpp`, `src/...` | `NormaliseAngleInRadians`; `euclidianDistance` keeps its spelling; `Min4` etc. constexpr; `Vector` → `Span`. |
| `platform/graphics/Region.h`, `.cpp` | `port` | `Graphics/Region.hpp`, `src/...` | `SmallList` storage; `Span` members `Y`/`SegmentIndex`; `krys::Span` written qualified inside `Region`. `Bounds()` uses `int64` differences. |
| `platform/graphics/transforms/AffineTransform.h`, `.cpp` | `port` | `Graphics/Transforms/AffineTransform.hpp`, `src/...` | `span()` → `AsSpan()`; `identity` → `AffineTransform::Identity`. `Blend` keeps upstream's doubling of components for non-`Replace` composite operations. |
| `platform/graphics/transforms/TransformationMatrix.h`, `.cpp` | `port` | `Graphics/Transforms/TransformationMatrix.hpp`, `src/...` | Portable path only (no NEON/SSE2). `M11()`.., `type()` → `GetType()`, `Identity`; decomposition members PascalCase. |

**SVG path pipeline** (`SVG/Internals/`, `boo::svg`):

| WebKit | Verdict | Krystal | Notes |
|---|---|---|---|
| `svg/SVGPathSeg.h` (`SVGPathSegType`) | `port` (partial) | `SVGPathSegType.hpp` | The enum only. |
| `svg/SVGPathConsumer.h`, `SVGPathSource.h` | `port` | same names | Enum classes (`NormalizedParsing` → `NormalisedParsing`); segment members PascalCase. No weak-pointer bases; the parser holds references. |
| `svg/SVGParserUtilities.h`, `.cpp` | `port` (partial) | same names | Parsers consume a `dom::DOMStringView&`; `parseNumber(StringView)` → `ParseNumberFromString`. Not ported: `parseKerningUnicodeString`, `parseGlyphName`. |
| `svg/SVGPathByteStream.h` | `port` | same name | Bytes held by value (upstream copy-on-write) and no cached platform `Path` (FIXME). `bytes()` → `GetBytes()`. The reader asserts the remaining size. |
| `svg/SVGPathByteStream{Source,Builder}`, `SVGPathStringViewSource`, `SVGPathStringBuilder`, `SVGPathParser` (`.h`, `.cpp`) | `port` | same names | |
| `svg/SVGPathUtilities.h`, `.cpp` | `port` (partial) | same name | `BuildSVGPathByteStreamFromString`, `BuildStringFromByteStream` only; the rest need `Path`/`SVGPathSegList` (FIXME). |
| `wtf/text/StringConcatenateNumbers.h` (`FormattedNumber::fixedPrecision`), `wtf/dtoa.h` (`numberToFixedPrecisionString`) | `port` | `FormatNumberFixedPrecision` in `Krystal.Booey/Core/NumberFormatting.hpp` | ECMAScript `toPrecision` rules on `std::to_chars`. |
| `wtf/MathExtras.h` (`piOverTwoFloat`) | `port` | Krystal.Core `Maths/Base.hpp` | |

### Upstream bugs found

- `genericParseNumber` returns `false` from a function returning `std::optional<float>` when the exponent is too large, so `1e39` parses as 0. Krystal fails.
- `FloatQuadTests`' `ExpectRotatedRect` computes its comparisons and drops them. Krystal checks them, with the trapezoid's size corrected to (26.833, 13.416).
- Kept as upstream: `ParserContext::Hash()`'s quirks and `AffineTransform::Blend` (see above).

### Krystal.Core changes made here

`EnumSet` storage (`unsigned_of_size_t<Bytes>` added to `TypeTraits.hpp`, sized from the highest value, ≤ 64 values); `#pragma once` added to `EnumSet.hpp`, `AllowCompactPointers.hpp`, `GetPtrHelper.hpp`, `UnalignedAccess.hpp`, `ZipTuple.hpp`; `Hash::Combine`'s private helper renamed `CombineInto` (an lvalue `size_t` seed picked it); `StringAtom::Storage()`; `Text/Unicode.hpp` includes `Text/ASCII.hpp`; `text::IsASCIIAlphaCaselessEqual`; `Text/CodePointIterator.hpp`; `LazyNeverDestroyed` asserts it was constructed before access and isn't constructed twice (debug builds, with `IsConstructed()`, as upstream) and is non-copyable (tested in `Krystal.Core.Tests/tests/Utils/NeverDestroyed.cpp`). Generated `DefaultHash<ValueId>`/`DefaultHash<PropertyId>` define `Hash`/`Equal` on the enum.

### Loose ends

- `StringAtom::View()` (Krystal.Core) dereferences null for `Null()`; `CounterValue` works around it, and `ImageValue`'s default initiator type would hit it in `LoadImage`.
- Krystal.Core's text tests (`tests/Text/*`) aren't in `Krystal.Core.Tests/CMakeLists.txt`; only `CodePointIterator.cpp` is.
- Unreviewed files still on the old or raw names: `Consumers/Image.cpp` (raw WebKit: `CSSGradientValue::create`, `CSSCrossfadeValue`, lowercase gradient members, `CSSVariableData::create` at `:1198`); `Consumers/Font.hpp` (`ConsumeFontTech` returns `SmallList<FontTechnology>`; `ConsumeFontFaceSrc` will need a `List`); `Consumers/Grid.hpp` (own `GridNamedAreaMapRow` alias); `Consumers/Shapes.cpp` (stub); `PrimitiveValueMappings.hpp` (raw `CSSFontFaceSrcValue.h`); `CSS/Font/FontFace.hpp` and `Style/BuilderState.hpp` (raw `FontTaggedSettings.h`); `Style/Values/Primitives/Position.hpp` (`x.offset`); `Style/Values/Primitives/CoordinatedValueList.hpp` (`PointerComparison`).

### Tests

- Every subclass: `tests/CSS/Values/<Name>.cpp` (one per source file), expectations from the upstream `.cpp`s, with `Krystal.Booey.Tests/CSS/ValueTestUtils.hpp` (`ValueFixture` calls `StaticValuePool::Init()`; `CssText`, `ToNarrow`, `Children`). Each covers `Create`, `Is<X>Value`, the accessors, every branch of `CustomCSSText`, `Equals` (equal and each field differing), `AddHash`, `VisitChildren`, and dependencies/base URL/subresources where customised. Only `PendingSubstitutionValue` has none (nothing but the deferred `ResolveValue`).
- Base and supporting types: tests mirror the source paths (`tests/CSS/Values/{Units,NumericTypesSerialisation,PrimitiveValue,Aggregates}.cpp`, `tests/CSS/Parser/ParserContext.cpp`, `tests/CSS/Values/{Primitives,Grid,Easing,Borders,TextDecoration,Shapes,FilterEffects,Motion,Images}/`, `tests/Rendering/Style/GridSpan.cpp`, `tests/Style/Values/Grid/GridPosition.cpp`, `tests/Graphics/`, `tests/SVG/Internals/SVGPath.cpp`, `tests/Core/NumberFormatting.cpp`, `tests/URL/`, Krystal.Core `tests/Text/CodePointIterator.cpp`). `GridPosition` tests reset `SetMaxPositionForTesting` to 1000000.
- Upstream tests converted: `TestWebKitAPI/Tests/WTF/URL.cpp` (except `URLDataURIStringSharing`, `PrintStream`, `IsolatedCopy`; `URLParser.cpp`'s cases are covered by the WPT data), and `IntRectTests`, `FloatRectTests`, `FloatQuadTests`, `RegionTests`, `AffineTransform`, `TransformationMatrix` (gtest → Catch2 script; minus `Casting`/`EnclosingIntRect` and `area()`). Region compares structurally (upstream compares `TextStream` dumps); the fuzz test uses a fixed seed.
- `tests/URL/WPT.cpp` runs WPT's `urltestdata.json`, `toascii.json` and `IdnaTestV2.json` (copied to `data/url/` from WebKit's `LayoutTests/imported/w3c/web-platform-tests/url/resources`). Three `IdnaTestV2.json` cases expect a V7 failure for U+32931/U+32B9A, which Unicode 17 assigned; the test expects them to be valid.
- Full Booey suite (2026-10-13): 1348 cases / 671304 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

## Property parser — in progress

Entry: `css/parser/CSSPropertyParser.cpp` (headers-only graph: 151 files, `tools/port/out/property-parser.md`). Most of that graph comes through `StyleBuilder.h`, `StyleCustomProperty.h` and `StylePrimitiveNumericTypes+CSSValueConversion.h`, which only the typed custom-property functions need.

**Plan (decided 2026-10-13):** `PropertyParser.cpp` can't link until the generated `PropertyParsing.cpp` does, and that includes `PropertyParserCustom.hpp` and calls every consumer (~43 `Consumers/*.cpp`, ~15k upstream lines; `Font.cpp` and `Shapes.cpp` are stubs, `Syntax.cpp` is missing). So, in layers, with the Style-dependent pieces declared and FIXME'd (the hybrid approach used for the Values):
- **a.** Parser plumbing: `Property`, `ParsedPropertyList`, `PropertyParserResult`, `PropertyShorthand` (+ generated shorthand functions), `CustomPropertySyntax`, `VariableParser`. Done.
- **b.** The consumers, in groups, compile-only first (as with the Value subclasses). Groups, by upstream include order (proposed 2026-10-13):
  - **b1.** Shared foundations: the remaining `*Definitions.hpp`, `MetaResolver`, `PrimitiveValue` (`CSSPrimitiveValueResolver`), `List`, `Percentage`, `String`. Done.
  - **b2.** Ident/keyword-level consumers: `Align`, `Anchor`, `Attr`, `Box`, `Display`, `Inline`, `PositionTry`, `Ratio`, `ScrollSnap`, `Scrollbars`, `Syntax` (was missing), `Transitions`, `UnicodeRange`, `URL`, `ViewTransition`, `WillChange`. Done.
  - **b3.** Numeric/structured consumers: `Animations`, `CounterStyles`, `Easing`, `Grid`, `Lists`, `Position`, `SVG`, `TextDecoration`, `Timeline`, `Transform`.
  - **b4.** Image-dependent consumers: `Image`, then `Background`, `Content`, `Filter`, `Masking`, `Motion`, `UI`.
  - **b5.** The stubs: `Font`, `Shapes` (~2k upstream lines).
- **c.** `PropertyParserCustom.hpp`, generated `PropertyParsing.cpp`, `PropertyParser.cpp`; then tests parsing properties from strings, and `ParserFastPaths::MaybeParseValue`/`ValuePool::CreateFontFaceValue`.

### Layer a: parser plumbing (2026-10-13)

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSProperty.h`, `.cpp` | `have*` | `CSS/Properties/Property.hpp`, `src/.../Property.cpp` | on | tested. The statics are defined by the generated `PropertyId.cpp`. See differences. |
| `ParsedPropertyVector` (in `CSSProperty.h`) | `have*` | `CSS/Properties/ParsedPropertyList.hpp` | on | `SmallList<Property, 63>` (upstream inline capacity 256). |
| `css/parser/CSSPropertyParserResult.h`, `.cpp` | `have` | `CSS/Properties/PropertyParserResult.hpp`, `src/...` | on | tested. See bugs. |
| `css/StylePropertyShorthand.h`, `.cpp` | `have` | `CSS/Properties/PropertyShorthand.hpp`, `src/.../PropertyShorthand.cpp` | on | tested. `length()` → `Size()`; `indexOfShorthandForLonghand` → `IndexOfShorthandForLonghand`. No `containerSize`. |
| `StylePropertyShorthandFunctions.h`, `.cpp` (generated) | `have` | `codegen/css_properties.py` → `PropertyShorthandFunctions.hpp`/`.cpp` | on | tested through `PropertyShorthand`. |
| `css/parser/CSSCustomPropertySyntax.h`, `.cpp` | `have` | `CSS/Parser/CustomPropertySyntax.hpp`, `src/...` | on | tested. The `.cpp` was missing. |
| `css/parser/CSSVariableParser.h`, `.cpp` | `have` (Style-free half) | `CSS/Parser/VariableParser.hpp`, `src/...` | on | tested. The `.cpp` was missing. |
| `isCustomPropertyName` (in `CSSPropertyParser.cpp`) | `have` | `IsCustomPropertyName` in `CSS/Parser/KeywordLookup.hpp` | on | tested. Moved from `PropertyParser.hpp` next to the other `CSSPropertyParser.cpp` lookups, so `VariableParser` can link before layer c. |

**Bugs fixed:**
- `PropertyParserResult::AddPropertyForAllLonghandsOfShorthand` copied the value with `ShareRef(*value)`, dereferencing null when a shorthand sets all its longhands to the implicit initial value (upstream: `value.copyRef()`).
- The generated `IsExposed` overloads were inside the anonymous namespace in `PropertyId.cpp` (only `IsExposedNotInvalidAndNotInternal` is static upstream), so nothing outside could link to them (`PropertyParserResult`, the generated `PropertyParsing.cpp`). Fixed in `codegen/css_properties.py`.
- `CustomPropertySyntax`: `Parse`/`ConsumeType` weren't static, `Universal()` returned the definition list instead of a `CustomPropertySyntax`, and nothing was defined.
- `VariableParser`: nothing was defined; `ParseDeclarationValue` returned `RefPtr<Value>` (upstream `RefPtr<CSSCustomPropertyValue>`).
- `TransitionShorthandForParsing` was declared but not defined; `LonghandsForShorthand` was declared, never defined or used (removed). `IndexOfShorthandProperty` used the raw `assert`.

**Added (upstream API):** `Property::operator==` (and `PropertyMetadata`'s), `Property::IsDirectionAwareProperty`, `Property::IsListValuedProperty`, `TransitionShorthandForParsing`, `CustomPropertySyntax::{Parse, ConsumeType, ContainsUnknownType}`, `VariableParser::{ContainsValidVariableReferences, ParseDeclarationValue, IsValidVariableName}`.

**Differences:**
- `PropertyMetadata` is a `PropertyFlag` set (`Important`, `Implicit`, `SetFromShorthand`) plus a 3-bit shorthand index (upstream: bitfields, 2-bit index). `Property`'s constructor takes `(id, value, important, implicit, setFromShorthand, shorthandIndex)` (upstream: `(id, value, important, isSetFromShorthand, index, implicit)`); the strong booleans catch a mix-up. `Value()` returns a reference; no `protectedValue()`.
- Not ported (not generated by `css_properties.py`): `colorProperties`/`physicalProperties` and `isColorProperty`, `animationUsesNonAdditiveOrCumulativeInterpolation`, `animationUsesNonNormalizedDiscreteInterpolation`, `animationIsAccelerated`, `allAcceleratedAnimationProperties` (the last two need `Settings`). They come with animations.
- `CustomPropertySyntax`: `Definition` is a `List` (alias `DefinitionType`); `Component`'s members are `Type`/`Multiplier`/`Ident`, declared with qualified types. Parses UTF-8 directly (no `StringParsingBuffer`/`readCharactersForParsing`); the type-name table is a linear search (no `SortedArrayMap`). `type(` is matched by `FunctionId() == ValueId::Type` (case-insensitive, like upstream's `equalLettersIgnoringASCIICase`).
- `VariableParser`: `ClassifyBlock`'s stack is a `List` (upstream `std::stack`); `ParseInitialValueForUniversalSyntax` is declared, not defined (FIXME; needs `style::CustomProperty`).

Tests: `tests/CSS/Properties/{Property,PropertyShorthand,PropertyParserResult}.cpp`, `tests/CSS/Parser/{CustomPropertySyntax,VariableParser}.cpp`: 9 cases / 309 assertions pass; `IsCustomPropertyName` added to `tests/CSS/Parser/KeywordLookup.cpp`. Full Booey suite: 1358 cases / 671619 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

### Layer b1: shared consumer foundations (2026-10-13)

From the include graph of the consumer `.cpp`s: every other consumer includes some of these. Paths under `css/parser/CSSPropertyParserConsumer+` (WebKit) and `CSS/Properties/Consumers/` (Krystal).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `IntegerDefinitions.h` | `have` | `IntegerDefinitions.hpp` | on | tested. See bugs. |
| `LengthDefinitions.h` | `have` | `LengthDefinitions.hpp` | on | tested. See bugs. |
| `LengthPercentageDefinitions.h` | `have` | `LengthPercentageDefinitions.hpp` | on | tested. |
| `AnglePercentageDefinitions.h` | `have` | `AnglePercentageDefinitions.hpp` | on | tested. |
| `TimeDefinitions.h`, `ResolutionDefinitions.h`, `FlexDefinitions.h`, `FrequencyDefinitions.h` | `have` | `TimeDefinitions.hpp`, `ResolutionDefinitions.hpp`, `FlexDefinitions.hpp`, `FrequencyDefinitions.hpp` | on | tested. |
| `MetaResolver.h` | `have` | `MetaResolver.hpp` | on | tested through `PrimitiveValueResolver`. |
| `CSSPrimitiveValueResolver.h` | `have` | `PrimitiveValue.hpp` (`PrimitiveValueResolver`, `PrimitiveValueResolverBase`) | on | tested. See bugs. |
| `List.h` | `have` | `List.hpp` | on | tested. `ListBounds`' members are `Min`/`Max`, `size_t` (upstream `unsigned`). |
| `Percentage.h`, `.cpp` | `have` | `Percentage.hpp`, `src/.../Percentage.cpp` | on | tested. Upstream's FIXME (calc() percentages aren't divided by 100) is kept, with a `[!shouldfail]` test. |
| `String.h`, `.cpp` | `have` | `String.hpp`, `src/.../String.cpp` | on | tested. |

**Bugs fixed:**
- `IntegerDefinitions.hpp` used the old `CalcAllowedSymbols` (header no longer exists) and the raw `assert`, so it didn't compile.
- `LengthValidator` didn't reject the quirky em unit (`__qem`) outside UA sheets, and didn't accept unitless lengths for the quirky-length properties in quirks mode (`Property::AcceptsQuirkyLength`).
- `PrimitiveValueResolverBase`: resolving a calc value hit `krys_not_implemented()` (now `PrimitiveValue::Create(value.ProtectedCalc())`), and integers were created as plain numbers (now `CreateInteger`).

**Differences:**
- `PercentageConsumer` takes the percentage unit as a template argument (layer 3c); the definitions pass it explicitly.

Tests: `tests/CSS/Properties/Consumers/{Definitions,PrimitiveValue,List,Percentage,String}.cpp`: 19 cases / 225 assertions pass (1 expected failure). Full Booey suite: 1377 cases / 671844 assertions; all pass except 3 expected failures.

### Layer b2: ident/keyword-level consumers (2026-10-13)

Paths under `css/parser/CSSPropertyParserConsumer+` (WebKit) and `CSS/Properties/Consumers/`, `src/CSS/Properties/Consumers/` (Krystal). Each is a `.h`/`.cpp` pair.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `Align`, `Anchor`, `Box`, `Display`, `Inline`, `PositionTry`, `Ratio`, `ScrollSnap`, `Scrollbars`, `Transitions`, `ViewTransition`, `WillChange` | `have` | same names | on | tested. Matched upstream (apart from the bugs below). |
| `Attr` | `have` | `Attr` | on | tested. See bugs. |
| `Syntax` | `have` | `Syntax` | on | tested. The `.cpp` was missing. |
| `UnicodeRange` | `have` | `UnicodeRange` | on | tested. See bugs and differences. |
| `URL` | `have` | `URL` | on | tested. See bugs. |

**Bugs fixed:**
- `UnicodeRange`: the `?` loop never advanced the buffer, so any range with a `?` failed once it passed six digits; `ConsumeUnicodeRangeToken` discarded an extra token after the range (upstream only skips whitespace); and number/dimension tokens were read with `IdentCodePoints()`, which asserts on those token types.
- `URL`: `url("")` and `integrity("")` were rejected, because an empty `ConsumeStringRaw` result was taken to mean "no string" (upstream checks for a null view). Now checks the token type first.
- `Attr` didn't compile: it built a `CSSOMStringAtom` for `AttrValue::Create`, which takes a `CSSOMString` (upstream's atom converts implicitly).
- `Box`/`ViewTransition` called the inline `PrimitiveValue::Create(ValueId)` without including `ValuePool.hpp`, which defines it.
- Raw `assert` in `Align`, `Anchor`, `Display` (now `krys_debug_assert`).

**Differences:**
- `UnicodeRange`: numeric tokens have no original text, so ranges containing them fail (see *Where we are*); delimiters are matched with `Token::Delimiter()`.
- `AllowedURLModifiers` is a flags enum with a `None` value (upstream: `OptionSet`).

Tests: one file per consumer under `tests/CSS/Properties/Consumers/`, using the new `ConsumeToText` helper (`Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp`): 30 cases / 436 assertions pass (3 expected failures). Full Booey suite: 1407 cases / 672280 assertions; all pass except 4 expected failures.

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
| `css/parser/CSSParserContext.h` | `have*` | `CSS/Parser/ParserContext.hpp` | on | Only the fields the parser reads were checked. No `baseURL`, `charset`, `enclosingRuleType`, `hasDocumentSecurityOrigin`, `loadedFromOpaqueSource`, constructors, hashing or `strictCSSParserContext()`. Flag members keep upstream's camelCase (as does the generated `PropertySettings`). Superseded: see *Values*. |
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
- `PropertyParserState::Pool` is a `ValuePool *` defaulting to `nullptr` instead of `CSSValuePool& pool { CSSValuePool::singleton() }` (TODO in the code); it goes back to a reference once `ValuePool` builds (layer 5). Done 2026-10-13.
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
| `platform/graphics/DoublePoint.h`, `DoubleSize.h`, `FloatRect.h`, `IntRect.h`, `transforms/*` | | | Ported with `PathValue` (see *Values*). |
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
| `css/parser/CSSPropertyParserConsumer+Color.h`, `.cpp` | `have` | `Consumers/Color.hpp`, `src/.../Color.cpp` | on | tested. The `.cpp` was a stub; now ported. `CSSColorParsingOptions` → `ColorParsingOptions`, `allowedColorTypes` → `AllowedColorTypes`. The internal `ColorParserState`/nester and helpers are in an anonymous namespace; `normalize*` → `Normalise*`. `consumeColor` (Value) and `consumeDynamicRangeLimit` (Value) were in `ColorValueConsumers.cpp` until 2026-10-13 (see layer 5c). |
| `css/parser/CSSPropertyParserConsumer+ColorInlines.h` | `port` | `Consumers/ColorInlines.hpp` | on | tested. |
| `css/parser/CSSPropertyParserConsumer+ColorInterpolationMethod.h`, `.cpp` | `have` | `Consumers/ColorInterpolationMethod.hpp`, `src/.../ColorInterpolationMethod.cpp` | on | tested. The `.cpp` was missing. The hue method table is a `switch` (no `SortedArrayMap`, as in 3c). |
| `css/parser/CSSPropertyParserConsumer+ColorAdjust.h`, `.cpp` | `have` | `Consumers/ColorAdjust.hpp`, `src/.../ColorAdjust.cpp` | on | tested. `consumeColorScheme` (Value) was in `ColorAdjustValueConsumers.cpp` until 2026-10-13. No `DARK_MODE_CSS` guard. |
| `css/values/color-adjust/CSSColorScheme.h`, `.cpp` | `have` | `CSS/Values/ColorAdjust/ColorScheme.hpp`, `src/.../ColorScheme.cpp` | on | tested. The `.cpp` (serialiser) was missing. |
| `css/parser/CSSParserFastPaths.h`, `.cpp` (colour half, `lengthValueRangeForPropertiesSupportingSimpleLengths`) | `have` | `CSS/Parser/ParserFastPaths.hpp`, `src/CSS/Parser/ParserFastPaths.cpp` | on | tested. The `.cpp` was missing. UTF-8 only, so no 8-/16-bit split; the WTF span helpers (`skipWhile`, `skipExactly`, …) are local helpers on `CSSOMStringView`. `charactersToDouble` → `std::from_chars` after skipping leading whitespace (same grammar as WTF's fast_float: no leading `+`, accepts `inf`/`nan`). `maybeParseValue` and the length/transform/display/opacity fast paths wait for the Values slice (`MaybeParseValue` is declared, not defined; TODO). |
| `css/parser/CSSPropertyParserConsumer+SymbolDefinitions.h`, `+Symbol.cpp` | `port` | `Consumers/SymbolDefinitions.hpp`, `src/.../Symbol.cpp` | on | tested. `validatedRange` → `ValidatedRange`. |
| `css/parser/CSSPropertyParserConsumer+AngleDefinitions.h` | `have` | `Consumers/AngleDefinitions.hpp` | on | tested. See bugs. |
| `css/parser/CSSPropertyParserConsumer+PercentageDefinitions.h`, `+KeywordDefinitions.h` | `have` | `Consumers/PercentageDefinitions.hpp`, `KeywordDefinitions.hpp` | on | tested through the colour parser. Match upstream. |
| `css/parser/CSSParserContext.cpp` (`strictCSSParserContext`) | `port` | `StrictParserContext()` in `CSS/Parser/ParserContext.hpp` | on | A function-local static instead of `MainThreadNeverDestroyed`. Superseded: see *Values*. |
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
| `css/parser/CSSPropertyParserConsumer+Color.cpp`, `+ColorAdjust.cpp` (Value-returning consumers) | `have` | `src/.../ColorValueConsumers.cpp`, `ColorAdjustValueConsumers.cpp` | on (compile-only) | Written in 5b. Stay separate from `Color.cpp`/`ColorAdjust.cpp` until `Value.cpp` builds, or the colour parser tests would pull in the pool. Merged back and deleted 2026-10-13: `ConsumeColor` (Value) follows `ConsumeUnresolvedColor` in `Color.cpp`, `ConsumeDynamicRangeLimit` (Value) ends it, and `ConsumeColorScheme` (Value) ends `ColorAdjust.cpp`, in upstream's order (tested in `Consumers/Color.cpp`/`ColorAdjust.cpp`). |
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
- `PropertyParserState::Pool` stays a pointer: defaulting it to `MainThreadPool()` would link `ValuePool.cpp` (and with it `PrimitiveValue`'s constructors) into every parser test. Done 2026-10-13: `ValuePool &` defaulting to `ValuePool::MainThreadPool()`, as upstream, and `PropertyParserState.hpp` includes `ValuePool.hpp`.

Tests: none at the time (instance tests came 2026-10-13; see *Values*). Full Booey suite: 812 cases / 652886 assertions; all pass except 2 expected failures (`[!shouldfail]`). Whole solution builds with no warnings.

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
