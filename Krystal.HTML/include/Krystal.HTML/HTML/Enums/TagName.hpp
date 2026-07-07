#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @brief An enumeration of all known tag names across the various namespaces (HTML, SVG, MathML).
  enum class TagName : uint8
  {
    Unknown,
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
    desc,
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
    foreignObject,
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
    math,
    menu,
    meta,
    meter,
    model,
    mi,
    mo,
    mn,
    ms,
    mtext,
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
    svg,
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
    color_profile,
    font_face,
    font_face_format,
    font_face_name,
    font_face_src,
    font_face_uri,
    missing_glyph,
    annotation_xml,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::TagName, 162u);

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr TagName ParseTagName(DOMStringView name) noexcept
  {
    assert(!name.empty());

    switch (name.size())
    {
      case 1:
      {
        switch (name[0])
        {
          case u8'a':
          {
            return TagName::a;
          }
          case u8'b':
          {
            return TagName::b;
          }
          case u8'i':
          {
            return TagName::i;
          }
          case u8'p':
          {
            return TagName::p;
          }
          case u8'q':
          {
            return TagName::q;
          }
          case u8's':
          {
            return TagName::s;
          }
          case u8'u':
          {
            return TagName::u;
          }
        }
        break;
      }
      case 2:
      {
        if (name == u8"br")
        {
          return TagName::br;
        }
        if (name == u8"dd")
        {
          return TagName::dd;
        }
        if (name == u8"dl")
        {
          return TagName::dl;
        }
        if (name == u8"dt")
        {
          return TagName::dt;
        }
        if (name == u8"em")
        {
          return TagName::em;
        }
        if (name == u8"hr")
        {
          return TagName::hr;
        }
        if (name == u8"h1")
        {
          return TagName::h1;
        }
        if (name == u8"h2")
        {
          return TagName::h2;
        }
        if (name == u8"h3")
        {
          return TagName::h3;
        }
        if (name == u8"h4")
        {
          return TagName::h4;
        }
        if (name == u8"h5")
        {
          return TagName::h5;
        }
        if (name == u8"h6")
        {
          return TagName::h6;
        }
        if (name == u8"li")
        {
          return TagName::li;
        }
        if (name == u8"mi")
        {
          return TagName::mi;
        }
        if (name == u8"mo")
        {
          return TagName::mo;
        }
        if (name == u8"mn")
        {
          return TagName::mn;
        }
        if (name == u8"ms")
        {
          return TagName::ms;
        }
        if (name == u8"ol")
        {
          return TagName::ol;
        }
        if (name == u8"ul")
        {
          return TagName::ul;
        }
        if (name == u8"rb")
        {
          return TagName::rb;
        }
        if (name == u8"rp")
        {
          return TagName::rp;
        }
        if (name == u8"rt")
        {
          return TagName::rt;
        }
        if (name == u8"td")
        {
          return TagName::td;
        }
        if (name == u8"th")
        {
          return TagName::th;
        }
        if (name == u8"tr")
        {
          return TagName::tr;
        }
        if (name == u8"tt")
        {
          return TagName::tt;
        }
        if (name == u8"ul")
        {
          return TagName::ul;
        }
        break;
      }
      case 3:
      {
        if (name == u8"bdi")
        {
          return TagName::bdi;
        }
        if (name == u8"bdo")
        {
          return TagName::bdo;
        }
        if (name == u8"big")
        {
          return TagName::big;
        }
        if (name == u8"col")
        {
          return TagName::col;
        }
        if (name == u8"del")
        {
          return TagName::del;
        }
        if (name == u8"dfn")
        {
          return TagName::dfn;
        }
        if (name == u8"dir")
        {
          return TagName::dir;
        }
        if (name == u8"div")
        {
          return TagName::div;
        }
        if (name == u8"img")
        {
          return TagName::img;
        }
        if (name == u8"ins")
        {
          return TagName::ins;
        }
        if (name == u8"kbd")
        {
          return TagName::kbd;
        }
        if (name == u8"map")
        {
          return TagName::map;
        }
        if (name == u8"nav")
        {
          return TagName::nav;
        }
        if (name == u8"pre")
        {
          return TagName::pre;
        }
        if (name == u8"rtc")
        {
          return TagName::rtc;
        }
        if (name == u8"sub")
        {
          return TagName::sub;
        }
        if (name == u8"sup")
        {
          return TagName::sup;
        }
        if (name == u8"sup")
        {
          return TagName::svg;
        }
        if (name == u8"svg")
        {
          return TagName::svg;
        }
        if (name == u8"var")
        {
          return TagName::var;
        }
        if (name == u8"wbr")
        {
          return TagName::wbr;
        }
        if (name == u8"xmp")
        {
          return TagName::xmp;
        }
        break;
      }
      case 4:
      {
        if (name == u8"abbr")
        {
          return TagName::abbr;
        }
        if (name == u8"area")
        {
          return TagName::area;
        }
        if (name == u8"base")
        {
          return TagName::base;
        }
        if (name == u8"body")
        {
          return TagName::body;
        }
        if (name == u8"cite")
        {
          return TagName::cite;
        }
        if (name == u8"code")
        {
          return TagName::code;
        }
        if (name == u8"data")
        {
          return TagName::data;
        }
        if (name == u8"desc")
        {
          return TagName::desc;
        }
        if (name == u8"font")
        {
          return TagName::font;
        }
        if (name == u8"form")
        {
          return TagName::form;
        }
        if (name == u8"head")
        {
          return TagName::head;
        }
        if (name == u8"html")
        {
          return TagName::html;
        }
        if (name == u8"link")
        {
          return TagName::link;
        }
        if (name == u8"main")
        {
          return TagName::main;
        }
        if (name == u8"mark")
        {
          return TagName::mark;
        }
        if (name == u8"math")
        {
          return TagName::math;
        }
        if (name == u8"menu")
        {
          return TagName::menu;
        }
        if (name == u8"meta")
        {
          return TagName::meta;
        }
        if (name == u8"nobr")
        {
          return TagName::nobr;
        }
        if (name == u8"ruby")
        {
          return TagName::ruby;
        }
        if (name == u8"samp")
        {
          return TagName::samp;
        }
        if (name == u8"slot")
        {
          return TagName::slot;
        }
        if (name == u8"span")
        {
          return TagName::span;
        }
        if (name == u8"time")
        {
          return TagName::time;
        }
        break;
      }
      case 5:
      {
        if (name == u8"aside")
        {
          return TagName::aside;
        }
        if (name == u8"audio")
        {
          return TagName::audio;
        }
        if (name == u8"blink")
        {
          return TagName::blink;
        }
        if (name == u8"embed")
        {
          return TagName::embed;
        }
        if (name == u8"frame")
        {
          return TagName::frame;
        }
        if (name == u8"image")
        {
          return TagName::image;
        }
        if (name == u8"input")
        {
          return TagName::input;
        }
        if (name == u8"label")
        {
          return TagName::label;
        }
        if (name == u8"meter")
        {
          return TagName::meter;
        }
        if (name == u8"model")
        {
          return TagName::model;
        }
        if (name == u8"mtext")
        {
          return TagName::mtext;
        }
        if (name == u8"param")
        {
          return TagName::param;
        }
        if (name == u8"small")
        {
          return TagName::small;
        }
        if (name == u8"style")
        {
          return TagName::style;
        }
        if (name == u8"table")
        {
          return TagName::table;
        }
        if (name == u8"tbody")
        {
          return TagName::tbody;
        }
        if (name == u8"tfoot")
        {
          return TagName::tfoot;
        }
        if (name == u8"thead")
        {
          return TagName::thead;
        }
        if (name == u8"title")
        {
          return TagName::title;
        }
        if (name == u8"track")
        {
          return TagName::track;
        }
        if (name == u8"video")
        {
          return TagName::video;
        }
        break;
      }
      case 6:
      {
        if (name == u8"applet")
        {
          return TagName::applet;
        }
        if (name == u8"button")
        {
          return TagName::button;
        }
        if (name == u8"canvas")
        {
          return TagName::canvas;
        }
        if (name == u8"center")
        {
          return TagName::center;
        }
        if (name == u8"dialog")
        {
          return TagName::dialog;
        }
        if (name == u8"figure")
        {
          return TagName::figure;
        }
        if (name == u8"footer")
        {
          return TagName::footer;
        }
        if (name == u8"header")
        {
          return TagName::header;
        }
        if (name == u8"hgroup")
        {
          return TagName::hgroup;
        }
        if (name == u8"iframe")
        {
          return TagName::iframe;
        }
        if (name == u8"keygen")
        {
          return TagName::keygen;
        }
        if (name == u8"legend")
        {
          return TagName::legend;
        }
        if (name == u8"nextid")
        {
          return TagName::nextid;
        }
        if (name == u8"object")
        {
          return TagName::object;
        }
        if (name == u8"option")
        {
          return TagName::option;
        }
        if (name == u8"output")
        {
          return TagName::output;
        }
        if (name == u8"script")
        {
          return TagName::script;
        }
        if (name == u8"search")
        {
          return TagName::search;
        }
        if (name == u8"select")
        {
          return TagName::select;
        }
        if (name == u8"source")
        {
          return TagName::source;
        }
        if (name == u8"spacer")
        {
          return TagName::spacer;
        }
        if (name == u8"strike")
        {
          return TagName::strike;
        }
        if (name == u8"strong")
        {
          return TagName::strong;
        }
        break;
      }
      case 7:
      {
        if (name == u8"acronym")
        {
          return TagName::acronym;
        }
        if (name == u8"address")
        {
          return TagName::address;
        }
        if (name == u8"article")
        {
          return TagName::article;
        }
        if (name == u8"bgsound")
        {
          return TagName::bgsound;
        }
        if (name == u8"caption")
        {
          return TagName::caption;
        }
        if (name == u8"isindex")
        {
          return TagName::isindex;
        }
        if (name == u8"listing")
        {
          return TagName::listing;
        }
        if (name == u8"marquee")
        {
          return TagName::marquee;
        }
        if (name == u8"noembed")
        {
          return TagName::noembed;
        }
        if (name == u8"picture")
        {
          return TagName::picture;
        }
        if (name == u8"section")
        {
          return TagName::section;
        }
        if (name == u8"summary")
        {
          return TagName::summary;
        }
        break;
      }
      case 8:
      {
        if (name == u8"basefont")
        {
          return TagName::basefont;
        }
        if (name == u8"colgroup")
        {
          return TagName::colgroup;
        }
        if (name == u8"datalist")
        {
          return TagName::datalist;
        }
        if (name == u8"details")
        {
          return TagName::details;
        }
        if (name == u8"fieldset")
        {
          return TagName::fieldset;
        }
        if (name == u8"frameset")
        {
          return TagName::frameset;
        }
        if (name == u8"multicol")
        {
          return TagName::multicol;
        }
        if (name == u8"noframes")
        {
          return TagName::noframes;
        }
        if (name == u8"optgroup")
        {
          return TagName::optgroup;
        }
        if (name == u8"progress")
        {
          return TagName::progress;
        }
        if (name == u8"template")
        {
          return TagName::template_;
        }
        if (name == u8"textarea")
        {
          return TagName::textarea;
        }
        if (name == u8"noscript")
        {
          return TagName::noscript;
        }
      }
      case 9:
      {
        if (name == u8"plaintext")
        {
          return TagName::plaintext;
        }
        if (name == u8"font-face")
        {
          return TagName::font_face;
        }
        break;
      }
      case 10:
      {
        if (name == u8"attachment")
        {
          return TagName::attachment;
        }
        if (name == u8"blockquote")
        {
          return TagName::blockquote;
        }
        if (name == u8"figcaption")
        {
          return TagName::figcaption;
        }
        break;
      }
      case 13:
      {
        if (name == u8"color-profile")
        {
          return TagName::color_profile;
        }
        if (name == u8"font-face-src")
        {
          return TagName::font_face_src;
        }
        if (name == u8"font-face-uri")
        {
          return TagName::font_face_uri;
        }
        if (name == u8"missing-glyph")
        {
          return TagName::missing_glyph;
        }
        if (name == u8"foreignobject")
        {
          return TagName::foreignObject;
        }
      }
      case 14:
      {
        if (name == u8"font-face-name")
        {
          return TagName::font_face_name;
        }
        if (name == u8"annotation-xml")
        {
          return TagName::annotation_xml;
        }
      }
      case 15:
      {
        if (name == u8"selectedcontent")
        {
          return TagName::selectedcontent;
        }
      }
      case 16:
      {
        if (name == u8"font-face-format")
        {
          return TagName::font_face_format;
        }
        break;
      }
    }

    return TagName::Unknown;
  }
}