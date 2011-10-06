{ -*- compile-command: "./compile_console.sh" -*- }
{
  Copyright 2010-2010 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

unit TestX3DNodesOptimizedProxy;

interface

uses
  Classes, SysUtils, fpcunit, testutils, testregistry, VectorMath, X3DNodes;

type
  TTestX3DNodesOptimizedProxy = class(TTestCase)
  published
    { Some of the geometry nodes, even though they have Proxy,
      have also optimized versions for some operations (like calculating
      bboxes).

      Test that they are actually used (not falling back on
      proxy), by giving them a nasty proxy that will bomb when you try
      to use it's methods. }
    procedure TestGeometryUsesOptimizedMethods;
  end;

implementation

uses Boxes3D, Shape;

{ TNastyProxy ---------------------------------------------------------------- }

type
  ENastyProxy = class(Exception);

  TNastyProxy = class(TAbstractGeometryNode)
    function BoundingBox(State: TVRMLGraphTraverseState;
      ProxyGeometry: TAbstractGeometryNode;
      ProxyState: TVRMLGraphTraverseState): TBox3D; override;
    function LocalBoundingBox(State: TVRMLGraphTraverseState;
      ProxyGeometry: TAbstractGeometryNode;
      ProxyState: TVRMLGraphTraverseState): TBox3D; override;
    function VerticesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
      ProxyGeometry: TAbstractGeometryNode;
      ProxyState: TVRMLGraphTraverseState): Cardinal; override;
    function TrianglesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
      ProxyGeometry: TAbstractGeometryNode;
      ProxyState: TVRMLGraphTraverseState): Cardinal; override;
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

function TNastyProxy.BoundingBox(State: TVRMLGraphTraverseState;
  ProxyGeometry: TAbstractGeometryNode;
  ProxyState: TVRMLGraphTraverseState): TBox3D;
begin
  raise ENastyProxy.Create('Something tried to use unoptimized BoundingBox');
  Result := EmptyBox3D; { silence compiler warnings }
end;

function TNastyProxy.LocalBoundingBox(State: TVRMLGraphTraverseState;
  ProxyGeometry: TAbstractGeometryNode;
  ProxyState: TVRMLGraphTraverseState): TBox3D;
begin
  raise ENastyProxy.Create('Something tried to use unoptimized LocalBoundingBox');
  Result := EmptyBox3D; { silence compiler warnings }
end;

function TNastyProxy.VerticesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
  ProxyGeometry: TAbstractGeometryNode;
  ProxyState: TVRMLGraphTraverseState): Cardinal;
begin
  raise ENastyProxy.Create('Something tried to use unoptimized VerticesCount');
  Result := 0; { silence compiler warnings }
end;

function TNastyProxy.TrianglesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
  ProxyGeometry: TAbstractGeometryNode;
  ProxyState: TVRMLGraphTraverseState): Cardinal;
begin
  raise ENastyProxy.Create('Something tried to use unoptimized TrianglesCount');
  Result := 0; { silence compiler warnings }
end;

function TNastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  raise ENastyProxy.Create('Something tried to use Proxy on TNastyProxy');
  Result := nil; { silence compiler warnings }
end;

{ Geometry nodes returning TNastyProxy --------------------------------------- }

type
  TConeNode_1_NastyProxy = class(TConeNode_1)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TConeNode_2_NastyProxy = class(TConeNode_2)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TCylinderNode_1_NastyProxy = class(TCylinderNode_1)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TCylinderNode_2_NastyProxy = class(TCylinderNode_2)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TBoxNode_NastyProxy = class(TBoxNode)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TCubeNode_1_NastyProxy = class(TCubeNode_1)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TSphereNode_1_NastyProxy = class(TSphereNode_1)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

  TSphereNode_2_NastyProxy = class(TSphereNode_2)
    function Proxy(var State: TVRMLGraphTraverseState;
      const OverTriangulate: boolean): TAbstractGeometryNode; override;
  end;

function TConeNode_1_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TConeNode_2_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TCylinderNode_1_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TCylinderNode_2_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TBoxNode_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TCubeNode_1_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TSphereNode_1_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

function TSphereNode_2_NastyProxy.Proxy(var State: TVRMLGraphTraverseState;
  const OverTriangulate: boolean): TAbstractGeometryNode;
begin
  Result := TNastyProxy.Create(NodeName, WWWBasePath);
end;

{ TTestX3DNodesOptimizedProxy ----------------------------------------------- }

procedure TTestX3DNodesOptimizedProxy.TestGeometryUsesOptimizedMethods;
var
  { Suffix O means OverTriangulate,
           NO means not OverTriangulate }
  NastyGeometry, GoodGeometry, ProxyGeometryO, ProxyGeometryNO: TAbstractGeometryNode;
  State, ProxyStateO, ProxyStateNO: TVRMLGraphTraverseState;
  NastyShape, ProxyShapeO, ProxyShapeNO: TShape;

  procedure FinalizeNode;
  begin
    FreeAndNil(ProxyShapeO);
    FreeAndNil(ProxyGeometryO);

    if ProxyStateO <> State then
      FreeAndNil(ProxyStateO) else
      ProxyStateO := nil;

    FreeAndNil(ProxyShapeNO);
    FreeAndNil(ProxyGeometryNO);

    if ProxyStateNO <> State then
      FreeAndNil(ProxyStateNO) else
      ProxyStateNO := nil;

    FreeAndNil(GoodGeometry);
    FreeAndNil(NastyShape);
    FreeAndNil(NastyGeometry);
  end;

  procedure InitializeNode(NodeClass: TX3DNodeClass; GoodNodeClass: TX3DNodeClass);
  begin
    FinalizeNode;
    NastyGeometry := (NodeClass.Create('', '')) as TAbstractGeometryNode;
    NastyShape := TShape.Create(nil, NastyGeometry, TVRMLGraphTraverseState.CreateCopy(State), nil);

    { create also proxy, inside it's own shape.
      This can be used to test that proxy results, *if* they would be used,
      would be the same. }
    GoodGeometry := (GoodNodeClass.Create('', '')) as TAbstractGeometryNode;

    ProxyStateO := State;
    ProxyGeometryO := GoodGeometry.Proxy(ProxyStateO, true);
    if ProxyGeometryO <> nil then
      ProxyShapeO := TShape.Create(nil, ProxyGeometryO, TVRMLGraphTraverseState.CreateCopy(ProxyStateO), nil) else
      ProxyShapeO := nil;

    ProxyStateNO := State;
    ProxyGeometryNO := GoodGeometry.Proxy(ProxyStateNO, false);
    if ProxyGeometryNO <> nil then
      ProxyShapeNO := TShape.Create(nil, ProxyGeometryNO, TVRMLGraphTraverseState.CreateCopy(ProxyStateNO), nil) else
      ProxyShapeNO := nil;
  end;

  procedure AssertBoxEqual(const B1, B2: TBox3D);
  begin
    Assert(FloatsEqual(B1.Data[0][0], B2.Data[0][0], 0.1));
    Assert(FloatsEqual(B1.Data[0][1], B2.Data[0][1], 0.1));
    Assert(FloatsEqual(B1.Data[0][2], B2.Data[0][2], 0.1));
    Assert(FloatsEqual(B1.Data[1][0], B2.Data[1][0], 0.1));
    Assert(FloatsEqual(B1.Data[1][1], B2.Data[1][1], 0.1));
    Assert(FloatsEqual(B1.Data[1][2], B2.Data[1][2], 0.1));
  end;

  { Check node has optimized (not using proxy) versions of
    BoundingBox, LocalBoundingBox, TrianglesCount.
    And check their results match results of the proxy. }
  procedure CheckNodeBBoxAndTrisCount;
  begin
    AssertBoxEqual(NastyShape.BoundingBox, ProxyShapeO.BoundingBox);
    AssertBoxEqual(NastyShape.BoundingBox, ProxyShapeNO.BoundingBox);
    AssertBoxEqual(NastyShape.LocalBoundingBox, ProxyShapeO.LocalBoundingBox);
    AssertBoxEqual(NastyShape.LocalBoundingBox, ProxyShapeNO.LocalBoundingBox);
    Assert(NastyShape.TrianglesCount(false) = ProxyShapeNO.TrianglesCount(false));
    Assert(NastyShape.TrianglesCount(true ) = ProxyShapeO .TrianglesCount(true ));
  end;

begin
  NastyGeometry := nil;
  GoodGeometry := nil;
  NastyShape := nil;
  ProxyShapeO := nil;
  ProxyShapeNO := nil;
  ProxyStateO := nil;
  ProxyStateNO := nil;
  ProxyGeometryO := nil;
  ProxyGeometryNO := nil;
  State := TVRMLGraphTraverseState.Create;
  try

    { create each node, and try to call the methods that should
      be optimized (should *not* call the proxy).
      For nodes that we know *have* some proxy, we can also compare
      do the optimized results match the proxy results.
      So we test do optimized BoundingBox etc.
      1. are used
      and
      2. return correct result }
    InitializeNode(TConeNode_1_NastyProxy, TConeNode_1);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TConeNode_2_NastyProxy, TConeNode_2);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TCylinderNode_1_NastyProxy, TCylinderNode_1);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TCylinderNode_2_NastyProxy, TCylinderNode_2);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TBoxNode_NastyProxy, TBoxNode);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TCubeNode_1_NastyProxy, TCubeNode_1);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TSphereNode_1_NastyProxy, TSphereNode_1);
    CheckNodeBBoxAndTrisCount;

    InitializeNode(TSphereNode_2_NastyProxy, TSphereNode_2);
    CheckNodeBBoxAndTrisCount;
  finally
    FinalizeNode;
    FreeAndNil(State);
  end;
end;

initialization
 RegisterTest(TTestX3DNodesOptimizedProxy);
end.