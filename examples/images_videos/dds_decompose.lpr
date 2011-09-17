{
  Copyright 2009-2010 Michalis Kamburelis.

  This file is part of "Kambi VRML game engine".

  "Kambi VRML game engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Kambi VRML game engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Simple test of DDS reading.

  Reads DDS file $1, outputs some information about it,
  and (if not --no-save) saves a sequence of PNG files xxx-yyy.png
  (where xxx comes
  from the basename of $1, and yyy is the consecutive number from 0,
  and the files are saved in $1 directory). }
program dds_decompose;

uses SysUtils, KambiUtils, Images, DDS, KambiWarnings, KambiStringUtils,
  KambiParameters;

var
  SaveDecomposed: boolean = true;

const
  Options: array[0..0] of TOption =
  ( (Short:'n'; Long:'no-save'; Argument: oaNone) );

  procedure OptionProc(OptionNum: Integer; HasArgument: boolean;
    const Argument: string; const SeparateArgs: TSeparateArgs; Data: Pointer);
  begin
    case OptionNum of
      0: SaveDecomposed := false;
      else raise EInternalError.Create('option not impl');
    end;
  end;

var
  DImg: TDDSImage;
  OutputName, OutputBaseName: string;
  I: Integer;
begin
  OnWarning := @OnWarningWrite;
  Parameters.Parse(Options, @OptionProc, nil);
  Parameters.CheckHigh(1);
  DImg := TDDSImage.Create;
  try
    DImg.LoadFromFile(Parameters[1]);
    DImg.Flatten3d;
    Writeln('DDS loaded:', nl,
      '  Width x Height x Depth: ', DImg.Width, ' x ', DImg.Height, ' x ', DImg.Depth, nl,
      '  Type: ', DDSTypeToString[DImg.DDSType], nl,
      '  Mipmaps: ', DImg.Mipmaps, nl,
      '  Simple 2D images inside: ', DImg.Images.Count);

    if SaveDecomposed then
    begin
      if not (DImg.Images[0] is TImage) then
        raise Exception.CreateFmt('Cannot save S3TC compressed images (image class is %s)',
          [DImg.Images[0].ClassName]);

      OutputBaseName := ExtractFilePath(Parameters[1]) +
        ExtractOnlyFileName(Parameters[1]) + '_';
      for I := 0 to DImg.Images.Count - 1 do
      begin
        OutputName := OutputBaseName + IntToStrZPad(I, 2) + '.png';
        Writeln('Writing ', OutputName);
        SaveImage(DImg.Images[I] as TImage, OutputName);
      end;
    end;

  finally FreeAndNil(DImg) end;
end.