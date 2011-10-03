{
  Copyright 2008-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{$ifdef read_interface}
  { }
  TAbstractEnvironmentalSensorNode = class(TAbstractSensorNode)
  public
    procedure CreateNode; override;

    private FFdCenter: TSFVec3f;
    public property FdCenter: TSFVec3f read FFdCenter;

    private FFdSize: TSFVec3f;
    public property FdSize: TSFVec3f read FFdSize;

    { Event: SFTime, out } { }
    private FEventEnterTime: TVRMLEvent;
    public property EventEnterTime: TVRMLEvent read FEventEnterTime;

    { Event: SFTime, out } { }
    private FEventExitTime: TVRMLEvent;
    public property EventExitTime: TVRMLEvent read FEventExitTime;
  end;

  TProximitySensorNode = class(TAbstractEnvironmentalSensorNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
    function TransformationChange: TNodeTransformationChange; override;

    { Event: SFVec3f, out } { }
    private FEventCenterOfRotation_changed: TVRMLEvent;
    public property EventCenterOfRotation_changed: TVRMLEvent read FEventCenterOfRotation_changed;

    { Event: SFRotation, out } { }
    private FEventOrientation_changed: TVRMLEvent;
    public property EventOrientation_changed: TVRMLEvent read FEventOrientation_changed;

    { Event: SFVec3f, out } { }
    private FEventPosition_changed: TVRMLEvent;
    public property EventPosition_changed: TVRMLEvent read FEventPosition_changed;
  end;

  TTransformSensorNode = class(TAbstractEnvironmentalSensorNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdTargetObject: TSFNode;
    public property FdTargetObject: TSFNode read FFdTargetObject;

    { Event: SFRotation, out } { }
    private FEventOrientation_changed: TVRMLEvent;
    public property EventOrientation_changed: TVRMLEvent read FEventOrientation_changed;

    { Event: SFVec3f, out } { }
    private FEventPosition_changed: TVRMLEvent;
    public property EventPosition_changed: TVRMLEvent read FEventPosition_changed;
  end;

  TVisibilitySensorNode = class(TAbstractEnvironmentalSensorNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;

{$endif read_interface}

{$ifdef read_implementation}
procedure TAbstractEnvironmentalSensorNode.CreateNode;
begin
  inherited;

  FFdCenter := TSFVec3f.Create(Self, 'center', Vector3Single(0, 0, 0));
   FdCenter.ChangesAlways := [chEnvironmentalSensorBounds];
  Fields.Add(FFdCenter);
  { X3D specification comment: (-Inf,Inf) }

  FFdSize := TSFVec3f.Create(Self, 'size', Vector3Single(0, 0, 0));
   FdSize.ChangesAlways := [chEnvironmentalSensorBounds];
  Fields.Add(FFdSize);
  { X3D specification comment: (-Inf,Inf) }

  FEventEnterTime := TVRMLEvent.Create(Self, 'enterTime', TSFTime, false);
  Events.Add(FEventEnterTime);

  FEventExitTime := TVRMLEvent.Create(Self, 'exitTime', TSFTime, false);
  Events.Add(FEventExitTime);

  DefaultContainerField := 'children';
end;

procedure TProximitySensorNode.CreateNode;
begin
  inherited;

  FEventCenterOfRotation_changed := TVRMLEvent.Create(Self, 'centerOfRotation_changed', TSFVec3f, false);
  Events.Add(FEventCenterOfRotation_changed);

  FEventOrientation_changed := TVRMLEvent.Create(Self, 'orientation_changed', TSFRotation, false);
  Events.Add(FEventOrientation_changed);

  FEventPosition_changed := TVRMLEvent.Create(Self, 'position_changed', TSFVec3f, false);
  Events.Add(FEventPosition_changed);

  DefaultContainerField := 'children';
end;

class function TProximitySensorNode.ClassNodeTypeName: string;
begin
  Result := 'ProximitySensor';
end;

class function TProximitySensorNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TProximitySensorNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcProximitySensor;
end;

procedure TTransformSensorNode.CreateNode;
begin
  inherited;

  FFdTargetObject := TSFNode.Create(Self, 'targetObject', [TAbstractX3DGroupingNode, TAbstractShapeNode]);
  Fields.Add(FFdTargetObject);

  FEventOrientation_changed := TVRMLEvent.Create(Self, 'orientation_changed', TSFRotation, false);
  Events.Add(FEventOrientation_changed);

  FEventPosition_changed := TVRMLEvent.Create(Self, 'position_changed', TSFVec3f, false);
  Events.Add(FEventPosition_changed);

  DefaultContainerField := 'children';
end;

class function TTransformSensorNode.ClassNodeTypeName: string;
begin
  Result := 'TransformSensor';
end;

class function TTransformSensorNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TVisibilitySensorNode.CreateNode;
begin
  inherited;

  DefaultContainerField := 'children';
end;

class function TVisibilitySensorNode.ClassNodeTypeName: string;
begin
  Result := 'VisibilitySensor';
end;

class function TVisibilitySensorNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure RegisterEnvironmentalSensorNodes;
begin
  NodesManager.RegisterNodeClasses([
    TProximitySensorNode,
    TTransformSensorNode,
    TVisibilitySensorNode
  ]);
end;

{$endif read_implementation}