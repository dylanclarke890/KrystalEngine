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

## Where we are (2026-10-14)

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
- Property parser, layer b3 (2026-10-13): the numeric/structured consumers built; all but `Timeline`, `Animations` and `CounterStyles` tested (those three wait for the generated `PropertyParsing.cpp`, layer c).
- Property parser, layer b4 (2026-10-13): the Image-dependent consumers built; `Image`, `Filter` and `UI` tested, `Background`, `Content`, `Masking` and `Motion` compile-only until layer c (and b5 for the shapes).
- Property parser, layer b5 (2026-10-13): `Font` (and the generic family names) built and tested; `Shapes` built, compile-only until layer c. Layer b is done.
- Property parser, layer c1 (2026-10-13): the generator reviewed against upstream's output (the build passes `ENABLE_DARK_MODE_CSS`/`ENABLE_VARIATION_FONTS`; 41 legacy `-webkit-` properties are dropped on purpose), `PropertyParserCustom.hpp` reviewed (8 behaviour bugs fixed), and the generated `PropertyParsing.cpp` built, linked and tested through the shorthands.
- Property parser, layer c2 (2026-10-13): `Timeline`, `Animations`, `CounterStyles`, `Background`, `Content`, `Masking`, `Motion` and `Shapes` tested; three `Background` consumers that only served dropped `-webkit-` properties are removed.
- Property parser, layer c3 (2026-10-13): `PropertyParser`, the value half of `ParserFastPaths` and `ValuePool::CreateFontFaceValue` built and tested; properties parse end-to-end from strings. The property parser slice is complete apart from the Style-dependent pieces under *Open*.
- Sheet parser, layer d1a (2026-10-13): pseudo-class/pseudo-element codegen (`codegen/css_pseudo_selectors.py`) generates the enums, the name maps, `UserAgentParts` and the `Selector` inlines; standard pseudos only. Maps and parts built and tested.
- Sheet parser, layer d1b (2026-10-13): `Selector`, `SelectorList`, `MutableSelector` (and `SelectorParserContext`, pulled forward from d1c) completed against upstream, built and tested, with specificity, hashing and full selector serialisation; selectors are built through `MutableSelector` until the parser lands.
- Sheet parser, layer d1c (2026-10-13): the selector parser (`SelectorParser`), with the namespace half of `StyleSheetContents`, `ParserEnum` and `style::PseudoElementIdentifier`, built and tested; selectors parse and serialise end to end from strings, including nesting resolution. The selector layer (d1) is done.
- Sheet parser, layer d2 (2026-10-13): declaration blocks (`StyleProperties`, `MutableStyleProperties`, `ImmutableStyleProperties`, `ShorthandSerialiser`, `PropertyInitialValues`) built and tested; declaration blocks serialise with shorthand collapsing. `Property.hpp` reviewed; Krystal.Core `Packed.hpp` fixed and tested. The `CSSParser`-dependent setters wait for d5 and the CSSOM wrapper half for the CSSOM slice.
- Sheet parser, layer d3 (2026-10-13): media and container query parsing and serialisation (`MediaQueryParser`, `ContainerQueryParser`, the shared condition grammar and feature parser, the feature schemas, `MediaQueryParserContext`) built and tested against the WPT parsing and serialisation cases. Evaluation is deferred; `CSSSupportsParser` moved to d5 and `MediaList` to the CSSOM slice.
- Sheet parser, layer d4 (2026-10-13): the rule model (`Rule` and every rule type, including the internal keyframe, counter style, view transition and position try rules upstream keeps in CSSOM headers), `AtRuleId`, the counter style descriptors, `FontFeatureValues`/`FontPaletteValues` and the parse side of `StyleSheetContents` built and tested. `@import` doesn't load; `@-internal-base-appearance` is dropped and `StyleSheet` moved to the CSSOM slice.
- Tokenizer, original text (2026-10-13): numeric tokens keep their original text and whitespace tokens their whitespace, so `TokenSerialisationMode::CustomProperty` serialises them as written, tokens compare by original text, and `<unicode-range>` accepts ranges with numbers (`U+0025-00FF`, `U+4??`). See *Slice 1*.
- Sheet parser, layer d5 (2026-10-13): `CSSParser` (`Parser`) and `CSSSupportsParser` (`SupportsParser`) built and tested; whole style sheets, single rules, keyframes, declaration lists and `@supports` conditions parse end to end. `MutableStyleProperties`' string setters, `ParseDeclaration` and `StyleSheetContents::ParseString` are defined, and the old parser is deleted. The sheet parser slice is done.
- CSSOM (2026-10-14): `CSSStyleSheet`, `StyleSheet`, `MediaList`, `CSSRule`/`CSSRuleList` and every rule wrapper, `CSSStyleDeclaration`/`CSSStyleProperties`, the descriptor classes, `Rule::CreateCSSOMWrapper` and `StyleSheetContentsCache` built and tested, connected to the DOM through the Style seam. Sheets are read and changed through the CSSOM (`insertRule`, `replaceSync`, `style.setProperty`, ...) with copy-on-write of shared contents. See the *CSSOM* section.
- Media query evaluation (2026-10-14): `GenericMediaQueryEvaluator`, `MediaQueryEvaluator` and every media feature's evaluation built and tested, against a host-filled `MediaEnvironment` (view size, media type, screen, pointing devices, user preferences); `style::Ratio` ported. Container query evaluation and `MediaQueryMatcher`/`matchMedia` are deferred. See the *Media query evaluation* section.
- DOM integration, layer e0 (2026-10-14): `DocumentOrShadowRoot`, `Element::OnAttributeChanged`, the parser's parsing-children hooks, quirks mode queries and a minimal `html::EventLoop`; six DOM bugs fixed (attribute old values, mutation records, `NodeFlags`, `relList`, slot names). See *DOM integration*.
- DOM integration, layer e1 (2026-10-14): `Document` URL, base URL (`<base>`), `CompleteURL` and a cached `CssParserContext()` that follows quirks mode; the `Document` constructors of `ParserContext`, `SelectorParserContext` and `MediaQueryParserContext`; the seam's URL and parser context functions removed. See *DOM integration*.
- DOM integration, layer e2 (2026-10-14): `StyledElement` (now the base of `HTMLElement`, `SVGElement` and a real `MathMLElement`), the `style` attribute's change steps, `InlineCSSStyleProperties` (`Style()`), `StyleAttributeMutationScope`, `Parser::ParseInlineStyleDeclaration` and lazy `style` attribute synchronisation; a cloning bug fixed. See *DOM integration*.
- DOM integration, layer e3 (2026-10-14): `style::Scope` (owned by `DocumentOrShadowRoot`): candidate nodes in tree order, pending sheets, preferred/alternate style sheet sets, active style sheets, updates batched on the event loop; LinkStyle `Sheet()` on `<style>`/`<link>`; the document's connected shadow roots. Three DOM bugs fixed (shadow trees never connected, shadow roots over-released, descendants over-retained). See *DOM integration*.
- DOM integration, layer e4 (2026-10-14): `<style>` elements own their sheets (`InlineStyleSheetOwner`, `CSSStyleSheet::CreateInline`, the owner-node half of `StyleSheetContents`), with `type`, `media`, `title`, `disabled` and `load` events; `document.styleSheets`/`shadowRoot.styleSheets` (`StyleSheetList`). Krystal.Core `StringAtom::View()` no longer crashes on the null atom. See *DOM integration*.
- DOM integration, layer e5 (2026-10-14): `adoptedStyleSheets` on documents and shadow roots (`CSSStyleSheetObservableArray` as a C++ list API with the constructed-sheet checks), adopting tree scopes on `CSSStyleSheet` (mutations reach every adopting scope), adopted sheets activated after owned sheets and listed in `styleSheets`. Two upstream bugs fixed (replaced and truncated sheets stayed adopted). See *DOM integration*.
- DOM integration, layer e6 (2026-10-14): selector matching (`SelectorChecker` and its state test functions, the `:has()` match element from `RuleFeature`) and `SelectorQuery`/`SelectorQueryCache`; `querySelector()`, `querySelectorAll()`, `matches()` and `closest()` work. State pseudo-classes follow the HTML spec where the DOM has the state and match nothing otherwise (FIXMEs). Three DOM bugs fixed (custom element names, the initial custom element state, empty namespaces) and one upstream bug (`|*` fast path). See *DOM integration*.
- DOM integration, layer e7 (2026-10-14): `matchMedia()` on `Document` (`MediaQueryMatcher`, `MediaQueryList`, `MediaQueryListEvent`); `change` events when the host calls `EvaluateMediaQueriesAndReportChanges()`. The seam's `RenderStyle` has the initial font, so font-relative units resolve in media queries. See *DOM integration*.
- Style resolution, layer f0 (2026-10-14): rule sets (`RuleData`, `RuleSet`, `RuleSetBuilder`, `RuleFeature`, `ScopeRuleSets`), the rule set half of `Style::Resolver` driven by `Style::Scope` (created on demand, shared between shadow trees, updated when sheets change), the user agent sheets (cleaned copies, embedded by codegen; every rule parses) and `UserAgentStyle`, counter styles (`CounterStyle`, `CounterStyleRegistry`), the selector filters, the `:has()` match cache and style relations in `SelectorChecker`; Krystal.Core `BloomFilter`. See *Style resolution*.
- Style resolution, layer f1 (2026-10-14): matching (`ElementRuleCollector`, `MatchResult`, `PageRuleCollector`, `PseudoElementRequest`): an element's (or pseudo-element's) matched declaration blocks per origin, in cascade order (scope, layer, specificity, `@scope` proximity, position), with inline style, `:host`, `::slotted()`, `::part()` and `@scope`; `@page` rules per page. `MatchResultCache` moved to f5. Two DOM bugs fixed (closed shadow roots were created open; internal code used the web-exposed `shadowRoot`/`assignedSlot` getters). See *Style resolution*.
- Style resolution, layer f2 (2026-10-14): the cascade (`PropertyCascade`): the winning declaration of each property, custom property and logical group property from a match result, by origin, importance, scope, layer, position and link state, with the included property types (inherited, non-cacheable, after animation, ...), the position fallback origin and the rollback cascades that `revert`/`revert-layer` use; `AnimatableCSSProperty`. `CustomProperty` moved to f3; the registry, `@property` and `var()` resolution moved to f4. The first half of style resolution (f0 to f2) is done. See *Style resolution*.
- Style resolution, layer f3a (2026-10-14): computed style foundations: the seam's `BuilderState` can be constructed (upstream's state for converting values), `BuilderChecking`, the generic, keyword, numeric, `LengthWrapper`, colour, `DynamicRangeLimit` and `Opacity` CSS value conversions and creations, Style `Position`, `URL` and `CoordinatedValueList`, `PrimitiveValueMappings` (every mapping whose type exists), `RenderStyleConstants` in full and the platform enums and data (`TextFlags`, `TextSpacing`, `UnicodeBidi`, `ScrollTypes`, `StyleAppearance`, `ThemeTypes`, `TouchAction`, `TabSize`, `GraphicsTypes`, `WindRule`, `ScrollAxis`). Computed values convert from parsed CSS, and create and serialise back. See *Style resolution*.
- Style resolution, layer f3b1 (2026-10-14): the small keyword and length groups (`box`, `sizing`, `flexbox`, `position`, `css2`, `display`, `break`, `multicol`, `page`, `math`, `rhythm`, `fill-stroke`, `contain`, `overflow`, `pointerevents`, `speech`, `view-transitions`, `will-change`, `counter-styles`, `lists`, `color-adjust`, `scrollbars`, `viewport`'s `Zoom`) with their conversions, creations, serialisations and blending, and their primitive value mappings. See *Layer f3b1*.
- Style resolution, layer f3b2 (2026-10-14): `align`, `inline`, `text`, `scroll-snap`, `ui` (but `Cursor`, moved to f3e) and `grid` (but `GridPositionsResolver`, moved to layout), the alignment data types, and their mappings. See *Layer f3b2*.
- Style resolution, layer f3c (2026-10-14): fonts: `FontSelectionAlgorithm`, `FontSizeAdjust`, `FontPalette`, `FontMetrics`, `FontDescription`, `FontCascadeDescription`, `SystemFontDatabase`, `LocaleToScriptMapping`, a `FontCascade` shell (no font data) that replaces the seam's font stand-ins, and the `fonts` group. See *Layer f3c*.
- Style resolution, layer f3d1 (2026-10-14): `TimingFunction` (with `UnitBezier` and `SpringSolver`), the `easing`, `animations`, `transitions` and `scroll-animations` groups (but the runtime timeline lists), `Animation` and `Transition` on the coordinated value lists, and `ScopedName` completed. See *Layer f3d1*.
- Style resolution, layer f3d2 (2026-10-14): the platform transform operations and the `transforms` group (with its functions), blending without an interpolation context (FIXMEs). See *Layer f3d2*.
- Style resolution, layer f3d3 (2026-10-14): the platform filter operations, `filter-effects` and `anchor-position`, and the position-try fallback in the seam (self-alignment flips again). `Path` moved to f3e. See *Layer f3d3*.
- Style resolution, layer f3e1 (2026-10-14): the `StyleImage` hierarchy (values only: no loading, clients or drawing), the platform gradient and image orientation, `images`, `content` and the cursor, and `BuilderState::CreateStyleImage`. See *Layer f3e1*.
- Style resolution, layer f3e2 (2026-10-14): the rounded rects, `backgrounds`, `borders`, `text-decoration`, the non-standard values still in `css-properties.json`, and `MaskMode`. See *Layer f3e2*.
- Style resolution, layer f3e3 (2026-10-14): the path data model, the SVG path builder, blender and absolute converter, `DataRef` and `TinyLRUCache`, and `shapes`. See *Layer f3e3*.
- Style resolution, layer f3e4 (2026-10-14): path operations and motion paths, `masking`, `motion` and `svg` values. See *Layer f3e4*.
- Style resolution, layer f3f1 (2026-10-14): the border, outline and counter value classes, `PositionTryOrder`, and the custom property storage (`CustomProperty`, `CustomPropertyData`). See *Layer f3f1*.
- Style resolution, layers f3f2 and f3f3 (2026-10-14): the computed style property generator, the `rendering/style` data groups, `style/computed` and `NameScope`. See *Layer f3f2 and f3f3*.
- Style resolution, layer f3f4 (2026-10-15): the render style property generator and `RenderStyle`, which replaces the seam's. See *Layer f3f4*.
- Style resolution, layer f4a (2026-10-15): f4 planned in four sublayers; the builder state (`BuilderState`, replacing the seam's) and the font size functions. See *Layer f4: the builder (planned)* and *Layer f4a*.
- Style resolution, layer f4b (2026-10-15): the generated builder (`BuilderGenerated`, its generator, `BuilderCustom`). See *Layer f4b*.
- Style resolution, layer f4c (2026-10-15): registered custom properties (`CustomPropertyRegistry`, `@property`, the typed custom property parsing). See *Layer f4c*.
- Style resolution, layer f4d (2026-10-15): `Builder`, `var()` resolution and `revert`/`revert-layer`; the builder state's test hook is gone. f4 is complete. See *Layer f4d*.
- Style resolution, layer f5a (2026-10-15): f5 planned in four sublayers; the style adjuster (`Adjuster`, applied by the builder too), `Style::Change`, `ResolvedStyle`, replaced elements and the containment functions. See *Layer f5: the resolver (planned)* and *Layer f5a*.

**Open / next candidates:**
- **Next:** style resolution, layer f5b (element resolution: the resolver's `styleFor*` functions and caches; see *Layer f5: the resolver (planned)*).
- Settings, noted with f0 (2026-10-14): parser contexts have no `Settings`, so features behind a settings flag that's on by default upstream are off in author sheets (`@view-transition` via crossDocumentViewTransitionsEnabled, `@position-try` via cssAnchorPositioningEnabled, `text-decoration-line: spelling-error | grammar-error`, ...). User agent sheets turn on upstream's list, which doesn't include the `text-decoration-line` flag (as upstream).
- DOM, not yet done (noted 2026-10-14, deferred by choice): `NodeAlgorithms::Equals` compares attributes by position and ignores extra attributes on the other element (the spec compares the counts and matches attributes regardless of order). `Document::Head()` returns null for parsed documents (`DOMTreeAccessors::GetHeadElement`); URL reflection (`dom::USVString` is a placeholder). only `classList` follows its attribute; `relList`, `sandbox`, `sizes`, `blocking` and `htmlFor` are parsed once when created and should update from `Element::OnAttributeChanged`. Mutation observers never deliver records: `QueueMutationObserverMicrotask`/`NotifyMutationObservers` are TODOs and can now be built on `html::EventLoop`.
- DOM, not yet done (noted with f1, 2026-10-14): `TreeQueries::NextShadowIncluding` visits a host's children before its shadow root and, when the host has children, never visits the shadow root (shadow-including tree order visits the shadow root right after its host). Shadow roots have no part mappings (`exportparts`), so `::part()` never reaches beyond the host's scope (FIXMEs in `SelectorChecker` and `ElementRuleCollector`). Documents have no focused area, so `:focus`/`:focus-visible` never match (FIXMEs).
- CSSOM, not yet ported (DOM integration): `<link>` sheets (resource loader) and the async `replace()` (promises). Also not yet ported: the Typed OM (`styleMap`) and the computed style declaration.
- Sheet parser, not yet ported (d5): the inspector entry points (`parseDeclarationListForInspector`, `parseStyleSheetForInspector`) are `wont`.
- Rule model, not yet ported (d4): `@import` loading (`ImportRule::RequestStyleSheet`, the cached sheet and its client) and the loader half of `StyleSheetContents` (`ParseAuthorStyleSheet`, `NotifyLoadedSheet`, `SubresourcesAllowReuse`, `IsLoadingSubresources`) come with the resource loader (its owner-node half came with e4 of the DOM integration); `RuleData::MaximumSelectorComponentCount` came with the rule set (f0).
- Query evaluation, not yet ported: container queries (`style/ContainerQueryEvaluator`, the container feature schemas' `evaluate`, FIXME in `src/CSS/Queries/ContainerQueryFeatures.cpp`) come with style resolution; font-relative units in media queries use the default style's font (`Resolver::DefaultStyleForElement`, the 16px `medium` font, since f3f4; a `FontCascade` without font data) until f5 resolves the user agent root style. `MediaEnvironment` is process-wide until documents have frames (FIXME). 
- Not yet ported, needed later: the `Style::Extractor` overload of `SerialiseShorthandValue` and `FontSelectionValueInlines.hpp` (still raw WebKit; `FontSelectionValue` came with f3c).
- Property parser, not yet ported (`Font`): the string parsers behind the `FontFace` interface and the canvas 2D context's `font` (`ParseUnresolvedFont`, `ParseFontFaceSrc`, `ParseFontFaceSizeAdjust`, `ParseFontFaceUnicodeRange`, `ParseFontFaceDisplay`, `ParseFontFaceFontStyle`, `ParseFontFaceFeatureSettings`, `ParseFontFaceFontWidth`, `ParseFontFaceFontWeight`; FIXME in `Consumers/Font.hpp`). They need `ScriptExecutionContext`/`Document` (quirks mode, value pool) and six of them the generated `PropertyParsing`.
- Property parser, not yet ported: `parseEasingFunctionDeprecated` (`CSSPropertyParserConsumer+Easing`; string → `TimingFunction`, used only by Web Animations; needs `TimingFunction` and `Style::createTimingFunctionDeprecated`) and `parseTransformRaw` (`CSSPropertyParserConsumer+Transform`; string → `Style::Transform`, used only by Web Animations; needs `Style::Transform`, `BuilderState` and the generated `consumeTransform`). Come with animations. Also `parseFilterValueListOrNoneRaw` (`CSSPropertyParserConsumer+Filter`; string → `Style::Filter`, used only by the canvas 2D context's `filter`; needs `Style::Filter`, `BuilderState` and `Document`); comes with canvas.
- Property parser, deferred definitions (need `style::CustomProperty`/`BuilderState`): `VariableParser::ParseInitialValueForUniversalSyntax`, `PropertyParser::ParseTypedCustomPropertyValue`/`ParseTypedCustomPropertyInitialValue`.
- Values, deferred definitions (declared, not defined, FIXME'd; define when `style::Builder`/`BuilderState` and the resource loader are ported): `VariableReferenceValue::ResolveVariableReferences`, `ResolveSingleValue`, `ResolveTokenRange`, `ResolveVariableReference`, `ResolveVariableFallback`, `TryResolveSimpleReference` (and so `ResolveAndCacheValue`); `PendingSubstitutionValue::ResolveValue` (also needs `PropertyParser::ParseValue` and `ParsedPropertyVector`); `FontFaceSrcResourceValue::GetFontLoadRequest` and its cached font (`CustomTraverseSubresources` returns `false` until then); `FontFaceSrcLocalValue::GetSVGFontFaceElement`/`SetSVGFontFaceElement` (need SVG fonts); `CreateStyleImage` on the ten image values (needs `style::BuilderState` and `StyleImage`, and the cached style image members of `NamedImageValue`/`CanvasValue`/`GradientValue`); `ImageValue::IsPending`, `LoadImage`, `GetCachedImage`, `KnownToBeOpaque` and its cached image (`CustomTraverseSubresources` returns `false`, and `CopyForComputedStyle` doesn't copy the cache, until then).
- Not yet ported, needed later: the graphics `Path` (cached by `SVGPathByteStream` upstream), the rest of `SVGPathUtilities` (needs `Path` and `SVGPathSegList`), `parseKerningUnicodeString`/`parseGlyphName`.
- The Style seam (`Style/Seam.hpp`, `src/Style/Seam.cpp`) stands in for `RenderStyle`, `RenderView`, `RenderBox`, `Style::BuilderState`, `AnchorPositionEvaluator`, `ContainerQueryEvaluator` and the `Document` colour (`StyleResolver` went with f0), URL, parser context and security API. Each placeholder is replaced when its slice is ported; see the layer 3b and layer 6 sections. Since layer 6, `RenderStyle` can be constructed (colour state; zoom and, since e7, the font are the initial values; since f3a, the writing mode, pseudo-element and unit-usage flags); its other members are still unreachable. Since f3a, `BuilderState` can be constructed too (see *Layer f3a*).
- `ParserContext`: the `Settings` constructor waits for a `Settings` port, and so does the generated `PropertySettings(const Settings &)` constructor; the `Document` constructors use default settings (FIXME). `LocalFrame`'s `StaticValuePool::Init()` call comes with `LocalFrame`.
- `CSS/Values/Concepts.hpp` (`CSSValueConcepts.h`) compiles but isn't reviewed.
- URL, not yet done: the DOM `URL` interface and `URLSearchParams` (WebKit's `DOMURL`/`URLDecomposition`, tested by WPT `setters_tests.json`), and `applyModifiersToLoaderOptions` (needs the resource loader).
- Krystal.Core: the `IntrusivePtr` `Downcast`/`DynamicDowncast` overloads can't be called with an explicit target and `Downcast` rejects null (nothing calls them yet); `OffsetRotateValue.cpp` has a FIXME workaround.
- Not yet ported, needed later: `StringBuilder`, `RefCountedFixedVector`, `DefaultHash<StringAtom>` (Krystal.Core; for `add(Hasher&, const Style::GridPosition&)`).
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
| `css/parser/CSSParserToken.h` | `have*` | `CSS/Parser/Token.hpp`, `src/CSS/Parser/Token.cpp` | on | compiles. Reviewed against upstream 2026-10-11. Numeric data in `Parser/NumericValue.hpp` instead of `CSSPrimitiveValue`. Spec token set: no `Comment`/`*Match`/`Column` tokens (they're `Delim`s), one `Whitespace` token, plus `UnicodeRange`. Not ported: `unitType()`/`stringToUnitType` (needs `CSSUnits`, Values slice), `convertToDimensionWithUnit`/`convertToPercentage` (tokenizer builds these directly), string-literal backing (`hasStringBacking`/`tryUseStringLiteralBacking`; tokens own their text). Numeric and whitespace tokens keep their original text (2026-10-13; see below). |
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
- **Fixed (2026-10-13): no original text.** Numeric tokens didn't keep their original text and whitespace tokens didn't keep their whitespace, so `TokenSerialisationMode::CustomProperty` serialised like `Normal` (custom property values lost repeated whitespace and number spelling, `2.100` → `2.1`), tokens compared numbers by value, and `<unicode-range>` rejected every range containing a number or dimension token. Added `Token::OriginalText()` (and its setter) and `InputStream::Slice()`; the tokenizer records the text of number, percentage (without the `%`), dimension (with the unit as written) and whitespace tokens. Upstream's `whitespaceCount` and newline tokens become the whitespace text: in `CustomProperty` mode tabs serialise as spaces and newlines are kept, and in `Normal` mode a whitespace token serialises as one space per upstream token (a run of spaces and tabs, and each newline), so `a \n b` → `a   b` as upstream. Equality compares the original text (whitespace with tabs as spaces). Differences: upstream keeps a dimension's original text only when its unit has no escapes and the number is shorter than 16 characters (otherwise it serialises the value and compares by value); Krystal always keeps it, escapes included, so `U+0\66` is rejected in both but `1.0\70x` serialises as written (upstream: `1px`). Tests: `tests/CSS/Parser/Token.cpp` (`OriginalText`, `Serialise(custom property)`, whitespace and equality cases), the `<unicode-range>`, `style()` and `SetCustomProperty` tests that were `[!shouldfail]`.
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
| `style/values/grid/StyleGridPosition.h`, `.cpp` | `have` | `Style/Values/Grid/GridPosition.hpp`, `src/...` | `boo::style`. Methods PascalCase (`Max`, `SetMaxPositionForTesting`, which can't be reset, as upstream). `Position` alias → `PositionType`. Not ported: `add(Hasher&, GridPosition)` (needs `DefaultHash<StringAtom>`), logging. `CSSValueConversion<GridPosition>` came with f4b. |
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
- `consumeUnresolvedLinearEasingFunction` accepts a trailing comma (`linear(0, 1,)`): a stop that fails after a comma just ends the loop. Krystal fails it (the grammar is `[ <number> && <percentage>{0,2} ]#`).
- `FloatQuadTests`' `ExpectRotatedRect` computes its comparisons and drops them. Krystal checks them, with the trapezoid's size corrected to (26.833, 13.416).
- Kept as upstream: `ParserContext::Hash()`'s quirks and `AffineTransform::Blend` (see above).

### Krystal.Core changes made here

`EnumSet` storage (`unsigned_of_size_t<Bytes>` added to `TypeTraits.hpp`, sized from the highest value, ≤ 64 values); `#pragma once` added to `EnumSet.hpp`, `AllowCompactPointers.hpp`, `GetPtrHelper.hpp`, `UnalignedAccess.hpp`, `ZipTuple.hpp`; `Hash::Combine`'s private helper renamed `CombineInto` (an lvalue `size_t` seed picked it); `StringAtom::Storage()`; `Text/Unicode.hpp` includes `Text/ASCII.hpp`; `text::IsASCIIAlphaCaselessEqual`; `Text/CodePointIterator.hpp`; `LazyNeverDestroyed` asserts it was constructed before access and isn't constructed twice (debug builds, with `IsConstructed()`, as upstream) and is non-copyable (tested in `Krystal.Core.Tests/tests/Utils/NeverDestroyed.cpp`). Generated `DefaultHash<ValueId>`/`DefaultHash<PropertyId>` define `Hash`/`Equal` on the enum.

### Loose ends

- `StringAtom::View()` (Krystal.Core) dereferences null for `Null()`; `CounterValue` works around it, and `ImageValue`'s default initiator type would hit it in `LoadImage`.
- Krystal.Core's text tests (`tests/Text/*`) aren't in `Krystal.Core.Tests/CMakeLists.txt`; only `CodePointIterator.cpp` is.
- Unreviewed files still on the old or raw names: `Consumers/Font.hpp` (`ConsumeFontTech` returns `SmallList<FontTechnology>`; `ConsumeFontFaceSrc` will need a `List`); `Consumers/Grid.hpp` (own `GridNamedAreaMapRow` alias); `Consumers/Shapes.cpp` (stub); `PrimitiveValueMappings.hpp` (raw `CSSFontFaceSrcValue.h`); `CSS/Font/FontFace.hpp` (raw `FontTaggedSettings.h`); `Style/Values/Primitives/Position.hpp` (`x.offset`); `Style/Values/Primitives/CoordinatedValueList.hpp` (`PointerComparison`).

### Tests

- Every subclass: `tests/CSS/Values/<Name>.cpp` (one per source file), expectations from the upstream `.cpp`s, with `Krystal.Booey.Tests/CSS/ValueTestUtils.hpp` (`ValueFixture` calls `StaticValuePool::Init()`; `CssText`, `ToNarrow`, `Children`). Each covers `Create`, `Is<X>Value`, the accessors, every branch of `CustomCSSText`, `Equals` (equal and each field differing), `AddHash`, `VisitChildren`, and dependencies/base URL/subresources where customised. Only `PendingSubstitutionValue` has none (nothing but the deferred `ResolveValue`).
- Base and supporting types: tests mirror the source paths (`tests/CSS/Values/{Units,NumericTypesSerialisation,PrimitiveValue,Aggregates}.cpp`, `tests/CSS/Parser/ParserContext.cpp`, `tests/CSS/Values/{Primitives,Grid,Easing,Borders,TextDecoration,Shapes,FilterEffects,Motion,Images}/`, `tests/Rendering/Style/GridSpan.cpp`, `tests/Style/Values/Grid/GridPosition.cpp`, `tests/Graphics/`, `tests/SVG/Internals/SVGPath.cpp`, `tests/Core/NumberFormatting.cpp`, `tests/URL/`, Krystal.Core `tests/Text/CodePointIterator.cpp`). `GridPosition` tests reset `SetMaxPositionForTesting` to 1000000.
- Upstream tests converted: `TestWebKitAPI/Tests/WTF/URL.cpp` (except `URLDataURIStringSharing`, `PrintStream`, `IsolatedCopy`; `URLParser.cpp`'s cases are covered by the WPT data), and `IntRectTests`, `FloatRectTests`, `FloatQuadTests`, `RegionTests`, `AffineTransform`, `TransformationMatrix` (gtest → Catch2 script; minus `Casting`/`EnclosingIntRect` and `area()`). Region compares structurally (upstream compares `TextStream` dumps); the fuzz test uses a fixed seed.
- `tests/URL/WPT.cpp` runs WPT's `urltestdata.json`, `toascii.json` and `IdnaTestV2.json` (copied to `data/url/` from WebKit's `LayoutTests/imported/w3c/web-platform-tests/url/resources`). Three `IdnaTestV2.json` cases expect a V7 failure for U+32931/U+32B9A, which Unicode 17 assigned; the test expects them to be valid.
- Full Booey suite (2026-10-13): 1348 cases / 671304 assertions; all pass except 2 expected failures. Whole solution builds with no warnings.

## Property parser — done (Style-dependent pieces deferred)

Entry: `css/parser/CSSPropertyParser.cpp` (headers-only graph: 151 files, `tools/port/out/property-parser.md`). Most of that graph comes through `StyleBuilder.h`, `StyleCustomProperty.h` and `StylePrimitiveNumericTypes+CSSValueConversion.h`, which only the typed custom-property functions need.

**Plan (decided 2026-10-13):** `PropertyParser.cpp` can't link until the generated `PropertyParsing.cpp` does, and that includes `PropertyParserCustom.hpp` and calls every consumer (~43 `Consumers/*.cpp`, ~15k upstream lines; `Font.cpp` and `Shapes.cpp` are stubs, `Syntax.cpp` is missing). So, in layers, with the Style-dependent pieces declared and FIXME'd (the hybrid approach used for the Values):
- **a.** Parser plumbing: `Property`, `ParsedPropertyList`, `PropertyParserResult`, `PropertyShorthand` (+ generated shorthand functions), `CustomPropertySyntax`, `VariableParser`. Done.
- **b.** The consumers, in groups, compile-only first (as with the Value subclasses). Groups, by upstream include order (proposed 2026-10-13):
  - **b1.** Shared foundations: the remaining `*Definitions.hpp`, `MetaResolver`, `PrimitiveValue` (`CSSPrimitiveValueResolver`), `List`, `Percentage`, `String`. Done.
  - **b2.** Ident/keyword-level consumers: `Align`, `Anchor`, `Attr`, `Box`, `Display`, `Inline`, `PositionTry`, `Ratio`, `ScrollSnap`, `Scrollbars`, `Syntax` (was missing), `Transitions`, `UnicodeRange`, `URL`, `ViewTransition`, `WillChange`. Done.
  - **b3.** Numeric/structured consumers: `Animations`, `CounterStyles`, `Easing`, `Grid`, `Lists`, `Position`, `SVG`, `TextDecoration`, `Timeline`, `Transform`. Done (`Timeline`, `Animations`, `CounterStyles` compile-only until layer c).
  - **b4.** Image-dependent consumers: `Image`, then `Background`, `Content`, `Filter`, `Masking`, `Motion`, `UI`. Done; `Image` calls `Filter`'s `ConsumeUnresolvedFilter`, so they went together. `Background`, `Content`, `Masking`, `Motion` compile-only until layer c.
  - **b5.** The stubs: `Font`, `Shapes` (~2k upstream lines). Done (`Shapes` compile-only until layer c).
- **c.** `PropertyParserCustom.hpp`, generated `PropertyParsing.cpp`, `PropertyParser.cpp`; then tests parsing properties from strings, and `ParserFastPaths::MaybeParseValue`/`ValuePool::CreateFontFaceValue`. Split (proposed 2026-10-13):
  - **c1.** Done (2026-10-13; see the two *Layer c1* sections below). Original plan: review `codegen/css_properties.py`'s `PropertyParsing.hpp`/`.cpp` output against upstream's generator (`Source/WebCore/css/scripts/process-css-properties.py`); only the shorthand functions and `IsExposed` have been checked (layer a). Then get `PropertyParserCustom.hpp` (hand-written shorthand parsers, 3,106 lines vs upstream 2,216; no raw WebKit names left, 114 commented lines, 2 FIXMEs; unreviewed) and the generated `PropertyParsing.cpp` (20,064 lines; generated each build, listed in `KRYSTAL_BOOEY_GENERATED_SOURCES` but not in `KRYSTAL_BOOEY_CSS_SOURCES`) compiling and linking. Generator review first: it's the riskiest, unreviewed part.
  - **c2.** Done (2026-10-13). Tests for the compile-only consumers, which link against the generated functions: `Timeline`, `Animations`, `CounterStyles`, `Background`, `Content`, `Masking`, `Motion`, `Shapes` (including all of `<basic-shape>` and the layer b4 `Content` `/` and `Motion` `ray()` fixes).
  - **c3.** Done (2026-10-13). `PropertyParser.cpp`/`.hpp` (1,064 lines vs upstream 904; translated, unreviewed), then property-from-string tests, `ParserFastPaths::MaybeParseValue` and `ValuePool::CreateFontFaceValue`. Still deferred after c3 (need Style/`Document`): typed custom-property parsing and the raw string parsers (`Font`/`FontFace`, `Filter`, `Transform`, `Easing`).
  - **Trial compile (2026-10-13, reverted):** with `PropertyParsing.cpp` added to `KRYSTAL_BOOEY_CSS_SOURCES` and compiled alone (MSBuild `-t:ClCompile -p:SelectedFiles=...`), it stopped at 13 errors, all in `PropertyParserCustom.hpp`'s border-radius and border-spacing shorthands (~lines 425-455): `state.ValuePool` (the member is `Pool`), `AddPropertyForCurrentShorthand` called with the wrong arguments, and `(void)shorthand` on an unnamed parameter. Expect more once those are fixed. Whether `PropertyParser.cpp` compiles is unknown (the trial didn't compile it).

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
- Not ported (not generated by `css_properties.py`): `colorProperties`/`physicalProperties` (`IsColorProperty` added in layer c3), `animationUsesNonAdditiveOrCumulativeInterpolation`, `animationUsesNonNormalizedDiscreteInterpolation`, `animationIsAccelerated`, `allAcceleratedAnimationProperties` (the last two need `Settings`). They come with animations.
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
- `UnicodeRange`: numbers are read from the tokens' original text (since 2026-10-13); delimiters are matched with `Token::Delimiter()`.
- `AllowedURLModifiers` is a flags enum with a `None` value (upstream: `OptionSet`).

Tests: one file per consumer under `tests/CSS/Properties/Consumers/`, using the new `ConsumeToText` helper (`Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp`): 30 cases / 436 assertions pass (3 expected failures). Full Booey suite: 1407 cases / 672280 assertions; all pass except 4 expected failures.

### Layer b3: numeric/structured consumers (2026-10-13)

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParserConsumer+Easing.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Easing.hpp`, `src/.../Easing.cpp` | on | tested (2026-10-13). Graph: 72 files (`tools/port/out/consumer-easing.md`), all already built. No `parseEasingFunctionDeprecated` (see *Where we are*). |
| `css/parser/CSSPropertyParserConsumer+Lists.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Lists.hpp`, `src/.../Lists.cpp` | on | tested (2026-10-13). Matched upstream. Graph: `tools/port/out/consumer-lists.md`. |
| `css/parser/CSSPropertyParserConsumer+Position.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Position.hpp`, `src/.../Position.cpp` | on | tested (2026-10-13). Graph: 67 files (`tools/port/out/consumer-position.md`), all already built. See bugs. |
| `css/parser/CSSPropertyParserConsumer+Grid.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Grid.hpp`, `src/.../Grid.cpp` | on | tested (2026-10-13). Graph: 79 files (`tools/port/out/consumer-grid.md`), all already built. See bugs. |
| `css/parser/CSSPropertyParserConsumer+SVG.h`, `.cpp` | `have` | `CSS/Properties/Consumers/SVG.hpp`, `src/.../SVG.cpp` | on | tested. Matched upstream. Graph: `tools/port/out/consumer-svg.md`. |
| `css/parser/CSSPropertyParserConsumer+TextDecoration.h`, `.cpp` | `have` | `CSS/Properties/Consumers/TextDecoration.hpp`, `src/.../TextDecoration.cpp` | on | tested. Matched upstream. Graph: `tools/port/out/consumer-textdecoration.md`. |
| `css/parser/CSSPropertyParserConsumer+Transform.h`, `.cpp` | `have` (no `parseTransformRaw`) | `CSS/Properties/Consumers/Transform.hpp`, `src/.../Transform.cpp` | on | tested. Graph: 117 files (`tools/port/out/consumer-transform.md`), the extra ones through `StyleBuilderState.h`/`StyleTransform.h` for `parseTransformRaw`. See bugs. |
| `css/parser/CSSPropertyParserConsumer+Timeline.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Timeline.hpp`, `src/.../Timeline.cpp` | on | compiles; tests wait for layer c (see *Where we are*). See bugs. |
| `css/parser/CSSPropertyParserConsumer+Animations.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Animations.hpp`, `src/.../Animations.cpp` | on | compiles; tests wait for layer c. See bugs. |
| `css/parser/CSSPropertyParserConsumer+CounterStyles.h`, `.cpp` | `have` | `CSS/Properties/Consumers/CounterStyles.hpp`, `src/.../CounterStyles.cpp` | on | compiles; tests wait for layer c. Matched upstream (comments fixed). |

**Bugs fixed:**
- `Easing`: `linear()` accepted a trailing comma (upstream bug; see *Upstream bugs found*). Raw `assert`s replaced with `krys_debug_assert`, one unbraced `if`, and a comment mangled by a rename ("tokens constraints").
- `Position`: `ConsumeTwoComponentPositionHorizontalUnresolved`/`VerticalUnresolved` were defined as `consumeTwoComponent...` (lowercase, no `noexcept`), so the declared functions were never defined. Unbraced `if`s.
- `Grid`: `ConsumeUnresolvedGridTemplateAreasRow`'s decode/encode `krys_debug_assert`s were inverted (`!= None`), so every valid `grid-template-areas` row asserted in debug; and an empty row string (`""`) was decoded before the whitespace check, which fails (`Decode` reports `EmptyInput`), so the check now runs on the UTF-8 text first. `fr` was matched by exact string, so `1FR` wasn't a flex breadth (now `ParseUnitType`, which ignores case).
- `Transform`: `ConsumeTranslate` called `Discard()` after the y value (upstream: skip whitespace), dropping the z value (`translate: 1px 2px 3px` → `1px 2px`). `ParseTransformRaw` was declared with a nonexistent return type (`CSSTransform`) and never defined; replaced with a FIXME (see *Where we are*). Raw `assert`.
- `Timeline`: `ConsumeAnimationTimelineView`'s empty-arguments check was inverted (`view()` with arguments returned an empty `view()`, and `view()` parsed nothing). `ParseSingleViewTimelineInsetItem`/`ParseSingleAnimationRange` never pumped the tokenizer, so they always saw an empty range.
- `Animations`: `ParseKeyframeKeyList` never pumped the tokenizer. Used `ResolveAsPercentageNoConversionDataRequired` (a TODO) where upstream uses `ResolveAsPercentageDeprecated` (now available).
- `TextDecoration`: `ConsumeTextShadow` was defined with a fully qualified name inside its own namespace (an IntelliSense workaround).
- `SVG`: raw `assert`.
- Comments and a spec URL mangled by an earlier `range` → `tokens` rename in `Timeline`, `Animations` and `CounterStyles` (`#counter-style-tokens`, `animation-tokens-start`), and a `tokensList` variable.

**Differences:**
- `Grid`: repeat counts are `Integer<[1,∞], uint32>` (upstream `unsigned`). Rows are decoded to UTF-32 to walk code points (upstream walks UTF-16 code units).

- `SingleAnimationRangeType` is declared in `Consumers/Timeline.hpp` in `css` (upstream: `Style::SingleAnimationRangeType`, forward-declared there and defined in `TimelineRangeValue.h`); move it when the Style side is ported.

Tests: `tests/CSS/Properties/Consumers/{Easing,Lists,Position,Grid,SVG,TextDecoration,Transform}.cpp`: 34 cases / 606 assertions pass. Full Booey suite: 1441 cases / 672886 assertions; all pass except 4 expected failures.

### Layer b4: Image-dependent consumers (2026-10-13)

Graphs: `tools/port/out/consumer-image.md` (107 files; new beyond the b3 graphs: the image `Value` subclasses, the filter-effects types, `CSSGradient.h`, `CSSURL.h`, all already built), `tools/port/out/consumer-filter.md` (128 files, the extra ones through `StyleBuilderState.h`/`StyleFilter.h` for `parseFilterValueListOrNoneRaw`), `consumer-{background,content,masking,motion,ui}.md` (new beyond the earlier graphs: only already-built `Value` subclasses and their CSS types, plus `CSSPropertyParserConsumer+Shapes.h`, the b5 stub).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParserConsumer+Image.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Image.hpp`, `src/.../Image.cpp` | on | tested. Rewritten: from the prefixed gradients on, the `.cpp` was still raw WebKit. See bugs and differences. |
| `css/parser/CSSPropertyParserConsumer+Filter.h`, `.cpp` | `have` (no `parseFilterValueListOrNoneRaw`) | `CSS/Properties/Consumers/Filter.hpp`, `src/.../Filter.cpp` | on | tested. See bugs. |
| `css/parser/CSSPropertyParserConsumer+UI.h`, `.cpp` | `have` | `CSS/Properties/Consumers/UI.hpp`, `src/.../UI.cpp` | on | tested. Matched upstream. |
| `css/parser/CSSPropertyParserConsumer+Background.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Background.hpp`, `src/.../Background.cpp` | on | compiles; tests wait for layer c. See bugs. |
| `css/parser/CSSPropertyParserConsumer+Content.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Content.hpp`, `src/.../Content.cpp` | on | compiles; tests wait for layer c. See bugs. |
| `css/parser/CSSPropertyParserConsumer+Masking.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Masking.hpp`, `src/.../Masking.cpp` | on | compiles; tests wait for layer c and b5. See bugs. |
| `css/parser/CSSPropertyParserConsumer+Motion.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Motion.hpp`, `src/.../Motion.cpp` | on | compiles; tests wait for layer c and b5. See bugs. |

**Bugs fixed:**
- `Image`: the `.cpp` didn't compile. Everything after `ComputeGradientColorInterpolationMethod` was raw WebKit (`CSSValue*` ids, `CSSGradientValue::create`, `SortedArrayMap`, lowercase gradient members, `CSSVariableData::create`, `OptionSet`), and the earlier half used the old lowercase member names (`.color`, `.parameters`, `legacyMethod`). The header had a duplicate `#pragma once`.
- `Filter`: the number/percentage filter functions read `FunctionNotation::name` (Krystal: `Name`), so the file didn't compile. `ConsumeUnresolvedFilterValueList` wasn't `noexcept`. `parseFilterValueListOrNoneRaw` was commented out in both files; replaced with a FIXME (see *Where we are*).
- `Content`: the visible-content list checked for `/` with `IdentCodePoints()`, which is empty for delimiter tokens, so it never stopped at `/` and `content: "a" / "alt"` failed. Now `Delimiter()`. `ConsumeCounterContent` wasn't `KRYS_NODISCARD`.
- `Motion`: the `ray()` size keyword was looked up on the outer range instead of the function's arguments, and never consumed, so any `ray()` with a size failed (`ray(45deg closest-side)`).
- `Background`: included `CSS/Calc/CalcTreeParser.hpp` (doesn't exist; `TreeParser.hpp`) and so didn't compile, nor did `IsCalcFunction` (in `calc`) or `list.value` (`Value`). `ConsumeWebkitBoxShadow` was defined but not declared; `ConsumeWebkitBoxReflect` and `ConsumeBackgroundSize` weren't `KRYS_NODISCARD`. Raw `assert`s (one in the header, with `<cassert>`), an unbraced `if`, and comments mangled by the `range` → `tokens` rename.
- `Masking`: `ConsumeClipPath` was defined with a fully qualified name inside its own namespace.

**Added (upstream API):** `ConsumeIdentUsingMapping`/`PeekIdentUsingMapping` in `Ident.hpp` (moved from a local in `Image.cpp`; `Motion` uses it too).

**Differences:**
- `Image`, `Motion`: the `SortedArrayMap` keyword tables are `switch`-based `*FromValueId` functions passed to `ConsumeIdentUsingMapping`. `image-set()`'s `type()` checks for a string token before `ConsumeStringRaw` (upstream checks for a null view, as in `URL`). No `shrinkToFit` on the stop lists (`SmallList` has none).
- `-apple-invert-lightness()` doesn't parse, as upstream: the keyword is `apple-invert-lightness`, and the `-apple-` → `-webkit-` keyword rewrite gives `-webkit-invert-lightness`, which doesn't exist. Upstream's own tests use `apple-invert-lightness()`.

Tests: `tests/CSS/Properties/Consumers/{Image,Filter}.cpp`: 17 cases / 321 assertions pass. `tests/CSS/Properties/Consumers/UI.cpp` and `ConsumeIdentUsingMapping` in `Ident.cpp`: 3 cases / 42 assertions pass. Full Booey suite: 1455 cases / 673206 assertions; all pass except 4 expected failures.

### Layer b5: `Font` and `Shapes` (2026-10-13)

Graphs: `tools/port/out/consumer-font.md` (152 files; new over the earlier graphs: the font `Value` subclasses, all already built; `FontFace.h`/`CSSFontFace.h` and `CSSParser.h`, which only the `FontFace` string parsers need; `CSSPrimitiveValueMappings.h`, which brings in ~20 Style keyword types and is only used for `FontTechnology`; and `CSSFontVariantLigaturesParser.h`/`CSSFontVariantNumericParser.h`, included but unused), `tools/port/out/consumer-shapes.md` (83 files, all already built: the shape types, `CSSBasicShapeValue`, `CSSPathValue`, the SVG path byte stream). Both `.cpp`s were empty stubs.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParserConsumer+Font.h`, `.cpp` | `have` (no string parsers) | `CSS/Properties/Consumers/Font.hpp`, `src/.../Font.cpp` | on | tested. Ported; see bugs and differences. |
| `css/parser/CSSPropertyParserConsumer+Shapes.h`, `.cpp` | `have` | `CSS/Properties/Consumers/Shapes.hpp`, `src/.../Shapes.cpp` | on | compiles; tests wait for layer c (`ConsumeShapeOutside` calls the generated `ConsumeShapeBox`, and the `<rect()>`/`<xywh()>`/`<inset()>` radii come from `Background`). Ported. |
| `css/WebKitFontFamilyNames.in` (generated `WebKitFontFamilyNames.h`, `.cpp`) | `have*` | `CSS/WebKitFontFamilyNames.hpp`, `src/CSS/WebKitFontFamilyNames.cpp` | on | tested through `Font`. Hand-written; see differences. |

**Bugs fixed:**
- `Font.cpp`, `Shapes.cpp`: empty, and in `css::CSSPropertyParserHelpers` (the headers declare `css::PropertyParserHelpers`), so nothing was defined.
- `Font.hpp`: used `PropertyParserState` without declaring it; `UnresolvedFont`'s members were lowercase. `Shapes.hpp`: `uint8_t`, and a stray `;` after the enum traits macro.

**Added:** `ConsumeUnresolvedFont` is public (upstream: `static`, called only by `parseUnresolvedFont`), so it's built and tested while the string parser waits.

**Differences:**
- `Font` is the `ENABLE(VARIATION_FONTS)` build (as are `FontStyleRangeValue`/`FontVariationValue`): font-weight numbers aren't limited to multiples of 100, `@font-face` `font-style` takes an angle range, and `ConsumeVariationTagValue` exists.
- The string parsers aren't ported (see *Where we are*). Nor are `forwardVariantTo` (unused upstream) and `parserMode` (needs `Document`).
- Null strings: `ConsumeFontFormat` returns `Maybe<CSSOMString>` (so `format("")` stays distinct from failure), and the family-name helpers return `Maybe<CSSOMStringAtom>`. `GenericFontFamily` returns `CSSOMStringAtom::Null()` for non-generic keywords.
- `Vector` → `List` for `UnresolvedFontFamily`, `ConsumeFontTech` and `ConsumeFontFeatureValuesPreludeFamilyNameList`.
- `fromCSSValueID<FontTechnology>` (in the unreviewed `PrimitiveValueMappings.hpp`) is a local `FontTechnologyFromValueId` in `Font.cpp`.
- `Shapes`: the `SortedArrayMap` keyword tables are `switch`-based `*FromValueId` functions passed to `ConsumeIdentUsingMapping`/`PeekIdentUsingMapping`; the circle and ellipse extent tables share one template. `ConsumeShapeCommandAffinity` and `ConsumeBasicShapePathFunctionParameters` drop their unused state parameter. `CSS::Rect` is `RectShape` (layer *Values*).
- `WebKitFontFamilyNames`: upstream generates it from `WebKitFontFamilyNames.in` (`make_names.pl`) as lazily-constructed globals. Here, one function per name (`SerifFamily()`, ...) returning a function-local static atom, and `FamilyName(index)` in place of the `familyNames`/`familyNamesData` lists. No `FamilyNamesList` (used by `FontSelector`; comes with it).
- A quoted generic family (`"serif"`) serialises unquoted, as upstream: `SerialiseFontFamily` only quotes non-identifiers.

Tests: `tests/CSS/Properties/Consumers/Font.cpp`: 14 cases / 248 assertions pass. Full Booey suite: 1469 cases / 673454 assertions; all pass except 4 expected failures.

### Layer c1: generator review (2026-10-13)

Reviewed by comparing output rather than scripts: upstream's `css/scripts/process-css-properties.py` was run on its `CSSProperties.json`, filtered to the properties in `codegen/css-properties.json`, with the same defines, and its `CSSPropertyParsing.h`/`.cpp` diffed against ours after normalising names (`CSSValueFoo` → `ValueId::Foo`, `consumeIncludingWhitespace` → `Discard` + `DiscardWhitespace`, `AnchorPolicy::Forbid` → `AllowAnchor(false)`, `OptionSet` lists → `|`, `move`, ...). After normalising, the 462 generated functions match line for line; the remaining differences are the assertion macros and named parameters on the `@function` descriptor stubs.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/scripts/process-css-properties.py` (`GenerateCSSPropertyParsing`, the term generators, `PropertyConsumer`s) | `have` | `codegen/css_properties.py` (`GeneratePropertyParsing`) | (codegen) | Reviewed via output. `PropertyParsing.cpp` isn't compiled yet (c1, next). |
| `css/CSSProperties.json` | `have*` | `codegen/css-properties.json` | (codegen) | Restructured schema (`module` for `category`; `parser-grammar` + `parser-grammar-unused: true` for `parser-grammar-unused: "<grammar>"`), PascalCase parser functions. See differences. |

**Bugs fixed:**
- The build passed no `--defines`, so `font-variation-settings`, `font-optical-sizing`, `color-scheme` and the variation-font forms of `font-width` and the `@font-face` `font-weight`/`font-style`/`font-width` descriptors weren't generated, though `Font` (layer b5) and `ColorScheme` (layer 5b) are ported for them. CMake now passes `--defines=ENABLE_DARK_MODE_CSS,ENABLE_VARIATION_FONTS`. Both scripts split `--defines` on spaces while `main.py`'s help says comma-separated; they now accept either.
- The longhand/descriptor parse functions guarded unexposed properties with `krys_unreachable()` (`std::unreachable()`: undefined behaviour in release); upstream's `ASSERT_NOT_REACHED()` is debug-only and returns null. Now `krys_debug_assert(false)`, as the shorthand guard already was.
- The exported shared-rule consumers in `PropertyParsing.hpp` weren't `KRYS_NODISCARD` (the property consumers were).

**Differences:**
- 41 legacy `-webkit-`-only properties are dropped from `css-properties.json` on purpose (decided by the user: Booey doesn't support legacy or WebKit-specific properties): `-webkit-backdrop-filter`, `-webkit-background-clip`/`-origin`, `-webkit-border-image`, `-webkit-border-radius`, `-webkit-box-*` (`align`, `direction`, `flex`, `flex-group`, `lines`, `ordinal-group`, `orient`, `pack`, `reflect`, `shadow`), `-webkit-column-axis`, `-webkit-column-break-*`, `-webkit-font-size-delta`, `-webkit-line-clamp`/`-grid`/`-snap`, `-webkit-marquee-*`, `-webkit-mask`, `-webkit-mask-box-image`, `-webkit-mask-clip`/`-composite`/`-source-type`, `-webkit-nbsp-mode`, `-webkit-perspective`, `-webkit-text-orientation`, `-webkit-text-size-adjust`, `-webkit-text-stroke*`, `-webkit-text-zoom`. Code that only serves them (e.g. their shorthand parsers in `PropertyParserCustom.hpp`) can be removed rather than ported.
- Defines left off: the Apple Pay, attachment element, touch, cursor visibility, text autosizing, overflow scrolling, touch callout and Core Animation/material features.

### Layer c1: custom shorthand parsers and generated parsing (2026-10-13)

`PropertyParserCustom.hpp` compared function by function with `css/parser/CSSPropertyParserCustom.h`. The generated `PropertyParsing.cpp` now compiles and links (a test calls into it, so the linker pulls in all its references).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParserCustom.h` | `have` | `CSS/Properties/PropertyParserCustom.hpp` | on | tested. See bugs and differences. |
| `CSSPropertyParsing.h`, `.cpp` (generated) | `have` | `codegen/css_properties.py` → `PropertyParsing.hpp`/`.cpp` | on | tested (longhands and shorthands through `PropertyParserCustom`). |
| `css/parser/CSSFontVariantLigaturesParser.h`, `CSSFontVariantNumericParser.h` | `have` | `CSS/Properties/FontVariantLigaturesParser.hpp`, `FontVariantNumericParser.hpp` | on | tested through `font-variant`. Matched upstream. |
| `css/CSSBorderImage.h`, `.cpp` | `have` | `CSS/Values/Borders/BorderImage.hpp`, `src/.../BorderImage.cpp` | on | compiles; the `.cpp` was missing (unresolved `CreateBorderImageValue` from `Background`). Ported. |
| `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h` | `have` | `CSS/Values/Primitives/NumericTypesValueCreation.hpp` | on | tested. See bugs. |
| `css/values/CSSValueTypes.cpp` (value creation) | `have` | `src/CSS/Values/TypesValueCreation.cpp` | on | compiles; was never built (unresolved `CreateCoalescingPairValue<Space>`). |

**Bugs fixed:**
- `grid-template: none` never matched: `none` was consumed from a copy of the range. The range wasn't restored before the `<string>` form (commented out), the `/` before the columns was looked for with `IdentCodePoints()` (empty for delimiters; as `Content`, layer b4), so `"a" / 1fr` failed, and trailing line names were moved into the list, so consecutive `[y] [z]` weren't merged into `[y z]`.
- `grid-area: a / b` defaulted `grid-column-end` to the row start (`a`) instead of the column start (`b`).
- `mask` with a position failed: the background/mask shorthand only handled `background-position-x/y`, not `-webkit-mask-position-x/y` (`mask`'s position longhands).
- `animation`/`transition`: `animation-timing-function`, `transition-property` and `transition-timing-function` were stubbed to null, so any timing function or transition property failed.
- `view-timeline`: the insets went into the axis list, leaving the inset list empty.
- `font`: the system font keywords (`font: caption`) were commented out.
- `overflow`: a single `-webkit-paged-x`/`-y` didn't set `overflow-x` to `auto`.
- `flex`: the `-webkit-min-content`, `-webkit-max-content`, `-webkit-fill-available`, `-webkit-fit-content` basis keywords were missing.
- Didn't compile: `state.ValuePool` (`state.Pool`), `(void)shorthand` on unnamed parameters, named unused parameters; `ConsumeBorderSpacingShorthand` wasn't `noexcept`; unbraced `if`s. Includes the unreviewed `PropertyParser.hpp` instead of `PropertyParsing.hpp`.
- `NumericTypesValueCreation.hpp`: the calc case called a nonexistent `get()` (upstream `protectedCalc()`); not `KRYS_NODISCARD`/`noexcept`.
- `FontVariant*Parser.hpp`: used the inline `PrimitiveValue::Create(ValueId)` without including `ValuePool.hpp`. `Finalize` → `FinalizeValue`.
- `TypesValueCreation.cpp`: same missing `ValuePool.hpp` include.
- Generator: the empty `@function` descriptor keyword stubs named parameters they don't use (C4100).

**Differences:**
- The shorthand parsers are defined in the class body (upstream: declared in the class, defined `inline` below).
- No parsers for the dropped `-webkit-` shorthands (`-webkit-border-radius`, `-webkit-border-image`, `-webkit-mask-box-image`, `-webkit-column-break-*`, `-webkit-text-orientation`, `-webkit-perspective`) or their keyword mapping (`mapFromColumnBreakBetween`); the background/mask shorthand has no `-webkit-background-clip/origin` or `-webkit-mask-clip/composite/source-type` cases. `-webkit-background-size` and `-webkit-mask-position` are still in `css-properties.json`, so they're kept.

Tests: `tests/CSS/Properties/PropertyParserCustom.cpp`: 2 cases / 36 assertions pass. Full Booey suite: 1471 cases / 673496 assertions; all pass except 4 expected failures. Whole solution builds with no warnings.

### Layer c2: tests for the compile-only consumers (2026-10-13)

Each consumer was first probed (its output printed for a set of inputs), the outputs checked against upstream and the specs, then frozen as tests. No consumer bugs were found; the layer b4 `Content` `/` and `Motion` `ray()` size fixes are now tested.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `CSSPropertyParserConsumer+Timeline`, `+Animations`, `+CounterStyles`, `+Content`, `+Masking`, `+Motion`, `+Shapes` | `have` | same names | on | tested. |
| `CSSPropertyParserConsumer+Background` | `have` | `Background` | on | tested. See differences. |

**Differences:**
- `Background`: `consumeUnresolvedWebKitBorderRadius` (and the two-value `-webkit-border-radius` quirk), `consumeWebkitBoxShadow` (and the `isWebkitBoxShadow` flag it set) and `consumeWebkitBoxReflect` are removed: they only served the dropped `-webkit-border-radius`, `-webkit-box-shadow` and `-webkit-box-reflect`. `BoxShadow::IsWebkitBoxShadow` and `ReflectValue` (Values layer) are kept. `BorderImageSliceFillDefault`/`BorderImageWidthOverridesWidthForLength` are kept, though only the dropped `-webkit-border-image`/`-webkit-mask-box-image` passed `Yes`.
- `ConsumeKeyframesName("")` gives an empty `<custom-ident>` that serialises as nothing, as upstream (the empty string isn't a keyword).

**Krystal.Core, flagged:** `StringAtom::View()` dereferences unconditionally, so calling it on `StringAtom::Null()` crashes (WTF's null `AtomString` gives an empty view). Callers must compare with `Null()` first.

Tests: `tests/CSS/Properties/Consumers/{Timeline,Animations,CounterStyles,Background,Content,Masking,Motion,Shapes}.cpp`: 26 cases. Full Booey suite: 1497 cases / 673896 assertions; all pass except 4 expected failures. Whole solution builds with no warnings.

### Layer c3: the property parser (2026-10-13)

`CSSPropertyParser.h`/`.cpp` compared with `PropertyParser.hpp`/`.cpp`; the value half of `CSSParserFastPaths.cpp` ported; `CSSValuePool::createFontFaceValue` added.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSPropertyParser.h`, `.cpp` | `have` (no typed custom-property parsing) | `CSS/Properties/PropertyParser.hpp`, `src/.../PropertyParser.cpp` | on | tested. See bugs and differences. The keyword/property-name lookups live in `KeywordLookup` (slice 1, layer a). |
| `css/parser/CSSParserFastPaths.cpp` (value half: `maybeParseValue` and the length, keyword, transform, `display`, `opacity` and colour fast paths) | `have` | `src/CSS/Parser/ParserFastPaths.cpp` | on | tested. Ported. |
| `CSSValuePool::createFontFaceValue` | `have` | `ValuePool::CreateFontFaceValue` | on | tested. |
| `CSSProperty::isColorProperty` (generated `colorProperties`) | `have*` | `Property::IsColorProperty`, generated into `PropertyId.cpp` | on | tested through the fast path. A `switch` (upstream: a bitset). |

**Bugs fixed:**
- `PropertyParser::ParseStylePropertyLonghand(string)` and `ParseCounterStyleDescriptor` never pumped the tokenizer, so they always saw an empty range (as `Timeline`/`Animations`, layer b3).
- Didn't compile: raw WebKit includes (`StyleBuilder.h`, ...) and the typed custom-property code (`style::toStyleFromCSSValue`, `downcast<CSSURLValue>`, ...); old `CustomPropertySyntax` member names (`type`, `multiplier`, `ident`); designated initialisers out of `PropertyParserState`'s member order; the root consumers' definitions weren't `noexcept`; `ShareRef` of a `const Value` in `-internal-auto-base()`; `SmallList::resize` to drop the properties of a failed parse (needs a default constructor; now `truncate`, upstream `shrink`). The `.cpp` had no BOM.
- `krys_assert`/`krys_unreachable` where upstream's `ASSERT`/`ASSERT_NOT_REACHED` are debug-only (`krys_unreachable` is undefined behaviour in release).
- `ValuePool`: the new cache holds `RefPtr<ValueList>`, so the constructor and destructor are out of line (the header only forward-declares `ValueList`).
- Generator: the generated `Property::IsInheritedProperty` used the raw `assert`.

**Differences:**
- `ParseTypedCustomPropertyValue`/`ParseTypedCustomPropertyInitialValue` (and the file-local `consumeTypedCustomPropertyValue`) aren't defined (FIXME in the header): they need `style::CustomProperty` and `style::BuilderState`.
- Fast path: UTF-8 only; `StringParsingBuffer` is a `CSSOMStringView` advanced with `remove_prefix`, and the transform-name checks compare prefixes case-insensitively rather than character by character.

Tests: `tests/CSS/Properties/PropertyParser.cpp` (longhands, shorthands, CSS-wide keywords, `var()`, failure rollback, `-internal-auto-base()`, `@keyframes`/`@position-try`/`@page` and descriptors, the string entry points, custom-property syntax and its dependencies, `ParseGlobalKeyword`, `CreateFontFaceValue`) and `ParserFastPaths::MaybeParseValue` in `tests/CSS/Parser/ParserFastPaths.cpp`: 11 cases / 255 assertions with the existing fast-path cases. Full Booey suite: 1504 cases / 674094 assertions; all pass except 4 expected failures. Whole solution builds with no warnings.

## Sheet parser — done (CSSOM, loading and evaluation deferred)

Entry: `css/parser/CSSParser.cpp` (headers-only graph: 123 files, `tools/port/out/sheet-parser.md`). Upstream has merged `CSSParserImpl` into `CSSParser` (~2,050 lines). It turns text into the internal rule model (`StyleRule*` in `StyleSheetContents`) and calls `PropertyParser::ParseValue` per declaration. About 80 of the graph's files (values, calc, colour, tokenizer, property parser) are done; everything above them is on disk but unreviewed and unbuilt.

**Decisions (2026-10-13):**
- `src/CSS/Parser.cpp`/`Parser/Parser.hpp` (own spec-driven implementation that built CSSOM objects directly, mostly TODOs) is replaced by a port of upstream's `CSSParser`.
- `Rules/Rule.hpp` is Krystal's `StyleRuleBase`/`StyleRule*`: the internal representation, as `Value` is for `CSSValue`. The CSSOM wrappers (`CSSOM/CSS*Rule`, `CSSStyleSheet`) are created from it and are a later slice.
- Serialisation is not deferred: serialisers (selector text, `ShorthandSerializer`/`cssText`, media query and rule serialisation) are completed in the layer that owns them, tested alongside parsing.

**Layers (bottom-up, each built and tested):**
- **d1. Selectors:** `CSSSelector`, `CSSSelectorList`, `MutableCSSSelector`, `CSSSelectorParserContext`, `CSSSelectorParser` (~4,100 upstream lines; Krystal `Selectors/*`). Upstream's `CSSSelectorEnums.h` and pseudo-class/element tables are generated by `css/scripts/process-css-pseudo-selectors.py`; Krystal's `Selectors/Enums/*` are hand-written, so likely a codegen step. Tests parse and serialise selectors.
- **d2. Declaration blocks:** `StyleProperties`, `MutableStyleProperties`, `ImmutableStyleProperties`, `StylePropertiesInlines` and `ShorthandSerializer` (~2,900 lines; Krystal `Properties/StyleProperties.hpp` etc., `ShorthandSerialiser.hpp`). Done.
- **d3. Conditions:** media/container query parsing and serialisation (`query/MediaQuery`, `GenericMediaQueryParser`/`Types`/`Serialization`, `MediaQueryParser`, `ContainerQuery`, `ContainerQueryParser`, the `MediaQueryFeatures`/`ContainerQueryFeatures` schemas the parser validates against), `MediaQueryParserContext`. Evaluation deferred. Done; `CSSSupportsParser` moved to d5 and `MediaList` to the CSSOM slice (decided 2026-10-13).
- **d4. Rule model:** `StyleRule` (all rule types), `StyleRuleImport` (no loading), `StyleRuleFunction`, `CSSAtRuleID`, `CSSParserEnum`, `CSSCounterStyleDescriptors`, `StyleSheet`, the parse side of `StyleSheetContents`. Done; `StyleSheet` moved to the CSSOM slice and `@-internal-base-appearance` dropped (decided 2026-10-13).
- **d5. `CSSParser`:** whole stylesheets parse end to end, with `CSSSupportsParser`. Done.
- **Later:** CSSOM wrappers (with `MediaList` and `StyleSheet`), `@import` loading (`CachedCSSStyleSheet`), `parseInlineStyleDeclaration` (needs `Element`/`Document`), query evaluation, `RuleSet`/style resolution.

**Boundaries:** `CSSParserObserver`/`CSSParserObserverWrapper` and `cssjit/CompiledSelector.h` are `wont`; `html/parser/NestingLevelIncrementer.h` is trivial (`SetForScope`-like); `platform/graphics/FontFeatureValues.h`/`FontPaletteValues.h` are small and come with d4.

### Layer d1a: pseudo-selector codegen (2026-10-13)

Graph from the five selector `.cpp`s (`CSSSelector`, `CSSSelectorList`, `MutableCSSSelector`, `CSSSelectorParserContext`, `CSSSelectorParser`; headers-only): 68 files (`tools/port/out/selectors.md`), nearly all done. New for d1: the five selector files, `SelectorPseudoTypeMap.h`, `CSSParserEnum.h` (pulled forward from d4), `style/PseudoElementIdentifier.h` and `PseudoElementType` (`rendering/style/RenderStyleConstants.h`). `dom::QualifiedName` and `StyleSheetContents`'s namespace lookup exist (the parser accepts a null sheet). Unresolved generated headers: `CSSSelectorEnums.h`, `CSSSelectorInlines.h`, `UserAgentParts.h` (this layer), plus `HTMLNames.h` and `Settings.h`.

Decision (2026-10-13): only standard pseudo-classes and pseudo-elements are supported, including ones nothing else supports yet (media playback, `:autofill`, `:fullscreen`, `:visited`, `::highlight`, view transitions, ...), so they're easy to wire up later. Prefixed (`-webkit-`, `-apple-`, `-internal-`), non-standard and legacy alias names are dropped.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSPseudoSelectors.json` | `have*` | `codegen/css-pseudo-selectors.json` | — | Filtered copy: 63 of 86 pseudo-classes and 25 of 89 pseudo-elements; no `aliases`, `conditional` or `status` fields. |
| `css/scripts/process-css-pseudo-selectors.py` | `have*` | `codegen/css_pseudo_selectors.py` (run by `codegen/main.py`) | — | See differences. |
| generated `CSSSelectorEnums.h` | `have` | generated `CSS/Selectors/SelectorEnums.hpp` | on | tested. `SelectorPseudoClass`/`SelectorPseudoElement` (upstream `CSSSelectorPseudoClass`/`CSSSelectorPseudoElement`) with contiguous enum traits. Replaces the hand-written `Selectors/Enums/PseudoClassId.hpp`/`PseudoElementId.hpp` stubs (deleted). |
| generated `SelectorPseudoClassAndCompatibilityElementMap.cpp`, `SelectorPseudoElementMap.cpp` | `have` | same names, generated under `CSS/Selectors/` | on | tested. gperf, upstream's options. |
| `css/SelectorPseudoTypeMap.h` | `have` | `CSS/Selectors/SelectorPseudoTypeMap.hpp` | on | tested. Includes `SelectorEnums.hpp` rather than `Selector.hpp`. |
| generated `UserAgentParts.h`, `.cpp` | `have` | generated `CSS/Selectors/UserAgentParts.hpp`, `.cpp` | on | tested. `css::UserAgentParts::DetailsContent()` etc. (PascalCase). |
| generated `CSSSelectorInlines.h` | `have` | generated `CSS/Selectors/SelectorInlines.hpp` | — | Generated, not yet included: defines `Selector` statics (`IsPseudoClassEnabled`, `IsPseudoElementEnabled`, `SelectorTextForPseudoClass`, `PseudoClass`/`PseudoElement` `RequiresArgument`/`MayHaveArgument`); built and tested with d1b. |

**Differences:**
- Dropped pseudos: all prefixed ones (5 `-webkit-`, 5 `-internal-` and 1 `-apple-` pseudo-classes; 59 `-webkit-`, 4 `-internal-` and 1 `-apple-` pseudo-elements, including every `::-webkit-scrollbar*` part and the form-control/media-control parts), WebKit's non-standard scrollbar pseudo-classes (`:horizontal`, `:vertical`, `:decrement`, `:increment`, `:start`, `:end`, `:double-button`, `:single-button`, `:no-button`, `:corner-present`), `:window-inactive`, `:immersive` (`<model>` element, not standard) and the legacy aliases (`:matches`, `:-webkit-any-link`, `:-webkit-autofill`, `:-webkit-full-screen`, `::-webkit-file-upload-button`, `::-webkit-input-placeholder`).
- So `SelectorPseudoElement` has no `UserAgentPartLegacyAlias` or `WebKitUnknown`, and there is no `nameForUserAgentPartLegacyAlias`. Upstream accepts any unknown `::-webkit-*` pseudo-element (as `WebKitUnknown`, matching nothing); here it is a parse error, as the standard says. The `isUASheetBehavior` gating of `-internal-` pseudos is gone with them.
- No `conditional`/`ENABLE()` support: every standard pseudo is always generated (upstream compiles out the media ones without `ENABLE(VIDEO)`, `:fullscreen` without `ENABLE(FULLSCREEN_API)`, ...). Settings flags remain (`viewTransitionsEnabled`, `popoverAttributeEnabled`, `cssAppearanceBaseEnabled`, `targetTextPseudoElementEnabled`, `thumbAndTrackPseudoElementsEnabled`), read from `SelectorParserContext` under upstream's names.
- The validator rejects any prefixed name instead of counting `-webkit-` names.
- UTF-8 only: the maps take a `CSSOMStringView` and hand it straight to gperf (no 16-bit path); non-ASCII names don't match.
- The gperf maps disable MSVC's C4267 (gperf's hash narrows `size_t` to `unsigned int`); upstream disables its own set of warnings around them.
- `codegen/utils.py`'s `run_gperf` takes the gperf options (default unchanged, for `ValueId`).

**Unbuilt d1b/d1c files touched:** `Selector.hpp`, `MutableSelector.hpp` and `SelectorParser.cpp` were renamed from `PseudoClassId`/`PseudoElementId` to the generated enums (mechanical; not reviewed). `Selector.cpp` still calls the stubs' `IsElementBackedPseudoElement` (d1b).

Tests: `tests/CSS/Selectors/{SelectorPseudoTypeMap,UserAgentParts}.cpp`: 10 cases / 63 assertions. Full Booey suite: 1514 cases / 674157 assertions; all pass except 4 expected failures. Builds with no warnings.

### Layer d1b: selectors (2026-10-13)

Graph: the d1a graph (`tools/port/out/selectors.md`) covers these files. The Krystal files were an own partial rewrite (about half of upstream's API, never built); they're now completed against upstream, keeping their names where the d1c parser and `Rules/Rule.hpp` already use them.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSSelector.h`, `.cpp` | `have*` | `CSS/Selectors/Selector.hpp`, `src/.../Selector.cpp`, `Enums/SelectorMatch.hpp`, `Enums/SelectorRelation.hpp`, `Enums/SelectorPagePseudoClass.hpp`, `SelectorFlag.hpp` | on | tested. See names, bugs and differences. |
| `css/CSSSelectorList.h`, `.cpp` | `have` | `CSS/Selectors/SelectorList.hpp`, `src/.../SelectorList.cpp` | on | tested. |
| `css/parser/MutableCSSSelector.h`, `.cpp` | `have` | `CSS/Selectors/MutableSelector.hpp`, `src/.../MutableSelector.cpp` | on | tested. |
| `css/parser/CSSSelectorParserContext.h`, `.cpp` | `have` (no `Document` constructor, no hash) | `CSS/Selectors/SelectorParserContext.hpp`, `src/.../SelectorParserContext.cpp` | on | tested through the pseudo-name parsing. Pulled forward from d1c: the generated `SelectorInlines.hpp` needs it. |
| `rendering/style/RenderStyleConstants.h` (`PseudoElementType`) | `have*` | `Rendering/Style/RenderStyleConstants.hpp` | on | tested. Standard pseudo-elements only (see differences). |
| generated `CSSSelectorInlines.h` | `have` | generated `CSS/Selectors/SelectorInlines.hpp` | on | tested (generated in d1a). |

**Names:** enums are free (`SelectorMatch`, `SelectorRelation`, `SelectorPagePseudoClass`) rather than nested. `Match::Unknown`/`Tag`/`Exact`/`Set`/`List`/`Hyphen`/`Contain`/`Begin`/`End` → `None`/`Type`/`AttributeEquals`/`AttributeExists`/`AttributeIncludes`/`AttributeDash`/`AttributeSubstring`/`AttributePrefix`/`AttributeSuffix`; `Relation::Subselector`/`DescendantSpace`/`DirectAdjacent`/`IndirectAdjacent` → `Compounding`/`Descendant`/`NextSibling`/`SubsequentSibling`. `precedingInComplexSelector`/`firstInCompound`/`lastInCompound`/`precedingInCompound` → `PrecedingComplexSelectorComponent`/`FirstInCompoundSelector`/`LastInCompoundSelector`/`PrecedingCompoundSelectorComponent`; `selectorList()` → `SubSelectors()` (would hide the type); `tagQName`/`tagLowercaseLocalName` → `TagQualifiedName`/`TagLocalNameLower`; `MutableCSSSelector::selector()` → `GetSelector()`; `prependInComplexSelector*` → `PrependComplexSelectorComponent*`; `CSSSelectorList::makeCopying{Simple,Complex}Selector`/`makeJoining`/`first`/`selectorAt`/`listSize`/`selectorsText` → `CopySimple`/`CopyComplex`/`Join`/`FirstSelectorComponent`/`SelectorComponentAt`/`SelectorCount`/`SelectorsText`. `AttributeMatchType` → `IsCaseSensitive`; `setValue(value, bool)` → `MatchLowercase`.

**Bugs fixed:**
- `Selector` had no copy constructor: the implicit one copied the union's raw pointers without a ref, so copying a `SelectorList` (or the sub-selectors in `RareData`) double-freed the rare data and the tag's `QualifiedName` storage. Now upstream's (deep-copies rare data), plus the `MutableSelectorCopy` constructor.
- A default-constructed `Selector` left its match, relation and pseudo type uninitialised (upstream: `Unknown`, `DescendantSpace`, 0).
- `EnumTraits<SelectorMatch>` said 14 values (there were 17; 18 with `PagePseudoClass`).
- `SetAttribute` only ever set the case-insensitive flag, never cleared it.
- `SelectorList::SelectorText()` popped two characters off an empty string for an empty list.
- `SelectorText` didn't serialise pseudo-classes, pseudo-elements or page pseudo-classes at all, and missed the implicit relative-nesting separator (`> .a`).
- `RareData::NthA`/`NthB` were `int64` (upstream `int`).
- `MutableSelector::NeedsImplicitShadowCombinatorForMatching()` was a stub returning `false`.
- DOM: `dom::AnyQualifiedName()` passed `(Null, *, *)` as (namespace, prefix, local name), giving prefix `*` and no namespace; upstream's `anyQName` has no prefix and namespace `*`. Fixed (tested in `tests/DOM/Internals/QualifiedNameCache.cpp`); it would have broken the parser's `*|` handling.
- Used the unreviewed `SerialiserIdioms` (UTF-32) and raw `assert`; now `Markup.hpp` and `krys_debug_assert`.

**Upstream bugs found:**
- `visitSimpleSelectors` with `VisitOnlySubject::Yes` has its condition inverted: it stops inside the subject compound and crosses combinators, the opposite of its comment. Nothing upstream passes `Yes`. Fixed per the comment (tested).
- `specificityForPage` reads `tagQName()` of `this` instead of the component being visited. Harmless upstream (a type selector is always first in its compound); fixed.

**Added (upstream API):** `Selector`: copy constructors, `ComputeSpecificity`/`ComputeSpecificityTuple`/`SpecificityForPage` (with the saturating `SelectorSpecificity`), `StylePseudoElementTypeFor`, `ParsePseudoElementName`, the generated pseudo statics, `PagePseudoClass`, `IsForPage`/`SetForPage`, `SetImplicit`, `SetLastInSelectorList`, `HasExplicitPseudoClassScope`, `HasScope`, `IsHostPseudoClass`, `IsScopePseudoClass`, `IsSiblingSelector`, `MatchesPseudoElement`, `HasDescendant(OrChild)Relation`, non-const `SubSelectors`; `VisitSimpleSelectors`/`HasExplicitNestingParent` are public. Free: `ComplexSelectorCanMatchPseudoElement`, `ComplexSelectorMatchesElementBackedPseudoElement`, `IsElementBackedPseudoElement` (replaces the deleted d1a stub), `PseudoClassIsRelativeToSiblings`, `IsTreeStructuralPseudoClass`, `IsLogicalCombinationPseudoClass`, `SelectorSpecificityIncrement`, `operator==(PossiblyQuotedIdentifier, atom)`. `SelectorRelation::ShadowDescendant`/`ShadowPartDescendant`/`ShadowSlotted`, `SelectorMatch::PagePseudoClass`, `SelectorFlag::IsForPage`. `SelectorList`: `IndexOfNextSelectorAfter`, `BuildSelectorsText`, move assignment. `MutableSelector`: `ParsePseudoClassSelector`/`ParsePseudoElementSelector`/`ParsePagePseudoSelector`, the copy-from-`Selector` constructors, `AdoptSelectorVector`, `SetArgument`/`SetNth`/`SetArgumentList`/`SetLangList`/`SetSubSelectors`/`SetForPage`/`SetImplicit`, `MatchesPseudoElement`, `IsHostPseudoClass`, `HasExplicitNestingParent`/`HasExplicitPseudoClassScope`, `StartsWithExplicitCombinator`, `PrependComplexSelectorComponentAsRelative`, `ReleaseFromComplexSelector`, const `LeftmostSimpleSelector`.

**Differences:**
- One `uint8` pseudo type, enums stored whole and a `SelectorFlag` set instead of upstream's bitfields; still 16 bytes (`static_assert`ed). Atoms are interned for the program's lifetime, so a plain value isn't ref-counted (upstream refs its `AtomStringImpl`).
- `add(Hasher&, ...)` → `HashComplexSelector(selector, mode)` and `DefaultHash<SelectorList>`; atoms and qualified names hash by their interned pointer.
- `IndexOfNextSelectorAfter` returns `Maybe<size_t>` (upstream `notFound`).
- `MutableSelectorList` is a `List` (upstream `Vector`; was `SmallList`); `Join` takes a `List`; argument and lang lists are `FixedVector` (were `SmallList`).
- No `UserAgentPartLegacyAlias`/`WebKitUnknown`/`WebKitAny` cases (dropped in d1a): `ParsePseudoElementName` returns nothing for unknown `-webkit-` names, and `nameForUserAgentPartLegacyAlias` isn't needed. `::cue` is always element-backed (upstream: `ENABLE(VIDEO)`).
- `PseudoElementType` has only the standard pseudo-elements: no `WebKitScrollbar*`, `WebKitResizer` or `InternalWritingSuggestions`; `allPublicPseudoElementTypes` waits for Style.
- No `destructorHasBeenCalled` debug bit; no `TextStream` printing.
- `SelectorParserContext`: the `Document` constructor is a FIXME (needs `Settings` and quirks); no hash/`HashTraits` yet (comes with whatever caches on it). The unused `DefaultCaseSensitivity` member is removed. Settings flags keep upstream's names, as in `ParserContext`.
- Specificity of a bare `&` hits a debug assertion, as upstream (nesting is resolved before specificity is computed).

Tests: `tests/CSS/Selectors/{Selector,SelectorList,MutableSelector}.cpp` (builder in `include/Krystal.Booey.Tests/CSS/SelectorTestUtils.hpp`): 33 cases. Full Booey suite: 1548 cases / 674451 assertions; all pass except 4 expected failures. Whole solution builds with no warnings.

### Layer d1c: the selector parser (2026-10-13)

Graph: the d1a graph (`tools/port/out/selectors.md`) covers `CSSSelectorParser`. The Krystal parser was an own partial port (no pseudo-class/pseudo-element arguments, no An+B, no nesting resolution, no `@supports`); it's now a full port of upstream. Decided with the user: the namespace half of `StyleSheetContents` is pulled forward from d4.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSSelectorParser.h`, `.cpp` | `have` | `CSS/Selectors/SelectorParser.hpp`, `src/.../SelectorParser.cpp` | on | tested. See bugs and differences. |
| `css/parser/CSSParserEnum.h` | `have*` | `CSS/Parser/ParserEnum.hpp` | on | `NestedContextType` → `NestedParserContextType` (moved out of the unreviewed `NestedParserContext.hpp`), `NestedContext` = `Maybe<...>`; `IsForgiving` → the existing `IsForgivingSelectorList`. |
| `css/StyleSheetContents.h`, `.cpp` (namespaces) | `have` (namespaces only) | `CSS/StyleSheetContents.hpp`, `src/CSS/StyleSheetContents.cpp` | on | tested. `defaultNamespace`, `namespaceURIFromPrefix`, `parserAddNamespace`; the rest is a FIXME for d4. The old CSSOM-based members (import/namespace/child `CSSRule` lists, `InsertRule`/`DeleteRule`) are removed: d4 replaces them with the rule model. |
| `style/PseudoElementIdentifier.h` | `have` | `Style/PseudoElementIdentifier.hpp` | on | tested. Hash → `DefaultHash`; no `HashTraits`/`TextStream`. |

**Bugs fixed (old Krystal parser):**
- `ConsumePseudo` was a stub returning null, so no pseudo-class or pseudo-element parsed; `IsSimpleSelectorValidAfterPseudoElement` asserted.
- No `_failedParsing`, so an invalid simple selector inside a compound (`.a[`) or a bad namespace prefix didn't fail the list; forgiving lists didn't reset it, and didn't store the unknown selector's text (so it serialised as nothing).
- `ConsumeName` and `ConsumeAttributeMatch` read delimiters through `IdentCodePoints()` (empty for delimiters), so `*`, `|` and every attribute operator misbehaved; `*|*` checked for an ident token.
- `ConsumeAttributeMatch` allowed whitespace inside an operator (`[a~ =b]`); the attribute flag accepted `s` without storing it.
- Nested lists never got their implicit `&`/`:scope`; no `SplitCompoundAtImplicitShadowCrossingCombinator`, so `::part()`, `::slotted()`, `::cue` and user agent parts weren't split into their own compound; the "pseudo-element in the previous compound" check didn't honour the UA-sheet exemption.
- `QualifiedName` arguments were already in Krystal's (namespace, prefix, local name) order, but `AnyQualifiedName()` was wrong (fixed in d1b), so `PrependTypeSelectorIfNeeded` added a needless universal selector.

**Added (upstream API):** `ConsumeComplexSelectorList`/`ConsumeNestedSelectorList`/`ConsumeComplexForgivingSelectorList`/`ConsumeNestedComplexForgivingSelectorList` are public, `ParseSelectorList(string)`, `SupportsComplexSelector`, `ResolveNestingParent`, `ParsePseudoElement`; free `ParseSelectorList(tokens)` and `ParseMutableSelectorList`; An+B parsing and serialisation; `:not`/`:is`/`:where`/`:has`/`:host`/`:nth-*(... of)`/`:lang`/`:dir`/`:state`/`:active-view-transition-type` and `::cue`/`::highlight`/`::view-transition-*`/`::part`/`::slotted` arguments.

**Differences:**
- Krystal's tokenizer has no match tokens (CSS Syntax 3), so `~=`, `|=`, `^=`, `$=`, `*=` are two delimiters with no whitespace allowed between them, and `ConsumeName` treats `|` followed by `=` as the dash-match operator rather than a namespace separator (upstream gets a `DashMatchToken`).
- Dropped with the non-standard pseudos (d1a): `isScrollbarPseudoClass` and the scrollbar/`::selection:window-inactive` cases of `isPseudoClassValidAfterPseudoElement`; `containsUnknownWebKitPseudoElements` (no `WebKitUnknown`, so `SupportsComplexSelector` only checks the parse); the `-webkit-media-text-track-display` quirk (the part doesn't exist). The ident `::cue` → `UserAgentPart` conversion is kept (always on; upstream `ENABLE(VIDEO)`).
- `extractCompoundFlags`'s `OptionSet<CompoundSelectorFlag>` (one flag) → a `bool`.
- An+B integers are clamped to `int32` (upstream `static_cast<int>` of the `double`, undefined when out of range); the `n-123` integer is parsed with `std::from_chars` (strict, as WTF's `parseInteger` for these inputs).
- `ParseSelectorList`'s string overload takes a `SelectorParserContext` (upstream a `CSSParserContext`; `SelectorParserContext(const ParserContext &)` converts).
- Serialising `*|rect` with no default namespace gives `rect` (the prefix maps to the default namespace and is dropped), as upstream.
- FIXME: the `s` attribute flag (Selectors 4) is rejected, as upstream; supporting it needs a selector flag and serialisation.

**Known failures (tests tagged `[!shouldfail]`):**
- `:heading`/`:heading()` (Selectors 5, `data/css-selector-parser/heading-01.dat`) aren't in upstream's pseudo-class table at this revision.
- `:host(:not(.a .b))` parses but WPT `parse-not.html` says it's invalid; upstream fails the same test (`parse-not-expected.txt`). Moved to `not-03.dat`.

**Test data fixed:** `an-plus-b-01.dat` had WPT's JavaScript `\n` escapes copied literally (now real newlines) and a corrupted expected output (`:nth-childsource(n+3)`).

Tests: `tests/CSS/Selectors/SelectorParser.cpp` (the `data/css-selector-parser/*.dat` suite, re-enabled, and API tests for namespaces, nesting, `ResolveNestingParent`, `SupportsComplexSelector`, `ParsePseudoElement`, forgiving lists, quirks mode, functional pseudo-classes and pseudo-elements): 35 cases. Full Booey suite: 1583 cases / 675018 assertions; all pass except 6 expected failures. Whole solution builds with no warnings.

### Layer d2: declaration blocks (2026-10-13)

Graph from `css/StyleProperties.cpp`, `MutableStyleProperties.cpp`, `ImmutableStyleProperties.cpp` and `ShorthandSerializer.cpp` (headers-only): 106 files (`tools/port/out/declarations.md`), nearly all done. New for d2: the four entries, `CSSPropertyInitialValues.cpp` and the review of `CSSProperty.h`. Boundaries: `CSSParser.h` (d5), `CSSStyleProperties.h` (CSSOM wrappers), `StyleExtractor.h` (Style), `FontSelectionValueInlines.h` (not needed: see differences). The Krystal files were an own partial port (headers mostly upstream's, no `Mutable`/`Immutable`/serialiser `.cpp`, never built). Decided with the user: the `CSSParser`-dependent methods are declared and FIXME'd until d5, the CSSOM wrapper half is removed until the CSSOM slice, `Property.hpp` is reviewed and `PropertyInitialValues.cpp` ported here, and `Packed.hpp` (Krystal.Core) is fixed and tested.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/StyleProperties.h`, `.cpp`, `StylePropertiesInlines.h` | `have` | `CSS/Properties/StyleProperties.hpp`, `StylePropertiesInlines.hpp`, `src/.../StyleProperties.cpp` | on | tested. |
| `css/MutableStyleProperties.h`, `.cpp` | `have` (no CSSOM wrapper; string setters undefined) | `CSS/Properties/MutableStyleProperties.hpp`, `src/.../MutableStyleProperties.cpp` | on | tested. |
| `css/ImmutableStyleProperties.h`, `.cpp` | `have` | `CSS/Properties/ImmutableStyleProperties.hpp`, `src/.../ImmutableStyleProperties.cpp` | on | tested, including deduplication. |
| `css/ShorthandSerializer.h`, `.cpp` | `have` (no `Style::Extractor` overload) | `CSS/Properties/ShorthandSerialiser.hpp`, `src/.../ShorthandSerialiser.cpp` | on | tested, every shorthand family. |
| `css/CSSPropertyInitialValues.h`, `.cpp` | `have` | `CSS/Properties/PropertyInitialValues.hpp`, `src/.../PropertyInitialValues.cpp` | on | tested. Uses the generated `InitialValues.hpp`. |
| `css/CSSProperty.h`, `.cpp` | `have` (reviewed) | `CSS/Properties/Property.hpp`, `src/.../Property.cpp` | on (already) | See bugs and differences. |
| `wtf/Packed.h` | `have` | Krystal.Core `Types/Packed.hpp` | header | tested (`Krystal.Core.Tests/tests/Types/Packed.cpp`). See bugs. |

**Names:** `serializeShorthandValue`/`serializeLonghandValue` → `SerialiseShorthandValue`/`SerialiseLonghandValue`; `PropertyReference::toCSSProperty` → `ToProperty`, `protectedValue` → `ProtectedValue`; `cssParserMode()` → `ParserMode()`; `isEmpty` → `empty` (container protocol, as before); `ImmutableStyleProperties::objectSize` → `ObjectSize`. `PropertyMetadata` field names are Krystal's (`Id`, `Flags`, `ShorthandIndex`; upstream bitfields `m_propertyID`, `m_important`, ...).

**Bugs fixed (old Krystal files):**
- `Property::Metadata()` returned by value, so every `PropertyReference` from a `MutableStyleProperties` held a dangling reference to a temporary.
- `StyleProperties.hpp` forward-declared `css::CachedResource` (it's `boo::CachedResource`) and a stray `css::Color`; the free `SerialiseLonghandValue(..., const Value &)` was declared `inline` but defined in the `.cpp`; `LonghandValueId(PropertyId, const Value *)` lacked the declared `noexcept`; `PropertyReference::CssText` didn't return.
- The shorthand serialiser returned a plain string and `AsTextInternal` tested `value.empty()` where upstream tests `isNull()`, so a shorthand that couldn't be serialised looked like one that serialised to the empty string. Now `Maybe<CSSOMString>` (see differences).
- `ImmutableStyleProperties` was allocated with the object's variable size but deleted with a sized `delete`; it now has an unsized class `operator delete`. Metadata and values are placement-constructed.
- `MutableStyleProperties`, `ImmutableStyleProperties`, `ShorthandSerialiser` and `PropertyInitialValues` had no `.cpp` (and the last one's header forward-declared `CSSOMString` as a class).
- Krystal.Core `Packed.hpp` had never compiled: `PackedAlignedPtr::get()`/`set()` copied in the wrong direction through a span of a by-value lambda parameter, `set()` `static_assert`ed a runtime value, the implicit "unspecified bool" conversion pointed at a member of the wrong type, `swap(nullptr)` called a missing `clear()`, and the `HashTableDeletedValueTag` constructor misplaced `noexcept`.
- Krystal.Core `Detection/CPU.hpp` defined `KRYS_CPU_BIG_ENDIAN` under MSVC (`__BYTE_ORDER__` and `__ORDER_BIG_ENDIAN__` are both undefined there, so `0 == 0`). Only `Packed.hpp` uses it.

**Upstream bugs found:**
- `serializeAnimationRange` returns an empty (non-null) string when `animation-range-start` and `animation-range-end` have different list lengths (including one list and one single value), so `cssText` emits `animation-range: ;` instead of the longhands. Fixed: any length mismatch can't be serialised (tested).

**Added (upstream API):** everything in the four `.cpp`s and `CSSPropertyInitialValues.cpp`. `PropertyReference::ProtectedValue`/`ToProperty` and the mutable `Value()` overload; `KRYS_LIFETIME_BOUND` on the property accessors. `PackedAlignedPtr::clear()`.

**Differences:**
- Null strings: `SerialiseShorthandValue` returns `Maybe<CSSOMString>` (`null` = can't be represented by the shorthand); `GetPropertyValue` returns `""` for it. Inside the serialiser, `SerialiseCommonValue`, `SerialiseBorder` and the other null-returning helpers also use `Maybe`.
- Indices are `Maybe<uint32>` (upstream `int`, -1 for not found); counts are `uint32`.
- Removed with the CSSOM wrappers (FIXME): the `_cssomWrapper` member, `cssStyleProperties()`, `ensureCSSStyleProperties()`, `ensureInlineCSSStyleProperties()`; `HasCSSOMWrapper()` returns `false`.
- Declared, not defined, until d5 (FIXME): `SetProperty(id, string, ...)` (both overloads), `SetCustomProperty`, `ParseDeclaration`. Tests build blocks with `PropertyParser`/`VariableParser` and `AddParsedProperties` (`StylePropertiesTestUtils.hpp`).
- No `Style::Extractor` overload of `SerialiseShorthandValue` (FIXME; comes with computed style).
- Dropped with the legacy `-webkit-` shorthands: the `WebkitMask`, `WebkitBorderImage`, `WebkitMaskBoxImage`, `WebkitBorderRadius`, `WebkitColumnBreak*`, `WebkitPerspective`, `WebkitTextOrientation` and `WebkitTextStroke` cases and `serializeColumnBreak`; the `-webkit-mask-clip` layer case. So `border-image` only checks that a `BorderImageWidthValue` doesn't override the border widths (only the legacy shorthand could set that).
- The font shorthand's width percentage → keyword conversion matches the nine keyword percentages exactly (`FontWidthKeyword(double)`) instead of round-tripping through `FontSelectionValue`; the results are the same (the keyword values are exactly representable, and anything that isn't failed the round trip upstream). `FontSelectionValue` isn't ported.
- `InitialValueTextForLonghand` also handles `0deg` (`glyph-orientation-horizontal`; upstream hits `ASSERT_NOT_REACHED`, though only an implicit initial value would reach it).
- Deduplication hashes with `Hash::Combine` into a `HashMap<size_t, ...>` and evicts the first entry when full (upstream: a random one).
- `RemoveProperties` searches the span (upstream builds a `HashSet`).
- `Property`'s constructor takes `(id, value, important, implicit, setFromShorthand, shorthandIndex)` (upstream `(id, value, important, isSetFromShorthand, index, implicit)`); `Property::IsShorthand`/`IsLonghand` (not upstream; duplicated the generated free functions) are removed.
- `PackedPtr` doesn't assert `AllowsCompactPointers` (Krystal has no compact heap; WTF only allows it for fast-malloc'd types). The implicit bool conversion is gone; `explicit operator bool` remains. `get`/`set` aren't `constexpr` (`bit_cast` of a pointer isn't).
- Shorthand serialisations follow upstream where browsers differ, e.g. `background-size: 10px` is the pair `10px auto`, `animation: none` serialises as `auto`, `outline: none` as `currentcolor`, `offset: none` as `normal` (all accepted by WPT).

Tests: `tests/CSS/Properties/{StyleProperties,ShorthandSerialiser,PropertyInitialValues}.cpp` (helpers in `include/Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp`): 20 cases / 1978 assertions. `Krystal.Core.Tests/tests/Types/Packed.cpp`: 4 cases / 35 assertions. Full Booey suite: 1603 cases / 676996 assertions; all pass except 6 expected failures. Core suite: 554 cases pass. Whole solution builds with no warnings.

### Layer d3: media and container queries (2026-10-13)

Graph from the nine d3 entries (`MediaQueryParser.cpp`, `ContainerQueryParser.cpp`, `GenericMediaQuerySerialization.cpp`, `MediaQueryFeatures.cpp`, `ContainerQueryFeatures.cpp`, `ContainerQuery.cpp`, `MediaQueryParserContext.cpp`, `MediaList.cpp`, `CSSSupportsParser.cpp`; headers-only): 146 files (`tools/port/out/conditions.md`). Nearly all of it is the evaluators' reach (`ContainerQueryFeatures.cpp` pulls in `StyleBuilder`, `RuleSet` and `SelectorChecker`); the parse side needs only done files. The Krystal headers were an own port close to upstream (no sources, never built). Decided with the user: `CSSSupportsParser` moves to d5 (it needs `CSSParser::supportsDeclaration`), `MediaList` to the CSSOM slice (it only wraps a parent sheet or rule), and the feature schemas are plain `FeatureSchema`s until evaluation lands.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/query/GenericMediaQueryTypes.h` | `have` | `CSS/Queries/GenericMediaQueryTypes.hpp` | on | tested. |
| `css/query/MediaQuery.h` | `have` | `CSS/Queries/MediaQuery.hpp` | on | tested. |
| `css/query/GenericMediaQueryParser.h`, `.cpp` | `have` | `CSS/Queries/GenericMediaQueryParser.hpp`, `src/.../GenericMediaQueryParser.cpp` | on | tested. |
| `css/query/GenericMediaQuerySerialization.h`, `.cpp` | `have` | `CSS/Queries/GenericMediaQuerySerialisation.hpp`, `src/...` | on | tested. |
| `css/query/MediaQueryParser.h`, `.cpp` | `have` | `CSS/Queries/MediaQueryParser.hpp`, `src/...` | on | tested. |
| `css/query/MediaQueryFeatures.h`, `.cpp` | `have*` (schemas only) | `CSS/Queries/MediaQueryFeatures.hpp`, `src/...` | on | tested. See differences. |
| `css/query/ContainerQuery.h`, `.cpp` | `have` | `CSS/Queries/ContainerQuery.hpp`, `ContainerQueryAxis.hpp`, `src/...` | on | tested. `Axis` is in its own header (the Style seam uses it). |
| `css/query/ContainerQueryParser.h`, `.cpp` | `have` | `CSS/Queries/ContainerQueryParser.hpp`, `src/...` | on | tested. |
| `css/query/ContainerQueryFeatures.h`, `.cpp` | `have*` (schemas only) | `CSS/Queries/ContainerQueryFeatures.hpp`, `src/...` | on | tested. |
| `css/MediaQueryParserContext.h`, `.cpp` | `have` (no `Document` constructor) | `CSS/Queries/MediaQueryParserContext.hpp`, `src/...` | on | tested through the parsers. |
| `css/query/GenericMediaQueryEvaluator.*`, `MediaQueryEvaluator.*`, `css/MediaQueryMatcher.*`, `style/ContainerQueryEvaluator.*` | | | | Evaluation; see *Media query evaluation*. |
| `css/MediaList.h`, `.cpp` | `?` | `CSS/Media/MediaList.hpp` | off | CSSOM slice. |
| `css/parser/CSSSupportsParser.h`, `.cpp` | `port` | — | — | d5. |
| `css/parser/MediaQueryBlockWatcher.h`, `.cpp` | `wont` | deleted | — | Dead upstream: nothing includes it. |

**Names:** `MQ`/`CQ` → `css::mq`/`css::cq`; `serialize` → `Serialise`; `FeatureSchema::type`/`valueType` → `FeatureType`/`FeatureValueType` (the nested enums are `Type`/`ValueType`); members that share their type's name are written `mq::Syntax Syntax`, `mq::Condition Condition`, ...; `MediaQueryResult::mediaQueryList` → `MediaQueryList`.

**Bugs fixed (old Krystal headers):**
- `GenericMediaQueryParser` forward-declared `MediaQueryParserContext` in `css::mq` (it's in `css`), and built a `GeneralEnclosed` from a `CSSOMStringAtom` (it holds strings).
- `Feature`, `MediaQuery` and `ContainerQuery` couldn't be default-constructed (`StringAtom` has no default constructor), so `TraverseFeatures`' dummy feature and `Condition {}` didn't compile; they now default to the null atom.
- `EnumTraits<MediaQueryDynamicDependency>` said 4 values (there are 3).
- `Comparison::Value`, `Condition::LogicalOperator` and similar members hid their own types.
- `ContainerQuery.hpp` declared `cq::FeatureSchemas::Width()` etc., which are never defined (upstream declares them too; the real ones are `cq::Features`). Removed.
- Lists were `SmallList` (upstream `Vector`); now `List`. `FeatureSchema::ValueIdentifiers` is a `FixedVector`, as upstream.

**Differences:**
- Non-standard features are dropped: `-webkit-animation`, `-webkit-device-pixel-ratio` (and its `min-`/`max-` forms), `-webkit-transform-2d`, `-webkit-transform-3d`, `-webkit-transition`, `-webkit-video-playable-inline` and `prefers-dark-interface`. So `MediaQueryParser` has no `schemaForFeatureName` override (it only gated `prefers-dark-interface`), and the plain-feature parser doesn't rewrite `-webkit-min-`/`-webkit-max-`. Each of these now parses as `<general-enclosed>` (valid, evaluates to unknown).
- `display-mode` and `prefers-color-scheme` are always on (upstream `ENABLE(APPLICATION_MANIFEST)`/`ENABLE(DARK_MODE_CSS)`); the codegen now also passes `ENABLE_APPLICATION_MANIFEST`, which only adds the `fullscreen`, `standalone`, `minimal-ui` and `browser` keywords.
- Schemas are plain `FeatureSchema`s whose `Evaluate` returns `Unknown` (FIXME); upstream's `BooleanSchema`/`IntegerSchema`/.../`SizeFeatureSchema`/`StyleFeatureSchema` subclasses and their value functions come with evaluation.
- `OptionSet<MediaQueryDynamicDependency>` and `OptionSet<Axis>` are flags enums with a `None` value.
- `GenericMediaQueryParser::ValidateFeature` passes the `MediaQueryParserContext` to `SchemaForFeatureName` (upstream passes the inner `CSSParserContext`, which converts back through the implicit constructor, copying it).
- `ConsumeTrailingImportantAndWhitespace` is a local copy of `CSSParser`'s until d5 (FIXME).
- `MediaQueryParser::Parse(string)` takes a `CSSOMStringView` and always tokenizes (Krystal's tokenizer can't fail, unlike `CSSTokenizer::tryCreate`).
- Serialising a name-only container query gives `--foo ` (trailing space), as upstream.

**Known failures (tests tagged `[!shouldfail]`):**
- Range syntax in `style()` and lists of container queries (css-conditional-5) aren't supported; they fail upstream too (`at-container-style-parsing-expected.txt`, `at-container-parsing-expected.txt`).
- `style()` values keep runs of whitespace and a number's original text since the tokenizer fix (2026-10-13). The `( prop: val  )` case fails upstream too (`[!shouldfail]`).

Tests: `tests/CSS/Queries/{MediaQueryParser,ContainerQueryParser}.cpp` (WPT `match-media-parsing.html`, `aspect-ratio-serialization.html`, `mq-invalid-media-type-*`, `at-container-parsing.html`, `at-container-style-parsing.html`, `at-container-serialization.html`, `at-container-style-serialization.html`, plus schema, feature and API tests): 19 cases / 416 assertions. Full Booey suite: 1622 cases / 677412 assertions; all pass except 8 expected failures. Whole solution builds with no warnings.

### Layer d4: the rule model (2026-10-13)

Graph from the seven d4 entries (`StyleRule.cpp`, `StyleRuleImport.cpp`, `StyleRuleFunction.cpp`, `parser/CSSAtRuleID.cpp`, `CSSCounterStyleDescriptors.cpp`, `StyleSheet.cpp`, `StyleSheetContents.cpp`; headers-only): 99 files (`tools/port/out/rules.md`). New for d4: the rule files, `platform/graphics/FontFeatureValues`/`FontPaletteValues` and the counter style descriptors. Outside the slice: `StyleRule.cpp` includes every CSSOM rule wrapper (only for `createCSSOMWrapper`), and `StyleSheetContents.cpp` reaches `CSSParser` (d5), `style/RuleSet`/`RuleData` (style resolution), `CachedCSSStyleSheet`, `FrameLoader` and `SecurityOrigin` (loader) and `Node`/`Document`. Upstream keeps five internal rules in CSSOM headers (`StyleRuleKeyframe`, `StyleRuleKeyframes`, `StyleRuleCounterStyle`, `StyleRuleViewTransition`, `StyleRulePositionTry`); here they're in `Rules/`, like the rest of the rule model. Decided with the user: `@-internal-base-appearance` is dropped (as the `-internal-` pseudos in d1a), and `css/StyleSheet.h` (the CSSOM abstract base) moves to the CSSOM slice.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/StyleRule.h`, `.cpp` | `have` (no CSSOM wrappers) | `CSS/Rules/Rule.hpp`, `src/.../Rule.cpp` | on | tested. |
| `css/StyleRuleType.h` | `have` | `CSS/Rules/RuleType.hpp` | on | tested. See bugs. |
| `css/StyleRuleImport.h`, `.cpp` | `have*` (no loading) | `CSS/Rules/ImportRule.hpp`, `src/...` | on | tested. |
| `css/StyleRuleFunction.h`, `.cpp` | `have` | `CSS/Rules/FunctionRule.hpp`, `src/...` | on | tested. |
| `css/CSSKeyframeRule.h`, `.cpp` (`StyleRuleKeyframe`) | `have` | `CSS/Rules/KeyframeRule.hpp`, `src/...` | on | tested. |
| `css/CSSKeyframesRule.h`, `.cpp` (`StyleRuleKeyframes`) | `have` | `CSS/Rules/KeyframesRule.hpp`, `src/...` | on | tested. |
| `css/CSSCounterStyleRule.h`, `.cpp` (`StyleRuleCounterStyle`, `toCounterStyleSystemEnum`) | `have` | `CSS/Rules/CounterStyleRule.hpp`, `src/...` | on | tested. |
| `css/CSSViewTransitionRule.h`, `.cpp` (`StyleRuleViewTransition`) | `have` | `CSS/Rules/ViewTransitionRule.hpp`, `src/...` | on | tested. |
| `css/CSSPositionTryRule.h`, `.cpp` (`StyleRulePositionTry`) | `have` | `CSS/Rules/PositionTryRule.hpp`, `src/...` | on | tested. |
| `css/CSSCounterStyleDescriptors.h`, `.cpp` | `have` | `CSS/Counter/CounterStyleDescriptors.hpp`, `src/...` | on | tested. See names. |
| `css/parser/CSSAtRuleID.h`, `.cpp` | `have` | `CSS/Parser/AtRuleId.hpp`, `src/...` | on | tested. |
| `css/StyleSheetContents.h`, `.cpp` | `have*` (parse side) | `CSS/StyleSheetContents.hpp`, `src/...` | on | tested. See differences. |
| `platform/graphics/FontFeatureValues.h`, `.cpp` | `have` | `Graphics/FontFeatureValues.hpp`, `src/...` | on | tested. |
| `platform/graphics/FontPaletteValues.h` | `have` | `Graphics/FontPaletteValues.hpp` | on | tested. |
| `css/StyleSheet.h`, `.cpp` | `?` | `CSS/StyleSheet.hpp` | off | CSSOM slice. |
| `css/parser/CSSParserEnum.h` | `have` | `CSS/Parser/ParserEnum.hpp` | on | Done in d1c. |

**Names:** `StyleRuleBase` → `Rule`, `StyleRuleX` → `XRule` (`StyleRuleFontFeatureValues` → `FontFeatureValuesRule`, `StyleRuleKeyframe` → `KeyframeRule`, ...); `StyleRule`, `StyleRuleWithNesting` keep their names and `StyleRuleNestedDeclarations` → `NestedDeclarationsRule`; `isStyleRuleWithNesting` → `IsStyleRuleWithNesting` (was `IsRuleWithNesting`). Getters that would hide a type: `selectorList()` → `GetSelectorList()`, `descriptor()` → `GetDescriptor()`, `parserContext()` → `GetParserContext()`, `cascadeLayerName()` → `GetCascadeLayerName()`, `styleSheetContents()` → `GetStyleSheetContents()`; the type is qualified where a getter keeps its name (`boo::FontPaletteValues`, `css::Selector`, `cq::ContainerQuery`). `CSSAtRuleID`/`cssAtRuleID` → `AtRuleId`/`AtRuleIdFromName`. `CSSCounterStyleDescriptors` → `CounterStyleDescriptors`, with its nested types lifted to namespace scope (`CounterStyleSystem`, `CounterStyleSymbol`, `CounterStylePad`, `CounterStyleNegativeSymbols`, `CounterStyleSpeakAs`, `CounterStyleName`, `CounterStyleRanges`, `CounterStyleSystemData`, `CounterStyleAdditiveSymbols`) because the PascalCase members (`Name`, `System`, `Pad`, ...) would clash with them; `ExplicitlySetDescriptors` (`OptionSet`) → the `CounterStyleDescriptor` flags enum; `*FromCSSValue` → `*FromValue`; `toCounterStyleSystemEnum` is declared in `CounterStyleDescriptors.hpp` (upstream `CSSCounterStyleRule.h`). `FontPaletteIndex::type`/`integer` → `IndexType`/`Integer`.

**Bugs fixed (old Krystal files):**
- `RuleType` didn't use the CSSOM `CSSRule.type` numbers (it started at `Unknown = 0` with `Style = 8`) and had a `Viewport` type upstream doesn't have.
- `PageRule::MutableProperties()` returned the declarations as mutable without copy-on-write, so an immutable block would have been modified through a `MutableStyleProperties &` it isn't.
- `Rule.hpp` had no `.cpp` (every constructor, `Create`, `Copy` and the destroying `operator delete` were undefined), lists were `SmallList` (upstream `Vector`), `ContainerRule::containerQuery()` wasn't PascalCase and `FontFeaturesValuesRule` had a typo.
- `Counter/CounterStyleDescriptors.hpp` was raw WebKit (`OptionSet`, `m_` members, a `char` literal for a `u8string` symbol), never compiled, with no `.cpp`.
- `Parser/AtRuleType.hpp` knew only `@charset`, `@import` and `@namespace` and defined a non-inline function in a header. Superseded by `AtRuleId`; deleted with the old parser in d5.

**Upstream bugs found:**
- `StyleSheetContents::wrapperInsertRule` inserts an `@namespace` rule at the sheet-wide `index` rather than at its index in the namespace list, so with `@import` rules before it the insert lands in the wrong place or out of bounds (`@import "a"; @namespace a url(a);` then `insertRule("@namespace c url(c)", 1)`). Fixed (tested).
- `StyleRuleCounterStyle::newValueInvalidOrEqual` and `CSSCounterStyleDescriptors::setSystem` are declared but never defined. Not ported.

**Differences:**
- Dropped: `@-internal-base-appearance` (`RuleType::InternalBaseAppearance`, `StyleRuleInternalBaseAppearance`, and `PropertyParser`'s `ConsumeInternalBaseAppearanceDescriptor`) and `@-webkit-keyframes`. Both are `AtRuleId::Invalid`.
- No `createCSSOMWrapper` (CSSOM slice; FIXME), no `debugDescription`/`TextStream` printing, no compiled selectors (JIT).
- `ImportRule` doesn't load: `RequestStyleSheet` is a FIXME no-op, so `StyleSheet()` stays null and `IsLoading()` false; no cached sheet, loader client, `setCSSStyleSheet` or `CanMakeWeakPtr` (only the client needed it).
- `StyleSheetContents`: `ParseString` is declared, not defined, until d5 (FIXME). Not ported until the loader or `CSSStyleSheet` (FIXME): `ParseAuthorStyleSheet`, `NotifyLoadedSheet`, `SubresourcesAllowReuse`, `IsLoadingSubresources`, `SingleOwnerNode`, `SingleOwnerDocument`, `StartLoadingDynamicSheet`. So `CheckLoaded` always completes a root sheet (there's never an owner node), and `TraverseSubresources` doesn't visit an import's cached sheet. `RuleData::maximumSelectorComponentCount` (8192) is a local constant until the rule set is ported. Client registration is kept (pointers only).
- `FontFeatureValues::Tags` is a `HashMap` (`std::unordered_map`), so the hash sums the entries (order-independent); upstream hashes in iteration order. No `TextStream` operator.
- `FontPaletteIndex` and `FontPaletteValues` have `explicit operator bool`.
- Keyframe offsets are written with `std::to_chars` (shortest fixed notation), as upstream's `StringBuilder::append(double)` for offsets in range.
- `KeyframeRule::Create` takes a `Span` of key pairs (the key list parser returns a `SmallList`).
- Indices and counts are `uint32`; out-of-range `RuleAt`/`WrapperInsertRule`/`WrapperDeleteRule` assert.
- As upstream, `KeyframesRule` copies share their keyframes, and `FunctionDeclarationsRule`/`FontFeatureValuesRule` copies share their declarations/values; every other rule copies deeply.

**Unbuilt files touched:** the five CSSOM headers (`CSSKeyframeRule.hpp`, `CSSKeyframesRule.hpp`, `CSSCounterStyleRule.hpp`, `CSSViewTransitionRule.hpp`, `CSSPositionTryRule.hpp`) lose their copies of the moved rules and include `Rules/` instead (mechanical). Left as they are: the old parser's `Parser/AtRuleType.hpp`, `AllowedRules.hpp`, `AllowedBlockRules.hpp`, `Rules/RuleFlag.hpp`, `Rules/ParsedRuleList.hpp` (d5); `Counter/CounterStyle.hpp` and `CounterStyleRegistry.hpp` still use the old descriptor names; `src/CSS/CSSRule.cpp` still uses `RuleType::Viewport` (CSSOM slice).

Tests: `tests/CSS/Rules/{Rule,KeyframesRule,CounterStyleRule,DescriptorRules}.cpp`, `tests/CSS/StyleSheetContents.cpp`, `tests/CSS/Parser/AtRuleId.cpp`, `tests/Graphics/{FontFeatureValues,FontPaletteValues}.cpp` (helpers in `include/Krystal.Booey.Tests/CSS/RuleTestUtils.hpp`): 25 cases / 1230 assertions. Full Booey suite: 1647 cases / 678642 assertions; all pass except 8 expected failures. Whole solution builds with no warnings.

### Layer d5: the sheet parser (2026-10-13)

Graph from `css/parser/CSSParser.cpp` and `css/parser/CSSSupportsParser.cpp` (headers-only): 124 files (`tools/port/out/css-parser.md`). Everything in it was already ported apart from the two parsers themselves, `html/parser/NestingLevelIncrementer.h` and `FontCustomPlatformData`'s two support queries; the CSSOM headers it reaches (`CSSStyleSheet.h`, `MediaList.h`) are only for the inspector or `parseInlineStyleDeclaration`. Decided with the user: `FontCustomPlatformData::SupportsFormat`/`SupportsTechnology` answer as upstream's FreeType/HarfBuzz port (WOFF2 and variation fonts on, no Graphite, no incremental) until there's a font backend.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/parser/CSSParser.h`, `.cpp` | `have` (no observer) | `CSS/Parser/Parser.hpp`, `src/CSS/Parser/Parser.cpp` | on | tested. Replaces the old spec-driven `Parser`. |
| `css/parser/CSSSupportsParser.h`, `.cpp` | `have` | `CSS/Parser/SupportsParser.hpp`, `src/...` | on | tested. |
| `html/parser/NestingLevelIncrementer.h` | `have` | `Types/NestingLevelIncrementer.hpp` | on | Was `NestedLevelIncrementer.hpp`. |
| `platform/graphics/FontCustomPlatformData.h` (`supportsFormat`, `supportsTechnology`) | `stub` | `Graphics/FontCustomPlatformData.hpp`, `src/...` | on | tested. Only the two support queries. |
| `css/parser/CSSParserTokenRange.cpp` (`consumeBlockCheckingForEditability`) | `have` | `Parser::ConsumeBlockCheckingForEditability` | on | tested. |
| `css/parser/CSSParserObserver.h`, `CSSParserObserverWrapper.h` | `wont` | | | Inspector. |

**Names:** `CSSParser` → `Parser`, `CSSSupportsParser` → `SupportsParser`; methods PascalCase (`parseStyleSheet` → `ParseStyleSheet`, `supportsCondition` → `SupportsCondition`, ...). Getters that would hide a type: `tokenizer()` → `GetTokenizer()`; `context()` → `Context()`. `NestingContext`'s members → `ParsedProperties`/`ParsedRules`; `topContext()` → `TopContext()`.

**Bugs fixed (old Krystal files):**
- `Parser/Parser.hpp`/`src/CSS/Parser.cpp` was an own implementation from the spec that built CSSOM objects directly and was mostly TODOs (only `@namespace`/`@import`/style rules); with `AllowedRules.hpp`, `AllowedBlockRules.hpp`, `AtRuleType.hpp`, `NestedParserContext.hpp`, `Rules/RuleFlag.hpp`, `Rules/ParsedRuleList.hpp` and its commented-out test, it's deleted.
- `NestingLevelIncrementer` was copyable (a copy decrements twice), took a `size_t` and wasn't `noexcept`; its file name had a typo.

**Upstream bugs found:**
- `parsePageSelector` dereferences the style sheet for the default namespace, but `parseRule` can be called without one (`CSSParser::parseRule("@page name {}", context, nullptr, ...)`). Krystal uses a sheet's initial default namespace (`*`) (tested).
- Kept as upstream: `@import ... supports()` is parsed in `CSS.supports()` mode, which falls back to general-enclosed and so is never invalid; the "discard the import rule" branch is unreachable, and `@import "a" supports(;)` is kept as not matching (tested).

**Differences:**
- No observer: the inspector entry points, the observer wrapper parameter and `ParsingStyleDeclarationsInRuleList` (only the observer used it) are dropped. No `consumeInternalBaseAppearanceRule` or `@-webkit-keyframes` (dropped in d4).
- No `parseInlineStyleDeclaration` until `Element` has a document with a parser context (FIXME).
- `ConsumeBlockCheckingForEditability` is a `Parser` member (upstream's is on `CSSParserTokenRange`), so `TokenRange` doesn't depend on `StyleSheetContents`. `-webkit-user-modify` is kept (no standard replacement).
- The tokenizer can't fail, so `GetTokenizer()` is never null (`SupportsCondition` keeps upstream's check).
- `ConsumeStringOrURI` only accepts a string inside `url(`; upstream reads the value of whatever single token is there.
- `@supports` condition text is trimmed of ASCII whitespace and the vertical tab only; upstream's `deprecatedIsSpaceOrNewline` also trims non-ASCII Unicode whitespace.
- Duplicate declarations are filtered into a list from the back and reversed (Krystal's `Property` has no default constructor); same order as upstream.

**Unbuilt files touched:** none. Left as they are: `src/CSS/CSSStyleSheet.cpp` still includes the deleted `Parser/Parser.hpp` and calls the old `Parser::ParseRule` (CSSOM slice).

Tests: `tests/CSS/Parser/{Parser,SupportsParser}.cpp`, `tests/Graphics/FontCustomPlatformData.cpp`: 42 cases / 725 assertions pass (the whitespace `[!shouldfail]` passes since the tokenizer fix). Full Booey suite: 1671 cases / 678937 assertions; all pass except 9 expected failures. Whole solution builds with no warnings.

## CSSOM — done (DOM integration deferred)

Graph from the 35 CSSOM `.cpp`s (`css/CSS*Rule*.cpp`, `CSSStyleSheet.cpp`, `StyleSheet.cpp`, `StyleSheetList.cpp`, `MediaList.cpp`, `CSSStyleDeclaration.cpp`, `CSSStyleProperties.cpp`, `PropertySetCSSDescriptors.cpp`, `CSSFunctionDeclarations.cpp`, `CSSNestedDeclarations.cpp`; headers-only): 183 files (`tools/port/out/cssom.md`). Apart from the CSSOM files themselves, almost everything in it was already ported. The new dependencies are integration points: `Document`, `Element`/`StyledElement`, `Node`, mutation observers, `Style::Scope`, `StyleResolver`, `StyleSheetContentsCache`, `RuleSet`, `CachedCSSStyleSheet`, the bindings and the Typed OM (`css/typedom`). The Krystal CSSOM files on disk (`CSS/CSSOM/*`, `CSSStyleSheet`, `StyleSheet`, `MediaList`, `CSSStyleDeclaration`, `CSSStyleProperties`, `Descriptors/*`) were unbuilt, partly raw WebKit or spec-driven sketches; each is rewritten from upstream.

**Decisions (2026-10-13/14):**
- The CSSOM connects to the DOM through the Style seam (`Style/Seam.hpp`): `style::Scope` and `style::Resolver` placeholders (nothing created a scope until e3 of the DOM integration ported `style::Scope`) and `Document*` functions for the base URL, URL completion, the parser context, security checks and keyframes notifications. Constructed sheets and sheets built from `StyleSheetContents` work fully; `<style>`/`<link>` ownership, inline styles, `adoptedStyleSheets` and `document.styleSheets` come with the DOM integration.
- One layer (sheet, declarations and every rule wrapper together), because `Rule::CreateCSSOMWrapper` needs every wrapper and several wrappers need the declarations.
- Dropped as non-standard: `DeprecatedCSSOMValue` (`getPropertyCSSValue`), the epub-cased IDL attributes and the `epub` prefix in script property names. `CSSInternalBaseAppearanceRule` is dropped with its rule (d4). Deferred: the Typed OM (`styleMap`), the computed style declaration and the async `replace()`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/StyleSheet.h`, `.cpp` | `have` | `CSS/StyleSheet.hpp` | on | tested. No `debugDescription`/`TextStream`. |
| `css/CSSStyleSheet.h`, `.cpp` | `have*` | `CSS/CSSStyleSheet.hpp`, `src/CSS/CSSStyleSheet.cpp` | on | tested. No `replace` (FIXME). `createInline` came with e4, adopting tree scopes with e5. |
| `css/MediaList.h`, `.cpp` | `have` | `CSS/Media/MediaList.hpp`, `src/...` | on | tested. |
| `css/CSSRule.h`, `.cpp` | `have` | `CSS/CSSOM/CSSRule.hpp`, `src/CSS/CSSOM/CSSRule.cpp` | on | tested. Was `src/CSS/CSSRule.cpp`. |
| `css/CSSRuleList.h`, `.cpp` | `have` | `CSS/CSSOM/CSSRuleList.hpp`, `src/...` | on | tested. |
| `css/CSSGroupingRule`, `CSSConditionRule`, `CSSMediaRule`, `CSSSupportsRule`, `CSSContainerRule`, `CSSScopeRule`, `CSSStartingStyleRule`, `CSSLayerBlockRule`, `CSSLayerStatementRule` (`.h`, `.cpp`) | `have` | `CSS/CSSOM/<same>.hpp`, `src/...` | on | tested. |
| `css/CSSStyleRule`, `CSSNestedDeclarations`, `CSSPageRule`, `CSSFontFaceRule`, `CSSKeyframeRule`, `CSSKeyframesRule` (`.h`, `.cpp`) | `have` | `CSS/CSSOM/<same>.hpp`, `src/...` | on | tested. No `styleMap` (Typed OM, FIXME). |
| `css/CSSImportRule`, `CSSNamespaceRule`, `CSSCounterStyleRule`, `CSSFontFeatureValuesRule`, `CSSFontPaletteValuesRule`, `CSSPropertyRule`, `CSSViewTransitionRule`, `CSSPositionTryRule`, `CSSFunctionRule`, `CSSFunctionDeclarations` (`.h`, `.cpp`) | `have` | `CSS/CSSOM/<same>.hpp`, `src/...` | on | tested. See bugs. |
| `css/CSSInternalBaseAppearanceRule.h`, `.cpp` | `wont` | | | Its rule was dropped in d4; the old header is deleted. |
| `css/CSSStyleDeclaration.h`, `.cpp` | `have` | `CSS/CSSStyleDeclaration.hpp`, `src/...` | on | tested. No `getPropertyCSSValue` or `settings()`. |
| `css/CSSStyleProperties.h`, `.cpp` | `have*` | `CSS/CSSStyleProperties.hpp`, `src/...` | on | tested. No `InlineCSSStyleProperties` (FIXME), epub names or deprecated value wrappers. |
| `css/PropertySetCSSDescriptors.h`, `.cpp` | `have` | `CSS/Descriptors/PropertySetCSSDescriptors.hpp`, `src/...` | on | tested. |
| `css/CSSFontFaceDescriptors`, `CSSPageDescriptors`, `CSSPositionTryDescriptors`, `CSSFunctionDescriptors` (`.h`, `.cpp`) | `have` | `CSS/Descriptors/<same>.hpp`, `src/...` | on | tested. |
| `css/StyleRule.cpp` (`createCSSOMWrapper`) | `have` | `Rule::CreateCSSOMWrapper` (`src/CSS/Rules/Rule.cpp`) | on | tested. |
| `css/MutableStyleProperties.h`, `.cpp`, `StyleProperties.cpp` (CSSOM wrapper half) | `have` | `MutableStyleProperties::CssStyleProperties`/`EnsureCSSStyleProperties`, `StyleProperties::HasCSSOMWrapper` | on | tested. No `ensureInlineCSSStyleProperties` (FIXME). |
| `style/StyleSheetContentsCache.h`, `.cpp` | `have` | `Style/StyleSheetContentsCache.hpp`, `src/...` | on | tested. |
| `style/StyleScope.h` | `port` | `Style/Scope.hpp` | on | Since e3 of the DOM integration. |
| `style/StyleResolver.h` | `stub` | `style::Resolver` in `Style/Seam.hpp` | on | Placeholder; scopes never create one. |
| `css/StyleSheetList.h`, `.cpp` | `port` | `CSS/StyleSheetList.hpp`, `src/...` | on | Since e4 of the DOM integration. |
| `css/CSSStyleSheetObservableArray.h`, `.cpp` | `port` | `CSS/CSSStyleSheetObservableArray.hpp`, `src/...` | on | Since e5 of the DOM integration. |
| `css/CSSComputedStyleDeclaration.h`, `.cpp` | `?` | | | Needs the Style builder and `RenderStyle`. |
| `css/DeprecatedCSSOM*` | `wont` | | | Non-standard (`getPropertyCSSValue`). |
| `css/typedom/*` | `?` | | | The Typed OM, a slice of its own. |
| `css/StyleAttributeMutationScope.h` | `?` | | | Inline styles (mutation records for the `style` attribute). |

**Names:** the CSSOM classes keep their IDL names (`CSSStyleSheet`, `CSSRule`, `CSSMediaRule`, `MediaList`, ...); methods are PascalCase, setters `Set*` (`setMediaText` → `SetMediaText`, `setDisabled` → `SetDisabled`). Getters that would hide a type: `styleSheet()` (`CSSRuleList`, `CSSImportRule`) → `GetStyleSheet()`, `styleDeclarationType()` → `GetStyleDeclarationType()`, `ruleType()` (descriptors) → `GetRuleType()`, `parserContext()` → `GetParserContext()`, `nestedContext()` → `GetNestedContext()`, `groupRule()` → `GetGroupRule()`, `styleRule()` → `GetStyleRule()`, `styleRuleContainer()`/`styleRuleScope()`/`styleRuleFunction()` → `GetContainerRule()`/`GetScopeRule()`/`GetFunctionRule()`. `styleRuleType()` → `StyleRuleType()`; `typeForCSSOM()` → `TypeForCSSOM()`; `styleScope()` → `StyleScope()`; `propertyValueFor*IDLAttribute` → `GetPropertyValueFor*IDLAttribute`; `getCSSPropertyIDFromJavaScriptPropertyName` → `GetPropertyIdFromJavaScriptPropertyName`; `cssValueFromText` → `CSSValueFromText`. `CSSStyleSheet::RuleMutationType` and `ContentsClonedForMutation` are `enum class`.

**Bugs fixed (old Krystal files):**
- The old `CSSStyleSheet`, `StyleSheet` and `MediaList` were spec-driven sketches: `Media()` was `krys_not_implemented()`, the constructor ignored its options, `CssRules()` returned an empty list, and nothing connected the sheet to `StyleSheetContents`. `src/CSS/CSSStyleSheet.cpp` still called the parser deleted in d5.
- `src/CSS/CSSRule.cpp` mapped rule types to the wrong CSSOM numbers (no `@charset`, a `Viewport` type that doesn't exist) and used members the header didn't declare.
- Most `CSSOM/*.hpp` were raw WebKit (`String`, `adoptRef`, `SPECIALIZE_TYPE_TRAITS_CSS_RULE`, licence headers) or half-translated, with no `.cpp`; the descriptor classes had no `.cpp`.

**Upstream bugs found:**
- `CSSViewTransitionRule::cssText` closes the `types` descriptor with `}` instead of `; `, giving `@view-transition { types: a b}}`. Fixed (tested).
- `CSSCounterStyleRule::setName` changes the descriptors' name but not the rule's, so `name` and `cssText` keep the old name. Fixed: both are set (tested).
- `CSSCounterStyleRule::setSystem` doesn't do the step its comment quotes ("if the new value would change the algorithm used, do nothing"). Fixed: a value with a different system is ignored (tested).
- `StyleSheetContentsCache::add` calls `addedToMemoryCache` even when the key is already present (the contents aren't stored, so the count is never undone). Krystal only counts a new entry (tested); `replaceSync` never adds an existing key.
- Kept as upstream: `PropertySetCSSStyleProperties::setPropertyInternal` (and the descriptors') calls `willMutate` before the exposure check, so an unexposed property leaves it unpaired (harmless: it only makes the contents mutable); `CSSKeyframeRule::setKeyText` changes the keyframe outside a mutation scope (no copy-on-write); `CSSPageRule::setSelectorText` parses a selector list, not a page selector; `CSSNamespaceRule::reattach` keeps the old rule; removing a shorthand returns an empty string (d2's `RemoveShorthandProperty` FIXME, tested); the `negative` descriptor always writes its suffix (`"-" ""`, d4, tested).

**Differences:**
- Through the seam, no sheet has a style scope, so `DidMutate`, `DidMutateRules`, `SetDisabled` and `WasMutated` never notify anything. Constructed sheets use the document's base URL and parser context (default settings; since e1 of the DOM integration); `CanAccessRules` asks `DocumentCanRequest`, which allows everything.
- Not ported until the DOM integration (FIXMEs): the async `Replace` and `CSSStyleDeclaration::settings()`. Inline styles came with e2, `CreateInline` and `StyleSheetList` with e4, the adopting tree scopes with e5.
- `CSSOMString` can't be null: `Href()` and `Title()` are empty for none; nullable attributes are `Maybe` (`CSSImportRule::LayerName`, `CSSPropertyRule::InitialValue`, `CSSFunctionRule::FunctionParameter::DefaultValue`). The rule model can't tell an empty `supports()` from none, so `@import "a" supports()` serialises without it, and `CSSPropertyRule` omits an empty `syntax`.
- `CSSImportRule::GetStyleSheet()` is always null (imports don't load); whether the imported sheet is origin-clean waits for the loader (FIXME). No `cssText(context)` override (its only job is URL replacement, which `SerialisationContext` doesn't support); the other `CssText(context)` overloads are kept.
- `StyleSheetContentsCache` evicts the first entry of its `std::unordered_map`, not a random one, and hashes the context with `ParserContext::Hash()`.
- `CSSStyleRule::NestedRules()` returns a reference (upstream copies the list on every call).
- Names and wrappers return `CSSOMString` or `CSSOMStringAtom` where upstream returns `String`/`AtomString`; `CSSNamespaceRule::Prefix()` is the null atom for the default namespace.
- `CSSRule`, `CSSStyleSheet` and `MediaList` use `WeakPtr` where upstream uses `SingleThreadWeakPtr`.
- Indices and counts are `uint32`.

**Unbuilt files touched:** none. `CSS/StyleSheetList.hpp` (old sketch) is left as it is for the DOM integration.

Tests: `tests/CSS/{CSSStyleSheet,CSSStyleProperties}.cpp`, `tests/CSS/Media/MediaList.cpp`, `tests/CSS/CSSOM/*.cpp` (one per wrapper), `tests/CSS/Descriptors/PropertySetCSSDescriptors.cpp`, `tests/Style/StyleSheetContentsCache.cpp` (helpers in `include/Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp`): 58 cases / 1294 assertions pass. Full Booey suite: 1732 cases / 680288 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

## Media query evaluation (2026-10-14)

Graph from the seven evaluation entries (`GenericMediaQueryEvaluator.cpp`, `MediaQueryEvaluator.cpp`, `MediaQueryFeatures.cpp`, `ContainerQueryFeatures.cpp`, `MediaQueryMatcher.cpp`, `MediaQueryList.cpp`, `style/ContainerQueryEvaluator.cpp`; headers-only): 145 files (`tools/port/out/evaluation.md`), nearly all already ported. The new dependencies are the frame, page, settings, theme and screen (media features), the render tree and style resolver (container queries), and `EventTarget`/`ActiveDOMObject` (`MediaQueryList`).

**Decisions (2026-10-14):**
- Media only. Container query evaluation (`ContainerQueryEvaluator`, the size schemas and `style()`) waits for style resolution: it needs composed-tree traversal, `containerType()`/`containerNames()`, `RenderBox` content sizes, `Style::Update` and `Style::Builder::resolveCustomPropertyForContainerQueries`. `MediaQueryMatcher`, `MediaQueryList`, `MediaQueryListEvent` and `matchMedia` wait for the DOM integration.
- Media features read a Krystal-side `MediaEnvironment` (`CSS/Queries/MediaEnvironment.hpp`) that the host fills in, instead of the frame, page, settings, theme and screen. It's process-wide (FIXME) and set with `SetMediaEnvironment`; `MediaEnvironmentFor(document)` is null until one is set, which stands for "the document has no view".

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/query/GenericMediaQueryEvaluator.h`, `.cpp` | `have` | `CSS/Queries/GenericMediaQueryEvaluator.hpp`, `src/...` | on | tested. |
| `css/query/MediaQueryEvaluator.h`, `.cpp` | `have` | `CSS/Queries/MediaQueryEvaluator.hpp`, `src/...` | on | tested. |
| `css/query/MediaQueryFeatures.cpp` (evaluation) | `have*` | `src/CSS/Queries/MediaQueryFeatures.cpp` | on | tested. Reads `MediaEnvironment`; see differences. |
| `style/values/primitives/StyleRatio.h` | `have` (no `.cpp`) | `Style/Values/Primitives/Ratio.hpp` | on | tested through `aspect-ratio`. `CSSValueCreation<Ratio>` is deferred with the other `CSSValueCreation`s (FIXME). |
| — | Krystal | `CSS/Queries/MediaEnvironment.hpp`, `src/...` | on | tested. Stands in for `LocalFrame`/`LocalFrameView`/`Page`/`Chrome`/`Settings`/`Theme` and the screen functions. |
| `css/query/ContainerQueryFeatures.cpp` (evaluation), `style/ContainerQueryEvaluator.h`, `.cpp` | `?` | `src/CSS/Queries/ContainerQueryFeatures.cpp` (schemas only); seam `style::ContainerQueryEvaluator` | — | Style resolution. |
| `css/MediaQueryMatcher.h`, `.cpp`, `css/MediaQueryList.*`, `css/MediaQueryListEvent.*` | `have` | `CSS/Queries/MediaQueryMatcher.hpp`, `CSS/Media/MediaQueryList.hpp`, `MediaQueryListEvent.hpp` | on | Done in e7 of the DOM integration. |

**Names:** `evaluate*` → `Evaluate*`; `toEvaluationResult` → `ToEvaluationResult`; `concreteEvaluator()` → `AsConcreteEvaluator()`; the schema base classes (`BooleanSchema`, `IntegerSchema`, `LengthSchema`, `RatioSchema`, `ResolutionSchema`, `IdentifierSchema`) keep their names and live in the `.cpp`. `NumberSchema` isn't ported: only the dropped `-webkit-device-pixel-ratio` used it.

**Bugs fixed (old Krystal headers):** `GenericMediaQueryEvaluator.hpp` included a commented-out `LayoutUnit` header and nothing for `FloatSize`, and had no `.cpp` (the `Evaluate*Feature` functions were never defined); `MediaQueryEvaluator.hpp` had no `.cpp`.

**Seam changes:** `RenderStyle::Zoom`, `UsedZoom`, `DeviceScaleFactor` (1) and `EvaluationTimeZoomEnabled` (off, the non-Cocoa default) are implemented, so lengths resolve against a constructed `RenderStyle`. `RenderViewOf` returns null (was unreachable). Added `RenderElement` (a `CheckedPtr` target for `FeatureEvaluationContext::Renderer`; `RenderBox` derives from it) and `AdjustForAbsoluteZoom(int32, const RenderView &)` (unreachable; only called with a render view).

**Differences:**
- `MediaEnvironment` holds the effective values. Upstream's forced accessibility settings (`forcedColorsAreInvertedAccessibilityValue`, `forcedPrefersContrastAccessibilityValue`, ...) and the theme's `userPrefers*` collapse into one field each; the host applies its own overrides.
- Site-specific quirks are dropped (`shouldSupportHoverMediaQueries`, `shouldPreventOrientationMediaQueryFromEvaluatingToLandscape`, `shouldHideCoarsePointerCharacteristics`).
- Upstream's `hover`/`pointer` features match `none` when there's no page; the environment defaults to a mouse (hover, fine pointer).
- `color-gamut`, `color`, `dynamic-range` and `monochrome` read the screen fields; upstream asks the main frame's view. `monochrome` doesn't need a local main frame.
- The evaluator checks `MediaEnvironmentFor(document)` where upstream checks `document->view()`.
- Font-relative units in media queries (`em`, `ex`, `ch`, ...) resolve against the seam's initial font since e7 of the DOM integration (no font data; css-values-4 fallbacks). Upstream resolves them against the root element's user-agent style, which `MediaQueryMatcher` builds.
- Dropped with their features in d3: the `-webkit-animation`, `-webkit-device-pixel-ratio`, `-webkit-transform-2d`/`3d`, `-webkit-transition`, `-webkit-video-playable-inline` and `prefers-dark-interface` schemas.

Tests: `tests/CSS/Queries/MediaQueryEvaluator.cpp` (media types, static results, dynamic dependencies, three-valued logic and every media feature against changed environments): 11 cases / 221 assertions pass. Full Booey suite: 1743 cases / 680509 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

## DOM integration (2026-10-14)

Connects the CSS port to the DOM (`DOM/`, `HTML/`), which is written from the WHATWG specs rather than ported. Upstream's integration points: `TreeScope` (owns the `Style::Scope` and adopted sheets), `Style::Scope` (sheet candidates, pending sheets, active sheets), `InlineStyleSheetOwner` (`<style>`), `StyledElement`/`ElementData` (the `style` attribute, presentational hints), `SelectorQuery`/`SelectorChecker` (`querySelector`, `matches`, `closest`), `StyleSheetList`, `CSSStyleSheetObservableArray` and `MediaQueryMatcher`.

**Layers:**
- **e0, DOM prerequisites:** `DocumentOrShadowRoot`, attribute change steps as a virtual, the parser's begin/finish parsing children hooks, `Document` quirks mode queries, a minimal event loop. Done.
- **e1, Document URL and parser context:** `Document` URL and base URL (fallback/frozen base URL, `<base>`), `CompleteURL`, a cached `CssParserContext()` that respects quirks mode, the `Document` constructors of `ParserContext`, `SelectorParserContext` and `MediaQueryParserContext`; the `Document*` URL and parser context seam functions go. Done.
- **e2, inline styles:** `StyledElement` between `Element` and `HTMLElement`/`SVGElement`/`MathMLElement`, the inline style, `InlineCSSStyleProperties`, `StyleAttributeMutationScope`, `Parser::ParseInlineStyleDeclaration`, lazy `style` attribute synchronisation. Presentational hints wait for the cascade. Done.
- **e3, `style::Scope` bookkeeping:** owned by `DocumentOrShadowRoot`; candidate nodes, pending sheets, preferred/alternate sets, `CollectActiveStyleSheets`, `UpdateActiveStyleSheets` without the render-tree gate, resolver update or invalidation. Updates are scheduled on the event loop. Done.
- **e4, sheet owners:** `InlineStyleSheetOwner`, `CSSStyleSheet::CreateInline`, the owner-node half of `StyleSheetContents`, `HTMLStyleElement`, `StyleSheetList` (`document.styleSheets`, `shadowRoot.styleSheets`). Done.
- **e5, `adoptedStyleSheets`:** on `DocumentOrShadowRoot`; `CSSStyleSheetObservableArray` becomes a plain C++ list API with the same checks; adopting tree scopes in `CSSStyleSheet`. Done.
- **e6, selector matching:** `SelectorChecker` (`QueryingRules` mode, no JIT, `SelectorFilter` or relations), then `SelectorQuery`/`SelectorQueryCache`. Done (one layer).
- **e7, `matchMedia`:** `MediaQueryMatcher`, `MediaQueryList`, `MediaQueryListEvent`, on `Document` (there's no window). Done.
- **Deferred:** `<link>` loading, `xml-stylesheet`, `@import`, render blocking (resource loader); `SVGStyleElement`; presentational hints and `<body>` colours (cascade); the `*ChangeInvalidation` classes; `getComputedStyle`.

**Decisions (2026-10-14):**
- No `TreeScope`. Its state lives on `DocumentOrShadowRoot` (the spec mixin, now a base of `Document` and `ShadowRoot`), and the tree scope of a node is found by walking to its root (`TreeQueries::Root`) instead of upstream's cached `Node::treeScope()`.
- **FIXME (performance):** cache each node's root (`DocumentOrShadowRoot`) like upstream's `treeScope()`, kept up to date by the insert, remove and adopt steps, and change `TreeQueries::Root`, `ShadowIncludingRoot` and the other root walks to use it. It's an easy win; the walks are O(depth) on every call.
- `StyledElement` keeps upstream's name. It's how upstream represents the `ElementCSSInlineStyle` mixin, which only `HTMLElement`, `SVGElement` and `MathMLElement` include.
- `Document` gets a real URL and base URL in e1.
- A minimal event loop (`html::EventLoop`) replaces upstream's timers; the DOM needs it anyway.

### Layer e0: DOM prerequisites (2026-10-14)

| Change | Krystal | Notes |
|---|---|---|
| `DocumentOrShadowRoot` | `DOM/DocumentOrShadowRoot.hpp`, `src/...` | Base of `Document` and `ShadowRoot`; holds the custom element registry and `ActiveElement` (moved out of the copied `#pragma region`s). `ShadowRoot` now has `ActiveElement` too (always null, as `Document`'s). |
| Attribute change steps | `Element::OnAttributeChanged` (virtual) | `ExtensibilityHooks::ElementAttributeChanged` forwards to it. The `class` and `slot` steps moved into `Element`, the slot `name` steps into `HTMLSlotElement`. Upstream's `Element::attributeChanged`. |
| Parsing children | `Element::BeginParsingChildren`, `FinishParsingChildren` (virtual), `IsFinishedParsingChildren`; `NodeFlags::IsParsingChildren` | `HTMLElementStack` begins on `Push`/`InsertBelow` and finishes on `Pop`/`Remove`, as upstream's `HTMLElementStack`. Elements not created by the parser are always finished. |
| Quirks mode | `Document::InQuirksMode`, `InLimitedQuirksMode`, `InNoQuirksMode` | |
| Event loop | `HTML/EventLoop.hpp`, `HTML/Enums/TaskSource.hpp`, `src/HTML/EventLoop.cpp` | One task queue shared by every task source, a microtask queue, `PerformMicrotaskCheckpoint`, `RunNextTask`, `RunUntilIdle`. No rendering step, task documents or scripting. `EventLoop::Current()` is process-wide (FIXME). Nothing queues on it yet. |

**Bugs fixed (DOM):**
- `ElementAlgorithms::ChangeAttribute` bound the old value as `DOMString &&` to the moved-from member, so the change steps and mutation records saw the new value as the old one.
- `HandleAttributeChanges` took `DOMStringView`s, so a missing value (attribute added or removed) looked like an empty one; it also passed the attribute's value as the new value on removal, so removing `class` left `classList` populated. Values are now `Maybe<DOMStringView>`.
- `MutationObserverAlgorithms::QueueMutationRecord`: the namespace and `attributeFilter` checks were inverted (no attribute record was ever queued), `attributeOldValue` was read from `attributes`, every record got the old value instead of the observer's mapped one, `target` was moved into the first record, and an observer registered on several inclusive ancestors got one record each. Rewritten to the spec steps.
- `NodeFlags` started at bit 1, so `IsHTMLSlotElement` (`1 << 10`) didn't fit the 10-bit field. Renumbered from bit 0.
- `HTMLLinkElement::RelList()` reflected the attribute `rellist` instead of `rel`.
- The slot `name` steps set the `name` attribute again (`slotElement.Name(...)`), queueing an extra mutation record. The slot's name is read from the attribute, so they no longer set anything.

**Open (DOM):** only `classList` follows its attribute; the other `DOMTokenList`s (`relList`, `sandbox`, `sizes`, `blocking`, `htmlFor`) are parsed once when created and miss later attribute changes (they should use `OnAttributeChanged`). The mutation observer microtask (`QueueMutationObserverMicrotask`, `NotifyMutationObservers`) can now be built on `EventLoop`.

Tests: `tests/DOM/Element.cpp` (attribute change steps, `classList`, attribute mutation records, slot type check), `tests/HTML/Parser/HTMLElementStack.cpp` (parsing children, quirks mode), `tests/HTML/HTMLLinkElement.cpp`, `tests/HTML/EventLoop.cpp`. Full Booey suite: 1752 cases / 680575 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e1: Document URL and parser context (2026-10-14)

| Change | Krystal | Notes |
|---|---|---|
| Document URL | `Document::GetURL`, `SetURL`; `URL()`, `DocumentURI()` | `about:blank` until the host sets one (`SetURL`, upstream's `setURL`; an empty URL becomes `about:blank`). Cloning a document copies it. `GetURL` because the IDL `URL()` returns the serialisation. |
| Base URL | `Document::FallbackBaseURL`, `BaseURL`, `ProcessBaseElement`, `UpdateBaseURL`; `Node::BaseURI` | As upstream: the document caches the frozen base URL of the first `<base href>` in the document tree, recomputed by `ProcessBaseElement` when a `base` element is inserted, removed or moved or its `href` changes (`HTMLBaseElement` hooks). The frozen base URL follows the spec: `href` parsed against the fallback base URL, and the fallback base URL instead if that fails or the scheme is `data` or `javascript`. |
| URL parsing | `Document::CompleteURL(url)`, `CompleteURL(url, baseURL)` | UTF-8 only, so no encoding. |
| Parser context | `Document::CssParserContext`, `InvalidateCachedCSSParserContext` | Cached; invalidated when the base URL or the mode changes (`Document::SetQuirksMode`, which the parser and cloning now use instead of writing `_quirksMode`). |
| `css::ParserContext(const Document &)`, `ParserContext(const Document &, sheetBaseURL, charset)` | `CSS/Parser/ParserContext.hpp`, `src/...` | Upstream's `CSSParserContext(const Document &[, ...])`: base URL, mode from quirks mode, `IsHTMLDocument`, `HasDocumentSecurityOrigin` (through the seam's `DocumentCanRequest`). Settings are the defaults (FIXME: `Settings`). |
| `SelectorParserContext(const Document &)`, `MediaQueryParserContext(const Document &)` | `CSS/Selectors/...`, `CSS/Queries/...` | As upstream; the selector context's settings are the defaults (FIXME). |
| Seam | `DocumentBaseURL`, `DocumentCompleteURL`, `DocumentCssParserContext` removed | `CSSStyleSheet::Create(Document &, Init)` uses `Document::BaseURL`/`CompleteURL` and `ParserContext(document, baseURL, {})`, as upstream. `DocumentCanRequest` and `DocumentKeyframesRuleDidChange` stay (no security origin or animations). |

**Differences:** no browsing contexts, so the fallback base URL is always the document's URL (no iframe `srcdoc` documents or about base URL; SPEC-VIOLATION in `FallbackBaseURL`), and every `<base>` is allowed (no CSP). Upstream keeps an invalid base element URL as an invalid base URL; Krystal follows the spec and falls back. Upstream's `m_baseURLOverride` and `documentURI` setter are not ported.

**Bugs fixed (DOM):** cloning a document dereferenced its custom element registry without a null check, so cloning any document without one crashed.

**Open (DOM):** `Document::Head()` returns null for a parsed document that has a `head` (`Body()` works; the bug is in `DOMTreeAccessors::GetHeadElement`). `HTMLBaseElement.href` and every URL-reflecting attribute still return `NotSupportedError`: `dom::USVString` is an empty placeholder, so reflection can't produce strings (the `href` getter's steps are written as a comment).

Tests: `tests/DOM/Document.cpp` (URL, base URL from `<base>` elements, the parser context and the `Document` constructors), `tests/CSS/CSSStyleSheet.cpp` (a constructed sheet's relative base URL). Full Booey suite: 1755 cases / 680625 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e2: inline styles (2026-10-14)

Graph from `dom/StyledElement.cpp` and `css/StyleAttributeMutationScope.cpp` (headers-only): 124 files (`tools/port/out/inline-style.md`), all ported or boundaries already handled; the new upstream pieces are `StyledElement`, `ElementData`'s inline style and dirty flag, `InlineCSSStyleProperties`, `StyleAttributeMutationScope` and `CSSParser::parseInlineStyleDeclaration`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `dom/StyledElement.h`, `.cpp` | `port` | `DOM/StyledElement.hpp`, `src/...` | on (DOM) | tested. Base of `HTMLElement`, `SVGElement` and `MathMLElement`. The inline style is a member (upstream keeps it in `ElementData`); the dirty flag is `NodeFlags::StyleAttributeIsDirty`. `cssomStyle()` is `Style()` (the IDL `style`). |
| `dom/ElementData.h` (`m_inlineStyle`, `styleAttributeIsDirty`) | `have*` | `StyledElement::_inlineStyle`, `NodeFlags::StyleAttributeIsDirty` | on | No shareable element data. |
| `dom/Element.cpp` (`synchronizeAllAttributes`, `setSynchronizedLazyAttribute`) | `have*` | `Element::SynchronizeAllAttributes`, `SetSynchronizedLazyAttribute` | on | tested. See differences. |
| `css/CSSStyleProperties.h`, `.cpp` (`InlineCSSStyleProperties`, the mutation scopes) | `port` | `CSS/CSSStyleProperties.hpp`, `src/...` | on | tested. |
| `css/MutableStyleProperties.cpp` (`ensureInlineCSSStyleProperties`) | `port` | `CSS/Properties/MutableStyleProperties.hpp`, `src/...` | on | tested. |
| `css/StyleAttributeMutationScope.h`, `.cpp` | `port` | `CSS/StyleAttributeMutationScope.hpp`, `src/...` | on | tested. |
| `css/parser/CSSParser.cpp` (`parseInlineStyleDeclaration`) | `port` | `Parser::ParseInlineStyleDeclaration` | on | tested. |
| `css/parser/CSSParserMode.h` (`strictToCSSParserMode`) | `port` | `StrictToParserMode` in `CSS/Parser/ParserMode.hpp` | on | |
| `mathml/MathMLElement.h` | `have*` | `MathML/MathMLElement.hpp`, `src/MathML/...`, `MathMLElementFactory` | on (MathML) | Written from MathML Core: every MathML-namespace element is a `MathMLElement` (was a plain `Element`). No MathML-specific behaviour. |
| `css/typedom/InlineStylePropertyMap.*` (`ensureAttributeStyleMap`) | `?` | — | — | Typed OM. |

**Lazy synchronisation.** As upstream, a CSSOM change only marks the `style` attribute dirty; the serialised inline style is written back (without change steps or a mutation record) when the attribute list is next read. Upstream reads attributes through a few `ElementData` accessors; Krystal's `Element::_attributes` is read directly in many places, so `SynchronizeAllAttributes()` is called by every reader: `Attr::Value()`, `HasAttributes`, `GetAttributeNames`, `SetAttribute`, `ToggleAttribute`, `HasAttribute(NS)`, `ElementAlgorithms::GetAttributeByName`/`ByNamespace`/`ChangeAttribute`/`RemoveAttribute`/`ReplaceAttribute`, `NamedNodeMap`, cloning and `Equals`. The xmlns lookups (`LocateNamespace`, `LocateNamespacePrefix`), adoption and `compareDocumentPosition` don't need it. New code that reads `_attributes` must call it.

**Added (upstream API):** `StyledElement::DirtyStyleAttribute`, `InvalidateStyleAttribute`, `InlineStyle`, the `SetInlineStyleProperty`/`SetInlineStyleCustomProperty`/`RemoveInlineStyle*`/`RemoveAllInlineStyleProperties` family; `MutableStyleProperties::EnsureInlineCSSStyleProperties`; `Node::IsStyledElement`; `MutationObserverAlgorithms::IsAttributeOldValueRequested` (upstream's `MutationObserverInterestGroup::isOldValueRequested`, so the scope only serialises the old value when an observer wants it).

**Bugs fixed (DOM):** cloning an element appended the original's `Attr` nodes to the clone instead of the copies (`NodeAlgorithms` clone), moving them out from under the original. `PropertySetCSSStyleProperties` was missing upstream's mutation scopes (no records for CSSOM changes).

**Differences:**
- No style resolution or rule sets yet: `StyleAttributeChanged`, `DirtyStyleAttribute` and `InvalidateStyleAttribute` don't invalidate style and never synchronise eagerly for `[style]` selectors (FIXMEs).
- SPEC-VIOLATION(CSP): the attribute is parsed without the Content Security Policy check (there is no CSP).
- No shareable element data, so upstream's "skip reparsing shared inline style" path doesn't exist; an attribute without a CSSOM wrapper is always parsed into a new immutable declaration.
- Dropped: `usesStyleBasedEditability` (`Document::setHasElementUsingStyleBasedEditability`; there's no editing), `addSubresourceAttributeURLs`/`replaceURLsInAttributeValue` (web archives), the inspector hooks, `setDidMutateSubtreeAfterSetInnerHTMLOnAncestors`, SVG `invalidateInstances`, the presentational hint half (comes with the cascade) and `ensureAttributeStyleMap` (Typed OM).
- `StyleAttributeMutationScope`'s custom element half (old value for `attributeChangedCallback`) is a TODO, like the rest of the custom element reactions. `EnqueueMutationRecord` on a nested scope marks the outermost one (upstream marks the nested scope, whose flag is never read).
- `InlineCSSStyleProperties::ParentElement()` returns `dom::Element *` (upstream: `StyledElement *`), to keep `StyledElement.hpp` out of the CSSOM header.
- As upstream, an SVG or MathML element's inline style is parsed in quirks mode (strict only for HTML elements in no-quirks documents).

Tests: `tests/DOM/StyledElement.cpp` (element types, the `style` attribute, CSSOM write-back through every reader, mutation records, the setters, parser modes, cloning): 7 cases / 130 assertions pass. Full Booey suite: 1762 cases / 680755 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e3: style scopes (2026-10-14)

Graph from `style/StyleScope.cpp` (headers-only): 131 files (`tools/port/out/style-scope.md`); the new pieces are `Style::Scope` itself, `Document::inDocumentShadowRoots` and the `sheet()` of the sheet owners. The resolver, invalidation, media query re-evaluation, layout dependencies and anchor positioning parts wait for style resolution.

**Decisions (2026-10-14):** fix the three shadow-tree/ref-counting DOM bugs below in this layer; give `<style>` and `<link>` LinkStyle's `Sheet()` now (null until e4 or the resource loader) so `CollectActiveStyleSheets` is complete. Until something creates those sheets, collection, tree order, filtering and style sheet sets can't be tested; they're tested with e4 (FIXME in `tests/Style/Scope.cpp`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleScope.h`, `.cpp` | `port` | `Style/Scope.hpp`, `src/...` | on | tested. Bookkeeping half; see differences. Replaces the seam's `style::Scope`. |
| `dom/Document.cpp` (`styleScope`, `inDocumentShadowRoots`, `didInsert`/`didRemoveInDocumentShadowRoot`) | `have*` | `DocumentOrShadowRoot::StyleScope`, `Document::InDocumentShadowRoots`, `DidInsertInDocumentShadowRoot`, `DidRemoveInDocumentShadowRoot` | on | tested. |
| `dom/ShadowRoot.cpp` (`styleScope`, `insertedIntoAncestor`, `removedFromAncestor`, `moveShadowRootToNewDocument`) | `have*` | `ShadowRoot::OnInsert`, `OnRemove`, `OnAdopt` | on | tested. Adoption gives the shadow root a new scope, as upstream. |
| `html/HTMLStyleElement.h`, `HTMLLinkElement.h` (`sheet()`) | `have*` | `Sheet()` | on | LinkStyle's `sheet`. |

**Names:** upstream's `Style::Scope::forNode` returns a reference, as does Krystal's `ForNode` (the seam's returned a pointer). `setLastSuccessfulPositionOptionIndexMap({ })` stays collapsed as `ClearLastSuccessfulPositionOptionIndexMap` (a no-op until anchor positioning, FIXME). `ResolverIfExists()` is always null (FIXME).

**Bugs fixed (DOM):**
- A shadow root never became connected: the insertion steps skipped any node without a parent, and attaching a shadow root to a connected host didn't connect it, so nothing in a shadow tree was ever connected. Shadow roots now follow their host (on insertion and in `AttachShadow`).
- Removing a host released its shadow root's reference once too often (the removal steps released every removed node without a parent, but shadow roots were never retained on insertion), leaving the host's `_shadowRoot` dangling. Shadow roots hold no tree reference now.
- Every insertion retained every shadow-including inclusive descendant, but removal only released the removed node, so each insert/remove cycle leaked a reference per descendant. Only the inserted node gains a tree reference now, and only the removed node loses it.
- Removal cleared `IsInShadowTree` on every removed node, including shadow roots and the nodes in their trees. It's now cleared only for nodes whose root is no longer a shadow root.
- Moving a host (`moveBefore`) tripped `NodeMoved`'s assertion on its shadow root, which has no parent.

**Differences:**
- No render tree or style resolution: `UpdateActiveStyleSheets` always runs (no living-render-tree gate, no deferral during style recalc) and never analyses the change, updates a resolver or invalidates style (FIXMEs). `ScheduleUpdate` doesn't invalidate `:host`/`::slotted` or clear the resolver. `usesStyleBasedEditability`, `usesHasPseudoClass`, the match result cache, the custom property and counter style registries, shared shadow tree resolvers, media query re-evaluation, viewport changes, layout dependencies and anchor positioning are not ported.
- Updates are a task on `html::EventLoop` (DOM manipulation task source) instead of a zero-delay timer. A queued task isn't cancelled when the update is flushed early; it finds nothing to do.
- No extension style sheets or `authorAndUserStylesEnabled` setting; no user agent shadow roots; no inspector.
- `CollectActiveStyleSheets`: xml-stylesheet processing instructions and `SVGStyleElement` are skipped (FIXMEs; no sheets yet). Adopted sheets came with e5. A `<link>`'s "enabled via script" state and its loading branch come with the resource loader. As upstream, `rel` is checked for the substring `alternate`, case-sensitively.
- Pending processing-instruction sheets aren't tracked, and removing the last pending sheet doesn't notify the document (`didRemoveAllPendingStylesheet`, FIXME: scripts and rendering don't wait for sheets).
- Candidates, pending sheets and in-document shadow roots are `List<WeakPtr<...>>` (upstream: weak hash sets); `ActiveStyleSheetsContains` is a linear search (upstream caches a hash set).
- `Document::Body()` only returns `<body>`, so a `<frameset>` doesn't count as the body when ordering candidates.

**Open (DOM):** a container node doesn't release its children when it's destroyed, and every node keeps its document alive (`_nodeDocument` is a `RefPtr`), so a document with children is never freed. In fact no document is freed: `Document` is a `Node` whose `_nodeDocument` is itself (noted in e5). Upstream breaks the cycle with the document's referencing-node count and `removeDetachedChildren`.

Tests: `tests/Style/Scope.cpp` (shadow tree connection and references, scope lookup and adoption, update scheduling and flushing, shadow scopes, links without sheets, pending sheets): 4 cases / 67 assertions pass; `tests/CSS/CSSStyleSheet.cpp` now expects an owner node's sheet to have its document's scope. Full Booey suite: 1766 cases / 680822 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e4: sheet owners (2026-10-14)

Graph from `dom/InlineStyleSheetOwner.cpp`, `html/HTMLStyleElement.cpp` and `css/StyleSheetList.cpp` (headers-only): 56 files (`tools/port/out/sheet-owners.md`), all ported or handled; the new pieces are the three entries, `CSSStyleSheet::createInline`, `StyleSheetContents`' owner-node half and the owner callbacks on `Node`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `dom/InlineStyleSheetOwner.h`, `.cpp` | `port` | `DOM/InlineStyleSheetOwner.hpp`, `src/...` | on (DOM) | tested. |
| `html/HTMLStyleElement.h`, `.cpp` | `have*` | `HTML/HTMLStyleElement.hpp`, `src/...` | on (HTML) | tested. Written from the HTML spec; the sheet half follows upstream. `disabled` was `krys_not_implemented()`. |
| `css/StyleSheetList.h`, `.cpp` | `port` | `CSS/StyleSheetList.hpp`, `src/...` | on | tested. Replaces the old sketch. |
| `css/CSSStyleSheet.cpp` (`createInline`) | `port` | `CSSStyleSheet::CreateInline` | on | tested. |
| `css/StyleSheetContents.cpp` (`singleOwnerNode`, `singleOwnerDocument`, `startLoadingDynamicSheet`, `checkLoaded`) | `port` | `StyleSheetContents` | on | tested through `<style>`. |
| `dom/Node.h` (`sheetLoaded`, `notifyLoadedSheetAndAllCriticalSubresources`, `startLoadingDynamicSheet`) | `port` | `Node::SheetLoaded`, `NotifyLoadedSheetAndAllCriticalSubresources`, `StartLoadingDynamicSheet` | on | |
| `dom/Document.cpp`, `ShadowRoot.cpp` (`styleSheets`) | `port` | `Document::StyleSheets`, `ShadowRoot::StyleSheets` | on | tested. Detached when the document or shadow root is destroyed. |

**Parser change:** `HTMLTreeBuilder::InsertForeignElement` begins parsing an element's children before inserting it (upstream attaches parser-created elements after pushing them), so a parsed `<style>` is a candidate from insertion but only creates its sheet once its text is complete. `Element`'s `friend class HTMLTreeBuilder` named a nonexistent `dom::HTMLTreeBuilder`; it's `html::HTMLTreeBuilder` now.

**Bugs fixed:** Krystal.Core `StringAtom::View()` dereferenced the null atom's storage, crashing on any null atom (a parsed `<style>` without `type` or `media` crashed the parser tests); it now reads as empty, as WTF's null `AtomString` does (Core test). The local workarounds are removed: `GenericMediaQueryParser`'s `IsNullOrEmpty` (now `View().empty()`, upstream's `isEmpty()`) and `QualifiedNameCache`'s null-namespace guard. `TreeSerialisation`'s `IsNull` stays: it's upstream's `isNull()`.

**Differences:**
- No `createdByParser` flag: the owner reads `Element::IsFinishedParsingChildren()` (set for parser-created elements from creation to the pop off the stack).
- SPEC-VIOLATION(CSP): inline sheets are created without the Content Security Policy check. No text positions (only CSP and the inspector use them), so `CreateInline` takes none and `isInline()`/`startPosition()` aren't ported. No user agent shadow trees, so no blank-URL/UA-sheet parser context for them.
- `load`/`error` are fired by a task on the networking task source (upstream: an `EventSender` timer, cancelled when the element is destroyed; the task keeps the element alive until it runs).
- Not ported: `addSubresourceAttributeURLs` (web archives), `dispatchPendingLoadEvents` (pages). `blocking` still doesn't follow its attribute (open DOM item).
- `StyleSheetList`'s named getter (non-standard) is dropped; `Item` returns `StyleSheet *` as upstream (FIXME upstream: should be `CSSStyleSheet`).
- `<link>` has `Sheet()` but nothing sets it until the resource loader; `SVGStyleElement` doesn't own a sheet yet.

Tests: `tests/HTML/HTMLStyleElement.cpp` (parsed sheets, tree order, text changes, removal, `type`, `media`, `title`, `disabled`, shadow roots, shared contents, `load`): 1 case / 60 assertions; `tests/Style/Scope.cpp` gains the collection tests deferred from e3 (empty and disabled sheets, style sheet sets): 5 cases / 81 assertions; Core `StringAtom` test. Full Booey suite: 1768 cases / 680896 assertions; all pass except 7 expected failures. Krystal.Core suite: 554 cases pass. Whole solution builds with no warnings.

### Layer e5: `adoptedStyleSheets` (2026-10-14)

Graph from `css/CSSStyleSheetObservableArray.cpp` (headers-only): 15 files (`tools/port/out/adopted-sheets.md`), all already ported; the boundaries are the bindings (`JSObservableArray`, `JSDOMConvert`) and the DOM (`TreeScope`, `Document`, `ShadowRoot`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/CSSStyleSheetObservableArray.h`, `.cpp` | `port` | `CSS/CSSStyleSheetObservableArray.hpp`, `src/...` | on | tested. A C++ list API instead of a `JSC::ObservableArray`; see differences. |
| `dom/TreeScope.h`, `.cpp` (`adoptedStyleSheets`, `setAdoptedStyleSheets`, `ensureAdoptedStyleSheets`) | `have*` | `DocumentOrShadowRoot::AdoptedStyleSheets`, `SetAdoptedStyleSheets`, `GetAdoptedStyleSheets` | on | tested. |
| `css/CSSStyleSheet.h`, `.cpp` (`addAdoptingTreeScope`, `removeAdoptingTreeScope`, `adoptingTreeScopes`, `forEachStyleScope`, `isDetached`) | `port` | `CSSStyleSheet` | on | tested. |
| `style/StyleScope.cpp` (`collectActiveStyleSheets`'s adopted half, `treeScope`) | `port` | `style::Scope` | on | tested. |
| `dom/ShadowRoot.cpp` (`insertedIntoAncestor`, `moveShadowRootToNewDocument`) | `have*` | `ShadowRoot::OnInsert`, `OnAdopt` | on | tested. |

**Names:** the IDL getter `adoptedStyleSheets` is `AdoptedStyleSheets()` (creates and returns the array, upstream's `ensureAdoptedStyleSheets`/`adoptedStyleSheetWrapper`); upstream's span accessor `adoptedStyleSheets()` is `GetAdoptedStyleSheets()` (doesn't create the array). The array keeps upstream's method names: `SetSheets`, `Sheets`, `SetValueAt`, `ValueAt`, `RemoveLast`, `ShrinkTo`, `Length`. `DocumentOrShadowRoot` now takes its root node (upstream's `TreeScope::m_rootNode`), which the array needs.

**Upstream bugs found:**
- `setValueAt` overwrites a sheet without `willRemoveSheet`, and `shrinkTo` (setting `length`) drops sheets without it, so a sheet no longer in `adoptedStyleSheets` keeps the tree scope in its adopting set: it isn't detached (`replaceSync` skips the contents cache) and its mutations keep updating the scope. Krystal releases the replaced sheet and each truncated one (tested).
- A sheet can be adopted twice (`[sheet, sheet]`), but the adopting set holds the tree scope once, so removing either copy releases it. Krystal only releases a tree scope when no copy of the sheet remains (tested).

**Differences:**
- No bindings: the array's methods take sheets directly (no conversion), `SetValueAt` asserts `index <= Length()` (the bindings return `false` past the end) and `ValueAt` returns null instead of `undefined`. The array is owned by its document or shadow root (upstream: ref-counted, shared with the JS wrapper).
- A shadow root's adopted sheets are always cleared when it moves to another document: there are no template contents owner documents (upstream keeps them when moving between a document and its template document).
- `ShadowRoot::OnInsert` reacts to adopted sheets without upstream's `document().frame()` check (no frames; the scope always updates, as in e3).
- Adopting tree scopes are a `List<WeakPtr<ContainerNode>>` (upstream: a weak hash set); entries freed with their tree scope are ignored and pruned on the next add.

Tests: `tests/CSS/CSSStyleSheetObservableArray.cpp` (the constructed-sheet checks, set/remove/shrink, duplicates, several tree scopes, activation order, `styleSheets`, mutations, disabled and titled sheets, shadow roots, adoption into another document, a freed tree scope): 2 cases / 107 assertions pass. A freed document can't be tested: no document is freed (FIXME in the test). Full Booey suite: 1770 cases / 681003 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e6: selector matching (2026-10-14)

Graph from `css/SelectorChecker.cpp` (headers-only): 59 files (`tools/port/out/selector-checker.md`), and from `dom/SelectorQuery.cpp` (`tools/port/out/selector-query.md`). Apart from the two entries, everything to port was already ported or belongs to style resolution (`StyleRelations`, `SelectorMatchingState`, `HasSelectorFilter`, `SelectorFilter`, `StyleScrollbarState`, `ContainerQueryEvaluator`); the rest of the reach is element state (`SelectorCheckerTestFunctions.h`: forms, focus, fullscreen, media, popovers, view transitions).

**Decisions (2026-10-14):** one layer (checker and queries together). State pseudo-classes are implemented where the state is already in the DOM or follows from attributes by the HTML spec; the others match nothing, with FIXMEs.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/SelectorChecker.h`, `.cpp` | `port` | `CSS/Selectors/SelectorChecker.hpp`, `src/...` | on | tested. No JIT, relations or `:has()` cache; see differences. |
| `css/SelectorCheckerTestFunctions.h` | `have*` | `CSS/Selectors/SelectorCheckerTestFunctions.hpp`, `src/...` | on | tested. Written from the HTML spec where upstream asks element state Krystal doesn't have; see differences. |
| `dom/SelectorQuery.h`, `.cpp` | `port` | `DOM/SelectorQuery.hpp`, `src/...` | on (DOM) | tested. No ID fast paths; see differences. |
| `style/RuleFeature.h`, `.cpp` (`MatchElement`, `isHasPseudoClassMatchElement`, `computeHasPseudoClassMatchElement`) | `port` (partial) | `Style/RuleFeature.hpp`, `src/...` | on | tested through `:has()`. The rest of the file comes with the rule set. |
| `style/StyleScope.cpp` (`hostForScopeOrdinal`, `assignedSlotForScopeOrdinal`) | `port` | `style::HostForScopeOrdinal`, `AssignedSlotForScopeOrdinal` (`Style/Scope.hpp`) | on | Only reached with non-element scope ordinals (style resolution). |
| `html/HTMLDocument.cpp` (`isCaseSensitiveAttribute`) | `port` | `dom::HTMLDocument::IsCaseSensitiveAttribute` (`src/DOM/HTMLDocument.cpp`, new) | on (DOM) | tested. |
| `dom/Document.cpp` (`selectorQueryForString`), `ContainerNode.cpp` (`querySelector`, `querySelectorAll`), `Element.cpp` (`matches`, `closest`) | `port` | `Document::SelectorQueryForString`, `ParentNode::QuerySelector`/`QuerySelectorAll`, `Element::Matches`/`Closest`/`WebkitMatchesSelector` | on | tested. Were `krys_not_implemented()`. |
| `style/StyleRelations.h`, `SelectorMatchingState.h`, `HasSelectorFilter.*`, `css/SelectorFilter.*`, `style/StyleScrollbarState.h` | `have` | | | Done in f0 of style resolution (`StyleScrollbarState` is `wont`). |
| `cssjit/*` | `wont` | | | No selector JIT. |

**Names:** `SelectorChecker::Match` (the enum) is `MatchOutcome`, since it would clash with `Match()`; `MatchResult::match`/`matchType` are `Outcome`/`Type`. `LocalContext::selector`/`element`/`visitedMatchType` are `CurrentSelector`/`Element`/`VisitedType`. The checking context keeps the requested pseudo-element as a `Maybe` (upstream splits it into fields for the JIT). New DOM accessors for upstream's: `Element::TagQualifiedName` (`tagQName`), `AttributeList` (`attributes()`, synchronised), `HasClassName`, `IdForStyleResolution`, `GetCustomElementState`; `Attr::GetQualifiedName`; `TreeQueries::ContainingShadowRoot`/`ShadowHost` (`Node::containingShadowRoot`/`shadowHost`). `Element::isLink()` (a node flag upstream) is the free function `css::IsLink`.

**Bugs fixed (DOM):**
- `NameValidation::IsValidCustomElementName` rejected names *with* a hyphen instead of names without one, so `a`, `div` and `span` were valid custom element names (created in the `undefined` state) and `x-widget` wasn't. Fixed (tested).
- Elements started in the `undefined` custom element state; "create an element" leaves every non-custom element `uncustomized`. The default is now `Uncustomized` (`:defined` is tested).
- `QualifiedName` kept an empty namespace as the empty atom, so `|p` (no namespace, which the selector parser resolves to the empty atom) never matched an element in no namespace. As upstream, the qualified name cache turns an empty namespace into null (tested).
- `querySelector()`, `querySelectorAll()`, `matches()` and `closest()` were `krys_not_implemented()`. `Matches`, `Closest` and `WebkitMatchesSelector` now return `ExceptionOr` (they throw a `SyntaxError`, per spec).

**Upstream bugs found:**
- The single type selector fast path never matches `ns|*` or `|*` (a namespace with a universal local name): `localNameMatches` compares the element's local name with `*`. Krystal skips the local name check for `*` (tested with `|*`).

**Differences:**
- No selector JIT. Style relations and the `:has()` match cache and filters came with f0 of style resolution (until then the `addStyleRelation` calls were dropped and `:has()` always traversed). No child index cache, so `:nth-child` counts siblings.
- Not in the standard-only enums (d1a), so not handled: the scrollbar pseudo-classes and pseudo-elements, `-webkit-any`, `-webkit-drag`, the internal pseudo-classes, `-apple-has-attachment`, `:window-inactive`, the strong-password autofill pseudo-classes and the legacy user agent part alias.
- State pseudo-classes implemented from the HTML spec: `:link`/`:any-link` (`a`/`area` with `href`, SVG `a` with `href` or `xlink:href`), `:checked` (checkbox/radio `Checked()`, option `Selected()`), `:default` (checked checkboxes/radios, selected options, the default button of the nearest ancestor form), `:disabled`/`:enabled` (actually disabled, with fieldsets and their first legend, options in disabled optgroups), `:required`/`:optional`, `:read-write`/`:read-only` (text-like inputs, textareas, `contenteditable`), `:placeholder-shown`, `:indeterminate` (checkboxes, progress without a value), `:defined`, `:lang()` (upstream's matching; `xml:lang`/`lang` across shadow boundaries), `:dir()`. `:visited` never matches (no history).
- Match nothing, with FIXMEs: `:hover`, `:active`, `:focus`, `:focus-visible`, `:focus-within`, `:target`, `:valid`, `:invalid`, `:in-range`, `:out-of-range`, `:user-valid`, `:user-invalid`, `:autofill`, `:state()`, `:modal`, `:popover-open`, `:fullscreen`, `:picture-in-picture`, the media playback pseudo-classes, `:future`/`:past`, `:active-view-transition(-type)` and the view-transition pseudo-elements that need an active transition. Radio buttons are never indeterminate (no radio groups); `:dir(auto)` and `<bdi>` are LTR (no bidi data); `:lang()` has no document default language; the default button ignores the `form` attribute; a textarea's value is its child text.
- `::part()` doesn't follow `exportparts` (FIXME): part names match only in the element's own scope. User agent parts never match (no user agent shadow roots).
- `SelectorQuery` has no ID fast paths (`RightMostWithIdMatch`, root filtering; FIXME until tree scopes map IDs), no cached first element with an attribute, no document-level `querySelectorAll` result cache (`resultForSelectorAll`, `shouldStoreInDocument`, `classNameToMatch`), no `<base>` shortcut and no SVG animated attribute bailout (no SMIL). The cache key has no security origin and evicts its first entry when full.
- The test functions live in a `.cpp` (upstream: an inline header). `Element::Matches` is `const`.

**Open (DOM):** `HTMLTreeBuilder::CreateElement` passes the `is` attribute's *name* as the `is` value instead of its value. `HTMLInputElement::Checked`, `Indeterminate` and `Value`, and `HTMLOptionElement::Selected`, reflect content attributes rather than checkedness, indeterminacy, value and selectedness, so `:checked` follows the attributes until form control state exists.

Tests: `tests/CSS/Selectors/SelectorChecker.cpp` (simple selectors, quirks mode, combinators, `:not`/`:is`/`:where`/`:has`, tree-structural pseudo-classes, `:scope`/`:host`, element state, `:lang`/`:dir`, link match type), `tests/DOM/SelectorQuery.cpp` (every fast path, element/fragment/shadow root queries, no-namespace elements, `matches`/`closest`, syntax errors, the cache): 13 cases / 522 assertions pass; `NameValidation` and `QualifiedNameCache` tests for the DOM fixes. Full Booey suite: 1785 cases / 681534 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer e7: `matchMedia` (2026-10-14)

Graph from `css/MediaQueryMatcher.cpp`, `css/MediaQueryList.cpp` and `css/MediaQueryListEvent.cpp` (headers-only): 88 files (`tools/port/out/match-media.md`). Everything to port was already ported or belongs to style resolution (reached through `StyleResolver.h` and `ResolvedStyle.h`). The `.cpp` includes reach the frame and view (`LocalFrame`, `LocalFrameView`, `DocumentView`), `Quirks` and `HTMLFrameOwnerElement`, `ActiveDOMObject` and `EventNames`.

**Decisions (2026-10-14):**
- Font-relative units resolve against the initial font: the seam's `RenderStyle` reports `medium` (16px) with no font data, so the metrics use the css-values-4 fallbacks (FIXME until fonts). Media queries are specified against initial values, so `em` and `rem` are already correct.
- Change events are reported when the host calls `Document::EvaluateMediaQueriesAndReportChanges()` (upstream: the media query step of `Page::updateRendering`), for example after `SetMediaEnvironment` (FIXME: call it from "update the rendering" once there's a rendering loop). As upstream, `matches` keeps its result until then.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `css/MediaQueryMatcher.h`, `.cpp` | `have` | `CSS/Queries/MediaQueryMatcher.hpp`, `src/...` | on | tested. See differences. |
| `css/MediaQueryList.h`, `.cpp` | `have` | `CSS/Media/MediaQueryList.hpp`, `src/...` | on | tested. See differences. |
| `css/MediaQueryListEvent.h`, `.cpp` | `have` | `CSS/Media/MediaQueryListEvent.hpp`, `src/...` | on | tested. |
| `dom/Document.cpp` (`mediaQueryMatcher`, `evaluateMediaQueriesAndReportChanges`), `page/LocalDOMWindow.cpp` (`matchMedia`) | `port` | `Document::GetMediaQueryMatcher`, `EvaluateMediaQueriesAndReportChanges`, `MatchMedia` | on | tested. `matchMedia()` is on the document (there's no window). |
| `style/StyleResolver.h` (`styleForElement` with `MatchOnlyUserAgentRules`) | `?` | | | Style resolution; see differences. |

**Names:** `mediaQueryMatcher()` → `GetMediaQueryMatcher()` (it would hide the type). `MediaQueryListEvent::create(type, media, matches)` (the trusted event) is replaced by `EventDispatcher::FireEvent<MediaQueryListEvent>`, as the spec's "fire an event"; the script constructor is `MediaQueryListEvent(type, init)`.

**Bugs fixed (old Krystal headers):** `MediaQueryMatcher.hpp` declared `EvaluateAll` as `KRYS_NODISCARD void` and used `SmallList` for the media query list (`css::mq::MediaQueryList` is a `List`, and the forward declaration clashed with it). `MediaQueryList.hpp` derived from both `RefCounted<MediaQueryList>` and `dom::EventTarget` (two reference counts) and kept the dynamic dependencies as a single `MediaQueryDynamicDependency`. Its type cast traits always returned false (TODO); they're removed, since nothing casts to it. None of the three had a `.cpp`.

**Seam changes:** `FontMetrics`, `FontCascadeDescription` and `FontCascade` can be constructed, and `RenderStyle`'s font members return the initial font: 16px computed and specified size, an ascent of 16, a line spacing of 19 (`normal`, 1.2em), no x-height, cap height or zero width (so `ex` is 0.5em and `cap` the ascent), a `ch` of 8 (0.5em) and an `ic` of 16 (1em). `ComputedLineHeight` and `ComputeSpecifiedLineHeight` are 19, and `EvaluateFixedLineHeight` is null (`normal` isn't fixed).

**Differences:**
- The root element's style isn't resolved from the user agent rules (no style resolver; FIXME): `DocumentElementUserAgentStyle` is a default `RenderStyle`, which only differs in the font. As upstream, there's no style (so nothing matches) without a view (no `MediaEnvironment`; upstream: no frame) or a document element.
- The media type comes from the `MediaEnvironment` (upstream: the frame's view).
- `MediaQueryList` isn't an `ActiveDOMObject` (no garbage collection or suspension), so it has no script execution context, `virtualHasPendingActivity` or `eventListenersDidChange`; it doesn't need the document. There's no `onchange` (the DOM has no event handler attributes yet).
- `matches` doesn't re-evaluate viewport-dependent queries after updating the owner document's layout (FIXME: needs nested documents); it doesn't collect the dynamic dependencies, which only that used.
- The `shouldSilenceMediaQueryListChangeEvents` site quirk is dropped.
- No logging (`LOG_WITH_STREAM(MediaQueries, ...)`).

Tests: `tests/CSS/Media/MediaQueryList.cpp` (evaluation against the environment, font-relative units, no view or document element, cached results, `media` serialisation, change events: order, trust, targets, undone changes, scheduled changes, removed listeners, released lists, duplicate listeners, and the event constructor): 4 cases / 57 assertions pass. Full Booey suite: 1789 cases / 681591 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

## Style resolution (planned 2026-10-14)

From matched rules to computed styles. Upstream: the resolver (`style/` top level, ~115 files and ~31k lines: rule sets, matching, the cascade, the builder, the tree resolver, invalidation), the computed style it writes into (`style/computed`, 16 files, plus storage generated from `CSSProperties.json`), its data groups (`rendering/style`, 113 files, ~17.6k lines) and the computed value types (`style/values`, ~400 files not yet ported).

**Decisions (2026-10-14):**
- Delivered in two halves. f0 to f2 (rule sets, matching, the cascade: declared values, no computed style) are this slice; f3 to f6 (computed style, builder, resolver, output) are planned in detail when they're reached.
- The computed style follows upstream: `Style::ComputedStyle`, the `rendering/style` data groups and the generated `ComputedStyleProperties`/`RenderStyleProperties`, so the builder and extractor port line for line.
- Every standard property's computed value type is ported in f3, before the builder (no staging by property group).
- Layout stays out: `RenderView`/`RenderBox` remain seam stand-ins.

**Layers:**
- **f0, rule sets:** `RuleData`, `RuleSet`, `RuleSetBuilder`, the rest of `RuleFeature` (invalidation features), `StyleScopeRuleSets`; `SelectorFilter`, `HasSelectorFilter`, `SelectorMatchingState` and `StyleRelations`, wired into `SelectorChecker` (deferred from e6); `UserAgentStyle` and the user agent sheets (`html.css`, `quirks.css`, `svg.css`, `mathml.css`, `counterStyles.css`, `popover.css`, `fullscreen.css`, `viewTransitions.css`, without `-webkit-` rules); `style::Scope`'s resolver and rule set hooks (the seam's `style::Resolver` placeholder goes). Done (`PropertyAllowlist` came with it).
- **f1, matching:** `ElementRuleCollector`, `MatchResult`, `PageRuleCollector`, `PseudoElementRequest`. Tested as the matched declarations of an element in cascade order. Done (`MatchResultCache` moved to f5).
- **f2, the cascade:** `PropertyCascade` (origins, `!important`, layers, logical and physical properties, the rollback cascades for `revert`/`revert-layer`). Tested as the winning declared value of each property. Done (`StyleCustomProperty` moved to f3; `CustomPropertyRegistry` and `var()` resolution moved to f4).
- **f3, computed style storage:** `style/computed`, the `rendering/style` data groups, codegen for the computed style and render style properties, every remaining `style/values` type with its conversions, `StyleCustomProperty` (moved from f2). Replaces the seam's `RenderStyle`. Planned in sublayers f3a to f3f (2026-10-14); see *Layer f3: computed style storage (planned)*.
- **f4, the builder (done in sublayers f4a to f4d on 2026-10-15; see *Layer f4: the builder (planned)*):** `BuilderState`, `Builder`, `BuilderCustom`, `BuilderChecking`, codegen for the generated builder; font resolution (`StyleResolveForFont`, `StyleFontSizeFunctions`) against a font description without platform font data; the remaining deferred value definitions (`PendingSubstitutionValue`, `CreateStyleImage` without loading); moved from f2: `CustomPropertyRegistry`, `CSSRegisteredCustomProperty` and `@property` registration (they need the render style's custom property storage and the typed custom property parsing), `var()` resolution (the deferred `VariableReferenceValue` definitions and the typed custom property parser functions: the builder applies custom properties on demand) and applying `revert`/`revert-layer` (the builder builds the rollback cascades).
- **f5, the resolver (planned in sublayers f5a to f5d on 2026-10-15; see *Layer f5: the resolver (planned)*):** `Style::Resolver`, `TreeResolver`, `Update`, `StyleAdjuster`, `MatchedDeclarationsCache`, `MatchResultCache` (moved from f1: it caches the resolved style), `StyleResolveForDocument`; `ContainerQueryEvaluator` (`style()` queries; size queries are unknown without layout); the user agent root style in `MediaQueryMatcher`; presentational hints and `<body>` colours.
- **f6, output and invalidation:** `Style::Extractor` (and its generated half), `getComputedStyle` (`CSSComputedStyleDeclaration`), `StyleDifference`, `StyleInvalidator` and the `*ChangeInvalidation` classes, hooked to the DOM's change steps.
- **Deferred:** animations and transitions (`StyleInterpolation*`, `Styleable`, the changed animatable properties), `AnchorPositionEvaluator` (layout), `InlineTextBoxStyle`, resource loading.

### Layer f3: computed style storage (planned 2026-10-14)

Graphs (headers-only, `--follow css/ style/ rendering/style/`): from `style/computed/StyleComputedStyle.cpp`, `StyleComputedStyleBase.cpp` and `rendering/style/RenderStyle.cpp`, 211 files (`tools/port/out/f3-computed.md`); from every header in `style/values`, `style/computed` and `rendering/style`, 633 files (`tools/port/out/f3-all.md`). Upstream size: `style/values` 533 files / ~58k lines (Booey has `color`, most of `primitives` and `viewport`), `rendering/style` 113 files / ~17.6k lines, `style/computed` 16 files / ~4.2k lines, plus the generated `StyleComputedStyleProperties` and `RenderStyleProperties` (their generators are in `codegen/css_properties.py`, commented out). `RenderStyle` wraps a `Style::ComputedStyle`; the data groups (`StyleNonInheritedData`, `StyleRareInheritedData`, ...) live in `rendering/style` and hold the value types.

Boundaries outside `css/`, `style/` and `rendering/style/`: platform data (text and scroll flag enums, `StyleAppearance`, `ThemeTypes`, `TouchAction`, `TabSize`, `Float`/`LayoutRoundedRect`, the font description types, `TimingFunction`, transform and filter operations), platform drawing (`Path`, `Image`, `Gradient`, `GeneratedImage`, `GraphicsContext`), the loader (`CachedImage`, `CachedResourceLoader`), the renderer (`RenderElement`, `RenderBox`, `RenderView`, `RenderTheme`, `PathOperation`, `MotionPath`), animations (`ScrollTimeline`, `ViewTimeline`, `TimelineRangeValue`) and SVG (`SVGPathByteStream` and friends, `SVGLengthContext`, `SVGURIReference`).

**Decisions (2026-10-14):**
- Value types come with their conversions and creation (`CSSValueConversion`/`CSSValueCreation`, usually inline in the type's own files), tested from parsed CSS. They run against the seam's `BuilderState`, grown into a constructible stand-in with the part of upstream's interface they use (`setCurrentPropertyInvalidAtComputedValueTime`, `cssToLengthConversionData`, `style`, `parentStyle`, `document`, `element`, `styleScopeOrdinal`, `createStyleImage`, `zoomWithTextZoomFactor`, `useSVGZoomRulesForLength`, `positionTryFallback`, `cssPropertyID`, `registerContentAttribute`). The real `BuilderState` replaces it in f4.
- The ~38 value `.cpp`s that read the style (`state.style()`, `parentStyle()`) use the seam's `RenderStyle`, grown under upstream's member names as each sublayer needs them, so the switch to the real `RenderStyle` in f3f is transparent.
- Fonts: `FontDescription`, `FontCascadeDescription`, `FontSelectionAlgorithm`, `FontSizeAdjust`, `FontPalette` are ported in full; `FontCascade` is a shell holding its description (no font data, glyph or width APIs; FIXME until there is a font backend). They replace the seam's font stand-ins.
- Platform: data types are ported and tested (the flag enums, rounded rects, `TimingFunction`, transform and filter operations); drawing types (`Path`, `Image`, `Gradient`, `GeneratedImage`, `GraphicsContext`) are minimal stand-ins whose drawing and geometry entry points are FIXMEs.
- Style images: the `StyleImage` hierarchy (cached, gradient, cross-fade, filter, image set, named, paint, canvas, cursor, invalid, multi, generated) is ported without loading or drawing: values, equality, serialisation, `BuilderState::createStyleImage` (completing the deferred `CreateStyleImage`). `CachedImage` loading, `image()` and client hooks are FIXMEs.
- Storage follows Booey's `css-properties.json` (528 properties): properties dropped from it get no storage, and properties behind disabled `ENABLE` flags follow the generator. Animations stay deferred: `+Blending` headers already ported stay; new interpolation code isn't added, and the `GenerateStyleInterpolationWrapperMap`/`GenerateStyleChangedAnimatablePropertiesGenerated` generators stay off.

**Sublayers (leaves first; style value groups are `style/values/<group>`):**
- **f3a, foundations (~8k lines upstream), done (2026-10-14; the rounded rects moved to f3e, with borders, and `CSSPrimitiveValueMappings` came with it):** the remaining `primitives` (`StyleValueTypes` and its `+CSSValueConversion`, the keyword and numeric `+CSSValueConversion`/`+CSSValueCreation`/`+Serialization` headers, `LengthWrapper` conversions, `Position`, `URL`, `CoordinatedValueList`), the colour conversions deferred from layer 6 (`Color`, `DynamicRangeLimit`, `Opacity`), `BuilderChecking`, the seam's `BuilderState` made constructible (and a test helper that converts a parsed value), `RenderStyleConstants` reviewed against upstream, and the platform enums and data the groups need (`TextFlags`, `TextSpacing`, `UnicodeBidi`, `ScrollTypes`, `StyleAppearance`, `ThemeTypes`, `TouchAction`, `TabSize`, `FloatRoundedRect`, `LayoutRoundedRect`).
- **f3b, keyword and length groups (~17k; split in two on 2026-10-14: f3b1, the small groups, done; f3b2, `align`, `inline`, `text`, `scroll-snap`, `ui` and `grid`, done):** `box`, `sizing`, `flexbox`, `position`, `css2`, `display`, `break`, `multicol`, `page`, `math`, `rhythm`, `size-adjust`, `fill-stroke`, `contain`, `overflow`, `pointerevents`, `speech`, `view-transitions`, `will-change`, `line-grid`, `counter-styles`, `lists`, `color-adjust`, `align`, `inline`, `text`, `scroll-snap`, `scrollbars`, `ui`, `viewport` (`StyleZoom`), and the remaining `grid`. May be split in two when reached.
- **f3c, fonts (~2.7k, plus ~1.1k platform), done (2026-10-14):** the font description types, the `FontCascade` shell, `fonts`.
- **f3d, timing, transforms and filters (~11.7k, plus ~3.6k platform; split in three on 2026-10-14: f3d1, timing and animations, done; f3d2, transforms, done; f3d3, filters and anchor positioning, done, with `Path` moved to f3e):** `TimingFunction`, transform operations, filter operations, the `Path` stand-in; `easing`, `transforms` (and `transforms/functions`), `filter-effects`, `animations`, `transitions`, `scroll-animations`, `anchor-position`.
- **f3e, images and painted values (~17.7k, plus the image classes; split in three on 2026-10-14: f3e1, style images, `images`, `content` and the cursor, done; f3e2, `backgrounds`, `borders`, `text-decoration`, `non-standard`, done (with `MaskMode` from `masking`); f3e3, paths and `shapes`, done; f3e4, `masking`, `motion`, `svg`, with `PathOperation`, done):** the `StyleImage` hierarchy and the drawing stand-ins (with the `Path` stand-in, moved from f3d3); `images`, `backgrounds`, `borders`, `shapes`, `masking`, `motion`, `content`, `text-decoration`, `svg`, `non-standard` (only the types behind properties still in `css-properties.json`).
- **f3f, storage (~22k, plus generated code; split in four on 2026-10-14: f3f1, the storage leaves and custom properties, done; f3f2, the generators; f3f3, the data groups with `style/computed`; f3f4, `RenderStyle`):** the `rendering/style` data groups and value classes (`BorderData`, `BorderValue`, `OutlineValue`, `CounterDirectives`, `SVGRenderStyle`, ...), `StyleCustomProperty` and `StyleCustomPropertyData`, `style/computed`, the `GenerateStyleComputedStyleProperties` and `GenerateRenderStyleProperties` generators (reviewed against upstream's output, as in c1), and `RenderStyle`, which replaces the seam's. Tested as initial values, getters and setters, copy-on-write of the data groups, inheritance (`inheritFrom`), equality and the custom property storage. Done (2026-10-15).

**Tests:** each value type: conversion from parsed CSS through the seam's `BuilderState` (including invalid-at-computed-value-time cases), creation back to a CSS value, serialisation and equality; platform data types against upstream's API tests where they exist.

### Layer f4: the builder (planned 2026-10-15)

Graph (headers-only, `--follow css/ style/ rendering/style/`) from `style/StyleBuilder.cpp`, `StyleBuilderState.cpp`, `StyleResolveForFont.cpp`, `StyleFontSizeFunctions.cpp`, `StyleCustomPropertyRegistry.cpp`, `css/CSSRegisteredCustomProperty.cpp`, `CSSVariableReferenceValue.cpp` and `CSSPendingSubstitutionValue.cpp`: 252 files (`tools/port/out/f4-all.md`), nearly all ported in f0 to f3. New: the entries, `StyleBuilderStateInlines.h`, `StyleBuilderCustom.h`, `StyleBuilderGenerated.h` (and the `GenerateStyleBuilderGenerated` generator), `CSSWideKeyword.h`. The resolver's files in the graph (`StyleUpdate`, `StyleChange`, `ContainerQueryEvaluator`, `MatchedDeclarationsCache`, `StyleAdjuster`) are f5's; `Styleable` is animations'.

**Sublayers:**
- **f4a, the builder state, done (2026-10-15):** `BuilderState` (and its inlines), replacing the seam's, and `StyleFontSizeFunctions`.
- **f4b, the generated builder, done (2026-10-15):** `GenerateStyleBuilderGenerated`, `StyleBuilderGenerated.h` and `StyleBuilderCustom.h` (the per-property apply functions: value, initial, inherit), with `maybeUpdateFontForLetterSpacingOrWordSpacing`.
- **f4c, registered custom properties, done (2026-10-15):** `CustomPropertyRegistry`, `CSSRegisteredCustomProperty`, `@property` registration in the resolver, the typed custom property parsing (`ParseTypedCustomPropertyValue`, `ParseTypedCustomPropertyInitialValue`, `ParseInitialValueForUniversalSyntax`) and `RenderStyle::createPtrWithRegisteredInitialValues`.
- **f4d, the builder, done (2026-10-15):** `Builder` (applying the cascade by priority, custom properties with cycle detection, logical groups, the rollback cascades for `revert`/`revert-layer`), `var()` resolution (the deferred `VariableReferenceValue` definitions) and `PendingSubstitutionValue::ResolveValue`. The builder test fixture's `SetCurrentProperty` hook goes (the builder sets the current property).

**Decisions (2026-10-15):**
- `StyleResolveForFont` is deferred to the `FontFace` interface and the canvas 2D context: its only callers are `CSSFontFace`, `EditingStyle` and the canvas contexts, and `resolveForUnresolvedFont` needs the unported `ParseUnresolvedFont` (see *Where we are*).

### Layer f5: the resolver (planned 2026-10-15)

Graphs (headers-only, `--follow css/ style/ rendering/style/`) from `style/StyleResolver.cpp`, `StyleTreeResolver.cpp`, `StyleAdjuster.cpp`, `MatchedDeclarationsCache.cpp`, `MatchResultCache.cpp`, `StyleResolveForDocument.cpp`, `ContainerQueryEvaluator.cpp`, `StyleUpdate.cpp`, `StyleChange.cpp` and `StyleRelations.cpp` (`tools/port/out/f5-*.md`): nearly everything in `css/`, `style/` and `rendering/style/` is ported. New: the entries, `ResolvedStyle.h`, `Styleable.h` (the element and pseudo-element pair; its animation half stays with animations), `dom/VisibilityAdjustment.h` (deferred in f5a). Upstream size: ~6.5k lines. Outside: the DOM's style state (`Node`'s style validity bits, `ElementRareData`'s computed styles, `NodeRenderStyle.h`, the composed tree iterators, `Document::resolveStyle`), the render tree (`RenderTreeUpdater`, `RenderView`, `RenderTheme`), the font selector (`CSSFontSelector`), animations (`Styleable`'s timelines and effects, `BlendingKeyframes`), anchor positioning and layout interleaving, `Settings`, frames and pages.

**Decisions (2026-10-15):**
- No render tree: resolved styles are kept on the elements. The DOM's style state is ported (validity flags, `ElementRareData`'s `computedStyle`/`displayContentsOrNoneStyle`, `Node::renderStyle`, `Element::computedStyle`/`resolveComputedStyle`), and a render tree updater stand-in applies a `Style::Update` by storing each element's style on the element (FIXME: the render tree). f6 builds on it.
- Deferred, as FIXMEs: animations (`styleForKeyframe`, `keyframeStylesForAnimation`, `adjustAnimatedStyle`, the tree resolver's animated updates), anchor positioning and position options, layout interleaving, the render theme's appearance adjustment, the font selector, site-specific quirks (`wont`) and text autosizing (iOS, `wont`).

**Sublayers:**
- **f5a, the adjuster, done (2026-10-15):** `Style::Change` (`determineChanges`), `ResolvedStyle`/`UnadjustedStyle`, `Style::Adjuster` (but `propagateToDocumentElementAndInitialContainingBlock`, which needs `Update`: f5d), and `Adjuster::adjustFromBuilder` in `Builder` (f4d's FIXME). Tested as the adjusted style of an element given its parent's.
- **f5b, element resolution:** `Resolver`'s `unadjustedStyleForElement`, `styleForElement`, `styleForPseudoElement`, `styleForPage`, `defaultStyleForElement`, `styleRulesForElement`/`pseudoStyleRulesForElement`, `setViewTransitionStyles`, `invalidateMatchedDeclarationsCache`; `MatchedDeclarationsCache`, `MatchResultCache`, `StyleResolveForDocument` (the root default style, and the user agent root style in `MediaQueryMatcher`), committing style relations (`StyleRelations.cpp`). Tested as resolved styles from style sheets.
- **f5c, presentational hints and container queries:** `StyledElement`'s presentational hint style and `collectPresentationalHintsForAttribute` on the HTML, SVG and MathML elements, the table's additional presentational hint style, `<body>` colours (`link`, `vlink`, `alink`, `text`, `bgcolor`), and `ContainerQueryEvaluator` (`style()` queries; size queries are unknown without layout) with `SelectorMatchingState`'s container query state.
- **f5d, the tree resolver:** the DOM's style state (see decisions), the composed tree iterators, `Styleable`, `Update`, `TreeResolver`, `PostResolutionCallbackDisabler`, `Adjuster::propagateToDocumentElementAndInitialContainingBlock`, `Document::resolveStyle`/`updateStyleIfNeeded` and the render tree updater stand-in. Tested as the styles of a document's elements after resolution, and after changes.

### Layer f5a: the adjuster (2026-10-15)

Graphs: `tools/port/out/f5-StyleAdjuster.md`, `f5-StyleChange.md` (the f5 plan's). `style/StyleAdjuster.h`, `.cpp`, `StyleChange.h`, `.cpp`, `ResolvedStyle.h`, and what the adjuster needs from the DOM and from `RenderStyle+GettersInlines.h`.

**Decisions (2026-10-15):**
- `VisibilityAdjustment` (set by the page's element targeting) isn't ported: elements have no visibility adjustment, so `adjustVisibilityForPseudoElement` and the force-hidden subtree adjustment are FIXMEs.
- `Element::isReplaced` comes now (a virtual, as upstream), with its overrides on `<img>` (unless it has `content`), `<input type=image>`, media elements, `<iframe>`, `<embed>`, `<object>` (no plug-in replacements), outermost `<svg>` and `<canvas>` (not replaced: no scripting; FIXME). So do the containment functions (`shouldApply*Containment`, `isSkippedContentRoot`) that were a FIXME in `RenderStyle.hpp`, and `Element::isRelevantToUser`, which is always false (no content relevancy; FIXME).
- Settings use upstream's defaults: `layerBasedSVGEngineEnabled` and `css3DTransformBackfaceVisibilityInteroperabilityEnabled` are off, so their branches are FIXMEs; `hiddenUntilFoundEnabled` is on; `detailsAutoExpandEnabled` only applies in user agent shadow trees (none; FIXME).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleAdjuster.h`, `.cpp` | `port` | `Style/Adjuster.hpp`, `src/...` | on | tested. See differences. |
| `style/StyleChange.h`, `.cpp` | `port` | `Style/Change.hpp`, `src/...` | on | tested. No `TextStream` operators. |
| `style/ResolvedStyle.h` | `port` | `Style/ResolvedStyle.hpp` | on | Used from f5b. |
| `dom/VisibilityAdjustment.h` | `port` (deferred) | | | See decisions. |
| `dom/Element.h` (`isReplaced`, `isRelevantToUser`) and the `isReplaced` overrides | `port` | `dom::Element`, the HTML and SVG elements | on | tested. See decisions. |
| `html/HTMLElement.cpp` (`isHiddenUntilFound`), `HTMLInputElement.cpp` (`isPasswordField`, `isImageButton`) | `port` | `html::HTMLElement`, `html::HTMLInputElement` | on | tested. From the `hidden` and `type` attributes. |
| `svg/SVGElement.cpp` (`isOutermostSVGSVGElement`, `isInnerSVGSVGElement`) | `port` | `svg::SVGElement` | on | tested. |
| `rendering/style/RenderStyle+GettersInlines.h` (`shouldApply*Containment`, `isSkippedContentRoot`) | `port` | `Rendering/Style/RenderStyle+GettersInlines.hpp` | on | tested. Was a FIXME. |

**Names:** `adjust()` → `Adjust()` and so on. `Change` is an `EnumSet` of sequential values (upstream: an `OptionSet` of bit flags); `inheritedChanges()` → `InheritedChanges()`, `determineChanges` → `DetermineChanges`. `ResolvedStyle`/`UnadjustedStyle` members are PascalCase, writing their types as `style::X`.

**Added (upstream API):** `Adjuster` (`AdjustFromBuilder`, which `Builder::AdjustAfterApplying` now calls; `Adjust`; `AdjustFirstLetterStyle`; `AdjustFirstLineStyle`; `AdjustSVGElementStyle`; `AdjustEventListenerRegionTypesForRootStyle`; `RestoreUsedDocumentElementStyleToComputed`), `Change`, `InheritedChanges`, `DetermineChanges`, `UnadjustedStyle`, `ResolvedStyle`, `Element::IsReplaced`/`IsRelevantToUser`, `HTMLElement::IsHiddenUntilFound`, `HTMLInputElement::IsPasswordField`/`IsImageButton`, `SVGElement::IsOutermostSVGSVGElement`/`IsInnerSVGSVGElement`, `ShouldApply{Layout,Size,InlineSize,Style,Paint}Containment`, `IsSkippedContentRoot`.

**Bug fixed (DOM):** the SVG element factory never created `SVGForeignObjectElement`: `<foreignObject>` was a plain `SVGElement` (so an `<svg>` inside it wasn't an outermost one, and the adjuster's `<foreignObject>` rules didn't apply). Other SVG elements still fall back (TODO in the factory).

**Differences:**
- No top layer (modal dialogs, popovers, fullscreen): only `::backdrop` is in the top layer, the document element is never an inert subtree root for a modal dialog or fullscreen element, and popover invokers aren't marked (FIXMEs).
- No user agent shadow trees: no `user-modify: read-only` on user agent parts, no media control text decoration boundary, no `<details>` auto-reveal (FIXMEs). No view transitions: an element captured in one isn't forced flat (FIXME).
- No render theme: `adjustThemeStyle` keeps its containment fix-up, but the theme's `adjustStyle` is a FIXME. No frame view: paged overflow doesn't set the column styles from the pagination mode (FIXME).
- Event listener regions (wheel, touch, interaction regions) are platform features that aren't enabled: the types are the parent's, and the root's come from no listeners (no window; FIXME).
- `-webkit-marquee-direction` was dropped, so `<marquee>` is always horizontal (no wrapping, start-aligned; the vertical 200px height never applies).
- `wont`: site-specific quirks (`adjustForSiteSpecificQuirks`), text autosizing (iOS), the Cocoa intrinsic margins for form controls, `-webkit-overflow-scrolling` and `appleVisualEffect`.
- `propagateToDocumentElementAndInitialContainingBlock` comes with `Update` (f5d); `adjustAnimatedStyle` with animations.

Tests: `tests/Style/Adjuster.cpp` (blockification by floats, positioning, the document element, `<legend>` and flex/grid parent boxes; ruby display types, inlinification and bidi isolation; `display: contents`; table parts; writing modes; grid auto flow; used z-index, including the builder's adjustment; overflow, including tables and text areas; 3D flattening; effective transparency; text decorations in effect; used touch action; legacy `justify-items`; password fields; marquees; `hidden=until-found`; `inert`; outermost and inner `<svg>` and `<foreignObject>`; `content-visibility`, the containment functions and replaced elements; first-letter and first-line; restoring the document element's writing mode; `DetermineChanges`): 6 cases; `[Change]`: 1 case. Full Booey suite: 2009 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f4d: the builder (2026-10-15)

Graph: the f4 plan's. `style/StyleBuilder.h`, `.cpp`, the deferred `CSSVariableReferenceValue` definitions (`var()` and `env()` resolution, fallbacks, the simple-reference shortcut, `resolveSingleValue`) and `CSSPendingSubstitutionValue::resolveValue`.

**Decisions (2026-10-15):**
- `Builder` keeps its state as `UniquePtr<BuilderState>` (the state can be checked-pointed); the rollback cascades are keyed by origin, scope ordinal and layer priority (upstream's tuple without its hash-table empty-value flag). The applied properties walk the priority ranges with `HasNormalProperty` (the cascade's present-property set is a `std::bitset`, with no set-bit iteration).
- FIXMEs: `Adjuster::adjustFromBuilder` (the style adjuster; done in f5a), `AnchorPositionEvaluator::resolvePositionTryFallbackProperty` (anchor positioning), `paint()` worklet input properties (paint worklets), and `env()`, which has no values without the document's `ConstantPropertyMap` (the page): an `env()` reference falls back, or is invalid at computed-value time.
- The builder tests' `BuilderState::SetCurrentProperty` hook is gone: only `Builder` sets the current property. The conversion fixture recreates its state for each conversion (so the invalid-at-computed-value-time flag is per conversion), and the conversions that read the current property's id, origin or style scope (scoped names, `view-transition-name`, `anchor-scope`/`timeline-scope`, `position-try-fallbacks`, self-alignment under position-try fallbacks, the explicitly-set flags) are tested by building the style with `BuilderFixture::Build` (an author declaration through `Builder`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleBuilder.h`, `.cpp` | `port` | `Style/Builder.hpp`, `src/...` | on | tested. |
| `css/CSSVariableReferenceValue.cpp` (resolution) | `port` | `src/CSS/Values/VariableReferenceValue.cpp` | on | tested. `env()` is a FIXME. |
| `css/CSSPendingSubstitutionValue.cpp` | `port` | `src/CSS/Values/PendingSubstitutionValue.cpp` | on | tested. |

**Added (upstream API):** `Builder` (applying by priority, logical groups, custom properties with cycle detection, registered custom properties, `revert`/`revert-layer` with the rollback cascades, invalid-at-computed-value-time handling, `@page` `size`, `ResolveCustomPropertyForContainerQueries`), `VariableReferenceValue::ResolveVariableReferences` and `ResolveSingleValue`, `PendingSubstitutionValue::ResolveValue`.

**Differences:** as in the decisions.

Tests: `tests/Style/Builder.cpp` (the cascade and priorities, global keywords, logical properties, `revert` and `revert-layer` for properties and custom properties, `var()` substitution, fallbacks, invalid references, shorthands, inheritance, cycles, keywords in fallbacks, registered custom properties and their syntax, font-relative cycles, container query resolution): 5 new cases; the tests that used the hook rewritten. Full Booey suite: 2002 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f4c: registered custom properties (2026-10-15)

Graph: the f4 plan's. `style/StyleCustomPropertyRegistry.h`, `.cpp`, `css/CSSRegisteredCustomProperty.h`, `.cpp`, the typed custom property parsing in `CSSPropertyParser.cpp` (`parseTypedCustomPropertyValue`, `parseTypedCustomPropertyInitialValue`, `consumeTypedCustomPropertyValue`), `CSSVariableParser::parseInitialValueForUniversalSyntax`, `RenderStyle::createPtrWithRegisteredInitialValues`, and the registry's users in `StyleScope` and `RuleSetBuilder`.

**Decisions (2026-10-15):**
- `css::RegisteredCustomProperty` is a plain struct (PascalCase members); its out-of-line destructor (upstream's, for incomplete types) isn't needed, so there's no `.cpp`. The registry keeps both maps as `HashMap<CSSOMStringAtom, UniquePtr<RegisteredCustomProperty>>`.
- FIXMEs: invalidating the matched declarations cache (f5), `Document::setHasStyleWithViewportUnits()` and the scope's viewport unit invalidation that calls `InvalidatePropertiesWithViewportUnits` (no viewport changes yet), and telling keyframe effects about registrations (animations).
- `Resolver::DefaultStyleForElement` starts from the registry's initial values (upstream also does for `initializeStateAndStyle` and `styleForPseudoElement`, which come with element resolution in f5).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleCustomPropertyRegistry.h`, `.cpp` | `port` | `Style/CustomPropertyRegistry.hpp`, `src/...` | on | tested. |
| `css/CSSRegisteredCustomProperty.h`, `.cpp` | `port` | `CSS/Properties/RegisteredCustomProperty.hpp` | on | Replaces the unreviewed raw file. |

**Added (upstream API):** `CustomPropertyRegistry` (with `Scope::GetCustomPropertyRegistry()`), `@property` registration in `RuleSetBuilder` (ignored in shadow trees), clearing the style sheets' registrations with the resolver, `RenderStyle::CreatePtrWithRegisteredInitialValues`, `PropertyParser::ParseTypedCustomPropertyValue` and `ParseTypedCustomPropertyInitialValue`, `VariableParser::ParseInitialValueForUniversalSyntax`.

**Differences:** as in the decisions.

Tests: `tests/Style/CustomPropertyRegistry.cpp` (registration order and precedence, invalid initial values, the universal syntax, viewport units, parsing initial values, the prototype style and default styles, `@property` rules and shadow trees) and `tests/CSS/Properties/PropertyParser.cpp` (typed values, lists, the universal syntax, global keywords, mismatches, initial values): 4 new cases. Full Booey suite: 1997 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f4b: the generated builder (2026-10-15)

Graph: the f4 plan's. `style/StyleBuilderGenerated.h`, `StyleBuilderCustom.h` and the `GenerateStyleBuilderGenerated` generator (`process-css-properties.py`), which writes `Style/BuilderGenerated.cpp` into the build's codegen directory (not committed). `StyleBuilderCustom.h` was translated by `conv.py`, then by hand; the generator was rewritten to emit Krystal code.

**Decisions (2026-10-15):**
- The generator emits `BuilderFunctions` (the generated apply functions) and `BuilderGenerated::ApplyProperty` into `krys::boo::style`, with property ids as `css::PropertyId::X`. `ASSERT_NOT_REACHED` for `Custom` and shorthands is `krys_debug_assert_msg(false, ...)`, as elsewhere. The file is compiled with `/bigobj` (MSVC's section limit).
- Dropped custom handlers: `applyValueWebkitLocale` (dead upstream: `-webkit-locale` is a generated font property), `applyValueFontSizeAdjust` (declared but never defined upstream), `applyValueWebkitTextZoom` (`-webkit-text-zoom` isn't in the JSON), and the `forwardInheritedValue` overloads for `-webkit-box-reflect`, `-webkit-line-clamp`, `-webkit-line-grid` and `-webkit-marquee-increment` (dropped properties). `color-scheme`'s handler stays (`ENABLE_DARK_MODE_CSS` is on); `line-height`'s isn't custom (`ENABLE_TEXT_AUTOSIZING` is off, as on most upstream ports).
- The font description accessor names drop `Webkit` as the render style ones do (`-webkit-font-smoothing` → `SetFontDescriptionFontSmoothing`, upstream's name). The coordinated value list getters the generator names are `GetAnimations()` and so on (they'd hide the types).
- The legacy inter-character ruby check walks `ParentElement()` for an HTML `rt` (no ancestor iterators yet).
- `CSSValueConversion<GridPosition>` (deferred in f3b2) is ported here: the builder needs it.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleBuilderGenerated.h` | `port` | `Style/BuilderGenerated.hpp` | on | tested. |
| `css/process-css-properties.py` (`GenerateStyleBuilderGenerated`) | `port` | `codegen/css_properties.py` → `Style/BuilderGenerated.cpp` (generated) | on | tested. |
| `style/StyleBuilderCustom.h` | `port` | `Style/BuilderCustom.hpp` | on | tested. |
| `style/values/grid/StyleGridPosition.cpp` (conversion) | `port` | `src/Style/Values/Grid/GridPosition.cpp` | on | |

**Added (upstream API):** `BuilderGenerated::ApplyProperty`, `BuilderCustom`, `ForwardInheritedValue`, the coordinated value list apply functions, `MaybeUpdateFontForLetterSpacingOrWordSpacing`, `CSSValueConversion<GridPosition>`.

**Differences:** as in the decisions.

Tests: `tests/Style/BuilderGenerated.cpp` (values, initial values, inheritance, keywords, fast path inheritance, visited link colours, font properties and the system font check, coordinated value lists) and `tests/Style/BuilderCustom.cpp` (colours and `currentcolor`, caret colour, explicitly set flags, line widths under zoom, counters, font sizes and families, zoom, spacing): 11 new cases. Full Booey suite: 1993 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f4a: the builder state (2026-10-15)

Graph: the f4 plan's. `style/StyleBuilderState.h`, `.cpp`, `StyleBuilderStateInlines.h`, `StyleFontSizeFunctions.h`, `.cpp`. Translated by `conv.py`, then by hand; the seam's `BuilderState` (and `BuilderContext`, `BuilderPositionTryFallback`) moved here, and its users include `Style/BuilderState.hpp`.

**Decisions (2026-10-15):**
- Settings: the seam has a `SettingsValues` with the font size settings (`defaultFontSize` 16, `defaultFixedFontSize` 13, `minimumFontSize` 0, `minimumLogicalFontSize` 0: the WebCore defaults) and `evaluationTimeZoomEnabled` (false; upstream turns it on for Cocoa), returned by `DocumentSettingsValues(document)` until there's a `Settings` port. `evaluationTimeZoomEnabled(BuilderState)` reads it (it was a seam member of `BuilderState`).
- `updateFont()` always updates: upstream updates when the font is dirty or the cascade has no fonts, and cascades have no fonts without a font backend. The font selector update (`fontCascade().update(&fontSelector)`) and `updateUseBackslashAsYenSymbol` are FIXMEs (the font cache).
- No frames: the text zoom factor is 1 (`zoomWithTextZoomFactor`, `computedFontSizeFromSpecifiedSize`, `updateFontForZoomChange`); `-webkit-text-zoom` and text autosizing aren't supported.
- `disableNativeAppearanceIfNeeded` uses the seam's `RenderTheme::HasAppearanceForElementTypeFromUAStyle` (upstream's list, without the user agent shadow tree part) for `Element::isDevolvableWidget()` too (the same elements; FIXME until the form controls).
- `treeResolutionState` and `anchorPositionedStates()` come with the tree resolver (FIXME); `SetCurrentProperty` stayed for the tests until `Builder` (removed in f4d). `fontDescription()` is `GetFontDescription()`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleBuilderState.h`, `.cpp`, `StyleBuilderStateInlines.h` | `port` | `Style/BuilderState.hpp`, `BuilderStateInlines.hpp`, `src/Style/BuilderState.cpp` | on | tested. Replaces the seam's (and the unreviewed raw `Style/BuilderState.hpp`). |
| `style/StyleFontSizeFunctions.h`, `.cpp` | `port` | `Style/FontSizeFunctions.hpp`, `src/...` | on | tested. |
| `style/StyleResolveForFont.h`, `.cpp` | `defer` | | | With `FontFace` and the canvas 2D context. |

**Added (upstream API):** the font description setters, `setZoom`, `setUsedZoom`, `setWritingMode`, `setTextOrientation`, `fontDirty`, `applyPropertyToRegularStyle`/`applyPropertyToVisitedLinkStyle`, `setFontSize`, `disableNativeAppearanceIfNeeded`, the font updates and `adjustStyleForInterCharacterRuby` (private, for `Builder`), and the builder's custom property and link match state. Seam: `SettingsValues`, `DocumentSettingsValues`, `RenderTheme::HasAppearanceForElementTypeFromUAStyle`.

**Differences:** as in the decisions.

Tests: `tests/Style/FontSizeFunctions.cpp` (keyword sizes in both tables and outside them, computed sizes with the minimum rules, zoom and SVG, legacy sizes, size adjustment) and `tests/Style/BuilderState.cpp` (font description setters and dirtiness, sizes under zoom, keyword sizes, variant groups, native appearance; the seam's cases renamed): 6 new cases. Full Booey suite: 1982 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3f4: RenderStyle (2026-10-15)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). `rendering/style/RenderStyle.h`, `.cpp`, `RenderStyle+GettersInlines.h`, `RenderStyle+SettersInlines.h`, `RenderStyleBase.h`, `.cpp`, `RenderStyleBase+ConstructionInlines.h`, `+GettersInlines.h`, `+SettersInlines.h`, `rendering/PaintPhase.h`, and the `GenerateRenderStyleProperties` generator. `RenderStyle` replaces the seam's: its callers include `RenderStyle+GettersInlines.hpp` (and `+SettersInlines.hpp` when they set), and move to the generated names. Translated by `conv.py` and scripts, then fixed by hand against the build.

**Decisions (2026-10-15):**
- `RenderStyleProperties` forwards every generated accessor to `ComputedStyle` under the same names (so `GetMathDepth()`, `GetTextAlign()`, `GetAspectRatio()`, ... replace the seam's `MathDepth()`, `TextAlign()`, `AspectRatio()`), `noexcept` and `KRYS_NODISCARD` on getters; upstream's `constexpr` forwarding getters are plain `inline`, since `ComputedStyle`'s aren't all `constexpr`.
- Styles aren't checked-pointer-able (`CanMakeCheckedPtr`): upstream exempts them from the heap-deletion check (`CheckedPtrDeleteCheckException::Yes`, marked as not for new code) because they live on the stack and in other objects; Krystal's has no exemption, and nothing holds checked pointers to styles.
- The clone tag is spelled `CloneTag::Clone` where `RenderStyle::Clone()` hides the enumerator. `RenderStyle::usedFloat()`/`usedClear()` (static, they read the renderer's containing block) are `GetUsedFloat()`/`GetUsedClear()` in name only: they and `isVisibleToHitTesting` (`HitTestRequest`) come with the renderers (FIXME), and `shouldApply{Layout,Size,InlineSize,Style,Paint}Containment` and `isSkippedContentRoot` with `dom::Element::IsReplaced`/`IsRelevantToUser` (FIXME).
- Dropped with their properties or features: `setColumnStylesFromPaginationMode` and `hasInlineColumnAxis` (`-webkit-column-axis`), `hasBoxReflect`, `usesLegacyScrollbarStyle` (`::-webkit-scrollbar`), and the `-webkit-text-stroke-*` fallbacks: without an explicit `stroke-color` the stroke width is the initial text stroke width (0) and the stroke colour `currentcolor`, and `hasPositiveStrokeWidth()` is false without an explicit `stroke-width`.
- `createPtrWithRegisteredInitialValues` comes with the custom property registry.
- `hyphenString()` returns the hyphen-minus: fonts have no glyph data to choose U+2010 (FIXME).
- `usedAccentColor` takes `style::ColorOptions` and resolves `Canvas` through `css::ColorFromKeyword` (the seam's system colours), as the rest of the colour code does.
- `Resolver::DefaultStyleForElement` (upstream's `defaultStyleForElement`) is static and takes the document, since `MediaQueryMatcher` has no way to reach the document's resolver yet; both use it for the stand-in root style, whose font is the default `medium` (16px; FIXME: the settings' size through `fontSizeForKeyword`). The seam's `RenderStyle` started with that font; a `RenderStyle` now starts with upstream's empty font description.
- Seam: `RenderStyle` and its helpers (`EvaluateFixedLineHeight`, `ComputeSpecifiedLineHeight`, `HasDarkOnlyColorScheme`, `AdjustFloatForAbsoluteZoom`) are gone; length resolution uses upstream's `lineHeight().tryFixed()` and `computeLineHeight(specifiedLineHeight())`, and `DocumentUseDarkAppearance` reads the style's `color-scheme` (dark without light, the page being light).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `rendering/style/RenderStyle.h`, `.cpp`, `RenderStyle+GettersInlines.h`, `RenderStyle+SettersInlines.h` | `port` | `Rendering/Style/RenderStyle*.hpp`, `src/...` | on | tested. |
| `rendering/style/RenderStyleBase.h`, `.cpp`, `RenderStyleBase+ConstructionInlines.h`, `+GettersInlines.h`, `+SettersInlines.h` | `port` | `Rendering/Style/RenderStyleBase*.hpp`, `src/...` | on | tested through `RenderStyle`. |
| generated `RenderStyleProperties.h`, `+GettersInlines.h`, `+SettersInlines.h` | `port` | generated `Krystal.Booey/Rendering/Style/RenderStyleProperties*.hpp` | on | |
| `rendering/PaintPhase.h` | `port` | `Rendering/PaintPhase.hpp` | on | `PaintBehavior` for `visitedDependentColor`. |

**Bugs fixed:**
- The generator's render style includes pointed at `Krystal/HTML/CSS/Style/...` (files that don't exist).
- Tests had tuned `lh`/`rlh` to the seam's rounded line spacing (19px); upstream's `computeLineHeight` uses the unrounded `lineSpacing()` (19.2px for the synthesized 16px font).

**Added (upstream API):** `Resolver::DefaultStyleForElement`; `PaintPhase` and `PaintBehavior`.

**Differences:** as in the decisions. `RenderStyle` has no default constructor (upstream's `create()`); tests build styles with `RenderStyle::Create()` or from the default style.

Tests: `tests/Rendering/Style/RenderStyle.cpp` (initial values against the default style, getters and setters across the flags, data groups and border edges, visited-link and has-explicitly-set accessors, logical accessors under `direction` and `writing-mode`, copy-on-write of clones and `Replace`, inheritance (`InheritFrom`, `CopyNonInheritedFrom`, fast path, anonymous styles), the pseudo-element style cache and cloning it, colour resolution through `currentcolor`, logical properties and visited links, zoom adjustment): 8 new cases; this also covers `ComputedStyle`'s getters, setters, inheritance and equality. The builder test fixture's styles start from the default style. Full Booey suite: 1976 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3f2 and f3f3: computed style generator, data groups and computed style (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`, `f3-computed.md`). Built together, since the data groups' constructors call the generated `ComputedStyle::Initial*()` functions and the generated `ComputedStyleProperties` sits between `ComputedStyleBase` and `ComputedStyle`. The data groups (`rendering/style/Style*Data.h`, `.cpp`, `BorderData.h`, `.cpp`, `SVGRenderStyle.h`, `.cpp`, `SVGRenderStyleDefs.h`, `.cpp`), `style/computed` (`StyleComputedStyleBase` and `StyleComputedStyle` with their `+ConstructionInlines`, `+GettersInlines`, `+SettersInlines` and `+InitialInlines`, and the `StyleComputedStyleProperties+*Custom*` files), `style/StyleNameScope.h`, `.cpp`, `style/values/scroll-animations/StyleScrollTimelines.h` and `StyleViewTimelines.h`, and the `GenerateStyleComputedStyleProperties` generator. Translated by `conv.py` and scripts (include resolution, member renames, boilerplate removal), then fixed by hand against the build.

**Decisions (2026-10-14):**
- Generated files go to the generated headers/sources directories at the path a hand-written file would have (`Krystal.Booey/Style/Computed/ComputedStyleProperties.hpp`, `Krystal.Booey/Rendering/Style/RenderStyleProperties.hpp`, `Style/BuilderGenerated.cpp`, ...), so they're included like any other header; `output_hpp_path`/`output_cpp_path` create the directories. The disabled builder, extractor, interpolation and changed-animatable generators are remapped too.
- `ComputedStyleProperties` and the computed style classes are in `krys::boo::style`. The generated accessors are PascalCase (`Width()`, `SetWidth()`, `InitialWidth()`, `VisitedLinkColor()`, `HasExplicitlySetColor()`, `LogicalWidth()`, `MarginStart()`); a getter named after a type in `krys::boo` or `krys::boo::style` takes `Get` (193 of them: `GetColor()`, `GetMathDepth()`, `GetPosition()`), since it would hide the type inside the class. The generator finds the type names by scanning the include tree (`krystal_type_names()`). The hand-written accessors follow the same rule (`GetFontDescription()`, `GetCounterDirectives()`).
- Storage names: the generator's default storage name is the capitalised upstream getter; `border-image-*` and `mask-border-*` name their `BorderImage`/`MaskBorder` members (`source`, ...) in `css-properties.json`, since those members are `Source`/`Slice`/`Width`/`Outset`/`Repeat`. Data group members are PascalCase; `color` is stored as the platform `boo::Color` (upstream's `Color` in `WebCore`).
- Generated code is `noexcept`, `KRYS_NODISCARD` on getters and initial values, braces every body, and uses `ToUnderlying` for enum bit-fields, `Access()`, `FromRaw`/`ToRaw` and the `WritingMode` API.
- Dropped with their properties (not in `css-properties.json`): the data group members for `-webkit-text-stroke-*`, `-webkit-line-grid`, `-webkit-nbsp-mode`, `color-rendering`, `-webkit-line-snap`, `-webkit-text-zoom`, `-webkit-line-clamp`, `-webkit-box-reflect`, `-webkit-column-axis`, the marquee and the deprecated flexible box (`StyleMarqueeData`, `StyleDeprecatedFlexibleBoxData`), and the `-webkit-box-direction` inherited flag; `ComputedStyle::setColumnStylesFromPaginationMode` (sets `-webkit-column-axis`, for the frame view's pagination). `PublicPseudoIDBits` is 17: there's no `::-webkit-scrollbar`.
- `StyleShadowSVGData` (`SVGRenderStyleDefs`) is dropped: dead upstream (nothing holds one).
- `dump`/`dumpDifferences`/`operator<<(TextStream&)` aren't ported (logging), as in f3f1. `checkedFontCascade()` isn't ported: `FontCascade` isn't checked-pointer-able.
- Placeholders: `ScrollTimeline` and `ViewTimeline` (`Animation/`, FIXME: web animations) keep the name, axis and insets they're created with, which is all the computed style uses; the seam gains `RenderTheme` with the focus ring width and offset.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `rendering/style/Style{AppleColorFilter,BackdropFilter,Background,BorderImage,Box,Filter,FlexibleBox,Font,Grid,GridItem,Inherited,MaskBorder,MiscNonInherited,MultiCol,NonInherited,RareInherited,RareNonInherited,Surround,Transform,VisitedLinkColor}Data.h`, `.cpp` | `port` | `Rendering/Style/...`, `src/...` | on | tested (initial values, copies, copy-on-write, fast path inheritance). |
| `rendering/style/BorderData.h`, `.cpp` | `port` | `Rendering/Style/BorderData.hpp`, `src/...` | on | tested. |
| `rendering/style/SVGRenderStyle.h`, `.cpp`, `SVGRenderStyleDefs.h`, `.cpp` | `port` | `Rendering/Style/...`, `src/...` | on | tested. |
| `rendering/style/StyleMarqueeData.h`, `.cpp`, `StyleDeprecatedFlexibleBoxData.h`, `.cpp` | `drop` | | | Non-standard. |
| `style/computed/StyleComputedStyleBase*.h`, `.cpp`, `StyleComputedStyle*.h`, `.cpp`, `StyleComputedStyleProperties+*Custom*.h`, `.cpp` | `port` | `Style/Computed/ComputedStyle*.hpp`, `src/Style/Computed/...` | on | Initial values tested; the rest with `RenderStyle` (f3f4), which owns the constructors. |
| generated `StyleComputedStyleProperties.h`, `+GettersInlines.h`, `+SettersInlines.h`, `+InitialInlines.h` | `port` | generated `Krystal.Booey/Style/Computed/ComputedStyleProperties*.hpp` | on | Every accessor compiles in `ComputedStyle.cpp`. |
| `style/StyleNameScope.h`, `.cpp` | `port` | `Style/NameScope.hpp`, `src/...` | on | tested. |
| `style/values/scroll-animations/StyleScrollTimelines.h`, `StyleViewTimelines.h` | `port` | `Style/Values/ScrollAnimations/...` | on | |

**Bugs fixed:**
- The generator's `name_for_methods` stripped `Krystal` instead of `Webkit`, so the render style accessors of the non-standard properties kept the prefix (`WebkitBorderHorizontalSpacing()` instead of upstream's `borderHorizontalSpacing()`) and didn't match the data group members. Fixed for the render style family.
- The generator's enum spelling overrides (`Nonzero` → `NonZero`, `Nowrap` → `NoWrap`, `Ltr` → `LTR`) were looked up by the qualified type name (`boo::WindRule`) and never matched, emitting enumerators that don't exist.
- The generator emitted `KRYS_EXPORT` (which doesn't exist) for upstream's exported accessors, and `CSS::Keyword::X { }` for keyword initial values (`css::keywords::X {}`).
- Upstream declares `ComputedStyle::copyPseudoElementsFrom` but never defines it (only `RenderStyle` does); the declaration is dropped.

**Added (upstream API):** `std::hash<css::CustomIdentifier>` (for `CommaSeparatedListHashSet<CustomIdentifier>`); `RenderTheme::PlatformFocusRingWidth`/`Offset` on the seam.

**Differences:**
- Krystal's `DataRef` has no `replace()`; the merging constructor uses `std::exchange`.
- `ComputedStyleBase::SetFontDescription` doesn't update the new font cascade with the old one's font selector (FIXME: Krystal's `FontCascade` has none).
- `CompareEqual` (the setter macros' comparison) converts an integer bit-field to `bool` before comparing it with a `bool`.
- The pseudo style cache is keyed through `DefaultHash<PseudoElementIdentifier>`.

Tests: `tests/Rendering/Style/StyleDataGroups.cpp` (data group initial values against `ComputedStyle`'s, copies and equality, copy-on-write through nested groups, fast path inheritance, `BorderData` widths, views, radii and equality, `SVGRenderStyle` defaults, copies and inheritance), `tests/Style/Computed/ComputedStyleInitial.cpp` (generated initial values, including the enum spelling overrides and name scopes), `tests/Style/NameScope.cpp` (alternatives, equality, conversion): 9 new cases. `ComputedStyle`'s getters, setters, inheritance and equality are tested through `RenderStyle` (f3f4). Full Booey suite: 1968 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3f1: storage leaves and custom properties (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`, `f3-computed.md`). The `rendering/style` value classes that don't need `ComputedStyle`'s initial values (`BorderValue.h`, `.cpp`, `OutlineValue.h`, `.cpp`, `CollapsedBorderValue.h`, `CounterDirectives.h`, `PositionTryOrder.h`, `.cpp`) and the custom property storage (`style/StyleCustomProperty.h`, `.cpp`, `rendering/style/StyleCustomPropertyData.h`, `.cpp`). Translated by `conv.py` and rewritten by hand.

**Decisions (2026-10-14):**
- f3f split in four: f3f1 (this layer); f3f2, the `GenerateStyleComputedStyleProperties` and `GenerateRenderStyleProperties` generators; f3f3, the data groups (`Style*Data`, `BorderData`, `SVGRenderStyle` and `SVGRenderStyleDefs`) with `style/computed`, since the data groups' constructors call the generated `ComputedStyle::initial*()` functions and `ComputedStyle` holds the data groups; f3f4, `RenderStyle`, replacing the seam's.
- `rendering/style/StyleTextEdge.h`, `.cpp` are dropped: dead upstream (`TextEdgeType` exists nowhere, and every `StyleTextEdge.h` include resolves to `style/values/inline`'s, ported in f3b2). `AutosizeStatus` is dropped too (`ENABLE_TEXT_AUTOSIZING`, which the generator leaves off).
- Names: `BorderValue`'s members are `Color`/`Width`/`Style`, `OutlineValue`'s `OutlineColor`/`OutlineWidth`/`OutlineOffset`/`OutlineStyle` (the last qualified as `boo::OutlineStyle` inside the struct), `CounterDirectives`' `ResetValue`/`IncrementValue`/`SetValue`, `CounterDirectiveMap::Map`, `CustomProperty::ValueList`'s `Values`/`Separator`; `CustomProperty::value()` → `GetValue()`, `name()` → `GetName()`, `propertyValueSerialization` → `PropertyValueSerialisation`; `CustomPropertyData::get`/`size` → `Get`/`Size`, and its entries are the map's `value_type` (`Entry`, `first`/`second`). The bitfields are `uint8`.
- `CustomProperty`'s `String` alternative is `css::CSSOMString`, and its variable data is `css::VariableData`; its tokens are `css::Token`s. Tokenizing a typed value serialises it against the seam's new `RenderStyle::DefaultStyleSingleton()`.
- `CustomPropertyData` keeps upstream's parent-linked copy-on-write (`ShareRefPtr` to the source); the debug-only `_hasChildren` check is under `KRYS_DEBUG`. `dumpDifferences` (logging) isn't ported.
- Mappings: `Style::PositionTryOrder` now has its `CSSPrimitiveValueMappings` entry; none remain deferred.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `rendering/style/BorderValue.h`, `.cpp`, `OutlineValue.h`, `.cpp`, `CollapsedBorderValue.h`, `CounterDirectives.h`, `PositionTryOrder.h`, `.cpp` | `port` | `Rendering/Style/...`, `src/...` | on | tested. |
| `style/StyleCustomProperty.h`, `.cpp` | `port` | `Style/CustomProperty.hpp`, `src/...` | on | tested. |
| `rendering/style/StyleCustomPropertyData.h`, `.cpp` | `port` | `Rendering/Style/StyleCustomPropertyData.hpp`, `src/...` | on | tested. |
| `rendering/style/StyleTextEdge.h`, `.cpp` | `drop` | | | Dead upstream. |
| `rendering/style/AutosizeStatus.h`, `.cpp` | `drop` | | | `ENABLE_TEXT_AUTOSIZING`. |

**Bugs fixed:**
- `CSS/Values/ValueSeparator.hpp` had no `#pragma once`, so including it twice in one translation unit redefined `css::ValueSeparator` (first hit by `CustomProperty.cpp`).

**Added (upstream API):** `RenderStyle::DefaultStyleSingleton()` on the seam; the `Style::PositionTryOrder` mapping.

**Differences:** none beyond the decisions.

Tests: `tests/Rendering/Style/BorderValues.cpp` (border visibility, transparency and hidden styles, collapsed borders and their adjusted widths, outlines and `ToBorderStyle`, counter directive maps, position-try order axes), `tests/Style/CustomProperty.cpp` (guaranteed-invalid, token and typed custom properties with their serialisation, tokens, CSS values and equality; custom property data: copy-on-write, overriding, iteration, key lookup by index, equality across parents): 6 new cases. Full Booey suite: 1959 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3e4: masking, motion and SVG values, with path operations (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/masking` (except `StyleMaskMode`, done in f3e2), `style/values/motion` and `style/values/svg`, `style/values/shapes/StylePathOperationWrappers.h`, `.cpp` (deferred from f3e3), and the path operations they hold: `rendering/PathOperation.h`, `.cpp`, `rendering/MotionPath.h`, `.cpp`, `rendering/TransformOperationData.h`, `.cpp`. Translated by the f3b1 script and `conv.py` and reviewed and fixed by hand; `PathOperation`, `MotionPath`, `TransformOperationData`, `MaskBorder` and `MaskLayer` were rewritten.

**Decisions (2026-10-14):**
- `PathOperation` follows `FilterOperation`: a non-virtual `operator==` over a virtual `Equals`; `Clone`, `CanBlend`, `Blend` and `GetPath` are `noexcept`; type traits via `KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION`. `ShapePathOperation::windRule()` → `GetWindRule()`. The declared-but-undefined `RayPathOperation::lengthForPath`/`lengthForContainPath` are dropped.
- `ReferencePathOperation` takes a raw `const svg::SVGElement *` (upstream `RefPtr<SVGElement>`), unused for now: taking the path from an SVG geometry element (`pathFromGraphicsElement`) waits for SVG geometry (FIXME). The `clip-path`/`offset-path` conversion computes the fragment as `FilterReference` does and passes no element (FIXMEs for `SVGURIReference::fragmentIdentifierFromIRIString`/`targetElementFromIRIString`), so reference paths have no path yet.
- `MotionPath` ports the geometry (`applyMotionPathTransform`, `computePathFor{Box,Shape,Ray}`, `needsUpdateAfterContainingBlockLayout`); `motionPathDataForRenderer` and its helpers need the renderer (FIXME). `TransformOperationData` has only the data constructor (its renderer constructor is a FIXME); its members are `BoundingBox`, `MotionPathData` (type qualified as `boo::MotionPathData`) and `IsSVGRenderer`; `MotionPathData`'s are `ContainingBlockBoundingRect`, `OffsetFromContainingBlock`, `UsedStartingPosition`.
- `MaskBorder` and the `MaskBorder*` components are generated from the reviewed `BorderImage*` ports with the upstream differences applied (initial slice `0`, width `auto`, no legacy `-webkit-border-image` flag, the shared CSS slice/width value classes); like `BorderImage`, `MaskBorder`'s members are `Source`/`Slice`/`Width`/`Outset`/`Repeat`, the alias accessors are dropped, and the default constructor uses literal initial values (FIXME: `ComputedStyle`'s, with f3f).
- `MaskLayer` is generated from `BackgroundLayer` the same way (`KRYS_FOR_EACH_MASK_LAYER_*`, `Get` getters; `composite`/`maskMode` stored, `attachment`/`blendMode` constant; `CompositeForPainting` ignores the last layer's operator).
- The path holders (`RayPath`, `ReferencePath`, `BasicShapePath`, `BoxPath`, `ClipPath`, `OffsetPath`) keep their operation in a private `_operation`; `SupportRayPathOperation` is an `enum class`.
- `OffsetAnchor`/`OffsetPosition` conversions return the literal initial values (`auto`, `normal`) where upstream asks `ComputedStyle` (FIXME-free: those are the initial values).
- Seam: `RenderStyle` gains `DisallowsFastPathInheritance`/`SetDisallowsFastPathInheritance` (set by `SVGPaint` for `currentcolor`).
- Names: `Ray`'s members are `Angle`/`Size`/`Contain`/`Position` (as the CSS side), `SVGPaint::URLNone`'s `Url`/`None`, `URLColor`'s `Url`/`Color`, `SVGStrokeDasharrayValue::Value`, `SVGStrokeDashoffset::Value`, `ClipRect::Value`; `Clip`'s optional rect is a private `_value`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `rendering/PathOperation.h`, `.cpp` | `port` | `Rendering/PathOperation.hpp`, `src/...` | on | tested. Reference paths need SVG geometry. |
| `rendering/MotionPath.h`, `.cpp` | `port` (partial) | `Rendering/MotionPath.hpp`, `src/...` | on | tested. `motionPathDataForRenderer` with the renderer. |
| `rendering/TransformOperationData.h`, `.cpp` | `port` (partial) | `Rendering/TransformOperationData.hpp` | on | The renderer constructor with the renderer. |
| `style/values/shapes/StylePathOperationWrappers.h`, `.cpp` | `port` | `Style/Values/Shapes/PathOperationWrappers.hpp`, `src/...` | on | tested. |
| `style/values/masking/*` | `port` | `Style/Values/Masking/...`, `src/...` | on | tested. See decisions. |
| `style/values/motion/*` | `port` | `Style/Values/Motion/...`, `src/...` | on | tested. |
| `style/values/svg/*` | `port` | `Style/Values/SVG/...`, `src/...` | on | tested. |

**Bugs fixed:**
- `ClipPath`'s `RefPtr<PathOperation>` constructors asserted `isValid(operation)` on the (moved-from) parameter rather than the member, so a ray operation was never caught; the assertion now checks the member.
- `OffsetPosition::isNormal()` tested for `auto`, so `normal` was never reported (and `auto` was reported as both); it now tests for `normal`.

**Added (upstream API):** `RenderStyle::DisallowsFastPathInheritance()` on the seam.

**Differences:** see decisions (no reference paths without SVG geometry, no motion path data without the renderer, literal initial values in `MaskBorder`). Behaviours kept from upstream and pinned by tests: `ray()` omits the default `closest-side`; `stroke-dasharray` blending repeats both lists to their least common multiple only when neither length divides the other.

Tests: `tests/Style/Values/Masking/Masking.cpp` (`clip` with blending, `clip-path` boxes, shapes, `path()`, references, equality and blending, `mask-border` and its longhands, mask layers), `tests/Style/Values/Motion/Motion.cpp` (`offset-path` rays, shapes, boxes and references with their motion paths and blending, path operations, the motion path transform, `offset-anchor`, `offset-position`, `offset-rotate`, `offset-distance`), `tests/Style/Values/SVG/SVG.cpp` (paint with blending, dashes with blending, baseline shift, glyph orientation, paint order, `d`): 12 new cases. The fixture enables `cssDPropertyEnabled`. Full Booey suite: 1953 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3e3: paths and shapes (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). The path data model (`platform/graphics/Path.h`, `.cpp`, `PathElement.h`, `PathImpl.h`, `.cpp`, `PathSegment.h`, `.cpp`, `PathSegmentData.h`, `.cpp`, `PathStream.h`, `.cpp`, `PathTraversalState.h`, `.cpp`, `RotationDirection.h`), the SVG path consumers that build and blend paths (`svg/SVGPathBuilder.h`, `.cpp`, `SVGPathBlender.h`, `.cpp`, `SVGPathAbsoluteConverter.h`, `.cpp`, and the rest of `SVGPathUtilities.h`, `.cpp`), WTF's `DataRef.h` and `TinyLRUCache.h`, and every file of `style/values/shapes` except `StylePathOperationWrappers.h`, `.cpp`. Translated by the f3b1 script and `conv.py` and reviewed and fixed by hand.

**Decisions (2026-10-14):**
- f3e3 is paths and shapes only; `masking`, `motion` and `svg` move to f3e4, with `rendering/PathOperation.h` and `StylePathOperationWrappers` (which need it).
- `Path` is the real data model rather than a stand-in, minus the platform: `PathImpl` is abstract and `PathStream` is its only implementation (no `PlatformPath`, no `PathCG`/`PathCairo`/`PathSkia`). Without a platform path, `AddPath`, `ApplyElements` and `Transform` skip what only the platform could handle (arcs, ellipses and continuous rounded rects that can't produce elements or be transformed; FIXMEs). Not ported (FIXME on `Path`): hit testing (`contains`, `strokeContains`), `strokeBoundingRect`, and `platformPath`; `ensureImplForTesting` is dropped (no test-only hooks). `PathUtilities` (rect unions, path outlines) is renderer code and waits for it.
- Names: the segment structs' members are PascalCase (`Point`, `ControlPoint`, `EndPoint`, `Radius`, `Center`, `StartAngle`, `RoundingStrategy`, ...), `PathElement` has `ElementType`/`Points`, `PathSegment::data()` → `GetData()`, the `canApplyElements`/`canTransform` constants are `CanApplyElements`/`CanTransform`, `clear` → `Clear`.
- `FloatRoundedRect::path()` → `GetPath()` (completing f3e2's FIXME).
- `SVGPathBlender`: `FloatBlendMode` is an `enum class` (`Horizontal`, `Vertical`); the sources are references and the optional consumer a raw pointer (upstream weak references). `BuildPathFromByteStream` doesn't cache the built path on the stream (FIXME).
- Core: `DataRef` (copy-on-write `Ref`: `Get()`, `Ptr()`, `Access()`) and `TinyLRUCache` (policy methods `IsKeyNull`, `CreateValueForNullKey`, `CreateValueForKey`, `CreateKeyForStorage`) in `Krystal.Core/Types`; `IsInRange` in `Core/MathExtras.hpp`.
- Shapes: public members PascalCase throughout; where a member would hide its type, the type gets a `*Type` alias (`AnchorType`, `VerticesType`, `CommandsType`, `InsetsType`, `DataType`). `ShapeOutside`'s members are `ShapeAndShapeBox::Shape`, `Image::Value` and `Value::Contents`; the `ShapeFunction.cpp` helpers named after types are `MakePosition`, `MakeCoordinatePair`, `MakeAbsoluteControlPoint`, `MakeRelativeControlPoint`. The invokers are `ComputePath` and `ComputeWindRule`. `css::Rect` is spelled `css::RectShape` on the CSS side.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/Path.h`, `.cpp`, `PathElement.h`, `PathImpl.h`, `.cpp`, `PathSegment.h`, `.cpp`, `PathSegmentData.h`, `.cpp`, `PathStream.h`, `.cpp`, `PathTraversalState.h`, `.cpp`, `RotationDirection.h` | `port` | `Graphics/...`, `src/...` | on | tested. See decisions. |
| `platform/graphics/RotationDirection.cpp` | `drop` | | | Logging only. |
| `platform/graphics/PathUtilities.h`, `.cpp` | `defer` | | | Renderer. |
| `svg/SVGPathBuilder.h`, `.cpp`, `SVGPathBlender.h`, `.cpp`, `SVGPathAbsoluteConverter.h`, `.cpp`, `SVGPathUtilities.h`, `.cpp` (the rest) | `port` | `SVG/Internals/...`, `src/...` | on | tested. |
| `wtf/DataRef.h`, `wtf/TinyLRUCache.h` | `port` | `Krystal.Core/Types/...` | on | tested (Krystal.Core.Tests). |
| `style/values/shapes/*` except `StylePathOperationWrappers.h`, `.cpp` | `port` | `Style/Values/Shapes/...`, `src/...` | on | tested. |
| `style/values/shapes/StylePathOperationWrappers.h`, `.cpp` | `defer` | | | f3e4, with `PathOperation`. |

**Bugs fixed:** none found.

**Added (upstream API):** none.

**Differences:** see decisions (no platform path, so some arcs and rounded rects are skipped by `ApplyElements`/`Transform`/`AddPath`; no built-path cache). `shape-outside` rejects `path()`, as upstream.

Tests: `Krystal.Core.Tests/tests/Types/DataRef.cpp` (`DataRef` copy-on-write, `TinyLRUCache` eviction, `RefCountedFixedVector`, `ArePointingToEqualData`), `tests/Graphics/Path.cpp` (lines, polylines and polygons, curve bounds, rects, ellipses, rounded rects, arcs, length and point-at-length, elements, translation, transforms, copies, `AddPath`), `tests/SVG/Internals/SVGPath.cpp` (building paths from strings and streams, blending, absolute conversion), `tests/Style/Values/Shapes/Shapes.cpp` (`shape-outside` boxes, `circle()`, `ellipse()`, `inset()`, `rect()`, `xywh()`, `polygon()`, `shape()`, computed paths, wind rules, blending): 4 new cases. Full Booey suite: 1941 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3e2: backgrounds, borders, text decoration and the non-standard values (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/backgrounds` (12 headers, 10 sources), `style/values/borders` (5 and 3) and `style/values/text-decoration` (7 and 7), the `non-standard` types behind properties in `css-properties.json` (`StyleWebKitBorderSpacing.h`, `StyleWebKitLineBoxContain.h`, `StyleWebKitLocale.h`, `.cpp`), `style/values/masking/StyleMaskMode.h`, `.cpp` (moved here from f3e3: `BackgroundLayer` needs it), and the rounded rects deferred from f3a (`platform/graphics/FloatRoundedRect.h`, `.cpp`, `LayoutRoundedRect.h`, `.cpp`). Translated by the f3b1 script and `conv.py` and reviewed and fixed by hand (`BackgroundLayer`, `BorderImage` and the rounded-rect headers rewritten).

**Decisions (2026-10-14):**
- Not ported from `non-standard`: `StyleWebKitBoxReflect`, `StyleWebKitLineClamp`, `StyleWebKitMarquee{Increment,Repetition,Speed}`, `StyleWebKitTextStrokeWidth` (their properties aren't in `css-properties.json`), and `StyleWebKitOverflowScrolling`, `StyleWebKitTouchCallout` (their properties are behind `ENABLE_WEBKIT_OVERFLOW_SCROLLING_CSS_PROPERTY` and `ENABLE_WEBKIT_TOUCH_CALLOUT_CSS_PROPERTY`, which the generator leaves off, as planned). `ToValueIdForWebkitMaskSourceType` is dropped too: `-webkit-mask-source-type` isn't a property here.
- Upstream `.cpp` files holding only logging (`operator<<(TextStream&, ...)`) aren't ported; the `TextStream` overloads are dropped throughout, as before.
- `FloatRoundedRect::path()` comes with the `Path` stand-in in f3e3 (FIXME). `FloatRoundedRect` and `LayoutRoundedRect` are otherwise complete; `move` → `Move`, `radii()` → `GetRadii()`.
- `BackgroundLayer`: as `Animation` (f3d1), the coordinated-value-list macros are `KRYS_FOR_EACH_BACKGROUND_LAYER_*` and the getters take a `Get` prefix (the accessor macros call `Get<Name>()`; several names would hide types). The generated `FillAttachment()`/`FillRepeat()` setters hide those enum types inside the class, so the class qualifies them (`boo::FillAttachment`). `hasOpaqueImage` and the `FillLayers` helpers `imagesAreLoaded`/`hasHDRContent` need the renderer or the resource loader (FIXMEs).
- `BorderImage`'s members are `Source`/`Slice`/`Width`/`Outset`/`Repeat` (as `css::BorderImageComponents`); upstream's alias accessors (`source()`, ... for generic code shared with `MaskBorder`) are dropped, since the members already have those names. Its default constructor uses the properties' initial values directly (FIXME: `ComputedStyle`'s initial values, with the storage in f3f).
- `LineWidth` reads the device scale factor from the seam's `RenderStyle::DeviceScaleFactor()`, as `LineHeight` does (upstream `document().deviceScaleFactor()`).
- `TextDecorationLine`'s flags are an `EnumSet` (upstream `OptionSet`): `Flag` is ordinal (`Underline` = 0, ...) and the `*Bit` constants are `1 << value`, so the packed byte is unchanged.
- `TextEmphasisStyle`'s marks are local UTF-8 constants (`Bullet`, `BlackCircle`, `SesameDot`, ...; WTF's `CharacterNames.h` has no counterpart in Krystal.Core), held in function-local statics.
- `shadowStyle()` → `GetShadowStyle()` (it would hide the `ShadowStyle` enum).
- Names: public members PascalCase throughout (`BoxShadow`/`TextShadow`'s `Color`, `Location`, `Blur`, `Spread`, `Inset`, `IsWebkitBoxShadow`, as the CSS side; `BorderImageSlice::Fill`, `BorderImageWidth::LegacyWebkitBorderImage`, the edge lists' `Values`, `RepeatStyle::Values`, `LineWidth::Value`, `CornerShapeValue::Superellipse`, `TextEmphasisStyle::Value` and `Shape::Fill`/`Mark`, `TextDecorationLine`'s locals).
- Mappings: `Style::MaskMode`, `Style::TextDecorationLine::Flag`, `Style::TextEmphasisPositionValue`, `Style::TextUnderlinePositionValue` and `Style::WebkitLineBoxContainValue` now have their `CSSPrimitiveValueMappings` entries; only `Style::PositionTryOrder` remains deferred.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/FloatRoundedRect.h`, `.cpp`, `LayoutRoundedRect.h`, `.cpp` | `port` | `Graphics/...`, `src/...` | on | tested. `path()` with f3e3. |
| `style/values/backgrounds/*` | `port` | `Style/Values/Backgrounds/...`, `src/...` | on | tested. See decisions. |
| `style/values/borders/*` | `port` | `Style/Values/Borders/...`, `src/...` | on | tested. |
| `style/values/text-decoration/*` | `port` | `Style/Values/TextDecoration/...`, `src/...` | on | tested. |
| `style/values/non-standard/StyleWebKitBorderSpacing.h`, `StyleWebKitLineBoxContain.h`, `StyleWebKitLocale.h`, `.cpp` | `port` | `Style/Values/NonStandard/...`, `src/...` | on | tested. |
| `style/values/non-standard/StyleWebKitBoxReflect.h`, `.cpp`, `StyleWebKitLineClamp.h`, `.cpp`, `StyleWebKitMarquee*.h`, `StyleWebKitTextStrokeWidth.h`, `.cpp`, `StyleWebKitOverflowScrolling.h`, `.cpp`, `StyleWebKitTouchCallout.h`, `.cpp` | `drop` | | | See decisions. |
| `style/values/masking/StyleMaskMode.h`, `.cpp` | `port` | `Style/Values/Masking/...`, `src/...` | on | Moved from f3e3. |

**Bugs fixed:** none found.

**Added (upstream API):** the mappings above.

**Differences:** see decisions (initial values inlined in `BorderImage`, the emphasis-mark constants, `EnumSet` flags). Behaviours kept from upstream and pinned by tests: `background-size` serialises a trailing `auto` (`10px auto`, an upstream FIXME), and `-webkit-locale` serialises as a custom ident (another upstream FIXME).

Tests: `tests/Graphics/RoundedRect.cpp` (radii scaling, expansion and constraints, corner geometry, intercepts, regions, layout rects, transposition, snapping), `tests/Style/Values/Backgrounds/Backgrounds.cpp` (`background-size`, `background-repeat`, layers and their clip max, the border-image longhands and shorthand value, blending, `LineWidth`), `tests/Style/Values/Borders/Borders.cpp` (`border-radius`, `box-shadow` with extents and blending, `corner-shape`), `tests/Style/Values/TextDecoration/TextDecoration.cpp` (`TextDecorationLine` packing, the text-decoration, underline and emphasis properties, `text-shadow`, the non-standard values): 11 cases. Full Booey suite: 1938 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3e1: style images, `images`, `content` and the cursor (2026-10-14)

Graphs: `tools/port/out/f3e-styleimage.md` (from `rendering/style/StyleImage.h`, following `rendering/style/` and `platform/graphics/`: 45 files, mostly the colour and geometry types ported earlier). Every file of the `StyleImage` hierarchy (`rendering/style/Style{,Cached,Generated,Multi,Invalid,Cursor,Crossfade,Filter,Gradient,Named,Canvas,Paint}Image.h`, `.cpp`, `StyleImageSet.h`, `.cpp`), `style/values/images` (5 headers, 4 sources), `style/values/content` (2 and 2) and `style/values/ui/StyleCursor.h`, `.cpp` (moved here from f3b2), with the platform data they need (`Gradient.h`, `.cpp`, `GradientColorStop.h`, `GradientColorStops.h`, `ImageOrientation.h`). Translated by the f3b1 script (and a new single-file `conv.py`) and reviewed and fixed by hand; the image classes were rewritten.

**Decisions (2026-10-14):**
- f3e split in three: f3e1 (this layer); f3e2, `backgrounds`, `borders` (with the rounded rects), `text-decoration` and `non-standard`; f3e3, the `Path` stand-in, `shapes`, `masking`, `motion` and `svg` (the groups that need `Path`).
- The `StyleImage` hierarchy is ported without loading, clients or drawing, as planned: values, equality, `ComputedStyleValue`, `Data`, `IsPending`, `ErrorOccurred`, `UsesDataProtocol`, `HasImage`, `Url`, the relative-size queries, `ImageScaleFactor`, `ImageHasNaturalDimensions`, `SelectedImage`, `GetCachedImage` (always null) and the type predicates. Left out, with a FIXME on the base class: `load`, `isLoaded`, `addClient`/`removeClient`/`hasClient`, `imageSize`, `computeIntrinsicDimensions`, `setContainerContextForRenderer`, `image`, `canRender`, `knownToBeOpaque`, and with them each subclass's `fixedSize`, `didAddClient`/`didRemoveClient`, the generated-image cache (`cachedImageForSize`, `CachedGeneratedImage`), the `CachedImageClient` bases of the cross-fade and filter images, the canvas image's `CanvasObserver` and element lookup, and the cached image's SVG resource lookups. Consequences: cached images stay pending; multi-images (sets, cursors) select nothing until they load (`SelectBestFitImage` is still callable); `StyleCrossfadeImage::Blend` returns null (it needs both inputs loaded), so `ImageWrapper` blending falls back to the destination image.
- Equality: as `FilterOperation`, a non-virtual `operator==` over a virtual `Equals`; the typed `equals(const X&)` overloads are `Equals`. `StyleImage` is `RefCounted` and `CanMakeWeakPtr` (upstream `RefCountedAndCanMakeWeakPtr`).
- `StyleImageSet::bestImageForScaleFactor` asks a local `IsSupportedImageMIMEType` (FIXME): until there's an image decoder, the formats every decoder handles (PNG, JPEG, GIF, BMP, WebP, AVIF, SVG, ICO). `updateDeviceScaleFactor` uses 1 (documents have no page).
- `ImageWithScale::MimeType` is a `Maybe<CSSOMString>` (Krystal strings have no null state; the CSS option value already uses `Maybe`); `Content::Data::AltText` likewise (`Markable<String>`).
- `GradientValue`, `CanvasValue` and `NamedImageValue` get their cached style image and an out-of-line destructor (the cache holds an incomplete `StyleImage`; `GradientValue`'s constructor also moved out of line). `GradientValue::CreateStyleImage` ports upstream's `StyleImageIsUncacheable` traversal.
- `Gradient` (platform) keeps its data, stops, `IsZeroSize`, `AdjustParametersForTiledDrawing` and `AddColorStop`; drawing, the cached hash and the rendering-resource observers are a FIXME. Its data members are PascalCase (`Point0`, `StartRadius`, ...); `data()` → `GetData()`, `colorInterpolationMethod()` → `GetColorInterpolationMethod()`. `GradientColorStop`'s members are `Offset`/`Color`.
- `ImageOrientation` (platform) is ported in full; `orientation()` → `GetOrientation()`.
- `Style::Gradient` mirrors `css::Gradient`'s names: stops' `Color`/`Position` (with `ColorType`/`PositionType` aliases), `ColorInterpolationMethod`, `GradientLine`/`GradientLineType`, `GradientBox`/`GradientBoxType`, `Size`/`SizeType`, `Stops`, `Angle`, `First`/`FirstRadius`/`Second`/`SecondRadius`. Repeating gradients count their generated stops in plain `size_t` (Krystal has no `CheckedSize`; the counts grow by one per loop iteration) and keep the 65536-stop cap.
- `Content`: the single-member wrappers' members are `Value`; `Counter`'s are `Identifier`/`Separator`/`Style`; `Data`'s are `List`/`AltText`, with the list type `ListType`. `attr()` reads the attribute with `GetAttributeNS(null, localName)` (upstream's null-namespace `QualifiedName`). `Cursor`'s members are `Images`/`Predefined` (the alias is `ImagesType`), `CursorImage`'s `Image`/`HotSpot`.
- Core: `ArePointingToEqualData` (`Utils/PointerComparison.hpp`, WTF's `PointerComparison.h`) and `RefCountedFixedVector` (`Types/RefCountedFixedVector.hpp`, a ref-counted `FixedVector` standing in for WTF's trailing-array one). CSS aggregates: `SpaceSeparatedRefCountedFixedVector`/`CommaSeparatedRefCountedFixedVector` (used by `quotes` and `cursor`).
- Seam: `BuilderState::CreateStyleImage` (upstream's dispatch over the image values, completing the deferred `createStyleImage`); `RenderStyle` gains `AppleColorFilter` storage (`AppleColorFilter`, `SetAppleColorFilter`, `HasAppleColorFilter`), `ColorByApplyingColorFilter`, `ColorWithColorFilter` and `HasAttrContent`/`SetHasAttrContent`.
- Mappings: `Style::ImageOrientation` now has its `CSSPrimitiveValueMappings` entry.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `rendering/style/StyleImage.h`, `StyleGeneratedImage.h`, `.cpp`, `StyleMultiImage.h`, `.cpp` | `port` (partial) | `Rendering/Style/...`, `src/...` | on | tested. See decisions (no loading, clients or drawing). |
| `rendering/style/StyleCachedImage.h`, `.cpp`, `StyleImageSet.h`, `.cpp`, `StyleCursorImage.h`, `.cpp`, `StyleInvalidImage.h`, `.cpp` | `port` (partial) | `Rendering/Style/...`, `src/...` | on | tested. |
| `rendering/style/StyleGradientImage.h`, `.cpp`, `StyleCrossfadeImage.h`, `.cpp`, `StyleFilterImage.h`, `.cpp`, `StyleNamedImage.h`, `.cpp`, `StyleCanvasImage.h`, `.cpp`, `StylePaintImage.h`, `.cpp` | `port` (partial) | `Rendering/Style/...`, `src/...` | on | tested (but `paint()`, behind `cssPaintingAPIEnabled`). |
| `platform/graphics/Gradient.h`, `.cpp`, `GradientColorStop.h`, `GradientColorStops.h` | `port` (partial) | `Graphics/...`, `src/...` | on | tested. No drawing. |
| `platform/graphics/ImageOrientation.h` | `port` | `Graphics/ImageOrientation.hpp` | on | tested. |
| `style/values/images/*` | `port` | `Style/Values/Images/...`, `src/...` | on | tested. |
| `style/values/content/*` | `port` | `Style/Values/Content/...`, `src/...` | on | tested. |
| `style/values/ui/StyleCursor.h`, `.cpp` | `port` | `Style/Values/UI/Cursor.hpp`, `src/...` | on | tested. |
| `css/CSS{Image,ImageSet,CursorImage,NamedImage,Canvas,Crossfade,FilterImage,Gradient,PaintImage}Value.cpp` (`createStyleImage`) | `port` | `src/CSS/Values/...` | on | The deferred `CreateStyleImage` definitions. |
| `wtf/PointerComparison.h`, `wtf/RefCountedFixedVector.h` | `port` | `Krystal.Core/Utils/PointerComparison.hpp`, `Krystal.Core/Types/RefCountedFixedVector.hpp` | on | See decisions. |

**Bugs fixed:** none found.

**Added (upstream API):** the seam members, the two list aggregates and the Core helpers above.

**Differences:** see decisions (partial image classes, the MIME-type stand-in, `Maybe` for nullable strings, unchecked stop counts).

Tests: `tests/Graphics/Gradient.cpp` (stops, zero size, tiled drawing, `ImageOrientation`), `tests/Style/Values/Images/Images.cpp` (every image function through `list-style-image`, image sets and their selection, the style-image cache, blending, platform gradients: end points, stop positions, repetition, `currentcolor`; `image-orientation`), `tests/Style/Values/Content/Content.cpp` (`content` with `attr()`, `quotes`, `cursor`): 10 cases. Full Booey suite: 1927 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3d3: filters and anchor positioning (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/filter-effects` (15 headers, 14 sources) and `style/values/anchor-position` (11 headers, 7 sources), the platform filter operations (`platform/graphics/filters/FilterOperation.h`, `.cpp`, `FilterOperations.h`, `.cpp`) and the two style-side operations (`rendering/style/ReferenceFilterOperation.h`, `.cpp`, `DropShadowFilterOperationWithStyleColor.h`, `.cpp`), translated by the f3b1 script and reviewed and fixed by hand (`FilterOperations` and `PositionTryFallback` rewritten). The `Path` stand-in planned for this layer moved to f3e: no f3d group needs it (`shapes`, `motion` and `masking` do).

**Decisions (2026-10-14):**
- `FilterOperation`: as `TransformOperation` (f3d2), a non-virtual `operator==` over a virtual `Equals`; the `dump(TextStream&)` overrides aren't ported. The `blend` overrides that return `this` return `ShareRefPtr(this)`.
- `FEGaussianBlur` and `FEDropShadow` are partial: only the static outset helpers `FilterOperations::outsets()` uses (`calculateUnscaledKernelSize`, `calculateOutsetSize`, `calculateOutsets`). The effects themselves come with the graphics backend.
- `ReferenceFilterOperation` holds its `Style::URL` and the fragment as a `StringAtom`; the `SVGFilterElement` lookup and the cached resource (`CachedSVGDocumentReference`) aren't ported (FIXME, with the resource loader).
- `css::FilterOperationType` (a Krystal stand-in from layer 6) is removed: the filter function descriptors now use `FilterOperation::Type`, as upstream does.
- `-apple-color-filter` stays: it's in `css-properties.json` (it isn't a `-webkit-` alias), behind `colorFilterEnabled`.
- Names: `Style::Filter`/`AppleColorFilter`'s value wrappers (`FilterValue`, `AppleColorFilterValue`) have `Value`, `Get()`, `Platform()` and `operator->`; the `toCSSX(const XFilterOperation&, ...)` helpers are `ToCSSX`. `PositionTryFallback`'s members are `RuleAndTacticsFallback`/`PositionAreaFallback` (upstream's `ruleAndTactics`/`positionArea` would hide the nested types), with `RuleAndTactics::Rule`/`Tactics` and `PositionArea::Properties`; `Tactics` is a `css::ListOrNull`.
- Seam: `BuilderContext` gains `PositionTryFallback` (a `Maybe<BuilderPositionTryFallback>`: the fallback's properties and tactics), `BuilderState::GetPositionTryFallback()`, and `AnchorPositionEvaluator::ResolvePositionTryFallbackValueForSelfPosition` (upstream's logic in full). With them, `align-self`/`justify-self` apply the fallback tactics again (the FIXME from f3b2 is gone).
- Seam: `RenderStyle::UsedZoomForLength()` collapses upstream's `usedZoomForLength()` chain: the used zoom when zoom is applied at evaluation time, else 1.
- Mappings: `Style::PositionVisibilityValue` and `Style::PositionTryFallbackTactic` now have their `CSSPrimitiveValueMappings` entries (they were on the deferred list).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/filters/FilterOperation.h`, `.cpp`, `FilterOperations.h`, `.cpp` | `port` | `Graphics/Filters/...`, `src/...` | on | tested. See decisions. |
| `platform/graphics/filters/FEGaussianBlur.h`, `.cpp`, `FEDropShadow.h`, `.cpp` | `port` (partial) | `Graphics/Filters/...`, `src/...` | on | tested. Outset helpers only. |
| `rendering/style/ReferenceFilterOperation.h`, `.cpp` | `port` (partial) | `Rendering/Style/...`, `src/...` | on | tested (through `filter: url(...)`). No resource lookup. |
| `rendering/style/DropShadowFilterOperationWithStyleColor.h`, `.cpp` | `port` | `Rendering/Style/...`, `src/...` | on | tested (through `filter`). |
| `style/values/filter-effects/*` | `port` | `Style/Values/FilterEffects/...`, `src/...` | on | tested. |
| `style/values/anchor-position/*` | `port` | `Style/Values/AnchorPosition/...`, `src/...` | on | tested. See decisions (`PositionTryFallback`). |
| `css/CSS/Values/FilterEffects/FilterOperationType.hpp` (Krystal) | removed | | off | Replaced by `FilterOperation::Type`. |

**Bugs fixed:** none found.

**Added:** the seam members above; the two keyword mappings.

**Differences:** see decisions (partial effects, reference filters without a resource lookup).

Tests: `tests/Graphics/Filters/FilterOperations.cpp` (operations, colour transforms, blending, outsets, `FEGaussianBlur`, list interpolation), `tests/Style/Values/FilterEffects/Filters.cpp` (`filter`, every function, references, blending, `backdrop-filter`, `-apple-color-filter`), `tests/Style/Values/AnchorPosition/AnchorPosition.cpp` (`anchor-name`, `position-anchor`, `position-visibility`, `position-area`, `position-try-fallbacks`, and `align-self`/`justify-self` under each tactic): 9 cases. Full Booey suite: 1917 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3d2: transforms (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/transforms` and `style/values/transforms/functions` (17 headers, 16 sources) and the platform transform operations (`platform/graphics/transforms/*TransformOperation*`, `TransformOperations.h`, `.cpp`, `TransformOperationsSharedPrimitivesPrefix.h`: 11 headers, 10 sources), translated by the f3b1 script (extended to subgroups and to platform files) and reviewed and fixed by hand (`TransformOperations` and `TransformFunctionWrapper` rewritten). `TransformationMatrix` came with layer 6.

**Decisions (2026-10-14):**
- `TransformOperation` and `Style::TransformFunctionBase`: as `TimingFunction` (f3d1), the virtual `operator==` is a non-virtual `operator==` over a virtual `Equals`; the `dump(TextStream&)` overrides aren't ported. The subclasses' typed `operator==(const XTransformOperation&)` overloads are dropped (the base operator covers them).
- `Blending<TransformList>`/`Blending<Transform>`/`Blending<TransformFunction>` take a `BlendingContext` instead of an `Interpolation::Context` (there's no interpolation layer yet). They need two things from it, both FIXMEs: the box to resolve percentages against (the client renderer's border box; until then an empty box), and the precomputed prefix of shared primitives for `transform` (until then the prefix comes from the two lists).
- `TransformOperations` holds a `List<Ref<TransformOperation>>`; the IPC coder friend isn't ported. `TransformOperations::at()` → `At()`, `first()`/`last()` → `First()`/`Last()`.
- Names: `TransformFunctionSizeDependencies`' `IsWidthDependent`/`IsHeightDependent`; `TransformFunctionWrapper::value` → `Value`, `function()` → `GetFunction()`; `Rotate`/`Scale`/`Translate` keep their function in `_value` (private); `TransformOrigin`'s members are `X`/`Y`/`Z` (its nested `X`/`Y`/`Z`/`XY` aliases are dropped, the namespace-level `TransformOriginX`, ... remain) and `xy()` → `XY()`; `TransformList::isEmpty()` keeps the container-protocol name (`ListOrNone` calls it); `PerspectiveTransformFunction::perspective()` → `GetPerspective()`; `TransformList::apply`'s `start` is a `size_t`.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/transforms/TransformOperation.h`, `.cpp`, `IdentityTransformOperation.h`, `.cpp`, `Matrix3DTransformOperation.h`, `.cpp`, `MatrixTransformOperation.h`, `.cpp`, `PerspectiveTransformOperation.h`, `.cpp`, `RotateTransformOperation.h`, `.cpp`, `ScaleTransformOperation.h`, `.cpp`, `SkewTransformOperation.h`, `.cpp`, `TranslateTransformOperation.h`, `.cpp` | `port` | `Graphics/Transforms/...`, `src/...` | on | tested. See decisions. |
| `platform/graphics/transforms/TransformOperations.h`, `.cpp`, `TransformOperationsSharedPrimitivesPrefix.h` | `port` | `Graphics/Transforms/...`, `src/...` | on | tested. See decisions. |
| `style/values/transforms/functions/*` | `port` | `Style/Values/Transforms/Functions/...`, `src/...` | on | tested (through `transform`). See decisions. |
| `style/values/transforms/StyleTransform.h`, `.cpp`, `StyleTransformFunction.h`, `.cpp`, `StyleTransformList.h`, `.cpp` | `port` | `Style/Values/Transforms/...`, `src/...` | on | tested. See decisions (blending). |
| `style/values/transforms/StyleRotate.h`, `.cpp`, `StyleScale.h`, `.cpp`, `StyleTranslate.h`, `.cpp`, `StylePerspective.h`, `.cpp`, `StylePerspectiveOrigin.h`, `StyleTransformOrigin.h` | `port` | `Style/Values/Transforms/...`, `src/...` | on | tested. |

**Bugs fixed:**
- The f3b1 converter mapped upstream's `nameLiteral(CSSValueID)` to `ToLower(ValueId)`, so `transform` serialised `translateX(...)` as `translatex(...)` (and likewise `translateY`, `rotateX`, `skewY`, ...). It now maps it to `ToString(ValueId)`, as the ledger's translation rules say (`nameLiteralForSerialization` stays `ToLower`). Only `StyleTransformFunction.cpp` called `nameLiteral` among the converted files.

**Added:** none beyond the ports.

**Differences:** see decisions (blending without an interpolation context).

Tests: `tests/Graphics/Transforms/TransformOperations.cpp`, `tests/Style/Values/Transforms/Transforms.cpp` (`transform` and every function, serialisation, creation, platform conversion, blending, `rotate`, `scale`, `translate`, `perspective`, the origins): 4 cases. Full Booey suite: 1908 cases; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3d1: timing functions, easing, animations, transitions and scroll timelines (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/{easing,animations,transitions,scroll-animations}` except the ones below (19 headers, 14 sources), translated by the f3b1 script and reviewed and fixed by hand (`Animation` and `Transition` rewritten), and the platform files they need: `platform/animation/TimingFunction.h`, `.cpp`, `platform/graphics/UnitBezier.h`, `SpringSolver.h`, and `animatablePropertyAsString` from `animation/WebAnimationUtilities`. `style/ScopedName.h`, `.cpp` completed.

**Decisions (2026-10-14):**
- f3d split in three: f3d1, timing and animations (this layer); f3d2, transforms (the transform operations, `transforms`, `transforms/functions`); f3d3, filters and anchor positioning (the filter operations, the `Path` stand-in, `filter-effects`, `anchor-position`).
- Moved to Web Animations: `scroll-animations/StyleScrollTimelines.h`, `StyleViewTimelines.h` (lists of runtime `ScrollTimeline`/`ViewTimeline` objects) and `SingleAnimationRangeStart`/`End::toTimelineRangeValue()` (needs the Typed OM's `CSSNumericValue`; FIXME in both).
- `TimingFunction`: the virtual `operator==` is a non-virtual `operator==` over a virtual `Equals` (MSVC finds the C++20 reversed candidates of an `operator==(const TimingFunction &)` declared in each subclass ambiguous when comparing two subclass objects). `SpringTimingFunction::create()` (for IPC decoding) isn't ported. The `TextStream` dumping is `CssText()`'s fallback, with `FormatCSSNumber` for every number (upstream writes a linear point's value with `TextStream`'s default double formatting).
- `Animation` and `Transition` use Krystal's coordinated value list macros (f3a): getters are `GetName()`, `GetDelay()`, ...; the data structs are `AnimationData`/`TransitionData` (the macros reach them through `Data()`). The property macros are `KRYS_FOR_EACH_ANIMATION_*`/`KRYS_FOR_EACH_TRANSITION_*`, kept one invocation per line with `// clang-format off`. Logging isn't ported.
- `ScopedName` gets its `switchOn`, `CSSValueConversion` and `MarkableTraits` (left out in layer 6; animation names serialise through it).
- Names: `SingleAnimationRange::start`/`end` → `Start`/`End`; `ScrollFunctionParameters`' `Scroller`/`Axis`; `ViewFunctionParameters`' `Axis`/`Insets`; `ViewTimelineInsetItem::value` → `Value`, `end()` → `End()`; `ProgressTimelineName::Value`; `SingleTransitionProperty`'s `UnknownProperty::Value`/`SingleProperty::Value`; `convertSingleAnimationRangeNameToCSSValueID` → `ConvertSingleAnimationRangeNameToValueId` (and the reverse); `CubicBezierTimingFunction::timingFunctionPreset()` → `GetTimingFunctionPreset()`, `StepsTimingFunction::stepPosition()` → `GetStepPosition()`, `TimingFunction::type()` → `GetType()`; the easing functions' parameter members are the CSS easing types' (`Parameters`, `Stops`, `Output`, `Input`, ...).
- `WebAnimationUtilities` gets only `AnimatablePropertyAsString` (in `Animation/WebAnimationUtilities.hpp`, next to `WebAnimationTypes.hpp`).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/animation/TimingFunction.h`, `.cpp` | `port` | `Graphics/Animation/TimingFunction.hpp`, `src/...` | on | tested. See decisions. |
| `platform/graphics/UnitBezier.h`, `SpringSolver.h` | `port` | `Graphics/UnitBezier.hpp`, `SpringSolver.hpp` | on | tested (through `TimingFunction`). |
| `animation/WebAnimationUtilities.h`, `.cpp` (`animatablePropertyAsString`) | `port` (partial) | `Animation/WebAnimationUtilities.hpp` | on | tested (through `transition-property`). The rest comes with Web Animations. |
| `style/ScopedName.h`, `.cpp` | `have` | `Style/ScopedName.hpp`, `src/Style/ScopedName.cpp` | on | tested. Completed; see decisions. |
| `style/values/easing/*` | `port` | `Style/Values/Easing/...`, `src/...` | on | tested, with the deprecated conversions (string to `TimingFunction`, for Web Animations). |
| `style/values/animations/*` | `port` | `Style/Values/Animations/...`, `src/...` | on | tested. See decisions (`Animation`, `toTimelineRangeValue`). |
| `style/values/transitions/*` | `port` | `Style/Values/Transitions/...`, `src/...` | on | tested. See decisions (`Transition`). |
| `style/values/scroll-animations/StyleProgressTimelineAxes.h`, `StyleProgressTimelineName.h`, `.cpp`, `StyleScrollFunction.h`, `.cpp`, `StyleViewFunction.h`, `.cpp`, `StyleViewTimelineInsets.h`, `.cpp` | `port` | `Style/Values/ScrollAnimations/...`, `src/...` | on | tested. |
| `style/values/scroll-animations/StyleScrollTimelines.h`, `StyleViewTimelines.h` | `port` (Web Animations) | | | Moved; see decisions. |

**Bugs fixed:** none found.

**Added:** `ScopedName`'s conversion (see decisions). Test helper: the fixture's parser context also enables `scrollDrivenAnimationsEnabled` and `springTimingFunctionEnabled`.

**Differences:** none beyond the decisions.

Tests: `tests/Graphics/Animation/TimingFunction.cpp`, `tests/Style/Values/Animations/AnimationGroups.cpp` (easing, animation values and timelines, ranges, `Animation`, `Transition`, scroll timelines): 7 cases. Full Booey suite: 1904 cases / 686254 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3c: fonts (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/fonts` (14 headers, 13 sources), translated by the f3b1 script and then reviewed and fixed by hand (`FontFamily` rewritten), and the platform font types they and the seam need: `platform/graphics/FontSelectionAlgorithm.h`, `.cpp`, `FontSizeAdjust.h`, `FontPalette.h`, `FontMetrics.h`, `FontBaseline.h`, `FontDescription.h`, `.cpp`, `FontCascadeDescription.h`, `.cpp`, `SystemFontDatabase.h`, `.cpp`, `platform/text/LocaleToScriptMapping.h`, `.cpp`, and a shell of `FontCascade.h`, `.cpp`. Hand-ported.

**Decisions (2026-10-14):**
- `FontCascade` is a shell (as planned): its description, letter and word spacing, generation, `enableKerning`/`requiresShaping` and their computations. No fonts, font selector, glyph, width, drawing or text layout APIs (FIXME until there's a font backend). With no primary font, `MetricsOfPrimaryFont()` is a fallback derived from the computed size: an ascent of 1em, a line spacing of 1.2em and an ideogram width of 1em, with no descent, line gap, x-height, cap height or zero width (the seam's former initial-font values, generalised to any size), so `ex`, `cap` and `ch` keep their CSS fallbacks. Equality compares the description and the spacing (upstream also compares the fonts, and two cascades without fonts are never equal). `CanMakeWeakPtr`/`CanMakeCheckedPtr` are left off until something points at a cascade.
- The seam's `FontMetrics`, `FontCascadeDescription` and `FontCascade` stand-ins are gone: the seam's `RenderStyle` holds a real `FontCascade`, initially `medium` (16px specified and computed, keyword size 4, no families), and grows `MutableFontCascade()`, `SetFontDescription()` and `ComputedFontSize()`. `ComputedLineHeight()`/`ComputeSpecifiedLineHeight()` are the primary font's line spacing (still for the initial `line-height`).
- `FontCascadeDescription`'s families are a `List<StringAtom>` held by value, not a shared `RefCountedFixedVector` (no Krystal.Core type yet; equality compares the lists as upstream's `arePointingToEqualData` does). So are `style::FontFamilies`' (`ToPlatform()`/`TakePlatform()` return the list). `effectiveFamilyCount`/`effectiveFamilyAt` are the non-Cocoa versions, and `FontFamilySpecification` is the family name (no platform specification alternative). `resolveFontSizeAdjustFromFontIfNeeded(const Font &)` comes with the font backend (FIXME).
- `SystemFontDatabase` keeps only the standard system font keywords (`caption`, `icon`, `menu`, `message-box`, `small-caption`, `status-bar`; the `-webkit-*-control` ones were dropped from the keywords). With no platform fonts, every system font is the standard family at 16px and normal weight (FIXME), as upstream's PlayStation port. `css::PropertyParserHelpers::LowerFontShorthand` and the `FontShorthand` sync check came with it.
- `FontDescription::setSpecifiedLocale` computes Han locales to `zh-hans`: the specialised Chinese locale comes from the user's preferred languages, and there are none yet (FIXME; `wtf/Language` isn't ported). `-webkit-body` converts to the empty family: `Settings::standardFontFamily()` is empty by default and there's no `Settings` port (FIXME).
- ICU's `uc` is linked publicly: `FontDescription.hpp` and `LocaleToScriptMapping.hpp` expose `UScriptCode`.
- Names: struct members are PascalCase; where the name would hide a type it's renamed: `FontSizeAdjust::metric` → `MetricType` (the `Metric` enum), `FontPalette::type` → `PaletteType` (the `Type` enum). Getters that would hide a `krys::boo` type get `Get`: `FontDescription`'s `GetTextRenderingMode`, `GetTextSpacingTrim`, `GetTextAutospace`, `GetNonCJKGlyphOrientation`, `GetFontStyleAxis`, `GetFontPalette`, `GetFontSizeAdjust`, `GetFontSelectionRequest`, `FontCascadeDescription::GetKerning`, `FontCascade::GetFontDescription`; in the style types `FontSizeAdjust::GetMetric`, `FontStyle::GetAngle`, `FontWeight::GetNumber`, `FontWidth::GetPercentage` and the variant types' `GetPlatform()` (they alias `Platform`). `FontSelectionAlgorithm`'s free functions are PascalCase (`NormalWeightValue()`, `IsItalic()`, ...). `add(Hasher&, ...)` overloads are `DefaultHash` specialisations.
- `TextStream` dumping isn't ported.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `platform/graphics/FontSelectionAlgorithm.h`, `.cpp` | `port` | `Graphics/FontSelectionAlgorithm.hpp`, `src/...` | on | tested. `Vector<bool, 256>` → a heap array (`List<bool>` packs bits). |
| `platform/graphics/FontSizeAdjust.h`, `FontPalette.h`, `FontMetrics.h`, `FontBaseline.h` | `port` | `Graphics/...` | on | tested. See decisions (names). |
| `platform/graphics/FontDescription.h`, `.cpp` | `port` | `Graphics/FontDescription.hpp`, `src/...` | on | tested. See decisions (locale). |
| `platform/graphics/FontCascadeDescription.h`, `.cpp` | `port` | `Graphics/FontCascadeDescription.hpp`, `src/...` | on | tested. See decisions (families). `familyNameHash`'s case-insensitive hash hashes the folded name. Text autosizing members not ported (`TEXT_AUTOSIZING` is off). |
| `platform/graphics/FontCascade.h`, `.cpp` | `port` (shell) | `Graphics/FontCascade.hpp`, `src/...` | on | tested. See decisions. |
| `platform/graphics/SystemFontDatabase.h`, `.cpp` | `port` | `Graphics/SystemFontDatabase.hpp`, `src/...` | on | tested. See decisions. |
| `platform/text/LocaleToScriptMapping.h`, `.cpp` | `port` | `Core/LocaleToScriptMapping.hpp`, `src/...` | on | tested. `SortedArrayMap` → a sorted array searched with `lower_bound` (ASCII case-insensitive, as upstream's packed keys). |
| `style/values/fonts/StyleFontFamily.h`, `.cpp`, `StyleFontFamilyName.h`, `.cpp` | `port` | `Style/Values/Fonts/FontFamily.hpp`, `FontFamilyName.hpp`, `src/...` | on | tested. See decisions (families, `-webkit-body`). |
| `style/values/fonts/StyleFontFeatureSettings.h`, `.cpp`, `StyleFontVariationSettings.h`, `.cpp`, `StyleFontOpentypeTag.h` | `port` | `Style/Values/Fonts/...`, `src/...` | on | tested, with blending. |
| `style/values/fonts/StyleFontPalette.h`, `.cpp`, `StyleFontSizeAdjust.h`, `.cpp`, `StyleFontStyle.h`, `.cpp`, `StyleFontWeight.h`, `.cpp`, `StyleFontWidth.h`, `.cpp` | `port` | `Style/Values/Fonts/...`, `src/...` | on | tested, with blending. |
| `style/values/fonts/StyleFontVariantAlternates.h`, `.cpp`, `StyleFontVariantEastAsian.h`, `.cpp`, `StyleFontVariantLigatures.h`, `.cpp`, `StyleFontVariantNumeric.h`, `.cpp` | `port` | `Style/Values/Fonts/...`, `src/...` | on | tested. |
| `css/CSSPrimitiveValueMappings.h` (`FontSizeAdjust::Metric`), `StylePrimitiveKeyword+ValueRepresentationNeeded.h` (`FontSizeAdjust::Metric`) | `have` | `CSS/PrimitiveValueMappings.hpp`, `Style/Values/Primitives/KeywordValueRepresentationNeeded.hpp` | on | Added with the type (their FIXMEs updated). |
| `css/parser/CSSPropertyParserConsumer+Font.h` (`lowerFontShorthand`) | `have` | `CSS/Properties/Consumers/Font.hpp` | on | Added with `SystemFontDatabase`; `IsSystemFontShorthand` gains the sync check. |

**Bugs fixed:**
- `Style::FontSizeAdjust` declares `DEFINE_VARIANT_LIKE_CONFORMANCE` but has no `switchOn`: it met the variant-like concept without being visitable. Krystal drops the conformance (it has its own conversion, creation and serialisation).

**Added:** `RenderStyle` (seam): a real `FontCascade`, `MutableFontCascade`, `SetFontDescription`, `ComputedFontSize`.

**Differences:**
- With no font data, `font-size-adjust: from-font` resolves against the fallback metrics: `ic-width`/`ic-height` resolve to 1, the other metrics to nothing (so it serialises as `none`).

Tests: `tests/Graphics/FontSelectionAlgorithm.cpp`, `tests/Graphics/FontDescription.cpp` (metrics, size adjust, palette, descriptions, cascade, system fonts), `tests/Core/LocaleToScriptMapping.cpp`, `tests/Style/Values/Fonts/Fonts.cpp` (every fonts type from parsed CSS, the system font keyword, creation, serialisation and blending): 23 cases. Full Booey suite: 1897 cases / 685833 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f3b2: alignment, inline, text, scroll snap, UI and grid (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/{align,inline,text,scroll-snap,ui,grid}` except the ones below, plus `rendering/style/StyleContentAlignmentData.h`, `.cpp` and `StyleSelfAlignmentData.h`, `.cpp`: 51 headers and 36 sources upstream. Translated by the f3b1 script, then reviewed and fixed by hand; the grid files that hold maps and track lists (`GridNamedLinesMap`, `GridOrderedNamedLinesMap`, `GridTemplateAreas`, `GridTemplateList`) were rewritten by hand.

**Decisions (2026-10-14):**
- Moved: `ui/StyleCursor.h`, `.cpp` to f3e (it holds images); `grid/StyleGridPositionsResolver.h`, `.cpp` to layout (it resolves positions against a `RenderGrid`).
- `align-self`/`justify-self` don't apply the position-try fallback tactics yet (FIXME in both): `BuilderState::positionTryFallback` and `AnchorPositionEvaluator` come with anchor positioning (f3d).
- `resize: -internal-textarea-auto` computes to `none`: it reads `Settings::textAreasAreResizable`, and there's no `Settings` port yet (FIXME); `none` is that setting's default.
- Upstream's `Vector`-backed members are `List`s and its `HashMap`s are `std::unordered_map` (`try_emplace` for `add`). `GridOrderedNamedLinesMap` drops the custom hash traits (`IntHash`, `UnsignedWithZeroKeyHashTraits`): `std::hash` handles zero keys.
- Public data members are PascalCase. `GridTemplateList::list` and the repeat entries' `list` are `Tracks` (a member named `List` would hide `krys::List` in the struct); `GridTrackMinMaxParameters::min`/`max` → `Min`/`Max`; `TextEdgePair`'s `Over`/`Under`, `TextIndent`'s `Length`/`Hanging`/`EachLine`, `TextSpacingTrim::Value`, `ScrollSnapAlign`'s `BlockAlign`/`InlineAlign`, `ScrollSnapType::ScrollSnapContainer`'s `Axis`/`Strictness`.
- The seam's `RenderStyle` grows `TextAlign()`/`SetTextAlign` and `TextAlignLast()`/`SetTextAlignLast` (no `Get`: no `krys::boo` type has those names).
- `-webkit-initial-letter`, `-webkit-hyphenate-limit-*` and `-webkit-line-box-contain` are still in `css-properties.json`, so `WebKitInitialLetter`, `HyphenateLimitEdge`/`HyphenateLimitLines` are ported.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `align/StyleAlignContent.h`, `.cpp`, `StyleAlignItems.h`, `.cpp`, `StyleAlignSelf.h`, `.cpp`, `StyleJustifyContent.h`, `.cpp`, `StyleJustifyItems.h`, `.cpp`, `StyleJustifySelf.h`, `.cpp`, `StyleBaselineAlignmentPreference.h`, `StyleGapGutter.h`, `StyleLegacyPosition.h`, `StyleOverflowPosition.h` | `port` | `Style/Values/Align/...`, `src/...` | on | tested. See decisions (position-try fallback). `overflowPosition()`/`baselineAlignmentPreference()` → `GetOverflowPosition()`/`GetBaselineAlignmentPreference()` (they'd hide their types). |
| `rendering/style/StyleContentAlignmentData.h`, `.cpp`, `StyleSelfAlignmentData.h`, `.cpp` | `port` | `Rendering/Style/StyleContentAlignmentData.hpp`, `StyleSelfAlignmentData.hpp`, `src/...` | on | tested (through the alignment types). `TextStream` dumping not ported. |
| `inline/StyleLineHeight.h`, `.cpp`, `StyleVerticalAlign.h`, `.cpp`, `StyleLineFitEdge.h`, `StyleTextBoxEdge.h`, `StyleTextEdge.h`, `StyleTextEdge+CSSValueConversion.h`, `StyleWebKitInitialLetter.h`, `.cpp` | `port` | `Style/Values/Inline/...` (`TextEdgeCSSValueConversion.hpp`), `src/...` | on | tested. `LineHeight`'s percentage and calc paths use the seam's `GetFontDescription()`. |
| `text/StyleHangingPunctuation.h`, `.cpp`, `StyleHyphenateCharacter.h`, `StyleHyphenateLimitEdge.h`, `StyleHyphenateLimitLines.h`, `StyleLetterSpacing.h`, `.cpp`, `StyleTabSize.h`, `.cpp`, `StyleTextAlign.h`, `.cpp`, `StyleTextAlignLast.h`, `.cpp`, `StyleTextAutospace.h`, `.cpp`, `StyleTextIndent.h`, `.cpp`, `StyleTextSpacingTrim.h`, `StyleTextTransform.h`, `.cpp`, `StyleWordSpacing.h`, `.cpp` | `port` | `Style/Values/Text/...`, `src/...` | on | tested. `TextAutospace::shouldApplySpacing(char32_t, char32_t)` comes with text layout (FIXME, as `text_spacing`'s). |
| `scroll-snap/StyleScrollMargin.h`, `.cpp`, `StyleScrollPadding.h`, `.cpp`, `StyleScrollSnapAlign.h`, `.cpp`, `StyleScrollSnapType.h`, `.cpp` | `port` | `Style/Values/ScrollSnap/...`, `src/...` | on | tested. |
| `ui/StyleAccentColor.h`, `.cpp`, `StyleResize.h`, `.cpp` | `port` | `Style/Values/UI/...`, `src/...` | on | tested. `Color::currentColor()` is `GetCurrentColor()` (layer 6). See decisions (`resize`). |
| `ui/StyleCursor.h`, `.cpp` | `port` (f3e) | | | Moved to f3e. |
| `grid/StyleGridAutoFlow.h`, `.cpp`, `StyleGridTrackBreadth.h`, `.cpp`, `StyleGridTrackSize.h`, `.cpp`, `StyleGridTrackSizes.h`, `.cpp`, `StyleItemTolerance.h`, `.cpp`, `StyleGridNamedAreaMap.h` | `port` | `Style/Values/Grid/...`, `src/...` | on | tested. `GridTrackSize::type()` → `GetType()`, `GridTrackBreadth::flex()` → `GetFlex()`. |
| `grid/StyleGridNamedLinesMap.h`, `StyleGridOrderedNamedLinesMap.h`, `StyleGridTemplateAreas.h`, `.cpp`, `StyleGridTemplateList.h`, `.cpp` | `port` | `Style/Values/Grid/...`, `src/...` | on | tested. Hand-ported; see decisions. `count()` → `Count()`. |
| `grid/StyleGridNamedLinesMap.cpp`, `StyleGridTemplateList.cpp`'s logging | `wont` | | | `TextStream` only. |
| `grid/StyleGridPositionsResolver.h`, `.cpp` | `port` (layout) | | | Moved to layout. |
| `css/CSSPrimitiveValueMappings.h` (`Style::Resize`, `TextAlign`, `TextAlignLast`, `HangingPunctuationValue`, `TextTransformValue`) | `have` | `CSS/PrimitiveValueMappings.hpp` | on | Added with their types. `fromCSSValueID<Style::TextAlign>` subtracts `CSSValueLeft` from the keyword (it relies on the keyword order); Krystal lists the cases. |
| `wtf/text/AtomString.h` (`MarkableTraits<AtomString>`) | `port` | `Krystal.Core/Types/StringAtom.hpp` | on | The null atom is the empty value (for `HyphenateCharacter`). |

**Bugs fixed:**
- `GridTemplateAreas(GridNamedAreaMap&&)` (upstream) builds the implicit named lines from its moved-from parameter (the parameter shadows the member), so a map passed by rvalue got no implicit `-start`/`-end` lines. Krystal builds them from the member.
- The generated `TextEdgeDerived` concept used `IsBaseOfTemplate<...>::value` (Krystal's is a concept, so the constraint never held) and the unqualified `VariantLike`; `text-box-edge`/`line-fit-edge` had no conversion.

**Added:** `RenderStyle` (seam): `TextAlign`/`SetTextAlign`, `TextAlignLast`/`SetTextAlignLast`. `MarkableTraits<StringAtom>` (Krystal.Core). Test helper: the fixture's parser context also enables `accentColorEnabled`, `cssAnchorPositioningEnabled`, `cssLineFitEdgeEnabled`, `cssTextAutospaceEnabled`, `cssTextBoxTrimEnabled` and `cssTextSpacingTrimEnabled`.

**Differences:**
- Upstream behaviour kept: `letter-spacing: normal` computes (and serialises) as `0px`.

Tests: `tests/Style/Values/AlignInlineTextGroups.cpp` (align, inline, text, scroll snap, UI), `tests/Style/Values/Grid/GridTemplate.cpp`: 8 cases. Full Booey suite: 1873 cases / 685202 assertions; all pass except 7 expected failures. Krystal.Core: 557 cases pass. Whole solution builds with no warnings.

### Layer f3b1: the small keyword and length groups (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`). Every file of `style/values/{box,sizing,flexbox,position,css2,display,break,multicol,page,math,rhythm,fill-stroke,contain,overflow,pointerevents,speech,view-transitions,will-change,counter-styles,lists,color-adjust,scrollbars,viewport}` except the ones below: 26 headers and 27 sources upstream. The files were translated by a script (names, namespaces, includes, signatures, braces) and then reviewed and fixed by hand against upstream.

**Decisions (2026-10-14):**
- Not ported (their properties were dropped): `flexbox/StyleWebKitBoxFlex.h`, `StyleWebKitBoxFlexGroup.h`, `StyleWebKitBoxOrdinalGroup.h` (deprecated flexbox), `line-grid/StyleWebKitLineGrid.h`, `size-adjust/StyleTextSizeAdjust.h`, `.cpp` (`-webkit-text-size-adjust`, behind the disabled `TEXT_AUTOSIZING`).
- The seam's `RenderStyle` grows the properties the value code reads, as storage plus a getter and setter each, under upstream's names in PascalCase: `AspectRatio()`, `MathDepth()`, `GetMathStyle()` (`Get` because `MathStyle` is a `krys::boo` type the getter would hide; Style value types live in `krys::boo::style`, so getters named after them don't hide them), and `LogicalAspectRatio()` (upstream's). The generated getters in f3f follow the same rule.
- Site-specific quirks aren't ported: `scrollbar-width: thin` ignores `DocumentQuirks::needsScrollbarWidthThinDisabledQuirk`.
- `will-change` checks property exposure against the document's parser context's property settings (standing in for the document's settings).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `box/StyleMargin.h`, `.cpp`, `StyleMarginTrim.h`, `.cpp`, `StylePadding.h` | `port` | `Style/Values/Box/Margin.hpp`, `MarginTrim.hpp`, `Padding.hpp`, `src/...` | on | tested. |
| `sizing/StyleAspectRatio.h`, `.cpp`, `StyleContainIntrinsicSize.h`, `.cpp`, `StyleMaximumSize.h`, `StyleMinimumSize.h`, `StylePreferredSize.h`, `.cpp` | `port` | `Style/Values/Sizing/...` | on | tested. `ContainIntrinsicSize`'s private `type`/`length` → `_type`/`_length`. `Ratio`'s members are `Numerator`/`Denominator`. |
| `flexbox/StyleFlexBasis.h`, `.cpp`, `StyleFlexGrow.h`, `StyleFlexShrink.h` | `port` | `Style/Values/Flexbox/...` | on | tested. |
| `position/StyleInset.h`, `display/StyleOrder.h`, `css2/StyleZIndex.h`, `.cpp` | `port` | `Style/Values/Position/Inset.hpp`, `Display/Order.hpp`, `CSS2/ZIndex.hpp`, `src/...` | on | tested. `ZIndex` befriends `RenderStyleProperties`/`StyleBoxData` (f3f) through forward declarations. |
| `break/StyleOrphans.h`, `.cpp`, `StyleWidows.h`, `.cpp`, `multicol/StyleColumnCount.h`, `.cpp`, `StyleColumnWidth.h`, `.cpp` | `port` | `Style/Values/Break/...`, `Multicol/...` | on | tested. |
| `page/StylePageSize.h`, `.cpp` | `port` | `Style/Values/Page/PageSize.hpp`, `src/...` | on | tested (values built directly: `size` is an `@page` descriptor). |
| `math/StyleMathDepth.h`, `.cpp`, `rhythm/StyleBlockStepSize.h`, `.cpp` | `port` | `Style/Values/Math/...`, `Rhythm/...` | on | tested. A local `value` shadowing the parameter in `MathDepth`'s conversion is renamed `addend`. |
| `fill-stroke/StyleStrokeMiterlimit.h`, `StyleStrokeWidth.h`, `.cpp` | `port` | `Style/Values/FillStroke/...` | on | tested. Public `value` members → `Value`. |
| `contain/StyleContain.h`, `.cpp`, `StyleContainerName.h` | `port` | `Style/Values/Contain/...` | on | tested. |
| `overflow/StyleBlockEllipsis.h`, `.cpp`, `StyleMaximumLines.h`, `StyleScrollBehavior.h`, `StyleScrollbarGutter.h`, `.cpp` | `port` | `Style/Values/Overflow/...` | on | tested. |
| `pointerevents/StyleTouchAction.h`, `.cpp`, `speech/StyleSpeakAs.h`, `.cpp` | `port` | `Style/Values/PointerEvents/...`, `Speech/...` | on | tested. `ToPlatform<TouchAction>` returns an `EnumSet<boo::TouchAction>`. |
| `view-transitions/StyleViewTransitionClass.h`, `StyleViewTransitionName.h`, `.cpp` | `port` | `Style/Values/ViewTransitions/...` | on | tested. `scopeOrdinal()` → `GetScopeOrdinal()`. |
| `will-change/StyleWillChange.h`, `.cpp` | `port` | `Style/Values/WillChange/...` | on | tested. Hand-ported. `feature()` → `GetFeature()` (it would hide `Feature`); the dropped `-webkit-` properties (`-webkit-perspective`, `-webkit-mask`, `-webkit-box-reflect`, `-webkit-backdrop-filter`, `-webkit-mask-box-image`) leave the stacking-context and containing-block lists. |
| `counter-styles/StyleCounterStyle.h`, `.cpp`, `lists/StyleListStyleType.h`, `.cpp` | `port` | `Style/Values/CounterStyles/...`, `Lists/...` | on | tested. No IPC. `nameString(id)` → `css::CSSOMStringAtom {css::ToString(id)}`. `CounterStyle::identifier` → `Identifier`. |
| `color-adjust/StyleColorScheme.h`, `.cpp` | `port` | `Style/Values/ColorAdjust/ColorScheme.hpp`, `src/...` | on | tested. Hand-ported. `colorScheme()` → `GetColorScheme()`; members `Schemes`/`Only`. `boo::ColorScheme` (`RenderStyleConstants`) values are sequential, for `EnumSet`. |
| `scrollbars/StyleScrollbarColor.h`, `.cpp`, `StyleScrollbarWidth.h`, `.cpp` | `port` | `Style/Values/Scrollbars/...` | on | tested. Parts members `Thumb`/`Track`. See decisions (quirk). |
| `viewport/StyleZoom.h` | `port` | `Style/Values/Viewport/Zoom.hpp` | on | tested. The `Value` alias → `Representation` (the member is `Value`). |
| `css/CSSPrimitiveValueMappings.h` (`Style::MarginTrimSide`, `ContainValue`, `ScrollBehavior`, `ScrollbarWidth`, `SpeakAsValue`, `TouchActionValue`) | `have` | `CSS/PrimitiveValueMappings.hpp` | on | Added with their types. |

**Bugs fixed:** `CSSValueConversion` of `css::SpaceSeparatedEnumSet`/`CommaSeparatedEnumSet` (f3a) called `map`; the enum set wrappers' function is `Map` (found when the first enum set types were converted).

**Added:** `RenderStyle` (seam): `AspectRatio`/`SetAspectRatio`/`LogicalAspectRatio`, `MathDepth`/`SetMathDepth`, `GetMathStyle`/`SetMathStyle`. `BuilderChecking` handles `css::Value` as the required type (upstream's `downcast<CSSValue>` is a no-op; Krystal's rejects same-type casts) and includes `PrimitiveValue.hpp`. Test helper: `BuilderFixture::Parse` uses its own parser context with the settings-gated properties enabled.

**Differences:**
- Upstream behaviour kept: `margin-trim: none` is invalid at computed-value time (it isn't one of the converted keywords) and computes to the initial `none`.

Tests: `tests/Style/Values/Box/Box.cpp`, `tests/Style/Values/Sizing/Sizing.cpp` (also flexbox), `tests/Style/Values/KeywordAndLengthGroups.cpp` (the other groups): 13 cases. Full Booey suite: 1865 cases / 684657 assertions; all pass except 7 expected failures. Krystal.Core: 557 cases pass. Whole solution builds with no warnings.

### Layer f3a: computed style foundations (2026-10-14)

Graphs: the planning graphs (`tools/port/out/f3-all.md`, `f3-computed.md`). New over the plan: `css/CSSPrimitiveValueMappings.h` (the keyword conversions, creations and serialisations are written against it), `platform/graphics/GraphicsTypes.h` (the rest of it), `WindRule.h` and `animation/ScrollAxis.h` (mapped there). `FloatRoundedRect`/`LayoutRoundedRect` moved to f3e: `StyleBorderRadius` is their only user.

**Decisions (2026-10-14):**
- The seam's `BuilderState` mirrors upstream: built from a `RenderStyle` and a `BuilderContext` (document, parent style, root element style, element), with upstream's conversion data, invalid-at-computed-value-time bits, content attribute registration, SVG zoom rules and tree counting. The builder's current property is set through `SetCurrentProperty` (collapses `Builder` being a friend); the fonts, custom properties, style images, `random()` caches and position fallbacks come with the builder (f4) or their sublayers. Conversions include `Style/Seam.hpp` (FIXME in `BuilderChecking.hpp` to include `Style/BuilderState.hpp` once the builder is ported).
- `PrimitiveValueMappings.hpp` maps every enum that exists; each Style value type's mappings (`ContainValue`, `TextAlign`, `TouchActionValue`, ... and `FontSizeAdjust::Metric`, listed in a FIXME at the top) come with its group. Mappings live in `krys::boo` (upstream's `WebCore`), so ADL finds them for the enums and `css::ToValueId(GlobalKeyword)` doesn't hide them.
- Enums whose only properties were dropped (deprecated flexbox `BoxAlign`/`BoxDirection`/`BoxLines`/`BoxOrient`/`BoxPack`, `-webkit-box-reflect`, `-webkit-column-axis`, `-webkit-line-snap`, marquee, `-webkit-nbsp-mode`, `-webkit-text-zoom`) and disabled features (`CursorVisibility`, Apple Pay, `AppleVisualEffect`, attachment, service controls) aren't ported, with their mappings.
- Coordinated value list owners declare a getter `GetX()` for every property (several would hide their types, e.g. `PositionX`, `BlendMode`); the macros use it.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/StyleBuilderState.h`, `.cpp` (the conversion half) | `stub` (seam) | `style::BuilderState`, `BuilderContext` in `Style/Seam.hpp`, `src/Style/Seam.cpp` | on | tested. See decisions. `cssPropertyID` → `CssPropertyId`. |
| `style/StyleBuilderChecking.h` | `have` | `Style/BuilderChecking.hpp` | on | tested (through the conversions). Was half converted. `requiredDowncast` returns a raw pointer, as upstream. `TypedRequiredList::list` → `List`. |
| `style/values/StyleValueTypes+CSSValueConversion.h` | `have` | `Style/Values/CSSValueConversion.hpp` | on | tested (strings, identifiers, lists; the wrapper/`ListOrNone`/`EnumSetOrKeyword` specialisations are instantiated by f3b's types). The fixed-vector lists are `css::SpaceSeparatedFixedList`/`CommaSeparatedFixedList`. |
| `style/values/primitives/StylePrimitiveKeyword+CSSValueConversion.h`, `+CSSValueCreation.h`, `+Serialization.h`, `+ValueRepresentationNeeded.h` | `have` | `KeywordCSSValueConversion.hpp`, `KeywordCSSValueCreation.hpp`, `KeywordSerialisation.hpp`, `KeywordValueRepresentationNeeded.hpp` | on | tested. Were raw WebKit. `+Logging` not ported. `BoxOrient` dropped from the list. |
| `StylePrimitiveNumericTypes+CSSValueConversion.h`, `+CSSValueCreation.h` | `have` | `NumericTypesCSSValueConversion.hpp`, `NumericTypesCSSValueCreation.hpp` | on | tested. Were raw WebKit. |
| `StyleLengthWrapper+CSSValueConversion.h`, `+DeprecatedCSSValueConversion.h`, `.cpp` | `have` | `LengthWrapperCSSValueConversion.hpp`, `LengthWrapperDeprecatedCSSValueConversion.hpp`, `src/...` | on | tested. `processKeyword` → `ProcessKeyword`. Elements have no renderer, so the deprecated conversion has no conversion data (FIXME): font- and viewport-relative lengths and calc don't convert. |
| `StylePosition.h`, `.cpp` | `have` | `Style/Values/Primitives/Position.hpp`, `src/...` | on | tested. Members `x`/`y` → `X`/`Y` (upstream's `X`/`Y` aliases dropped; nothing uses them). |
| `StyleURL.h`, `.cpp` | `have` | `Url.hpp`, `src/.../Url.cpp` | on | tested. `toStyleWithScriptExecutionContext` takes a `dom::Document` (there's no `ScriptExecutionContext`). Logging not ported. |
| `StyleCoordinatedValueList.h`, `StyleCoordinatedValueListValue.h` | `have` | `CoordinatedValueList.hpp`, `CoordinatedValueListValue.hpp` | on | tested with a test layer. Were half converted. See decisions; methods PascalCase (`PrepareForUse`, `UsedLength`, ...), owners' `clone`/`computedValueUsesUsedValues`/`baseProperty`/`properties` → `Clone`/`ComputedValueUsesUsedValues`/`BaseProperty`/`Properties`. |
| `StyleRatio.h`, `.cpp` (`CSSValueCreation<Ratio>`) | `have` | `Ratio.hpp`, `src/.../Ratio.cpp` | on | tested. |
| `style/values/color/StyleColor.cpp`, `StyleDynamicRangeLimit.cpp`, `StyleOpacity.cpp` (the CSS value halves) | `port` | `Color.cpp`, `DynamicRangeLimit.cpp`, `src/.../Opacity.cpp` | on | tested. Deferred from layer 6. `element()->isLink()` → `css::IsLink(element)`. |
| `css/CSSPrimitiveValueMappings.h` | `have` | `CSS/PrimitiveValueMappings.hpp` | on | tested. Was raw WebKit; converted by script from upstream. See decisions. `toCSSValueID`/`fromCSSValueID`/`fromCSSValue` → `ToValueId`/`FromValueId`/`FromCSSValue`. Replaces the local mappings in `ColorLayers.cpp`, `RayValue.cpp` and `Font.cpp`. |
| `rendering/style/RenderStyleConstants.h`, `.cpp` | `have` | `Rendering/Style/RenderStyleConstants.hpp`, `src/...` | on | tested. Now complete (was `CSSBoxType` and `PseudoElementType` only). `parentPseudoElement`/`clipMax`/`alwaysPageBreak`/`transformBoxToCSSBoxType` → PascalCase; `allPublicPseudoElementTypes`/`allPseudoElementTypes` → `AllPublicPseudoElementTypes`/`AllPseudoElementTypes` (no internal ones). `DumpStyleValues` and the `operator<<`s not ported. |
| `platform/text/TextFlags.h`, `.cpp` | `port` | `Core/TextFlags.hpp`, `src/Core/TextFlags.cpp` | on | tested. Struct members PascalCase; `FontVariantAlternates::values()` → `GetValues()`; null strings → `Maybe<CSSOMString>`; `add(Hasher&, ...)` → `DefaultHash` specialisations. `FourCharacterTagHash` gained an `operator()` for `HashMap`. |
| `platform/text/TextSpacing.h`, `.cpp` | `port` (partial) | `Core/TextSpacing.hpp`, `src/Core/TextSpacing.cpp` | on | tested. Namespace `TextSpacing` → `text_spacing`; `TextAutospace::Type` is a namespace-level `TextAutospaceType` (sequential, for `EnumSet`). `characterClass`, `isIdeograph`, `getHalfWidthFontIfNeeded`, `shouldApplySpacing(char32_t, char32_t)` and `textAutospaceSize` come with text layout (FIXME). |
| `platform/text/UnicodeBidi.h`, `platform/StyleAppearance.h`, `platform/ThemeTypes.h`, `platform/TouchAction.h`, `platform/graphics/TabSize.h` | `port` | `Core/UnicodeBidi.hpp`, `Core/StyleAppearance.hpp`, `Core/ThemeTypes.hpp`, `Core/TouchAction.hpp`, `Graphics/TabSize.hpp` | on | tested (`UnicodeBidi`, `TabSize`). `TouchAction` values are sequential (`EnumSet`). `TabSize`'s members are private (`Value()`, `IsSpaces()`). |
| `platform/ScrollTypes.h` | `port` (partial) | `Core/ScrollTypes.hpp` | on | `OverscrollBehavior`, `OverflowAnchor` and `ScrollbarWidth` only; the scrolling types come with scrolling. |
| `platform/graphics/GraphicsTypes.h`, `WindRule.h`, `animation/ScrollAxis.h` | `port` | `Graphics/GraphicsTypes.hpp`, `Graphics/WindRule.hpp`, `Animation/ScrollAxis.hpp` | on | `GraphicsTypes` was `BlendMode` only. `TextDrawingMode` values are sequential. The canvas name/parse functions come with the canvas (FIXME). |
| `platform/graphics/FloatRoundedRect.h`, `LayoutRoundedRect.h` | `port` (f3e) | | | Moved to f3e. |

**Bugs fixed:**
- `WritingMode::SetTextOrientation` cleared `OrientationShift` (the vertical typographic bit) instead of the orientation mask: setting `text-orientation` on a vertical writing mode made it horizontal-typographic, and the old orientation bits weren't cleared.

**Added:** `RenderStyle` (seam): `SetDirection`/`SetTextOrientation`/`SetWritingMode` and the writing mode, `GetPseudoElementType`/`GetPseudoElementIdentifier`/`SetPseudoElementIdentifier`, `UsesViewportUnits`/`UsesContainerUnits`/`UsesTreeCountingFunctions` and their setters; `AdjustFloatForAbsoluteZoom` (upstream's). `HasPseudoElementType` removed (`LengthResolution.cpp` uses `GetPseudoElementType`, as upstream). Test helper: `BuilderFixture` (`Krystal.Booey.Tests/Style/BuilderTestUtils.hpp`), which parses a declaration and converts its value through the seam's `BuilderState`.

**Differences:**
- Style URL: an invalid value converts to the URL parsed from `""` (non-null, invalid), as upstream.
- `CoordinatedValueList::operator==` compares the data pointers, then the data (upstream's `arePointingToEqualData`).

Tests: `tests/CSS/PrimitiveValueMappings.cpp`, `tests/Style/BuilderState.cpp`, `tests/Style/Values/CSSValueConversion.cpp`, `tests/Style/Values/Color/ColorConversion.cpp`, `tests/Style/Values/Primitives/{Position,Url,CoordinatedValueList}.cpp`, `tests/Core/TextFlags.cpp` (also `TextSpacing`), `tests/Rendering/Style/RenderStyleConstants.cpp` (also `UnicodeBidi`, `TabSize`, `WritingMode`): 20 cases. Full Booey suite: 1852 cases / 684017 assertions; all pass except 7 expected failures. Krystal.Core: 557 cases pass. Whole solution builds with no warnings.

### Layer f0: rule sets (2026-10-14)

Graphs from the nine f0 entries (headers-only): `style/RuleSet.cpp` (89 files), `RuleSetBuilder.cpp` (130), `RuleData.cpp` (85), `RuleFeature.cpp` (41), `StyleScopeRuleSets.cpp` (91), `UserAgentStyle.cpp` (105), `HasSelectorFilter.cpp` (39), `StyleRelations.cpp` (4) and `css/SelectorFilter.cpp` (4) (`tools/port/out/f0-*.md`). Most of the reach is through `StyleResolver.h` (the rest of style resolution: matching, the cascade, the builder) and is left to later layers. New outside the plan: `PropertyAllowlist` (pulled from f1, `RuleData` needs it), `DeclarationOrigin`, the rule set half of `Style::Resolver`, `CSSCounterStyle`/`CSSCounterStyleRegistry` (unreviewed on disk), WTF's `BloomFilter` and `numGraphemeClusters`.

**Decisions (2026-10-14):**
- The rule set half of `Style::Resolver` comes now: creation and initialisation (no root default style; FIXME, as in e7), the rule sets, the media query evaluator, author style sheets, keyframes, shared shadow tree resolvers, and `Style::Scope`'s resolver hooks (create, reconstruct, reset or append on style sheet changes). Style resolution (`styleForElement` and the rest) waits for f5.
- Of the at-rules a rule set hands to the resolver, `@keyframes` (the resolver's keyframes map), `@counter-style` (the scope's counter style registry; `CSSCounterStyle` and `CSSCounterStyleRegistry` are reviewed here so `counterStyles.css` registers), `@view-transition` and `@position-try` (the rule set) are handled. `@property` registers with the scope's custom property registry (since f4c); `@font-face`, `@font-palette-values` and `@font-feature-values` wait for the font selector (FIXMEs).
- The user agent sheets are checked in as cleaned copies (`codegen/user-agent-style-sheets/`), embedded by a codegen step. See *User agent sheets* below.
- Style relations are collected now (`SelectorChecker`'s `addStyleRelation` calls); committing them to element flags and styles (`commitRelations`, `commitRelationsToRenderStyle`, `copyRelations`) waits for the resolver (f5).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/RuleData.h`, `.cpp` | `port` | `Style/RuleData.hpp`, `src/...` | on | tested. |
| `style/RuleSet.h`, `.cpp` | `port` | `Style/RuleSet.hpp`, `src/...` | on | tested. No `selectorsForDebugging` (debug dumping). |
| `style/RuleSetBuilder.h`, `.cpp` | `port` | `Style/RuleSetBuilder.hpp`, `src/...` | on | tested. See differences. |
| `style/RuleFeature.h`, `.cpp` | `have` | `Style/RuleFeature.hpp`, `src/...` | on | tested. Completes e6's partial port. |
| `style/StyleScopeRuleSets.h`, `.cpp` | `port` | `Style/ScopeRuleSets.hpp`, `src/...` | on | tested. No user style sheets (FIXME) or inspector. |
| `style/StyleResolver.h`, `.cpp` (creation, `initialize`, `appendAuthorStyleSheets`, keyframes maps, `hasSelectorFor*`, dynamic media queries, `viewTransitionRule`, sharing) | `port` (partial) | `Style/Resolver.hpp`, `src/...` | on | tested. Replaces the seam's `style::Resolver`. Style resolution comes with f5. |
| `style/UserAgentStyle.h`, `.cpp` | `have*` | `Style/UserAgentStyle.hpp`, `src/...` | on | tested. Old header used `css::RuleSet`; rewritten. See differences. |
| `css/*.css` (the user agent sheets), `css/make-css-file-arrays.pl` | `port` | `codegen/user-agent-style-sheets/*.css`, `codegen/user_agent_style_sheets.py` → `CSS/UserAgentStyleSheets.hpp`/`.cpp` (generated) | on | tested (every rule parses). See *User agent sheets*. |
| `style/PropertyAllowlist.h`, `.cpp` | `port` | `Style/PropertyAllowlist.hpp`, `src/...` | on | Through `RuleData`. Pulled from f1. |
| `style/DeclarationOrigin.h` | `port` | `Style/DeclarationOrigin.hpp` | on | |
| `css/SelectorFilter.h`, `.cpp` | `port` | `CSS/Selectors/SelectorFilter.hpp`, `src/...` | on | tested. No `collectHashesForTesting` (test hook). |
| `style/HasSelectorFilter.h`, `.cpp` | `port` | `Style/HasSelectorFilter.hpp`, `src/...` | on | tested. No hover state (FIXME). |
| `style/SelectorMatchingState.h` | `port` | `Style/SelectorMatchingState.hpp` | on | tested. No container query evaluation state (FIXME, f5). |
| `style/StyleRelations.h` | `port` | `Style/Relations.hpp` | on | tested. The `.cpp` (committing relations) comes with f5. |
| `css/SelectorChecker.cpp` (`addStyleRelation`, the `:has()` match cache and filters) | `have` | `CSS/Selectors/SelectorChecker.cpp` | on | tested. Were FIXMEs from e6. |
| `style/StyleScope.cpp` (`resolver`, `createDocumentResolver`, `createOrFindSharedShadowTreeResolver`, `unshareShadowTreeResolverBeforeMutation`, `makeResolverSharingKey`, `clearResolver`, `clearViewTransitionStyles`, `releaseMemory`, `analyzeStyleSheetChange`, `updateResolver`, `counterStyleRegistry`, the resolver half of `scheduleUpdate`/`updateActiveStyleSheets`) | `port` | `Style::Scope` | on | tested. |
| `css/CSSCounterStyle.h`, `.cpp`, `css/CSSCounterStyleRegistry.h`, `.cpp` | `have*` | `CSS/Counter/CounterStyle.hpp`, `CounterStyleRegistry.hpp`, `src/...` | on | tested. Old headers were raw WebKit with no `.cpp`; rewritten. |
| `wtf/BloomFilter.h` | `port` | Krystal.Core `Types/BloomFilter.hpp` | on (Core) | tested (Core). No SHA1-digest or string overloads. |
| `wtf/text/TextBreakIterator.h` (`numGraphemeClusters`) | `port` | `NumGraphemeClusters` (`Core/TextBreakIterator.hpp`, `src/...`) | on | Through counter style pads. ICU's character break iterator. |
| `wtf/unicode/CharacterNames.h` (the four small triangles) | `port` | Krystal.Core `Text/Unicode.hpp` | on (Core) | |
| `style/StyleScrollbarState.h` | `wont` | | | The scrollbar pseudo-classes and pseudo-elements were dropped (d1a). |
| `style/InspectorCSSOMWrappers.h` | `wont` | | | Inspector. |

**Names:** `Relation::type` → `RelationType` (the member would clash with the `Type` enum). `RuleData::styleRule()`/`selector()` keep their names; `matchBasedOnRuleHash()` → `GetMatchBasedOnRuleHash()`, `propertyAllowlist()` → `GetPropertyAllowlist()` (they'd hide the types). `RuleSet::viewTransitionRule()`, `ScopeRuleSets::viewTransitionRule()` and `Resolver::viewTransitionRule()` → `GetViewTransitionRule()`; `ScopeRuleSets::selectorsForStyleAttribute()` → `GetSelectorsForStyleAttribute()`; `Resolver::scopeType()` → `GetScopeType()`; `Scope::resolver()` → `GetResolver()`, `counterStyleRegistry()` → `GetCounterStyleRegistry()`. `DynamicMediaQueryEvaluationChanges::type` → `ChangeType`. `CSSCounterStyle` → `css::CounterStyle`, `CSSCounterStyleRegistry` → `css::CounterStyleRegistry`, `counterStyle(name, map)` → `GetCounterStyle`. `HasSelectorFilter::type()` → `GetType()`. The user agent sheet arrays (`htmlUserAgentStyleSheet`, ...) are `css::HTMLUserAgentStyleSheet` (a `CSSOMStringView`) and so on. `OptionSet<UsedRuleType>` → `EnumSet<UsedRuleType>` (index enumerators). Identifiers are hashed for the bloom filters with `std::hash<StringAtom>` (upstream: the atom's string hash), kept non-zero.

**Bugs fixed (old Krystal headers):** `Style/UserAgentStyle.hpp` declared the rule sets as `css::RuleSet` and the style version as `size_t`, and had no `.cpp`. `CSS/Counter/CounterStyle.hpp` and `CounterStyleRegistry.hpp` were raw WebKit (`String`, `OptionSet`, `<WebCore/...>` includes, allocator macros, `m_` members) and had never compiled.

**Upstream bugs found:**
- `RuleData::m_propertyAllowlist` has 2 bits, but with `ENABLE(VIDEO)` `PropertyAllowlist` has five values, so `CueBackground` (4) is stored as `None`. Krystal uses 3 bits (it doesn't change behaviour yet: nothing returns `CueBackground`, see differences).

**User agent sheets:** `codegen/user-agent-style-sheets/` holds `html.css`, `quirks.css`, `svg.css`, `mathml.css`, `mathmlLegacyFontSizeMath.css`, `counterStyles.css`, `popover.css`, `viewTransitions.css` and `horizontalFormControls.css`, as preprocessed for a non-Apple platform with MathML and fullscreen enabled (upstream runs them through the C preprocessor). From those copies: the style rules whose every selector uses a non-standard pseudo-class or pseudo-element (`-webkit-`, `-internal-`, `-apple-`, which the selector parser rejects since d1a) are removed: in `html.css` the rules for `:-internal-media-document`, `:-internal-html-document`, `::-internal-writing-suggestions`, `::-internal-loading-auto-fill-button`, `::-internal-color-swatch-overlay`, every `::-webkit-` form control part (text field decoration, search buttons, date and time fields, spin button, the auto-fill buttons, caps lock indicator, list button, slider, colour swatch, validation bubble, progress) and `:-webkit-autofill*`; in `fullscreen.css` the `:-internal-fullscreen-document` and `:-internal-in-window-fullscreen` rules. Rules left empty by the preprocessing (seven iOS-only `input` checkbox and radio rules in `html.css`) are removed too. Declarations are kept as upstream: the `-webkit-` properties and keywords the sheets use are the ones Krystal kept (`-webkit-link`, `-webkit-focus-ring-color`, `-internal-th-center`, `-webkit-user-select`, ...). The codegen step minifies each sheet as `make-css-file-arrays.pl` does (comments stripped, whitespace collapsed) into a byte array. Not embedded, because nothing loads them with default settings: `htmlSwitchControl.css` (SwitchControlEnabled is off), `mathmlCoreExtras.css` (CoreMathMLEnabled), `mathmlFontSizeMath.css` (CSSMathDepthEnabled) and `fullscreen.css` (no Fullscreen API). `themeAdwaita.css` is a GTK theme (`wont`).

**Differences:**
- No style resolution yet: the resolver has no `styleForElement`, `styleForPseudoElement`, `styleForKeyframe`, `styleForPage`, `defaultStyleForElement`, `keyframeStylesForAnimation`, `keyframeRulesForName` (needs `TimingFunction`), `setViewTransitionStyles`, `styleRulesForElement`, the matched declarations cache or `invalidateMatchedDeclarationsCache`. Its root default style is a default `RenderStyle` (FIXME, as `MediaQueryMatcher` in e7), and its media type comes from the `MediaEnvironment` (upstream: the frame's view).
- `Style::Scope` updates the resolver but doesn't invalidate style after a sheet change (FIXME, f6), and doesn't invalidate `:host`/`::slotted` style on a contents change. No custom property registry (FIXME, f4), match result cache (f5), font selector, SVG font face rules or render view font update (FIXMEs). Shared shadow tree resolvers evict the first entry when there are more than 256 (upstream: a random one). No user agent shadow roots, so `isForUserAgentShadowTree` is false.
- `ScopeRuleSets` has no user style sheets (`initializeUserStyle` adds nothing; FIXME: extension style sheets) and no inspector wrappers. Its invalidation rule set builders are kept in first-seen order (upstream: a hash map's order).
- `RuleSetBuilder` keeps a dynamic media query's affected rules in first-seen order (upstream: a hash set copied to a vector). Anonymous cascade layers get a random `anon_` name from `std::mt19937_64` (upstream: a cryptographically random number).
- `RuleSet` doesn't clone `::cue` rules for the cue background box (`::-internal-cue-background` was dropped with the internal pseudos) and doesn't skip SVG animatable attributes when bucketing attribute selectors (no SMIL; FIXMEs).
- `UserAgentStyle` doesn't add the render theme's extra sheet. Settings use upstream's defaults: popovers and view transitions on, vertical form controls, MathML Core, `math-depth` and the switch control off. The fullscreen sheet is never loaded (no Fullscreen API).
- `HasSelectorFilter` doesn't add hovered elements' salted hashes (no hover state). `SelectorMatchingState` has no `ContainerQueryEvaluationState` (f5).
- `CounterStyleRegistry::resolvedCounterStyle` takes a name (upstream: a `Style::CounterStyle`, which comes with f3). `CounterStyle` drops upstream's unused members (`m_fallbackName`, `m_extendsReference`, `m_isExtendedUnresolved`) and the declared-but-undefined `usesNegativeSign` and `extractDataFromSystemDescriptor`. A representation the system can't produce is `Maybe` empty (upstream: a null `String`).
- `RuleFeatureSet::Clear` doesn't reset `UsedMatchElements`, as upstream.
- In author sheets, `spelling-error`/`grammar-error` (`text-decoration-line`), `@view-transition` and `@position-try` are behind settings that are off without `Settings` (FIXME in the tests; noted under *Open*).

**Seam changes:** the `style::Resolver` placeholder is removed; `CSSStyleSheet`'s keyframes path now calls the real resolver. `DocumentKeyframesRuleDidChange` stays (a no-op until animations).

**Also:** `Element::ClassNames()` (upstream `classNames()`: the class list, deduplicated, ASCII-lowercased in quirks mode). `css::Rule` allows compact pointers (`KRYS_ENABLE_COMPACT_POINTERS`, for `RuleData`'s `CompactRefPtrTuple`). `CSSStyleRule` and `StyleSheetContents` use `RuleData::MaximumSelectorComponentCount` (were FIXMEs).

Tests: `tests/Style/RuleSet.cpp` (buckets, rule data, cascade layers, conditional rules, nesting and `@scope`, features, resolver mutating rules), `tests/Style/UserAgentStyle.cpp` (every rule of every embedded sheet parses, sheets added per element, view transition keyframes), `tests/CSS/Counter/CounterStyle.cpp` (predefined and author counter styles: every system, ranges, fallbacks, pads, negatives, extends and cycles), `tests/CSS/Selectors/SelectorFilter.cpp` (selector hashes, ancestor filtering, quirks mode, `:has()` filters, the `:has()` match cache, style relations), `tests/Style/Resolver.cpp` (the document's resolver, additive/reset/reconstruct updates, at-rules, shared shadow tree resolvers, dynamic media queries): 21 cases / 663 assertions pass; Krystal.Core `tests/Types/BloomFilter.cpp`: 3 cases / 22 assertions. Full Booey suite: 1810 cases / 682254 assertions; all pass except 7 expected failures. Whole solution builds with no warnings.

### Layer f1: matching (2026-10-14)

Graphs from the five f1 entries (headers-only): `style/ElementRuleCollector.cpp` (91 files), `MatchResult.h` (59), `MatchResultCache.cpp` (63), `PageRuleCollector.cpp` (63) and `PseudoElementRequest.h` (3) (`tools/port/out/f1-*.md`). Nothing new beyond f0's reach: the extra files come through `ContainerQueryEvaluator`, `ResolvedStyle` and `RenderStyle`, which are later layers. New outside the plan: `TreeQueries::GetShadowRoot` and `TreeQueries::ParentOrShadowHostElement` (DOM internals the collector needs).

**Decisions (2026-10-14):**
- `MatchResultCache` moves to f5: it caches the resolved style (`UnadjustedStyle`, a `RenderStyle` clone) and returns the cascade's `IncludedProperties`, so it needs f2, f3 and the resolver.
- Rules inside `@container` never match until there is a container query evaluator (f5): every container query is unknown, as when the element has no eligible query container (FIXME).

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/PseudoElementRequest.h` | `port` | `Style/PseudoElementRequest.hpp` | on | tested. No scrollbar state (the scrollbar pseudo-elements were dropped in d1a). |
| `style/MatchResult.h` | `port` | `Style/MatchResult.hpp` | on | tested. |
| `style/ElementRuleCollector.h`, `.cpp` | `port` | `Style/ElementRuleCollector.hpp`, `src/...` | on | tested. See differences. |
| `style/PageRuleCollector.h`, `.cpp` | `port` | `Style/PageRuleCollector.hpp`, `src/...` | on | tested. |
| `style/MatchResultCache.h`, `.cpp` | `port` (f5) | | | Moved to f5. |
| `cssjit/SelectorCompiler.h` | `wont` | | | The selector JIT; the collector always uses `SelectorChecker`. |
| `dom/ElementTextDirection.h` (`computeTextDirectionIfDirIsAuto`) | `port` (partial) | `ComputeTextDirectionIfDirIsAuto` (`src/Style/ElementRuleCollector.cpp`) | on | tested. Auto directionality is always `ltr` (FIXME, as `:dir()` in e6). |

**Names:** `MatchedProperties`/`MatchResult`/`MatchedRule` members are PascalCase; the members that share their type's name (`FromStyleAttribute`, `IsCacheable`, `CascadeLayerPriority`, `RuleData`) write the type as `style::X`. `ElementRuleCollector::matchResult()` and `PageRuleCollector::matchResult()` → `GetMatchResult()`, `PseudoElementRequest::type()` → `GetType()` (they'd hide the types). `MatchResult::isEmpty()` → `IsEmpty()`. `add(Hasher&, const MatchedProperties&)` and `add(Hasher&, const MatchResult&)` → `DefaultHash<MatchedProperties>` and `DefaultHash<MatchResult>` (whose `Equal` is `CacheablePropertiesEqual`). Upstream's `Element::isLink()` → `css::IsLink`, `parentOrShadowHostElement()` → `TreeQueries::ParentOrShadowHostElement`, the internal `shadowRoot()` → `TreeQueries::GetShadowRoot`, the internal `assignedSlot()` → `SlotAlgorithms::GetAssignedSlot`.

**Bugs fixed (DOM, found by the collector's tests):**
- `ElementAlgorithms::AttachShadowRoot` never set the new shadow root's mode, so every shadow root was open: `attachShadow({mode: "closed"})` was exposed through `shadowRoot`.
- Internal code used the web-exposed getters (`Element::ShadowRoot()`, `Element::AssignedSlot()`), which hide closed shadow roots; hidden by the first bug, it breaks once roots can be closed: `SlotAlgorithms::FindSlot` (slottables in closed roots got no slot), named slot assignment on insertion (dereferenced null), cloning (clonable closed roots weren't cloned), `TreeQueries::NextShadowIncluding` (skipped closed roots), and in style code `SelectorChecker` (`:host`, `::slotted()`) and `AssignedSlotForScopeOrdinal`. They now use `TreeQueries::GetShadowRoot` and `SlotAlgorithms::GetAssignedSlot`.

**Differences:**
- No user agent shadow trees, so no user agent part rules, `::cue` rules, or user agent parts matching as their host (FIXMEs). No selector JIT.
- No presentational hints or the table's additional presentational hint style (FIXME, f5); no SMIL animated style (FIXME).
- `dir=auto` adds a `direction: ltr` declaration (upstream: the auto directionality, which needs bidi data; FIXME).
- Focus and focus-visible rules aren't collected (documents have no focused area; FIXME). Named view transition pseudo-elements get no captured class list (no active view transition; FIXME).
- `@container` rules never match (see decisions). Part mappings are empty (no `exportparts`; FIXME), so `::part()` rules only come from the host's scope.
- A partially cacheable declaration block hashes as 0 (upstream adds nothing to the hasher).
- Attributes are read with `AttributeList()`, which synchronises the `style` attribute first (upstream: `attributesWithoutUpdate`).

Tests: `tests/Style/ElementRuleCollector.cpp` (cascade order by specificity and position, empty rules, author-only matching, a separate rule set, cascade layers, inline style and its cacheability, quirks and link user agent rules, `dir=auto`, `:host`/`::slotted()`/`::part()` in open and closed shadow roots, `@scope` proximity, limits, implicit roots and `:scope`, `@container`, requested pseudo-elements and highlights, keyframes, the selector filter and relations, `MatchResult` cacheability and hashing, `@page` rules for left/right/first pages in both directions): 10 cases / 388 assertions pass; `tests/DOM/Algorithms/TreeQueries.cpp`: 2 new cases (`GetShadowRoot` in both modes, `ParentOrShadowHostElement`). Full Booey suite: 1822 cases / 682657 assertions; all pass except 7 expected failures. Krystal.Core: 557 cases pass. Whole solution builds with no warnings.

### Layer f2: the cascade (2026-10-14)

Graphs from the six planned entries (headers-only): `style/PropertyCascade.cpp` (121 files), `StyleCustomProperty.cpp` (116), `StyleCustomPropertyRegistry.cpp` (116), `css/CSSVariableReferenceValue.cpp` (117), `CSSCustomPropertyValue.cpp` (33) and `CSSRegisteredCustomProperty.cpp` (67) (`tools/port/out/f2-*.md`). The reach is through `StyleBuilderState.h`, `RenderStyle` and the computed value types, which are later layers. New outside the plan: `AnimatableCSSProperty` (`animation/WebAnimationTypes.h`, the type only).

**Decisions (2026-10-14):**
- f2 is the cascade only. `var()`, `revert` and `revert-layer` are resolved by `Style::Builder` (`applyCustomProperty`, `ensureRollbackCascadeFor*`), so their resolution moves to f4; the rollback cascades themselves are the cascade's and are tested here. `Style::CustomProperty` holds computed value types (`ImageWrapper`, `TransformFunction`, `URL`, `CustomIdentifier`, `String`), so it moves to f3. `CustomPropertyRegistry` (and `CSSRegisteredCustomProperty`, `@property` registration) needs the render style's custom property storage and the typed custom property parsing, so it moves to f4.
- `AnimatableCSSProperty` is ported now (the alias, not the rest of `WebAnimationTypes.h`), so the cascade keeps its animation layer and the after-animation and after-transition passes are tested.

| WebKit | Verdict | Krystal | CMake | Notes |
|---|---|---|---|---|
| `style/PropertyCascade.h`, `.cpp` | `port` | `Style/PropertyCascade.hpp`, `src/...` | on | tested. See differences. |
| `animation/WebAnimationTypes.h` (`AnimatableCSSProperty`) | `port` (partial) | `Animation/WebAnimationTypes.hpp` | on | tested (through the cascade). The type only. |
| `style/StyleCustomProperty.h`, `.cpp` | `port` (f3) | | | Moved to f3. |
| `style/StyleCustomPropertyRegistry.h`, `.cpp` | `port` (f4) | | | Moved to f4; ported in f4c. |
| `css/CSSRegisteredCustomProperty.h`, `.cpp` | `port` (f4) | | | Moved to f4 (with the registry); ported in f4c. |
| `css/CSSVariableReferenceValue.cpp` (resolution) | `port` (f4) | | | The deferred definitions move to f4 with `Style::Builder`. |
| `css/CSSCustomPropertyValue.h`, `.cpp` | `have` | `CSS/Values/CustomPropertyValue.hpp`, `src/...` | on | Unchanged (reviewed with the values). |

**Names:** `Property` and `IncludedProperties` members are PascalCase; the members that share their type's name (`Origin`, `CascadeLayerPriority`, `FromStyleAttribute`) write the type as `PropertyCascade::Origin`/`style::X`. `customProperty()` → `GetCustomProperty()` (it would hide `style::CustomProperty`, f3). `normalPropertyTypes()`/`startingStylePropertyTypes()`/`normalProperties()` → `NormalPropertyTypes()`/`StartingStylePropertyTypes()`/`NormalProperties()`. `PropertyType` is an `EnumSet` of sequential values (upstream: an `OptionSet` of bit flags).

**Differences:**
- No `PropertyType::BaseAppearanceStyle` or `UsedRuleType::BaseAppearance` (`@-internal-base-appearance` was dropped in d4), and no `addBaseAppearanceStyles` (declared upstream, never defined).
- `Property::CssValue` holds `const css::Value *` (declaration blocks hand out const values); upstream holds `CSSValue *`.
- `PropertyBitSet` is a `std::bitset` (upstream: WTF's `BitSet`), as `Parser.cpp` and `StyleProperties.cpp` already do. `HashSet<AnimatableCSSProperty>` hashes with `std::hash` of the variant (upstream's `DefaultHash`/`HashTraits` specialisations aren't needed by Krystal's `HashSet`).
- `OverriddenAnimatedProperties()` returns by value (upstream: `const` by value).

Tests: `tests/Style/PropertyCascade.cpp` (origins and importance; important declarations across layers, the style attribute and shadow tree scopes; link match states; logical property groups and pair resolution in both directions; custom properties; included property types: inherited, non-inherited, explicitly inherited, ids, non-cacheable and the declarations that may override them, starting style, allowlists; rollback cascades for `revert` and `revert-layer`, with scopes and every property included; the position fallback origin; the after-animation and after-transition passes for animated properties, font size, line height and custom properties; a cascade built from an element's matched rules): 10 cases / 833 assertions pass. Full Booey suite: 1832 cases / 683490 assertions; all pass except 7 expected failures. Krystal.Core: 557 cases pass. Whole solution builds with no warnings.

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
| `css/values/CSSValueTypes.h`, `.cpp` | `have` | `CSS/Values/Types.hpp`, `src/.../Types.cpp`, `src/.../TypesValueCreation.cpp` | on | tested. `TypesValueCreation.cpp` built from layer c1 (2026-10-13; it was left off after the Values slice); see *Layer c1*. |
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
| `css/values/primitives/CSSPrimitiveNumericTypes+CSSValueCreation.h` | `have` | `NumericTypesValueCreation.hpp` | on | tested (through `border-radius`, layer c1). See *Layer c1*. |
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

The unreviewed `Style/AnchorPositionEvaluator.hpp` (raw WebKit) still declares the full class; nothing built includes it (`BuilderState` was ported in f4a). When the style resolver is ported, the real classes replace the seam's and the collapsed members go back to upstream's call chains.

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
| `style/ScopedName.h` | `have*` | `Style/ScopedName.hpp` | on | tested. The struct only: `switchOn` (needs `Style::CustomIdentifier`), `CSSValueConversion`, `MarkableTraits` and `operator<<` not ported. (Completed with f3d1, but `operator<<`.) |
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
| `css/parser/CSSPropertyParserConsumer+Ident.h`, `.cpp` | `have` | `Consumers/Ident.hpp`, `src/.../Ident.cpp` | header on, `.cpp` off | Raw consumers tested. The value-returning `ConsumeIdent`/`ConsumeIdentRange`/`ConsumeCustomIdent`/`ConsumeDashedIdent` moved to `Ident.cpp` (as upstream): `PrimitiveValue::Create(ValueId)` needs `ValuePool`, so it waits for layer 5. `ConsumeIdentUsingMapping`/`PeekIdentUsingMapping` (added in property parser layer b4, tested) take a `ValueId` → `Maybe<T>` callable instead of a `SortedArrayMap`. Extra: `DiscardIdent`. Raw custom/dashed idents return `Maybe<CSSOMString>` (`null` for upstream's null `String`). |
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
| `css/parser/CSSParserFastPaths.h`, `.cpp` (colour half, `lengthValueRangeForPropertiesSupportingSimpleLengths`) | `have` | `CSS/Parser/ParserFastPaths.hpp`, `src/CSS/Parser/ParserFastPaths.cpp` | on | tested. The `.cpp` was missing. UTF-8 only, so no 8-/16-bit split; the WTF span helpers (`skipWhile`, `skipExactly`, …) are local helpers on `CSSOMStringView`. `charactersToDouble` → `std::from_chars` after skipping leading whitespace (same grammar as WTF's fast_float: no leading `+`, accepts `inf`/`nan`). The value half (`maybeParseValue` and the length/transform/display/opacity fast paths) was ported in property parser layer c3. |
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
