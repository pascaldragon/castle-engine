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
  { This is a VRML >= 2.0 grouping node.
    This will push/pop full TVRMLGraphTraverseState in Before/AfterTraverse.
    It also propagates DirectionalLights in any child to all children
    in VRML2ActiceLights.

    To keep X3D hierarchy, this descends from X3DChildNode.
    This way TAbstractX3DGroupingNode and TStaticGroupNode inherit
    also from X3DChildNode. }
  TAbstractGroupingNode = class(TAbstractChildNode)
  private
    BeforeTraversePushedState: boolean;
  protected
    { If @true (default implementation in this class), then this really
      behaves like a grouping node. If @false, this allows everything
      to "leak out" (transform changes, VRML 1.0 state changes etc.).
      Possibility useful for VRML 1.0 extension
      http://castle-engine.sourceforge.net/kambi_vrml_extensions.php#section_ext_wwwinline_separate }
    function SeparateGroup: boolean; virtual;

    procedure BeforeTraverse(StateStack: TVRMLGraphTraverseStateStack); override;
    procedure AfterTraverse(StateStack: TVRMLGraphTraverseStateStack); override;

    { Change State.Transform for children nodes.

      This should be done here, not in overriden BeforeTraverse.
      Reason: for storing ClipPlanes transformations, we need to know
      transformed matrix inside TAbstractGroupingNode.BeforeTraverse implementation.
      And we also do StateStack.Push inside TAbstractGroupingNode.BeforeTraverse
      implementation... So ApplyTransform must happen in the middle of
      TAbstractGroupingNode.BeforeTraverse call. }
    procedure ApplyTransform(State: TVRMLGraphTraverseState); virtual;
  end;

  IAbstractBoundedObject = interface(IX3DNode)
  ['{0EF47711-754B-4BA8-97E5-0A9601E9C542}']
    property FdBboxCenter: TSFVec3f { read GetFdbboxCenter }; { }
    property FdBboxSize: TSFVec3f { read GetFdbboxSize }; { }
  end;

  TAbstractX3DGroupingNode = class(TAbstractGroupingNode, IAbstractBoundedObject)
  private
    procedure EventAddChildrenReceive(
      Event: TVRMLEvent; Value: TVRMLField; const Time: TVRMLTime);
    procedure EventRemoveChildrenReceive(
      Event: TVRMLEvent; Value: TVRMLField; const Time: TVRMLTime);
  public
    procedure CreateNode; override;

    { Event: MFNode, in } { }
    private FEventAddChildren: TVRMLEvent;
    public property EventAddChildren: TVRMLEvent read FEventAddChildren;

    { Event: MFNode, in } { }
    private FEventRemoveChildren: TVRMLEvent;
    public property EventRemoveChildren: TVRMLEvent read FEventRemoveChildren;

    private FFdChildren: TMFNode;
    public property FdChildren: TMFNode read FFdChildren;

    private FFdBboxCenter: TSFVec3f;
    public property FdBboxCenter: TSFVec3f read FFdBboxCenter;

    private FFdBboxSize: TSFVec3f;
    public property FdBboxSize: TSFVec3f read FFdBboxSize;

    private FFdRender: TSFBool;
    { This is an extension from Avalon, see
      [http://instant-reality.com/documentation/nodetype/ChildGroup/].
      We simply ignore it for now. }
    public property FdRender: TSFBool read FFdRender;
  end;

  TGroupNode = class(TAbstractX3DGroupingNode)
  protected
    procedure DirectEnumerateActive(
      Func: TEnumerateChildrenFunction); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;

    class function ForVRMLVersion(const Version: TVRMLVersion): boolean;
      override;

    class function URNMatching(const URN: string): boolean; override;
  end;
  TGroupNode_2 = TGroupNode;

  TStringLongIntMap = specialize TFPGMap<string, LongInt>;
  TStringStringMap = class(specialize TFPGMap<string, string>)
  public
    { Set given key value, trying to preserve previous key value too.
      This is useful for safely setting X3D META values.

      Compared to normal PutKeyData, this behaves smarter if given Name
      is already set. If it's set with the same Content, we do nothing.
      If the Content is different, we move previous content to a
      @code(Name + '-previous') key.
      This way previous content value is preserved once (but not more,
      to not grow the X3D file indefinitely). }
    procedure PutPreserve(const Name, Content: string);
  end;

  { A top-level VRML node. This is what you get by parsing 3D model from file.

    It is declared as a descendant of VRML >= 2.0 Group node,
    but it's used with all VRML versions (including VRML 1.0 and Inventor).
    This makes things simple (previously we had two separate TX3DRootNode_1
    and TX3DRootNode, which was complicating stuff).
    Children (for all VRML/X3D versions) are inside FdChildren field.

    This way VRML/X3D files may have many nodes at the top level
    (which is a standard feature of VRML >= 2.0, but we also allow it for
    VRML 1.0 as a commonly used extension). It may also have prototypes,
    routes etc. at the root level.

    This also allows us to record in one place some information that
    is returned by the parser. Like parsed VRML/X3D version, X3D profile,
    some namespace information (exported names and such). }
  TX3DRootNode = class(TGroupNode)
  private
    FHasForceVersion: boolean;
    FSaveAsRootNode: boolean;
    FProfile: string;
    FComponents: TStringLongIntMap;
    FMeta: TStringStringMap;
    FPrototypeNames: TVRMLPrototypeNames;
    FExportedNames: TX3DNodeNames;
    procedure TopLevelCheckChild(const Child: TX3DNode);
  protected
    function DeepCopyCore(CopyState: TX3DNodeDeepCopyState): TX3DNode; override;

    { Root node never saves/restores the traversing state.
      This means that all state changes "leak out" from a root node.
      This is a good thing: if root node isn't used as a final TVRMLScene.RootNode,
      then it is placed inside Inline node, which will do save/restore anyway.
      And this way VRML 1.0 WWWInline with separate=FALSE may also work too. }
    function SeparateGroup: boolean; override;
  public
    ForceVersion: TVRMLVersion;

    procedure CreateNode; override;
    destructor Destroy; override;

    { Should SaveToStream take care to nicely save us, treating as
      a root node of whole VRML file or prototype.

      If this node isn't a root node, you should set this to @false
      (or saving this node may generate dumb contents).
      Although, generally, you should avoid using the TX3DRootNode
      class at all for non-root nodes. }
    property SaveAsRootNode: boolean read FSaveAsRootNode write FSaveAsRootNode
      default true;

    { Save contents to the stream.

      If SaveAsRootNode then this is saved in a special way,
      such that only the contents are written, without surrounding
      braces (for classic encoding) or XML element (for xml encoding).
      This way, when saving, we hide the fact that everything was wrapped
      in an artificial TX3DRootNode. }
    procedure SaveToStream(Writer: TX3DWriter); override;

    { Set HasForceVersion to @true to force saving this model with
      given ForceVersion. }
    property HasForceVersion: boolean
      read FHasForceVersion write FHasForceVersion default false;

    { Profile required for this X3D file. See X3D spec about profiles.
      Every X3D file must always define a profile.
      (However, we use this class also for VRML 1.0 and 2.0 root nodes,
      where profile is empty.)
      Relevant only if this node is the root of X3D file. }
    property Profile: string read FProfile write FProfile;

    { Components and levels required for this X3D file.
      Relevant if this node is the root of X3D file. }
    property Components: TStringLongIntMap read FComponents;

    { Meta keys and values of this X3D file.
      Relevant if this node is the root of X3D file. }
    property Meta: TStringStringMap read FMeta;

    { Set properties to force saving this node graph as X3D.
      If we're already configured to save as X3D (major version >= 3)
      then do nothing. Otherwise, sets major/minor versions
      (by default: for X3D 3.2; make sure AMajor is >= 3)
      and sets X3D profile (default: Interchange). }
    procedure ForceSaveAsX3D(const AMajor: Integer = 3;
      const AMinor: Integer = 2;
      const AProfile: string = 'Interchange');

    { Global prototype namespace at the end of parsing the file.
      Useful for things like EXTERNPROTO implementation. }
    property PrototypeNames: TVRMLPrototypeNames read FPrototypeNames;

    { Node names exported from the file by EXPORT keyword.
      Useful for importing them later, to handle IMPORT/EXPORT X3D mechanism. }
    property ExportedNames: TX3DNodeNames read FExportedNames;
  end;

  TStaticGroupNode = class(TAbstractGroupingNode, IAbstractBoundedObject)
  protected
    procedure DirectEnumerateActive(
      Func: TEnumerateChildrenFunction); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;

    private FFdChildren: TMFNode;
    public property FdChildren: TMFNode read FFdChildren;

    private FFdBboxCenter: TSFVec3f;
    public property FdBboxCenter: TSFVec3f read FFdBboxCenter;

    private FFdBboxSize: TSFVec3f;
    public property FdBboxSize: TSFVec3f read FFdBboxSize;
  end;

  { Switch node.

    Compatibility node: this class is used for both VRML 97 and X3D.
    Although field name changed for X3D (in VRML 97, "children"
    was called "choice"), this is handled seamlessly using our
    TVRMLFieldOrEvent.AddAlternativeField mechanism. In other words,
    programmer simply works with FdChildren field, and when reading/writing
    VRML file we take care to actually use either "children" or "choice" name,
    depending on used VRML version. }
  TSwitchNode = class(TAbstractX3DGroupingNode)
  protected
    procedure DirectEnumerateActive(
      Func: TEnumerateChildrenFunction); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;

    private FFdWhichChoice: TSFInt32;
    public property FdWhichChoice: TSFInt32 read FFdWhichChoice;

    class function ForVRMLVersion(const Version: TVRMLVersion): boolean;
      override;

    class function URNMatching(const URN: string): boolean; override;

    function TransformationChange: TNodeTransformationChange; override;
  end;
  TSwitchNode_2 = TSwitchNode;

  { Interface for all VRML 2.0 / X3D transformation nodes.
    These nodes transform all their children (the ones enumerated
    by traversing, so all active children).

    The transformation change may be signalled by TVRMLField.Changes
    including chTransform flag. For some special nodes, like Billboard,
    the transformation changes are automatically managed. }
  ITransformNode = interface(IX3DNode)
  ['{6991EC7D-0821-4393-B5B3-08DD967CFE05}']
  end;

  TTransformNode = class(TAbstractX3DGroupingNode, ITransformNode)
  protected
    procedure DirectEnumerateActive(
      Func: TEnumerateChildrenFunction); override;
    procedure ApplyTransform(State: TVRMLGraphTraverseState); override;
  public
    procedure CreateNode; override;
    class function ClassNodeTypeName: string; override;
    class function URNMatching(const URN: string): boolean; override;
    class function ForVRMLVersion(const Version: TVRMLVersion): boolean;
      override;
    function TransformationChange: TNodeTransformationChange; override;

    private FFdCenter: TSFVec3f;
    public property FdCenter: TSFVec3f read FFdCenter;

    private FFdRotation: TSFRotation;
    public property FdRotation: TSFRotation read FFdRotation;

    private FFdScale: TSFVec3f;
    public property FdScale: TSFVec3f read FFdScale;

    private FFdScaleOrientation: TSFRotation;
    public property FdScaleOrientation: TSFRotation read FFdScaleOrientation;

    private FFdTranslation: TSFVec3f;
    public property FdTranslation: TSFVec3f read FFdTranslation;
  end;
  TTransformNode_2 = TTransformNode;

{$endif read_interface}

{$ifdef read_implementation}
type
  TGeneralGroupingEnumerator = class
    State: TVRMLGraphTraverseState;
    WasPointingDeviceSensor: boolean;
    procedure EnumerateChildrenFunction(Node, Child: TX3DNode);
  end;

  procedure TGeneralGroupingEnumerator.EnumerateChildrenFunction(
    Node, Child: TX3DNode);
  var
    CP: PClipPlane;
  begin
    if (Child is TAbstractLightNode) and
       (TAbstractLightNode(Child).Scope = lsLocal) then
      { We do CreateLightInstance with State from TAbstractGroupingNode node,
        while precisely we should rather use State from TAbstractLightNode
        traverse. But, fortunately, State of TAbstractLightNode doesn't
        change inside (TAbstractLightNode doesn't do anything inside
        BeforeTraverse), so this is the same thing. }
      { TODO: we ignore here light radius, which may be relevant
        for TAbstractPositionalLightNode with HasRadius = true.
        E.g. PointLight with global = false should be limited by *both*
        scope (in graph hierarchy) and radius, as far as I understand. }
      State.AddLight(
        TAbstractLightNode(Child).CreateLightInstance(State)) else
    if Child is TAbstractPointingDeviceSensorNode then
    begin
      { Testing for WasPointingDeviceSensor guarantees that multiple
        pointing devices within the same grouping node will all work. }
      if not WasPointingDeviceSensor then
      begin
        State.PointingDeviceSensors.Clear;
        State.PointingDeviceSensors.Transform := State.Transform;
        State.PointingDeviceSensors.InvertedTransform := State.InvertedTransform;
        WasPointingDeviceSensor := true;
      end;

      State.PointingDeviceSensors.Add(Child);
    end else
    if Child is TClipPlaneNode then
    begin
      CP := State.AddClipPlane;
      CP^.Node := TClipPlaneNode(Child);
      CP^.Transform := State.Transform;
    end else
    if Child is TLocalFogNode then
      State.LocalFog := TLocalFogNode(Child) else
    if Child is TEffectNode then
    begin
      if State.Effects = nil then
        State.Effects := TX3DNodeList.Create(false);
      State.Effects.Add(Child);
    end;
  end;

function TAbstractGroupingNode.SeparateGroup: boolean;
begin
  Result := true;
end;

procedure TAbstractGroupingNode.BeforeTraverse(
  StateStack: TVRMLGraphTraverseStateStack);
var
  Enumerator: TGeneralGroupingEnumerator;
begin
  inherited;

  BeforeTraversePushedState := SeparateGroup;
  if BeforeTraversePushedState then
    StateStack.Push;

  ApplyTransform(StateStack.Top);

  (*We append lights with lsLocal scope (global=FALSE and VRML/X3D >= 2.0)
    to the current State.

    It may seem wasteful to enumerate children twice
    (first we enumerate here, then actual Traverse enumerates,
    this time recursively, for the second time). But it turned out to
    be the fastest and simplest method of propagating lsLocal lights
    correctly...

    Other (rejected) approach was to add ParentGroup to TLightInstance
    and do it as post-processing step, i.e. in TVRMLScene in
    AddGlobalLights take this ParentGroup and add light everywhere.
    But that was 1. even slower, since it must traverse ParentGroup once again
    for each directional light, and for each shape within this ParentGroup,
    it must find all it's occurrences inside Shapes, and add light there
    2. it fails ugly in case of DEF / USE of shapes.

    See demo_models/lights_materials/directional_light_scope.wrl test. Imagine
    (simplified VRML below) :

      Group {
        DEF S Shape { .... some sphere .... }
        Group {
          USE S
          DirectionalLight { }
          USE S
        }
        USE S
      }

    What would happen here? ParentGroup.Enumerate would find S *two* times.
    For each of these occurrences, it would find *four* shapes with
    matching node value. So 1. all four spheres would be lighted (incorrect,
    only two spheres in the middle should be lighted) 2. all of them would
    be lighted two times by the same light... Fix for 1st problem would
    require us to record some list of parents within State (which would
    awfully slow down Traverse work, that already is too crowded). Fix for 2nd
    problem would require some intelligent avoiding of duplicates
    (set light only for first node, that is both matching and has the light
    not set yet).

    So fixing the above solution would be quite more convoluted and slower
    than simple, correct solution below. *)

  { We also use it to collect State.PointingDeviceSensors,
    as it turns our that the same approach is possible, for the same reasons.
    And the same goes for State.ClipPlanes.
    Moreover, we do it all in a single DirectEnumerateActive call,
    so this doesn't slow down the code.
  }

  { It's a little unclean to test "Self is ...", since usually virtual
    methods should replace this. But in this case, this is (for now)
    the simplest solution. Anchor node serves as a pointing device
    for all it's children, so it just places itself in
    State.PointingDeviceSensors.

    Note that if Anchor node itself has other pointing device sensors
    inside, they will override the Anchor --- as far as I know, this
    is what VRML specificaion expects? (if not, we should just set
    Enumerator.WasPointingDeviceSensor := true when inside Anchor,
    so the solution will be trivial). }
  if Self is TAnchorNode then
  begin
    StateStack.Top.PointingDeviceSensors.Clear;
    StateStack.Top.PointingDeviceSensors.Transform := StateStack.Top.Transform;
    StateStack.Top.PointingDeviceSensors.InvertedTransform := StateStack.Top.InvertedTransform;
    StateStack.Top.PointingDeviceSensors.Add(Self);
  end;

  Enumerator := TGeneralGroupingEnumerator.Create;
  try
    Enumerator.State := StateStack.Top;
    Enumerator.WasPointingDeviceSensor := false;
    DirectEnumerateActive(@Enumerator.EnumerateChildrenFunction);
  finally FreeAndNil(Enumerator) end;
end;

procedure TAbstractGroupingNode.ApplyTransform(State: TVRMLGraphTraverseState);
begin
end;

procedure TAbstractGroupingNode.AfterTraverse(
  StateStack: TVRMLGraphTraverseStateStack);
begin
  if BeforeTraversePushedState then
    StateStack.Pop;

  inherited;
end;

procedure TAbstractX3DGroupingNode.CreateNode;
begin
  inherited;

  FEventAddChildren := TVRMLEvent.Create(Self, 'addChildren', TMFNode, true);
  Events.Add(FEventAddChildren);
  EventaddChildren.OnReceive.Add(@EventAddChildrenReceive);

  FEventRemoveChildren := TVRMLEvent.Create(Self, 'removeChildren', TMFNode, true);
  Events.Add(FEventRemoveChildren);
  EventRemoveChildren.OnReceive.Add(@EventRemoveChildrenReceive);

  FFdChildren := TMFNode.Create(Self, 'children', IAbstractChildNode);
   FdChildren.ChangesAlways := [chEverything];
  Fields.Add(FFdChildren);

  FFdBboxCenter := TSFVec3f.Create(Self, 'bboxCenter', ZeroVector3Single);
  FFdBboxCenter.Exposed := false;
  Fields.Add(FFdBboxCenter);
  { X3D specification comment: (-Inf,Inf) }

  FFdBboxSize := TSFVec3f.Create(Self, 'bboxSize', Vector3Single(-1, -1, -1));
  FFdBboxSize.Exposed := false;
  Fields.Add(FFdBboxSize);
  { X3D specification comment: [0,Inf) or -1 -1 -1 }

  FFdRender := TSFBool.Create(Self, 'render', true);
  Fields.Add(FFdRender);

  DefaultContainerField := 'children';
end;

procedure TAbstractX3DGroupingNode.EventAddChildrenReceive(
  Event: TVRMLEvent; Value: TVRMLField; const Time: TVRMLTime);
var
  MF: TMFNode;
  I: Integer;
begin
  MF := Value as TMFNode;
  for I := 0 to MF.Count - 1 do
    if FdChildren.Items.IndexOf(MF[I]) = -1 then
      FdChildren.Add(MF[I]);

  { X3D spec says explicitly
    "A children field is not allowed to contain nodes that are
    ancestors of the grouping node"
    so correct X3D file cannot produce cycles. Cycles would be very bad,
    since
    - they will make memory leaks (reference count will be > 0 for grouping node)
    - and, worse, TX3DNode.Traverse will hang infinitely

    TODO: So invalid VRML file can produce mem leaks and hang the engine...

    Later: actually, cycles have to be allowed for some cases, see TODOs
    elsewhere. So we should just enable cycles. And here, we should not worry
    about them. }
end;

procedure TAbstractX3DGroupingNode.EventRemoveChildrenReceive(
  Event: TVRMLEvent; Value: TVRMLField; const Time: TVRMLTime);
var
  MF: TMFNode;
  I: Integer;
begin
  MF := Value as TMFNode;

  { Note that X3D specification directly says
    "A children field is not allowed to directly contain
    multiple instances of the same node."
    Thus the question how we should deal with multiple items to remove
    (use Delete or DeleteAll in implementation below?) doesn't exist.

    Note that I didn't find such notice in VRML 97 specification.
    This seems to be undefined case in VRML 97? }

  for I := 0 to MF.Count - 1 do
    FdChildren.Items.Remove(MF[I]);
end;

procedure TGroupNode.CreateNode;
begin
  inherited;

  DefaultContainerField := 'children';
end;

class function TGroupNode.ClassNodeTypeName: string;
begin
  Result := 'Group';
end;

class function TGroupNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TGroupNode.ForVRMLVersion(const Version: TVRMLVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TGroupNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction);
var
  I: Integer;
begin
  inherited;
  for I := 0 to FdChildren.Count - 1 do
    Func(Self, FdChildren[I]);
end;

procedure TStringStringMap.PutPreserve(const Name, Content: string);
var
  PreviousContent: string;
  I: Integer;
begin
  I := IndexOf(Name);
  if I <> -1 then
  begin
    PreviousContent := Data[I];
    if PreviousContent <> Content then
    begin
      { move current content to -previous name }
      KeyData[Name + '-previous'] := PreviousContent;
      { set new content }
      Data[I] := Content;
    end;
  end else
    KeyData[Name] := Content;
end;

procedure TX3DRootNode.CreateNode;
begin
  inherited;
  FComponents := TStringLongIntMap.Create;
  FMeta := TStringStringMap.Create;
  FSaveAsRootNode := true;
end;

destructor TX3DRootNode.Destroy;
begin
  FreeAndNil(FComponents);
  FreeAndNil(FMeta);
  FreeAndNil(FPrototypeNames);
  FreeAndNil(FExportedNames);
  inherited;
end;

procedure TX3DRootNode.SaveToStream(Writer: TX3DWriter);
var
  I: integer;
  FileItems: TVRMLFileItemList;
begin
  if SaveAsRootNode then
  begin
    { Special things for root node saving:
      - NodeName is ignored (should be '').
      - Fields are ignored, except "children", that holds children
        (for any VRML version, including VRML 1).
        It's output in a special way, without writing "children [...]" around.
        Other fields are ignored (should never have any meaning). }

    FileItems := TVRMLFileItemList.Create(false);
    try
      for I := 0 to Prototypes.Count - 1 do
        FileItems.Add(Prototypes[I]);

      Assert(VRML1ChildrenCount = 0);

      for i := 0 to FdChildren.Count - 1 do
        FileItems.Add(FdChildren[I]);

      for I := 0 to Routes.Count - 1 do
        FileItems.Add(Routes[I]);

      for I := 0 to ImportsList.Count - 1 do
        FileItems.Add(ImportsList[I]);

      for I := 0 to ExportsList.Count - 1 do
        FileItems.Add(ExportsList[I]);

      FileItems.SaveToStream(Writer);
    finally FreeAndNil(FileItems) end;
  end else
    inherited;
end;

function TX3DRootNode.DeepCopyCore(CopyState: TX3DNodeDeepCopyState): TX3DNode;
var
  Res: TX3DRootNode;
begin
  Result := inherited;

  Res := Result as TX3DRootNode;

  Res.HasForceVersion := HasForceVersion;
  Res.ForceVersion := ForceVersion;

  Res.Profile := Profile;
  Res.Components.Assign(Components);
  Res.Meta.Assign(Meta);
end;

procedure TX3DRootNode.ForceSaveAsX3D(const AMajor: Integer = 3;
  const AMinor: Integer = 2;
  const AProfile: string = 'Interchange');
begin
  if (not HasForceVersion) or
     (ForceVersion.Major < 3) then
  begin
    HasForceVersion := true;
    ForceVersion.Major := AMajor;
    ForceVersion.Minor := AMinor;
    Profile := AProfile;
  end;
end;

function TX3DRootNode.SeparateGroup: boolean;
begin
  Result := false;
end;

procedure TX3DRootNode.TopLevelCheckChild(const Child: TX3DNode);
begin
  { We could just do here FdChildren.WarningIfChildNotAllowed(Child),
    but the warning message of it is a little unfriendly: it uncovers
    to user that we have a special TX3DRootNode that descends from Group node.
    So instead, do it by hand.

    Also, add LayerSet (not allowed in Group.FdChildren).
    This is special (see http://www.web3d.org/x3d/specifications/ISO-IEC-19775-1.2-X3D-AbstractSpecification/Part01/concepts.html#Rootnodes),
    "4.3.2 Root nodes" from X3D 3.2 spec.
  }

  if not (Supports(Child, IAbstractChildNode) or (Child is TLayerSetNode)) then
    OnWarning(wtMajor, 'VRML/X3D', Format('Node "%s" is not allowed at the top level of the VRML/X3D file',
      [Child.NodeTypeName]));
end;

procedure TStaticGroupNode.CreateNode;
begin
  inherited;

  FFdChildren := TMFNode.Create(Self, 'children', IAbstractChildNode);
   FdChildren.Exposed := false;
   FdChildren.ChangesAlways := [chEverything];
  Fields.Add(FFdChildren);

  FFdBboxCenter := TSFVec3f.Create(Self, 'bboxCenter', Vector3Single(0, 0, 0));
  FFdBboxCenter.Exposed := false;
  Fields.Add(FFdBboxCenter);
  { X3D specification comment: (-Inf,Inf) }

  FFdBboxSize := TSFVec3f.Create(Self, 'bboxSize', Vector3Single(-1, -1, -1));
  FFdBboxSize.Exposed := false;
  Fields.Add(FFdBboxSize);
  { X3D specification comment: [0,Inf) or -1 -1 -1 }

  DefaultContainerField := 'children';
end;

class function TStaticGroupNode.ClassNodeTypeName: string;
begin
  Result := 'StaticGroup';
end;

class function TStaticGroupNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

procedure TStaticGroupNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction);
var
  I: Integer;
begin
  for I := 0 to FdChildren.Count - 1 do
    Func(Self, FdChildren[I]);
end;

procedure TSwitchNode.CreateNode;
begin
  inherited;

  Fdchildren.AddAlternativeName('choice', 2);

  FFdWhichChoice := TSFInt32.Create(Self, 'whichChoice', -1);
   FdWhichChoice.ChangesAlways := [chSwitch2];
  Fields.Add(FFdWhichChoice);
  { X3D specification comment: [-1,Inf) }

  DefaultContainerField := 'children';
end;

class function TSwitchNode.ClassNodeTypeName: string;
begin
  Result := 'Switch';
end;

class function TSwitchNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TSwitchNode.ForVRMLVersion(const Version: TVRMLVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TSwitchNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction);
begin
  if Between(FdWhichChoice.Value, 0, FdChildren.Count - 1) then
    Func(Self, FdChildren[FdWhichChoice.Value]);
end;

function TSwitchNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcSwitch;
end;

procedure TTransformNode.CreateNode;
begin
  inherited;

  FFdCenter := TSFVec3f.Create(Self, 'center', Vector3Single(0, 0, 0));
   FdCenter.ChangesAlways := [chTransform];
  Fields.Add(FFdCenter);
  { X3D specification comment: (-Inf,Inf) }

  FFdRotation := TSFRotation.Create(Self, 'rotation', Vector3Single(0, 0, 1), 0);
   FdRotation.ChangesAlways := [chTransform];
  Fields.Add(FFdRotation);
  { X3D specification comment: [-1,1] or (-Inf,Inf) }

  FFdScale := TSFVec3f.Create(Self, 'scale', Vector3Single(1, 1, 1));
   FdScale.ChangesAlways := [chTransform];
  Fields.Add(FFdScale);
  { X3D specification comment: (-Inf, Inf) }

  FFdScaleOrientation := TSFRotation.Create(Self, 'scaleOrientation', Vector3Single(0, 0, 1), 0);
   FdScaleOrientation.ChangesAlways := [chTransform];
  Fields.Add(FFdScaleOrientation);
  { X3D specification comment: [-1,1] or (-Inf,Inf) }

  FFdTranslation := TSFVec3f.Create(Self, 'translation', Vector3Single(0, 0, 0));
   FdTranslation.ChangesAlways := [chTransform];
  Fields.Add(FFdTranslation);
  { X3D specification comment: (-Inf,Inf) }

  DefaultContainerField := 'children';
end;

class function TTransformNode.ClassNodeTypeName: string;
begin
  Result := 'Transform';
end;

class function TTransformNode.URNMatching(const URN: string): boolean;
begin
  Result := (inherited URNMatching(URN)) or
    (URN = URNVRML97Nodes + ClassNodeTypeName) or
    (URN = URNX3DNodes + ClassNodeTypeName);
end;

class function TTransformNode.ForVRMLVersion(const Version: TVRMLVersion): boolean;
begin
  Result := Version.Major >= 2;
end;

procedure TTransformNode.DirectEnumerateActive(Func: TEnumerateChildrenFunction);
var
  I: Integer;
begin
  for I := 0 to FdChildren.Count - 1 do
    Func(Self, FdChildren[I]);
end;

function TTransformNode.TransformationChange: TNodeTransformationChange;
begin
  Result := ntcTransform;
end;

{ Apply transformation to a TVRMLGraphTraverseState.
  Useful for various classes that behave like Transform node. }
procedure TransformMatrices(var Transform, InvertedTransform: TMatrix4Single;
  var TransformScale: Single;
  center: TVector3Single;
  rotation: TVector4Single;
  scale: TVector3Single;
  scaleOrientation: TVector4Single;
  translation: TVector3Single);
var
  M, IM: TMatrix4Single;
  MRotateScaleOrient, IMRotateScaleOrient: TMatrix4Single;
begin
  { Inherited TAbstractGroupingNode already saved Transform and such.

    To make InvertedTransform, we multiply inverted matrices in inverted order
    below. }

  MultMatricesTranslation(Transform, InvertedTransform,
    VectorAdd(Translation, Center));

  { We avoid using RotationMatricesRad when angle = 0, since this
    is often the case, and it makes TransformState much faster
    (which is important --- TransformState is important for traversing state). }
  if Rotation[3] <> 0 then
  begin
    { Note that even rotation Axis = zero is OK, both M and IM will be
      identity in this case. }
    RotationMatricesRad(Rotation, M, IM);
    Transform := MatrixMult(Transform, M);
    InvertedTransform := MatrixMult(IM, InvertedTransform);
  end;

  if (Scale[0] <> 1) or
     (Scale[1] <> 1) or
     (Scale[2] <> 1) then
  begin
    if ScaleOrientation[3] <> 0 then
    begin
      RotationMatricesRad(ScaleOrientation, MRotateScaleOrient, IMRotateScaleOrient);
      Transform := MatrixMult(Transform, MRotateScaleOrient);
      InvertedTransform := MatrixMult(IMRotateScaleOrient, InvertedTransform);
    end;

    { For scaling, we explicitly request that if ScalingFactor contains
      zero, IM will be forced to be identity (the 2nd param to ScalingMatrices
      is "true"). That's because VRML allows
      scaling factor to have 0 components (we need InvertedTransform only
      for special tricks). }

    ScalingMatrices(Scale, true, M, IM);
    Transform := MatrixMult(Transform, M);
    InvertedTransform := MatrixMult(IM, InvertedTransform);

    TransformScale *= (Scale[0] + Scale[1] + Scale[2]) / 3;

    if ScaleOrientation[3] <> 0 then
    begin
      { That's right, we reuse MRotateScaleOrient and IMRotateScaleOrient
        matrices below. Since we want to reverse them now, so normal
        Transform is multiplied by IM and InvertedTransform is multiplied by M. }
      Transform := MatrixMult(Transform, IMRotateScaleOrient);
      InvertedTransform := MatrixMult(MRotateScaleOrient, InvertedTransform);
    end;
  end;

  MultMatricesTranslation(Transform, InvertedTransform, VectorNegate(Center));
end;

procedure TransformState(State: TVRMLGraphTraverseState;
  center: TVector3Single;
  rotation: TVector4Single;
  scale: TVector3Single;
  scaleOrientation: TVector4Single;
  translation: TVector3Single);
begin
  TransformMatrices(
    State.Transform, State.InvertedTransform, State.TransformScale,
    center, rotation, scale, scaleOrientation, translation);
end;

procedure TTransformNode.ApplyTransform(State: TVRMLGraphTraverseState);
begin
  inherited;
  TransformState(State, Fdcenter.Value, Fdrotation.Value,
    Fdscale.Value, FdscaleOrientation.Value, Fdtranslation.Value);
end;

procedure RegisterGroupingNodes;
begin
  NodesManager.RegisterNodeClasses([
    TGroupNode,
    TStaticGroupNode,
    TSwitchNode,
    TTransformNode
  ]);
end;
{$endif read_implementation}