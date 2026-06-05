#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  enum class HTMLTagName : uint8
  {
    unknown,
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
    annotation_xml,
    color_profile,
    font_face,
    font_face_format,
    font_face_name,
    font_face_src,
    font_face_uri,
    missing_glyph,
  };

  KRYS_NODISCARD constexpr Maybe<HTMLTagName> TryParseHTMLTagName(DOMStringView tagName) noexcept
  {
    if (tagName.empty())
    {
      return Null;
    }

    switch (tagName.size())
    {
      case 1:
      {
        switch (tagName[0])
        {
          case u8'A':
          {
            return HTMLTagName::a;
          }
          case u8'B':
          {
            return HTMLTagName::b;
          }
          case u8'I':
          {
            return HTMLTagName::i;
          }
          case u8'P':
          {
            return HTMLTagName::p;
          }
          case u8'Q':
          {
            return HTMLTagName::q;
          }
          case u8'S':
          {
            return HTMLTagName::s;
          }
          case u8'U':
          {
            return HTMLTagName::u;
          }
        }
        break;
      }
      case 2:
      {
        if (tagName == u8"BR")
        {
          return HTMLTagName::br;
        }
        if (tagName == u8"DD")
        {
          return HTMLTagName::dd;
        }
        if (tagName == u8"DL")
        {
          return HTMLTagName::dl;
        }
        if (tagName == u8"DT")
        {
          return HTMLTagName::dt;
        }
        if (tagName == u8"EM")
        {
          return HTMLTagName::em;
        }
        if (tagName == u8"HR")
        {
          return HTMLTagName::hr;
        }
        if (tagName == u8"H1")
        {
          return HTMLTagName::h1;
        }
        if (tagName == u8"H2")
        {
          return HTMLTagName::h2;
        }
        if (tagName == u8"H3")
        {
          return HTMLTagName::h3;
        }
        if (tagName == u8"H4")
        {
          return HTMLTagName::h4;
        }
        if (tagName == u8"H5")
        {
          return HTMLTagName::h5;
        }
        if (tagName == u8"H6")
        {
          return HTMLTagName::h6;
        }
        if (tagName == u8"LI")
        {
          return HTMLTagName::li;
        }
        if (tagName == u8"OL")
        {
          return HTMLTagName::ol;
        }
        if (tagName == u8"UL")
        {
          return HTMLTagName::ul;
        }
        if (tagName == u8"RB")
        {
          return HTMLTagName::rb;
        }
        if (tagName == u8"RP")
        {
          return HTMLTagName::rp;
        }
        if (tagName == u8"RT")
        {
          return HTMLTagName::rt;
        }
        if (tagName == u8"TD")
        {
          return HTMLTagName::td;
        }
        if (tagName == u8"TH")
        {
          return HTMLTagName::th;
        }
        if (tagName == u8"TR")
        {
          return HTMLTagName::tr;
        }
        if (tagName == u8"TT")
        {
          return HTMLTagName::tt;
        }
        if (tagName == u8"UL")
        {
          return HTMLTagName::ul;
        }
        break;
      }
      case 3:
      {
        if (tagName == u8"BDI")
        {
          return HTMLTagName::bdi;
        }
        if (tagName == u8"BDO")
        {
          return HTMLTagName::bdo;
        }
        if (tagName == u8"BIG")
        {
          return HTMLTagName::big;
        }
        if (tagName == u8"COL")
        {
          return HTMLTagName::col;
        }
        if (tagName == u8"DEL")
        {
          return HTMLTagName::del;
        }
        if (tagName == u8"DFN")
        {
          return HTMLTagName::dfn;
        }
        if (tagName == u8"DIR")
        {
          return HTMLTagName::dir;
        }
        if (tagName == u8"DIV")
        {
          return HTMLTagName::div;
        }
        if (tagName == u8"IMG")
        {
          return HTMLTagName::img;
        }
        if (tagName == u8"INS")
        {
          return HTMLTagName::ins;
        }
        if (tagName == u8"KBD")
        {
          return HTMLTagName::kbd;
        }
        if (tagName == u8"MAP")
        {
          return HTMLTagName::map;
        }
        if (tagName == u8"NAV")
        {
          return HTMLTagName::nav;
        }
        if (tagName == u8"PRE")
        {
          return HTMLTagName::pre;
        }
        if (tagName == u8"RTC")
        {
          return HTMLTagName::rtc;
        }
        if (tagName == u8"SUB")
        {
          return HTMLTagName::sub;
        }
        if (tagName == u8"SUP")
        {
          return HTMLTagName::sup;
        }
        if (tagName == u8"VAR")
        {
          return HTMLTagName::var;
        }
        if (tagName == u8"WBR")
        {
          return HTMLTagName::wbr;
        }
        if (tagName == u8"XMP")
        {
          return HTMLTagName::xmp;
        }
        break;
      }
      case 4:
      {
        if (tagName == u8"ABBR")
        {
          return HTMLTagName::abbr;
        }
        if (tagName == u8"AREA")
        {
          return HTMLTagName::area;
        }
        if (tagName == u8"BASE")
        {
          return HTMLTagName::base;
        }
        if (tagName == u8"BODY")
        {
          return HTMLTagName::body;
        }
        if (tagName == u8"CITE")
        {
          return HTMLTagName::cite;
        }
        if (tagName == u8"CODE")
        {
          return HTMLTagName::code;
        }
        if (tagName == u8"DATA")
        {
          return HTMLTagName::data;
        }
        if (tagName == u8"FONT")
        {
          return HTMLTagName::font;
        }
        if (tagName == u8"FORM")
        {
          return HTMLTagName::form;
        }
        if (tagName == u8"HEAD")
        {
          return HTMLTagName::head;
        }
        if (tagName == u8"HTML")
        {
          return HTMLTagName::html;
        }
        if (tagName == u8"LINK")
        {
          return HTMLTagName::link;
        }
        if (tagName == u8"MAIN")
        {
          return HTMLTagName::main;
        }
        if (tagName == u8"MARK")
        {
          return HTMLTagName::mark;
        }
        if (tagName == u8"MENU")
        {
          return HTMLTagName::menu;
        }
        if (tagName == u8"META")
        {
          return HTMLTagName::meta;
        }
        if (tagName == u8"NOBR")
        {
          return HTMLTagName::nobr;
        }
        if (tagName == u8"RUBY")
        {
          return HTMLTagName::ruby;
        }
        if (tagName == u8"SAMP")
        {
          return HTMLTagName::samp;
        }
        if (tagName == u8"SLOT")
        {
          return HTMLTagName::slot;
        }
        if (tagName == u8"SPAN")
        {
          return HTMLTagName::span;
        }
        if (tagName == u8"TIME")
        {
          return HTMLTagName::time;
        }
        break;
      }
      case 5:
      {
        if (tagName == u8"ASIDE")
        {
          return HTMLTagName::aside;
        }
        if (tagName == u8"AUDIO")
        {
          return HTMLTagName::audio;
        }
        if (tagName == u8"BLINK")
        {
          return HTMLTagName::blink;
        }
        if (tagName == u8"EMBED")
        {
          return HTMLTagName::embed;
        }
        if (tagName == u8"FRAME")
        {
          return HTMLTagName::frame;
        }
        if (tagName == u8"IMAGE")
        {
          return HTMLTagName::image;
        }
        if (tagName == u8"INPUT")
        {
          return HTMLTagName::input;
        }
        if (tagName == u8"LABEL")
        {
          return HTMLTagName::label;
        }
        if (tagName == u8"METER")
        {
          return HTMLTagName::meter;
        }
        if (tagName == u8"MODEL")
        {
          return HTMLTagName::model;
        }
        if (tagName == u8"PARAM")
        {
          return HTMLTagName::param;
        }
        if (tagName == u8"SMALL")
        {
          return HTMLTagName::small;
        }
        if (tagName == u8"STYLE")
        {
          return HTMLTagName::style;
        }
        if (tagName == u8"TABLE")
        {
          return HTMLTagName::table;
        }
        if (tagName == u8"TBODY")
        {
          return HTMLTagName::tbody;
        }
        if (tagName == u8"TFOOT")
        {
          return HTMLTagName::tfoot;
        }
        if (tagName == u8"THEAD")
        {
          return HTMLTagName::thead;
        }
        if (tagName == u8"TITLE")
        {
          return HTMLTagName::title;
        }
        if (tagName == u8"TRACK")
        {
          return HTMLTagName::track;
        }
        if (tagName == u8"VIDEO")
        {
          return HTMLTagName::video;
        }
        break;
      }
      case 6:
      {
        if (tagName == u8"APPLET")
        {
          return HTMLTagName::applet;
        }
        if (tagName == u8"BUTTON")
        {
          return HTMLTagName::button;
        }
        if (tagName == u8"CANVAS")
        {
          return HTMLTagName::canvas;
        }
        if (tagName == u8"CENTER")
        {
          return HTMLTagName::center;
        }
        if (tagName == u8"DIALOG")
        {
          return HTMLTagName::dialog;
        }
        if (tagName == u8"FIGURE")
        {
          return HTMLTagName::figure;
        }
        if (tagName == u8"FOOTER")
        {
          return HTMLTagName::footer;
        }
        if (tagName == u8"HEADER")
        {
          return HTMLTagName::header;
        }
        if (tagName == u8"HGROUP")
        {
          return HTMLTagName::hgroup;
        }
        if (tagName == u8"IFRAME")
        {
          return HTMLTagName::iframe;
        }
        if (tagName == u8"KEYGEN")
        {
          return HTMLTagName::keygen;
        }
        if (tagName == u8"LEGEND")
        {
          return HTMLTagName::legend;
        }
        if (tagName == u8"NEXTID")
        {
          return HTMLTagName::nextid;
        }
        if (tagName == u8"OBJECT")
        {
          return HTMLTagName::object;
        }
        if (tagName == u8"OPTION")
        {
          return HTMLTagName::option;
        }
        if (tagName == u8"OUTPUT")
        {
          return HTMLTagName::output;
        }
        if (tagName == u8"SCRIPT")
        {
          return HTMLTagName::script;
        }
        if (tagName == u8"SEARCH")
        {
          return HTMLTagName::search;
        }
        if (tagName == u8"SELECT")
        {
          return HTMLTagName::select;
        }
        if (tagName == u8"SOURCE")
        {
          return HTMLTagName::source;
        }
        if (tagName == u8"SPACER")
        {
          return HTMLTagName::spacer;
        }
        if (tagName == u8"STRIKE")
        {
          return HTMLTagName::strike;
        }
        if (tagName == u8"STRONG")
        {
          return HTMLTagName::strong;
        }
        break;
      }
      case 7:
      {
        if (tagName == u8"ACRONYM")
        {
          return HTMLTagName::acronym;
        }
        if (tagName == u8"ADDRESS")
        {
          return HTMLTagName::address;
        }
        if (tagName == u8"ARTICLE")
        {
          return HTMLTagName::article;
        }
        if (tagName == u8"BGSOUND")
        {
          return HTMLTagName::bgsound;
        }
        if (tagName == u8"CAPTION")
        {
          return HTMLTagName::caption;
        }
        if (tagName == u8"ISINDEX")
        {
          return HTMLTagName::isindex;
        }
        if (tagName == u8"LISTING")
        {
          return HTMLTagName::listing;
        }
        if (tagName == u8"MARQUEE")
        {
          return HTMLTagName::marquee;
        }
        if (tagName == u8"NOEMBED")
        {
          return HTMLTagName::noembed;
        }
        if (tagName == u8"PICTURE")
        {
          return HTMLTagName::picture;
        }
        if (tagName == u8"SECTION")
        {
          return HTMLTagName::section;
        }
        if (tagName == u8"SUMMARY")
        {
          return HTMLTagName::summary;
        }
        break;
      }
      case 8:
      {
        if (tagName == u8"BASEFONT")
        {
          return HTMLTagName::basefont;
        }
        if (tagName == u8"COLGROUP")
        {
          return HTMLTagName::colgroup;
        }
        if (tagName == u8"DATALIST")
        {
          return HTMLTagName::datalist;
        }
        if (tagName == u8"DETAILS")
        {
          return HTMLTagName::details;
        }
        if (tagName == u8"FIELDSET")
        {
          return HTMLTagName::fieldset;
        }
        if (tagName == u8"FRAMESET")
        {
          return HTMLTagName::frameset;
        }
        if (tagName == u8"MULTICOL")
        {
          return HTMLTagName::multicol;
        }
        if (tagName == u8"NOFRAMES")
        {
          return HTMLTagName::noframes;
        }
        if (tagName == u8"OPTGROUP")
        {
          return HTMLTagName::optgroup;
        }
        if (tagName == u8"PROGRESS")
        {
          return HTMLTagName::progress;
        }
        if (tagName == u8"TEMPLATE")
        {
          return HTMLTagName::template_;
        }
        if (tagName == u8"TEXTAREA")
        {
          return HTMLTagName::textarea;
        }
        if (tagName == u8"NOSCRIPT")
        {
          return HTMLTagName::noscript;
        }
      }
      case 9:
      {
        if (tagName == u8"PLAINTEXT")
        {
          return HTMLTagName::plaintext;
        }
        if (tagName == u8"FONT-FACE")
        {
          return HTMLTagName::font_face;
        }
        break;
      }
      case 10:
      {
        if (tagName == u8"ATTACHMENT")
        {
          return HTMLTagName::attachment;
        }
        if (tagName == u8"BLOCKQUOTE")
        {
          return HTMLTagName::blockquote;
        }
        if (tagName == u8"FIGCAPTION")
        {
          return HTMLTagName::figcaption;
        }
        break;
      }
      case 13:
      {
        if (tagName == u8"COLOR-PROFILE")
        {
          return HTMLTagName::color_profile;
        }
        if (tagName == u8"FONT-FACE-SRC")
        {
          return HTMLTagName::font_face_src;
        }
        if (tagName == u8"FONT-FACE-URI")
        {
          return HTMLTagName::font_face_uri;
        }
        if (tagName == u8"MISSING-GLYPH")
        {
          return HTMLTagName::missing_glyph;
        }
      }
      case 14:
      {
        if (tagName == u8"FONT-FACE-NAME")
        {
          return HTMLTagName::font_face_name;
        }
        if (tagName == u8"ANNOTATION-XML")
        {
          return HTMLTagName::annotation_xml;
        }
      }
      case 15:
      {
        if (tagName == u8"SELECTEDCONTENT")
        {
          return HTMLTagName::selectedcontent;
        }
      }
      case 16:
      {
        if (tagName == u8"FONT-FACE-FORMAT")
        {
          return HTMLTagName::font_face_format;
        }
        break;
      }
    }

    return Null;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::HTMLTagName, 152u);