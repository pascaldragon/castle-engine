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
  TGeneratedShadowMapNode = class;

  TLightScope = (
    { Light shines everywhere. VRML/X3D >= 2 calls these lights global. }
    lsGlobal,
    { Light shines only within it's VRML/X3D grouping node.
      VRML/X3D >= 2 calls these lights not global. }
    lsLocal,
    { Light shines only on the following shapes within
      it's VRML/X3D grouping node.
      This is used by VRML 1.0 (and Inventor) light sources. }
    lsLocalVRML1);

  { Base class for all VRML / X3D light nodes.

    Note that even the old VRML 1.0 light nodes inherit from this.
    Although they interpret some bits differently
    ("ambientIntensity" < 0 has special meaning).
    But most of the fields behave identically, they only have different
    default values. }
  TAbstractLightNode = class(TAbstractChildNode)
  private
    FTransform: TMatrix4Single;
    SavedDefaultShadowMap: boolean;
    SavedDefaultShadowMapUpdate: TTextureUpdate;
    SavedDefaultShadowMapSize: Integer;
    SavedDefaultShadowMapScale: Single;
    SavedDefaultShadowMapBias: Single;
    SavedDefaultShadowMapCompareMode: string;
  protected
    procedure BeforeTraverse(StateStack: TX3DGraphTraverseStateStack); override;
    procedure MiddleTraverse(StateStack: TX3DGraphTraverseStateStack); override;
  public
    procedure CreateNode; override;

    private FFdOn: TSFBool;
    public property FdOn: TSFBool read FFdOn;

    private FFdIntensity: TSFFloat;
    public property FdIntensity: TSFFloat read FFdIntensity;

    private FFdColor: TSFColor;
    public property FdColor: TSFColor read FFdColor;

    private FFdAmbientIntensity: TSFFloat;
    public property FdAmbientIntensity: TSFFloat read FFdAmbientIntensity;

    private FFdGlobal: TSFBool;
    public property FdGlobal: TSFBool read FFdGlobal;

    private FFdShadowVolumes: TSFBool;
    public property FdShadowVolumes: TSFBool read FFdShadowVolumes;

    private FFdShadowVolumesMain: TSFBool;
    public property FdShadowVolumesMain: TSFBool read FFdShadowVolumesMain;

    private FFdShowProxyGeometry: TSFBool;
    { showProxyGeometry field is an Avalon extension, see
      [http://instant-reality.com/documentation/nodetype/Light/]. }
    public property FdShowProxyGeometry: TSFBool read FFdShowProxyGeometry;

    private FFdProjectionNear: TSFFloat;
    { projectionNear / projectionFar / up are Kambi extensions, see
      [http://castle-engine.sourceforge.net/x3d_extensions.php#section_ext_light_projective] }
    public property FdProjectionNear: TSFFloat read FFdProjectionNear;

    private FFdProjectionFar: TSFFloat;
    public property FdProjectionFar: TSFFloat read FFdProjectionFar;

    private FFdUp: TSFVec3f;
    public property FdUp: TSFVec3f read FFdUp;

    private FFdDefaultShadowMap: TSFNode;
    public property FdDefaultShadowMap: TSFNode read FFdDefaultShadowMap;

    private FFdShadows: TSFBool;
    public property FdShadows: TSFBool read FFdShadows;

    private FFdEffects: TMFNode;
    public property FdEffects: TMFNode read FFdEffects;

    { Transformation of this light node.
      Normal lights can be instanced many times within the scene, with
      various transformation, so @italic(this transformation property
      cannot be used).

      However, in special cases, you know that light node occurs only once
      within the scene (see
      [http://castle-engine.sourceforge.net/x3d_extensions.php#section_ext_shadow_maps]).
      Then it's useful.

      It is gathered during traversing. Last BeforeTraverse call for this
      node sets Transform properties. By default, it represents identity
      transformation.

      @groupBegin }
    property Transform: TMatrix4Single read FTransform;
    { @groupEnd }

    { Matrices for rendering shadow map from this light.
      Identity in this class, override for subclasses able to do shadow mapping.
      @groupBegin }
    function ProjectionMatrix: TMatrix4Single; virtual;
    function ModelviewMatrix: TMatrix4Single; virtual;
    function ModelviewRotationMatrix: TMatrix4Single; virtual;
    function GetProjectorMatrix: TMatrix4Single;
    { @groupEnd }

    { Light location, direction and up vectors, for projection.
      Useful when you think of lights as cameras (for shadow maps).

      DirectionLocal must be exactly zero for a PointLight (that doesn't
      have a direction).

      @groupBegin }
    function ProjectionLocationLocal: TVector3Single; virtual;
    function ProjectionLocation: TVector3Single;
    function ProjectionDirectionLocal: TVector3Single; virtual;
    function ProjectionDirection: TVector3Single;
    procedure GetView(out Pos, Dir, Up: TVector3Single);
    { @groupEnd }

    { Calculate distances between the given Box and this light source.
      This is intended to capture the depth distances where the box
      resides, useful for calculating e.g. depth ranges to capture in
      the shadow maps.
      Depending on light source type, various distance measures may be used,
      appropriate to light sources projection.

      Always MinDistance <= MaxDistance. They may be negative when
      we measure along the light's direction.

      @raises EBox3DEmpty When used with an empty box. }
    procedure Box3DDistances(const Box: TBox3D;
      out MinDistance, MaxDistance: Single); virtual; abstract;

    { Create TLightInstance record describing this light node under given
      State. }
    function CreateLightInstance(State: TX3DGraphTraverseState): TLightInstance;

    { Update TLightInstance record when lighting State changes.
      Assumes that LightInstance.Node = Self.

      This will set LightInstance.Transform properties, and recalculate
      all LightInstance properties based on Transform. }
    procedure UpdateLightInstanceState(
      var LightInstance: TLightInstance;
      State: TX3DGraphTraverseState);

    { Update TLightInstance record when lighting location/direction (and other
      properties precalculated on TLightInstance) change.
      Assumes that LightInstance.Node = Self. }
    procedure UpdateLightInstance(
      var LightInstance: TLightInstance); virtual;

    function TransformationChange: TNodeTransformationChange; override;

    { Internal, to workaround small X3DShadowMaps problem. @exclude }
    procedure DefaultShadowMapSave(Node: TGeneratedShadowMapNode);
    { Internal, to workaround small X3DShadowMaps problem. @exclude }
    function DefaultShadowMapLoad(Node: TGeneratedShadowMapNode): boolean;

    { Light scope. Default implementation returns lsGlobal or lsLocal,
      depending on "global" field value (this follows VRML/X3D >= 2.0 rules). }
    function Scope: TLightScope; virtual;

    { Position expressed in homogeneous coordinates.
      For positional lights, the last component is always 1.
      For directional lights, the last component is always 0.

      Note that this expressed is in the local light node coordinate system. }
    function Position: TVector4Single; virtual; abstract;
  end;

  TAbstractDirectionalLightNode = class(TAbstractLightNode)
  private
    function GetDirection: TVector3Single;
    procedure SetDirection(const Value: TVector3Single);
    function GetProjectionRectangle: TVector4Single;
    procedure SetProjectionRectangle(const Value: TVector4Single);
    function GetProjectionLocation: TVector3Single;
    procedure SetProjectionLocation(const Value: TVector3Single);
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;

    private FFdDirection: TSFVec3f;
    public property FdDirection: TSFVec3f read FFdDirection;
    property Direction: TVector3Single read GetDirection write SetDirection;

    private FFdProjectionRectangle: TSFVec4f;
    public property FdProjectionRectangle: TSFVec4f read FFdProjectionRectangle;
    property ProjectionRectangle: TVector4Single read GetProjectionRectangle write SetProjectionRectangle;

    private FFdProjectionLocation: TSFVec3f;
    public property FdProjectionLocation: TSFVec3f read FFdProjectionLocation;
    property ProjectionLocation: TVector3Single read GetProjectionLocation write SetProjectionLocation;

    procedure UpdateLightInstance(var LightInstance: TLightInstance); override;

    function ProjectionMatrix: TMatrix4Single; override;
    function ModelviewMatrix: TMatrix4Single; override;
    function ModelviewRotationMatrix: TMatrix4Single; override;
    function ProjectionLocationLocal: TVector3Single; override;
    function ProjectionDirectionLocal: TVector3Single; override;
    procedure Box3DDistances(const Box: TBox3D;
      out MinDistance, MaxDistance: Single); override;
    function Position: TVector4Single; override;
  end;

  TAbstractPositionalLightNode = class(TAbstractLightNode)
  private
    function GetAttenuation: TVector3Single;
    procedure SetAttenuation(const Value: TVector3Single);
    function GetLocation: TVector3Single;
    procedure SetLocation(const Value: TVector3Single);
    function GetRadius: Single;
    procedure SetRadius(const Value: Single);
  public
    procedure CreateNode; override;

    private FFdAttenuation: TSFVec3f;
    public property FdAttenuation: TSFVec3f read FFdAttenuation;
    property Attenuation: TVector3Single read GetAttenuation write SetAttenuation;

    private FFdLocation: TSFVec3f;
    public property FdLocation: TSFVec3f read FFdLocation;
    property Location: TVector3Single read GetLocation write SetLocation;

    { Calculate light intensity drop because of the distance to the light.
      This follows the equation @code(1/max( attenuation[0] + ... ))
      from the VRML/X3D specification, it is the same as OpenGL attenuation.

      Since calculating the DistanceToLight for the @link(Attenuation)
      method may be time-consuming in some situations,
      you can check DistanceNeededForAttenuation first.
      When the DistanceNeededForAttenuation returns @false,
      then the value of DistanceToLight parameter is ignored (you can
      pass anything).

      The DistanceToLight should be a distance in the light source
      local coordinate system. TODO: although our renderers currently
      ignore this: ray-tracer uses global coord system,
      OpenGL (fixed-function and shader) renderer uses eye coord system
      (should be equal to global coord system for normal cameras).

      @groupBegin }
    function DistanceNeededForAttenuation: boolean;
    function CalculateAttenuation(const DistanceToLight: Single): Single; overload;
    function CalculateAttenuation(const DistanceToLight: Double): Double; overload;
    { @groupEnd }

    { Is attenuation relevant. When @false, you know that @link(Attenuation)
      function always returns 1, so there's no point in using it at all. }
    function HasAttenuation: boolean;

    private FFdRadius: TSFFloat;
    public property FdRadius: TSFFloat read FFdRadius;
    property Radius: Single read GetRadius write SetRadius;

    { Should the "radius" field be taken into account. }
    function HasRadius: boolean; virtual;

    procedure UpdateLightInstance(var LightInstance: TLightInstance); override;
    function Position: TVector4Single; override;
  end;

  TAbstractPointLightNode = class(TAbstractPositionalLightNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;

    function ProjectionLocationLocal: TVector3Single; override;
    procedure Box3DDistances(const Box: TBox3D;
      out MinDistance, MaxDistance: Single); override;
  end;

  TDirectionalLightNode = class(TAbstractDirectionalLightNode)
  public
    procedure CreateNode; override;
    class function URNMatching(const URN: string): boolean; override;

    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;
  end;
  TDirectionalLightNode_2 = TDirectionalLightNode;

  TPointLightNode = class(TAbstractPointLightNode)
  public
    procedure CreateNode; override;
    class function URNMatching(const URN: string): boolean; override;

    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;
  end;
  TPointLightNode_2 = TPointLightNode;

  TSpotLightNode = class(TAbstractPositionalLightNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdBeamWidth: TSFFloat;
    public property FdBeamWidth: TSFFloat read FFdBeamWidth;

    private FFdCutOffAngle: TSFFloat;
    public property FdCutOffAngle: TSFFloat read FFdCutOffAngle;

    private FFdDirection: TSFVec3f;
    public property FdDirection: TSFVec3f read FFdDirection;

    private FFdProjectionAngle: TSFFloat;
    public property FdProjectionAngle: TSFFloat read FFdProjectionAngle;

    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;

    procedure UpdateLightInstance(var LightInstance: TLightInstance); override;

    function ProjectionMatrix: TMatrix4Single; override;
    function ModelviewMatrix: TMatrix4Single; override;
    function ModelviewRotationMatrix: TMatrix4Single; override;
    function ProjectionLocationLocal: TVector3Single; override;
    function ProjectionDirectionLocal: TVector3Single; override;
    procedure Box3DDistances(const Box: TBox3D;
      out MinDistance, MaxDistance: Single); override;

    { Spot cutoff angle (based on cutOffAngle).

      Expressed in degrees, clamped to correct range
      (since user can input any value in VRML, and also conversion
      radians -> degrees could accidentally raise value slightly > 90,
      so cutOffAngle = 1.5708 is in degrees 90.0002104591,
      which would cause OpenGL fixed-function error). }
    function SpotCutoffDeg: Single;

    function SpotCosCutoff: Single;

    { Approximate spot exponent that could be used to render this spot light.
      Do not use this, unless you really don't have to.
      X3D spot light should have a linear fallback, from beamWidth to cutOffAngle,
      and there is no sensible way to approximate it by an exponent.
      Use this only if you have to render spot light with exponent,
      e.g. for OpenGL fixed-function pipeline. }
    function SpotExponentApproximate: Single;
  end;
  TSpotLightNode_2 = TSpotLightNode;

{$endif read_interface}

{$ifdef read_implementation}
const
  FallbackProjectionNear = 1;
  FallbackProjectionFar = 100;
  FallbackProjectionRectangle: TVector4Single = (-10, -10, 10, 10);

procedure TAbstractLightNode.CreateNode;
begin
  inherited;

  FFdGlobal := TSFBool.Create(Self, 'global', false);
   FdGlobal.ChangesAlways := [chEverything];
  Fields.Add(FFdGlobal);

  FFdOn := TSFBool.Create(Self, 'on', true);
   FdOn.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdOn);

  FFdIntensity := TSFFloat.Create(Self, 'intensity', 1);
   FdIntensity.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdIntensity);

  FFdColor := TSFColor.Create(Self, 'color', Vector3Single(1, 1, 1));
   FdColor.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdColor);

  FFdAmbientIntensity := TSFFloat.Create(Self, 'ambientIntensity', 0);
   FdAmbientIntensity.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdAmbientIntensity);

  FFdShadowVolumes := TSFBool.Create(Self, 'shadowVolumes', false);
   FdShadowVolumes.AddAlternativeName('kambiShadows', 0);
   FdShadowVolumes.ChangesAlways := [chLightForShadowVolumes];
  Fields.Add(FFdShadowVolumes);

  FFdShadowVolumesMain := TSFBool.Create(Self, 'shadowVolumesMain', false);
   FdShadowVolumesMain.AddAlternativeName('kambiShadowsMain', 0);
   FdShadowVolumesMain.ChangesAlways := [chLightForShadowVolumes];
  Fields.Add(FFdShadowVolumesMain);

  FFdShowProxyGeometry := TSFBool.Create(Self, 'showProxyGeometry', false);
  Fields.Add(FFdShowProxyGeometry);

  FFdProjectionNear := TSFFloat.Create(Self, 'projectionNear', 0);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdProjectionNear.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdProjectionNear);

  FFdProjectionFar := TSFFloat.Create(Self, 'projectionFar', 0);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdProjectionFar.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdProjectionFar);

  FFdUp := TSFVec3f.Create(Self, 'up', ZeroVector3Single);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdUp.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdUp);

  FFdDefaultShadowMap := TSFNode.Create(Self, 'defaultShadowMap', [TGeneratedShadowMapNode]);
   FdDefaultShadowMap.Exposed := false;
   FdDefaultShadowMap.ChangesAlways := [chEverything];
  Fields.Add(FFdDefaultShadowMap);

  FFdShadows := TSFBool.Create(Self, 'shadows', false);
   FdShadows.Exposed := false;
   FdShadows.ChangesAlways := [chShadowMaps];
  Fields.Add(FFdShadows);

  FFdEffects := TMFNode.Create(Self, 'effects', [TEffectNode]);
   FdEffects.Exposed := false;
   FdEffects.ChangesAlways := [chEverything];
  Fields.Add(FFdEffects);

  DefaultContainerField := 'children';

  FTransform := IdentityMatrix4Single;
end;

procedure TAbstractLightNode.BeforeTraverse(
  StateStack: TX3DGraphTraverseStateStack);
begin
  inherited;

  FTransform := StateStack.Top.Transform;
end;

function TAbstractLightNode.ProjectionMatrix: TMatrix4Single;
begin
  Result := IdentityMatrix4Single;
end;

function TAbstractLightNode.ModelviewMatrix: TMatrix4Single;
begin
  Result := IdentityMatrix4Single;
end;

function TAbstractLightNode.ModelviewRotationMatrix: TMatrix4Single;
begin
  Result := IdentityMatrix4Single;
end;

function TAbstractLightNode.GetProjectorMatrix: TMatrix4Single;
begin
  Result := ProjectionMatrix * ModelviewMatrix;
end;

function TAbstractLightNode.ProjectionLocationLocal: TVector3Single;
begin
  Result := ZeroVector3Single;
end;

function TAbstractLightNode.ProjectionLocation: TVector3Single;
begin
  Result := MatrixMultPoint(Transform, ProjectionLocationLocal);
end;

function TAbstractLightNode.ProjectionDirection: TVector3Single;
begin
  Result := MatrixMultDirection(Transform, ProjectionDirectionLocal);
end;

function TAbstractLightNode.ProjectionDirectionLocal: TVector3Single;
begin
  Result := ZeroVector3Single;
end;

procedure TAbstractLightNode.GetView(out Pos, Dir, Up: TVector3Single);
begin
  Pos := MatrixMultPoint    (Transform, ProjectionLocationLocal);
  Dir := MatrixMultDirection(Transform, ProjectionDirectionLocal);

  Up := FdUp.Value;
  { Up = zero means "calculate anything suitable".
    We could let AnyOrthogonalVector do this job, but when +Y is sensible
    (when it results in something non-parallel to Dir), let's use it.
    This makes calculated "up" more deterministic. }
  if PerfectlyZeroVector(Up) then
    Up := Vector3Single(0, 1, 0);
  Up := MatrixMultDirection(Transform, Up);
  { When the "up" vector is parallel to the dir then fix it.
    Note: when "up" is not parallel, then ModelviewMatrix will
    take care of adjusting it to be orthogonal. }
  if VectorsParallel(Up, Dir) then
    Up := AnyOrthogonalVector(Dir);
end;

function TAbstractLightNode.CreateLightInstance(
  State: TX3DGraphTraverseState): TLightInstance;
begin
  Result.Node := Self;
  Result.WorldCoordinates := false;
  UpdateLightInstanceState(Result, State);
end;

procedure TAbstractLightNode.UpdateLightInstanceState(
  var LightInstance: TLightInstance;
  State: TX3DGraphTraverseState);
begin
  LightInstance.Transform := State.Transform;
  LightInstance.TransformScale := State.TransformScale;
  UpdateLightInstance(LightInstance);
end;

procedure TAbstractLightNode.UpdateLightInstance(
  var LightInstance: TLightInstance);
begin
  { Nothing to do in this class. }
  Assert(LightInstance.Node = Self);
end;

procedure TAbstractLightNode.MiddleTraverse(StateStack: TX3DGraphTraverseStateStack);
begin
  inherited;
  if Scope = lsLocalVRML1 then
    StateStack.Top.AddLight(CreateLightInstance(StateStack.Top));
end;

function TAbstractLightNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcLight;
end;

procedure TAbstractLightNode.DefaultShadowMapSave(Node: TGeneratedShadowMapNode);
begin
  SavedDefaultShadowMap := true;
  SavedDefaultShadowMapUpdate      := Node.FdUpdate      .Value;
  SavedDefaultShadowMapSize        := Node.FdSize        .Value;
  SavedDefaultShadowMapScale       := Node.FdScale       .Value;
  SavedDefaultShadowMapBias        := Node.FdBias        .Value;
  SavedDefaultShadowMapCompareMode := Node.FdCompareMode .Value;
end;

function TAbstractLightNode.DefaultShadowMapLoad(Node: TGeneratedShadowMapNode): boolean;
begin
  Result := SavedDefaultShadowMap;
  if Result then
  begin
    Node.FdUpdate      .Value := SavedDefaultShadowMapUpdate     ;
    Node.FdSize        .Value := SavedDefaultShadowMapSize       ;
    Node.FdScale       .Value := SavedDefaultShadowMapScale      ;
    Node.FdBias        .Value := SavedDefaultShadowMapBias       ;
    Node.FdCompareMode .Value := SavedDefaultShadowMapCompareMode;
  end;
end;

function TAbstractLightNode.Scope: TLightScope;
begin
  if FdGlobal.Value then
    Result := lsGlobal else
    Result := lsLocal;
end;

procedure TAbstractDirectionalLightNode.CreateNode;
begin
  inherited;

  FFdDirection := TSFVec3f.Create(Self, 'direction', Vector3Single(0, 0, -1));
   FdDirection.ChangesAlways := [chLightInstanceProperty, chLightLocationDirection];
  Fields.Add(FFdDirection);

  FFdProjectionRectangle := TSFVec4f.Create(Self, 'projectionRectangle', ZeroVector4Single);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdProjectionRectangle.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdProjectionRectangle);

  FFdProjectionLocation := TSFVec3f.Create(Self, 'projectionLocation', ZeroVector3Single);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdProjectionLocation.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdProjectionLocation);
end;

class function TAbstractDirectionalLightNode.ClassNodeTypeName: string;
begin
  Result := 'DirectionalLight';
end;

procedure TAbstractDirectionalLightNode.UpdateLightInstance(
  var LightInstance: TLightInstance);
begin
  inherited;
  LightInstance.Direction := Normalized(MatrixMultDirection(
    LightInstance.Transform, FdDirection.Value));
end;

function TAbstractDirectionalLightNode.ProjectionMatrix: TMatrix4Single;
var
  N, F: Single;
  R: TVector4Single;
begin
  { If author didn't provide and X3DShadowMaps unit didn't calculate
    values for some fields, then use FallbackProjection* defaults here. }

  N := FdProjectionNear.Value;
  if N = 0 then N := FallbackProjectionNear;

  F := FdProjectionFar.Value;
  if F = 0 then F := FallbackProjectionFar;

  R := FdProjectionRectangle.Value;
  if PerfectlyZeroVector(R) then R := FallbackProjectionRectangle;

  { Beware: order of projectionRectangle
    is different than typical OpenGL and our OrthoProjMatrix params. }
  Result := OrthoProjMatrix(R[0], R[2], R[1], R[3], N, F);
end;

function TAbstractDirectionalLightNode.ModelviewMatrix: TMatrix4Single;
var
  Pos, Dir, Up: TVector3Single;
begin
  GetView(Pos, Dir, Up);
  Result := LookDirMatrix(Pos, Dir, Up);
end;

function TAbstractDirectionalLightNode.ModelviewRotationMatrix: TMatrix4Single;
var
  Pos, Dir, Up: TVector3Single;
begin
  GetView(Pos, Dir, Up);
  Result := LookDirMatrix(ZeroVector3Single, Dir, Up);
end;

function TAbstractDirectionalLightNode.ProjectionLocationLocal: TVector3Single;
begin
  Result := FdProjectionLocation.Value;
end;

function TAbstractDirectionalLightNode.ProjectionDirectionLocal: TVector3Single;
begin
  Result := FdDirection.Value;
end;

procedure TAbstractDirectionalLightNode.Box3DDistances(const Box: TBox3D;
  out MinDistance, MaxDistance: Single);
begin
  Box.DirectionDistances(ProjectionLocation, ProjectionDirection, MinDistance, MaxDistance);
end;

function TAbstractDirectionalLightNode.Position: TVector4Single;
begin
  Result := Vector4Single(-FdDirection.Value, 0);
end;

function TAbstractDirectionalLightNode.GetDirection: TVector3Single;
begin
  Result := FdDirection.Value;
end;

procedure TAbstractDirectionalLightNode.SetDirection(const Value: TVector3Single);
begin
  FdDirection.Send(Value);
end;

function TAbstractDirectionalLightNode.GetProjectionRectangle: TVector4Single;
begin
  Result := FdProjectionRectangle.Value;
end;

procedure TAbstractDirectionalLightNode.SetProjectionRectangle(const Value: TVector4Single);
begin
  FdProjectionRectangle.Send(Value);
end;

function TAbstractDirectionalLightNode.GetProjectionLocation: TVector3Single;
begin
  Result := FdProjectionLocation.Value;
end;

procedure TAbstractDirectionalLightNode.SetProjectionLocation(const Value: TVector3Single);
begin
  FdProjectionLocation.Send(Value);
end;

procedure TAbstractPositionalLightNode.CreateNode;
begin
  inherited;

  FFdLocation := TSFVec3f.Create(Self, 'location', Vector3Single(0, 0, 0));
   FdLocation.ChangesAlways := [chLightInstanceProperty, chLightLocationDirection];
  Fields.Add(FFdLocation);
  { X3D specification comment: (-Inf,Inf) }

  FFdAttenuation := TSFVec3f.Create(Self, 'attenuation', Vector3Single(1, 0, 0));
   FdAttenuation.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdAttenuation);
  { X3D specification comment: [0,Inf) }

  FFdRadius := TSFFloat.Create(Self, 'radius', 100);
   FdRadius.ChangesAlways := [chLightInstanceProperty];
  Fields.Add(FFdRadius);
  { X3D specification comment: [0,Inf) }
end;

function TAbstractPositionalLightNode.DistanceNeededForAttenuation: boolean;
begin
  Result := (FdAttenuation.Value[1] <> 0) or
            (FdAttenuation.Value[2] <> 0);
end;

function TAbstractPositionalLightNode.HasAttenuation: boolean;
begin
  Result := ((FdAttenuation.Value[0] <> 1) and
             (FdAttenuation.Value[0] <> 0)) or
            (FdAttenuation.Value[1] <> 0) or
            (FdAttenuation.Value[2] <> 0);
end;

function TAbstractPositionalLightNode.HasRadius: boolean;
begin
  Result := true;
end;

procedure TAbstractPositionalLightNode.UpdateLightInstance(
  var LightInstance: TLightInstance);
begin
  inherited;

  LightInstance.Location := MatrixMultPoint(LightInstance.Transform,
    FdLocation.Value);

  { TODO: For non-uniform scale, this will simply use average scale.
    This is not fully correct, VRML spec doesn't clarify this
    but I guess that the intention was that the non-uniform scale will
    make radius non-uniform, i.e. light volume will not be a regular sphere
    but some 3d ellipsoid. Unfortunately this would require quite more
    work, AddGlobalLights (in TCastleSceneCore) would then have to check for collision
    between
      sphere transformed by matrix Transform
    and
      bounding box
    which I don't know how to do *easily*... }
  LightInstance.Radius := FdRadius.Value * LightInstance.TransformScale;
end;

{$define ATTENUATION_IMPLEMENTATION:=
begin
 (* moglibysmy tu nie badac czy DistanceNeededForAttenuation i zawsze
    robic wersje pelna (bo przeciez
      FdAttenuation.Value[1] * DistanceToLight +
      FdAttenuation.Value[2] * Sqr(DistanceToLight)
    i tak bedzie = 0 gdy FdAttenuation.Value[1] = FdAttenuation.Value[2] = 0.
    Ale wydaje mi sie ze tak jest szybciej - testowanie kosztuje nas
    troszke czasu ale mozemy sobie w ten sposob ocalic 2 x mnozenie i dodawanie. *)

 (* we check whether attenuation = (0, 0, 0). VRML 97 spec says that specifying
    (0, 0, 0) should be equal to specifying (1, 0, 0). (well, we avoid
    division by zero possibility this way so it's quite sensible, even
    if it wastes some time) *)
 if (FdAttenuation.Value[0] = 0) and
    (FdAttenuation.Value[1] = 0) and
    (FdAttenuation.Value[2] = 0) then result := 1;

 if DistanceNeededForAttenuation then
  result := 1/ CastleUtils.max(FdAttenuation.Value[0] +
                   FdAttenuation.Value[1] * DistanceToLight +
                   FdAttenuation.Value[2] * Sqr(DistanceToLight), Single(1.0)) else
  result := 1/ CastleUtils.max(FdAttenuation.Value[0], Single(1.0));
end;}

function TAbstractPositionalLightNode.CalculateAttenuation(const DistanceToLight: Single): Single;
ATTENUATION_IMPLEMENTATION

function TAbstractPositionalLightNode.CalculateAttenuation(const DistanceToLight: Double): Double;
ATTENUATION_IMPLEMENTATION

function TAbstractPositionalLightNode.Position: TVector4Single;
begin
  Result := Vector4Single(FdLocation.Value, 1);
end;

function TAbstractPositionalLightNode.GetAttenuation: TVector3Single;
begin
  Result := FdAttenuation.Value;
end;

procedure TAbstractPositionalLightNode.SetAttenuation(const Value: TVector3Single);
begin
  FdAttenuation.Send(Value);
end;

function TAbstractPositionalLightNode.GetLocation: TVector3Single;
begin
  Result := FdLocation.Value;
end;

procedure TAbstractPositionalLightNode.SetLocation(const Value: TVector3Single);
begin
  FdLocation.Send(Value);
end;

function TAbstractPositionalLightNode.GetRadius: Single;
begin
  Result := FdRadius.Value;
end;

procedure TAbstractPositionalLightNode.SetRadius(const Value: Single);
begin
  FdRadius.Send(Value);
end;

procedure TAbstractPointLightNode.CreateNode;
begin
  inherited;
  { no new fields - this is just TAbstractPositionalLightNode }
end;

class function TAbstractPointLightNode.ClassNodeTypeName: string;
begin
  Result := 'PointLight';
end;

function TAbstractPointLightNode.ProjectionLocationLocal: TVector3Single;
begin
  Result := FdLocation.Value;
end;

procedure TAbstractPointLightNode.Box3DDistances(const Box: TBox3D;
  out MinDistance, MaxDistance: Single);
begin
  Box.PointDistances(ProjectionLocation, MinDistance, MaxDistance);
end;

procedure TDirectionalLightNode.CreateNode;
begin
  inherited;

  FdGlobal.Value := false;

  DefaultContainerField := 'children';
end;

class function TDirectionalLightNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TDirectionalLightNode.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TPointLightNode.CreateNode;
begin
  inherited;

  FdGlobal.Value := true;

  DefaultContainerField := 'children';
end;

class function TPointLightNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TPointLightNode.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TSpotLightNode.CreateNode;
begin
  inherited;

  FFdBeamWidth := TSFFloat.Create(Self, 'beamWidth', Pi / 2);
   FdBeamWidth.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdBeamWidth);
  { X3D specification comment: (0,Pi/2] }

  FFdCutOffAngle := TSFFloat.Create(Self, 'cutOffAngle', Pi / 4);
   FdCutOffAngle.ChangesAlways := [chVisibleNonGeometry];
  Fields.Add(FFdCutOffAngle);
  { X3D specification comment: (0,Pi/2] }

  FFdDirection := TSFVec3f.Create(Self, 'direction', Vector3Single(0, 0, -1));
   FdDirection.ChangesAlways := [chLightInstanceProperty, chLightLocationDirection];
  Fields.Add(FFdDirection);
  { X3D specification comment: (-Inf,Inf) }

  FdGlobal.Value := true;

  FFdProjectionAngle := TSFFloat.Create(Self, 'projectionAngle', 0);
   { We want to set UpdateNeeded := true for all GeneratedShadowMap using
     this light. For now, simply send chVisibleGeometry that will
     force updating all GeneratedShadowMaps. }
   FdProjectionAngle.ChangesAlways := [chVisibleGeometry];
  Fields.Add(FFdProjectionAngle);

  DefaultContainerField := 'children';
end;

class function TSpotLightNode.ClassNodeTypeName: string;
begin
  Result := 'SpotLight';
end;

class function TSpotLightNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TSpotLightNode.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TSpotLightNode.UpdateLightInstance(
  var LightInstance: TLightInstance);
begin
  inherited;
  LightInstance.Direction := Normalized(MatrixMultDirection(
    LightInstance.Transform, FdDirection.Value));
end;

function TSpotLightNode.ProjectionMatrix: TMatrix4Single;
var
  Angle, N, F: Single;
begin
  { If author didn't provide and X3DShadowMaps unit didn't calculate
    values for some fields, then use FallbackProjection* defaults here. }

  if FdprojectionAngle.Value <= 0 then
    Angle := 2 * FdCutOffAngle.Value else
    Angle := FdProjectionAngle.Value;

  N := FdProjectionNear.Value;
  if N = 0 then N := FallbackProjectionNear;

  F := FdProjectionFar.Value;
  if F = 0 then F := FallbackProjectionFar;

  Result := PerspectiveProjMatrixRad(Angle, 1, N, F);
end;

function TSpotLightNode.ModelviewMatrix: TMatrix4Single;
var
  Pos, Dir, Up: TVector3Single;
begin
  GetView(Pos, Dir, Up);
  Result := LookDirMatrix(Pos, Dir, Up);
end;

function TSpotLightNode.ModelviewRotationMatrix: TMatrix4Single;
var
  Pos, Dir, Up: TVector3Single;
begin
  GetView(Pos, Dir, Up);
  Result := LookDirMatrix(ZeroVector3Single, Dir, Up);
end;

function TSpotLightNode.ProjectionLocationLocal: TVector3Single;
begin
  Result := FdLocation.Value;
end;

function TSpotLightNode.ProjectionDirectionLocal: TVector3Single;
begin
  Result := FdDirection.Value;
end;

procedure TSpotLightNode.Box3DDistances(const Box: TBox3D;
  out MinDistance, MaxDistance: Single);
begin
  { TODO: MaxDistance should be a little larger, as spot light rays
    are not parallel. }
  Box.DirectionDistances(ProjectionLocation, ProjectionDirection, MinDistance, MaxDistance);
end;

function TSpotLightNode.SpotCutoffDeg: Single;
begin
  Result := Min(90, RadToDeg(FdCutOffAngle.Value));
end;

function TSpotLightNode.SpotCosCutoff: Single;
begin
  Result := Cos(FdCutOffAngle.Value);
end;

function TSpotLightNode.SpotExponentApproximate: Single;
begin
  { There is no way to exactly translate beamWidth to OpenGL GL_SPOT_EXPONENT.
    GL_SPOT_EXPONENT is an exponent for cosinus.
    beamWidth says to use constant intensity within beamWidth angle,
    and linear drop off to cutOffAngle.
    See [http://castle-engine.sourceforge.net/vrml_engine_doc/output/xsl/html/chapter.opengl_rendering.html#section.vrml_lights]
    for more discussion. }

  if FdBeamWidth.Value >= FdCutOffAngle.Value then
    Result := 0 else
    Result := Clamped(0.5 / Max(FdBeamWidth.Value, 0.0001), 0.0, 128.0);
end;

procedure RegisterLightingNodes;
begin
  NodesManager.RegisterNodeClasses([
    TDirectionalLightNode,
    TPointLightNode,
    TSpotLightNode
  ]);
end;

{$endif read_implementation}
