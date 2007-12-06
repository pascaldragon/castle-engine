{
  Copyright 2002-2005,2007 Michalis Kamburelis.

  This file is part of "Kambi VRML game engine".

  "Kambi VRML game engine" is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  "Kambi VRML game engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Kambi VRML game engine"; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
}

{ @abstract(Handling of 3D models in Wavefront OBJ format.)

  First implementation based on information from [http://www.gametutorials.com].
  Written without any particular reason --- I just saw some code doing so,
  and it seemed extremely easy, and so I decided to implement it too.
  And it works: we simply read "v", "vt" and "f" lines and that's all.

  Later extended to handle also normal vectors and materials,
  based on [http://www.fileformat.info/format/wavefrontobj/]
  and [http://www.fileformat.info/format/material/].
  Texture filename is also read from material file. }

unit Object3dOBJ;

interface

uses VectorMath, KambiUtils, Classes, KambiClassUtils, SysUtils, Boxes3d;

{$define read_interface}

type
  TWavefrontMaterial = class
    Name: string;
    AmbientColor, DiffuseColor, SpecularColor, TransmissionColor: TVector3Single;
    IlluminationModel: Cardinal;
    Opacity: Single;
    SpecularExponent: Single;
    Sharpness, IndexOfRefraction: Single;
    DiffuseTextureFileName: string;

    { Initializes material with default values.
      Since Wavefront specification doesn't say what the default values are,
      we just assign something along the lines of default VRML material. }
    constructor Create(const AName: string);
  end;

  TObjectsListItem_1 = TWavefrontMaterial;
  {$I objectslist_1.inc}
  TWavefrontMaterialsList = class(TObjectsList_1)
    { Find material with given name, @nil if not found. }
    function TryFindName(const Name: string): TWavefrontMaterial;
  end;

  TWavefrontFace = record
    VertIndices, TexCoordIndices, NormalIndices: TVector3Cardinal;
    HasTexCoords: boolean;
    HasNormals: boolean;

    { Material assigned to this face. @nil means that no material was assigned. }
    Material: TWavefrontMaterial;
  end;
  PWavefrontFace = ^TWavefrontFace;

  TDynArrayItem_1 = TWavefrontFace;
  PDynArrayItem_1 = PWavefrontFace;
  {$define DYNARRAY_1_IS_STRUCT}
  {$I dynarray_1.inc}
  TDynWavefrontFaceArray = TDynArray_1;

  { 3D model in OBJ file format. }
  TObject3dOBJ = class(TObjectBBox)
  private
    FVerts: TDynVector3SingleArray;
    FTexCoords: TDynVector2SingleArray;
    FNormals: TDynVector3SingleArray;
    FFaces: TDynWavefrontFaceArray;
    FBoundingBox: TBox3d;
    FMaterials: TWavefrontMaterialsList;
  public
    constructor Create(const fname: string);
    destructor Destroy; override;

    { @groupBegin

      Model data.

      Contents of Verts, TexCoords, Normals and Faces are read-only
      for users of this class. }
    property Verts: TDynVector3SingleArray read FVerts;
    property TexCoords: TDynVector2SingleArray read FTexCoords;
    property Normals: TDynVector3SingleArray read FNormals;
    property Faces: TDynWavefrontFaceArray read FFaces;
    { @groupEnd }

    property Materials: TWavefrontMaterialsList read FMaterials;

    function BoundingBox: TBox3d; override;
  end;

  EInvalidOBJFile = class(Exception);

{$undef read_interface}

implementation

uses KambiStringUtils, KambiFilesUtils, DataErrors;

{$define read_implementation}
{$I dynarray_1.inc}
{$I objectslist_1.inc}

{ TWavefrontMaterial --------------------------------------------------------- }

constructor TWavefrontMaterial.Create(const AName: string);
begin
  inherited Create;

  Name := AName;

  AmbientColor := Vector3Single(0.2, 0.2, 0.2);
  DiffuseColor := Vector3Single(0.8, 0.8, 0.8);
  SpecularColor := Vector3Single(0, 0, 0);

  { This is not necessarily good, I don't use it anywhere for now }
  TransmissionColor := Vector3Single(0, 0, 0);

  { Blender exported writes such illumination, I guess it's good default }
  IlluminationModel := 2;

  Opacity := 1.0;

  SpecularExponent := 1;
  Sharpness := 60;
  IndexOfRefraction := 1;

  DiffuseTextureFileName := '';
end;

{ TWavefrontMaterialsList ---------------------------------------------------- }

function TWavefrontMaterialsList.TryFindName(const Name: string):
  TWavefrontMaterial;
var
  I: Integer;
begin
  for I := 0 to Count - 1 do
  begin
    Result := Items[I];
    if Result.Name = Name then Exit;
  end;
  Result := nil;
end;

{ TObject3dOBJ --------------------------------------------------------------- }

constructor TObject3dOBJ.Create(const fname: string);
var
  BasePath: string;

  procedure ReadFacesFromOBJLine(const line: string; Material: TWavefrontMaterial);
  var
    face: TWavefrontFace;

    { Zainicjuj indeksy numer indiceNum face na podstawie VertexStr }
    procedure ReadIndices(const VertexStr: string;
      IndiceNum: integer);
    var
      VertexSeekPos: Integer;

      { Reads and updates VertexStr and VertexSeekPos, and sets
        IndiceExists and IndiceValue. IndiceExists is set to @false
        if next indice is indeed empty,
        or if we're standing at the end of VertexStr string.

        Just call it in sequence to read indices from VertexStr.
        Remember that empty indice may be followed by non-empty,
        e.g. VectorStr = '2//3' is allowed, and means that vertex
        index is 2, there's no texCoord index, and normal index is 3. }
      procedure NextIndice(out IndiceExists: boolean;
        out IndiceValue: Cardinal);
      var
        NewVertexSeekPos: Integer;
      begin
        NewVertexSeekPos := VertexSeekPos;

        while (NewVertexSeekPos <= Length(VertexStr)) and
          (VertexStr[NewVertexSeekPos] <> '/') do
          Inc(NewVertexSeekPos);

        IndiceExists := NewVertexSeekPos > VertexSeekPos;
        if IndiceExists then
          { we subtract 1, because indexed in OBJ are 1-based and we
            prefer 0-based }
          IndiceValue := StrToInt(CopyPos(
            VertexStr, VertexSeekPos, NewVertexSeekPos - 1)) - 1;

        { We add +1 to skip our ending '/' char.
          Note that we add this even if VertexSeekPos was already
          > Length(VertexStr), but this is harmless. }
        VertexSeekPos := NewVertexSeekPos + 1;
      end;

    var
      IndiceHasVertex, IndiceHasTexCoord, IndiceHasNormal: boolean;
    begin
      VertexSeekPos := 1;

      { read vertex index }
      NextIndice(IndiceHasVertex, Face.VertIndices[IndiceNum]);
      if not IndiceHasVertex then
        raise EInvalidOBJFile.CreateFmt(
          'Invalid OBJ vertex indexes "%s"', [VertexStr]);

      { read texCoord index }
      NextIndice(IndiceHasTexCoord, Face.TexCoordIndices[IndiceNum]);

      { read normal index }
      NextIndice(IndiceHasNormal, Face.NormalIndices[IndiceNum]);

      { update Face.HasXxx using IndiceHasXxx }
      Face.HasTexCoords := Face.HasTexCoords and IndiceHasTexCoord;
      Face.HasNormals := Face.HasNormals and IndiceHasNormal;
    end;

  var
    SeekPos: integer;

    function NextVertex: string;
    begin
      Result := NextToken(Line, SeekPos);
      if Result = '' then
        raise EInvalidOBJFile.CreateFmt(
          'Incomplete OBJ face specification "%s"', [Line]);
      while SCharIs(Line, SeekPos, WhiteSpaces) do Inc(SeekPos);
    end;

  begin
    { ReadIndices will eventually change this to @false, if for any
      vertex normal or texCoord will not be present. }
    Face.HasTexCoords := true;
    Face.HasNormals := true;
    Face.Material := Material;

    SeekPos := 1;

    ReadIndices(NextVertex, 0);

    { we check this, and eventually exit (without generating any face),
      because blender exporter writes 2-item faces when the mesh is
      edges-only. TODO: we should hadle 2-item faces as edges
      (and probably 1-item faces as single points?). }
    if SeekPos > Length(Line) then Exit;

    ReadIndices(NextVertex, 1);
    if SeekPos > Length(Line) then Exit;

    ReadIndices(NextVertex, 2);

    Faces.AppendItem(Face);

    while SeekPos <= Length(Line) do
    begin
      Face.VertIndices[1] := Face.VertIndices[2];
      Face.TexCoordIndices[1] := Face.TexCoordIndices[2];
      Face.NormalIndices[1] := Face.NormalIndices[2];

      ReadIndices(NextVertex, 2);

      Faces.AppendItem(Face);
    end;
  end;

  function ReadTexCoordFromOBJLine(const line: string): TVector2Single;
  var
    SeekPos: integer;
  begin
    SeekPos := 1;
    result[0] := StrToFloat(NextToken(line, SeekPos));
    result[1] := StrToFloat(NextToken(line, SeekPos));
    { nie uzywamy DeFormat - bo tex coord w OBJ moze byc 3d (z trzema
      parametrami) a my uzywamy i tak tylko dwoch pierwszych }
  end;

  { Reads single line from Wavefront OBJ or materials file.
    This takes care of stripping comments.
    If current line is empty or only comment, LineTok = ''.
    Otherwise, LineTok <> '' and LineAfterMarker contains the rest of the line. }
  procedure ReadLine(var F: TextFile; out LineTok, LineAfterMarker: string);
  var
    S: string;
    SeekPosAfterMarker: Integer;
  begin
    Readln(F, S);
    S := STruncateHash(S);

    { calculate first line token }
    SeekPosAfterMarker := 1;
    LineTok := NextToken(S, SeekPosAfterMarker);
    if LineTok <> '' then
      LineAfterMarker := Trim(SEnding(S, SeekPosAfterMarker));
  end;

  procedure ReadMaterials(const FileName: string);
  var
    IsMaterial: boolean;

    function ReadRGBColor(const Line: string): TVector3Single;
    var
      FirstToken: string;
    begin
      FirstToken := NextTokenOnce(Line);
      if SameText(FirstToken, 'xyz') or SameText(FirstToken, 'spectral') then
        { we can't interpret other colors than RGB, so we silently ignore them }
        Result := Vector3Single(1, 1, 1) else
        Result := Vector3SingleFromStr(Line);
    end;

    procedure CheckIsMaterial(const AttributeName: string);
    begin
      if not IsMaterial then
        raise EInvalidOBJFile.CreateFmt(
          'Material not named yet, but it''s %s specified', [AttributeName]);
    end;

  var
    F: TextFile;
    LineTok, LineAfterMarker: string;
  begin
    { Specification doesn't say what to do when multiple matlib directives
      encountered, i.e. when ReadMaterials is called multiple times.
      I simply add to Materials list. }

    { is some material in current file ? If false, then Materials is empty
      or last material is from some other file. }
    IsMaterial := false;

    SafeReset(F, CombinePaths(BasePath, FileName), true);
    try
      while not Eof(F) do
      begin
        ReadLine(F, LineTok, LineAfterMarker);
        if LineTok = '' then Continue;

        case ArrayPosText(LineTok, ['newmtl', 'Ka', 'Kd', 'Ks', 'Tf', 'illum',
          'd', 'Ns', 'sharpness', 'Ni', 'map_Kd']) of
          0: begin
               Materials.Add(TWavefrontMaterial.Create(LineAfterMarker));
               IsMaterial := true;
             end;
          1: begin
               CheckIsMaterial('ambient color (Ka)');
               Materials.Last.AmbientColor := ReadRGBColor(LineAfterMarker);
             end;
          2: begin
               CheckIsMaterial('diffuse color (Kd)');
               Materials.Last.DiffuseColor := ReadRGBColor(LineAfterMarker);
             end;
          3: begin
               CheckIsMaterial('specular color (Ks)');
               Materials.Last.SpecularColor := ReadRGBColor(LineAfterMarker);
             end;
          4: begin
               CheckIsMaterial('transmission filter color (Tf)');
               Materials.Last.TransmissionColor := ReadRGBColor(LineAfterMarker);
             end;
          5: begin
               CheckIsMaterial('illumination model (illum)');
               Materials.Last.IlluminationModel := StrToInt(LineAfterMarker);
             end;
          6: begin
               CheckIsMaterial('dissolve (d)');
               Materials.Last.Opacity := StrToFloat(LineAfterMarker);
             end;
          7: begin
               CheckIsMaterial('specular exponent (Ns)');
               Materials.Last.SpecularExponent := StrToFloat(LineAfterMarker);
             end;
          8: begin
               CheckIsMaterial('sharpness');
               Materials.Last.Sharpness := StrToFloat(LineAfterMarker);
             end;
          9: begin
               CheckIsMaterial('index of refraction (Ni)');
               Materials.Last.IndexOfRefraction := StrToFloat(LineAfterMarker);
             end;
          10:begin
               CheckIsMaterial('diffuse map (map_Kd)');
               Materials.Last.DiffuseTextureFileName := LineAfterMarker;
             end;
          else { we ignore other linetoks };
        end;
      end;
    finally CloseFile(F) end;
  end;

var
  F: TextFile;
  LineTok, LineAfterMarker: string;
  GroupName: string;
  UsedMaterial: TWavefrontMaterial;
begin
  inherited Create;

  BasePath := ExtractFilePath(ExpandFileName(FName));

  FVerts := TDynVector3SingleArray.Create;
  FTexCoords := TDynVector2SingleArray.Create;
  FNormals := TDynVector3SingleArray.Create;
  FFaces := TDynWavefrontFaceArray.Create;
  FMaterials := TWavefrontMaterialsList.Create;

  UsedMaterial := nil;

  SafeReset(f, fname, true);
  try
    while not Eof(f) do
    begin
      ReadLine(F, LineTok, LineAfterMarker);

      { LineTok = '' means "this line is a comment" }
      if LineTok = '' then Continue;

      { specialized token line parsing }
      case ArrayPosText(lineTok, ['v', 'vt', 'f', 'vn', 'g', 'mtllib', 'usemtl']) of
        0: Verts.AppendItem(Vector3SingleFromStr(lineAfterMarker));
        1: TexCoords.AppendItem(ReadTexCoordFromOBJLine(lineAfterMarker));
        2: ReadFacesFromOBJLine(lineAfterMarker, UsedMaterial);
        3: Normals.AppendItem(Vector3SingleFromStr(lineAfterMarker));
        4: GroupName := LineAfterMarker;
        5: ReadMaterials(LineAfterMarker);
        6: begin
             UsedMaterial := Materials.TryFindName(LineAfterMarker);
             if UsedMaterial = nil then
               DataNonFatalError(Format('Unknown material name "%s"',
                 [LineAfterMarker]));
           end;
        else { we ignore other linetoks };
      end;
    end;
  finally CloseFile(f) end;

  FBoundingBox := CalculateBoundingBox(
    PVector3Single(Verts.Items), Verts.Count, SizeOf(TVector3Single));
end;

destructor TObject3dOBJ.Destroy;
begin
  FreeAndNil(FVerts);
  FreeAndNil(FTexCoords);
  FreeAndNil(FNormals);
  FreeAndNil(FFaces);
  FreeWithContentsAndNil(FMaterials);
  inherited;
end;

function TObject3dOBJ.BoundingBox: TBox3d;
begin
  Result := FBoundingBox;
end;

end.
