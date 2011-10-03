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

{ Save routines, common for both XML and classic encoding. }

{$ifdef read_interface}

{ Which VRML/X3D version should be used to save this 3D model. }
function SaveVRMLVersion(Node: TX3DNode): TVRMLVersion;

{ Write VRML/X3D model to a file.
  Generates a complete file, with header lines (XML headers,
  or #VRML / #X3D in classic encoding) and everything.

  Generator and Source, if not empty, will be used to set appropriate
  META values of X3D root file node. This way you can indicate inside X3D file
  the generator (your program) name and source (original 3D model file name).
  If this is not an X3D root file node, we will record it inside a comment.

  When ForceConvertingToX3D or when Encoding <> xeClassic,
  the node graph will be converted to X3D (if it isn't X3D already).
  This can be used to convert VRML 2.0 to X3D.
  For VRML 1.0 or Inventor, this will just save them in X3D (XML or classic)
  encoding, which is not really usable. We simply don't implement
  VRML 1.0/Inventor convertion (it's difficult, and not worth the trouble,
  VRML 1.0/Inventor are really old).

  Note that converting to X3D will change the nodes graph, possibly
  removing some nodes. If you use TVRMLScene, you should call
  TVRMLScene.BeforeNodesFree before and TVRMLScene.ChangedAll
  when such convertion occcurs (you can easily test whether it may
  happen by SaveVRMLWillConvertToX3D). If you use Encoding=xeClassic
  and ForceConvertingToX3D=false, you're safe: convertion will not happen.

  @param(Version Which VRML/X3D specification version should be used
    to encode. It should be calculated by SaveVRMLVersion(Node).
    You often want to calculate it yourself, before calling SaveVRML,
    this way you can propose a sensible saved model extension for user
    (.wrl for VRML <= 2.0 in classic encoding, .x3dv for X3D in classic encoding).

    Note that @italic(this parameter is @bold(not) a mechanism to convert
    between various VRML/X3D versions). This procedure does not convert VRML/X3D
    nodes/fields inside. For example, you can't just change Version.Major value
    from 1 to 3 to convert VRML 1.0 to X3D.
    It would cause some information to be encoded in X3D style
    (e.g. generated file will have X3D header and PROFILE information),
    but the nodes would stil have VRML 1.0 names and fields.
    Which means that you would not generate correct X3D file.

    So this should really be calculated based on model contents,
    usually by SaveVRMLVersion(Node).

    A limited VRML 2.0 convertion is possible by ForceConvertingToX3D = true.
    In this case, Version will be automatically changed to X3D anyway.)

  @groupBegin }
procedure SaveVRML(Node: TX3DNode;
  Stream: TStream; const Generator, Source: string;
  Version: TVRMLVersion; const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean = false); overload;
procedure SaveVRML(Node: TX3DNode;
  const Filename, Generator, Source: string;
  const Version: TVRMLVersion; const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean = false); overload;
procedure SaveVRML(Node: TX3DNode;
  Stream: TStream; const Generator, Source: string;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean = false); overload;
procedure SaveVRML(Node: TX3DNode;
  const Filename, Generator, Source: string;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean = false); overload;
{ @groupEnd }

function SaveVRMLWillConvertToX3D(Version: TVRMLVersion;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean): boolean;

{$endif read_interface}

{$ifdef read_implementation}

type
  { TX3DWriter extended with NodeNames. }
  TX3DWriterNames = class(TX3DWriter)
    { Record known node names.
      This allows to write correct DEF / USE when saving. }
    NodeNames: TX3DNodeNames;

    constructor Create(AStream: TStream;
      const AVersion: TVRMLVersion; const AEncoding: TX3DEncoding);
    destructor Destroy; override;
  end;

constructor TX3DWriterNames.Create(AStream: TStream;
  const AVersion: TVRMLVersion; const AEncoding: TX3DEncoding);
begin
  inherited;
  NodeNames := TX3DNodeNames.Create(false);
end;

destructor TX3DWriterNames.Destroy;
begin
  FreeAndNil(NodeNames);
  inherited;
end;

const
  { Used when we have to save as X3D, but profile information is missing. }
  DefaultX3DSaveProfile = 'Interchange';

function SaveVRMLVersion(Node: TX3DNode): TVRMLVersion;
begin
  if (Node is TX3DRootNode) and
     TX3DRootNode(Node).HasForceVersion then
    Result := TX3DRootNode(Node).ForceVersion else
    { Use latest X3D. We should promote latest, best version of VRML/X3D.
      Also, this avoids convertion to X3D when saving it to XML encoding. }
    Result := X3DVersion;
end;

type
  TVRMLToX3DConverter = class
    procedure Convert(ParentNode: TX3DNode; var Node: TX3DNode);
  end;

procedure TVRMLToX3DConverter.Convert(ParentNode: TX3DNode; var Node: TX3DNode);

  function CoordinateNode(Coords: TMFVec3f): TCoordinateNode;
  begin
    Result := TCoordinateNode.Create('', Node.WWWBasePath);
    Result.FdPoint.AssignValue(Coords);
  end;

  function ConvertNurbsCurve(Node: TNurbsCurveNode_2): TNurbsCurveNode;
  begin
    Result := TNurbsCurveNode.Create(Node.NodeName, Node.WWWBasePath);
    Result.PositionInParent := Node.PositionInParent;

    Result.FdcontrolPoint.Value := CoordinateNode(Node.FdcontrolPoint);
    Result.Fdweight.Items.Assign(Node.FdWeight.Items); { converts float to double }
    Result.Fdtessellation.AssignValue(Node.Fdtessellation);
    Result.Fdknot.Items.Assign(Node.Fdknot.Items); { converts float to double }
    Result.Fdorder.AssignValue(Node.Fdorder);
    Result.FdClosed.Value := true; { true means "possibly closed" }
  end;

  function ConvertNurbsSurface(Node: TNurbsSurfaceNode): TNurbsPatchSurfaceNode;
  begin
    Result := TNurbsPatchSurfaceNode.Create(Node.NodeName, Node.WWWBasePath);
    Result.PositionInParent := Node.PositionInParent;

    Result.FdcontrolPoint.Value := CoordinateNode(Node.FdcontrolPoint);
    Result.FdtexCoord.AssignValue(Node.FdtexCoord);
    Result.FduTessellation.AssignValue(Node.FduTessellation);
    Result.FdvTessellation.AssignValue(Node.FdvTessellation);
    Result.Fdweight.Items.Assign(Node.FdWeight.Items); { converts float to double }
    Result.FdSolid.AssignValue(Node.FdSolid);

    Result.FdUClosed.Value := true; { true means "possibly closed" }
    Result.FdUDimension.AssignValue(Node.FdUDimension);
    Result.FdUKnot.Items.Assign(Node.FdUknot.Items); { converts float to double }
    Result.FdUOrder.AssignValue(Node.FdUorder);

    Result.FdVClosed.Value := true; { true means "possibly closed" }
    Result.FdVDimension.AssignValue(Node.FdVDimension);
    Result.FdVKnot.Items.Assign(Node.FdVKnot.Items); { converts float to double }
    Result.FdVOrder.AssignValue(Node.FdVOrder);
  end;

  function ConvertNurbsPositionInterpolator(Node: TNurbsPositionInterpolatorNode_2): TNurbsPositionInterpolatorNode;
  begin
    Result := TNurbsPositionInterpolatorNode.Create(Node.NodeName, Node.WWWBasePath);
    Result.PositionInParent := Node.PositionInParent;

    Result.FdcontrolPoint.Value := CoordinateNode(Node.FdKeyValue);
    Result.Fdknot.Items.Assign(Node.Fdknot.Items); { converts float to double }
    Result.Fdorder.AssignValue(Node.Fdorder);
    Result.Fdweight.Items.Assign(Node.FdKeyWeight.Items); { converts float to double }
  end;

begin
  { The nodes needing to be converted from VRML 2.0 to X3D:
    - Some can be found by grepping for "Version.Major = 2".
      This will find which nodes have ForVRMLVersion method requiring
      only VRML 2.0. These are only:
      TLODNode_2, TNurbsCurveNode_2, TNurbsPositionInterpolatorNode_2.
    - TLODNode_2 doesn't actually need convertion. Only a field name
      changes, which should be taken care of NameForVersion mechanism.
    - TNurbsSurfaceNode needs additionally to be changed in
      TNurbsPatchSurfaceNode.

    NURBS differences are summarized in
    http://castle-engine.sourceforge.net/vrml_implementation_nurbs.php#section_vrml2_support

    TODO: this simple replacement will not work for cases when:
    - you have ROUTEs leading to old nodes (ROUTEs will become disconnected
      when destroying old nodes)
    - you rely of reUSEing old nodes (in new version, we will have new node
      for each previous reference).
    - If you declare your VRML 2 nurbs nodes by EXTERNPROTO
      (like White Dune does) e.g. by "urn:web3d:vrml97:node:NurbsCurve"
      we *will* convert your NurbsCurve,
      but we will not remove (or modify) the EXTERNPROTO.
      If you save to classic encoding, this will cause problems,
      as when reading we will see EXTERNPROTO and try to read VRML 2 version
      of the node.
      If you save to XML encoding, this is avoided, as then we have
      <NurbsCurve> instead of <ProtoInstance name="NurbsCurve">,
      and the invalid externproto is not relevant.
  }
  if Node is TNurbsCurveNode_2 then
    Node := ConvertNurbsCurve(TNurbsCurveNode_2(Node)) else
  if Node is TNurbsSurfaceNode then
    Node := ConvertNurbsSurface(TNurbsSurfaceNode(Node)) else
  if Node is TNurbsPositionInterpolatorNode_2 then
    Node := ConvertNurbsPositionInterpolator(TNurbsPositionInterpolatorNode_2(Node));
end;

function SaveVRMLWillConvertToX3D(Version: TVRMLVersion;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean): boolean;
begin
  Result := (Version.Major < 3) and
    (ForceConvertingToX3D or (Encoding <> xeClassic));
end;

procedure SaveVRML(Node: TX3DNode;
  Stream: TStream; const Generator, Source: string;
  Version: TVRMLVersion;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean);
var
  Writer: TX3DWriterNames;

  procedure ConvertToX3D;
  begin
    { Converts VRML (1, 2) to X3D simply by changing Version used to save.
      This will work fine for VRML 2 -> X3D (except for some NURBS nodes
      different between VRML 2 and X3D, these will be dealt with TVRMLToX3DConverter).
      This will not really work for VRML 1. }
    if Version.Major <= 1 then
      OnWarning(wtMajor, 'VRML/X3D', 'Converting VRML 1.0 or Inventor to X3D is not implemented. You will get VRML 1.0/Inventor nodes encoded like for X3D, which probably is not much useful.');
    Version.Major := 3;
    Version.Minor := 0;

    { Since we convert in-place (replacing nodes inside),
      then Node must be a TX3DRootNode to replace in it the version.
      Fortunately, our SaveVRMLVersion already guarantees it:
      if a node isn't TX3DRootNode, it's always treated as X3D already,
      so a convertion will not happen. }
    Assert(Node is TX3DRootNode, 'Nodes must be TX3DRootNode to convert to X3D, to mark new version.');
    (Node as TX3DRootNode).HasForceVersion := true;
    (Node as TX3DRootNode).ForceVersion := Version;

    Node.EnumerateReplaceChildren(@TVRMLToX3DConverter(nil).Convert);
  end;

  procedure Classic;

    procedure SaveProfile(Root: TX3DRootNode);
    begin
      if Root.Profile <> '' then
        Writer.Writeln('PROFILE ' + Root.Profile + NL) else
        Writer.Writeln('PROFILE ' + DefaultX3DSaveProfile + NL);
    end;

    procedure SaveComponents(Root: TX3DRootNode);
    var
      I: Integer;
    begin
      for I := 0 to Root.Components.Count - 1 do
        Writer.Writeln(Format('COMPONENT %s:%d',
          [ Root.Components.Keys[I],
            Root.Components.Data[I] ]));
      if Root.Components.Count <> 0 then
        Writer.Write(NL);
    end;

    procedure SaveMetas(Root: TX3DRootNode);
    var
      I: Integer;
    begin
      for I := 0 to Root.Meta.Count - 1 do
        Writer.Writeln(Format('META %s %s',
          [ StringToX3DClassic(Root.Meta.Keys[I]),
            StringToX3DClassic(Root.Meta.Data[I])]));
      if Root.Meta.Count <> 0 then
        Writer.Write(NL);
    end;

  const
    VRML10Header = '#VRML V1.0 ascii';
    VRML20Header = '#VRML V2.0 utf8';
    X3DHeader = '#X3D V%d.%d utf8';
  var
    VRMLHeader, PrecedingComment: string;
  begin
    if Version.Major <= 1 then
      VRMLHeader := VRML10Header else
    if Version.Major = 2 then
      VRMLHeader := VRML20Header else
    if Version.Major >= 3 then
      VRMLHeader := Format(X3DHeader, [Version.Major, Version.Minor]);

    Writer.Writeln(VRMLHeader + NL { yes, one more NL, to look good });

    if (Version.Major >= 3) and (Node is TX3DRootNode) then
    begin
      if Generator <> '' then
        TX3DRootNode(Node).Meta.PutPreserve('generator', Generator);
      if Source <> '' then
        TX3DRootNode(Node).Meta.PutPreserve('source', Source);
      SaveProfile(TX3DRootNode(Node));
      SaveComponents(TX3DRootNode(Node));
      SaveMetas(TX3DRootNode(Node));
    end else
    if Version.Major >= 3 then
    begin
      { X3D, but no TX3DRootNode. Use default X3D profile. }
      Writer.Writeln('PROFILE ' + DefaultX3DSaveProfile);
      if Generator <> '' then
        Writer.Writeln(Format('META %s %s',
          [ StringToX3DClassic('generator'),
            StringToX3DClassic(Generator)]));
      if Source <> '' then
        Writer.Writeln(Format('META %s %s',
          [ StringToX3DClassic('source'),
            StringToX3DClassic(Source)]));
      Writer.Writeln;
    end else
    if (Generator <> '') or (Source <> '') then
    begin
      { write Generator and Source as comment }
      PrecedingComment := '';
      if Generator <> '' then
        PrecedingComment += '# Generated by ' + SReplaceChars(Generator, [#10, #13], ' ') + NL;
      if Source <> '' then
        PrecedingComment += '# Source model: ' + SReplaceChars(Source, [#10, #13], ' ') + NL;
      Writer.Writeln(PrecedingComment);
    end;

    { Node may be TX3DRootNode here, that's OK,
      TX3DRootNode.SaveToStream will magically handle this right. }
    Node.SaveToStream(Writer);
  end;

  procedure Xml;

    function Profile: string;
    begin
      if (Node is TX3DRootNode) and
         (TX3DRootNode(Node).Profile <> '') then
        Result := TX3DRootNode(Node).Profile else
        Result := DefaultX3DSaveProfile;
    end;

    procedure SaveComponents(Root: TX3DRootNode);
    var
      I: Integer;
    begin
      for I := 0 to Root.Components.Count - 1 do
        Writer.WritelnIndent(Format('<component name=%s level="%d" />',
          [ StringToX3DXml(Root.Components.Keys[I]),
            Root.Components.Data[I] ]));
    end;

    procedure SaveMetas(Root: TX3DRootNode);
    var
      I: Integer;
    begin
      for I := 0 to Root.Meta.Count - 1 do
        Writer.WritelnIndent(Format('<meta name=%s content=%s />',
          [ StringToX3DXml(Root.Meta.Keys[I]),
            StringToX3DXml(Root.Meta.Data[I])]));
    end;

  begin
    Writer.Writeln(Format(
      '<?xml version="1.0" encoding="UTF-8"?>' + NL +
      '<!DOCTYPE X3D PUBLIC "ISO//Web3D//DTD X3D %d.%d//EN" "http://www.web3d.org/specifications/x3d-%0:d.%1:d.dtd">' + NL+
      '<X3D profile=%2:s version="%0:d.%1:d"' + NL+
      '     xmlns:xsd="http://www.w3.org/2001/XMLSchema-instance"' + NL+
      '     xsd:noNamespaceSchemaLocation="http://www.web3d.org/specifications/x3d-%0:d.%1:d.xsd">' + NL+
      '<head>',
      [Version.Major, Version.Minor, StringToX3DXml(Profile)]));
    Writer.IncIndent;

    if Node is TX3DRootNode then
    begin
      if Generator <> '' then
        TX3DRootNode(Node).Meta.PutPreserve('generator', Generator);
      if Source <> '' then
        TX3DRootNode(Node).Meta.PutPreserve('source', Source);
      SaveComponents(TX3DRootNode(Node));
      SaveMetas(TX3DRootNode(Node));
    end else
    begin
      { write Generator etc. directly, as we don't have TX3DRootNode.Meta available }
      if Generator <> '' then
        Writer.WritelnIndent(Format('<meta name="generator" content=%s />',
          [StringToX3DXml(Generator)]));
      if Source <> '' then
        Writer.WritelnIndent(Format('<meta name="source" content=%s />',
          [StringToX3DXml(Source)]));
    end;

    Writer.DecIndent;
    Writer.Writeln('</head>' + NL + '<Scene>');

    { Node may be TX3DRootNode here, that's OK,
      TX3DRootNode.SaveToStream will magically handle this right. }
    Writer.IncIndent;
    Node.SaveToStream(Writer);
    Writer.DecIndent;

    Writer.Writeln('</Scene>' +NL + '</X3D>');
  end;

begin
  if SaveVRMLWillConvertToX3D(Version, Encoding, ForceConvertingToX3D) then
    ConvertToX3D;

  Writer := TX3DWriterNames.Create(Stream, Version, Encoding);
  try
    case Encoding of
      xeClassic: Classic;
      xeXML:     Xml;
      else raise EInternalError.Create('Saving Encoding?');
    end;
  finally FreeAndNil(Writer) end;
end;

procedure SaveVRML(Node: TX3DNode;
  const Filename, Generator, Source: string; const Version: TVRMLVersion;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean);
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(Filename, fmCreate);
  try
    SaveVRML(Node, Stream, Generator, Source, Version,
      Encoding, ForceConvertingToX3D);
  finally Stream.Free end;
end;

procedure SaveVRML(Node: TX3DNode;
  Stream: TStream; const Generator, Source: string;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean);
begin
  SaveVRML(Node, Stream, Generator, Source, SaveVRMLVersion(Node),
    Encoding, ForceConvertingToX3D);
end;

procedure SaveVRML(Node: TX3DNode;
  const FileName, Generator, Source: string;
  const Encoding: TX3DEncoding;
  const ForceConvertingToX3D: boolean);
begin
  SaveVRML(Node, FileName, Generator, Source, SaveVRMLVersion(Node),
    Encoding, ForceConvertingToX3D);
end;

{$endif read_implementation}