{
  Copyright 2002-2012 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Convert fonts available on Windows to TBitmapFont and TOutlineFont.

  This unit heavily depends on GetGlpyhOutline WinAPI function.
  This function is our "core" of converting Windows fonts to
  TBitmapFont or TOutlineFont. Unfortunately, this makes this unit
  Windows-only forever.
  In the future I plan to write some similiar unit, but portable.
  Probably using FreeType2 library.
}

unit CastleWinFontConvert;

interface

uses Windows, CastleBitmapFonts, CastleOutlineFonts;

{ Both functions grab character c from Windows font currently selected
  on device context dc. Remeber to free resulting pointer by FreeMem.

  "Underline" and "strikeout" : I observed that it does not matter whether
  font is underline or strikeout (e.g. what values for fdwUnderline/StrikeOut
  you gave when calling CreateFont). Fonts converted by this function are
  always NOT underlined and not strikeout.
  On the other hand, "bold" (actually "weight") and "italic" matter,
  i.e. fonts created by this functions reflect the weight and italic state
  of the font.
  This is true both for Font2BitmapChar and Font2OutlineChar.
  This is an effect of how Windows.GetGlyphOutline works.
  However I didn't find any place documenting these details about
  GetGlyphOutline.
  This is not a result of the fact that TTF fonts are usually distributed
  with separate bold, italic, and bold+italic versions, because I checked
  this with fonts that are distributed without bold, italic, bold+italic
  versions (i.e. bold, italic, bold+italic versions must be automatically
  synthesized by Windows) and for such fonts it works the same :
  bold, italic, b+i versions can be constructed by Font2XxxChar versions
  below, and underline and strikeout properties of Windows font are
  ignored.

  @groupBegin }
function Font2BitmapChar_HDc(dc: HDc; c: char): PBitmapChar;
function Font2OutlineChar_HDc(dc: HDc; c: char): POutlineChar;
{ @groupEnd }

{ Grab currently selected Windows font on device context dc.
  Remeber to free resulting font later by FreeAndNilFont.
  @groupBegin }
function Font2BitmapFont_HDc(dc: HDc): TBitmapFont;
function Font2OutlineFont_HDc(dc: HDc): TOutlineFont;
{ @groupEnd }

{ Usually much more comfortable versions of Font2XxxFont_HDc
  and Font2XxxChar_HDc.
  They take HFont, not HDc. This way they avoid some strangeness
  of GetGlyphOutline function (that requires as an argument HDc with
  selected font, not HFont)

  @groupBegin }
function Font2BitmapChar(WinFont: HFont; c: char): PBitmapChar;
function Font2OutlineChar(WinFont: HFont; c: char): POutlineChar;
function Font2BitmapFont(WinFont: HFont): TBitmapFont;
function Font2OutlineFont(WinFont: HFont): TOutlineFont;
{ @groupEnd }

{ Free and nil Font instance, freeing also all characters by FreeMem.
  Use this only on fonts with characters created by GetMem.
  @groupBegin }
procedure FreeAndNilFont(var Font: TBitmapFont); overload;
procedure FreeAndNilFont(var Font: TOutlineFont); overload;
{ @groupEnd }

implementation

uses SysUtils, CastleUtils, CastleGenericLists;

const
  IdentityMat2:Mat2= { identity matrix }
    (eM11:(fract:0; value:1); eM12:(fract:0; value:0);    { 1.0  0.0 }
     eM21:(fract:0; value:0); eM22:(fract:0; value:1));   { 0.0  1.0 }

function Font2BitmapChar_HDc(dc: HDC; c: char): PBitmapChar;
var GlyphData: PByteArray;
    GlyphMetrics: TGlyphMetrics;
    GlyphDataSize: Dword;
    j: Cardinal;
    RowByteLength: Cardinal;
begin
 GlyphData := nil;
 try
  { get GlyphDataSize }
  GlyphDataSize := GetGlyphOutline(dc, Ord(c), GGO_BITMAP, GlyphMetrics, 0,
    nil, IdentityMat2);
  OSCheck( GlyphDataSize <> GDI_ERROR, 'GlyphDataSize');

  if GlyphDataSize = 0 then
  begin
   { Assert((gmBlackBoxX = 1) and (gmBlackBoxY = 1)) works under Win2000 Prof,
     Assert((gmBlackBoxX = 0) and (gmBlackBoxY = 0)) works under Win98.
     So I just ignore values of gmBlackBoxX and gmBlackBoxY -- it seems
     that they have no guaranteed value when GlyphDataSize = 0. }
   Result := GetMem(SizeOf(TBitmapCharInfo));

   Result^.Info.Alignment := 4;
   Result^.Info.XOrig := -GlyphMetrics.gmptGlyphOrigin.x;
   Result^.Info.YOrig := -GlyphMetrics.gmptGlyphOrigin.y;
   Result^.Info.XMove := GlyphMetrics.gmCellIncX;
   Result^.Info.YMove := GlyphMetrics.gmCellIncY;
   Result^.Info.Width := 0;
   Result^.Info.Height := 0;
  end else
  begin
   GlyphData := GetMem(GlyphDataSize);
   Check( GetGlyphOutline(dc, Ord(c), GGO_BITMAP, GlyphMetrics, GlyphDataSize,
     GlyphData, IdentityMat2) <> GDI_ERROR, 'GetGlyphOutline failed');

    { alignment w zwroconej data jest zawsze 4 }
    RowByteLength := BitmapCharRowByteLength(GlyphMetrics.gmBlackBoxX, 4);

    { Under Win98 this assertion was always true.
        Assert(GlyphDataSize = linelen * gmBlackBoxY);
      Under Win2000 Prof this is no longer true -- it seems that GlyphDataSize
      is sometimes excessively large, e.g. I have here gmBlackBoxX = 12
      (so linelen = 4), gmBlackBoxY = 19, so GlyphDataSize should be 19*4.
      But it is 80 = 20*4.

      So I'm correcting here GlyphDataSize to something smaller. This
      leads to no harm because GetMem(Pointer(GlyphData), GlyphDataSize);
      is already done. So I already allocated memory. I will just look at
      this memory area as though it would be smaller than it is. }
   Assert(GlyphDataSize >= RowByteLength * GlyphMetrics.gmBlackBoxY);
   GlyphDataSize := RowByteLength * GlyphMetrics.gmBlackBoxY;

   Result := GetMem(SizeOf(TBitmapCharInfo) + GlyphDataSize*SizeOf(Byte));

   Result^.Info.Alignment := 4;
   Result^.Info.XOrig := -GlyphMetrics.gmptGlyphOrigin.x;
   Result^.Info.YOrig := Integer(GlyphMetrics.gmBlackBoxY) - GlyphMetrics.gmptGlyphOrigin.y;
   Result^.Info.XMove := GlyphMetrics.gmCellIncX;
   Result^.Info.YMove := GlyphMetrics.gmCellIncY;
   Result^.Info.Width:=  GlyphMetrics.gmBlackBoxX;
   Result^.Info.Height := GlyphMetrics.gmBlackBoxY;

   { copy GlyphData do Result^.Data line by line.
     We must replace line order - TBitmapFont wants lines bottom -> top,
     while GlyphData has lines top -> bottom. }
   for j := 0 to GlyphMetrics.gmBlackBoxY-1 do
    Move(GlyphData[(Result^.Info.Height - j - 1) * RowByteLength],
         Result^.Data[j * RowByteLength],
         RowByteLength);
  end;
 finally
  FreeMemNiling(Pointer(GlyphData));
 end;
end;

type
  TOutlineCharItemList = specialize TGenericStructList<TOutlineCharItem>;

function Font2OutlineChar_HDc(dc: HDC; c: char): POutlineChar;
var GlyphMetrics: TGlyphMetrics;
    GlyphDataSize: Dword;
    Buffer : Pointer;
    BufferEnd, PolygonEnd : PtrUInt;
    PolHeader : PTTPolygonHeader;
    PolCurve : PTTPolyCurve;
    i, j: integer;
type TArray_PointFX = packed array[0..High(Word)] of TPointFX;
     PArray_PointFX = ^TArray_PointFX;
var PointsFX: PArray_PointFX;
    ResultItems: TOutlineCharItemList;
    ResultInfo: TOutlineCharInfo;
    lastPunkt: record x, y: Single end;
    Dlug: Cardinal;

  procedure ResultItemsAdd(Kind: TPolygonKind; Count: Cardinal{ = 0}); overload;
  { use only with Kind <> pkPoint }
  var
    Item: POutlineCharItem;
  begin
    Item := ResultItems.Add;
    Assert(Kind <> pkPoint);
    Item^.Kind := Kind;
    Item^.Count := Count;
  end;

  procedure ResultItemsAdd(Kind: TPolygonKind {Count: Cardinal = 0 }); overload;
  begin
   ResultItemsAdd(Kind, 0);
  end;

  procedure ResultItemsAdd(x, y: Single); overload;
  var
    Item: POutlineCharItem;
  begin
    Item := ResultItems.Add;
    Item^.Kind := pkPoint;
    Item^.x := x;
    Item^.y := y;
  end;

  function ToFloat(const Val: TFixed): Extended;
  begin
   result := Val.value + (Val.fract / Dword(High(Val.fract))+1 );
  end;

begin
 Result := nil; { <- only to avoid Delphi warning }

 ResultItems := TOutlineCharItemList.Create;
 try
  Buffer := nil;
  try
   { get Buffer }
   GlyphDataSize := GetGlyphOutline(dc, Ord(c), GGO_NATIVE, GlyphMetrics, 0, nil,
     IdentityMat2);
   OSCheck( GlyphDataSize <> GDI_ERROR, 'GetGlyphOutline');
   Buffer := GetMem(GlyphDataSize);
   Check( GetGlyphOutline(dc, Ord(c), GGO_NATIVE, GlyphMetrics, GlyphDataSize,
     Buffer, identityMat2) <> GDI_ERROR, 'GetGlyphOutline');

   { convert GlyphMetrics to ResultInfo (ResultInfo.Polygons/ItemsCount will be
     calculated later) }
   ResultInfo.MoveX := GlyphMetrics.gmCellIncX;
   ResultInfo.MoveY := GlyphMetrics.gmCellIncY;
   ResultInfo.Height := GlyphMetrics.gmptGlyphOrigin.y + Integer(GlyphMetrics.gmBlackBoxY);

   { wskazniki na BufferEnd lub dalej wskazuja ZA Bufferem }
   BufferEnd := PtrUInt(Buffer)+GlyphDataSize;

   { calculate ResultItems. Only "Count" fields are left not initialized. }
   PolHeader := Buffer; { pierwszy PolHeader }
   while PtrUInt(PolHeader) < BufferEnd do
   begin
    { czytaj PolHeader }
    ResultItemsAdd(pkNewPolygon);
    PolygonEnd := PtrUInt(PointerAdd(PolHeader, PolHeader^.cb));
    lastPunkt.x := ToFloat(PolHeader^.pfxStart.x);
    lastPunkt.y := ToFloat(PolHeader^.pfxStart.y);

    { czytaj PolCurves }
    PolCurve := PointerAdd(PolHeader, SizeOf(TTPolygonHeader)); { pierwszy PolCurve }
    while PtrUInt(PolCurve) < PolygonEnd do
    begin
     case PolCurve^.wType of
      TT_PRIM_LINE : ResultItemsAdd(pkLines);
      TT_PRIM_QSPLINE : ResultItemsAdd(pkBezier);
      else raise Exception.Create('UNKNOWN PolCurve^.wType !!!!');
     end;
     ResultItemsAdd(lastPunkt.x, lastPunkt.y);
     PointsFX := @PolCurve^.apfx[0];
     for i := 0 to PolCurve^.cpfx-1 do
     begin
      lastPunkt.x := ToFloat(PointsFX^[i].x);
      lastPunkt.y := ToFloat(PointsFX^[i].y);
      ResultItemsAdd(lastPunkt.x, lastPunkt.y);
     end;
     { nastepny PolCurve: }
     PolCurve := PointerAdd(PolCurve, SizeOf(TTPolyCurve) +
       (PolCurve^.cpfx-1)*SizeOf(TPointFX));
    end;

    { zakoncz ten polygon }
    with PolHeader^.pfxStart do ResultItemsAdd(ToFloat(x), ToFloat(y));

    { nastepny PolHeader: }
    PolHeader := Pointer(PolCurve);
   end;
  finally FreeMemNiling(Buffer) end;

  { calculate "Count" fields for items with Kind <> pkPoint in ResultItems }
  for i := 0 to ResultItems.Count - 1 do
     case ResultItems.L[i].Kind of
     pkNewPolygon:
       begin
        dlug := 0;
        for j := i+1 to ResultItems.Count - 1 do
         case ResultItems.L[j].Kind of
          pkLines, pkBezier : Inc(dlug);
          pkNewPolygon : break;
         end;
        ResultItems.L[i].Count := dlug;
       end;
     pkLines, pkBezier:
       begin
        dlug := 0;
        for j := i+1 to ResultItems.Count - 1 do
         if ResultItems.L[j].Kind = pkPoint then Inc(dlug) else break;
        ResultItems.L[i].Count := dlug;
       end;
    end;

  { calculate Result^.Info.PolygonsCount/ItemsCount }
  ResultInfo.ItemsCount := ResultItems.Count;
  ResultInfo.PolygonsCount := 0;
  for i := 0 to ResultItems.Count - 1 do
   if ResultItems.L[i].Kind = pkNewPolygon then Inc(ResultInfo.PolygonsCount);

  { get mem for Result and fill Result^ with calculated data }
  Result := GetMem(SizeOf(TOutlineCharInfo) +
    ResultInfo.ItemsCount*SizeOf(TOutlineCharItem));
  try
   Result^.Info := ResultInfo;
   Move(ResultItems.L[0], Result^.Items,
     ResultInfo.ItemsCount*SizeOf(TOutlineCharItem));
  except FreeMem(Result); raise end;

 finally ResultItems.Free end;
end;

{ Font2XxxFont_HDc ----------------------------------------------- }

function Font2BitmapFont_HDc(dc: HDC): TBitmapFont;
var c: char;
begin
  Result := TBitmapFont.Create;
  try
    for c := Low(char) to High(char) do
      Result.Data[c] := Font2BitmapChar_HDc(dc, c);
  except FreeAndNilFont(Result); raise end;
end;

function Font2OutlineFont_HDc(dc: HDC): TOutlineFont;
var c: char;
begin
  Result := TOutlineFont.Create;
  try
    for c := Low(char) to High(char) do
      Result.Data[c] := Font2OutlineChar_HDc(dc, c);
  except FreeAndNilFont(Result); raise end;
end;

{ versions without _HDc ------------------------------------------- }

function Font2BitmapChar(WinFont: HFont; c: char): PBitmapChar;
{$I winfontconvert_dc_from_winfont_declare.inc}
begin
 {$I winfontconvert_dc_from_winfont_begin.inc}
 Result := Font2BitmapChar_HDc(dc, c);
 {$I winfontconvert_dc_from_winfont_end.inc}
end;

function Font2OutlineChar(WinFont: HFont; c: char): POutlineChar;
{$I winfontconvert_dc_from_winfont_declare.inc}
begin
 {$I winfontconvert_dc_from_winfont_begin.inc}
 Result := Font2OutlineChar_HDc(dc, c);
 {$I winfontconvert_dc_from_winfont_end.inc}
end;

function Font2BitmapFont(WinFont: HFont): TBitmapFont;
{$I winfontconvert_dc_from_winfont_declare.inc}
begin
  {$I winfontconvert_dc_from_winfont_begin.inc}
  Result := Font2BitmapFont_HDc(dc);
  {$I winfontconvert_dc_from_winfont_end.inc}
end;

function Font2OutlineFont(WinFont: HFont): TOutlineFont;
{$I winfontconvert_dc_from_winfont_declare.inc}
begin
  {$I winfontconvert_dc_from_winfont_begin.inc}
  Result := Font2OutlineFont_HDc(dc);
  {$I winfontconvert_dc_from_winfont_end.inc}
end;

{ FreeAndNilFont ------------------------------------------------------------- }

procedure FreeAndNilFont(var Font: TBitmapFont);
var
  c: char;
begin
  for c := Low(char) to High(char) do FreeMem(Pointer(Font.Data[c]));
  FreeAndNil(Font);
end;

procedure FreeAndNilFont(var Font: TOutlineFont);
var
  c: char;
begin
  for c := Low(char) to High(char) do FreeMem(Pointer(Font.Data[c]));
  FreeAndNil(Font);
end;

end.