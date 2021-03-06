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

{ VRML 97 nodes for HAnim 1.0 and 200x specifications.

  For X3D there's different file, x3d_h-anim, as for X3D nodes are
  prefixed with "HAnim" before their name.

  VRML 97 nodes actually inherit from X3D nodes, so you should write
  your code to use X3D nodes (like THAnimDisplacerNode) and then VRML 97
  versions will also be handled automatically. }
{ }

{$ifdef read_interface}
  TDisplacerNode = class(THAnimDisplacerNode)
  public
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;

  THumanoidNode = class(THAnimHumanoidNode)
  public
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;

  TJointNode = class(THAnimJointNode)
  public
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;

  TSegmentNode = class(THAnimSegmentNode)
  public
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;

  TSiteNode = class(THAnimSiteNode)
  public
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
  end;
{$endif read_interface}

{$ifdef read_implementation}
class function TDisplacerNode.ClassNodeTypeName: string;
begin
  Result := 'Displacer';
end;

class function TDisplacerNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

class function THumanoidNode.ClassNodeTypeName: string;
begin
  Result := 'Humanoid';
end;

class function THumanoidNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

class function TJointNode.ClassNodeTypeName: string;
begin
  Result := 'Joint';
end;

class function TJointNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

class function TSegmentNode.ClassNodeTypeName: string;
begin
  Result := 'Segment';
end;

class function TSegmentNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

class function TSiteNode.ClassNodeTypeName: string;
begin
  Result := 'Site';
end;

class function TSiteNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName);
end;

procedure RegisterVRML97HAnimNodes;
begin
  NodesManager.RegisterNodeClasses([
    TDisplacerNode,
    THumanoidNode,
    TJointNode,
    TSegmentNode,
    TSiteNode
  ]);
end;
{$endif read_implementation}