{
  Copyright 2014-2014 Michalis Kamburelis.

  This file is part of "Castle Spine".

  "Castle Spine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Spine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{$apptype GUI}

{ "Castle Spine" standalone game binary. }
program castle_spine;

{$ifdef MSWINDOWS}
  {$R ../automatic-windows-resources.res}
{$endif MSWINDOWS}

uses CastleWindow, CastleConfig, CastleParameters, CastleLog, CastleUtils,
  CastleSoundEngine, CastleClassUtils,
  Game;

const
  Options: array [0..0] of TOption =
  (
    (Short:  #0; Long: 'debug-log'; Argument: oaNone)
  );

procedure OptionProc(OptionNum: Integer; HasArgument: boolean;
  const Argument: string; const SeparateArgs: TSeparateArgs; Data: Pointer);
begin
  case OptionNum of
    0: InitializeLog;
    else raise EInternalError.Create('OptionProc');
  end;
end;

begin
  { do not set FullScreen, only to easily test that resizing the window
    still looks Ok }
//  Window.FullScreen := true;
  Window.ParseParameters;
  Parameters.Parse(Options, @OptionProc, nil);

  Window.OpenAndRun;
end.
