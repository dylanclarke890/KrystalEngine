"""Makes data/webfonts/FeatureTest.ttf and data/webfonts/PaletteTest.ttf.

FeatureTest.ttf: features can be told apart by the width of "a". Its glyphs are boxes 700 units high, and "a" and
"b" are 1000 units (an em) wide. The features substitute wider boxes for "a":

    ss01         2000 units
    ss02         3000 units
    salt 1, 2    4000, 5000 units (an alternate set)
    cv01         6000 units

PaletteTest.ttf: "a" is a colour glyph (COLR version 0) of two boxes: the whole em in colour 0, and a box in its
middle (250..750 across, 50..550 up) in colour 1. Its palettes (CPAL) are:

    0                      #FF0000, #00FF00
    1 (light background)   #0000FF, #FFFF00
    2 (dark background)    #800080, #00FFFF

Needs fontTools: pip install fonttools. Run it from the repository root.
"""
from fontTools.feaLib.builder import addOpenTypeFeaturesFromString
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

UNITS_PER_EM = 1000


def box(left, bottom, right, top):
    pen = TTGlyphPen(None)
    pen.moveTo((left, bottom))
    pen.lineTo((left, top))
    pen.lineTo((right, top))
    pen.lineTo((right, bottom))
    pen.closePath()
    return pen.glyph()


def empty():
    return TTGlyphPen(None).glyph()


def start(family, glyphs, widths, outlines):
    builder = FontBuilder(UNITS_PER_EM, isTTF=True)
    builder.setupGlyphOrder(glyphs)
    builder.setupCharacterMap({0x20: "space", 0x61: "a", 0x62: "b"})
    builder.setupGlyf(outlines)
    # The left side bearing is the outline's left edge.
    builder.setupHorizontalMetrics({name: (widths[name], getattr(outlines[name], "xMin", 0)) for name in glyphs})
    builder.setupHorizontalHeader(ascent=800, descent=-200)
    builder.setupNameTable({"familyName": family, "styleName": "Regular"})
    builder.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=800, usWinDescent=200)
    builder.setupPost()
    return builder


def feature_font():
    widths = {".notdef": 1000, "space": 1000, "a": 1000, "b": 1000, "a.ss01": 2000, "a.ss02": 3000,
              "a.salt1": 4000, "a.salt2": 5000, "a.cv01": 6000}
    glyphs = list(widths)
    outlines = {name: empty() if name == "space" else box(0, 0, width, 700) for name, width in widths.items()}
    builder = start("Feature Test", glyphs, widths, outlines)
    addOpenTypeFeaturesFromString(builder.font, """
        feature ss01 { sub a by a.ss01; } ss01;
        feature ss02 { sub a by a.ss02; } ss02;
        feature salt { sub a from [a.salt1 a.salt2]; } salt;
        feature cv01 { sub a by a.cv01; } cv01;
    """)
    builder.save("Krystal.Booey.Tests/data/webfonts/FeatureTest.ttf")


def palette_font():
    widths = {".notdef": 1000, "space": 1000, "a": 1000, "b": 1000, "whole": 1000, "middle": 1000}
    glyphs = list(widths)
    outlines = {".notdef": box(0, 0, 1000, 700), "space": empty(), "a": box(0, -200, 1000, 800),
                "b": box(0, 0, 1000, 700), "whole": box(0, -200, 1000, 800), "middle": box(250, 50, 750, 550)}
    builder = start("Palette Test", glyphs, widths, outlines)
    builder.setupCOLR({"a": [("whole", 0), ("middle", 1)]}, version=0)

    def rgba(hex_colour):
        value = int(hex_colour, 16)
        return ((value >> 16) / 255, ((value >> 8) & 0xFF) / 255, (value & 0xFF) / 255, 1.0)

    builder.setupCPAL(
        [[rgba("FF0000"), rgba("00FF00")], [rgba("0000FF"), rgba("FFFF00")], [rgba("800080"), rgba("00FFFF")]],
        paletteTypes=[0, 1, 2])
    builder.save("Krystal.Booey.Tests/data/webfonts/PaletteTest.ttf")


feature_font()
palette_font()
