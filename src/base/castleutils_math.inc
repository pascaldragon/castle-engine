{
  Copyright 2001-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Very basic operations on numbers.
  Also things related to setting 8087 Control Word. }

{$ifdef read_interface}

{ Swap variables values.
  @groupBegin }
procedure SwapValues(var a, b: Byte); overload;
procedure SwapValues(var a, b: Int64); overload;
procedure SwapValues(var a, b: Integer); overload;
procedure SwapValues(var a, b: Cardinal); overload;
procedure SwapValues(var a, b: Single); overload;
procedure SwapValues(var a, b: Double); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure SwapValues(var a, b: Extended); overload;
{$endif}
procedure SwapValues(var a, b: char); overload;
procedure SwapValues(var a, b: Pointer); overload;
{ @groupEnd }

{ Make sure the Smaller value is <= than the Larger value,
  by eventually swapping them.
  @groupBegin }
procedure OrderUp(var Smaller, Larger: Int64   ); overload;
procedure OrderUp(var Smaller, Larger: Integer ); overload;
procedure OrderUp(var Smaller, Larger: Cardinal); overload;
procedure OrderUp(var Smaller, Larger: Single  ); overload;
procedure OrderUp(var Smaller, Larger: Double  ); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure OrderUp(var Smaller, Larger: Extended); overload;
{$endif}
{ @groupEnd }

{ Assign the smaller value from X, Y to Smaller variable, the other one
  to Larger variable.
  @groupBegin }
procedure OrderUp(x, y: Integer;  var Smaller, Larger: Integer ); overload;
procedure OrderUp(x, y: Cardinal; var Smaller, Larger: Cardinal); overload;
procedure OrderUp(x, y: Single;   var Smaller, Larger: Single  ); overload;
procedure OrderUp(x, y: Double;   var Smaller, Larger: Double  ); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure OrderUp(x, y: Extended; var Smaller, Larger: Extended); overload;
{$endif}
{ @groupEnd }

{ Return minimum / maximum from 2 / 3 items.
  @groupBegin }
function min(const a, b: Int64): Int64; overload;
function min(const a, b: integer): integer; overload;
function min(const a, b: cardinal): cardinal; overload;
function min(const a, b: Single): Single; overload;
function min(const a, b: Double): Double; overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function min(const a, b: Extended): Extended; overload;
{$endif}

function min(const a, b, c: Int64): Int64; overload;
function min(const a, b, c: integer): integer; overload;
function min(const a, b, c: cardinal): cardinal; overload;
function min(const a, b, c: Single): Single; overload;
function min(const a, b, c: Double): Double; overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function min(const a, b, c: Extended): Extended; overload;
{$endif}

function max(const a, b: Int64): Int64; overload;
function max(const a, b: integer): integer; overload;
function max(const a, b: cardinal): cardinal; overload;
function max(const a, b: Single): Single; overload;
function max(const a, b: Double): Double; overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function max(const a, b: Extended): Extended; overload;
{$endif}

function max(const a, b, c: Int64): Int64; overload;
function max(const a, b, c: integer): integer; overload;
function max(const a, b, c: cardinal): cardinal; overload;
function max(const a, b, c: Single): Single; overload;
function max(const a, b, c: Double): Double; overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function max(const a, b, c: Extended): Extended; overload;
{$endif}
{ @groupEnd }

{ Update value of A to be a minimum of A, B.
  Works like @code(A := Min(A, B)), but is marginally faster,
  since you don't have to do anything when A is already smaller.
  @groupBegin }
procedure MinTo1st(var a: Int64   ; const b: Int64   ); overload;
procedure MinTo1st(var a: Integer ; const b: Integer ); overload;
procedure MinTo1st(var a: Cardinal; const b: Cardinal); overload;
procedure MinTo1st(var a: Single  ; const b: Single  ); overload;
procedure MinTo1st(var a: Double  ; const b: Double  ); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure MinTo1st(var a: Extended; const b: Extended); overload;
{$endif}
{ @groupEnd }

{ Update value of A to be a maximum of A, B.
  Works like @code(A := Max(A, B)), but is marginally faster,
  since you don't have to do anything when A is already larger.
  @groupBegin }
procedure MaxTo1st(var a: Int64   ; const b: Int64   ); overload;
procedure MaxTo1st(var a: Integer ; const b: Integer ); overload;
procedure MaxTo1st(var a: Cardinal; const b: Cardinal); overload;
procedure MaxTo1st(var a: Single  ; const b: Single  ); overload;
procedure MaxTo1st(var a: Double  ; const b: Double  ); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure MaxTo1st(var a: Extended; const b: Extended); overload;
{$endif}
{ @groupEnd }

{ Index (0, 1 or 2) of maximum / minimum of 3 numbers.
  @groupBegin }
function IndexMax(const a0, a1, a2: Double): Integer; overload;
function IndexMin(const a0, a1, a2: Double): Integer; overload;
{ @groupEnd }

function Between(const a, vBegin, vEnd: Int64): boolean; overload;
function Between(const a, vBegin, vEnd: integer): boolean; overload;
function Between(const a, vBegin, vEnd: cardinal): boolean; overload;
function Between(const a, vBegin, vEnd: Float): boolean; overload;
function Between(const a, vBegin, vEnd: Char): boolean; overload;

function Clamped(const a, vBegin, vEnd: Int64): Int64; overload;
function Clamped(const a, vBegin, vEnd: integer): integer; overload;
function Clamped(const a, vBegin, vEnd: cardinal): cardinal; overload;
function Clamped(const a, vBegin, vEnd: Single): Single; overload;
function Clamped(const a, vBegin, vEnd: Double): Double; overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function Clamped(const a, vBegin, vEnd: Extended): Extended; overload;
{$endif}

procedure Clamp(var a: Int64; const vBegin, vEnd: Int64); overload;
procedure Clamp(var a: integer; const vBegin, vEnd: integer); overload;
procedure Clamp(var a: cardinal; const vBegin, vEnd: cardinal); overload;
procedure Clamp(var a: Single; const vBegin, vEnd: Single); overload;
procedure Clamp(var a: Double; const vBegin, vEnd: Double); overload;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure Clamp(var a: Extended; const vBegin, vEnd: Extended); overload;
{$endif}

{ 3D coordinates (0, 1 or 2) except the given coordinate.
  If Coord = 0, then it sets First = 1 and Second = 2.
  And so on --- for each Coord value, we set First and Second to the remaining
  indexes. }
procedure RestOf3dCoords(coord: integer; out first, second: integer);

{ Increase Value by Change, nicely wrapping in [0..MaxValue], accepting
  also negative Change. The final value is always in the [0..MaxValue] range,
  even when initial Value was outside. }
function ChangeIntCycle(value, change, maxValue: integer): integer;

{ Linear interpolation between two values. Returns @code((1-A)*L + A*H).
  @groupBegin }
function Lerp(const a: Single; const l, h: Integer): Single; overload;
function Lerp(const a: Single; const l, h: Cardinal): Single; overload;
function Lerp(const a, l, h: Single): Single; overload;
function Lerp(const a: Double; const l, h: Integer): Double; overload;
function Lerp(const a: Double; const l, h: Cardinal): Double; overload;
function Lerp(const a, l, h: Double): Double; overload;
{ @groupEnd }

const
  enatural = 2.71828182845905;
  sqrt2 = 1.4142135623730950488016887242097;
  { Half of Pi. Taken from FPC sources, file rtl/inc/genmath.inc }
  HalfPi = 1.57079632679489661923;

{ Smallest multiple of Multiplicator that is still >= Value. }
function RoundUpToMultiply(value, multiplicator: Integer): Integer;

{ Largest power of 2 still <= Value.
  When Value = 0 returns 0. }
function BiggestPowerOf2(Value: Cardinal): Cardinal;

{ Exponent of the largest power of 2 that it's still <= Value.
  Like BiggestPowerOf2, except that this returns which power of 2 it is,
  while BiggestPowerOf2 just returns exactly this power.
  Bigget2Exponent(0) = -1. }
function Biggest2Exponent(Value: Cardinal): integer;

{ Smallest exponent such that 2^this exponent is >= Value.
  Smallest2Exponent(0) = -1 (this is different situation than
  Smallest2Exponent(1), when we return 0. }
function Smallest2Exponent(Value: Cardinal): Integer;

{ Smallest power of 2 that is >= Value.
  Like Smallest2Exponent, except here we return 2^Smallest2Exponent(Value).
  Returns 0 when Value = 0. }
function Smallest2Power(Value: Cardinal): Cardinal;

{ Check is Value an exact power of 2. }
function IsPowerOf2(Value: Cardinal): boolean;

function DivRoundUp(Value, Divider: Cardinal): Cardinal; overload;
function DivRoundUp(Value, Divider: Integer): Integer; overload;

{ Linearly map value from a one range to another.

  Consider how SourceVal is placed in the [SourceBegin .. SourceEnd]
  (it may be outside of this range, it all still works OK).
  It has some distance to range start (SourceBegin), and some distance
  to range end (SourceEnd).

  We want to find another number, that is identically placed in another range
  [DestBegin .. DestEnd].

  @orderedList(
    @item(Such that @italic(Distance(SourceVal,
      SourceBegin) / Distance(SourceVal, SourceEnd) =
      Distance(Result, DestBegin) / Distance(Result, DestEnd)).)

    @item(And such that it's on the same side of the range.
      So if SourceVal is beyond SourceEnd (that is,
      @code(SourceBegin < SourceEnd < SourceVal) or
      @code(SourceBegin > SourceEnd > SourceVal)) then the result
      must be similarly beyond DestEnd.

      As you can see, this works regardless if one, or both,
      of the ranges increase (range end is larger than range begin).)
  )

  @groupBegin }
function MapRange(sourceVal, sourceBegin, sourceEnd, destBegin, destEnd: integer): float; overload;
function MapRange(sourceVal, sourceBegin, sourceEnd, destBegin, destEnd: float  ): float; overload;
{ @groupEnd }

{ Random float value in the given range.
  Make sure RangeBegin < RangeEnd. }
function RandomFloatRange(const RangeBegin, RangeEnd: Float): Float;

{ Angle between a 2D line segments and OX axis. Angle 0 means that Y1 = Y2
  and X2 > X1, then the angle increases as you rotate CCW. In radians. }
function AngleRadPointToPoint(x1, y1, x2, y2: Single): Single;

{ Calculate power Base to Exponent, knowing both arguments (and so,
  also the result) are >= 0. }
function NatNatPower(Base, Exponent: Cardinal): Cardinal;

{ Random -1 or +1. }
function RandomPlusMinus: integer;

{$ifdef FPC}
{ }
function ArcCot(x: Float): Float;
{$endif FPC}

{ Trivial factorial with Int64 result. Beware, the results very quickly
  stop to fit inside Int64 range. }
function SmallFactorial(n: Integer): Int64;

{ Better DivMod version, in case Dividend may be < 0.

  This fixes lack of DivMod with negative Remainder (for FPC at least
  older than 2.2.4, see fpc-devel thread "Math.DivMod results should be signed"
  on 2006-03-21, http://www.mail-archive.com/fpc-devel@lists.freepascal.org/msg04565.html).

  And workarounds faulty DivMod behavior with FPC 2.4.0,
  see http://bugs.freepascal.org/view.php?id=15453 }
procedure KamDivMod(Dividend: Integer; Divisor: Word;
  out Result, Remainder: SmallInt);

{ Like DivMod (return result of integer division and a remainder),
  but always return Remainder >= 0.

  This is useful in case when Dividend < 0.
  Standard DivMod (and Pascal div and mod operators) return then
  Result rounded toward zero, and Remainder may be < 0.
  This procedure will return Result rounded toward negative infinity,
  and Remainder will always be >= 0. }
procedure DivUnsignedMod(Dividend: Integer; Divisor: Word;
  out Result: Smallint; out Remainder: Word);

{ Returns Ceil(A / B), but calculated faster and more precisely
  (without floating-point help). }
function CeilDiv(const A, B: Cardinal): Cardinal;

{ Calculate integer division and modulo on two float arguments.
  Current implementation assumes that A >= 0 and B > 0
  (so Result is always >= 0).

  Tries to secure against floating point imprecision.
  It's always guaranteed that Remainder >= 0 and <= B (and usually should be
  < B, but this cannot be guaranteed). }
procedure FloatDivMod(const A, B: Double;
  out DivResult: Int64; out Remainder: Double);

{ Calculate float modulo of division on two float arguments.
  See FloatDivMod for more comments. }
function FloatModulo(const A, B: Double): Double;

procedure MinMax(const x0, x1, x2: Double; out min, max: Double); overload;
procedure MinMax(const x0, x1, x2: Single; out min, max: Single); overload;

{ Our version of CoTan, to workaround
  http://www.freepascal.org/mantis/view.php?id=9944. }
function KamCoTan(const Value: Float): Float;

{ Floor from Sqrt(Value). }
function IntSqrt(const Value: Cardinal): Cardinal;

{$endif read_interface}

{$ifdef read_implementation}

procedure SwapValues(var a, b: Byte);     var Tmp: Byte;     begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Int64);    var Tmp: Int64;    begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Integer);  var Tmp: integer;  begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Cardinal); var Tmp: Cardinal; begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Single);   var Tmp: Single;   begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Double);   var Tmp: Double;   begin Tmp := A; A := B; B := Tmp; end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure SwapValues(var a, b: Extended); var Tmp: Extended; begin Tmp := A; A := B; B := Tmp; end;
{$endif}
procedure SwapValues(var a, b: char);     var Tmp: char;     begin Tmp := A; A := B; B := Tmp; end;
procedure SwapValues(var a, b: Pointer);  var Tmp: Pointer;  begin Tmp := A; A := B; B := Tmp; end;

procedure OrderUp(var Smaller, Larger: Int64);    begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
procedure OrderUp(var Smaller, Larger: Integer);  begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
procedure OrderUp(var Smaller, Larger: Cardinal); begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
procedure OrderUp(var Smaller, Larger: Single);   begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
procedure OrderUp(var Smaller, Larger: Double);   begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure OrderUp(var Smaller, Larger: Extended); begin if Smaller > Larger then SwapValues(Smaller, Larger); end;
{$endif}

procedure OrderUp(x, y: Integer;  var Smaller, Larger: Integer);  begin if X <= Y then begin Smaller := X; Larger := Y end else begin Smaller := Y; Larger := X end; end;
procedure OrderUp(x, y: Cardinal; var Smaller, Larger: Cardinal); begin if X <= Y then begin Smaller := X; Larger := Y end else begin Smaller := Y; Larger := X end; end;
procedure OrderUp(x, y: Single;   var Smaller, Larger: Single);   begin if X <= Y then begin Smaller := X; Larger := Y end else begin Smaller := Y; Larger := X end; end;
procedure OrderUp(x, y: Double;   var Smaller, Larger: Double);   begin if X <= Y then begin Smaller := X; Larger := Y end else begin Smaller := Y; Larger := X end; end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure OrderUp(x, y: Extended; var Smaller, Larger: Extended); begin if X <= Y then begin Smaller := X; Larger := Y end else begin Smaller := Y; Larger := X end; end;
{$endif}

function Min(const a, b: Int64): Int64;       begin if A < B then Result := A else Result := B end;
function Min(const a, b: integer): integer;   begin if A < B then Result := A else Result := B end;
function Min(const a, b: cardinal): cardinal; begin if A < B then Result := A else Result := B end;
function Min(const a, b: Single): Single;     begin if A < B then Result := A else Result := B end;
function Min(const a, b: Double): Double;     begin if A < B then Result := A else Result := B end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function Min(const a, b: Extended): Extended; begin if A < B then Result := A else Result := B end;
{$endif}

function Max(const a, b: Int64): Int64;       begin if A > B then Result := A else Result := B end;
function Max(const a, b: integer): integer;   begin if A > B then Result := A else Result := B end;
function Max(const a, b: cardinal): cardinal; begin if A > B then Result := A else Result := B end;
function Max(const a, b: Single): Single;     begin if A > B then Result := A else Result := B end;
function Max(const a, b: Double): Double;     begin if A > B then Result := A else Result := B end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function Max(const a, b: Extended): Extended; begin if A > B then Result := A else Result := B end;
{$endif}

function Min(const a, b, c: Int64): Int64;       begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
function Min(const a, b, c: integer): integer;   begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
function Min(const a, b, c: cardinal): cardinal; begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
function Min(const a, b, c: Single): Single;     begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
function Min(const a, b, c: Double): Double;     begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function Min(const a, b, c: Extended): Extended; begin if A < B then begin if C < A then result := C else result := A end else begin if C < B then result := C else result := B end; end;
{$endif}

function Max(const a, b, c: Int64): Int64;       begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
function Max(const a, b, c: integer): integer;   begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
function Max(const a, b, c: cardinal): cardinal; begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
function Max(const a, b, c: Single): Single;     begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
function Max(const a, b, c: Double): Double;     begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
function Max(const a, b, c: Extended): Extended; begin if A > B then begin if C > A then Result := C else Result := A end else begin if C > B then Result := C else Result := B end; end;
{$endif}

procedure MinTo1st(var a: Int64   ; const b: Int64   ); begin if B < A then A := B end;
procedure MinTo1st(var a: Integer ; const b: Integer ); begin if B < A then A := B end;
procedure MinTo1st(var a: Cardinal; const b: Cardinal); begin if B < A then A := B end;
procedure MinTo1st(var a: Single  ; const b: Single  ); begin if B < A then A := B end;
procedure MinTo1st(var a: Double  ; const b: Double  ); begin if B < A then A := B end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure MinTo1st(var a: Extended; const b: Extended); begin if B < A then A := B end;
{$endif}

procedure MaxTo1st(var a: Int64   ; const b: Int64   ); begin if B > A then A := B end;
procedure MaxTo1st(var a: Integer ; const b: Integer ); begin if B > A then A := B end;
procedure MaxTo1st(var a: Cardinal; const b: Cardinal); begin if B > A then A := B end;
procedure MaxTo1st(var a: Single  ; const b: Single  ); begin if B > A then A := B end;
procedure MaxTo1st(var a: Double  ; const b: Double  ); begin if B > A then A := B end;
{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure MaxTo1st(var a: Extended; const b: Extended); begin if B > A then A := B end;
{$endif}

function IndexMax(const a0, a1, a2: Double): Integer;
begin
 if a0 > a1 then
 begin
  if a2 > a0 then result := 2 else result := 0
 end else
 begin
  if a2 > a1 then result := 2 else result := 1
 end;
end;

function IndexMin(const a0, a1, a2: Double): Integer;
begin
 if a0 < a1 then
 begin
  if a2 < a0 then result := 2 else result := 0
 end else
 begin
  if a2 < a1 then result := 2 else result := 1
 end;
end;

function Between(const a, vBegin, vEnd: Int64): boolean;    begin Result := (vBegin <= a) and (a <= vend) end;
function Between(const a, vBegin, vEnd: integer): boolean;  begin Result := (vBegin <= a) and (a <= vend) end;
function Between(const a, vBegin, vEnd: cardinal): boolean; begin Result := (vBegin <= a) and (a <= vend) end;
function Between(const a, vBegin, vEnd: Float): boolean;    begin Result := (vBegin <= a) and (a <= vend) end;
function Between(const a, vBegin, vEnd: Char): boolean;     begin Result := (vBegin <= a) and (a <= vend) end;

function Clamped(const a, vBegin, vEnd: Int64): Int64;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;

function Clamped(const a, vBegin, vEnd: integer): integer;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;

function Clamped(const a, vBegin, vEnd: cardinal): cardinal;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;

function Clamped(const a, vBegin, vEnd: Single): Single;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;

function Clamped(const a, vBegin, vEnd: Double): Double;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;

{$ifndef EXTENDED_EQUALS_DOUBLE}
function Clamped(const a, vBegin, vEnd: Extended): Extended;
begin
  if a < vBegin then result := vBegin else
  if a > vEnd then result := vEnd else
    result := a;
end;
{$endif}

procedure Clamp(var a: Int64;    const vBegin, vEnd: Int64);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;

procedure Clamp(var a: integer;  const vBegin, vEnd: integer);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;

procedure Clamp(var a: cardinal; const vBegin, vEnd: cardinal);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;

procedure Clamp(var a: Single;   const vBegin, vEnd: Single);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;

procedure Clamp(var a: Double;   const vBegin, vEnd: Double);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;

{$ifndef EXTENDED_EQUALS_DOUBLE}
procedure Clamp(var a: Extended; const vBegin, vEnd: Extended);
begin
  if a < vBegin then a := vBegin else
  if a > vEnd then a := vEnd;
end;
{$endif}

procedure RestOf3dCoords(coord: integer; out first, second: integer);
begin
 if coord = 0 then first := 1 else first := 0;
 if coord = 2 then second := 1 else second := 2;
end;

function ChangeIntCycle(value, change, maxValue: integer): integer;
begin
 Inc(maxValue);
 result := (value+change) mod maxValue;
 {teraz wynik jest w zakresie -maxValue..maxValue gdzie maxValue to jest to
  oryginalne maxValue jakie dostalismy}
 if result < 0 then result := result+maxValue;
end;

{ Lerp : l + (h-l) * a = l + a*h - a*l = (1-a) *l + a*h }
function Lerp(const a: Single; const l, h: Integer): Single;  begin result := l + (h-l) * a; end;
function Lerp(const a: Single; const l, h: Cardinal): Single; begin result := l + (h-l) * a; end;
function Lerp(const a, l, h: Single): Single;                 begin result := l + (h-l) * a; end;
function Lerp(const a: Double; const l, h: Integer): Double;  begin result := l + (h-l) * a; end;
function Lerp(const a: Double; const l, h: Cardinal): Double; begin result := l + (h-l) * a; end;
function Lerp(const a, l, h: Double): Double;                 begin result := l + (h-l) * a; end;

function RoundUpToMultiply(value, multiplicator: Integer): Integer;
begin
 if value mod multiplicator = 0 then
  result := value else
  result := ((value div multiplicator)+1)*multiplicator;
end;

function BiggestPowerOf2(Value: Cardinal): Cardinal;
begin
 { szukamy najwiekszej potegi dwojki <= Value.
   To proste na bitach : znajdz pierwszy od lewej (najstarszy) bit rowny 1.
   Reszte bitow na prawo wyzeruj i zwroc calosc jako result. }
 result := Cardinal(1 shl (SizeOf(Cardinal)*8-1));
 while (result <> 0) and ((result and Value) = 0) do result := result shr 1;
end;

function Biggest2Exponent(Value: Cardinal): integer;
begin
 result := -1;
 while Value <> 0 do
  begin Inc(result); Value := Value shr 1 end;
end;

function Smallest2Exponent(Value: Cardinal): Integer;
begin
 if Value = 0 then begin result := -1; Exit end;

 for result := SizeOf(Value)*8-1 downto 0 do
  if ((LongWord(1) shl result) and Value) <> 0 then Break;

 { Wiemy ze Value <> 0 wiec zawsze z powyzszego for'a wyjdziemy breakiem.
   Teraz result okresla pozycje najbardziej znaczacej 1-ki w liczbie Value.
   Jezeli wszystkie pozostale bity sa rowne 0 (czyli Value = 1 shl result)
   to result juz jest dobry. Wpp. zeby 2^result bylo >= Value musimy
   zwiekszyc result o 1.}
 if Value <> Cardinal(1) shl result then Inc(result);
end;

function Smallest2Power(Value: Cardinal): Cardinal;
var Exp: Integer;
begin
 Exp := Smallest2Exponent(Value);
 if Exp = -1 then
  result := 0 else
  result := 1 shl Exp;
end;

function IsPowerOf2(Value: Cardinal): boolean;
var i: Cardinal;
begin
 i := Cardinal(1 shl (SizeOf(Cardinal)*8-1));
 while (i <> 0) do
 begin
  if i = Value then begin result := true; exit end;
  i := i shr 1;
 end;
 result := false;
end;

function DivRoundUp(Value, Divider: Cardinal): Cardinal;
begin
  Result := (Value + Divider - 1) div Divider;
end;

function DivRoundUp(Value, Divider: Integer): Integer;
begin
  Result := (Value + Divider - 1) div Divider;
end;

function MapRange(sourceVal, sourceBegin, sourceEnd, destBegin, destEnd: integer): float;
begin
  { Map range SourceBegin .. SourceEnd to 0..1, and express SourceVal there. }
  Result := (sourceVal - sourceBegin) / (sourceEnd - sourceBegin);
  { Map range 0..1 to DestBegin .. DestEnd }
  Result := Result * (destEnd - destBegin) + destBegin;
end;

function MapRange(sourceVal, sourceBegin, sourceEnd, destBegin, destEnd: float  ): float;
begin
  { Map range SourceBegin .. SourceEnd to 0..1, and express SourceVal there. }
  Result := (sourceVal - sourceBegin) / (sourceEnd - sourceBegin);
  { Map range 0..1 to DestBegin .. DestEnd }
  Result := Result * (destEnd - destBegin) + destBegin;
end;

function RandomFloatRange(const RangeBegin, RangeEnd: Float): Float;
begin
 result := Random*(RangeEnd-RangeBegin) + RangeBegin;
end;

function AngleRadPointToPoint(x1, y1, x2, y2: Single): Single;
begin
 if y2 > y1 then
 begin
  if SameValue(x1, x2) then
   result := Pi/2 else
  begin
   result := ArcTan( (y2-y1) / (x2-x1) );
   if result < 0 then result := result +Pi;
  end;
 end else
 if y1 > y2 then
 begin
  if SameValue(x1, x2) then
   result := Pi*3/2 else
  begin
   result := ArcTan( (y2-y1) / (x2-x1) );
   if result < 0 then result := result +Pi;
   result := result +Pi;
  end;
 end else
 if x2 > x1 then result := 0 else result := Pi;
end;

function NatNatPower(Base, Exponent: Cardinal): Cardinal;
var i : Cardinal;
begin
 result := 1;
 i := Exponent;
 while i > 0 do
 begin
  while not Odd(i) do
  begin
   i := i shr 1;
   base := sqr(base);
  end;
  i := i-1;
  result := result*base;
 end;
end;

function RandomPlusMinus: integer;
const PlusMinus: array[0..1]of integer = (-1,+1);
begin
 result := PlusMinus[Random(2)];
end;

{$ifdef FPC}
function ArcCot(x: Float): Float;
begin
 result := ArcTan(1/x);
end;
{$endif FPC}

function SmallFactorial(n: Integer): Int64;
begin
 Result := 1;
 while n > 1 do begin Result := Result * n; Dec(n) end;
end;

procedure KamDivMod(Dividend: Integer; Divisor: Word;
  out Result, Remainder: SmallInt);
{$ifdef VER2_4_0}
{ Workaround http://bugs.freepascal.org/view.php?id=15453.
  DivMod overload with SmallInt params already exist in Math unit,
  so this is not our concern. }
begin
  if Dividend < 0 then
  begin
    { Use DivMod with >=0 dividend }
    DivMod(-Dividend, Divisor, Result, Remainder);
    { The documented behavior of Pascal's div/mod operators and DivMod
      on negative dividends is to return Result closer to zero and
      a negative Remainder. Which means that we can just negate both
      Result and Remainder, and all it's Ok. }
    Result := -Result;
    Remainder := -Remainder;
  end else
    DivMod(Dividend, Divisor, Result, Remainder);
{$else}
{ Workaround lack of DivMod overload with signed (SmallInt) params.
  This is needed only for old FPC (at least older than 2.2.4). }
var
  UnsignedResult: Word absolute Result;
  UnsignedRemainder: Word absolute Remainder;
begin
  DivMod(Dividend, Divisor, UnsignedResult, UnsignedRemainder);
{$endif}
end;

procedure DivUnsignedMod(Dividend: Integer; Divisor: Word;
  out Result: Smallint; out Remainder: Word);
var
  SignedRemainder: SmallInt;
begin
  KamDivMod(Dividend, Divisor, Result, SignedRemainder);
  if SignedRemainder < 0 then
  begin
    Dec(Result);
    Remainder := Divisor + SignedRemainder;
  end else
    Remainder := SignedRemainder;
end;

function CeilDiv(const A, B: Cardinal): Cardinal;
var
  DivResult, ModResult: Cardinal;
begin
  { I would like to use DivMod, but it's limited to 16-bit values.
    DivMod(A, B, DivResult, ModResult); }
  DivResult := A div B;
  ModResult := A mod B;
  if ModResult = 0 then
    Result := DivResult else
    Result := DivResult + 1;
end;

procedure FloatDivMod(const A, B: Double;
  out DivResult: Int64; out Remainder: Double);
begin
  DivResult := Trunc(A / B);
  Remainder := A - B * DivResult;
  { Now the tricks to make this (somewhat) more numerically stable,
    i.e. resistant to tricky cases without unprecise floating-point
    calculation. }
  if Remainder < 0 then
    Remainder := 0 else
  if Remainder >= B then
  begin
    Remainder -= B;
    Inc(DivResult);

    { Mathematically, we could do above two lines in a loop
      "while Remainder >= A", but this could loop (practically)
      forever for very small (or zero) B.

      Instead we just clamp Remainder to [0, B] once again, the hard way. }

    Clamp(Remainder, 0, B);
  end;
end;

function FloatModulo(const A, B: Double): Double;
var
  DivResult: Int64;
begin
  FloatDivMod(A, B, DivResult, Result);
end;

procedure MinMax(const x0, x1, x2: Double; out min, max: Double);
begin
  min := x0;
  max := x0;
  if   x1 < min then min := x1 else
    if x1 > max then max := x1;
  if   x2 < min then min := x2 else
    if x2 > max then max := x2;
end;

procedure MinMax(const x0, x1, x2: Single; out min, max: Single);
begin
  min := x0;
  max := x0;
  if   x1 < min then min := x1 else
    if x1 > max then max := x1;
  if   x2 < min then min := x2 else
    if x2 > max then max := x2;
end;

function KamCoTan(const Value: Float): Float;
var
  ASin, ACos: Float;
begin
  SinCos(Value, ASin, ACos);
  Result := Acos / ASin;
end;

function IntSqrt(const Value: Cardinal): Cardinal;
{ See http://en.wikipedia.org/wiki/Integer_square_root.

  This isn't nice impl, as it uses floating-point numbers to calculate
  integer function... However, doing it on ints, doesn't make this
  process terminate nicely. Hacks like in primes.cpp from
  http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
  don't calculate floor sqrt properly (e.g. for 8 return 3).

  So for now, calculate on floats. Autotests check it's Ok.

  Actually, when calculating on floats, I can just use Trunc(Sqrt())...
  So this is quite pointless exercise. }
(*
var
  Old, New: Float;
begin
  if Value = 0 then Exit(0); { to not fail by division by zero }

  New := Value;
  repeat
    Old := New;
    New := (Old + Value / Old) / 2;
  until Abs(Old - New) < 0.5;
  Result := Trunc(New);
end;
*)
begin
  Result := Trunc(Sqrt(Value));
end;

{$endif read_implementation}