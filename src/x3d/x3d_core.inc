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

{$ifdef read_interface}

type
  { }
  IAbstractNode = interface(IX3DNode)
  ['{399AC6B2-C2B3-4EEA-AB74-D52C4836B3B9}']
    property FdMetadata: TSFNode;
  end;

  { Base X3D node.

    Almost all X3D nodes inherit from this. (Exception:
    X3DMetadataObject. So you cannot have metadata of metadata of...) }
  TAbstractNode = class(TX3DNode, IAbstractNode)
  public
    procedure CreateNode; override;

    private FFdMetadata: TSFNode;
    public property FdMetadata: TSFNode read FFdMetadata;
  end;

  IAbstractChildNode = interface(IAbstractNode)
  ['{ECE6F71D-CA90-4B6C-8835-EAA68EEDAEED}']
  end;

  TAbstractChildNode = class(TAbstractNode, IAbstractChildNode)
  public
    procedure CreateNode; override;
  end;

  TAbstractBindableNode = class(TAbstractChildNode)
  private
    FTransform: TMatrix4Single;
    FTransformScale: Single;
  protected
    procedure BeforeTraverse(StateStack: TVRMLGraphTraverseStateStack); override;
  public
    procedure CreateNode; override;

    { Event: SFBool, in } { }
    private FEventSet_bind: TVRMLEvent;
    public property EventSet_bind: TVRMLEvent read FEventSet_bind;

    { Event: SFTime, out } { }
    private FEventBindTime: TVRMLEvent;
    public property EventBindTime: TVRMLEvent read FEventBindTime;

    { Event: SFBool, out } { }
    private FEventIsBound: TVRMLEvent;
    public property EventIsBound: TVRMLEvent read FEventIsBound;

    { Transformation of this bindable node.

      Bound nodes cannot be instantiated multiple number of times
      (this would make simple event like "set_bind" not possible,
      as it would not be known in what coordinate space the node is bound),
      so it's perfectly safe and comfortable to just keep their transformation
      here, a their property.

      It is gathered during traversing. Last BeforeTraverse call for this
      node sets Transform properties. By default, these represent identity
      transformation.

      Note that using TransformScale for bindable nodes like fog
      is a little simplification. Theoretically, the scale can be non-uniform,
      and around an arbitrary axis. So to apply e.g. fog, we should
      transform the 3D world back into local fog coordinate system,
      and calculate the distances there. Instead right now we do
      the opposite: transform stuff like fog VisibilityRange by
      TransformScale, and calculate distances in world coordinate
      system. This is a simplification, but in practice it's perfect
      (who uses non-uniform fog scale?) and it can be expressed for renderers
      (OpenGL) without any problems.

      @groupBegin }
    property Transform: TMatrix4Single read FTransform;
    property TransformScale: Single read FTransformScale;
    { @groupEnd }
  end;

  TAbstractInfoNode = class(TAbstractChildNode)
  end;

  IAbstractMetadataObject = interface(IX3DNode)
  ['{2D631FBF-683B-4259-A98D-74589893447C}']
    property FdName: TSFString { read GetFdname }; { }
    property FdReference: TSFString { read GetFdreference }; { }
  end;

  { X3DSensorNode when it's a secondary ancestor.
    (When it's a primary ancestor, I use class TAbstractSensorNode.) }
  IAbstractSensorNode = interface(IAbstractChildNode)
  ['{7434C62F-8084-40C3-AA57-08F9B574655A}']
    property FdEnabled: TSFBool;

    { Event: SFBool, out } { }
    property EventIsActive: TVRMLEvent;
  end;

  TAbstractSensorNode = class(TAbstractChildNode, IAbstractSensorNode)
  public
    procedure CreateNode; override;

    private FFdEnabled: TSFBool;
    public property FdEnabled: TSFBool read FFdEnabled;

    { Event: SFBool, out } { }
    private FEventIsActive: TVRMLEvent;
    public property EventIsActive: TVRMLEvent read FEventIsActive;
  end;

  TMetadataDoubleNode = class(TAbstractNode, IAbstractMetadataObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;

    private FFdReference: TSFString;
    public property FdReference: TSFString read FFdReference;

    private FFdValue: TMFDouble;
    public property FdValue: TMFDouble read FFdValue;
  end;

  TMetadataFloatNode = class(TAbstractNode, IAbstractMetadataObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;

    private FFdReference: TSFString;
    public property FdReference: TSFString read FFdReference;

    private FFdValue: TMFFloat;
    public property FdValue: TMFFloat read FFdValue;
  end;

  TMetadataIntegerNode = class(TAbstractNode, IAbstractMetadataObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;

    private FFdReference: TSFString;
    public property FdReference: TSFString read FFdReference;

    private FFdValue: TMFInt32;
    public property FdValue: TMFInt32 read FFdValue;
  end;

  TMetadataSetNode = class(TAbstractNode, IAbstractMetadataObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;

    private FFdReference: TSFString;
    public property FdReference: TSFString read FFdReference;

    private FFdValue: TMFNode;
    public property FdValue: TMFNode read FFdValue;
  end;

  TMetadataStringNode = class(TAbstractNode, IAbstractMetadataObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;

    private FFdReference: TSFString;
    public property FdReference: TSFString read FFdReference;

    private FFdValue: TMFString;
    public property FdValue: TMFString read FFdValue;
  end;

  TWorldInfoNode = class(TAbstractInfoNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdInfo: TMFString;
    public property FdInfo: TMFString read FFdInfo;

    private FFdTitle: TSFString;
    public property FdTitle: TSFString read FFdTitle;
  end;

{$endif read_interface}

{$ifdef read_implementation}

procedure TAbstractNode.CreateNode;
begin
  inherited;

  FFdMetadata := TSFNode.Create(Self, 'metadata', IAbstractMetadataObject);
  Fields.Add(FFdMetadata);
end;

procedure TAbstractChildNode.CreateNode;
begin
  inherited;

  { It's natural that X3DChildNode goes into "children" node of parent.
    Actually, this makes many other
      DefaultContainerField := 'children'
    lines in our x3d_*.inc include files useless. }
  DefaultContainerField := 'children';
end;

procedure TAbstractBindableNode.CreateNode;
begin
  inherited;

  FEventSet_bind := TVRMLEvent.Create(Self, 'set_bind', TSFBool, true);
  Events.Add(FEventSet_bind);

  FEventBindTime := TVRMLEvent.Create(Self, 'bindTime', TSFTime, false);
  Events.Add(FEventBindTime);

  FEventIsBound := TVRMLEvent.Create(Self, 'isBound', TSFBool, false);
  Events.Add(FEventIsBound);

  FTransform := IdentityMatrix4Single;
  FTransformScale := 1;
end;

procedure TAbstractBindableNode.BeforeTraverse(
  StateStack: TVRMLGraphTraverseStateStack);
begin
  inherited;

  FTransform := StateStack.Top.Transform;
  FTransformScale := StateStack.Top.TransformScale;
end;

procedure TAbstractSensorNode.CreateNode;
begin
  inherited;

  FFdEnabled := TSFBool.Create(Self, 'enabled', true);
  Fields.Add(FFdEnabled);

  FEventIsActive := TVRMLEvent.Create(Self, 'isActive', TSFBool, false);
  Events.Add(FEventIsActive);
end;

procedure TMetadataDoubleNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  Fields.Add(FFdName);

  FFdReference := TSFString.Create(Self, 'reference', '');
  Fields.Add(FFdReference);

  FFdValue := TMFDouble.Create(Self, 'value', []);
  Fields.Add(FFdValue);

  DefaultContainerField := 'metadata';
end;

class function TMetadataDoubleNode.ClassNodeTypeName: string;
begin
  Result := 'MetadataDouble';
end;

class function TMetadataDoubleNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMetadataFloatNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  Fields.Add(FFdName);

  FFdReference := TSFString.Create(Self, 'reference', '');
  Fields.Add(FFdReference);

  FFdValue := TMFFloat.Create(Self, 'value', []);
  Fields.Add(FFdValue);

  DefaultContainerField := 'metadata';
end;

class function TMetadataFloatNode.ClassNodeTypeName: string;
begin
  Result := 'MetadataFloat';
end;

class function TMetadataFloatNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMetadataIntegerNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  Fields.Add(FFdName);

  FFdReference := TSFString.Create(Self, 'reference', '');
  Fields.Add(FFdReference);

  FFdValue := TMFInt32.Create(Self, 'value', []);
  Fields.Add(FFdValue);

  DefaultContainerField := 'metadata';
end;

class function TMetadataIntegerNode.ClassNodeTypeName: string;
begin
  Result := 'MetadataInteger';
end;

class function TMetadataIntegerNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMetadataSetNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  Fields.Add(FFdName);

  FFdReference := TSFString.Create(Self, 'reference', '');
  Fields.Add(FFdReference);

  FFdValue := TMFNode.Create(Self, 'value', IAbstractMetadataObject);
  Fields.Add(FFdValue);

  DefaultContainerField := 'metadata';
end;

class function TMetadataSetNode.ClassNodeTypeName: string;
begin
  Result := 'MetadataSet';
end;

class function TMetadataSetNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMetadataStringNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  Fields.Add(FFdName);

  FFdReference := TSFString.Create(Self, 'reference', '');
  Fields.Add(FFdReference);

  FFdValue := TMFString.Create(Self, 'value', []);
  Fields.Add(FFdValue);

  DefaultContainerField := 'metadata';
end;

class function TMetadataStringNode.ClassNodeTypeName: string;
begin
  Result := 'MetadataString';
end;

class function TMetadataStringNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TWorldInfoNode.CreateNode;
begin
  inherited;

  FFdInfo := TMFString.Create(Self, 'info', []);
  FFdInfo.Exposed := false;
  Fields.Add(FFdInfo);

  FFdTitle := TSFString.Create(Self, 'title', '');
  FFdTitle.Exposed := false;
  Fields.Add(FFdTitle);
end;

class function TWorldInfoNode.ClassNodeTypeName: string;
begin
  Result := 'WorldInfo';
end;

class function TWorldInfoNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure RegisterCoreNodes;
begin
  NodesManager.RegisterNodeClasses([
    TMetadataDoubleNode,
    TMetadataFloatNode,
    TMetadataIntegerNode,
    TMetadataSetNode,
    TMetadataStringNode,
    TWorldInfoNode
  ]);
end;

{$endif read_implementation}