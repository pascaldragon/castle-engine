{
  Copyright 2006-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ A precalculated 3D animation rendered in OpenGL (T3DPrecalculatedAnimation). }
unit PrecalculatedAnimation;

interface

uses SysUtils, Classes, X3DNodes, GLRenderer, CastleSceneCore, CastleScene,
  CastleUtils, Boxes3D, CastleClassUtils, PrecalculatedAnimationCore, KeysMouse,
  CastleTimeUtils, Frustum, VectorMath, Base3D, Triangle,
  FGL {$ifdef VER2_2}, FGLObjectList22 {$endif};

type
  TGetRootNodeWithTime = procedure (const Index: Cardinal;
    out RootNode: TX3DRootNode; out Time: Single) of object;

  { A "precalculated" animation of VRML model done by
    interpolating between any number of model states.

    After constructing an object of this class, you must actually
    load it's animation by calling Load or LoadFromFile or LoadFromVRMLEvents
    etc.

    When loading you must provide one or more VRML models with
    their associated times. Animation will show a transition from the first
    model to the last. If models are "structurally equal" then the transition
    between two successive models will be smooth, otherwise a sudden change
    will be shown. "Structurally equal" means
    the same nodes hierarchy, the same names of nodes,
    the same values of all fields (with the exception of fields
    that are floating-point based and so can be interpolated, for example
    SFFloat, SFVec3f and equivalent MFXxx fields).
    For multi-valued fields (MFXxx) that can be interpolated: note that values
    of items may differ, but still the counts of items must be equal.

    This creates a list of @link(Scenes) such that
    @unorderedList(
      @itemSpacing Compact
      @item the first scene on the list is exactly the 1st object
      @item the last scene on the list is exactly the last object
      @item(intermediate scenes are accordingly interpolated between
        the two surrounding "predefined" by you scenes)
    )

    For example, first object may be a small sphere with blue color, the other
    object may be a larger sphere with white color, and the simplest
    times are 0.0 for the 1st scene and 1.0 for the 2nd scene.
    The animation will show the blue sphere growing larger
    and fading into the white
    color. Of course, any kind of models is allowed --- e.g. it can
    be a walking man at various stages, so in effect you get an animation
    of walking man.

    A special case when you pass only one scene to this class is allowed
    (it may be handy in some situations). This will obviously produce
    just a still result, i.e. resulting T3DPrecalculatedAnimation will be just
    a wrapper around single T3DScene instance.

    For more information see the "VRML engine documentation",
    [http://castle-engine.sourceforge.net/vrml_engine_doc/].
    Specifically the section
    "Non-interactive precalculated animation: T3DPrecalculatedAnimation",
    [http://castle-engine.sourceforge.net/vrml_engine_doc/output/xsl/html/section.animation_precalculated.html]. }
  T3DPrecalculatedAnimation = class(T3DPrecalculatedAnimationCore)
  private
    FScenes: T3DSceneList;
    function GetScenes(I: Integer): T3DScene;
  private
    Renderer: TGLRenderer;
    FCache: TGLRendererContextCache;
    OwnsCache: boolean;
    FTimeBegin, FTimeEnd: Single;
    FTimeLoop: boolean;
    FTimeBackwards: boolean;
    FOwnsFirstRootNode: boolean;
    FLoaded: boolean;
    FTimePlaying: boolean;
    FTimePlayingSpeed: Single;
    FTimeAtLoad: TFloatTime;
    FTime: TFloatTime;
    FShadowMaps: boolean;
    FShadowMapsDefaultSize: Cardinal;
    FTryFirstSceneDynamic: boolean;

    ValidBoundingBox: boolean;
    FBoundingBox: TBox3D;
    FCollisionUseLastScene: boolean;
    FInitialViewpointIndex: Cardinal;
    FInitialViewpointName: string;

    procedure SetShadowMaps(const Value: boolean);
    procedure SetShadowMapsDefaultSize(const Value: Cardinal);

    function InfoBoundingBox: string;
  private
    { Helpers for Load implementation. }
    Load_RootNodes: TX3DNodeList;
    Load_Times: TSingleList;
    procedure Load_GetRootNodeWithTime(const Index: Cardinal;
      out RootNode: TX3DRootNode; out Time: Single);
  private
    { Helpers for LoadFromVRMLEvents implementation. }
    LoadFromVRMLEvents_TimeBegin: Single;
    LoadFromVRMLEvents_Scene: T3DSceneCore;
    LoadFromVRMLEvents_ScenesPerTime: Cardinal;
    procedure LoadFromVRMLEvents_GetRootNodeWithTime(const Index: Cardinal;
      out RootNode: TX3DRootNode; out Time: Single);
    procedure LoadFromVRMLEvents_GetRootNodeWithTime_Progress(
      const Index: Cardinal;
      out RootNode: TX3DRootNode; out Time: Single);

    procedure SetOwnsFirstRootNode(const Value: boolean);
  protected
    { Internal version of @link(Load) routines, feasible to load
      from both ready RootNodes array and to automatically generate RootNodes
      on the fly.

      GetRootNodeWithTime will be called with indexes from 0 to RootNodesCount - 1.
      It's guaranteed that it will be called in this order (from 0 upwards to
      RootNodesCount - 1) and will be called exactly once for each index.
      So it's safe to e.g. create RootNode with some costly operation there.

      Note that RootNode passed to GetRootNodeWithTime becomes owned by
      this class. Well, you can get control over only the first one,
      by AOwnsFirstRootNode, but you cannot free it anyway while this is loaded.

      See @link(Load) for more information, including the meaning of
      EqualityEpsilon. }
    procedure LoadCore(
      GetRootNodeWithTime: TGetRootNodeWithTime;
      RootNodesCount: Cardinal;
      AOwnsFirstRootNode: boolean;
      ScenesPerTime: Cardinal;
      const EqualityEpsilon: Single);
  public
    constructor Create(AOwner: TComponent); override;

    { Constructor that allows you to pass your own Cache instance. }
    constructor CreateCustomCache(AOwner: TComponent;
      ACache: TGLRendererContextCache);

    destructor Destroy; override;

    { Load the animation scenes.
      Must be called (this or some other loading routine like LoadFromFile)
      before you do almost anything with this object.
      @link(Loaded) changes to @true after calling this.

      @param(RootNodes
        Models describing the "predefined" frames of animation.
        They must descend from TX3DRootNode.

        For all nodes except the first: They are @italic(always)
        owned by this class --- that's needed,
        because actually we may do some operations on these models when
        building animation (including even freeing some RootNodes,
        if we will find that they are equivalent to some other RootNodes).
        They all must point to different objects.

        You must supply at least one item here (you cannot make an animation
        from 0 items).)

      @param(Times Array specifying the point of time for each "predefined"
        frame. Length of this array must equal to length of RootNodes array.)

      @param(ScenesPerTime
        This says how many scenes will be used for period of time equal to 1.0.
        This will determine Scenes.Count.
        RootNodes[0] always takes Scenes[0] and RootNodes[High(RootNodes)]
        always takes Scenes[Scenes.High].

        Note that if we will find that some nodes along the way are
        exactly equal, we may drop scenes count between --- because if they are
        both equal, we can simply render the same scene for some period
        of time. This is an optimization, and you shouldn't notice it at all,
        since rendeting will be the same (but less memory-consuming).

        Special value ScenesPerTime = 0 means that you want to have only the
        RootNodes you explicitly passed in the scene, not more.
        No more intermediate scenes will ever be created.
        This creates a trivial animation that suddenly jumps from
        one RootNode to the next at specified times. It may be useful if you
        already have generated a lot of RootNodes, densely distributed
        over time, and you don't need T3DPrecalculatedAnimation to insert any more
        scenes.)

      @param(EqualityEpsilon
        This will be used for comparing fields, to decide if two fields
        (and, consequently, nodes) are equal. It will be simply
        passed to TVRMLField.Equals.

        You can pass here 0 to use exact comparison, but it's
        advised to use here something > 0. Otherwise we could waste
        display list memory (and loading time) for many frames of the
        same node that are in fact equal.)
    }
    procedure Load(
      RootNodes: TX3DNodeList;
      AOwnsFirstRootNode: boolean;
      ATimes: TSingleList;
      ScenesPerTime: Cardinal;
      const EqualityEpsilon: Single);

    { Load precalculated animation by playing a single VRML file with
      events (interpolators, TimeSensor and such working).
      Conceptually, this "records" interactive animation stored in VRML file
      into T3DPrecalculatedAnimation precalculated animation.

      ATimeBegin, ATimeEnd tell what time slice should be recorded.
      They will also set @link(TimeBegin) and @link(TimeEnd) properties.

      @param(ScenesPerTime
        tells with what density should the animation be recorded.
        See @link(Load) for ScenesPerTime, EqualityEpsilon precise documentation.
        Note that special value ScenesPerTime = 0 is interpreted here as
        "record only one, initial frame".)

      @param(ProgressTitle When <> '' we will use Progress.Init, Step, Fini
        to display nice progress of operation.) }
    procedure LoadFromVRMLEvents(
      RootNode: TX3DRootNode;
      AOwnsRootNode: boolean;
      const ATimeBegin, ATimeEnd: Single;
      ScenesPerTime: Cardinal;
      const EqualityEpsilon: Single;
      const ProgressTitle: string);

    { Load a dumb animation that consists of only one frame (so actually
      there's  no animation, everything is static).

      This just calls @link(Load) with parameters such that
      @orderedList(
        @item(RootNodes list contains one specified node)
        @item(Times contain only one item 0.0)
        @item(ScenesPerTime and EqualityEpsilon have some unimportant
          values --- they are not meaningfull when you have only one scene)
      )

      This is usefull when you know that you have a static scene,
      but still you want to treat it as T3DPrecalculatedAnimation. }
    procedure LoadStatic(
      RootNode: TX3DNode;
      AOwnsRootNode: boolean);

    { This loads T3DPrecalculatedAnimation by loading it's parameters
      (models to use, times to use etc.) from given file.

      Various file formats are possible, everything that can be handled by
      LoadVRMLSequence, in particular simple 3D model files, MD3,
      kanim (described on
      [http://castle-engine.sourceforge.net/kanim_format.php]).

      If you need more control over loading, for example you want to
      change some parameters at loading (for example, ScenesPerTime
      and EqualityEpsilon of kanim files), you should use
      more flexible (and less comfortable to use)
      LoadFromFileToVars class procedure (specialized for kanim files)
      or LoadVRMLSequence (if you want to handle any files).

      @link(Loaded) property changes to @true after calling this.

      @param(AllowStdIn If @true, then FileName = '-' is understood
        as "standard input".)

      @param(LoadTime If @true then loading changes
        current TimeLoop and TimeBackwards properties.
        Sometimes this is sensible (you want to allow control over them
        from the file), sometimes not (e.g. you set suitable values for them
        by code).

        Note that, independent of this, you can always change TimeLoop
        and TimeBackwards properties later,
        since these properties are writeable at any time.) }
    procedure LoadFromFile(const FileName: string;
      const AllowStdIn: boolean; const LoadTime: boolean);

    { This releases all resources allocared by Load (or LoadFromFile).
      @link(Loaded) property changes to @false after calling this.

      It's safe to call this even if @link(Loaded) is already @false --- then
      this will do nothing. }
    procedure Close;

    property Loaded: boolean read FLoaded;

    { Is the RootNode in first scene owned by this T3DPrecalculatedAnimation instance?
      If yes, it will be freed at closing the animation.
      Otherwise, you are responsible for freeing it yourself
      (but you cannot do this while animation is loaded, anyway). }
    property OwnsFirstRootNode: boolean
      read FOwnsFirstRootNode write SetOwnsFirstRootNode;

    { You can read anything from Scenes below. But you cannot set some
      things: don't set their scenes Attributes properties.
      Use only our @link(Attributes).

      The scenes here have T3DSceneCore.Static set to @true, which means
      we assume you will not modify their VRML nodes graph (by TVRMLField.Send
      and such). Note that this doesn't prevent you from enabling
      T3DSceneCore.ProcessEvents on the first scene (T3DSceneCore.ProcessEvents
      will be property handled regardless of T3DSceneCore.Static value). }
    property Scenes[I: Integer]: T3DScene read GetScenes;
    function ScenesCount: Integer;

    { Just a shortcut for Scenes[0]. }
    function FirstScene: T3DScene;

    { Just a shortcut for Scenes[ScenesCount - 1]. }
    function LastScene: T3DScene;

    { Prepare all scenes for rendering. Basically, this calls
      PrepareResources(...) for all Scenes.

      There's also a special memory (and prepare time) optimization used
      for prManifoldAndBorderEdges: we use the fact that animation scenes are
      "structurally equal", and so prepare and share one manifold edges
      information for all scenes.

      ProgressStep = @true is especially useful with this: we'll call
      Progress.Step then after preparing each scene.
      For portability, always check PrepareResourcesSteps, but for now this
      is just always equal ScenesCount. }
    procedure PrepareResources(Options: TPrepareResourcesOptions;
      ProgressStep: boolean; BaseLights: TAbstractLightInstancesList); override;
    function PrepareResourcesSteps: Cardinal; override;

    { This calls FreeResources for all scenes, it's useful if you know
      that you will not need some allocated resources anymore and you
      want to conserve memory use.

      See T3DSceneCore.FreeResource documentation for a description of what
      are possible resources to free.

      Note in case you pass frRootNode: the first scene has OwnsRootNode
      set to what you passed as OwnsFirstRootNode. Which means that
      if you passed OwnsFirstRootNode = @true, then frRootNode will @bold(not
      free) the initial RootNodes[0]. }
    procedure FreeResources(Resources: TSceneFreeResources);

    { Close anything associated with current OpenGL context in this class.
      This calls GLContextClose on every Scenes[], and additionally may close
      some other internal things here. }
    procedure GLContextClose; override;

    { Just a shortcut for TimeEnd - TimeBegin. }
    function TimeDuration: Single;

    { This is TimeDuration * 2 if TimeBackwards, otherwise it's just
      TimeDuration. In other words, this is the time of the one "full"
      (forward + backward) animation. }
    function TimeDurationWithBack: Single;

    { First and last time that you passed to Load (or that were read
      from file by LoadFromFile).
      In other words, Times[0] and Times[High(Times)].
      @groupBegin }
    property TimeBegin: Single read FTimeBegin;
    property TimeEnd: Single read FTimeEnd;
    { @groupEnd }

    { Appropriate scene from @link(Scenes) based on given Time.
      If Time is between given TimeBegin and TimeEnd,
      then this will be appropriate scene in the middle.

      For Time outside the range TimeBegin .. TimeEnd
      behavior depends on TimeLoop and TimeBackwards properties:

      @unorderedList(
        @item(When not TimeLoop and not TimeBackwards then:

          If Time is < TimeBegin, always the first scene will
          be returned. If Time is > TimeEnd, always the last scene will
          be returned.

          So there will no real animation outside
          TimeBegin .. TimeEnd timeline.)

        @item(When not TimeLoop and TimeBackwards then:

          If Time is < TimeBegin, always the first scene will
          be returned. If Time is between TimeEnd and
          TimeEnd + TimeDuration, then the animation
          will be played backwards. When Time is > TimeEnd + TimeDuration,
          again always the first scene will be returned.

          So between TimeEnd and TimeEnd + TimeDuration
          animation will be played backwards, and
          there will no real animation outside
          TimeBegin .. TimeEnd + TimeDuration timeline.)

        @item(When TimeLoop and not TimeBackwards then:

          Outside TimeBegin .. TimeEnd, animation will cycle.
          This means that e.g. between TimeEnd and TimeEnd + TimeDuration
          animation will be played just like between TimeBegin and TimeEnd.)

        @item(When TimeLoop and TimeBackwardsm then:

          Outside TimeBegin .. TimeEnd, animation will cycle.
          Cycle between TimeEnd and TimeEnd + TimeDuration will
          go backwards. Cycle between TimeEnd + TimeDuration
          and TimeEnd + TimeDuration * 2 will again go forward.
          And so on.)
      )
    }
    function SceneFromTime(const Time: Single): T3DScene;

    { Appropriate scene from @link(Scenes) based on current @link(Time).
      This is just a shortcut for SceneFromTime(@link(Time)),
      useful if you track animation time in our @link(Time) property. }
    function CurrentScene: T3DScene;

    { Attributes controlling rendering.
      See TSceneRenderingAttributes and TVRMLRenderingAttributes
      for documentation of properties.

      You can change properties of this
      object at any time, but beware that some changes may force
      time-consuming regeneration of some things (like OpenGL display lists)
      in the nearest Render of the scenes.
      So explicitly calling PrepareResources may be useful after
      changing these Attributes.

      Note that Attributes may be accessed and even changed when the scene
      is not loaded (e.g. before calling Load / LoadFromFile).
      Also, Attributes are preserved between various animations loaded. }
    function Attributes: TSceneRenderingAttributes;

    { The sum of bounding boxes of all animation frames.

      Result of this function is cached, which means that it usually returns
      very fast. But you have to call ChangedAll when you changed something
      inside Scenes[] using some direct Scenes[].RootNode operations,
      to force recalculation of this box. }
    function BoundingBox: TBox3D; override;

    { Call this before directly freeing some VRML nodes in animation scenes. }
    procedure BeforeNodesFree;

    { Call this when you changed something
      inside Scenes[] using some direct Scenes[].RootNode operations.
      This calls T3DScene.ChangedAll on all Scenes[]
      and invalidates some cached things inside this class. }
    procedure ChangedAll;

    { Returns some textual info about this animation.
      Similar to T3DScene.Info. }
    function Info(
      ATriangleVerticesCounts,
      ABoundingBox,
      AManifoldAndBorderEdges: boolean): string;

    { Handling key and mouse events, overriding TUIControl methods.

      We pass key and mouse events only if there's exactly one scene
      (ScenesCount = 1), as there's no sensible way of activating
      VRML events when T3DPrecalculatedAnimation contains more than one scene.
      (Precalculated animation of this class, and interactive
      animation by T3DSceneCore.ProcessEvents do not mix sensibly.)

      So when ScenesCount = 1, we simply pass key and mouse events to
      the only Scene[0]. Be sure to turn on @code(Scene[0].ProcessEvents := true)
      if you want to make actual use of it.

      @groupBegin }
    function KeyDown(Key: TKey; C: char): boolean; override;
    function KeyUp(Key: TKey; C: char): boolean; override;
    function MouseDown(const Button: TMouseButton): boolean; override;
    function MouseUp(const Button: TMouseButton): boolean; override;
    function MouseMove(const RayOrigin, RayDirection: TVector3Single;
      RayHit: T3DCollision): boolean; override;
    { @groupEnd }

    procedure Idle(const CompSpeed: Single); override;

    { Initial world time, set by the ResetTimeAtLoad call.
      This can be useful for showing user
      time like @code("Animation Time: LoadTime + %f") on status bar.

      0 means that starting @link(Time) was TimeBegin of the animation
      (0.0 in case of normal VRML files, usually 0.0 in case of Kanim).
      Note that even when TimeBegin <> 0 (for Kanim), we still set
      TimeAtLoad to 0, this is nicer to show to user.

      Other value means that we used current real time as time origin,
      following VRML/X3D specification.
      See also [http://castle-engine.sourceforge.net/vrml_time_origin_considered_uncomfortable.php] }
    property TimeAtLoad: TFloatTime read FTimeAtLoad;

    { Current time of the animation. Although you do not have to use it:
      you can always acccess any point in time of the animtion by SceneFromTime.
      But sometimes tracking the current time here is most natural
      and comfortable.

      When we have exactly one scene in Scenes, our methods (ResetTime,
      ResetTimeAtLoad and Idle) will synchronize Scenes[0].Time
      always to the same value as our own @link(Time).
      This makes time-dependent nodes (like TimeSensor,
      MovieTexture etc.) inside this scene work Ok. }
    property Time: TFloatTime read FTime;

    { Set @link(Time) to initial value after loading a world. }
    procedure ResetTimeAtLoad(const ForceTimeOrigin: boolean = false);

    { Set @link(Time) to arbitrary value. }
    procedure ResetTime(const NewValue: TFloatTime);

    procedure Render(const Frustum: TFrustum; const Params: TRenderParams); override;
    procedure RenderShadowVolume(
      ShadowVolumeRenderer: TBaseShadowVolumeRenderer;
      const ParentTransformIsIdentity: boolean;
      const ParentTransform: TMatrix4Single); override;

    procedure GetHeightAbove(const Position, GravityUp: TVector3Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc;
      out IsAbove: boolean; out AboveHeight: Single;
      out AboveGround: P3DTriangle); override;
    function MoveAllowed(
      const OldPos, ProposedNewPos: TVector3Single; out NewPos: TVector3Single;
      const CameraRadius: Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function MoveAllowedSimple(
      const OldPos, ProposedNewPos: TVector3Single;
      const CameraRadius: Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function MoveBoxAllowedSimple(
      const OldPos, ProposedNewPos: TVector3Single;
      const ProposedNewBox: TBox3D;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function SegmentCollision(const Pos1, Pos2: TVector3Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function SphereCollision(const Pos: TVector3Single; const Radius: Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function BoxCollision(const Box: TBox3D;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean; override;
    function RayCollision(
      out IntersectionDistance: Single;
      const Ray0, RayVector: TVector3Single;
      const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): T3DCollision; override;
    procedure UpdateGeneratedTextures(
      const RenderFunc: TRenderFromViewFunction;
      const ProjectionNear, ProjectionFar: Single;
      const OriginalViewportX, OriginalViewportY: LongInt;
      const OriginalViewportWidth, OriginalViewportHeight: Cardinal); override;
    procedure VisibleChangeNotification(const Changes: TVisibleChanges); override;
    function Dragging: boolean; override;

    property Cache: TGLRendererContextCache read FCache;

    { Turn this on to treat specially the case when a single scene (Scenes.Count = 1)
      is loaded: we will set this scene's Static = @false.
      This allows you to enable VRML/X3D events and dynamically change the scene
      in this very special case.
      The normal behavior, when we load many scenes (or when this property is @false),
      is to set all children scenes Static = @true.

      Practically, this is useful only for tools like view3dscene, that want
      to have full VRML/X3D events when possible, and at the same time they want
      to load everything as T3DPrecalculatedAnimation, for ease of coding.

      To put it simply, just don't use this in normal programs -- it's a hack.

      Although Static can be later changed, but changing it (after loading) to @false
      is expensive (needs ChangedAll, that also recalculates shape tree, forces
      shape octree and other recalculations). That's why this property is needed,
      it sets Static correctly before loading the contents. }
    property TryFirstSceneDynamic: boolean
      read FTryFirstSceneDynamic write FTryFirstSceneDynamic default false;
  published
    { Is the animation time playing, and how fast.

      For exact meaning of our TimePlaying, TimePlayingSpeed, see
      T3DSceneCore.TimePlaying, T3DSceneCore.TimePlayingSpeed.
      Like in T3DSceneCore, these are realized by our @link(Idle) method,
      so Time is automatically increased in @link(Idle) which is called
      automatically if you added this to some TCastleWindowCustom.Controls or
      TCastleControlCustom.Controls.

      Note that Scenes[0].TimePlaying, Scenes[0].TimePlayingSpeed do not matter
      when you're operating on the T3DPrecalculatedAnimation level.
      They will not affect our @link(Time), or even Scenes[0].Time,
      and they will not be synchronized with our values.

      @groupBegin }
    property TimePlaying: boolean read FTimePlaying write FTimePlaying default true;
    property TimePlayingSpeed: Single read FTimePlayingSpeed write FTimePlayingSpeed default 1.0;
    { @groupEnd }

    { See SceneFromTime for description what this property does. }
    property TimeLoop: boolean read FTimeLoop write FTimeLoop default true;

    { See SceneFromTime for description what this property does. }
    property TimeBackwards: boolean
      read FTimeBackwards write FTimeBackwards default false;

    { Should collision checking check also last animation frame.

      Regardless of this value, we always check collision with the
      first animation frame (FirstScene), of course only when
      FirstScene.OctreeCollisions is initialized, and only if
      @link(Collides) and @link(Exists).

      When CollisionUseLastScene is @true, we will also check collision
      with the last animation frame's octree, i.e. LastScene.OctreeCollisions.
      (Of course, only if it's initialized, e.g. by adding
      ssDynamicCollisions to the LastScene.Spatial property.)
      So when CollisionUseLastScene, collision checking sees the animation
      as a sum of first and last frames geometry.
      CollisionUseLastScene
      is useful if the object is moving, but the move is very slight,
      so that the sum of first and last scenes geometry is good enough
      approximation of the whole geometry at any point of the animation.

      Although it seems like a totally dumb way to check for collisions,
      it's suitable for many purposes (see e.g. uses on "castle hall" level),
      it's simple and not memory-consuming, and you don't have to take
      any action when animation frame changes (because @link(Time) changes
      don't change the colliding geometry, so the animation is static from
      the point of view of collision checking routines).

      TODO: In the future other collision methods may be available.
      First of all, checking with sum of all bounding boxes, or with particular
      scene time box, should be available. }
    property CollisionUseLastScene: boolean
      read FCollisionUseLastScene
      write FCollisionUseLastScene default false;

    { At loading, process the animation to support shadow maps.
      See T3DSceneCore.ShadowMaps and related properties for documentation.
      @groupBegin }
    property ShadowMaps: boolean read FShadowMaps write SetShadowMaps default true;
    property ShadowMapsDefaultSize: Cardinal
      read FShadowMapsDefaultSize write SetShadowMapsDefaultSize
      default DefaultShadowMapsDefaultSize;
    { @groupEnd }

    property InitialViewpointIndex: Cardinal
      read FInitialViewpointIndex write FInitialViewpointIndex;

    property InitialViewpointName: string
      read FInitialViewpointName write FInitialViewpointName;
  end;

  T3DPrecalculatedAnimationList = specialize TFPGObjectList<T3DPrecalculatedAnimation>;

procedure Register;

implementation

uses Math, X3DFields, ProgressUnit, X3DLoad, CastleLog, DateUtils,
  Shape;

procedure Register;
begin
  RegisterComponents('Kambi', [T3DPrecalculatedAnimation]);
end;

{ T3DPrecalculatedAnimationScene ------------------------------------------------------ }

type
  T3DPrecalculatedAnimationScene = class(T3DScene)
  private
    FParentAnimation: T3DPrecalculatedAnimation;
  public
    constructor CreateForAnimation(
      ARootNode: TX3DRootNode; AOwnsRootNode: boolean;
      ACustomRenderer: TGLRenderer;
      AParentAnimation: T3DPrecalculatedAnimation;
      AStatic: boolean);
    property ParentAnimation: T3DPrecalculatedAnimation read FParentAnimation;
    procedure DoGeometryChanged(const Change: TGeometryChange;
      LocalGeometryShape: TShape); override;
    procedure VisibleChangeHere(const Changes: TVisibleChanges); override;
    procedure CursorChange; override;
  end;

constructor T3DPrecalculatedAnimationScene.CreateForAnimation(
  ARootNode: TX3DRootNode; AOwnsRootNode: boolean;
  ACustomRenderer: TGLRenderer;
  AParentAnimation: T3DPrecalculatedAnimation;
  AStatic: boolean);
begin
  { ParentAnimation is used by DoGeometryChanged, which is virtual and
    *may* be called by ChangedAll, which *may* called by inherited constructor.
    So ParentAnimation must be set even before inherited constructor. }
  FParentAnimation := AParentAnimation;

  inherited CreateCustomRenderer(nil, ACustomRenderer);

  ShadowMaps := FParentAnimation.ShadowMaps;
  ShadowMapsDefaultSize := FParentAnimation.ShadowMapsDefaultSize;
  InitialViewpointIndex := FParentAnimation.InitialViewpointIndex;
  InitialViewpointName := FParentAnimation.InitialViewpointName;

  Static := AStatic;

  Load(ARootNode, AOwnsRootNode);
end;

procedure T3DPrecalculatedAnimationScene.DoGeometryChanged(const Change: TGeometryChange;
  LocalGeometryShape: TShape);
begin
  inherited;
  ParentAnimation.ValidBoundingBox := false;
end;

procedure T3DPrecalculatedAnimationScene.VisibleChangeHere(const Changes: TVisibleChanges);
begin
  inherited;
  ParentAnimation.VisibleChangeHere(Changes);
end;

procedure T3DPrecalculatedAnimationScene.CursorChange;
begin
  inherited;

  { Maybe in the future we will update here our own cursor, for now: no need.
    See T3DList.ListCursorChange implementation comments. }

  ParentAnimation.CursorChange;
end;

{ EModelsStructureDifferent -------------------------------------------------- }

type
  EModelsStructureDifferent = class(Exception)
    constructor CreateFmt(const S: string; const Args: array of const);
  end;

constructor EModelsStructureDifferent.CreateFmt(const S: string;
  const Args: array of const);
begin
  inherited CreateFmt('Models are structurally different: ' + S, Args);
end;

{ T3DPrecalculatedAnimation ------------------------------------------------------------ }

{ About Create and CreateCustomCache relationship:

  Note that Create cannot call CreateCustomCache and depend that
  CreateCustomCache calls "inherited Create". This wouldn't be nice
  for descendants: If some T3DPrecalculatedAnimation descendant would override "Create"
  to do his initialization, and we would create this descendant by
  CreateCustomCache --- we would miss executing descendant's constructor code.

  So only our Create may call "inherited Create".
  CreateCustomCache should always call just "Create" (virtual). }

constructor T3DPrecalculatedAnimation.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);

  if Cache = nil then
  begin
    OwnsCache := true;
    FCache := TGLRendererContextCache.Create;
  end;

  Renderer := TGLRenderer.Create(TSceneRenderingAttributes, Cache);

  FTimeLoop := true;
  FTimeBackwards := false;
  FTimePlaying := true;
  FTimePlayingSpeed := 1.0;
  FShadowMaps := true;
  FShadowMapsDefaultSize := DefaultShadowMapsDefaultSize;
end;

constructor T3DPrecalculatedAnimation.CreateCustomCache(AOwner: TComponent;
  ACache: TGLRendererContextCache);
begin
  OwnsCache := false;
  FCache := ACache;

  Create(AOwner);
end;

destructor T3DPrecalculatedAnimation.Destroy;
begin
  Close;
  FreeAndNil(Renderer);
  if OwnsCache then
    FreeAndNil(FCache) else
    FCache := nil;
  inherited;
end;

procedure T3DPrecalculatedAnimation.LoadCore(
  GetRootNodeWithTime: TGetRootNodeWithTime;
  RootNodesCount: Cardinal;
  AOwnsFirstRootNode: boolean;
  ScenesPerTime: Cardinal;
  const EqualityEpsilon: Single);

  { This will check that Model1 and Model2 are exactly equal,
    or that at least interpolating (see VRMLModelLerp) is possible.

    If models are structurally different (which means that even
    interpolating between Model1 and Model2 is not possible),
    it will raise EModelsStructureDifferent. }
  procedure CheckVRMLModelsStructurallyEqual(Model1, Model2: TX3DNode);

    procedure CheckSFNodesStructurallyEqual(Field1, Field2: TSFNode);
    begin
      if (Field1.Value <> nil) and (Field2.Value <> nil) then
      begin
        CheckVRMLModelsStructurallyEqual(Field1.Value, Field2.Value);
      end else
      if not ((Field1.Value = nil) and (Field2.Value = nil)) then
        raise EModelsStructureDifferent.CreateFmt('Field "%s" of type SFNode ' +
          'is once NULL and once not-NULL', [Field1.Name]);
    end;

    procedure CheckMFNodesStructurallyEqual(Field1, Field2: TMFNode);
    var
      I: Integer;
    begin
      if Field1.Items.Count <> Field2.Items.Count then
        raise EModelsStructureDifferent.CreateFmt(
          'Different number of children in MFNode fields: "%d" and "%d"',
          [Model1.VRML1ChildrenCount, Model2.VRML1ChildrenCount]);

      for I := 0 to Field1.Items.Count - 1 do
        CheckVRMLModelsStructurallyEqual(Field1[I], Field2[I]);
    end;

  var
    I: Integer;
  begin
    { Yes, Model1 and Model2 must have *exactly* the same classes. }
    if Model1.ClassType <> Model2.ClassType then
      raise EModelsStructureDifferent.CreateFmt(
        'Different nodes classes: "%s" and "%s"',
        [Model1.ClassName, Model2.ClassName]);

    { Make sure that *Inline content is loaded now. }
    if Model1 is TInlineNode then
    begin
      TInlineNode(Model1).LoadInlined(false);
      TInlineNode(Model2).LoadInlined(false);
    end;

    if Model1.NodeName <> Model2.NodeName then
      raise EModelsStructureDifferent.CreateFmt(
        'Different names of nodes: "%s" and "%s"',
        [Model1.NodeName, Model2.NodeName]);

    if Model1.WWWBasePath <> Model2.WWWBasePath then
      raise EModelsStructureDifferent.CreateFmt(
        'Different WWWBasePath of nodes: "%s" and "%s"',
        [Model1.WWWBasePath, Model2.WWWBasePath]);

    if Model1.VRML1ChildrenCount <> Model2.VRML1ChildrenCount then
      raise EModelsStructureDifferent.CreateFmt(
        'Different number of children in nodes: "%d" and "%d"',
        [Model1.VRML1ChildrenCount, Model2.VRML1ChildrenCount]);

    for I := 0 to Model1.VRML1ChildrenCount - 1 do
      CheckVRMLModelsStructurallyEqual(Model1.VRML1Children[I], Model2.VRML1Children[I]);

    { Yes, the situation below can happen. *Usually* when we know
      that Model1 and Model2 are equal classes then we know that
      they have the same number of fields of the same type.
      However, for TX3DUnknownNode, it's not that easy. Two different instances
      of TX3DUnknownNode class may have completely different fields,
      so we must safeguard against this. }
    if Model1.Fields.Count <> Model2.Fields.Count then
      raise EModelsStructureDifferent.CreateFmt(
        'Different number of fields in nodes: "%d" and "%d"',
        [Model1.Fields.Count, Model2.Fields.Count]);

    for I := 0 to Model1.Fields.Count - 1 do
    begin
      if Model1.Fields[I].ClassType <> Model2.Fields[I].ClassType then
        raise EModelsStructureDifferent.CreateFmt(
          'Different type of field number %d in nodes: "%s" and "%s"',
          [I, Model1.Fields[I].ClassName, Model2.Fields[I].ClassName]);

      if Model1.Fields[I] is TSFNode then
        CheckSFNodesStructurallyEqual(
          TSFNode(Model1.Fields[I]), TSFNode(Model2.Fields[I])) else
      if Model1.Fields[I] is TMFNode then
        CheckMFNodesStructurallyEqual(
          TMFNode(Model1.Fields[I]), TMFNode(Model2.Fields[I])) else
      if Model1.Fields[I].CanAssignLerp then
      begin
        if Model1.Fields[I] is TVRMLMultField then
        begin
          try
            (Model1.Fields[I] as TVRMLMultField).CheckCountEqual
              (Model2.Fields[I] as TVRMLMultField);
          except
            (* Translate EVRMLMultFieldDifferentCount exception
               (may be raised by TVRMLMultField.CheckCountEqual above)
               to EModelsStructureDifferent. *)
            on E: EVRMLMultFieldDifferentCount do
              raise EModelsStructureDifferent.CreateFmt('%s', [E.Message]);
          end;
        end;
        { Else we have single-value field that can lerp.
          No need to check anything in this case,
          it's ready to go (that is, to lerp). }
      end else
      begin
        { Check fields for equality.

          Some special fields like TInlineNode.FdUrl do not
          have to be equal, as they don't have any role for the
          "real" meaning of the model. I mean, if TInlineNode.Inlined
          contents (loaded from pointed file) have the same structure,
          then we're happy. And it's handy to allow this --- see e.g.
          examples/models/gus_1_final.wrl and
          examples/models/gus_2_final.wrl trick. }

        if not (
           ( (Model1 is TInlineNode)            and (Model1.Fields[I].Name = 'url') ) or
           Model1.Fields[I].Equals(Model2.Fields[I], EqualityEpsilon)
           ) then
          raise EModelsStructureDifferent.CreateFmt(
            'Fields "%s" (class "%s") are not equal',
            [Model1.Fields[I].Name, Model1.Fields[I].ClassName]);
      end;
    end;
  end;

  { This will merge equal children of Model1 and Model2,
    and check that Model1 and Model2 are exactly equal.

    It assumes that models are structurally equal, i.e. that you
    already did run CheckVRMLModelsStructurallyEqual over them.

    It works recursively: first it checks for every children
    are they equal. For each pair that is equal, it replaces
    given children in Model2 with appropriate children of Model1.
    At the end, if every children pair was equal and additionally
    if all fields are equal, then it returns true.

    Such copying of references is useful, because then we simply copy given
    node's reference instead of duplicating this object.
    This way Model1 and Model2 and all models interpolated along the way
    can share the same object reference. This is very good, because:

    1. If nodes are equal then creating new object each
       time would mean that I create a lot of objects with exactly the
       same contents. So memory is wasted, without any good reason.

    2. For nodes like ImageTexture, this is good because then the image
       is loaded from the file only once. This means that memory is saved,
       once again. This also means that in case when texture file doesn't
       exist, user gets only 1 warning/error message (instead of getting
       warning/error message for each duplicated TImageTextureNode instance).

    3. Also for nodes like ImageTexture, this means that if we use the same
       GLRenderer to render every model of the animation,
       then GLRenderer will recognize this and given texture
       will be loaded only once for OpenGL. So loading time and
       memory are saved *once again*  (otherwise OpenGL would allocate
       internal copy of texture for each duplicated node, once again
       wasting a lot of memory).

       Although 2. and 3. are actually somewhat void right now,
       as we have a more general cache that caches texture resources
       even across different nodes right now (the only need is to have
       equal URLs).

    4. And later the Shape cache of TGLRenderer can speed
       up loading time and conserve memory use, if it sees the same
       reference to given GeometryNode twice. }
  function VRMLModelsMerge(Model1, Model2: TX3DNode): boolean;

    function SFNodesMerge(Field1, Field2: TSFNode): boolean;
    begin
      Result := true;

      { Equality was already checked by CheckVRMLModelsStructurallyEqual,
        so now if one SFNode value is not nil, we know that the other
        one is not nil too. }
      if Field1.Value <> nil then
      begin
        if VRMLModelsMerge(Field1.Value, Field2.Value) then
          Field1.Value := Field2.Value else
          Result := false;
      end;
    end;

    function MFNodesMerge(Field1, Field2: TMFNode): boolean;
    var
      I: Integer;
    begin
      Result := true;

      { Note that we already know that Counts are equals,
        checked already by CheckVRMLModelsStructurallyEqual. }
      Assert(Field1.Items.Count = Field2.Items.Count);
      for I := 0 to Field1.Items.Count - 1 do
      begin
        if VRMLModelsMerge(Field1[I], Field2[I]) then
        begin
          { Think of this as
              Field1[I] := Field2[I]
            but I can't call this directly, I must use Field1.Replace
            to not mess reference counts. }
          Field1.Replace(I, Field2[I]);
        end else
          Result := false;
      end;
    end;

  var
    I: Integer;
  begin
    Result := true;

    { Note that this loop will iterate over every Children,
      even if somewhere along the way we will already set Result to false.
      Even if we already know that Result is false, we stil want to
      merge Model1 and Model2 children as much as we can. }
    for I := 0 to Model1.VRML1ChildrenCount - 1 do
    begin
      if VRMLModelsMerge(Model1.VRML1Children[I], Model2.VRML1Children[I]) then
      begin
        { Tests: Writeln('merged child ', I, ' of class ',
          Model1.VRML1Children[I].NodeTypeName); }
        Model1.VRML1Children[I] := Model2.VRML1Children[I];
      end else
        Result := false;
    end;

    if not Result then Exit;

    for I := 0 to Model1.Fields.Count - 1 do
    begin
      if Model1.Fields[I] is TSFNode then
      begin
        if not SFNodesMerge(TSFNode(Model1.Fields[I]),
                            TSFNode(Model2.Fields[I])) then
          Result := false;
      end else
      if Model1.Fields[I] is TMFNode then
      begin
        if not MFNodesMerge(TMFNode(Model1.Fields[I]),
                            TMFNode(Model2.Fields[I])) then
          Result := false;
      end else
      if Model1.Fields[I].CanAssignLerp then
      begin
        if not Model1.Fields[I].Equals(Model2.Fields[I], EqualityEpsilon) then
          Result := false;
      end;

      { Other fields were already checked by CheckVRMLModelsStructurallyEqual }
    end;
  end;

  { Linear interpolation between Model1 and Model2.
    A = 0 means Model1, A = 1 means Model2, A between 0 and 1 is lerp
    between Model1 and Model2.

    If Model1 and Model2 are the same object (the same references),
    then this will return just Model1. This way it keeps memory optimization
    described by VRMLModelsMerge. This is also true if both Model1 and Model2
    are nil: then you can safely call this and it will return also nil. }
  function VRMLModelLerp(const A: Single; Model1, Model2: TX3DNode): TX3DNode;

    procedure SFNodeLerp(Target, Field1, Field2: TSFNode);
    begin
      Target.Value := VRMLModelLerp(A, Field1.Value, Field2.Value);
    end;

    procedure MFNodeLerp(Target, Field1, Field2: TMFNode);
    var
      I: Integer;
    begin
      for I := 0 to Field1.Items.Count - 1 do
        Target.Add(VRMLModelLerp(A, Field1[I], Field2[I]));
    end;

  var
    I: Integer;
  begin
    if Model1 = Model2 then
      Exit(Model1);

    Result := TX3DNodeClass(Model1.ClassType).Create(Model1.NodeName,
      Model1.WWWBasePath);
    try
      { We already loaded all inlines (in CheckVRMLModelsStructurallyEqual).
        We have to mark it now, by setting Loaded := true field as necessary
        inside inline nodes --- otherwise, they could be loaded again
        (adding content to already existing nodes, making content loaded
        more than once). }
      if Result is TInlineNode then
      begin
        TInlineNode(Result).LoadedInlineDirectly;
      end;

      { TODO: the code below doesn't deal efficiently with the situation when single
        TX3DNode is used as a child many times in one of the nodes.
        (through VRML "USE" keyword). Code below will then unnecessarily
        create copies of such things (wasting construction time and memory),
        instead of reusing the same object reference. }
      for I := 0 to Model1.VRML1ChildrenCount - 1 do
        Result.VRML1ChildAdd(VRMLModelLerp(A, Model1.VRML1Children[I], Model2.VRML1Children[I]));

      { TODO: for TX3DUnknownNode, we should fill here Result.Fields.
        Also for TX3DPrototypeNode. }

      for I := 0 to Model1.Fields.Count - 1 do
      begin
        if Model1.Fields[I] is TSFNode then
        begin
          SFNodeLerp(
            (Result.Fields[I] as TSFNode),
            (Model1.Fields[I] as TSFNode),
            (Model2.Fields[I] as TSFNode));
        end else
        if Model1.Fields[I] is TMFNode then
        begin
          MFNodeLerp(
            (Result.Fields[I] as TMFNode),
            (Model1.Fields[I] as TMFNode),
            (Model2.Fields[I] as TMFNode));
        end else
        if Model1.Fields[I].CanAssignLerp then
        begin
          Result.Fields[I].AssignLerp(A, Model1.Fields[I], Model2.Fields[I]);
        end else
        begin
          { These fields cannot be interpolated.
            So just copy to Result.Fields[I]. }
          Result.Fields[I].Assign(Model1.Fields[I]);
        end;
      end;
    except
      FreeAndNil(Result);
      raise;
    end;
  end;

var
  SceneStatic: boolean;

  function CreateOneScene(Node: TX3DRootNode;
    OwnsRootNode: boolean): T3DPrecalculatedAnimationScene;
  begin
    Result := T3DPrecalculatedAnimationScene.CreateForAnimation(
      Node, OwnsRootNode, Renderer, Self, SceneStatic);
  end;

var
  I: Integer;
  StructurallyEqual, RootNodesEqual: boolean;
  LastSceneIndex: Integer;
  LastSceneRootNode, NewRootNode: TX3DRootNode;
  LastTime, NewTime: Single;
  SceneIndex: Integer;
begin
  Close;

  FOwnsFirstRootNode := AOwnsFirstRootNode;

  { We want all the scenes to be dynamic only when
    (TryFirstSceneDynamic and (FScenes.Count = 1)).
    We don't know yet FScenes.Count, but FScenes.Count = 1 is quite special:
    it only (if and only if) occurs if RootNodesCount = 1. }
  SceneStatic := not (TryFirstSceneDynamic and (RootNodesCount = 1));

  FScenes := T3DSceneList.Create(false);

  { calculate FScenes contents now }

  { RootNodes[0] goes to FScenes[0], that's easy }
  GetRootNodeWithTime(0, NewRootNode, NewTime);

  FScenes.Count := 1;
  FScenes[0] := CreateOneScene(NewRootNode, OwnsFirstRootNode);
  LastSceneIndex := 0;
  LastTime := NewTime;
  LastSceneRootNode := NewRootNode;

  { calculate TimeBegin at this point }
  FTimeBegin := NewTime;

  for I := 1 to RootNodesCount - 1 do
  begin
    { Now add RootNodes[I] }
    GetRootNodeWithTime(I, NewRootNode, NewTime);

    StructurallyEqual := false;

    try
      CheckVRMLModelsStructurallyEqual(LastSceneRootNode, NewRootNode);
      StructurallyEqual := true;
    except
      on E: EModelsStructureDifferent do
      begin
        if Log then
          WritelnLog('PrecalculatedAnimation', Format(
            'Nodes %d and %d structurally different, so animation will not be smoothed between them: ',
            [I - 1, I]) + E.Message);
      end;
    end;

    FScenes.Count := FScenes.Count +
      Max(1, Round((NewTime - LastTime) * ScenesPerTime));

    if StructurallyEqual then
    begin
      { Try to merge it with LastSceneRootNode.
        Then initialize FScenes[LastSceneIndex + 1 to FScenes.Count - 1]. }
      RootNodesEqual := VRMLModelsMerge(NewRootNode, LastSceneRootNode);
      if RootNodesEqual then
      begin
        { In this case I don't waste memory, and I'm simply reusing
          LastSceneRootNode. Actually, I'm just copying FScenes[LastSceneIndex].
          This way I have a series of the same instances of T3DScene
          along the way. When freeing FScenes, we will be smart and
          avoid deallocating the same pointer twice. }
        FreeAndNil(NewRootNode);
        for SceneIndex := LastSceneIndex + 1 to FScenes.Count - 1 do
          FScenes[SceneIndex] := FScenes[LastSceneIndex];
      end else
      begin
        for SceneIndex := LastSceneIndex + 1 to FScenes.Count - 2 do
          FScenes[SceneIndex] := CreateOneScene(VRMLModelLerp(
            MapRange(SceneIndex, LastSceneIndex, FScenes.Count - 1, 0.0, 1.0),
            LastSceneRootNode, NewRootNode) as TX3DRootNode, true);
        FScenes[FScenes.Count - 1] := CreateOneScene(NewRootNode, true);
        LastSceneRootNode := NewRootNode;
      end;
    end else
    begin
      { We cannot interpolate between last and new node.
        So just duplicate last node until FScenes.Count - 2,
        and at FScenes.Last insert new node. }
      for SceneIndex := LastSceneIndex + 1 to FScenes.Count - 2 do
        FScenes[SceneIndex] := FScenes[LastSceneIndex];
      FScenes[FScenes.Count - 1] := CreateOneScene(NewRootNode, true);
      LastSceneRootNode := NewRootNode;
    end;

    LastTime := NewTime;
    LastSceneIndex := FScenes.Count - 1;
  end;

  { calculate TimeEnd at this point }
  FTimeEnd := NewTime;

  FLoaded := true;
end;

procedure T3DPrecalculatedAnimation.Load_GetRootNodeWithTime(const Index: Cardinal;
  out RootNode: TX3DRootNode; out Time: Single);
begin
  RootNode := Load_RootNodes[Index] as TX3DRootNode;
  Time := Load_Times[Index];
end;

procedure T3DPrecalculatedAnimation.Load(
  RootNodes: TX3DNodeList;
  AOwnsFirstRootNode: boolean;
  ATimes: TSingleList;
  ScenesPerTime: Cardinal;
  const EqualityEpsilon: Single);
begin
  Assert(RootNodes.Count = ATimes.Count);
  Load_RootNodes := RootNodes;
  Load_Times := ATimes;

  LoadCore(@Load_GetRootNodeWithTime, RootNodes.Count,
    AOwnsFirstRootNode, ScenesPerTime, EqualityEpsilon);
end;

procedure T3DPrecalculatedAnimation.LoadFromVRMLEvents_GetRootNodeWithTime(
  const Index: Cardinal;
  out RootNode: TX3DRootNode; out Time: Single);
begin
  Time := LoadFromVRMLEvents_TimeBegin;
  if LoadFromVRMLEvents_ScenesPerTime <> 0 then
    Time += Index / LoadFromVRMLEvents_ScenesPerTime;

  if Index = 0 then
    LoadFromVRMLEvents_Scene.ResetTime(Time) else
    LoadFromVRMLEvents_Scene.SetTime(Time);

  RootNode := LoadFromVRMLEvents_Scene.RootNode.DeepCopy as TX3DRootNode;
end;

procedure T3DPrecalculatedAnimation.LoadFromVRMLEvents_GetRootNodeWithTime_Progress(
  const Index: Cardinal;
  out RootNode: TX3DRootNode; out Time: Single);
begin
  LoadFromVRMLEvents_GetRootNodeWithTime(Index, RootNode, Time);
  Progress.Step;
end;

procedure T3DPrecalculatedAnimation.LoadFromVRMLEvents(
  RootNode: TX3DRootNode;
  AOwnsRootNode: boolean;
  const ATimeBegin, ATimeEnd: Single;
  ScenesPerTime: Cardinal;
  const EqualityEpsilon: Single;
  const ProgressTitle: string);
var
  Count: Cardinal;
begin
  LoadFromVRMLEvents_ScenesPerTime := ScenesPerTime;
  LoadFromVRMLEvents_TimeBegin := ATimeBegin;
  LoadFromVRMLEvents_Scene := T3DSceneCore.Create(nil);
  try
    LoadFromVRMLEvents_Scene.Load(RootNode, AOwnsRootNode);

    Count := Max(1, Round((ATimeEnd - ATimeBegin) * ScenesPerTime));

    LoadFromVRMLEvents_Scene.ProcessEvents := true;

    if ProgressTitle <> '' then
    begin
      Progress.Init(Count, ProgressTitle);
      try
        LoadCore(@LoadFromVRMLEvents_GetRootNodeWithTime_Progress, Count,
          true, 0, EqualityEpsilon);
      finally
        Progress.Fini;
      end;
    end else
    begin
      LoadCore(@LoadFromVRMLEvents_GetRootNodeWithTime, Count,
        true, 0, EqualityEpsilon);
    end;

    { Although LoadCore sets FTimeEnd already, it may be a little
      smaller than ATimeEnd if ScenesPerTime is very small.
      Last scene generated by LoadFromVRMLEvents_GetRootNodeWithTime
      will not necessarily "hit" exactly TimeEnd.
      In particular, when ScenesPerTime = 0, LoadCore will just set
      FTimeEnd to TimeBegin...

      Since we guarantee in the interface that FTimeEnd will be exactly
      equal to ATimeEnd after LoadFromVRMLEvents, we fix it here. }

    FTimeEnd := ATimeEnd;
  finally FreeAndNil(LoadFromVRMLEvents_Scene) end;
end;

procedure T3DPrecalculatedAnimation.LoadStatic(
  RootNode: TX3DNode;
  AOwnsRootNode: boolean);
var
  RootNodes: TX3DNodeList;
  ATimes: TSingleList;
begin
  RootNodes := TX3DNodeList.Create(false);
  try
    ATimes := TSingleList.Create;
    try
      RootNodes.Add(RootNode);
      ATimes.Add(0);
      Load(RootNodes, AOwnsRootNode, ATimes, 1, 0.0);
    finally FreeAndNil(ATimes) end;
  finally FreeAndNil(RootNodes) end;
end;

procedure T3DPrecalculatedAnimation.LoadFromFile(const FileName: string;
  const AllowStdIn, LoadTime: boolean);
var
  Times: TSingleList;
  RootNodes: TX3DNodeList;
  ScenesPerTime: Cardinal;
  EqualityEpsilon: Single;
  NewTimeLoop, NewTimeBackwards: boolean;
begin
  Times := TSingleList.Create;
  RootNodes := TX3DNodeList.Create(false);
  try
    LoadVRMLSequence(FileName, AllowStdIn,
      RootNodes, Times, ScenesPerTime, EqualityEpsilon,
      NewTimeLoop, NewTimeBackwards);

    Load(RootNodes, true, Times, ScenesPerTime, EqualityEpsilon);

    if LoadTime then
    begin
      TimeLoop := NewTimeLoop;
      TimeBackwards := NewTimeBackwards;
    end;
  finally
    FreeAndNil(Times);
    FreeAndNil(RootNodes);
  end;
end;

procedure T3DPrecalculatedAnimation.Close;
var
  I: Integer;
begin
  { This is called from destructor, so this must always check whether
    things are <> nil before trying to free them. }

  GLContextClose;

  if FScenes <> nil then
  begin
    { Although FScenes.Count should always be > 0 when FScenes allocated,
      this is a destructor so we must handle various abnormal situations
      if we exit with exception. }
    if FScenes.Count <> 0 then
    begin
      { Now we must note that we may have a sequences of the same scenes
        on FScenes. So we must deallocate smartly, to avoid deallocating
        the same pointer more than once. }
      for I := 0 to FScenes.Count - 2 do
      begin
        if FScenes[I] = FScenes[I+1] then
          FScenes[I] := nil { set to nil, just for safety } else
          FPGObjectList_FreeAndNilItem(FScenes, I);
      end;
      FPGObjectList_FreeAndNilItem(FScenes, FScenes.Count - 1);
    end;

    FreeAndNil(FScenes);
  end;

  ValidBoundingBox := false;

  FLoaded := false;
end;

function T3DPrecalculatedAnimation.GetScenes(I: Integer): T3DScene;
begin
  Result := FScenes[I];
end;

function T3DPrecalculatedAnimation.ScenesCount: Integer;
begin
  if Loaded then
    Result := FScenes.Count else
    Result := 0;
end;

function T3DPrecalculatedAnimation.FirstScene: T3DScene;
begin
  Result := FScenes.First;
end;

function T3DPrecalculatedAnimation.LastScene: T3DScene;
begin
  Result := FScenes.Last;
end;

procedure T3DPrecalculatedAnimation.PrepareResources(Options: TPrepareResourcesOptions;
  ProgressStep: boolean; BaseLights: TAbstractLightInstancesList);
var
  I: Integer;
  SceneOptions: TPrepareResourcesOptions;
begin
  if not Loaded then Exit;

  for I := 0 to FScenes.Count - 1 do
  begin
    { For I <> 0, we don't want to pass prManifoldAndBorderEdges to scenes. }
    SceneOptions := Options;
    if I <> 0 then
      Exclude(SceneOptions, prManifoldAndBorderEdges);

    FScenes[I].PrepareResources(SceneOptions, false, BaseLights);

    { TODO: this isn't so simple, since not all scenes have to structurally
      equal anymore. }
    if (prManifoldAndBorderEdges in Options) and (I <> 0) then
      FScenes[I].ShareManifoldAndBorderEdges(
        FScenes[0].ManifoldEdges, FScenes[0].BorderEdges);

    if ProgressStep then
      Progress.Step;
  end;
end;

function T3DPrecalculatedAnimation.PrepareResourcesSteps: Cardinal;
begin
  Result := ScenesCount;
end;

procedure T3DPrecalculatedAnimation.FreeResources(Resources: TSceneFreeResources);
var
  I: Integer;
begin
  for I := 0 to FScenes.Count - 1 do
    FScenes[I].FreeResources(Resources);
end;

procedure T3DPrecalculatedAnimation.GLContextClose;
{ Note that this is called from destructor, so we must be extra careful
  here and check is everything <> nil before freeing it. }
begin
  if FScenes <> nil then
    FScenes.GLContextClose;

  if Renderer <> nil then
    Renderer.UnprepareAll;
end;

function T3DPrecalculatedAnimation.TimeDuration: Single;
begin
  Result := TimeEnd - TimeBegin;
end;

function T3DPrecalculatedAnimation.TimeDurationWithBack: Single;
begin
  Result := TimeDuration;
  if TimeBackwards then
    Result *= 2;
end;

function T3DPrecalculatedAnimation.SceneFromTime(const Time: Single): T3DScene;
var
  SceneNumber: Integer;
  DivResult: SmallInt;
  ModResult: Word;
begin
  if FScenes.Count = 1 then
  begin
    { In this case TimeBegin = TimeEnd, so it's better to not perform
      any MapRange(Time, TimeBegin, TimeEnd, ...) calculation here
      and just treat this as a special case. }
    SceneNumber := 0;
  end else
  begin
    { I use FScenes.Count, not FScenes.Count - 1 as the highest range value.
      This is critical. On the short range (not looping), it may seem
      that FScenes.Count - 1 is more appropriate, since the last scene
      corresponds exactly to TimeEnd. But that's not good for looping:
      in effect float range TimeDuration would contain one scene less,
      and so when looking at large Time values, the scenes are slightly shifted
      within time.

      This causes problems when code relies on the meaning of some time
      values. E.g. if TimeBegin = 0, you expect that Time = k * TimeEnd,
      for any k, will result in the LastScene generated (assuming
      backwards is @true). This is needed for tricks like smooth animations
      concatenation, see "the rift" in RiftCreatures unit.

      When using FScenes.Count - 1, we would break this, as scenes are shifted
      by one in each range. }
    SceneNumber := Floor(MapRange(Time, TimeBegin, TimeEnd, 0, FScenes.Count));

    DivUnsignedMod(SceneNumber, FScenes.Count, DivResult, ModResult);

    if TimeLoop then
    begin
      if TimeBackwards and Odd(DivResult) then
        SceneNumber := FScenes.Count - 1 - ModResult else
        SceneNumber := ModResult;
    end else
    begin
      if TimeBackwards then
      begin
        if (DivResult < 0) or (DivResult > 1) then
          SceneNumber := 0 else
        if DivResult = 1 then
          SceneNumber := FScenes.Count - 1 - ModResult;
          { else DivResult = 0, so SceneNumber is already correct }
      end else
      begin
        if DivResult < 0 then
          SceneNumber := 0 else
        if DivResult > 0 then
          SceneNumber := FScenes.Count - 1;
      end;
    end;
  end;

  Result := FScenes[SceneNumber];
end;

function T3DPrecalculatedAnimation.Attributes: TSceneRenderingAttributes;
begin
  Result := TSceneRenderingAttributes(Renderer.Attributes);
end;

function T3DPrecalculatedAnimation.BoundingBox: TBox3D;

  procedure ValidateBoundingBox;
  var
    I: Integer;
  begin
    FBoundingBox := FScenes[0].BoundingBox;
    for I := 1 to FScenes.Count - 1 do
      FBoundingBox.Add(FScenes[I].BoundingBox);
    ValidBoundingBox := true;
  end;

begin
  if Loaded and Exists then
  begin
    if not ValidBoundingBox then
      ValidateBoundingBox;
    Result := FBoundingBox;
  end else
    Result := EmptyBox3D;
end;

procedure T3DPrecalculatedAnimation.BeforeNodesFree;
var
  I: Integer;
begin
  for I := 0 to FScenes.Count - 1 do
    FScenes[I].BeforeNodesFree;
end;

procedure T3DPrecalculatedAnimation.ChangedAll;
var
  I: Integer;
begin
  for I := 0 to FScenes.Count - 1 do
    FScenes[I].ChangedAll;
  ValidBoundingBox := false;
end;

function T3DPrecalculatedAnimation.InfoBoundingBox: string;
var
  BBox: TBox3D;
begin
  BBox := BoundingBox;
  Result := 'Bounding box (of the whole animation) : ' + BBox.ToNiceStr;
  if not BBox.IsEmpty then
  begin
    Result += ', average size : ' + FloatToNiceStr(BBox.AverageSize);
  end;
  Result += NL;
end;

function T3DPrecalculatedAnimation.Info(
  ATriangleVerticesCounts,
  ABoundingBox,
  AManifoldAndBorderEdges: boolean): string;
begin
  Result := '';

  if ATriangleVerticesCounts then
  begin
    Result += FirstScene.InfoTriangleVerticesCounts;
  end;

  if ABoundingBox then
  begin
    if Result <> '' then Result += NL;
    { We do not call FirstScene.InfoBoundingBox here, instead we want
      to get full bounding box of the animation. }
    Result += InfoBoundingBox;
  end;

  if AManifoldAndBorderEdges then
  begin
    if Result <> '' then Result += NL;
    Result += FirstScene.InfoManifoldAndBorderEdges;
  end;
end;

procedure T3DPrecalculatedAnimation.SetOwnsFirstRootNode(const Value: boolean);
var
  I: Integer;
begin
  if Value <> FOwnsFirstRootNode then
  begin
    FOwnsFirstRootNode := Value;
    if FScenes <> nil then
    begin
      Assert(FScenes.Count > 0, 'When the Scenes <> nil, anim is loaded so should always have at least one scene');

      { OwnsFirstRootNode corresponds to FScenes[0].OwnsRootNode.
        But note that it's allowed to have duplicates of scenes
        in one consecutive range of FScenes. So we have to iterate over
        the first FScenes (while they are equal to FScenes[0]). }

      for I := 0 to FScenes.Count - 1 do
      begin
        if FScenes[I] = FScenes[0] then
          FScenes[I].OwnsRootNode := Value else
          Break;
      end;
    end;
  end;
end;

function T3DPrecalculatedAnimation.KeyDown(Key: TKey; C: char): boolean;
begin
  if ScenesCount = 1 then
    Result := Scenes[0].KeyDown(Key, C) else
    Result := false;
end;

function T3DPrecalculatedAnimation.KeyUp(Key: TKey; C: char): boolean;
begin
  if ScenesCount = 1 then
    Result := Scenes[0].KeyUp(Key, C) else
    Result := false;
end;

function T3DPrecalculatedAnimation.MouseDown(const Button: TMouseButton): boolean;
begin
  if ScenesCount = 1 then
    Result := Scenes[0].MouseDown(Button) else
    Result := false;
end;

function T3DPrecalculatedAnimation.MouseUp(const Button: TMouseButton): boolean;
begin
  if ScenesCount = 1 then
    Result := Scenes[0].MouseUp(Button) else
    Result := false;
end;

function T3DPrecalculatedAnimation.MouseMove(const RayOrigin, RayDirection: TVector3Single;
  RayHit: T3DCollision): boolean;
begin
  if ScenesCount = 1 then
    Result := Scenes[0].MouseMove(RayOrigin, RayDirection, RayHit) else
    Result := false;
end;

procedure T3DPrecalculatedAnimation.ResetTimeAtLoad(const ForceTimeOrigin: boolean = false);

  function TimeOriginAtLoad: boolean;
  var
    N: TNavigationInfoNode;
  begin
    Result := false;

    if Loaded then
    begin
      N := Scenes[0].NavigationInfoStack.Top as TNavigationInfoNode;
      if (N <> nil) and
         (N is TKambiNavigationInfoNode) then
        Result := TKambiNavigationInfoNode(N).FdTimeOriginAtLoad.Value;
    end;
  end;

begin
  if (ScenesCount > 1) or ForceTimeOrigin or TimeOriginAtLoad then
  begin
    FTimeAtLoad := 0.0;
    ResetTime(TimeBegin);
  end else
  begin
    FTimeAtLoad := DateTimeToUnix(Now);
    ResetTime(TimeAtLoad);
  end;
end;

procedure T3DPrecalculatedAnimation.ResetTime(const NewValue: TFloatTime);
begin
  FTime := NewValue;

  { Ignored when SceneAnimation.ScenesCount <> 1, as scenes' ProcessEvents
    is always false then and Time wouldn't have much sense anyway. }
  if ScenesCount = 1 then
    Scenes[0].ResetTime(NewValue);
end;

procedure T3DPrecalculatedAnimation.Idle(const CompSpeed: Single);
var
  OldTime: TFloatTime;
begin
  inherited;

  { Ignore Idle calls when CompSpeed is precisely zero
    (this may happen, and is good, see TCastleWindowBase.IgnoreNextIdleSpeed).
    In this case, time increase will be zero so the whole code
    will not do anything anyway. }

  if Loaded and TimePlaying and (CompSpeed <> 0) then
  begin
    OldTime := FTime;
    FTime += TimePlayingSpeed * CompSpeed;

    { When ScenesCount = 1, it's sensible for single scene to receive
      events, to increase it's time. Note that T3DSceneCore.SetTime
      will signal when redisplay will be needed (something visible changed),
      we don't have to worry about it.

      We call Scenes[0].SetTime direcly, instead of calling Scenes[0].Idle.
      This way we do not have to worry to set scene's initial time, TimePlaying,
      TimePlayingSpeed to our values. }
    if ScenesCount = 1 then
      Scenes[0].SetTime(Time);

    { Call VisibleChangeHere only if the displayed animation frame actually changed.
      This way, we avoid wasting CPU cycles if the loaded scene is actually
      still, or if the animation stopped running. }
    if (SceneFromTime(OldTime) <>
        SceneFromTime(Time)) then
      VisibleChangeHere([vcVisibleGeometry, vcVisibleNonGeometry]);
  end;
end;

function T3DPrecalculatedAnimation.CurrentScene: T3DScene;
begin
  Result := SceneFromTime(Time);
end;

procedure T3DPrecalculatedAnimation.Render(const Frustum: TFrustum; const Params: TRenderParams);
begin
  if Loaded and Exists then
    CurrentScene.Render(Frustum, Params);
end;

procedure T3DPrecalculatedAnimation.RenderShadowVolume(
  ShadowVolumeRenderer: TBaseShadowVolumeRenderer;
  const ParentTransformIsIdentity: boolean;
  const ParentTransform: TMatrix4Single);
begin
  if Loaded and Exists and CastsShadow then
    CurrentScene.RenderShadowVolume(ShadowVolumeRenderer,
      ParentTransformIsIdentity, ParentTransform);
end;

procedure T3DPrecalculatedAnimation.GetHeightAbove(
  const Position, GravityUp: TVector3Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc;
  out IsAbove: boolean; out AboveHeight: Single;
  out AboveGround: P3DTriangle);

  procedure MakeScene(Scene: T3DSceneCore);
  var
    NewIsAbove: boolean;
    NewAboveHeight: Single;
    NewAboveGround: PTriangle;
  begin
    Scene.GetHeightAbove(
      Position, GravityUp, TrianglesToIgnoreFunc,
      NewIsAbove, NewAboveHeight, NewAboveGround);

    if NewAboveHeight < AboveHeight then
    begin
      IsAbove := NewIsAbove;
      AboveHeight := NewAboveHeight;
      AboveGround := NewAboveGround;
    end;
  end;

begin
  inherited;

  if Loaded and Exists and Collides then
  begin
    MakeScene(FirstScene);
    if CollisionUseLastScene then
      MakeScene(LastScene);
  end;
end;

function T3DPrecalculatedAnimation.MoveAllowed(
  const OldPos, ProposedNewPos: TVector3Single; out NewPos: TVector3Single;
  const CameraRadius: Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  if Loaded and Exists and Collides then
  begin
    Result := FirstScene.MoveAllowed(OldPos, ProposedNewPos, NewPos,
      CameraRadius, TrianglesToIgnoreFunc);

    { Call MoveAllowed on FirstScene, on the LastScene use only
      MoveAllowedSimple (no wall sliding).
      Reason: see T3DList.MoveAllowed implementation. }

    if Result and CollisionUseLastScene then
    begin
      Result := LastScene.MoveAllowedSimple(OldPos, NewPos,
        CameraRadius, TrianglesToIgnoreFunc);
    end;
  end else
  begin
    Result := true;
    NewPos := ProposedNewPos;
  end;
end;

function T3DPrecalculatedAnimation.MoveAllowedSimple(
  const OldPos, ProposedNewPos: TVector3Single;
  const CameraRadius: Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  Result := (not Loaded) or (not Exists) or (not Collides) or
    (FirstScene.MoveAllowedSimple(
       OldPos, ProposedNewPos,
       CameraRadius, TrianglesToIgnoreFunc) and
       ( (not CollisionUseLastScene) or
         LastScene.MoveAllowedSimple(
           OldPos, ProposedNewPos,
           CameraRadius, TrianglesToIgnoreFunc) ));
end;

function T3DPrecalculatedAnimation.MoveBoxAllowedSimple(
  const OldPos, ProposedNewPos: TVector3Single;
  const ProposedNewBox: TBox3D;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  Result := (not Loaded) or (not Exists) or (not Collides) or
    (FirstScene.MoveBoxAllowedSimple(OldPos, ProposedNewPos, ProposedNewBox,
       TrianglesToIgnoreFunc) and
       ( (not CollisionUseLastScene) or
         LastScene.MoveBoxAllowedSimple(OldPos, ProposedNewPos, ProposedNewBox,
           TrianglesToIgnoreFunc) ));
end;

function T3DPrecalculatedAnimation.SegmentCollision(const Pos1, Pos2: TVector3Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  Result := Loaded and Exists and Collides and
    ( FirstScene.SegmentCollision(Pos1, Pos2, TrianglesToIgnoreFunc) or
      (CollisionUseLastScene and
        (LastScene.SegmentCollision(Pos1, Pos2, TrianglesToIgnoreFunc)))
    );
end;

function T3DPrecalculatedAnimation.SphereCollision(
  const Pos: TVector3Single; const Radius: Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  Result := Loaded and Exists and Collides and
    ( FirstScene.SphereCollision(Pos, Radius, TrianglesToIgnoreFunc) or
      (CollisionUseLastScene and
        (LastScene.SphereCollision(Pos, Radius, TrianglesToIgnoreFunc)))
    );
end;

function T3DPrecalculatedAnimation.BoxCollision(
  const Box: TBox3D;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): boolean;
begin
  Result := Loaded and Exists and Collides and
    ( FirstScene.BoxCollision(Box, TrianglesToIgnoreFunc) or
      (CollisionUseLastScene and
        (LastScene.BoxCollision(Box, TrianglesToIgnoreFunc)))
    );
end;

function T3DPrecalculatedAnimation.RayCollision(
  out IntersectionDistance: Single;
  const Ray0, RayVector: TVector3Single;
  const TrianglesToIgnoreFunc: T3DTriangleIgnoreFunc): T3DCollision;
var
  NewIntersectionDistance: Single;
  NewResult: T3DCollision;
begin
  Result := nil;
  IntersectionDistance := 0; { Only to silence compiler warning }

  if Loaded and Exists and Collides then
  begin
    Result := FirstScene.RayCollision(IntersectionDistance,
      Ray0, RayVector, TrianglesToIgnoreFunc);

    if CollisionUseLastScene then
    begin
      { try the same thing on LastScene }
      NewResult := LastScene.RayCollision(NewIntersectionDistance,
        Ray0, RayVector, TrianglesToIgnoreFunc);

      if NewResult <> nil then
      begin
        if (Result = nil) or (NewIntersectionDistance < IntersectionDistance) then
        begin
          IntersectionDistance := NewIntersectionDistance;
          SysUtils.FreeAndNil(Result);
          Result := NewResult;
        end else
          FreeAndNil(NewResult);
      end;
    end;

    if Result <> nil then
      Result.Hierarchy.Insert(0, Self);
  end;
end;

procedure T3DPrecalculatedAnimation.UpdateGeneratedTextures(
  const RenderFunc: TRenderFromViewFunction;
  const ProjectionNear, ProjectionFar: Single;
  const OriginalViewportX, OriginalViewportY: LongInt;
  const OriginalViewportWidth, OriginalViewportHeight: Cardinal);
begin
  inherited;
  if Loaded then
    CurrentScene.UpdateGeneratedTextures(
      RenderFunc, ProjectionNear, ProjectionFar,
      OriginalViewportX, OriginalViewportY,
      OriginalViewportWidth, OriginalViewportHeight);
end;

procedure T3DPrecalculatedAnimation.VisibleChangeNotification(const Changes: TVisibleChanges);
begin
  inherited;
  if Loaded then
    CurrentScene.VisibleChangeNotification(Changes);
end;

function T3DPrecalculatedAnimation.Dragging: boolean;
begin
  Result := inherited;
  if Result then Exit;

  if Loaded then
    Result := CurrentScene.Dragging;
end;

procedure T3DPrecalculatedAnimation.SetShadowMaps(const Value: boolean);
var
  I: Integer;
begin
  if Value <> FShadowMaps then
  begin
    FShadowMaps := Value;
    if FScenes <> nil then
    begin
      for I := 0 to FScenes.Count - 1 do
        FScenes[I].ShadowMaps := Value;
    end;
  end;
end;

procedure T3DPrecalculatedAnimation.SetShadowMapsDefaultSize(const Value: Cardinal);
var
  I: Integer;
begin
  if Value <> FShadowMapsDefaultSize then
  begin
    FShadowMapsDefaultSize := Value;
    if FScenes <> nil then
    begin
      for I := 0 to FScenes.Count - 1 do
        FScenes[I].ShadowMapsDefaultSize := Value;
    end;
  end;
end;

end.