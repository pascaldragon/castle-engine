{
  Copyright 2004-2010 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

unit TestCastleClassUtils;

interface

uses Classes, SysUtils, fpcunit, testutils, testregistry, CastleUtils,
  CastleClassUtils, FGL {$ifdef VER2_2}, FGLObjectList22 {$endif};

type
  TStreamFromStreamFunc = function(Stream: TStream): TPeekCharStream of object;

  TTestCastleClassUtils = class(TTestCase)
  private
    BufferSize: LongWord;
    procedure TestIndirectReadStream(
      StreamFromStreamFunc: TStreamFromStreamFunc);
    function StreamPeekCharFromStream(Stream: TStream): TPeekCharStream;
    function BufferedReadStreamFromStream(Stream: TStream): TPeekCharStream;
  published
    procedure TestStreamPeekChar;
    procedure TestBufferedReadStream;
    procedure TestObjectsListSort;
    procedure TestNotifyEventArray;
  end;

  TFoo = class
    constructor Create(AI: Integer; AnS: string);
  public
    I: Integer;
    S: string;
  end;

  TFooList = class(specialize TFPGObjectList<TFoo>)
  public
    procedure SortFoo;
  end;

implementation

{ TFoo, TFoosList ------------------------------------------------------------ }

constructor TFoo.Create(AI: Integer; AnS: string);
begin
  I := AI;
  S := AnS;
end;

function IsFooSmaller(const A, B: TFoo): Integer;
begin
  Result := A.I - B.I;
end;

procedure TFooList.SortFoo;
begin
  Sort(@IsFooSmaller);
end;

{ TTestCastleClassUtils ------------------------------------------------------- }

procedure TTestCastleClassUtils.TestIndirectReadStream(
  StreamFromStreamFunc: TStreamFromStreamFunc);
var
  SStream: TStringStream;
  ReaderStream: TPeekCharStream;
  Buf: array[0..2]of Byte;
begin
 SStream := TStringStream.Create(#1#2#3#4#5#6#7#8#9#10#11#12);
 try
  SStream.Position := 0;
  ReaderStream := StreamFromStreamFunc(SStream);
  try
   Assert(ReaderStream.Size = 12);
   Assert(ReaderStream.PeekChar = 1);
   Assert(ReaderStream.PeekChar = 1);
   Assert(ReaderStream.PeekChar = 1);
   Assert(ReaderStream.ReadChar = 1);

   Assert(ReaderStream.PeekChar = 2);
   Assert(ReaderStream.ReadChar = 2);

   Assert(ReaderStream.ReadChar = 3);

   Assert(ReaderStream.ReadChar = 4);

   Assert(ReaderStream.ReadUpto([#8, #9, #10]) = #5#6#7);

   Assert(ReaderStream.Position = 7);
   Assert(ReaderStream.ReadChar = 8);
   Assert(ReaderStream.Position = 8);

   ReaderStream.ReadBuffer(Buf, 3);
   Assert(Buf[0] =  9);
   Assert(Buf[1] = 10);
   Assert(Buf[2] = 11);

   Assert(ReaderStream.PeekChar = 12);
   Assert(ReaderStream.ReadChar = 12);

   Assert(ReaderStream.Read(Buf, 1) = 0);
   Assert(ReaderStream.PeekChar = -1);
   Assert(ReaderStream.Read(Buf, 1) = 0);
  finally ReaderStream.Free end;
 finally SStream.Free end;
end;

function TTestCastleClassUtils.StreamPeekCharFromStream(Stream: TStream):
  TPeekCharStream;
begin
 Result := TSimplePeekCharStream.Create(Stream, false);
end;

procedure TTestCastleClassUtils.TestStreamPeekChar;
begin
 TestIndirectReadStream(@StreamPeekCharFromStream);
end;

function TTestCastleClassUtils.BufferedReadStreamFromStream(Stream: TStream):
  TPeekCharStream;
begin
 Result := TBufferedReadStream.Create(Stream, false, BufferSize);
end;

procedure TTestCastleClassUtils.TestBufferedReadStream;
var i: Integer;
begin
 for i := 1 to 20 do
 begin
  BufferSize := i;
  TestIndirectReadStream(@BufferedReadStreamFromStream);
 end;
end;

procedure TTestCastleClassUtils.TestObjectsListSort;
var
  L: TFooList;
begin
  L := TFooList.Create(true);
  try
    L.Add(TFoo.Create(123, 'abc'));
    L.Add(TFoo.Create(-5, 'ZZZ'));
    L.Add(TFoo.Create(65, 'zuzanna'));
    L.SortFoo;
    Assert(L.Count = 3);
    Assert(L[0].I = -5);
    Assert(L[1].I = 65);
    Assert(L[2].I = 123);
  finally FreeAndNil(L) end;
end;

type
  TObj = class
    procedure Dummy(Sender: TObject);
  end;

procedure TObj.Dummy(Sender: TObject);
begin
end;

procedure TTestCastleClassUtils.TestNotifyEventArray;
{ There's a trap when implementing lists of methods: normal comparison operator
  is nonsense for methods, it compares only the code pointer.
  See http://bugs.freepascal.org/view.php?id=11868 ,
  http://bugs.freepascal.org/view.php?id=9228 .
  Make sure our TNotifyEventList doesn't have this problem. }
var
  A: TNotifyEventList;
  O1, O2, O3: TObj;
begin
  A := TNotifyEventList.Create;
  try
    O1 := TObj.Create;
    O2 := TObj.Create;
    O3 := TObj.Create;
    Assert(A.IndexOf(@O1.Dummy) = -1);
    Assert(A.IndexOf(@O2.Dummy) = -1);
    Assert(A.IndexOf(@O3.Dummy) = -1);

    A.Add(@O1.Dummy);
    Assert(A.IndexOf(@O1.Dummy) = 0);
    Assert(A.IndexOf(@O2.Dummy) = -1);
    Assert(A.IndexOf(@O3.Dummy) = -1);

    A.Add(@O2.Dummy);
    Assert(A.IndexOf(@O1.Dummy) = 0);
    Assert(A.IndexOf(@O2.Dummy) = 1);
    Assert(A.IndexOf(@O3.Dummy) = -1);

    A.Remove(@O1.Dummy);
    Assert(A.IndexOf(@O1.Dummy) = -1);
    Assert(A.IndexOf(@O2.Dummy) = 0);
    Assert(A.IndexOf(@O3.Dummy) = -1);

    FreeAndNil(O1);
    FreeAndNil(O2);
    FreeAndNil(O3);
  finally FreeAndNil(A) end;
end;

initialization
 RegisterTest(TTestCastleClassUtils);
end.