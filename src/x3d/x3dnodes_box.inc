{
  Copyright 2002-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Make a box.

  @param(MaterialIndex VRML 1.0 material index, generated like for
    material binding per part. Will be used with
    material binding BIND_PER_FACE_INDEXED. Pass @nil if you don't
    need it.) }
procedure Box_Proxy(CoordIndex: TLongIntList;
  Coord: TVector3SingleList; Normal: TVector3SingleList;
  TexCoord: TVector2SingleList; MaterialIndex: TLongIntList;
  OverTriangulate: boolean; Sizes: TVector3Single;
  KambiTriangulation: TKambiTriangulationNode);

var
  Detail: TVector3Cardinal;

  { Make a rectangle that changes along the RestOf3dCoords(ConstCoord, ...).
    E.g. for ConstCoord = 0, it changes along YZ (so X below corresponds
    actually to global Y, and Y below corresponds actually to global Z).

    X1, Y1 specify the corner of the 1st vertex, we go CCW on the face
    to the (-X1, -Y1) corner. So the sign of X1, Y1 determine which side
    of the face is CCW.

    @param(TextureSIsY By default, texture S goes along X and texture T
      goes along Y. Pass TextureSIsY = @true to change it.) }
  procedure MakeRect(const ConstCoord: integer; const ConstCoordValue: Single;
    const X1, Y1: Single; const TexX1, TexY1: Single;
    const FaceMaterialIndex: Integer;
    const TextureSIsY: boolean = false);

    function MakeTexCoord(const S, T: Single): TVector2Single;
    begin
      if TextureSIsY then
      begin
        Result[0] := T;
        Result[1] := S;
      end else
      begin
        Result[0] := S;
        Result[1] := T;
      end;
    end;

  var
    FaceIndex, Index, C1, C2: integer;
    X2, Y2, TexX2, TexY2: Single;
    I, J, NewCoords, ThisIndex: Integer;
    N: TVector3Single;
  begin
    RestOf3dCoords(ConstCoord, C1, C2);

    Index := Coord.Count;
    NewCoords := (Detail[C1] + 2) * (Detail[C2] + 2);
    Coord.Count := Coord.Count + NewCoords;
    Normal.Count := Normal.Count + NewCoords;
    if TexCoord <> nil then TexCoord.Count := TexCoord.Count + NewCoords;

    X2 := -X1;
    Y2 := -Y1;
    TexX2 := 1 - TexX1;
    TexY2 := 1 - TexY1;

    { Normal vector is constant for the whole rectangle }
    if ConstCoordValue > 0 then
      N[ConstCoord] :=  1 else
      N[ConstCoord] := -1;
    N[C1] := 0;
    N[C2] := 0;

    for I := 0 to Detail[C1] + 1 do
      for J := 0 to Detail[C2] + 1 do
      begin
        ThisIndex := Index + I * (Detail[C2] + 2) + J;

        Coord.L[ThisIndex][C1] := Lerp(I / (Detail[C1] + 1), X1, X2);
        Coord.L[ThisIndex][C2] := Lerp(J / (Detail[C2] + 1), Y1, Y2);
        Coord.L[ThisIndex][ConstCoord] := ConstCoordValue;

        Normal.L[ThisIndex] := N;

        if TexCoord <> nil then
          TexCoord.L[ThisIndex] := MakeTexCoord(
            Lerp(I / (Detail[C1] + 1), TexX1, TexX2),
            Lerp(J / (Detail[C2] + 1), TexY1, TexY2));
      end;

    for I := 1 to Detail[C1] + 1 do
      for J := 1 to Detail[C2] + 1 do
      begin
        FaceIndex := CoordIndex.Count;
        CoordIndex.Count := CoordIndex.Count + 5;
        CoordIndex.L[FaceIndex    ] := Index + (I - 1) * (Detail[C2] + 2) + J - 1;
        CoordIndex.L[FaceIndex + 1] := Index +  I      * (Detail[C2] + 2) + J - 1;
        CoordIndex.L[FaceIndex + 2] := Index +  I      * (Detail[C2] + 2) + J;
        CoordIndex.L[FaceIndex + 3] := Index + (I - 1) * (Detail[C2] + 2) + J;
        CoordIndex.L[FaceIndex + 4] := -1;
      end;

    if MaterialIndex <> nil then
      MaterialIndex.AddDuplicate(FaceMaterialIndex, (Detail[C1] + 1) * (Detail[C2] + 1));
  end;

begin
  { For VRML 1.0, some of these MF fields by default have non-empty content.
    It's safest to just clean them. }
  CoordIndex.Count := 0;
  Coord.Count := 0;
  Normal.Count := 0;
  if TexCoord <> nil then TexCoord.Count := 0;
  if MaterialIndex <> nil then MaterialIndex.Count := 0;

  { Currently, all three Detail* used are always equal.
    Although the code is written to support different Detail* values,
    just in case in the future it will be useful. }
  if OverTriangulate then
    Detail[0] := KambiTriangulation.RectDivisions else
    Detail[0] := 0;
  Detail[1] := Detail[0];
  Detail[2] := Detail[0];

  Sizes /= 2;

  MakeRect(0, -Sizes[0], +Sizes[1], -Sizes[2], 1, 0, 2, true);
  MakeRect(0,  Sizes[0], -Sizes[1], -Sizes[2], 0, 1, 3, true);
  MakeRect(1, -Sizes[1], -Sizes[0], -Sizes[2], 0, 0, 5);
  MakeRect(1,  Sizes[1], +Sizes[0], -Sizes[2], 1, 1, 4);
  MakeRect(2, -Sizes[2], +Sizes[0], -Sizes[1], 0, 0, 1);
  MakeRect(2,  Sizes[2], -Sizes[0], -Sizes[1], 0, 0, 0);
end;

function TBoxNode.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
var
  CoordNode: TCoordinateNode;
  NormalNode: TNormalNode;
  TexCoordNode: TTextureCoordinateNode;
  TexCoords: TVector2SingleList;
  IFS: TIndexedFaceSetNode absolute Result;
begin
  IFS := TIndexedFaceSetNode.Create(NodeName, WWWBasePath);
  try
    CoordNode := TCoordinateNode.Create('', WWWBasePath);
    IFS.FdCoord.Value := CoordNode;

    NormalNode := TNormalNode.Create('', WWWBasePath);
    IFS.FdNormal.Value := NormalNode;
    IFS.FdNormalPerVertex.Value := true;

    if (FdTexCoord.Value <> nil) and FdTexCoord.CurrentChildAllowed then
    begin
      { No need for Box_Proxy to create tex coords. }
      IFS.FdTexCoord.Value := FdTexCoord.Value;
      TexCoords := nil;
    end else
    begin
      TexCoordNode := TTextureCoordinateNode.Create('', WWWBasePath);
      IFS.FdTexCoord.Value := TexCoordNode;
      TexCoords := TexCoordNode.FdPoint.Items;
    end;

    Box_Proxy(IFS.FdCoordIndex.Items,
      CoordNode.FdPoint.Items, NormalNode.FdVector.Items, TexCoords, nil,
      OverTriangulate, FdSize.Value,
      State.LastNodes.KambiTriangulation);

    IFS.FdSolid.Value := FdSolid.Value;

    { Smooth nothing. Not really needed, we use explicit normal node now. }
    IFS.FdCreaseAngle.Value := 0;
  except FreeAndNil(Result); raise end;
end;

function TCubeNode_1.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
var
  CoordNode: TCoordinate3Node_1;
  NormalNode: TNormalNode;
  NormalBinding: TNormalBindingNode_1;
  TexCoordNode: TTextureCoordinate2Node_1;
  ShapeHints: TShapeHintsNode_1;
  MaterialBinding: TMaterialBindingNode_1;
  MaterialIndex: TLongIntList;
  IFS: TIndexedFaceSetNode_1 absolute Result;
begin
  IFS := TIndexedFaceSetNode_1.Create(NodeName, WWWBasePath);
  try
    { we have to modify State, so make a copy of it }
    State := TVRMLGraphTraverseState.CreateCopy(State);

    CoordNode := TCoordinate3Node_1.Create('', WWWBasePath);
    State.SetLastNodes(vsCoordinate3, CoordNode, true);

    NormalNode := TNormalNode.Create('', WWWBasePath);
    State.SetLastNodes(vsNormal, NormalNode, true);

    NormalBinding := TNormalBindingNode_1.Create('', WWWBasePath);
    { NormalBinding.value = PER_VERTEX means we use niPerVertexCoordIndexed,
      so coordIndex chooses the normal. }
    NormalBinding.FdValue.Value := BIND_PER_VERTEX;
    State.SetLastNodes(vsNormalBinding, NormalBinding, true);

    TexCoordNode := TTextureCoordinate2Node_1.Create('', WWWBasePath);
    State.SetLastNodes(vsTextureCoordinate2, TexCoordNode, true);

    ShapeHints := TShapeHintsNode_1.Create('', WWWBasePath);
    { For VRML 1.0, Box is never solid. }
    ShapeHints.FdshapeType.Value := SHTYPE_UNKNOWN;
    ShapeHints.FdvertexOrdering.Value := VERTORDER_COUNTERCLOCKWISE;
    { Smooth nothing. Not really needed, we use explicit normal node now. }
    ShapeHints.FdCreaseAngle.Value := 0;
    State.SetLastNodes(vsShapeHints, ShapeHints, true);

    { calculate MaterialBinding node and MaterialIndex }
    MaterialBinding := TMaterialBindingNode_1.Create('', WWWBasePath);
    if State.LastNodes.MaterialBinding.FdValue.Value in
      [BIND_PER_PART, BIND_PER_PART_INDEXED,
       BIND_PER_FACE, BIND_PER_FACE_INDEXED] then
    begin
      MaterialIndex := IFS.FdMaterialIndex.Items;
      MaterialBinding.FdValue.Value := BIND_PER_FACE_INDEXED;
    end else
    begin
      MaterialIndex := nil;
      MaterialBinding.FdValue.Value := BIND_OVERALL;
    end;
    State.SetLastNodes(vsMaterialBinding, MaterialBinding, true);

    Box_Proxy(IFS.FdCoordIndex.Items,
      CoordNode.FdPoint.Items, NormalNode.FdVector.Items,
      TexCoordNode.FdPoint.Items, MaterialIndex,
      OverTriangulate,
      Vector3Single(FdWidth.Value, FdHeight.Value, FdDepth.Value),
      State.LastNodes.KambiTriangulation);

    { For VRML 1.0, unfortunately textureCoordIndex must be set
      (even though it's exactly equivalent to coordIndex).
      This is a problem of VRML 1.0 "state" idea --- there is no
      other way to "turn off" texture than to just use empty textureCoordIndex. }
    IFS.FdTextureCoordIndex.Items.Assign(IFS.FdCoordIndex.Items);
  except FreeAndNil(Result); raise end;
end;