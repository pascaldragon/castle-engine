{
  Copyright 2008-2014 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ Nodes that are defined by Avalon extensions, see
  [http://instant-reality.com/] and in particular the specifications on
  [http://instant-reality.com/documentation/nodetype/]. }

{$ifdef read_interface}
  { MatrixTransform node for VRML/X3D >= 2.0. From Avalon extension,
    see [http://instant-reality.com/documentation/nodetype/MatrixTransform/]. }
  TMatrixTransformNode = class(TAbstractX3DGroupingNode, ITransformNode)
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
    procedure ApplyTransform(State: TX3DGraphTraverseState); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
    class function ForVRMLVersion(const Version: TX3DVersion): boolean;
      override;
    function TransformationChange: TNodeTransformationChange; override;

    private FFdMatrix: TSFMatrix4f;
    public property FdMatrix: TSFMatrix4f read FFdMatrix;
  end;
  TMatrixTransformNode_2 = TMatrixTransformNode;

  TTeapotNode = class(TAbstractX3DGeometryNode)
  protected
    function DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer; override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdSize: TSFVec3f;
    public property FdSize: TSFVec3f read FFdSize;

    private FFdSolid: TSFBool;
    public property FdSolid: TSFBool read FFdSolid;

    private FFdManifold: TSFBool;
    public property FdManifold: TSFBool read FFdManifold;

    private FFdTexCoord: TSFNode;
    public property FdTexCoord: TSFNode read FFdTexCoord;

    function Proxy(var State: TX3DGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
    function ProxyUsesOverTriangulate: boolean; override;

    function TexCoordField: TSFNode; override;
    function AutoGenerate3DTexCoords: boolean; override;
  end;

  TLoggerNode = class(TAbstractChildNode)
  private
    WriteStream: TStream;
    OwnsWriteStream: boolean;
    WriteStreamInitialized: boolean;
    procedure EventWriteReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
  public
    procedure CreateNode; override;
    destructor Destroy; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdLevel: TSFInt32;
    public property FdLevel: TSFInt32 read FFdLevel;

    private FFdLogFile: TSFString;
    public property FdLogFile: TSFString read FFdLogFile;

    private FFdEnabled: TSFBool;
    public property FdEnabled: TSFBool read FFdEnabled;

    private FEventWrite: TXFAnyEvent;
    public property EventWrite: TXFAnyEvent read FEventWrite;
  end;

  TConverterNode = class(TAbstractChildNode)
  private
    procedure EventInReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdEnabled: TSFBool;
    public property FdEnabled: TSFBool read FFdEnabled;

    { Event in } { }
    private FEventIn: TXFAnyEvent;
    public property EventIn: TXFAnyEvent read FEventIn;

    { Event out } { }
    private FEventOut: TXFAnyEvent;
    public property EventOut: TXFAnyEvent read FEventOut;
  end;

  TRenderedTextureNode = class(TAbstractTextureNode)
  private
    FGeneratedTextureHandler: TGeneratedTextureHandler;
  public
    procedure CreateNode; override;
    destructor Destroy; override;

    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    property GeneratedTextureHandler: TGeneratedTextureHandler
      read FGeneratedTextureHandler;

    private FFdViewpoint: TSFNode;
    public property FdViewpoint: TSFNode read FFdViewpoint;

    private FFdBackground: TSFNode;
    public property FdBackground: TSFNode read FFdBackground;

    private FFdFog: TSFNode;
    public property FdFog: TSFNode read FFdFog;

    private FFdEnvironment: TSFNode;
    public property FdEnvironment: TSFNode read FFdEnvironment;

    private FFdScene: TSFNode;
    public property FdScene: TSFNode read FFdScene;

    private FFdDimensions: TMFInt32;
    public property FdDimensions: TMFInt32 read FFdDimensions;

    private FFdDepthMap: TMFBool;
    public property FdDepthMap: TMFBool read FFdDepthMap;

    private FFdForeground: TSFNode;
    public property FdForeground: TSFNode read FFdForeground;

    private FFdZOffset: TMFInt32;
    public property FdZOffset: TMFInt32 read FFdZOffset;

    private FFdTargets: TMFNode;
    public property FdTargets: TMFNode read FFdTargets;

    private FFdDescription: TSFString;
    public property FdDescription: TSFString read FFdDescription;

    { Event out } { }
    private FEventViewing: TSFMatrix4fEvent;
    public property EventViewing: TSFMatrix4fEvent read FEventViewing;

    { Event out } { }
    private FEventProjection: TSFMatrix4fEvent;
    public property EventProjection: TSFMatrix4fEvent read FEventProjection;

    private FFdUpdate: TSFTextureUpdate;
    public property FdUpdate: TSFTextureUpdate read FFdUpdate;

    private FFdFrameBufferMode: TSFString;
    public property FdFrameBufferMode: TSFString read FFdFrameBufferMode;

    private FFdExcludeNodes: TMFNode;
    public property FdExcludeNodes: TMFNode read FFdExcludeNodes;

    private FFdTriggerName: TSFString;
    public property FdTriggerName: TSFString read FFdTriggerName;

    { Event in } { }
    private FEventTriggerSlot: TSFTimeEvent;
    public property EventTriggerSlot: TSFTimeEvent read FEventTriggerSlot;

    { Event out } { }
    private FEventImage_changed: TSFImageEvent;
    public property EventImage_changed: TSFImageEvent read FEventImage_changed;

    private FFdTextureProperties: TSFNode;
    public property FdTextureProperties: TSFNode read FFdTextureProperties;

    private FFdRepeatS: TSFBool;
    public property FdRepeatS: TSFBool read FFdRepeatS;

    private FFdRepeatT: TSFBool;
    public property FdRepeatT: TSFBool read FFdRepeatT;

    private FFdRepeatR: TSFBool;
    public property FdRepeatR: TSFBool read FFdRepeatR;

    { Event out } { }
    private FEventRendering: TSFBoolEvent;
    public property EventRendering: TSFBoolEvent read FEventRendering;
  end;

  TPlaneNode = class(TAbstractX3DGeometryNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdSize: TSFVec2f;
    public property FdSize: TSFVec2f read FFdSize;

    private FFdSolid: TSFBool;
    public property FdSolid: TSFBool read FFdSolid;

    function Proxy(var State: TX3DGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
    function ProxyUsesOverTriangulate: boolean; override;
  end;

  TTogglerNode = class(TAbstractChildNode)
  private
    procedure EventStatusReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    procedure EventNotStatusReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    procedure EventToggleReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    procedure EventSetReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    procedure EventResetReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdStatus: TSFBool;
    public property FdStatus: TSFBool read FFdStatus;

    private FFdNotStatus: TSFBool;
    public property FdNotStatus: TSFBool read FFdNotStatus;

    { Event in } { }
    private FEventToggle: TXFAnyEvent;
    public property EventToggle: TXFAnyEvent read FEventToggle;

    { Event in } { }
    private FEventSet: TXFAnyEvent;
    public property EventSet: TXFAnyEvent read FEventSet;

    { Event in } { }
    private FEventReset: TXFAnyEvent;
    public property EventReset: TXFAnyEvent read FEventReset;

    { Event out } { }
    private FEventChanged: TSFBoolEvent;
    public property EventChanged: TSFBoolEvent read FEventChanged;

    { Event out } { }
    private FEventOn: TSFBoolEvent;
    public property EventOn: TSFBoolEvent read FEventOn;

    { Event out } { }
    private FEventOff: TSFBoolEvent;
    public property EventOff: TSFBoolEvent read FEventOff;

    private FFdEnabled: TSFBool;
    public property FdEnabled: TSFBool read FFdEnabled;
  end;
{$endif read_interface}

{$ifdef read_implementation}
procedure TMatrixTransformNode.CreateNode;
begin
  inherited;

  FFdMatrix := TSFMatrix4f.Create(Self, 'matrix', IdentityMatrix4Single);
   FdMatrix.ChangesAlways := [chTransform];
  Fields.Add(FFdMatrix);

  DefaultContainerField := 'children';
end;

class function TMatrixTransformNode.ClassNodeTypeName: string;
begin
  Result := 'MatrixTransform';
end;

class function TMatrixTransformNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

class function TMatrixTransformNode.ForVRMLVersion(const Version: TX3DVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

function TMatrixTransformNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
var
  I: Integer;
begin
  Result := nil;
  for I := 0 to FdChildren.Count - 1 do
  begin
    Result := Func(Self, FdChildren[I]);
    if Result <> nil then Exit;
  end;
end;

procedure TMatrixTransformNode.ApplyTransform(State: TX3DGraphTraverseState);
var
  InvertedMatrix: TMatrix4Single;
begin
  inherited;

  { Inherited TAbstractGroupingNode already saved State.Transform and such. }

  State.Transform := MatrixMult(State.Transform, FdMatrix.Value);

  if TryMatrixInverse(FdMatrix.Value, InvertedMatrix) then
  begin
    State.InvertedTransform :=
      MatrixMult(InvertedMatrix, State.InvertedTransform);
  end else
  begin
    if Log then
      WritelnLogMultiline('Matrix',
        'Cannot invert matrix:' + NL + MatrixToRawStr(FdMatrix.Value, '  '));

    { When determinant is zero, we treat inverted matrix
      like identity (like in TMatrixTransformNode_1.Transformation).
      So no need to multiply State.InvertedTransform by anything. }
  end;

  State.TransformScale *= FdMatrix.TransformScale;
end;

function TMatrixTransformNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcTransform;
end;

procedure TTeapotNode.CreateNode;
begin
  inherited;

  FFdSize := TSFVec3f.Create(Self, 'size', Vector3Single(3, 3, 3));
   FdSize.Exposed := false;
   FdSize.ChangesAlways := [chGeometry];
  Fields.Add(FFdSize);

  FFdSolid := TSFBool.Create(Self, 'solid', true);
   FdSolid.Exposed := false;
   FdSolid.ChangesAlways := [chGeometry];
  Fields.Add(FFdSolid);

  FFdManifold := TSFBool.Create(Self, 'manifold', false);
   FdManifold.Exposed := false;
   FdManifold.ChangesAlways := [chGeometry];
  Fields.Add(FFdManifold);

  FFdTexCoord := TSFNode.Create(Self, 'texCoord', [TTextureCoordinateGeneratorNode, TProjectedTextureCoordinateNode, TMultiTextureCoordinateNode]);
   FdTexCoord.ChangesAlways := [chGeometry];
  Fields.Add(FFdTexCoord);
end;

class function TTeapotNode.ClassNodeTypeName: string;
begin
  Result := 'Teapot';
end;

class function TTeapotNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

function TTeapotNode.Proxy(var State: TX3DGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
const
{ TeapotCoord: array [0..0] of TVector3Single = ((0, 0, 0));
  TeapotCoordIndex: array [0..0] of LongInt = (0);
  TeapotManifoldCoord: array [0..0] of TVector3Single = ((0, 0, 0));
  TeapotManifoldCoordIndex: array [0..0] of LongInt = (0);
}
  {$I teapot/teapot.inc}
var
  CoordNode: TCoordinateNode;
  IFS: TIndexedFaceSetNode absolute Result;
begin
  IFS := TIndexedFaceSetNode.Create(NodeName, BaseUrl);
  try
    CoordNode := TCoordinateNode.Create('', BaseUrl);
    IFS.FdCoord.Value := CoordNode;
    if FdManifold.Value then
      CoordNode.FdPoint.Items.AssignArray(TeapotManifoldCoord) else
      CoordNode.FdPoint.Items.AssignArray(TeapotCoord);

    { Scale Coords according to "size" field.
      Assumes that original coords are good for default size (3, 3, 3). }
    if not FdSize.EqualsDefaultValue then
    begin
      CoordNode.FdPoint.Items.MultiplyComponents(
        VectorScale(FdSize.Value, 1/3));
    end;

    if FdManifold.Value then
      IFS.FdCoordIndex.Items.AddArray(TeapotManifoldCoordIndex) else
      IFS.FdCoordIndex.Items.AddArray(TeapotCoordIndex);

    IFS.FdSolid.Value := FdSolid.Value;

    if FdTexCoord.CurrentChildAllowed then
      IFS.FdTexCoord.Value := FdTexCoord.Value;

    { Set CreaseAngle to anything larger than Pi, to make this completely
      smooth later using fast CreateSmoothNormalsCoordinateNode routine. }
    IFS.FdCreaseAngle.Value := 4;
  except FreeAndNil(Result); raise end;
end;

function TTeapotNode.ProxyUsesOverTriangulate: boolean;
begin
  Result := false;
end;

function TTeapotNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction): Pointer;
begin
  Result := inherited;
  if Result <> nil then Exit;

  Result := FdtexCoord.SearchValid(Func);
  if Result <> nil then Exit;
end;

function TTeapotNode.TexCoordField: TSFNode;
begin
  Result := FdTexCoord;
end;

function TTeapotNode.AutoGenerate3DTexCoords: boolean;
begin
  Result := (FdTexCoord.Value = nil) or not FdTexCoord.CurrentChildAllowed;
end;

procedure TLoggerNode.CreateNode;
begin
  inherited;

  FFdLevel := TSFInt32.Create(Self, 'level', 1);
  Fields.Add(FFdLevel);

  FFdLogFile := TSFString.Create(Self, 'logFile', '');
  FFdLogFile.Exposed := false;
  Fields.Add(FFdLogFile);

  FFdEnabled := TSFBool.Create(Self, 'enabled', true);
  Fields.Add(FFdEnabled);

  FEventWrite := TXFAnyEvent.Create(Self, 'write', true);
  Events.Add(FEventWrite);
  FEventWrite.OnReceive.Add(@EventWriteReceive);
end;

destructor TLoggerNode.Destroy;
begin
  if WriteStreamInitialized then
  begin
    if OwnsWriteStream then
      FreeAndNil(WriteStream);
  end;

  inherited;
end;

procedure TLoggerNode.EventWriteReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);

  procedure StdOutNotAvail;
  begin
    OnWarning(wtMajor, 'VRML/X3D',
      'Logger node wants to write on standard output, but it seems that stdout (standard output) ' +
      'is not available. Under Windows you should explicitly ' +
      'redirect program''s stdout to make it available, e.g. ' +
      'run "' + ApplicationName + ' > ' + ApplicationName + '.log".');
  end;

  procedure WritelnLogMessage;
  var
    S: string;
    Writer: TX3DWriterNames;
  begin
    S := Format('Logger "%s": received field "%s" (%s). Time: %f.',
      [NodeName, Value.Name, Value.TypeName, Time.Seconds]);

    if FdLevel.Value = 1 then
    begin
      WritelnStr(WriteStream, S);
    end else
    begin
      Assert(FdLevel.Value >= 2);

      if (FdLevel.Value >= 3) and (Value.ParentNode <> nil) then
      begin
        S += Format(' Sending node: "%s" (%s).',
          [(Value.ParentNode as TX3DNode).NodeName,
           (Value.ParentNode as TX3DNode).NodeTypeName]);
      end;

      WriteStr(WriteStream, S + ' Value: ');

      Writer := TX3DWriterNames.Create(WriteStream, X3DVersion, xeClassic);
      try
        Value.FieldSaveToStream(Writer, true);
      finally
        FreeAndNil(Writer);
      end;
    end;
  end;

begin
  if FdEnabled.Value and (Fdlevel.Value > 0) then
  begin
    { First reception of "write" event causes initialization of
      WriteStream, based on logFile field value. This is Ok, since
      logFile is not exposed, so it cannot change after the file is loaded. }
    if not WriteStreamInitialized then
    begin
      Assert(WriteStream = nil);

      WriteStreamInitialized := true;

      if FdLogFile.Value <> '' then
      begin
        WriteStream := TFileStream.Create(
          FileNameAutoInc(ApplicationName + '_logger_' +
            DeleteURIExt(ExtractURIName(FdLogFile.Value)) + '_%d.log'), fmCreate);
        OwnsWriteStream := true;
      end else
      begin
        WriteStream := StdOutStream;
        OwnsWriteStream := false;
        if WriteStream = nil then
        begin
          { report stdout not available, leave WriteStream = nil and
            WriteStreamInitialized = true. This way we will not try to
            reinit this again. }
          StdOutNotAvail;
          Exit;
        end;
      end;
    end;

    if WriteStream <> nil then
    begin
      try
        WritelnLogMessage;
      except
        on E: EWriteError do
        begin
          if not OwnsWriteStream then
          begin
            { This means that we have stdout, and it just failed...
              This is possible on Windows.

              Ideally, check for "StdOutStream = nil" should be all that is needed,
              and wrapping WritelnStr inside try...except should not be needed.
              But... see StdOutStream comments: you cannot
              depend on the fact that "StdOutStream <> nil means that stdout
              is actually available (because user redirected stdout etc.). }
            StdOutNotAvail;
          end else
            raise;
        end;
      end;
    end;
  end;
end;

class function TLoggerNode.ClassNodeTypeName: string;
begin
  Result := 'Logger';
end;

class function TLoggerNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

procedure TConverterNode.CreateNode;
begin
  inherited;

  FFdEnabled := TSFBool.Create(Self, 'enabled', true);
  Fields.Add(FFdEnabled);

  FEventIn := TXFAnyEvent.Create(Self, 'in', true);
  Events.Add(FEventIn);
  FEventIn.OnReceive.Add(@EventInReceive);

  FEventOut := TXFAnyEvent.Create(Self, 'out', true);
  Events.Add(FEventOut);
end;

procedure TConverterNode.EventInReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if FdEnabled.Value then
    { TODO: uhhhmmmm, I cannot convert here (as I don't know to what I should
      convert... Conversion must be done along the route, when destination
      is known. }
    EventOut.Send(Value, Time);
end;

class function TConverterNode.ClassNodeTypeName: string;
begin
  Result := 'Converter';
end;

class function TConverterNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

procedure TRenderedTextureNode.CreateNode;
begin
  inherited;

  FFdViewpoint := TSFNode.Create(Self, 'viewpoint', [TAbstractViewpointNode]);
   FdViewpoint.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdViewpoint);

  FFdBackground := TSFNode.Create(Self, 'background', [TAbstractBackgroundNode]);
   FdBackground.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdBackground);

  FFdFog := TSFNode.Create(Self, 'fog', [TFogNode]);
   FdFog.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdFog);

  FFdEnvironment := TSFNode.Create(Self, 'environment', [TX3DNode] { nothing sensible in core X3D for this });
   FdEnvironment.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdEnvironment);

  FFdScene := TSFNode.Create(Self, 'scene', [TX3DNode]);
   FdScene.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdScene);

  FFdDimensions := TMFInt32.Create(Self, 'dimensions',
    [ DefaultRenderedTextureWidth,
      DefaultRenderedTextureHeight, 4, 1, 1]);
   FdDimensions.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdDimensions);

  FFdDepthMap := TMFBool.Create(Self, 'depthMap', []);
   FdDepthMap.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdDepthMap);

  FFdForeground := TSFNode.Create(Self, 'foreground', [TX3DNode] { nothing sensible in core X3D for this });
   FdForeground.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdForeground);

  FFdZOffset := TMFInt32.Create(Self, 'zOffset', []);
   FdZOffset.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdZOffset);

  FFdTargets := TMFNode.Create(Self, 'targets', [TAbstractTextureNode]);
   FdTargets.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdTargets);

  FFdDescription := TSFString.Create(Self, 'description', '');
  Fields.Add(FFdDescription);

  FEventViewing := TSFMatrix4fEvent.Create(Self, 'viewing', false);
  Events.Add(FEventViewing);

  FEventProjection := TSFMatrix4fEvent.Create(Self, 'projection', false);
  Events.Add(FEventProjection);

  FFdUpdate := TSFTextureUpdate.Create(Self, 'update', upNone);
  Fields.Add(FFdUpdate);

  FFdFrameBufferMode := TSFString.Create(Self, 'frameBufferMode', 'auto');
   FdFrameBufferMode.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdFrameBufferMode);

  FFdExcludeNodes := TMFNode.Create(Self, 'excludeNodes', [TX3DNode]);
   FdExcludeNodes.Exposed := false;
   FdExcludeNodes.ChangesAlways := [chGeneratedTextureUpdateNeeded];
  Fields.Add(FFdExcludeNodes);

  FFdTriggerName := TSFString.Create(Self, 'triggerName', 'Render');
   FdTriggerName.Exposed := false;
  Fields.Add(FFdTriggerName);

  FEventTriggerSlot := TSFTimeEvent.Create(Self, 'triggerSlot', true);
  Events.Add(FEventTriggerSlot);

  FEventImage_changed := TSFImageEvent.Create(Self, 'image_changed', false);
  Events.Add(FEventImage_changed);

  FFdTextureProperties := TSFNode.Create(Self, 'textureProperties', [TTexturePropertiesNode]);
   FdTextureProperties.Exposed := false;
   FdTextureProperties.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdTextureProperties);

  FFdRepeatS := TSFBool.Create(Self, 'repeatS', true);
   FdRepeatS.Exposed := false;
   FdRepeatS.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdRepeatS);

  FFdRepeatT := TSFBool.Create(Self, 'repeatT', true);
   FdRepeatT.Exposed := false;
   FdRepeatT.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdRepeatT);

  FFdRepeatR := TSFBool.Create(Self, 'repeatR', true);
   FdRepeatR.Exposed := false;
   FdRepeatR.ChangesAlways := [chTextureRendererProperties];
  Fields.Add(FFdRepeatR);

  FEventRendering := TSFBoolEvent.Create(Self, 'rendering', false);
  Events.Add(FEventRendering);

  FGeneratedTextureHandler := TGeneratedTextureHandler.Create;
  FGeneratedTextureHandler.FUpdate := FdUpdate;
end;

destructor TRenderedTextureNode.Destroy;
begin
  FreeAndNil(FGeneratedTextureHandler);
  inherited;
end;

class function TRenderedTextureNode.ClassNodeTypeName: string;
begin
  Result := 'RenderedTexture';
end;

class function TRenderedTextureNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

procedure TPlaneNode.CreateNode;
begin
  inherited;

  FFdSize := TSFVec2f.Create(Self, 'size', Vector2Single(2, 2));
   FdSize.Exposed := false;
   FdSize.ChangesAlways := [chGeometry];
  Fields.Add(FFdSize);
  { X3D specification comment: (0,Inf) }

  FFdSolid := TSFBool.Create(Self, 'solid', true);
   FdSolid.Exposed := false;
   FdSolid.ChangesAlways := [chGeometry];
  Fields.Add(FFdSolid);
end;

class function TPlaneNode.ClassNodeTypeName: string;
begin
  Result := 'Plane';
end;

class function TPlaneNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

function TPlaneNode.Proxy(var State: TX3DGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := Rectangle2DProxy(Self, FdSize.Value, FdSolid.Value);
end;

function TPlaneNode.ProxyUsesOverTriangulate: boolean;
begin
  Result := false;
end;

type
  TSFBoolIgnorable = class(TSFBool)
  protected
    class function ExposedEventsFieldClass: TX3DFieldClass; override;
    procedure ExposedEventReceive(Event: TX3DEvent; AValue: TX3DField;
      const Time: TX3DTime); override;
  end;

class function TSFBoolIgnorable.ExposedEventsFieldClass: TX3DFieldClass;
begin
  Result := TSFBool;
end;

procedure TSFBoolIgnorable.ExposedEventReceive(Event: TX3DEvent; AValue: TX3DField;
  const Time: TX3DTime);
begin
  { This makes status and notStatus input events ignored,
    and output events not generated when enabled = FALSE. }
  if (ParentNode <> nil) and
     (not (ParentNode as TTogglerNode).FdEnabled.Value) then Exit;

  inherited;
end;

procedure TTogglerNode.CreateNode;
begin
  inherited;

  FFdStatus := TSFBoolIgnorable.Create(Self, 'status', false);
   FdStatus.OnReceive.Add(@EventStatusReceive);
  Fields.Add(FFdStatus);

  FFdNotStatus := TSFBoolIgnorable.Create(Self, 'notStatus', true);
   FdNotstatus.OnReceive.Add(@EventNotStatusReceive);
  Fields.Add(FFdNotStatus);

  FEventToggle := TXFAnyEvent.Create(Self, 'toggle', true);
  FEventToggle.OnReceive.Add(@EventToggleReceive);
  Events.Add(FEventToggle);

  FEventSet := TXFAnyEvent.Create(Self, 'set', true);
   EventSet.OnReceive.Add(@EventSetReceive);
  Events.Add(FEventSet);

  FEventReset := TXFAnyEvent.Create(Self, 'reset', true);
   EventReset.OnReceive.Add(@EventResetReceive);
  Events.Add(FEventReset);

  FEventChanged := TSFBoolEvent.Create(Self, 'changed', false);
  Events.Add(FEventChanged);

  FEventOn := TSFBoolEvent.Create(Self, 'on', false);
  Events.Add(FEventOn);

  FEventOff := TSFBoolEvent.Create(Self, 'off', false);
  Events.Add(FEventOff);

  FFdEnabled := TSFBool.Create(Self, 'enabled', true);
  Fields.Add(FFdEnabled);
end;

class function TTogglerNode.ClassNodeTypeName: string;
begin
  Result := 'Toggler';
end;

class function TTogglerNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNKambiNodes + ClassNodeTypeName) or
    (URN = URNKambiNodes2 + ClassNodeTypeName);
end;

procedure TTogglerNode.EventStatusReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  { Do not send event to notStatus, if it already has correct value
    (so our own value didn't actually changed).
    This avoids calling status and notStatus events in a loop.
    By the way, also avoid sending excessive changed/on/off. }
  if FdnotStatus.Value <> not FdStatus.Value then
  begin
    FdnotStatus.Send(not FdStatus.Value);
    EventChanged.Send(true, Time);
    if FdStatus.Value then EventOn.Send(true, Time);
    if not FdStatus.Value then EventOff.Send(true, Time);
  end;
end;

procedure TTogglerNode.EventNotStatusReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  { Do not send event to status, if it already has correct value
    (so our own value didn't actually changed).
    This avoids calling status and notStatus events in a loop.
    By the way, also avoid sending excessive changed/on/off. }
  if FdnotStatus.Value <> not FdStatus.Value then
  begin
    FdStatus.Send(not FdnotStatus.Value);
    EventChanged.Send(true, Time);
    if FdStatus.Value then EventOn.Send(true, Time);
    if not FdStatus.Value then EventOff.Send(true, Time);
  end;
end;

procedure TTogglerNode.EventToggleReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if FdEnabled.Value then
    FdStatus.Send(not FdStatus.Value);
end;

procedure TTogglerNode.EventSetReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if FdEnabled.Value then
    FdStatus.Send(true);
end;

procedure TTogglerNode.EventResetReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  if FdEnabled.Value then
    FdStatus.Send(false);
end;

procedure RegisterAvalonNodes;
begin
  NodesManager.RegisterNodeClasses([
    TMatrixTransformNode,
    TTeapotNode,
    TLoggerNode,
    TConverterNode,
    TRenderedTextureNode,
    TPlaneNode,
    TTogglerNode
  ]);
end;
{$endif read_implementation}
