#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include <cassert>

namespace Krys::HTML::ElementNames
{
  namespace HTML
  {
    inline DOMStringAtom a = u8"a";
    inline DOMStringAtom abbr = u8"abbr";
    inline DOMStringAtom acronym = u8"acronym";
    inline DOMStringAtom address = u8"address";
    inline DOMStringAtom applet = u8"applet";
    inline DOMStringAtom area = u8"area";
    inline DOMStringAtom article = u8"article";
    inline DOMStringAtom aside = u8"aside";
    inline DOMStringAtom audio = u8"audio";
    inline DOMStringAtom attachment = u8"attachment";
    inline DOMStringAtom b = u8"b";
    inline DOMStringAtom base = u8"base";
    inline DOMStringAtom basefont = u8"basefont";
    inline DOMStringAtom bdi = u8"bdi";
    inline DOMStringAtom bdo = u8"bdo";
    inline DOMStringAtom bgsound = u8"bgsound";
    inline DOMStringAtom big = u8"big";
    inline DOMStringAtom blink = u8"blink";
    inline DOMStringAtom blockquote = u8"blockquote";
    inline DOMStringAtom body = u8"body";
    inline DOMStringAtom br = u8"br";
    inline DOMStringAtom button = u8"button";
    inline DOMStringAtom canvas = u8"canvas";
    inline DOMStringAtom caption = u8"caption";
    inline DOMStringAtom center = u8"center";
    inline DOMStringAtom cite = u8"cite";
    inline DOMStringAtom code = u8"code";
    inline DOMStringAtom col = u8"col";
    inline DOMStringAtom colgroup = u8"colgroup";
    inline DOMStringAtom data = u8"data";
    inline DOMStringAtom datalist = u8"datalist";
    inline DOMStringAtom dd = u8"dd";
    inline DOMStringAtom del = u8"del";
    inline DOMStringAtom details = u8"details";
    inline DOMStringAtom dfn = u8"dfn";
    inline DOMStringAtom dialog = u8"dialog";
    inline DOMStringAtom dir = u8"dir";
    inline DOMStringAtom div = u8"div";
    inline DOMStringAtom dl = u8"dl";
    inline DOMStringAtom dt = u8"dt";
    inline DOMStringAtom em = u8"em";
    inline DOMStringAtom embed = u8"embed";
    inline DOMStringAtom fieldset = u8"fieldset";
    inline DOMStringAtom figcaption = u8"figcaption";
    inline DOMStringAtom figure = u8"figure";
    inline DOMStringAtom font = u8"font";
    inline DOMStringAtom footer = u8"footer";
    inline DOMStringAtom form = u8"form";
    inline DOMStringAtom frame = u8"frame";
    inline DOMStringAtom frameset = u8"frameset";
    inline DOMStringAtom h1 = u8"h1";
    inline DOMStringAtom h2 = u8"h2";
    inline DOMStringAtom h3 = u8"h3";
    inline DOMStringAtom h4 = u8"h4";
    inline DOMStringAtom h5 = u8"h5";
    inline DOMStringAtom h6 = u8"h6";
    inline DOMStringAtom head = u8"head";
    inline DOMStringAtom header = u8"header";
    inline DOMStringAtom hgroup = u8"hgroup";
    inline DOMStringAtom hr = u8"hr";
    inline DOMStringAtom html = u8"html";
    inline DOMStringAtom i = u8"i";
    inline DOMStringAtom iframe = u8"iframe";
    inline DOMStringAtom image = u8"image";
    inline DOMStringAtom img = u8"img";
    inline DOMStringAtom input = u8"input";
    inline DOMStringAtom ins = u8"ins";
    inline DOMStringAtom isindex = u8"isindex";
    inline DOMStringAtom kbd = u8"kbd";
    inline DOMStringAtom keygen = u8"keygen";
    inline DOMStringAtom label = u8"label";
    inline DOMStringAtom legend = u8"legend";
    inline DOMStringAtom li = u8"li";
    inline DOMStringAtom link = u8"link";
    inline DOMStringAtom listing = u8"listing";
    inline DOMStringAtom main = u8"main";
    inline DOMStringAtom map = u8"map";
    inline DOMStringAtom mark = u8"mark";
    inline DOMStringAtom marquee = u8"marquee";
    inline DOMStringAtom menu = u8"menu";
    inline DOMStringAtom meta = u8"meta";
    inline DOMStringAtom meter = u8"meter";
    inline DOMStringAtom model = u8"model";
    inline DOMStringAtom multicol = u8"multicol";
    inline DOMStringAtom nav = u8"nav";
    inline DOMStringAtom nextid = u8"nextid";
    inline DOMStringAtom nobr = u8"nobr";
    inline DOMStringAtom noembed = u8"noembed";
    inline DOMStringAtom noframes = u8"noframes";
    inline DOMStringAtom object = u8"object";
    inline DOMStringAtom ol = u8"ol";
    inline DOMStringAtom optgroup = u8"optgroup";
    inline DOMStringAtom option = u8"option";
    inline DOMStringAtom output = u8"output";
    inline DOMStringAtom p = u8"p";
    inline DOMStringAtom param = u8"param";
    inline DOMStringAtom picture = u8"picture";
    inline DOMStringAtom plaintext = u8"plaintext";
    inline DOMStringAtom pre = u8"pre";
    inline DOMStringAtom progress = u8"progress";
    inline DOMStringAtom q = u8"q";
    inline DOMStringAtom rb = u8"rb";
    inline DOMStringAtom rp = u8"rp";
    inline DOMStringAtom rt = u8"rt";
    inline DOMStringAtom rtc = u8"rtc";
    inline DOMStringAtom ruby = u8"ruby";
    inline DOMStringAtom s = u8"s";
    inline DOMStringAtom samp = u8"samp";
    inline DOMStringAtom script = u8"script";
    inline DOMStringAtom search = u8"search";
    inline DOMStringAtom section = u8"section";
    inline DOMStringAtom select = u8"select";
    inline DOMStringAtom selectedcontent = u8"selectedcontent";
    inline DOMStringAtom slot = u8"slot";
    inline DOMStringAtom small = u8"small";
    inline DOMStringAtom source = u8"source";
    inline DOMStringAtom spacer = u8"spacer";
    inline DOMStringAtom span = u8"span";
    inline DOMStringAtom strike = u8"strike";
    inline DOMStringAtom strong = u8"strong";
    inline DOMStringAtom style = u8"style";
    inline DOMStringAtom sub = u8"sub";
    inline DOMStringAtom summary = u8"summary";
    inline DOMStringAtom sup = u8"sup";
    inline DOMStringAtom table = u8"table";
    inline DOMStringAtom tbody = u8"tbody";
    inline DOMStringAtom td = u8"td";
    inline DOMStringAtom template_ = u8"template"; // conflicts with template keyword
    inline DOMStringAtom textarea = u8"textarea";
    inline DOMStringAtom tfoot = u8"tfoot";
    inline DOMStringAtom th = u8"th";
    inline DOMStringAtom thead = u8"thead";
    inline DOMStringAtom time = u8"time";
    inline DOMStringAtom title = u8"title";
    inline DOMStringAtom tr = u8"tr";
    inline DOMStringAtom track = u8"track";
    inline DOMStringAtom tt = u8"tt";
    inline DOMStringAtom u = u8"u";
    inline DOMStringAtom ul = u8"ul";
    inline DOMStringAtom var = u8"var";
    inline DOMStringAtom video = u8"video";
    inline DOMStringAtom wbr = u8"wbr";
    inline DOMStringAtom xmp = u8"xmp";
    inline DOMStringAtom noscript = u8"noscript";
  }

  namespace SVG
  {
    inline DOMStringAtom svg = u8"svg";
  }

  namespace MathML
  {
    inline DOMStringAtom math = u8"math";
    inline DOMStringAtom annotation_xml = u8"annotation-xml";
  }
}

namespace Krys::HTML
{
  enum class ElementName : uint8
  {
    Unknown,
    // HTML elements
    a,
    abbr,
    acronym,
    address,
    applet,
    area,
    article,
    aside,
    audio,
    attachment,
    b,
    base,
    basefont,
    bdi,
    bdo,
    bgsound,
    big,
    blink,
    blockquote,
    body,
    br,
    button,
    canvas,
    caption,
    center,
    cite,
    code,
    col,
    colgroup,
    data,
    datalist,
    dd,
    del,
    details,
    dfn,
    dialog,
    dir,
    div,
    dl,
    dt,
    em,
    embed,
    fieldset,
    figcaption,
    figure,
    font,
    footer,
    form,
    frame,
    frameset,
    h1,
    h2,
    h3,
    h4,
    h5,
    h6,
    head,
    header,
    hgroup,
    hr,
    html,
    i,
    iframe,
    image,
    img,
    input,
    ins,
    isindex,
    kbd,
    keygen,
    label,
    legend,
    li,
    link,
    listing,
    main,
    map,
    mark,
    marquee,
    menu,
    meta,
    meter,
    model,
    multicol,
    nav,
    nextid,
    nobr,
    noembed,
    noframes,
    object,
    ol,
    optgroup,
    option,
    output,
    p,
    param,
    picture,
    plaintext,
    pre,
    progress,
    q,
    rb,
    rp,
    rt,
    rtc,
    ruby,
    s,
    samp,
    script,
    search,
    section,
    select,
    selectedcontent,
    slot,
    small,
    source,
    spacer,
    span,
    strike,
    strong,
    style,
    sub,
    summary,
    sup,
    table,
    tbody,
    td,
    template_, // conflicts with template keyword
    textarea,
    tfoot,
    th,
    thead,
    time,
    title,
    tr,
    track,
    tt,
    u,
    ul,
    var,
    video,
    wbr,
    xmp,
    noscript,

    // SVG elements
    color_profile,
    font_face,
    font_face_format,
    font_face_name,
    font_face_src,
    font_face_uri,
    missing_glyph,

    // MathML elements
    annotation_xml,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ElementName, 153u);

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr ElementName ParseElementName(DOMStringView tagName) noexcept
  {
    assert(!tagName.empty());

    switch (tagName.size())
    {
      case 1:
      {
        switch (tagName[0])
        {
          case u8'a':
          {
            return ElementName::a;
          }
          case u8'b':
          {
            return ElementName::b;
          }
          case u8'i':
          {
            return ElementName::i;
          }
          case u8'p':
          {
            return ElementName::p;
          }
          case u8'q':
          {
            return ElementName::q;
          }
          case u8's':
          {
            return ElementName::s;
          }
          case u8'u':
          {
            return ElementName::u;
          }
        }
        break;
      }
      case 2:
      {
        if (tagName == u8"br")
        {
          return ElementName::br;
        }
        if (tagName == u8"dd")
        {
          return ElementName::dd;
        }
        if (tagName == u8"dl")
        {
          return ElementName::dl;
        }
        if (tagName == u8"dt")
        {
          return ElementName::dt;
        }
        if (tagName == u8"em")
        {
          return ElementName::em;
        }
        if (tagName == u8"hr")
        {
          return ElementName::hr;
        }
        if (tagName == u8"h1")
        {
          return ElementName::h1;
        }
        if (tagName == u8"h2")
        {
          return ElementName::h2;
        }
        if (tagName == u8"h3")
        {
          return ElementName::h3;
        }
        if (tagName == u8"h4")
        {
          return ElementName::h4;
        }
        if (tagName == u8"h5")
        {
          return ElementName::h5;
        }
        if (tagName == u8"h6")
        {
          return ElementName::h6;
        }
        if (tagName == u8"li")
        {
          return ElementName::li;
        }
        if (tagName == u8"ol")
        {
          return ElementName::ol;
        }
        if (tagName == u8"ul")
        {
          return ElementName::ul;
        }
        if (tagName == u8"rb")
        {
          return ElementName::rb;
        }
        if (tagName == u8"rp")
        {
          return ElementName::rp;
        }
        if (tagName == u8"rt")
        {
          return ElementName::rt;
        }
        if (tagName == u8"td")
        {
          return ElementName::td;
        }
        if (tagName == u8"th")
        {
          return ElementName::th;
        }
        if (tagName == u8"tr")
        {
          return ElementName::tr;
        }
        if (tagName == u8"tt")
        {
          return ElementName::tt;
        }
        if (tagName == u8"ul")
        {
          return ElementName::ul;
        }
        break;
      }
      case 3:
      {
        if (tagName == u8"bdi")
        {
          return ElementName::bdi;
        }
        if (tagName == u8"bdo")
        {
          return ElementName::bdo;
        }
        if (tagName == u8"big")
        {
          return ElementName::big;
        }
        if (tagName == u8"col")
        {
          return ElementName::col;
        }
        if (tagName == u8"del")
        {
          return ElementName::del;
        }
        if (tagName == u8"dfn")
        {
          return ElementName::dfn;
        }
        if (tagName == u8"dir")
        {
          return ElementName::dir;
        }
        if (tagName == u8"div")
        {
          return ElementName::div;
        }
        if (tagName == u8"img")
        {
          return ElementName::img;
        }
        if (tagName == u8"ins")
        {
          return ElementName::ins;
        }
        if (tagName == u8"kbd")
        {
          return ElementName::kbd;
        }
        if (tagName == u8"map")
        {
          return ElementName::map;
        }
        if (tagName == u8"nav")
        {
          return ElementName::nav;
        }
        if (tagName == u8"pre")
        {
          return ElementName::pre;
        }
        if (tagName == u8"rtc")
        {
          return ElementName::rtc;
        }
        if (tagName == u8"sub")
        {
          return ElementName::sub;
        }
        if (tagName == u8"sup")
        {
          return ElementName::sup;
        }
        if (tagName == u8"var")
        {
          return ElementName::var;
        }
        if (tagName == u8"wbr")
        {
          return ElementName::wbr;
        }
        if (tagName == u8"xmp")
        {
          return ElementName::xmp;
        }
        break;
      }
      case 4:
      {
        if (tagName == u8"abbr")
        {
          return ElementName::abbr;
        }
        if (tagName == u8"area")
        {
          return ElementName::area;
        }
        if (tagName == u8"base")
        {
          return ElementName::base;
        }
        if (tagName == u8"body")
        {
          return ElementName::body;
        }
        if (tagName == u8"cite")
        {
          return ElementName::cite;
        }
        if (tagName == u8"code")
        {
          return ElementName::code;
        }
        if (tagName == u8"data")
        {
          return ElementName::data;
        }
        if (tagName == u8"font")
        {
          return ElementName::font;
        }
        if (tagName == u8"form")
        {
          return ElementName::form;
        }
        if (tagName == u8"head")
        {
          return ElementName::head;
        }
        if (tagName == u8"html")
        {
          return ElementName::html;
        }
        if (tagName == u8"link")
        {
          return ElementName::link;
        }
        if (tagName == u8"main")
        {
          return ElementName::main;
        }
        if (tagName == u8"mark")
        {
          return ElementName::mark;
        }
        if (tagName == u8"menu")
        {
          return ElementName::menu;
        }
        if (tagName == u8"meta")
        {
          return ElementName::meta;
        }
        if (tagName == u8"nobr")
        {
          return ElementName::nobr;
        }
        if (tagName == u8"ruby")
        {
          return ElementName::ruby;
        }
        if (tagName == u8"samp")
        {
          return ElementName::samp;
        }
        if (tagName == u8"slot")
        {
          return ElementName::slot;
        }
        if (tagName == u8"span")
        {
          return ElementName::span;
        }
        if (tagName == u8"time")
        {
          return ElementName::time;
        }
        break;
      }
      case 5:
      {
        if (tagName == u8"aside")
        {
          return ElementName::aside;
        }
        if (tagName == u8"audio")
        {
          return ElementName::audio;
        }
        if (tagName == u8"blink")
        {
          return ElementName::blink;
        }
        if (tagName == u8"embed")
        {
          return ElementName::embed;
        }
        if (tagName == u8"frame")
        {
          return ElementName::frame;
        }
        if (tagName == u8"image")
        {
          return ElementName::image;
        }
        if (tagName == u8"input")
        {
          return ElementName::input;
        }
        if (tagName == u8"label")
        {
          return ElementName::label;
        }
        if (tagName == u8"meter")
        {
          return ElementName::meter;
        }
        if (tagName == u8"model")
        {
          return ElementName::model;
        }
        if (tagName == u8"param")
        {
          return ElementName::param;
        }
        if (tagName == u8"small")
        {
          return ElementName::small;
        }
        if (tagName == u8"style")
        {
          return ElementName::style;
        }
        if (tagName == u8"table")
        {
          return ElementName::table;
        }
        if (tagName == u8"tbody")
        {
          return ElementName::tbody;
        }
        if (tagName == u8"tfoot")
        {
          return ElementName::tfoot;
        }
        if (tagName == u8"thead")
        {
          return ElementName::thead;
        }
        if (tagName == u8"title")
        {
          return ElementName::title;
        }
        if (tagName == u8"track")
        {
          return ElementName::track;
        }
        if (tagName == u8"video")
        {
          return ElementName::video;
        }
        break;
      }
      case 6:
      {
        if (tagName == u8"applet")
        {
          return ElementName::applet;
        }
        if (tagName == u8"button")
        {
          return ElementName::button;
        }
        if (tagName == u8"canvas")
        {
          return ElementName::canvas;
        }
        if (tagName == u8"center")
        {
          return ElementName::center;
        }
        if (tagName == u8"dialog")
        {
          return ElementName::dialog;
        }
        if (tagName == u8"figure")
        {
          return ElementName::figure;
        }
        if (tagName == u8"footer")
        {
          return ElementName::footer;
        }
        if (tagName == u8"header")
        {
          return ElementName::header;
        }
        if (tagName == u8"hgroup")
        {
          return ElementName::hgroup;
        }
        if (tagName == u8"iframe")
        {
          return ElementName::iframe;
        }
        if (tagName == u8"keygen")
        {
          return ElementName::keygen;
        }
        if (tagName == u8"legend")
        {
          return ElementName::legend;
        }
        if (tagName == u8"nextid")
        {
          return ElementName::nextid;
        }
        if (tagName == u8"object")
        {
          return ElementName::object;
        }
        if (tagName == u8"option")
        {
          return ElementName::option;
        }
        if (tagName == u8"output")
        {
          return ElementName::output;
        }
        if (tagName == u8"script")
        {
          return ElementName::script;
        }
        if (tagName == u8"search")
        {
          return ElementName::search;
        }
        if (tagName == u8"select")
        {
          return ElementName::select;
        }
        if (tagName == u8"source")
        {
          return ElementName::source;
        }
        if (tagName == u8"spacer")
        {
          return ElementName::spacer;
        }
        if (tagName == u8"strike")
        {
          return ElementName::strike;
        }
        if (tagName == u8"strong")
        {
          return ElementName::strong;
        }
        break;
      }
      case 7:
      {
        if (tagName == u8"acronym")
        {
          return ElementName::acronym;
        }
        if (tagName == u8"address")
        {
          return ElementName::address;
        }
        if (tagName == u8"article")
        {
          return ElementName::article;
        }
        if (tagName == u8"bgsound")
        {
          return ElementName::bgsound;
        }
        if (tagName == u8"caption")
        {
          return ElementName::caption;
        }
        if (tagName == u8"isindex")
        {
          return ElementName::isindex;
        }
        if (tagName == u8"listing")
        {
          return ElementName::listing;
        }
        if (tagName == u8"marquee")
        {
          return ElementName::marquee;
        }
        if (tagName == u8"noembed")
        {
          return ElementName::noembed;
        }
        if (tagName == u8"picture")
        {
          return ElementName::picture;
        }
        if (tagName == u8"section")
        {
          return ElementName::section;
        }
        if (tagName == u8"summary")
        {
          return ElementName::summary;
        }
        break;
      }
      case 8:
      {
        if (tagName == u8"basefont")
        {
          return ElementName::basefont;
        }
        if (tagName == u8"colgroup")
        {
          return ElementName::colgroup;
        }
        if (tagName == u8"datalist")
        {
          return ElementName::datalist;
        }
        if (tagName == u8"details")
        {
          return ElementName::details;
        }
        if (tagName == u8"fieldset")
        {
          return ElementName::fieldset;
        }
        if (tagName == u8"frameset")
        {
          return ElementName::frameset;
        }
        if (tagName == u8"multicol")
        {
          return ElementName::multicol;
        }
        if (tagName == u8"noframes")
        {
          return ElementName::noframes;
        }
        if (tagName == u8"optgroup")
        {
          return ElementName::optgroup;
        }
        if (tagName == u8"progress")
        {
          return ElementName::progress;
        }
        if (tagName == u8"template")
        {
          return ElementName::template_;
        }
        if (tagName == u8"textarea")
        {
          return ElementName::textarea;
        }
        if (tagName == u8"noscript")
        {
          return ElementName::noscript;
        }
      }
      case 9:
      {
        if (tagName == u8"plaintext")
        {
          return ElementName::plaintext;
        }
        if (tagName == u8"font-face")
        {
          return ElementName::font_face;
        }
        break;
      }
      case 10:
      {
        if (tagName == u8"attachment")
        {
          return ElementName::attachment;
        }
        if (tagName == u8"blockquote")
        {
          return ElementName::blockquote;
        }
        if (tagName == u8"figcaption")
        {
          return ElementName::figcaption;
        }
        break;
      }
      case 13:
      {
        if (tagName == u8"color-profile")
        {
          return ElementName::color_profile;
        }
        if (tagName == u8"font-face-src")
        {
          return ElementName::font_face_src;
        }
        if (tagName == u8"font-face-uri")
        {
          return ElementName::font_face_uri;
        }
        if (tagName == u8"missing-glyph")
        {
          return ElementName::missing_glyph;
        }
      }
      case 14:
      {
        if (tagName == u8"font-face-name")
        {
          return ElementName::font_face_name;
        }
        if (tagName == u8"annotation-xml")
        {
          return ElementName::annotation_xml;
        }
      }
      case 15:
      {
        if (tagName == u8"selectedcontent")
        {
          return ElementName::selectedcontent;
        }
      }
      case 16:
      {
        if (tagName == u8"font-face-format")
        {
          return ElementName::font_face_format;
        }
        break;
      }
    }

    return ElementName::Unknown;
  }
}