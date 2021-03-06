{
  Copyright 2002-2014 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{$ifdef read_interface}
  { Common ancestor for both VRML 1.0 camera nodes and VRML/X3D >= 2.0 viewpoint
    nodes. }
  TAbstractViewpointNode = class(TAbstractBindableNode)
  private
    procedure EventSet_BindReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
  public
    procedure CreateNode; override;
    function TransformationChange: TNodeTransformationChange; override;

    private FFdOrientation: TSFRotation;
    public property FdOrientation: TSFRotation read FFdOrientation;

    private FFdDirection: TMFVec3f;
    public property FdDirection: TMFVec3f read FFdDirection;

    private FFdUp: TMFVec3f;
    public property FdUp: TMFVec3f read FFdUp;

    private FFdGravityUp: TSFVec3f;
    public property FdGravityUp: TSFVec3f read FFdGravityUp;

    private FEventCameraMatrix: TSFMatrix4fEvent;
    public property EventCameraMatrix: TSFMatrix4fEvent read FEventCameraMatrix;

    private FEventCameraInverseMatrix: TSFMatrix4fEvent;
    public property EventCameraInverseMatrix: TSFMatrix4fEvent read FEventCameraInverseMatrix;

    private FEventCameraRotationMatrix: TSFMatrix3fEvent;
    public property EventCameraRotationMatrix: TSFMatrix3fEvent read FEventCameraRotationMatrix;

    private FEventCameraRotationInverseMatrix: TSFMatrix3fEvent;
    public property EventCameraRotationInverseMatrix: TSFMatrix3fEvent read FEventCameraRotationInverseMatrix;

    private FFdCameraMatrixSendAlsoOnOffscreenRendering: TSFBool;
    public property FdCameraMatrixSendAlsoOnOffscreenRendering: TSFBool read FFdCameraMatrixSendAlsoOnOffscreenRendering;

    { Position of the viewpoint.

      This is an abstract function, not a simple FdPosition, because one of
      the descendants (TAbstractX3DViewpointNode) doesn't have FdPosition field.
      For now, all non-abstract descendants (for both VRML 1.0 and VRML >= 2.0)
      override this and expose their position field correctly. }
    function Position: TSFVec3f; virtual; abstract;

    class function ProjectionType: TProjectionType; virtual; abstract;

    { Calculate camera vectors (position, direction, up, gravity up).
      Follows VRML/X3D specification:

      @unorderedList(
        @item(position is taken directly from FdPosition field,)
        @item(direction and up are (respectively) -Z and +Y rotated by FdOrientation,)
        @item(gravity up is +Y.)
      )

      They are all then transformed by the current viewpoint transformation
      (determined by parent nodes like Transform).

      One conclusion from the above is that the only way to change the gravity up
      vector (this determines in which direction viewer falls down)
      is to use the Transform node around the viewpoint node.

      Additionally, as an extension, we also look at FdDirection and FdUp
      and FdGravityUp vectors. See
      http://castle-engine.sourceforge.net/x3d_extensions.php#section_ext_cameras_alt_orient

      Returned CamDir, CamUp, GravityUp are @italic(always normalized). }
    procedure GetView(out CamPos, CamDir, CamUp, GravityUp: TVector3Single);

    private FFdDescription: TSFString;
    public property FdDescription: TSFString read FFdDescription;

    { Description generated smart (trying to use FdDescription field,
      falling back on other information to help user identify the node). }
    function Description: string; virtual;
  end;

  TX3DViewpointClassNode = class of TAbstractViewpointNode;

  TAbstractX3DViewpointNode = class(TAbstractViewpointNode)
  public
    procedure CreateNode; override;

    private FFdCenterOfRotation: TSFVec3f;
    public property FdCenterOfRotation: TSFVec3f read FFdCenterOfRotation;

    private FFdJump: TSFBool;
    public property FdJump: TSFBool read FFdJump;

    private FFdRetainUserOffsets: TSFBool;
    public property FdRetainUserOffsets: TSFBool read FFdRetainUserOffsets;

    { Matrices for projecting texture from this viewpoint,
      for ProjectedTextureCoordinate.
      Override ProjectionMatrix for subclasses (ModelviewMatrix
      is already correctly defined here).
      @groupBegin }
    function ProjectionMatrix: TMatrix4Single; virtual;
    function ModelviewMatrix: TMatrix4Single;
    function GetProjectorMatrix: TMatrix4Single;
    { @groupEnd }
  end;

  TBillboardNode = class(TAbstractX3DGroupingNode, ITransformNode)
  private
    FCameraPosition, FCameraDirection, FCameraUp: TVector3Single;
    FCameraViewKnown: boolean;
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
    procedure ApplyTransform(State: TX3DGraphTraverseState); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
    function TransformationChange: TNodeTransformationChange; override;

    property CameraViewKnown: boolean read FCameraViewKnown;
    property CameraPosition: TVector3Single read FCameraPosition;
    property CameraDirection: TVector3Single read FCameraDirection;
    property CameraUp: TVector3Single read FCameraUp;

    procedure CameraChanged(const APosition, ADirection, AUp: TVector3Single);

    private FFdAxisOfRotation: TSFVec3f;
    public property FdAxisOfRotation: TSFVec3f read FFdAxisOfRotation;
  end;

  TCollisionNode = class(TAbstractX3DGroupingNode, IAbstractSensorNode)
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
    function DirectEnumerateActiveForTraverse(
      Func: TEnumerateChildrenFunction;
      StateStack: TX3DGraphTraverseStateStack): Pointer; override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdEnabled: TSFBool;
    public property FdEnabled: TSFBool read FFdEnabled;

    { Event out } { }
    private FEventCollideTime: TSFTimeEvent;
    public property EventCollideTime: TSFTimeEvent read FEventCollideTime;

    { Event out } { }
    private FEventIsActive: TSFBoolEvent;
    public property EventIsActive: TSFBoolEvent read FEventIsActive;

    private FFdProxy: TSFNode;
    public property FdProxy: TSFNode read FFdProxy;
  end;

  { LOD node for VRML/X3D >= 2.0. This is not for VRML <= 1.0 LOD node.

    It's a common ancestor for VRML 2.0 LOD (TLODNode_2) and X3D LOD (TLODNode).
    Unfortunately, we cannot have a simple common class for both VRML 97
    and X3D because there would be a name clash for "level_changed" event:

    @unorderedList(
      @item(
        For VRML 2.0, main MFNode field was named "level" and so "level_changed"
        is an event reporting when MFNode changed.)

      @item(For X3D, main MFNode field is named "children", and so "children_changed"
        reports MFNode changes. "level_changed" is a new field, SFInt32,
        indicating which child is chosen.)
    )

    So level_changed has completely different meanings for VRML 97 and X3D.
    As an extension we'll add "levelIndex_changed", SFInt32, to be analogous
    to X3D "level_changed". This way both VRML 2.0 and X3D LOD nodes have
    the same capabilities, and common interface for programmer
    (programmer should use X3D event/fields names for Pascal property names),
    but for parser they will use different names.
  }
  TAbstractLODNode = class(TAbstractX3DGroupingNode)
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;

    private FFdCenter: TSFVec3f;
    public property FdCenter: TSFVec3f read FFdCenter;

    private FFdRange: TMFFloat;
    public property FdRange: TMFFloat read FFdRange;

    private FFdForceTransitions: TSFBool;
    public property FdForceTransitions: TSFBool read FFdForceTransitions;

    { Event out } { }
    private FEventLevel_changed: TSFInt32Event;
    public property EventLevel_changed: TSFInt32Event read FEventLevel_changed;

    function TransformationChange: TNodeTransformationChange; override;
  end;

  TLODNode_2 = class(TAbstractLODNode)
  public
    procedure CreateNode; override;

    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;
    class function URNMatching(const URN: string): boolean; override;
  end;

  TLODNode = class(TAbstractLODNode)
  public
    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;
    class function URNMatching(const URN: string): boolean; override;
  end;
  TLODNode_3 = TLODNode;

  TOptionalBlendingSort = (obsDefault, obsNone, obs2D, obs3D);

  TNavigationInfoNode = class(TAbstractBindableNode)
  private
    procedure EventSet_BindReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    function GetBlendingSort: TOptionalBlendingSort;
    procedure SetBlendingSort(const Value: TOptionalBlendingSort);
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdAvatarSize: TMFFloat;
    public property FdAvatarSize: TMFFloat read FFdAvatarSize;

    private FFdHeadlight: TSFBool;
    public property FdHeadlight: TSFBool read FFdHeadlight;

    private FFdSpeed: TSFFloat;
    public property FdSpeed: TSFFloat read FFdSpeed;

    private FFdTransitionTime: TSFTime;
    public property FdTransitionTime: TSFTime read FFdTransitionTime;

    private FFdTransitionType: TMFString;
    public property FdTransitionType: TMFString read FFdTransitionType;

    private FFdType: TMFString;
    public property FdType: TMFString read FFdType;

    private FFdVisibilityLimit: TSFFloat;
    public property FdVisibilityLimit: TSFFloat read FFdVisibilityLimit;

    private FFdBlendingSort: TSFStringEnum;
    public property FdBlendingSort: TSFStringEnum read FFdBlendingSort;
    property BlendingSort: TOptionalBlendingSort
      read GetBlendingSort write SetBlendingSort;

    { Event out } { }
    private FEventTransitionComplete: TSFBoolEvent;
    public property EventTransitionComplete: TSFBoolEvent read FEventTransitionComplete;
  end;

  TOrthoViewpointNode = class(TAbstractX3DViewpointNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdFieldOfView: TMFFloat;
    public property FdFieldOfView: TMFFloat read FFdFieldOfView;

    private FFdPosition: TSFVec3f;
    public property FdPosition: TSFVec3f read FFdPosition;

    function Position: TSFVec3f; override;
    class function ProjectionType: TProjectionType; override;
    function ProjectionMatrix: TMatrix4Single; override;

    { Fix given field of view value for window aspect ratio.
      The idea is that OrthoViewpoint.fieldOfView specifies the minimal
      extents. Depending on your window aspect ratio, you may need to make
      one extent (vertical or horizontal) larger to adjust. }
    class procedure AspectFieldOfView(
      var FieldOfView: TVector4Single;
      const WindowWidthToHeight: Single);
  end;

  TViewpointNode = class(TAbstractX3DViewpointNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdFieldOfView: TSFFloat;
    public property FdFieldOfView: TSFFloat read FFdFieldOfView;

    private FFdPosition: TSFVec3f;
    public property FdPosition: TSFVec3f read FFdPosition;

    function Position: TSFVec3f; override;
    class function ProjectionType: TProjectionType; override;

    { This calculates proper angle of view for typical rectangular
      display, based on given fieldOfView field value.
      Result is in radians (just like fieldOfView VRML field).

      If you want to calculate horizontal angle of view then
      pass as ThisToOtherSizeRatio your window's width / height.
      If you want to calculate vertical angle of view then
      pass as ThisToOtherSizeRatio your window's height / width.
      For this method it doesn't really matter which is horizontal
      and which is vertical, both are treated the same.

      This works following VRML spec. So the angle of view for
      smaller window size is set to fieldOfViee. The other angle
      can always be calculated by AdjustViewAngleRadToAspectRatio
      (this implements the same equation that is mentioned in VRML spec).
      The larger angle cannot be larger than Pi, and may force the
      smaller angle to be smaller than fieldOfView. }
    function AngleOfView(const ThisToOtherSizeRatio: Single): Single;

    { This is like AngleOfView, but it allows you to specify
      FieldOfView as a parameter. }
    class function ViewpointAngleOfView(
      FieldOfView: Single;
      const ThisToOtherSizeRatio: Single): Single;
    function ProjectionMatrix: TMatrix4Single; override;
  end;

  TViewpointGroupNode = class(TAbstractChildNode)
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdCenter: TSFVec3f;
    public property FdCenter: TSFVec3f read FFdCenter;

    private FFdChildren: TMFNode;
    public property FdChildren: TMFNode read FFdChildren;

    private FFdDescription: TSFString;
    public property FdDescription: TSFString read FFdDescription;

    private FFdDisplayed: TSFBool;
    public property FdDisplayed: TSFBool read FFdDisplayed;

    private FFdRetainUserOffsets: TSFBool;
    public property FdRetainUserOffsets: TSFBool read FFdRetainUserOffsets;

    private FFdSize: TSFVec3f;
    public property FdSize: TSFVec3f read FFdSize;

    { Description generated smart (trying to use FdDescription field,
      falling back on other information to help user identify the node). }
    function Description: string;
  end;
{$endif read_interface}

{$ifdef read_implementation}
procedure TAbstractViewpointNode.CreateNode;
begin
  inherited;

  { In X3D, this is part of X3DViewpointNode.
    In our engine, X3DViewpointNode descends from TAbstractViewpointNode
    inheriting "orientation" field this way. }
  FFdOrientation := TSFRotation.Create(Self, 'orientation', Vector3Single(0, 0, 1), 0);
   FdOrientation.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdOrientation);
  { X3D specification comment: [-1,1],(-Inf,Inf) }

  FFdDirection := TMFVec3f.Create(Self, 'direction', []);
   FdDirection.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdDirection);

  FFdUp := TMFVec3f.Create(Self, 'up', []);
   FdUp.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdUp);

  FFdGravityUp := TSFVec3f.Create(Self, 'gravityUp', DefaultX3DGravityUp);
   FdGravityUp.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdGravityUp);

  FEventCameraMatrix := TSFMatrix4fEvent.Create(Self, 'cameraMatrix', false);
  Events.Add(FEventCameraMatrix);

  FEventCameraInverseMatrix := TSFMatrix4fEvent.Create(Self, 'cameraInverseMatrix', false);
  Events.Add(FEventCameraInverseMatrix);

  FEventCameraRotationMatrix := TSFMatrix3fEvent.Create(Self, 'cameraRotationMatrix', false);
  Events.Add(FEventCameraRotationMatrix);

  FEventCameraRotationInverseMatrix := TSFMatrix3fEvent.Create(Self, 'cameraRotationInverseMatrix', false);
  Events.Add(FEventCameraRotationInverseMatrix);

  FFdCameraMatrixSendAlsoOnOffscreenRendering := TSFBool.Create(Self, 'cameraMatrixSendAlsoOnOffscreenRendering', false);
  Fields.Add(FFdCameraMatrixSendAlsoOnOffscreenRendering);

  FFdDescription := TSFString.Create(Self, 'description', '');
  Fields.Add(FFdDescription);

  Eventset_bind.OnReceive.Add(@EventSet_BindReceive);
end;

procedure TAbstractViewpointNode.GetView(
  out CamPos, CamDir, CamUp, GravityUp: TVector3Single);
begin
  CamPos := Position.Value;

  if FdDirection.Items.Count > 0 then
  begin
    CamDir := FdDirection.Items.L[0];
    if ZeroVector(CamDir) then
    begin
      OnWarning(wtMajor, 'VRML/X3D', 'Viewpoint "direction" must not be zero, assuming defaults');
      CamDir := FdOrientation.RotatedPoint( DefaultX3DCameraDirection );
    end;
  end else
    CamDir := FdOrientation.RotatedPoint( DefaultX3DCameraDirection );

  if FdUp.Items.Count > 0 then
  begin
    CamUp := FdUp.Items.L[0];
    if ZeroVector(CamUp) then
    begin
      OnWarning(wtMajor, 'VRML/X3D', 'Viewpoint "up" must not be zero, assuming defaults');
      CamUp := FdOrientation.RotatedPoint( DefaultX3DCameraUp );
    end;
  end else
    CamUp := FdOrientation.RotatedPoint( DefaultX3DCameraUp );

  GravityUp := FdGravityUp.Value;
  if ZeroVector(GravityUp) then
    GravityUp := DefaultX3DGravityUp;

  { Niestety, macierz ponizej moze cos skalowac wiec nawet jesli powyzej
    uzylismy FdOrientation.RotatedPoint( DefaultX3DCameraDirection/Up ) i wiemy ze CamDir/Up
    jest znormalizowane - to i tak musimy je tutaj znormalizowac.
    TODO: byloby dobrze uzyc tutaj czegos jak MatrixMultDirectionNoScale }
  CamPos    := MatrixMultPoint(Transform, CamPos);
  CamDir    := Normalized( MatrixMultDirection(Transform, CamDir) );
  CamUp     := Normalized( MatrixMultDirection(Transform, CamUp) );
  GravityUp := Normalized( MatrixMultDirection(Transform, GravityUp) );

  Assert(FloatsEqual(VectorLenSqr(CamDir), 1.0, 0.0001));
  Assert(FloatsEqual(VectorLenSqr(CamUp), 1.0, 0.0001));
end;

procedure TAbstractViewpointNode.EventSet_BindReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if Scene <> nil then
    Scene.GetViewpointStack.Set_Bind(Self, (Value as TSFBool).Value);
end;

function TAbstractViewpointNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcViewpoint;
end;

function TAbstractViewpointNode.Description: string;
begin
  Result := FdDescription.Value;
  { if node doesn't have a "description" field, or it's left empty, use node name }
  if Result = '' then
    Result := NodeName;
  { if even the node name is empty, just show node type. }
  if Result = '' then
    Result := NodeTypeName;
end;

procedure TAbstractX3DViewpointNode.CreateNode;
begin
  inherited;

  FFdCenterOfRotation := TSFVec3f.Create(Self, 'centerOfRotation', Vector3Single(0, 0, 0));
  Fields.Add(FFdCenterOfRotation);
  { X3D specification comment: (-Inf,Inf) }

  FFdJump := TSFBool.Create(Self, 'jump', true);
  Fields.Add(FFdJump);

  FFdRetainUserOffsets := TSFBool.Create(Self, 'retainUserOffsets', false);
  Fields.Add(FFdRetainUserOffsets);

  DefaultContainerField := 'children';
end;

function TAbstractX3DViewpointNode.ProjectionMatrix: TMatrix4Single;
begin
  Result := IdentityMatrix4Single;
end;

function TAbstractX3DViewpointNode.ModelviewMatrix: TMatrix4Single;
var
  CamPos, CamDir, CamUp, CamGravityUp: TVector3Single;
begin
  GetView(CamPos, CamDir, CamUp, CamGravityUp);
  Result := LookDirMatrix(CamPos, CamDir, CamUp);
end;

function TAbstractX3DViewpointNode.GetProjectorMatrix: TMatrix4Single;
begin
  Result := ProjectionMatrix * ModelviewMatrix;
end;

procedure TBillboardNode.CreateNode;
begin
  inherited;

  FFdAxisOfRotation := TSFVec3f.Create(Self, 'axisOfRotation', Vector3Single(0, 1, 0));
  Fields.Add(FFdAxisOfRotation);
  { X3D specification comment: (-Inf,Inf) }

  DefaultContainerField := 'children';
end;

class function TBillboardNode.ClassNodeTypeName: string;
begin
  Result := 'Billboard';
end;

class function TBillboardNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TBillboardNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
begin
  Result := FdChildren.SearchValid(Func);
  if Result <> nil then Exit;
end;

procedure TBillboardNode.ApplyTransform(State: TX3DGraphTraverseState);
var
  NewX, NewY, NewZ, BillboardToViewer, P1, P2, LocalDirection, LocalUp: TVector3Single;
  Angle: Single;
  M, IM: TMatrix4Single;
  PlaneAxis: TVector4Single;
  PlaneAxisDir: TVector3Single absolute PlaneAxis;
begin
  if CameraViewKnown then
  begin
    if PerfectlyZeroVector(FdAxisOfRotation.Value) then
    begin
      LocalDirection := MatrixMultDirection(State.InvertedTransform, FCameraDirection);
      LocalUp        := MatrixMultDirection(State.InvertedTransform, FCameraUp);
      { although FCameraDirection/Up are for sure normalized and orthogonal,
        but State.InvertedTransform may contain scaling,
        so be sure to normalize again the result.
        For safety, also call MakeVectorsOrthoOnTheirPlane. }
      NormalizeVar(LocalDirection);
      NormalizeVar(LocalUp);
      MakeVectorsOrthoOnTheirPlane(LocalDirection, LocalUp);

      NewX := VectorProduct(LocalDirection, LocalUp);
      NewY := LocalUp;
      NewZ := -LocalDirection;

      TransformCoordsMatrices(NewX, NewY, NewZ, M, IM);
      State.Transform := MatrixMult(State.Transform, M);
      State.InvertedTransform := MatrixMult(IM, State.InvertedTransform);
    end else
    begin
      { vector from node origin to CameraPosition, in local coords }
      BillboardToViewer := MatrixMultPoint(State.InvertedTransform, CameraPosition);

      { plane of axisOfRotation }
      PlaneAxisDir := FdAxisOfRotation.Value;
      PlaneAxis[3] := 0;

      { we want to have a rotation that changes UnitVector3Single[2]
        into BillboardToViewer. But the rotation axis is fixed
        to axisOfRotation (we cannot just take their VectorProduct).
        So project both points on a plane orthogonal to axisOfRotation,
        and calculate angle there. }
      P1 := PointOnPlaneClosestToPoint(PlaneAxis, UnitVector3Single[2]);
      P2 := PointOnPlaneClosestToPoint(PlaneAxis, BillboardToViewer);

      if { axisOfRotation paralell to Z axis? Then nothing sensible to do. }
        ZeroVector(P1) or
        { billboard-to-viewer vector parallel to axisOfRotation (includes
          the case when billboard-to-viewer vector is zero in local coords,
          which means that camera standing at Billboard origin)?
          Then nothing sensible to do. }
        ZeroVector(P2) then
        Exit;

      Angle := RotationAngleRadBetweenVectors(P1, P2, FdAxisOfRotation.Value);

      RotationMatricesRad(Angle, FdAxisOfRotation.Value, M, IM);
      State.Transform := MatrixMult(State.Transform, M);
      State.InvertedTransform := MatrixMult(IM, State.InvertedTransform);
    end;
  end;
end;

procedure TBillboardNode.CameraChanged(
  const APosition, ADirection, AUp: TVector3Single);
begin
  FCameraViewKnown := true;
  FCameraPosition := APosition;
  FCameraDirection := ADirection;
  FCameraUp := AUp;
end;

function TBillboardNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcTransform;
end;

procedure TCollisionNode.CreateNode;
begin
  inherited;

  FFdEnabled := TSFBool.Create(Self, 'enabled', true);
  { In VRML 2.0, Collision didn't descent from X3DSensorName and had
    special field "collide". In X3D, "enabled" is used for the exact
    same purpose. }
   FdEnabled.AddAlternativeName('collide', 2);
   FdEnabled.ChangesAlways := [chEverything];
  Fields.Add(FFdEnabled);

  FEventCollideTime := TSFTimeEvent.Create(Self, 'collideTime', false);
  Events.Add(FEventCollideTime);

  FEventIsActive := TSFBoolEvent.Create(Self, 'isActive', false);
  Events.Add(FEventIsActive);

  FFdProxy := TSFNode.Create(Self, 'proxy', IAbstractChildNode);
   FdProxy.Exposed := false;
   FdProxy.ChangesAlways := [chEverything];
  Fields.Add(FFdProxy);

  DefaultContainerField := 'children';
end;

class function TCollisionNode.ClassNodeTypeName: string;
begin
  Result := 'Collision';
end;

class function TCollisionNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TCollisionNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
begin
  Result := FdProxy.SearchValid(Func);
  if Result <> nil then Exit;

  Result := FdChildren.SearchValid(Func);
  if Result <> nil then Exit;
end;

function TCollisionNode.DirectEnumerateActiveForTraverse(
  Func: TEnumerateChildrenFunction;
  StateStack: TX3DGraphTraverseStateStack): Pointer;
begin
  Result := nil;
  if FdEnabled.Value then
  begin
    if FdProxy.Value = nil then
    begin
      { Collision node doesn't do anything in this trivial case,
        children are treated just like by Group. }
      Result := FdChildren.SearchValid(Func);
      if Result <> nil then Exit;
    end else
    begin
      { This is the interesting case:
        proxy is not visible,
        children are not collidable. }

      Inc(StateStack.Top.InsideInvisible);
      try
        Result := FdProxy.SearchValid(Func);
        if Result <> nil then Exit;
      finally Dec(StateStack.Top.InsideInvisible) end;

      Inc(StateStack.Top.InsideIgnoreCollision);
      try
        Result := FdChildren.SearchValid(Func);
        if Result <> nil then Exit;
      finally Dec(StateStack.Top.InsideIgnoreCollision) end;
    end;
  end else
  begin
    { Nothing is collidable in this case. So proxy is just ignored. }
    Inc(StateStack.Top.InsideIgnoreCollision);
    try
      Result := FdChildren.SearchValid(Func);
      if Result <> nil then Exit;
    finally Dec(StateStack.Top.InsideIgnoreCollision) end;
  end;
end;

procedure TAbstractLODNode.CreateNode;
begin
  inherited;

  FFdCenter := TSFVec3f.Create(Self, 'center', ZeroVector3Single);
   FdCenter.Exposed := false;
   { Just redisplay, and new appropriate LOD children will be displayed. }
   FdCenter.ChangesAlways := [chRedisplay];
  Fields.Add(FFdCenter);

  FFdRange := TMFFloat.Create(Self, 'range', []);
   FdRange.Exposed := false;
   { Just redisplay, and new appropriate LOD children will be displayed. }
   FdRange.ChangesAlways := [chRedisplay];
  Fields.Add(FFdRange);
  { X3D specification comment: [0,Inf) or -1 }

  FFdForceTransitions := TSFBool.Create(Self, 'forceTransitions', false);
  FFdForceTransitions.Exposed := false;
  Fields.Add(FFdForceTransitions);

  FEventLevel_changed := TSFInt32Event.Create(Self, 'level_changed', false);
  Events.Add(FEventLevel_changed);

  DefaultContainerField := 'children';
end;

class function TAbstractLODNode.ClassNodeTypeName: string;
begin
  Result := 'LOD';
end;

function TAbstractLODNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
begin
  Result := nil;
  { For now we simply always use the best LOD version,
    avoiding whole issue of choosing proper LOD child. }
  if FdChildren.Items.Count >= 1 then
  begin
    Result := Func(Self, FdChildren[0]);
    if Result <> nil then Exit;
  end;
end;

function TAbstractLODNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcLOD;
end;

procedure TLODNode_2.CreateNode;
begin
  inherited;
  FdChildren.AddAlternativeName('level', 2);
  Eventlevel_changed.AddAlternativeName('levelIndex_changed', 2);
end;

class function TLODNode_2.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

class function TLODNode_2.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major = 2;
end;

class function TLODNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TLODNode.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major >= 3;
end;

procedure TNavigationInfoNode.CreateNode;
const
  BlendingSortNames: array [TOptionalBlendingSort] of string =
  ('DEFAULT', 'NONE', '2D', '3D');
begin
  inherited;

  FFdAvatarSize := TMFFloat.Create(Self, 'avatarSize', [0.25, 1.6, 0.75]);
   FdAvatarSize.ChangesAlways := [chNavigationInfo];
  Fields.Add(FFdAvatarSize);
  { X3D specification comment: [0,Inf) }

  FFdHeadlight := TSFBool.Create(Self, 'headlight', true);
   FdHeadlight.ChangesAlways := [chHeadLightOn];
  Fields.Add(FFdHeadlight);

  FFdSpeed := TSFFloat.Create(Self, 'speed', 1.0);
   FdSpeed.ChangesAlways := [chNavigationInfo];
  Fields.Add(FFdSpeed);
  { X3D specification comment: [0,Inf) }

  FFdTransitionTime := TSFTime.Create(Self, 'transitionTime', 1.0);
  Fields.Add(FFdTransitionTime);
  { X3D specification comment: [0, Inf) }

  FFdTransitionType := TMFString.Create(Self, 'transitionType', ['LINEAR']);
  Fields.Add(FFdTransitionType);
  { X3D specification comment: ["TELEPORT","LINEAR","ANIMATE",...] }

  { TODO: default value was ["WALK", "ANY"] in VRML 97.
    X3D changed default value. }
  FFdType := TMFString.Create(Self, 'type', ['EXAMINE', 'ANY']);
   FdType.ChangesAlways := [chNavigationInfo];
  Fields.Add(FFdType);
  { X3D specification comment: ["ANY","WALK","EXAMINE","FLY","LOOKAT","NONE",...] }

  FFdVisibilityLimit := TSFFloat.Create(Self, 'visibilityLimit', 0.0);
  Fields.Add(FFdVisibilityLimit);
  { X3D specification comment: [0,Inf) }

  FEventTransitionComplete := TSFBoolEvent.Create(Self, 'transitionComplete', false);
  Events.Add(FEventTransitionComplete);

  FFdBlendingSort := TSFStringEnum.Create(Self, 'blendingSort', BlendingSortNames, Ord(obsDefault));
   FdBlendingSort.ChangesAlways := [];
  Fields.Add(FFdBlendingSort);

  DefaultContainerField := 'children';

  Eventset_bind.OnReceive.Add(@EventSet_BindReceive);
end;

class function TNavigationInfoNode.ClassNodeTypeName: string;
begin
  Result := 'NavigationInfo';
end;

class function TNavigationInfoNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TNavigationInfoNode.EventSet_BindReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if Scene <> nil then
    Scene.GetNavigationInfoStack.Set_Bind(Self, (Value as TSFBool).Value);
end;

function TNavigationInfoNode.GetBlendingSort: TOptionalBlendingSort;
begin
  Result := TOptionalBlendingSort(FdBlendingSort.EnumValue);
end;

procedure TNavigationInfoNode.SetBlendingSort(const Value: TOptionalBlendingSort);
begin
  FdBlendingSort.SendEnumValue(Ord(Value));
end;

procedure TOrthoViewpointNode.CreateNode;
begin
  inherited;

  FFdFieldOfView := TMFFloat.Create(Self, 'fieldOfView', [-1, -1, 1, 1]);
   FdFieldOfView.ChangesAlways := [chViewpointProjection];
  Fields.Add(FFdFieldOfView);
  { X3D specification comment:  (-Inf,Inf) }

  FFdPosition := TSFVec3f.Create(Self, 'position', Vector3Single(0, 0, 10));
   FdPosition.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdPosition);
  { X3D specification comment: (-Inf,Inf) }

  DefaultContainerField := 'children';
end;

class function TOrthoViewpointNode.ClassNodeTypeName: string;
begin
  Result := 'OrthoViewpoint';
end;

class function TOrthoViewpointNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TOrthoViewpointNode.Position: TSFVec3f;
begin
  Result := FdPosition;
end;

class function TOrthoViewpointNode.ProjectionType: TProjectionType;
begin
  Result := ptOrthographic;
end;

function TOrthoViewpointNode.ProjectionMatrix: TMatrix4Single;
var
  OrthoDimensions: TVector4Single;
begin
  { default OrthoDimensions, for OrthoViewpoint }
  OrthoDimensions[0] := -1;
  OrthoDimensions[1] := -1;
  OrthoDimensions[2] :=  1;
  OrthoDimensions[3] :=  1;

  if FdFieldOfView.Items.Count > 0 then OrthoDimensions[0] := FdFieldOfView.Items[0];
  if FdFieldOfView.Items.Count > 1 then OrthoDimensions[1] := FdFieldOfView.Items[1];
  if FdFieldOfView.Items.Count > 2 then OrthoDimensions[2] := FdFieldOfView.Items[2];
  if FdFieldOfView.Items.Count > 3 then OrthoDimensions[3] := FdFieldOfView.Items[3];

  { TODO: for currently bound viewpoint, we should honour
    fieldOfView and aspect ratio of current window,
    by calling AspectFieldOfView. }

  Result := OrthoProjMatrix(
    { Beware: order of OrthoViewpoint.fieldOfView and OrthoDimensions
      is different than typical OpenGL and our OrthoProjMatrix params. }
    OrthoDimensions[0],
    OrthoDimensions[2],
    OrthoDimensions[1],
    OrthoDimensions[3],
    1, 100); { TODO: near, far projection testing values }
end;

class procedure TOrthoViewpointNode.AspectFieldOfView(
  var FieldOfView: TVector4Single; const WindowWidthToHeight: Single);

  { Scale the extent. Since AspectFieldOfView should only make FieldOfView
    larger (because OrthoViewpoint.fieldOfView gives the minimal extents),
    so given here Scale should always be >= 1. }
  procedure ScaleExtent(const Scale: Single; var Min, Max: Single);
  var
    L, Middle: Single;
  begin
    Middle := (Min + Max) / 2;
    L := Max - Min;

    if L < 0 then
    begin
      if Log then
        WritelnLog('OrthoViewpoint', 'OrthoViewpoint.fieldOfView max extent smaller than min extent');
      Exit;
    end;

    Min := Middle - Scale * L / 2;
    Max := Middle + Scale * L / 2;
  end;

var
  FOVAspect: Single;
begin
  if FloatsEqual(FieldOfView[2], FieldOfView[0]) or
     FloatsEqual(FieldOfView[3], FieldOfView[1]) then
  begin
    if Log then
      WritelnLog('OrthoViewpoint', 'OrthoViewpoint.fieldOfView extent (max-min) is zero');
    Exit;
  end;

  FOVAspect :=
    (FieldOfView[2] - FieldOfView[0]) /
    (FieldOfView[3] - FieldOfView[1]);

  { The idea is to change FieldOfView, such that at the end the above
    equation would calculate FOVAspect as equal to WindowWidthToHeight.

    To do this, multiply above equation by WindowWidthToHeight / FOVAspect.
    We have to transform put this scale into either horizontal or vertical
    extent, since we only want to make FieldOfView larger (never smaller). }

  if FOVAspect > WindowWidthToHeight then
  begin
    ScaleExtent(FOVAspect / WindowWidthToHeight, FieldOfView[1], FieldOfView[3]);
  end else
  if FOVAspect < WindowWidthToHeight then
  begin
    ScaleExtent(WindowWidthToHeight / FOVAspect, FieldOfView[0], FieldOfView[2]);
  end;
end;

procedure TViewpointNode.CreateNode;
begin
  inherited;

  FFdFieldOfView := TSFFloat.Create(Self, 'fieldOfView', DefaultViewpointFieldOfView);
   FdFieldOfView.ChangesAlways := [chViewpointProjection];
  Fields.Add(FFdFieldOfView);
  { X3D specification comment: (0,Pi) }

  FFdPosition := TSFVec3f.Create(Self, 'position', Vector3Single(0, 0, 10));
   FdPosition.ChangesAlways := [chViewpointVectors];
  Fields.Add(FFdPosition);
  { X3D specification comment: (-Inf,Inf) }

  DefaultContainerField := 'children';
end;

class function TViewpointNode.ClassNodeTypeName: string;
begin
  Result := 'Viewpoint';
end;

class function TViewpointNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TViewpointNode.Position: TSFVec3f;
begin
  Result := FdPosition;
end;

class function TViewpointNode.ProjectionType: TProjectionType;
begin
  Result := ptPerspective;
end;

function TViewpointNode.AngleOfView(
  const ThisToOtherSizeRatio: Single): Single;
begin
  Result := ViewpointAngleOfView(FdFieldOfView.Value, ThisToOtherSizeRatio);
end;

class function TViewpointNode.ViewpointAngleOfView(
  FieldOfView: Single;
  const ThisToOtherSizeRatio: Single): Single;
var
  OtherAngle: Single;
begin
  ClampVar(FieldOfView, 0.01, Pi - 0.01);

  if ThisToOtherSizeRatio < 1 then
  begin
    { So the resulting angle is the smaller one. }
    Result := FieldOfView;
    OtherAngle :=
      AdjustViewAngleRadToAspectRatio(Result, 1 / ThisToOtherSizeRatio);
    if OtherAngle > Pi then
      Result := AdjustViewAngleRadToAspectRatio(Pi, ThisToOtherSizeRatio);
  end else
  begin
    { So the resulting angle is the larger one. }
    OtherAngle := FieldOfView;
    Result :=
      AdjustViewAngleRadToAspectRatio(OtherAngle, ThisToOtherSizeRatio);
    if Result > Pi then
      Result := Pi;
  end;
end;

function TViewpointNode.ProjectionMatrix: TMatrix4Single;
begin
  { TODO: for currently bound viewpoint, we should honour
    fieldOfView and aspect ratio of current window? }
  Result := PerspectiveProjMatrixRad(
    FdFieldOfView.Value, 1,
    1, 100); { TODO: near, far projection testing values }
end;

procedure TViewpointGroupNode.CreateNode;
begin
  inherited;

  FFdCenter := TSFVec3f.Create(Self, 'center', Vector3Single(0, 0, 0));
  Fields.Add(FFdCenter);
  { X3D specification comment: (-Inf,Inf) }

  FFdChildren := TMFNode.Create(Self, 'children', [TAbstractX3DViewpointNode, TViewpointGroupNode]);
  Fields.Add(FFdChildren);

  FFdDescription := TSFString.Create(Self, 'description', '');
  Fields.Add(FFdDescription);

  FFdDisplayed := TSFBool.Create(Self, 'displayed', true);
  Fields.Add(FFdDisplayed);

  FFdRetainUserOffsets := TSFBool.Create(Self, 'retainUserOffsets', false);
  Fields.Add(FFdRetainUserOffsets);

  FFdSize := TSFVec3f.Create(Self, 'size', Vector3Single(0, 0, 0));
  Fields.Add(FFdSize);
  { X3D specification comment: (-Inf,Inf) }

  DefaultContainerField := 'children';
end;

class function TViewpointGroupNode.ClassNodeTypeName: string;
begin
  Result := 'ViewpointGroup';
end;

class function TViewpointGroupNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TViewpointGroupNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
begin
  Result := inherited;
  if Result <> nil then Exit;

  Result := FdChildren.SearchValid(Func);
  if Result <> nil then Exit;
end;

function TViewpointGroupNode.Description: string;
begin
  Result := FdDescription.Value;
  if Result = '' then
    Result := NodeName;
  if Result = '' then
    Result := NodeTypeName;
end;

procedure RegisterNavigationNodes;
begin
  NodesManager.RegisterNodeClasses([
    TBillboardNode,
    TCollisionNode,
    TLODNode_2,
    TLODNode,
    TNavigationInfoNode,
    TOrthoViewpointNode,
    TViewpointNode,
    TViewpointGroupNode
  ]);
end;
{$endif read_implementation}
