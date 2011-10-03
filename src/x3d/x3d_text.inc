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
  { }
  TAbstractFontStyleNode = class(TAbstractNode)
  public
    procedure CreateNode; override;
  end;

  { Font family that can be specified by FontStyle node in family
    field. First three fields are equal (after casting by Ord) to
    three values of FSFAMILY_* constants. }
  TVRMLFontFamily = (ffSerif, ffSans, ffTypeWriter);

  { Font justification that can be specified by FontStyle in
    justify/justification field. First three fields are equal
    (after casting by Ord) to JUSTIFICATION_* constants. }
  TVRMLFontJustify = (fjBegin, fjMiddle, fjEnd);

  TFontStyleNode = class(TAbstractFontStyleNode)
  private
    JustifyWarningUppercaseDone, JustifyWarningObsoleteDone,
      JustifyWarningNotSupportedDone: boolean;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdFamily: TMFString;
    public property FdFamily: TMFString read FFdFamily;

    private FFdHorizontal: TSFBool;
    public property FdHorizontal: TSFBool read FFdHorizontal;

    private FFdJustify: TMFString;
    public property FdJustify: TMFString read FFdJustify;

    private FFdLanguage: TSFString;
    public property FdLanguage: TSFString read FFdLanguage;

    private FFdLeftToRight: TSFBool;
    public property FdLeftToRight: TSFBool read FFdLeftToRight;

    private FFdSize: TSFFloat;
    public property FdSize: TSFFloat read FFdSize;

    private FFdSpacing: TSFFloat;
    public property FdSpacing: TSFFloat read FFdSpacing;

    private FFdStyle: TSFString;
    public property FdStyle: TSFString read FFdStyle;

    private FFdTopToBottom: TSFBool;
    public property FdTopToBottom: TSFBool read FFdTopToBottom;

    class function ForVRMLVersion(const Version: TVRMLVersion): boolean;
      override;

    function Family: TVRMLFontFamily;
    function Bold: boolean;
    function Italic: boolean;
    function Justify: TVRMLFontJustify;
    function TTF_Font: PTrueTypeFont;

    class function DefaultSize: Single;
    class function DefaultSpacing: Single;
    class function DefaultFamily: TVRMLFontFamily;
    class function DefaultBold: boolean;
    class function DefaultItalic: boolean;
    class function DefaultJustify: TVRMLFontJustify;
    class function DefaultTTF_Font: PTrueTypeFont;

    class function ClassTTF_Font(AFamily: TVRMLFontFamily;
      const ABold, AItalic: boolean): PTrueTypeFont;
  end;
  TFontStyleNode_2 = TFontStyleNode;

  TTextNode = class(TAbstractX3DGeometryNode)
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdFontStyle: TSFNode;
    public property FdFontStyle: TSFNode read FFdFontStyle;

    private FFdLength: TMFFloat;
    public property FdLength: TMFFloat read FFdLength;

    private FFdMaxExtent: TSFFloat;
    public property FdMaxExtent: TSFFloat read FFdMaxExtent;

    private FFdString: TMFString;
    public property FdString: TMFString read FFdString;

    { Event: MFVec2f, out } { }
    private FEventLineBounds: TVRMLEvent;
    public property EventLineBounds: TVRMLEvent read FEventLineBounds;

    { Event: SFVec3f, out } { }
    private FEventOrigin: TVRMLEvent;
    public property EventOrigin: TVRMLEvent read FEventOrigin;

    { Event: SFVec2f, out } { }
    private FEventTextBounds: TVRMLEvent;
    public property EventTextBounds: TVRMLEvent read FEventTextBounds;

    private FFdSolid: TSFBool;
    public property FdSolid: TSFBool read FFdSolid;

    function LocalBoundingBox(State: TVRMLGraphTraverseState;
      ProxyGeometry: TAbstractGeometryNode; ProxyState: TVRMLGraphTraverseState): TBox3D; override;
    function VerticesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
      ProxyGeometry: TAbstractGeometryNode; ProxyState: TVRMLGraphTraverseState): Cardinal; override;
    function TrianglesCount(State: TVRMLGraphTraverseState; OverTriangulate: boolean;
      ProxyGeometry: TAbstractGeometryNode; ProxyState: TVRMLGraphTraverseState): Cardinal; override;

    { This returns FdFontStyle.Value. Returns nil if FdFontStyle.Value
      is nil or if it's not TFontStyleNode. }
    function FontStyle: TFontStyleNode;
  end;

{$endif read_interface}

{$ifdef read_implementation}
procedure TAbstractFontStyleNode.CreateNode;
begin
  inherited;

  DefaultContainerField := 'fontStyle';
end;

const
  TTF_Font_Results: array[TVRMLFontFamily, boolean, boolean]of PTrueTypeFont =
  (              {   [],                          [italic],                            [bold],                      [italic, bold] }
    {serif}      ( ((@TTF_BitstreamVeraSerif),   (@TTF_BitstreamVeraSerif_Italic)),    ((@TTF_BitstreamVeraSerif_Bold),    (@TTF_BitstreamVeraSerif_Bold_Italic)) ),
    {sans}       ( ((@TTF_BitstreamVeraSans),    (@TTF_BitstreamVeraSans_Italic)),     ((@TTF_BitstreamVeraSans_Bold),     (@TTF_BitstreamVeraSans_Bold_Italic)) ),
    {typewriter} ( ((@TTF_BitstreamVeraSansMono),(@TTF_BitstreamVeraSansMono_Italic)), ((@TTF_BitstreamVeraSansMono_Bold), (@TTF_BitstreamVeraSansMono_Bold_Italic)) )
  );

procedure TFontStyleNode.CreateNode;
begin
  inherited;

  FFdFamily := TMFString.Create(Self, 'family', ['SERIF']);
   FdFamily.Exposed := false;
   FdFamily.ChangesAlways := [chFontStyle];
  Fields.Add(FFdFamily);

  FFdHorizontal := TSFBool.Create(Self, 'horizontal', true);
   FdHorizontal.Exposed := false;
   FdHorizontal.ChangesAlways := [chFontStyle];
  Fields.Add(FFdHorizontal);

  FFdJustify := TMFString.Create(Self, 'justify', ['BEGIN']);
   FdJustify.Exposed := false;
   FdJustify.ChangesAlways := [chFontStyle];
  Fields.Add(FFdJustify);
  { X3D specification comment: ["BEGIN","END","FIRST","MIDDLE",""] }

  FFdLanguage := TSFString.Create(Self, 'language', '');
   FdLanguage.Exposed := false;
   FdLanguage.ChangesAlways := [chFontStyle];
  Fields.Add(FFdLanguage);

  FFdLeftToRight := TSFBool.Create(Self, 'leftToRight', true);
   FdLeftToRight.Exposed := false;
   FdLeftToRight.ChangesAlways := [chFontStyle];
  Fields.Add(FFdLeftToRight);

  FFdSize := TSFFloat.Create(Self, 'size', DefaultSize);
   FdSize.Exposed := false;
   FdSize.ChangesAlways := [chFontStyle];
  Fields.Add(FFdSize);
  { X3D specification comment: (0,Inf) }

  FFdSpacing := TSFFloat.Create(Self, 'spacing', DefaultSpacing);
   FdSpacing.Exposed := false;
   FdSpacing.ChangesAlways := [chFontStyle];
  Fields.Add(FFdSpacing);
  { X3D specification comment: [0,Inf) }

  FFdStyle := TSFString.Create(Self, 'style', 'PLAIN');
   FdStyle.Exposed := false;
   FdStyle.ChangesAlways := [chFontStyle];
  Fields.Add(FFdStyle);
  { X3D specification comment: ["PLAIN"|"BOLD"|"ITALIC"|"BOLDITALIC"|""] }

  FFdTopToBottom := TSFBool.Create(Self, 'topToBottom', true);
   FdTopToBottom.Exposed := false;
   FdTopToBottom.ChangesAlways := [chFontStyle];
  Fields.Add(FFdTopToBottom);
end;

class function TFontStyleNode.ClassNodeTypeName: string;
begin
  Result := 'FontStyle';
end;

class function TFontStyleNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TFontStyleNode.ForVRMLVersion(const Version: TVRMLVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

function TFontStyleNode.TTF_Font: PTrueTypeFont;
begin
  Result := TTF_Font_Results[Family, Bold, Italic];
end;

class function TFontStyleNode.ClassTTF_Font(
  AFamily: TVRMLFontFamily; const ABold, AItalic: boolean): PTrueTypeFont;
begin
  Result := TTF_Font_Results[AFamily, ABold, AItalic];
end;

function TFontStyleNode.Family: TVRMLFontFamily;
var
  I: Integer;
begin
  for I := 0 to FdFamily.Items.Count - 1 do
    if FdFamily.Items[I] = 'SERIF' then
      Exit(ffSerif) else
    if FdFamily.Items[I] = 'SANS' then
      Exit(ffSans) else
    if FdFamily.Items[I] = 'TYPEWRITER' then
      Exit(ffTypeWriter) else
      OnWarning(wtMajor, 'VRML/X3D', 'Font family "' + FdFamily.Items[I] + '" not supported');

  { If no supported values on FdFamily.Items then fall back to serif }
  Result := ffSerif;
end;

const
  StyleBold = 'BOLD';
  StyleBoldItalic = 'BOLDITALIC';
  StyleItalic = 'ITALIC';
  StylePlain = 'PLAIN';

function TFontStyleNode.Bold: boolean;
begin
  Result :=
    (FdStyle.Value = StyleBold) or
    (FdStyle.Value = StyleBoldItalic);

  { This is the end of calculating Result.
    But we would like to make a warning in case of invalid FdStyle
    value, so we do check below. }

  if not Result then
  begin
    if not (
      (FdStyle.Value = StyleItalic) or
      (FdStyle.Value = StylePlain) or
      (FdStyle.Value = '')) then
      OnWarning(wtMajor, 'VRML/X3D', 'Font style "' + FdStyle.Value + '" not supported');
  end;
end;

function TFontStyleNode.Italic: boolean;
begin
  Result :=
    (FdStyle.Value = StyleItalic) or
    (FdStyle.Value = StyleBoldItalic);

  { This is the end of calculating Result.
    But we would like to make a warning in case of invalid FdStyle
    value, so we do check below. }

  if not Result then
  begin
    if not (
      (FdStyle.Value = StyleBold) or
      (FdStyle.Value = StylePlain) or
      (FdStyle.Value = '')) then
      OnWarning(wtMajor, 'VRML/X3D', 'Font style "' + FdStyle.Value + '" not supported');
  end;
end;

function TFontStyleNode.Justify: TVRMLFontJustify;
const
  SJustifyObsolete = 'Font justify "%s" should not be used in VRML >= 2.0, use "%s" instead';
var
  J: string;
begin
  if FdJustify.Items.Count = 0 then
    Result := fjBegin else
  begin
    { Some X3D models use lowercase names, like [http://instant-reality.com/]
      test models. }
    J := UpperCase(FdJustify.Items[0]);
    if (J <> FdJustify.Items[0]) and not JustifyWarningUppercaseDone then
    begin
      OnWarning(wtMajor, 'VRML/X3D', Format('Font justify value "%s" should be specified in uppercase',
        [ FdJustify.Items[0] ]));
      JustifyWarningUppercaseDone := true;
    end;

    if (J = 'BEGIN') or
       (J = 'FIRST') then
      Result := fjBegin else
    if J = 'MIDDLE' then
      Result := fjMiddle else
    if J = 'END' then
      Result := fjEnd else

    { Some X3D models use old justify names, like cic.nist.gov X3D demos. }
    if J = 'LEFT' then
    begin
      if not JustifyWarningObsoleteDone then
      begin
        OnWarning(wtMajor, 'VRML/X3D', Format(SJustifyObsolete, ['LEFT', 'BEGIN']));
        JustifyWarningObsoleteDone := true;
      end;
      Result := fjBegin;
    end else
    if J = 'CENTER' then
    begin
      if not JustifyWarningObsoleteDone then
      begin
        OnWarning(wtMajor, 'VRML/X3D', Format(SJustifyObsolete, ['CENTER', 'MIDDLE']));
        JustifyWarningObsoleteDone := true;
      end;
      Result := fjMiddle;
    end else
    if J = 'RIGHT' then
    begin
      if not JustifyWarningObsoleteDone then
      begin
        OnWarning(wtMajor, 'VRML/X3D', Format(SJustifyObsolete, ['RIGHT', 'END']));
        JustifyWarningObsoleteDone := true;
      end;
      Result := fjEnd;
    end else

    begin
      Result := fjBegin;
      if not JustifyWarningNotSupportedDone then
      begin
        OnWarning(wtMajor, 'VRML/X3D', 'Font justify "' + FdJustify.Items[0] + '" not supported');
        JustifyWarningNotSupportedDone := true;
      end;
    end;
  end;
end;

class function TFontStyleNode.DefaultSize: Single;
begin
  Result := 1;
end;

class function TFontStyleNode.DefaultSpacing: Single;
begin
  Result := 1;
end;

class function TFontStyleNode.DefaultFamily: TVRMLFontFamily;
begin
  Result := ffSerif;
end;

class function TFontStyleNode.DefaultBold: boolean;
begin
  Result := false;
end;

class function TFontStyleNode.DefaultItalic: boolean;
begin
  Result := false;
end;

class function TFontStyleNode.DefaultJustify: TVRMLFontJustify;
begin
  Result := fjBegin;
end;

class function TFontStyleNode.DefaultTTF_Font: PTrueTypeFont;
begin
  Result := TTF_Font_Results[DefaultFamily, DefaultBold, DefaultItalic];
end;

procedure TTextNode.CreateNode;
begin
  inherited;

  FFdFontStyle := TSFNode.Create(Self, 'fontStyle', [TAbstractFontStyleNode]);
   FdFontStyle.ChangesAlways := [chGeometry];
  Fields.Add(FFdFontStyle);

  FFdLength := TMFFloat.Create(Self, 'length', []);
   FdLength.ChangesAlways := [chGeometry];
  Fields.Add(FFdLength);
  { X3D specification comment: [0,Inf) }

  FFdMaxExtent := TSFFloat.Create(Self, 'maxExtent', 0.0);
   FdMaxExtent.ChangesAlways := [chGeometry];
  Fields.Add(FFdMaxExtent);
  { X3D specification comment: [0,Inf) }

  FFdString := TMFString.Create(Self, 'string', []);
   FdString.ChangesAlways := [chGeometry];
  Fields.Add(FFdString);

  FEventLineBounds := TVRMLEvent.Create(Self, 'lineBounds', TMFVec2f, false);
  Events.Add(FEventLineBounds);

  FEventOrigin := TVRMLEvent.Create(Self, 'origin', TSFVec3f, false);
  Events.Add(FEventOrigin);

  FEventTextBounds := TVRMLEvent.Create(Self, 'textBounds', TSFVec2f, false);
  Events.Add(FEventTextBounds);

  FFdSolid := TSFBool.Create(Self, 'solid', false);
   FdSolid.Exposed := false;
   FdSolid.ChangesAlways := [chGeometry];
  Fields.Add(FFdSolid);
end;

class function TTextNode.ClassNodeTypeName: string;
begin
  Result := 'Text';
end;

class function TTextNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

function TTextNode.FontStyle: TFontStyleNode;
begin
  if (FdFontStyle.Value <> nil) and
     (FdFontStyle.Value is TFontStyleNode) then
    Result := TFontStyleNode(FdFontStyle.Value) else
    Result := nil;
end;

procedure RegisterTextNodes;
begin
  NodesManager.RegisterNodeClasses([
    TFontStyleNode,
    TTextNode
  ]);
end;

{$endif read_implementation}