{
  Copyright 2007-2014 Michalis Kamburelis.

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
  TAbstractShaderNode = class(TAbstractAppearanceChildNode)
  public
    procedure CreateNode; override;

    { Event in } { }
    private FEventActivate: TSFBoolEvent;
    public property EventActivate: TSFBoolEvent read FEventActivate;

    { Event out } { }
    private FEventIsSelected: TSFBoolEvent;
    public property EventIsSelected: TSFBoolEvent read FEventIsSelected;

    { Event out } { }
    private FEventIsValid: TSFBoolEvent;
    public property EventIsValid: TSFBoolEvent read FEventIsValid;

    private FFdLanguage: TSFString;
    public property FdLanguage: TSFString read FFdLanguage;
  end;

  TAbstractVertexAttributeNode = class(TAbstractGeometricPropertyNode)
  public
    procedure CreateNode; override;

    private FFdName: TSFString;
    public property FdName: TSFString read FFdName;
  end;

  IAbstractProgrammableShaderObject = interface
  ['{B027FD9B-2A68-4653-8A26-91DB51BE47ED}']
  end;

  TComposedShaderNode = class(TAbstractShaderNode, IAbstractProgrammableShaderObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdParts: TMFNode;
    public property FdParts: TMFNode read FFdParts;
  end;

  TFloatVertexAttributeNode = class(TAbstractVertexAttributeNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdValue: TMFFloat;
    public property FdValue: TMFFloat read FFdValue;

    private FFdNumComponents: TSFInt32;
    public property FdNumComponents: TSFInt32 read FFdNumComponents;
  end;

  TMatrix3VertexAttributeNode = class(TAbstractVertexAttributeNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdValue: TMFMatrix3f;
    public property FdValue: TMFMatrix3f read FFdValue;
  end;

  TMatrix4VertexAttributeNode = class(TAbstractVertexAttributeNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdValue: TMFMatrix4f;
    public property FdValue: TMFMatrix4f read FFdValue;
  end;

  TPackagedShaderNode = class(TAbstractShaderNode, IAbstractUrlObject, IAbstractProgrammableShaderObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdUrl: TMFString;
    public property FdUrl: TMFString read FFdUrl;
  end;

  TProgramShaderNode = class(TAbstractShaderNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdPrograms: TMFNode;
    public property FdPrograms: TMFNode read FFdPrograms;
  end;

  TShaderPartNode = class(TAbstractNode, IAbstractUrlObject)
  private
    ContentsCached: boolean;
    ContentsCache: string;
    procedure EventUrlReceive(
      Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
    function GetShaderType: TShaderType;
    procedure SetShaderType(const Value: TShaderType);
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdUrl: TMFString;
    public property FdUrl: TMFString read FFdUrl;

    private FFdType: TSFStringEnum;
    public property FdType: TSFStringEnum read FFdType;

    { Shader type. }
    property ShaderType: TShaderType read GetShaderType write SetShaderType;

    { Load shader code from node's URLs.
      In X3D XML encoding, also loads node's XML CDATA as a last resort.
      Will return '' if no valid URLs are present,
      loading errors are reported to OnWarning. }
    function Contents: string;
  end;

  TShaderProgramNode = class(TAbstractNode, IAbstractUrlObject, IAbstractProgrammableShaderObject)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdUrl: TMFString;
    public property FdUrl: TMFString read FFdUrl;

    private FFdType: TSFStringEnum;
    public property FdType: TSFStringEnum read FFdType;
  end;
{$endif read_interface}

{$ifdef read_implementation}
procedure TAbstractShaderNode.CreateNode;
begin
  inherited;

  FEventActivate := TSFBoolEvent.Create(Self, 'activate', true);
  Events.Add(FEventActivate);

  FEventIsSelected := TSFBoolEvent.Create(Self, 'isSelected', false);
  Events.Add(FEventIsSelected);

  FEventIsValid := TSFBoolEvent.Create(Self, 'isValid', false);
  Events.Add(FEventIsValid);

  FFdLanguage := TSFString.Create(Self, 'language', '');
   FdLanguage.Exposed := false;
   FdLanguage.ChangesAlways := [chEverything];
  Fields.Add(FFdLanguage);
  { X3D specification comment: ["CG"|"GLSL"|"HLSL"|...] }

  DefaultContainerField := 'shaders';
end;

procedure TAbstractVertexAttributeNode.CreateNode;
begin
  inherited;

  FFdName := TSFString.Create(Self, 'name', '');
  FFdName.Exposed := false;
  Fields.Add(FFdName);

  DefaultContainerField := 'attrib';
end;

procedure TComposedShaderNode.CreateNode;
begin
  inherited;

  FFdParts := TMFNode.Create(Self, 'parts', [TShaderPartNode]);
   FdParts.ChangesAlways := [chEverything];
  Fields.Add(FFdParts);

  { Note that additional ComposedShader fields should have
      ChangesAlways := [chVisibleGeometry, chVisibleNonGeometry]
    because you want to redisplay when they change.

    But TGLRenderer takes care of it already. When shaders'
    uniform variables change, we call Scene.VisibleChangeHere.
    Implemented inside TX3DShaderProgram.EventReceive.

    TODO: but when you don't use events, but directly change them
    and call ChangedField, we should do something here? }

  HasInterfaceDeclarations := AllAccessTypes;
end;

class function TComposedShaderNode.ClassNodeTypeName: string;
begin
  Result := 'ComposedShader';
end;

class function TComposedShaderNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TFloatVertexAttributeNode.CreateNode;
begin
  inherited;

  FFdValue := TMFFloat.Create(Self, 'value', []);
  Fields.Add(FFdValue);
  { X3D specification comment: (-Inf,Inf) }

  FFdNumComponents := TSFInt32.Create(Self, 'numComponents', 4);
  FFdNumComponents.Exposed := false;
  Fields.Add(FFdNumComponents);
  { X3D specification comment: [1..4] }
end;

class function TFloatVertexAttributeNode.ClassNodeTypeName: string;
begin
  Result := 'FloatVertexAttribute';
end;

class function TFloatVertexAttributeNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMatrix3VertexAttributeNode.CreateNode;
begin
  inherited;

  FFdValue := TMFMatrix3f.Create(Self, 'value', []);
  Fields.Add(FFdValue);
  { X3D specification comment: (-Inf,Inf) }
end;

class function TMatrix3VertexAttributeNode.ClassNodeTypeName: string;
begin
  Result := 'Matrix3VertexAttribute';
end;

class function TMatrix3VertexAttributeNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TMatrix4VertexAttributeNode.CreateNode;
begin
  inherited;

  FFdValue := TMFMatrix4f.Create(Self, 'value', []);
  Fields.Add(FFdValue);
  { X3D specification comment: (-Inf,Inf) }
end;

class function TMatrix4VertexAttributeNode.ClassNodeTypeName: string;
begin
  Result := 'Matrix4VertexAttribute';
end;

class function TMatrix4VertexAttributeNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TPackagedShaderNode.CreateNode;
begin
  inherited;

  FFdUrl := TMFString.Create(Self, 'url', []);
  Fields.Add(FFdUrl);
  { X3D specification comment: [URI] }

  HasInterfaceDeclarations := AllAccessTypes;
  CDataField := FdUrl;
end;

class function TPackagedShaderNode.ClassNodeTypeName: string;
begin
  Result := 'PackagedShader';
end;

class function TPackagedShaderNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TProgramShaderNode.CreateNode;
begin
  inherited;

  FFdPrograms := TMFNode.Create(Self, 'programs', [TShaderProgramNode]);
  Fields.Add(FFdPrograms);
end;

class function TProgramShaderNode.ClassNodeTypeName: string;
begin
  Result := 'ProgramShader';
end;

class function TProgramShaderNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TShaderPartNode.CreateNode;
begin
  inherited;

  FFdUrl := TMFString.Create(Self, 'url', []);
   FdUrl.ChangesAlways := [chEverything]; { TODO - something more optimized }
   FdUrl.OnReceive.Add(@EventUrlReceive);
  Fields.Add(FFdUrl);
  { X3D specification comment: [URI] }

  FFdType := TSFStringEnum.Create(Self, 'type', ShaderTypeNameX3D, Ord(stVertex));
   FdType.Exposed := false;
   FdType.ChangesAlways := [chEverything];
  Fields.Add(FFdType);
  { X3D specification comment: ["VERTEX"|"FRAGMENT"] }

  DefaultContainerField := 'parts';
  CDataField := FdUrl;
end;

class function TShaderPartNode.ClassNodeTypeName: string;
begin
  Result := 'ShaderPart';
end;

function ShaderLoadContents(const Node: TX3DNode; const FdUrl: TMFString): string;

  function IsDirectShaderSource(const S: string): boolean;
  { URL is recognized as direct shader source if it
    doesn't start with any protocol *and*
    has some newlines (needed to recognize relative URLs without
    protocol correctly). }
  begin
    Result := (CharsPos([#10, #13], S) > 0) and (URIProtocol(S) = '');
  end;

var
  I: Integer;
  FullUrl, MimeType: string;
begin
  Result := '';

  for I := 0 to FdUrl.Items.Count - 1 do
  begin
    if IsDirectShaderSource(FdUrl.Items[I]) then
    begin
      Result := FdUrl.Items[I];
      Exit;
    end else
    begin
      FullUrl := Node.PathFromBaseUrl(FdUrl.Items[I]);
      try
        Result := FileToString(FullUrl, false, MimeType);
        if (MimeType <> '') and
           (MimeType <> 'text/plain') then
          OnWarning(wtMajor, 'VRML/X3D', Format('MIME type "%s" not recognized as shader source. Using anyway. If this is a valid shader source mime type that should be recognized, please submit a feature request.', [MimeType]));
        Exit;
      except
        on E: Exception do
          { Remember that OnWarning *may* raise an exception. }
          OnWarning(wtMinor, 'VRML/X3D', Format(SLoadError,
            [E.ClassName, 'shader source', URIDisplay(FullUrl), E.Message]));
      end;
    end;
  end;
end;

function TShaderPartNode.Contents: string;
begin
  if not ContentsCached then
  begin
    ContentsCache := ShaderLoadContents(Self, FdUrl);
    ContentsCached := true;
  end;
  Result := ContentsCache;
end;

procedure TShaderPartNode.EventUrlReceive(
  Event: TX3DEvent; Value: TX3DField; const Time: TX3DTime);
begin
  ContentsCached := false;
end;

class function TShaderPartNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TShaderPartNode.GetShaderType: TShaderType;
begin
  Result := TShaderType(FdType.EnumValue);
end;

procedure TShaderPartNode.SetShaderType(const Value: TShaderType);
begin
  FdType.SendEnumValue(Ord(Value));
end;

procedure TShaderProgramNode.CreateNode;
begin
  inherited;

  FFdUrl := TMFString.Create(Self, 'url', []);
  Fields.Add(FFdUrl);
  { X3D specification comment: [URI] }

  FFdType := TSFStringEnum.Create(Self, 'type', ShaderTypeNameX3D, Ord(stVertex));
  FFdType.Exposed := false;
  Fields.Add(FFdType);
  { X3D specification comment: ["VERTEX"|"FRAGMENT"] }

  DefaultContainerField := 'programs';

  HasInterfaceDeclarations := AllAccessTypes;
  CDataField := FdUrl;
end;

class function TShaderProgramNode.ClassNodeTypeName: string;
begin
  Result := 'ShaderProgram';
end;

class function TShaderProgramNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure RegisterShadersNodes;
begin
  NodesManager.RegisterNodeClasses([
    TComposedShaderNode,
    TFloatVertexAttributeNode,
    TMatrix3VertexAttributeNode,
    TMatrix4VertexAttributeNode,
    TPackagedShaderNode,
    TProgramShaderNode,
    TShaderPartNode,
    TShaderProgramNode
  ]);
end;
{$endif read_implementation}
