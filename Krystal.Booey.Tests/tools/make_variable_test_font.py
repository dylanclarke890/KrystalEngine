"""Makes data/webfonts/VariableTest.ttf: a variable font whose axes can be told apart by the width of its glyphs.

Every glyph is a box 1000 units wide (an em) at the default instance, and each axis adds to the width, so the width
of a character at 100px says which values reached the font:

    wght 100..400..900   -500 at 100, +500 at 900
    wdth 50..100..200    -250 at 50, +1000 at 200
    slnt -20..0..0       +200 at -20
    ital 0..0..1         +300 at 1
    opsz 8..14..72       +400 at 72
    TEST 0..0..100       +700 at 100

Needs fontTools: pip install fonttools. Run it from the repository root.
"""
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.ttLib.tables.TupleVariation import TupleVariation

GLYPHS = [".notdef", "space", "a", "b"]
UNITS_PER_EM = 1000
WIDTH = 1000
HEIGHT = 700
AXES = [
    ("wght", 100, 400, 900, "Weight"),
    ("wdth", 50, 100, 200, "Width"),
    ("slnt", -20, 0, 0, "Slant"),
    ("ital", 0, 0, 1, "Italic"),
    ("opsz", 8, 14, 72, "Optical Size"),
    ("TEST", 0, 0, 100, "Test"),
]
# (axis, peak of the region, extra width at that peak).
DELTAS = [
    ("wght", -1.0, -500),
    ("wght", 1.0, 500),
    ("wdth", -1.0, -250),
    ("wdth", 1.0, 1000),
    ("slnt", -1.0, 200),
    ("ital", 1.0, 300),
    ("opsz", 1.0, 400),
    ("TEST", 1.0, 700),
]


def box():
    pen = TTGlyphPen(None)
    pen.moveTo((0, 0))
    pen.lineTo((0, HEIGHT))
    pen.lineTo((WIDTH, HEIGHT))
    pen.lineTo((WIDTH, 0))
    pen.closePath()
    return pen.glyph()


builder = FontBuilder(UNITS_PER_EM, isTTF=True)
builder.setupGlyphOrder(GLYPHS)
builder.setupCharacterMap({0x20: "space", 0x61: "a", 0x62: "b"})
builder.setupGlyf({name: box() if name != "space" else TTGlyphPen(None).glyph() for name in GLYPHS})
builder.setupHorizontalMetrics({name: (WIDTH, 0) for name in GLYPHS})
builder.setupHorizontalHeader(ascent=800, descent=-200)
builder.setupNameTable({"familyName": "Variable Test", "styleName": "Regular"})
builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=800, usWinDescent=200)
builder.setupPost()
builder.setupFvar([(tag, low, default, high, name) for tag, low, default, high, name in AXES], [])

variations = {}
for name in GLYPHS:
    tuples = []
    if name != "space":
        for axis, peak, width in DELTAS:
            region = {axis: (min(peak, 0.0), peak, max(peak, 0.0))}
            # The box's four points, then the four phantom points: only the right edge and the advance move.
            deltas = [(0, 0), (0, 0), (width, 0), (width, 0), (0, 0), (width, 0), (0, 0), (0, 0)]
            tuples.append(TupleVariation(region, deltas))
    else:
        for axis, peak, width in DELTAS:
            region = {axis: (min(peak, 0.0), peak, max(peak, 0.0))}
            tuples.append(TupleVariation(region, [(0, 0), (width, 0), (0, 0), (0, 0)]))
    variations[name] = tuples
builder.setupGvar(variations)
builder.save("Krystal.Booey.Tests/data/webfonts/VariableTest.ttf")
print("wrote Krystal.Booey.Tests/data/webfonts/VariableTest.ttf")
