# Web font test files

Made from `../fonts/Ahem.ttf` (public domain; see `../fonts/README.md`) with fontTools, for the web font tests. They're kept apart from `fonts/`, whose fonts the tests register as installed fonts.

- `Ahem.woff`, `Ahem.woff2`: Ahem as WOFF and WOFF 2 (`TTFont.flavor`).
- `AhemCFF.otf`: Ahem's glyphs as CFF outlines (family "Ahem CFF"), an OpenType font with PostScript outlines.

The same files are in `Krystal.UI.Tests/golden/scenes/fonts/` for the `webfonts` page golden, and `Ahem.woff2` in `Krystal.Editor/pages/fonts/` for the editor page.
