{
  Copyright 2001-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{$ifdef read_window_interface}
private
  glcontext: GLXContext;
  windowXID: TWindow;
  parentwinXID: TWindow; { refreshed on ReparentNotify, needed for X_Configureevent }
  XVisual: PXVisualInfo;
  colormap: TColormap;
  procedure X_KeyPress(const event: TXKeyEvent);
  procedure X_KeyRelease(const event: TXKeyEvent);
  procedure X_ConfigureNotify(const event: TXConfigureEvent);
  procedure X_ButtonPress(const event: TXButtonPressedEvent);
  procedure X_ButtonRelease(const event: TXButtonReleasedEvent);
  procedure X_MotionNotify(const event: TXMotionEvent);
  procedure X_FocusOut(const event: TXFocusOutEvent);

  procedure UpdateCursor;
{$endif read_window_interface}

{$ifdef read_application_interface}
private
  FXDisplayName: string;
  FXDisplay: PDisplay;
  FXScreen: Integer;
  InitializeXDisplayDone: boolean;
  XCursors: array [TMouseCursor] of TCursor;

  { uzywamy tego aby wylapac quit message. Inicjowane w SetXDisplayName }
  wmDeleteWindow: TAtom;
  { Arrays for fast conversion between X keysyms (cut off to least significant
    byte) to our TKey values. The most significant byte of X keysym determines
    which array to use. }
  KeySym0ToKey, KeySymFFToKey: TKeysBytes;
  { KeySymToKey; zamienia ksym na TKey, zwraca K_None jezeli
    nie dalo sie skonwertowac (bo moze ten keysym nie ma odpowiednika
    w naszym malym TKey ? a moze ksym byl = NoSymbol ?) }
  function KeySymToKey(ksym: TKeySym): TKey;
  { Seek on OpenWindows window with WindowXID = seekWindowXID.
    Returns nil if not found. }
  function FindWindowXID(seekWindowXID: TWindow): TCastleWindowBase;
  procedure SetXDisplayName(const Value: string);

  { Check XDisplay. Call this once you're sure that XDisplay is set to
    value desired by used now.

    And since we know now that XDisplay is OK, we initialize some
    things: XScreen, check for glx extension, get WM_DELETE_WINDOW
    (we do it actually only if this was run for the 1st time).
    These things actually have be done only once, after XDisplay selection.
    But note that this can't be called in SetXDisplayName automatically.

    So you should call this when you know that last SetXDisplayName was
    called --- good choice is before initializing your window. }
  procedure InitializeXDisplay;
public
  { XDisplayName uzywane przez wszystkie nastepne wywolania Window.Open.
    Application nie jest przygotowany na zarzadzanie oknami na wielu displayach i
    implementacja TCastleWindowBase zaklada ze po zrobieniu Open Application.XDisplay bedzie
    caly czas takie samo - wiec NIE ZMIENIAJ XDisplayName po zrobieniu jakiegos Open !
    XDisplayName bezposrednio po przypisaniu moze ulec zmianie - bo
    SetXDisplayName powoduje cos jak FXDisplayName := XDisplayName(FDiaplayName),
    a wiec X serwer moze cos od razu powiedziec o tym jak on rozumie ta
    XDisplayName.

    Default = '', zinterpretowane przez X serwer. }
  property XDisplayName: string read FXDisplayName write SetXDisplayName;

  property XDisplay: PDisplay read FXDisplay;

  property XScreen: Integer read FXScreen;
{$endif read_application_interface}

{$ifdef read_implementation}

{ Czyli implementacja CastleWindow za pomoca XWindows poprzez
  Xlib + glX.

  Punktem wyjscia byl znacznie przerobiony, rozszerzony, poprawiony prosty
  programik testgl jaki dostalem razem z modulem opengl12 pod Kylixa.
  Byl tam prosciutki schemat "openGL poprzez glX" i na nim sie opieralem
  na poczatku. Potem znaczna pomoca okazal sie glxdino w "OpenGL and X"
  by Mark Kilgard i Lesson1 z Nehe w wersji glX.
  No a wreszcie mam wszystko z "Xlib Programming Manual". }

const
  AcceptedEventsMask = ExposureMask or StructureNotifyMask or
    KeyPressMask or KeyReleaseMask {key events} or
    ButtonPressMask or ButtonReleaseMask or PointerMotionMask {mouse events} or
    FocusChangeMask;

{$I castlewindow_dialogs_by_messages.inc}

{ TCastleWindowBase --------------------------------------------------------  }

procedure TCastleWindowBase.OpenWinSystemDepend;
var
  sizeHints: TXSizeHints;
  AttrValueMask: Cardinal;
  attr: TXSetWindowAttributes;
  WindTitleProp: TXTextProperty;
  ClassHint: TXClassHint;
  WMHints: TXWMHints;
  PWindTitle: PChar;
  Attribs: TLongIntList;
  UseFBConfig: boolean;
  FBConfigs: PGLXFBConfig;
  FBConfig: TGLXFBConfig;
  FBConfigsCount: Integer;
const
  AllPointerEventsMask = ButtonPressMask or ButtonReleaseMask or
    EnterWindowMask or LeaveWindowMask or PointerMotionMask;

  { Used with glXCreateContextAttribsARB to select 3.0 forward-compatible context }
  Context30Forward: array [0..6] of Integer =
  ( GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
    GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    None
  );
begin
  Application.InitializeXDisplay;

  { Use new glX 1.3 functions to create context, that take TGLXFBConfig?
    About modern GLX context selection, see
    http://www.opengl.org/wiki/Creating_an_OpenGL_Context and
    http://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_%28GLX%29 }
  UseFBConfig := GLX_version_1_3(Application.XDisplay);

  Attribs := TLongIntList.Create;
  try
    { constant Attribs part }
    if UseFBConfig then
      Attribs.AddArray([GLX_X_RENDERABLE, 1 { true }]) else
      Attribs.Add(GLX_RGBA);

    if DoubleBuffer then
      if UseFBConfig then
        Attribs.AddArray([GLX_DOUBLEBUFFER, 1]) else
        Attribs.Add(GLX_DOUBLEBUFFER);

    Attribs.AddArray([
      { Buffer sizes below are all in bits. }
      GLX_DEPTH_SIZE, DepthBufferBits,
      GLX_STENCIL_SIZE, StencilBufferBits,
      GLX_ALPHA_SIZE, AlphaBits,
      GLX_ACCUM_RED_SIZE, AccumBufferBits[0],
      GLX_ACCUM_GREEN_SIZE, AccumBufferBits[1],
      GLX_ACCUM_BLUE_SIZE, AccumBufferBits[2],
      GLX_ACCUM_ALPHA_SIZE, AccumBufferBits[3] ]);

    if MultiSampling > 1 then
    begin
      if GLX_ARB_multisample(Application.XDisplay, Application.XScreen) then
      begin
        Attribs.AddArray([
          GLX_SAMPLE_BUFFERS_ARB, 1,
          GLX_SAMPLES_ARB, MultiSampling ]);
        if Log then
          WritelnLog('MultiSampling', 'GLX_ARB_multisample supported, using multisampling');
      end else
        raise EGLContextNotPossible.CreateFmt('Multisampling (%d samples) ' +
          'requested, but GLX_ARB_multisample not supported on this screen',
          [MultiSampling]);
    end;

    { end of Attribs array }
    Attribs.Add(None);

    if UseFBConfig then
    begin
      FBConfigs := glXChooseFBConfig(Application.XDisplay, Application.XScreen,
        PInteger(Attribs.List), FBConfigsCount);
      if FBConfigsCount = 0 then
        raise EGLContextNotPossible.CreateFmt('No frame buffer configurations that match the specified attributes (%s)',
          [RequestedBufferAttributes]);
      { just choose the first FB config from the FBConfigs list.
        More involved selection possible. }
      FBConfig := FBConfigs^;
      XVisual := glXGetVisualFromFBConfig(Application.XDisplay, FBConfig);
    end else
      XVisual := glXChooseVisual(Application.XDisplay, Application.XScreen, PInteger(Attribs.List));
  finally FreeAndNil(Attribs) end;

  if XVisual = nil then
    raise EGLContextNotPossible.CreateFmt(
      'X visual with requested attributes (%s) not found',
      [ RequestedBufferAttributes ]);
  { Application.XScreen should always be XVisual.screen now }

  parentwinXID := XRootWindow(Application.XDisplay, Application.XScreen);

  colormap := XCreateColormap(Application.XDisplay, parentwinXID,
                              XVisual^.visual, AllocNone);
  { see glxdino example for more involved way to get colormap }
  attr.colormap := colormap;
  attr.border_pixel := 0;
  attr.event_mask := AcceptedEventsMask;
  AttrValueMask := CWBorderPixel or CWColormap or CWEventMask;

  if fullscreen then
  begin
    attr.override_redirect := XBool_true;
    AttrValueMask := AttrValueMask or CWOverrideRedirect;
  end;

  windowXID := XCreateWindow(Application.XDisplay, parentwinXID,
    left, top, width, height,
    0, XVisual^.depth, InputOutput, XVisual^.visual,
    AttrValueMask, @attr);

  if FFullscreen and Visible then
  begin
    { XMapWindow must be called before doing all the "grab" operations.
      This also means that this whole piece of code cannot be done
      when Visible = @false. }
    XMapWindow(Application.XDisplay, windowXID);
    check( XGrabKeyboard(Application.XDisplay, windowXID, XBool_True, GrabModeAsync, GrabModeAsync,
       CurrentTime) = GrabSuccess, 'cannot grab the keyboard');
    check( XGrabPointer(Application.XDisplay, windowXID, XBool_True,
       AcceptedEventsMask and AllPointerEventsMask,
       GrabModeAsync, GrabModeAsync, windowXID, None, CurrentTime) = GrabSuccess,
       'cannot grab the pointer');

    { W fullscreen nie realizujemy sizeHints (ktore zapewniaja realizacje
      funkcji ResizeAllowed <> raAllowed). ALE w fullscreen nigdy okno nie moze
      byc resized (wiec nie potrzeba nam sizeHints zeby to powiedziec...)
      wiec ok.

      Podobnie, w fullscreen nie realizujemy wmDeleteWindow ktore zapewnia
      realizacje uslugi OnCloseQuery. ALE w fullscreen user nie moze
      powiedziec window-managerowi ze chce zamknac to okno skoro window
      manager niemal nie wie o istnieniu naszego okna (grabbed klawisze,
      mysz, nasze okno bez ramki) wiec ok. }
    { TODO: kiedy juz zrobisz fullscreen bez Grabow i override_redirecta
      to chyba WM_DELETE_WINDOW pojdzie do fullscreen ?}
    { TODO: zrobic fullscreen bez Grabow (i ew. override_redirect) }
  end else
  begin
    { only set window title and handle wm_delete_events if in windowed mode }
    XSetWMProtocols(Application.XDisplay, windowXID, @Application.wmDeleteWindow, 1);

    { ready sizeHints structure }
    sizeHints.x := left;
    sizeHints.y := top;
    sizeHints.width := width;
    sizeHints.height := height;
    sizeHints.flags := USSize or USPosition;
    if ResizeAllowed = raAllowed then
    begin
      sizeHints.min_width := minWidth;
      sizeHints.min_height := minHeight;
      sizeHints.max_width := maxWidth;
      sizeHints.max_height := maxHeight;
    end else
    begin
      sizeHints.min_width := width;
      sizeHints.min_height := height;
      sizeHints.max_width := width;
      sizeHints.max_height := height;
    end;
    sizeHints.flags := sizeHints.flags or PMinSize or PMaxSize;

    { ready WMHints, ClassHints }
    WMHints.flags := InputHint;
    WMHints.input := XBool_True;
    ClassHint.res_name := StrNew(PChar(ProgramName));
    ClassHint.res_class := StrNew(PChar(ProgramName));

    { ready WindTitle }
    PWindTitle := PChar(FCaption);
    Check( XStringListToTextProperty(@PWindTitle, 1, @WindTitleProp) <> 0, 'not enough memory for XStringListToTextProperty');

    { use prepared WindTitleProp, Size/WM/ClassHints to give a lot of
      hints to WindowManager (that will hopefully honour them...) }
    XSetWMProperties_Pascal(Application.XDisplay, windowXID, @WindTitleProp, @WindTitleProp,
       @sizeHints, @WMHints, @ClassHint);

    { realease allocated memory for structs for XSetWMProperties }
    XFree(WindTitleProp.value);
    StrDispose(ClassHint.res_name);
    StrDispose(ClassHint.res_class);

    if Visible then
      XMapWindow(Application.XDisplay, windowXID);
  end;

  if UseFBConfig then
  begin
    { For now, we just use GLX_ARB_create_context for testing
      "how much can we render with forward-compatible context".
      TODO:
      - The ability to change Context30Forward version is not available.
        Or not request compatibility, or debug, etc.
      - We also don't properly catch errors: we should catch X errors
        (see XSync and XSetErrorHandler inside
        http://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_%28GLX%29).
        Right now, our XLibUtils catches X errors, but without XSync
        it's not known where they occur anyway. }
//    if GLX_ARB_create_context(Application.XDisplay, Application.XScreen) then
//      glcontext := glXCreateContextAttribsARB(Application.XDisplay, FBConfig, nil, true, Context30Forward) else
      glcontext := glXCreateNewContext(Application.XDisplay, FBConfig, GLX_RGBA_TYPE, nil, true);
  end else
    glcontext := glXCreateContext(Application.XDisplay, XVisual, nil, true);

  Check(glcontext <> nil, 'Could not create rendering context');

  UpdateCursor;

  Application.OpenWindowsAdd(Self);

  MakeCurrent;
end;

procedure TCastleWindowBase.CloseBackend;
var
  dummy_event: TXEvent;
begin
  if glcontext <> nil then
  begin
    glXDestroyContext(Application.XDisplay, glcontext);
    glcontext := nil;
  end;

  if FFullscreen and Visible then
  begin
    XUnGrabPointer(Application.XDisplay, CurrentTime);
    XUnGrabKeyboard(Application.XDisplay, CurrentTime);
  end;

  { czyscimy kolejke komunikatow dla tego okienka.
    To nie daje nam absolutnej gwarancji ze do naszej kolejki nie trafia
    nigdy komunikaty juz zamknietych okienek bo przeciez messsage queue
    jest wypelniana w innym procesie wiec my mozemy wyczyscic a tu
    zaraz potem cos nam wpadnie. Wiec i tak w Application.ProcessMessage
    musimy sprawdzic czy dostalismy message dla istniejacego okienka.
    ALE ponizsze moze nam zaoszczedzic sporo czasu w ProcessMessage
    bo mimo wszystko zapewne zazwyczaj oczywsci kolejke z wiekszosci
    smieci dla tego okienka. }
  while XBool(XCheckWindowEvent(
    Application.XDisplay, windowXID, Longint($FFFFFFFF), @dummy_event)) <> XBool_False do ;

  if windowXID <> None then begin XDestroyWindow(Application.XDisplay, windowXID); windowXID := None end;
  if colormap  <> None then begin XFreeColormap(Application.XDisplay, colormap);   colormap := None  end;
  if XVisual   <> nil  then begin XFree(XVisual);                                  XVisual := nil    end;
end;

procedure TCastleWindowBase.SetCursor(const Value: TMouseCursor);
begin
  if FCursor <> Value then
  begin
    FCursor := Value;
    if not Closed then
      UpdateCursor;
  end;
end;

procedure TCastleWindowBase.SetCustomCursor(const Value: TRGBAlphaImage);
begin
  FCustomCursor := Value;
  { TODO }
end;

procedure TCastleWindowBase.UpdateCursor;

  function CreateNoneCursor: TCursor;
  var
    bm_no: TPixmap;
    cmap: TColormap;
    black, dummy: TXColor;
  const
    bm_no_data: array[0..7] of Byte = (0, 0, 0, 0, 0, 0, 0, 0);
  begin
    { Based on [http://www.linuxforums.org/forum/linux-programming-scripting/59012-xlib-hide-mouse-pointer.html] }
    cmap := DefaultColormap(Application.XDisplay, DefaultScreen(Application.XDisplay));
    XAllocNamedColor(Application.XDisplay, cmap, 'black', @black, @dummy);
    try
      bm_no := XCreateBitmapFromData(Application.XDisplay, WindowXID, @bm_no_data, 8, 8);
      try
        Result := XCreatePixmapCursor(Application.XDisplay, bm_no, bm_no, @black, @black, 0, 0);
      finally
        if bm_no <> None then
          XFreePixmap(Application.XDisplay, bm_no);
      end;
    finally
      XFreeColors(Application.XDisplay, cmap, @black.pixel, 1, 0);
    end;
  end;

const
  XCursorShapeFromMy: array [mcStandard .. High(TMouseCursor)] of LongWord =
  ( XC_left_ptr, XC_watch, XC_xterm, XC_hand2 );
begin
  { TODO: for now mcCustom is treated like mcDefault }
  if (Cursor = mcDefault) or (Cursor = mcCustom) then
  begin
    XUndefineCursor(Application.XDisplay, WindowXID);
  end else
  begin
    if Application.XCursors[Cursor] = X.None then
    begin
      { initialize XCursors[Cursor] }
      if Cursor = mcNone then
        Application.XCursors[Cursor] := CreateNoneCursor else
        Application.XCursors[Cursor] := XCreateFontCursor(Application.XDisplay,
          XCursorShapeFromMy[Cursor]);
    end;

    XDefineCursor(Application.XDisplay, WindowXID, Application.XCursors[Cursor]);
  end;
end;

procedure TCastleWindowBase.SetMousePosition(const NewMouseX, NewMouseY: Integer);
begin
  if not Closed then
    XWarpPointer(Application.XDisplay, X.None, WindowXID,
      0, 0, 0, 0, NewMouseX, NewMouseY);
end;

procedure TCastleWindowBase.BackendMakeCurrent;
begin
  Assert(not Closed);
  Check( glXMakeCurrent(Application.XDisplay, windowXID, glcontext), 'glXMakeCurrent');
end;

procedure TCastleWindowBase.CreateBackend;
begin
  windowXID := None;
end;

procedure TCastleWindowBase.SetCaption(const Value: string);
var
  PValue: PChar;
  WindTitleProp: TXTextProperty;
begin
  fCaption := Value;

  if windowXID <> None then
  begin
    { create, use, free WindTitleProp }
    PValue := PChar(Value);
    Check( XStringListToTextProperty(@PValue, 1, @WindTitleProp) <> 0,
      'not enough memory for XStringListToTextProperty');
    XSetTextProperty(Application.XDisplay, windowXID, @WindTitleProp, XInternAtom(Application.XDisplay, 'WM_NAME', XBool_True) );
    XFree(WindTitleProp.value);
  end;
end;

function TCastleWindowBase.RedirectKeyDownToMenuCommand: boolean;
begin
 Result := true;
end;

procedure TCastleWindowBase.BackendMenuInitialize;
begin
  { does nothing }
end;

procedure TCastleWindowBase.BackendMenuFinalize;
begin
  { does nothing }
end;

procedure TCastleWindowBase.MenuUpdateCaption(Entry: TMenuEntryWithCaption);
begin
  { does nothing }
end;

procedure TCastleWindowBase.MenuUpdateEnabled(Entry: TMenuEntryWithCaption);
begin
  { does nothing }
end;

procedure TCastleWindowBase.MenuUpdateChecked(Entry: TMenuItemChecked);
begin
  { does nothing }
end;

function TCastleWindowBase.MenuUpdateCheckedFast: boolean;
begin
  Result := false;
end;

procedure TCastleWindowBase.SwapBuffers;
begin
  glXSwapBuffers(Application.XDisplay, windowXID);
end;

procedure TCastleWindowBase.X_KeyPress(const event: TXKeyEvent);
var
  k: TKeySym;
  CharKey: char;
  Key: TKey;
begin
  { calculate k, CharKey }
  if XLookupString(@event, @CharKey, 1, @k, nil) = 0 then
    CharKey := #0;

  { maybe do DoKeyDown }
  case k of
    XK_Shift_L:   SetPrivateModifiersDown(mkShift, false, true);
    XK_Shift_R:   SetPrivateModifiersDown(mkShift, true,  true);
    XK_Control_L: SetPrivateModifiersDown(mkCtrl,  false, true);
    XK_Control_R: SetPrivateModifiersDown(mkCtrl,  true,  true);
    XK_Alt_L:     SetPrivateModifiersDown(mkAlt,   false, true);
    XK_Alt_R:     SetPrivateModifiersDown(mkAlt,   true,  true);
    else begin
      Key := Application.KeySymToKey(k);
      if (Key <> K_None) or (CharKey <> #0) then DoKeyDown(Key, CharKey);
    end;
  end;
end;

procedure TCastleWindowBase.X_KeyRelease(const event: TXKeyEvent);
var
  k: TKeySym;
  CharKey: char;
  Key: TKey;
begin
  { Note: "XLib Progr Manual" warns that KeyRelease is not guaranteed on
    "some very old systems". I didn't ever see such system...
    Anyway, in such case we can't make DoKeyUp, OnKeyUp, all Keys will remain
    pressed forever until window unfocused... }

  { calculate k, CharKey }
  XLookupString(@event, @CharKey, 1, @k, nil);

  { maybe do DoKeyUp }
  case k of
    XK_Shift_L:   SetPrivateModifiersDown(mkShift, false, false);
    XK_Shift_R:   SetPrivateModifiersDown(mkShift, true,  false);
    XK_Control_L: SetPrivateModifiersDown(mkCtrl,  false, false);
    XK_Control_R: SetPrivateModifiersDown(mkCtrl,  true,  false);
    XK_Alt_L:     SetPrivateModifiersDown(mkAlt,   false, false);
    XK_Alt_R:     SetPrivateModifiersDown(mkAlt,   true,  false);
    else begin
      Key := Application.KeySymToKey(k);
      if Key <> K_None then DoKeyUp(Key);
    end;
  end;
end;

procedure TCastleWindowBase.X_ConfigureNotify(const event: TXConfigureEvent);
var
  dummy_win: TWindow;
begin
  { event.x , event.y are relative to our parent.
    We always create our window giving XRootWindow(Application.XDisplay, Application.XScreen)
    as parent and we want our fLeft, fTop to be relitive to that
    window. BUT window managers may redirect our window and put
    it in some thick border, for example. Than this border is our parent
    and our position is given relative to that border. But we want it
    to be relative to XRootWindow(...) to be useful for future window
    creation !! So we translate coords from our parentwinXID to
    XRootWindow(...) }
  XTranslateCoordinates(Application.XDisplay,
    parentwinXID, XRootWindow(Application.XDisplay, Application.XScreen),
    {event.x, event.y,}0, 0, @fLeft, @fTop, @dummy_win);

  { TODO: for now, Left and Top indicate corner of the window *with frame*
    (while they should indicate corner of GL area only, according to interface). }
  DoResize(event.width, event.height, false);
end;

function xbtnToMouseButton(button: Cardinal; var mbtn: TMouseButton): boolean;
begin
  result := true;
  case button of
    Button1 : mbtn := mbLeft;
    Button2 : mbtn := mbMiddle;
    Button3 : mbtn := mbRight;
    else result := false;
  end;
end;

procedure TCastleWindowBase.X_ButtonPress(const event: TXButtonPressedEvent);
var
  btn: TMouseButton;
begin
  if xbtnToMouseButton(event.button, btn) then
    DoMouseDown(event.x, event.y, btn) else
  case Event.Button of
    Button4: DoMouseWheel( 1, true);
    Button5: DoMouseWheel(-1, true);
    6: DoMouseWheel( 1, false); { not sure if this is possible }
    7: DoMouseWheel(-1, false); { not sure if this is possible }
  end;
end;

procedure TCastleWindowBase.X_ButtonRelease(const event: TXButtonReleasedEvent);
var
  btn: TMouseButton;
begin
  if xbtnToMouseButton(event.button, btn) then
    DoMouseUp(event.x, event.y, btn)
end;

procedure TCastleWindowBase.X_MotionNotify(const event: TXMotionEvent);
begin
  { We could refresh MousePressed now. But no need for now?
  mousePressed:=[];
  if (Button1Mask and event.state) <> 0 then Include(mousePressed, mbLeft);
  if (Button2Mask and event.state) <> 0 then Include(mousePressed, mbMiddle);
  if (Button3Mask and event.state) <> 0 then Include(mousePressed, mbRight);
  }
  DoMouseMove(event.x, event.y);
end;

procedure TCastleWindowBase.X_FocusOut(const event: TXFocusOutEvent);
begin
  { Clear KeysDown and MousePressed information.
    Otherwise, we could miss the keyup or mouseup event, if user pressed
    the key/mouse with our window in focus, then (holding the key/mouse)
    switched to other window. }
  ReleaseAllKeysAndMouse;
end;

{ TGLApplication ------------------------------------------------------- }

function TGLApplication.FindWindowXID(seekWindowXID: TWindow): TCastleWindowBase;
var
  i: integer;
begin
  for i := 0 to OpenWindowsCount-1 do
    if OpenWindows[i].WindowXID = seekWindowXID then
      Exit(OpenWindows[i]);
  result := nil;
end;

procedure TGLApplication.SetXDisplayName(const Value: string);
begin
  if XDisplay <> nil then XCloseDisplay(XDisplay);

  FXDisplayName := Xlib.XDisplayName(PCharOrNil(Value));

  FXDisplay := XOpenDisplay(PCharOrNil(Value));

  { Tests: Writeln('Setting XDisplayName to "', Value, '" success: ', FXDisplay <> nil); }

  { I used to check this, if XDisplay = nil then raise ECheckFailed....;
    but actually that's bad: at the beginning we call SetXDisplayName('')
    to set to default display, and that doesn't have to succeed since
    a default display may
    not be available (e.g. you run from a Linux console, or on Mac OS X
    not from X11 xterm) --- then you have to specify display name, the default
    display name '' will not be OK.

    So the check for XDisplay = nil will be done at window initialization. }
end;

function TGLApplication.KeySymToKey(KSym: TKeySym): TKey;
begin
  result := K_None;
  if ksym <> NoSymbol then
    case ksym and $FF00 of
      { Note that KeySym*ToKey values may be K_None, and so we may return K_None. }
      $0000: Exit(KeySym0ToKey [ksym and $FF]);
      $FF00: Exit(KeySymFFToKey[ksym and $FF]);
   end;
end;

procedure TGLApplication.InitializeXDisplay;
{$ifdef DARWIN}
const
  AltXDisplayName = ':0';
{$endif}
var
  GlxExtensions: string;
begin
  if XDisplay = nil then
  begin
    {$ifdef DARWIN}
    Writeln(ErrOutput, Format('Opening the default X display "%s" failed, retrying with "%s" to try to attach to running X server on Mac OS X.',
      [XDisplayName, AltXDisplayName]));
    XDisplayName := AltXDisplayName;
    if XDisplay = nil then
    {$endif}

    raise Exception.CreateFmt('XOpenDisplay could not open display "%s"', [XDisplayName]);
  end;

  if not InitializeXDisplayDone then
  begin
    FXScreen := XDefaultScreen(XDisplay);

    if not GLX_version_1_0(XDisplay) then
      raise Exception.Create('glX extension (version at least 1.0) not found (necessary for OpenGL-based programs)') else
    if Log then
    begin
      GlxExtensions := glXQueryExtensionsString(XDisplay, FXScreen);
      WritelnLogMultiline('GLX', 'GLX extension at least 1.0 found.' +NL+
        'Versions (determined by checking both glXQueryExtension, glXQueryVersion and assigned entry points):' +NL+
        Format('  Version 1.1: %s', [CastleStringUtils.BoolToStr[GLX_version_1_1(XDisplay)]]) +NL+
        Format('  Version 1.2: %s', [CastleStringUtils.BoolToStr[GLX_version_1_2(XDisplay)]]) +NL+
        Format('  Version 1.3: %s', [CastleStringUtils.BoolToStr[GLX_version_1_3(XDisplay)]]) +NL+
        Format('  Version 1.4: %s', [CastleStringUtils.BoolToStr[GLX_version_1_4(XDisplay)]]) +NL+
        NL+
        'Important extensions (determined by checking glXQueryExtensionsString and assigned entry points):' +NL+
        Format('  GLX_ARB_multisample: %s'   , [CastleStringUtils.BoolToStr[GLX_ARB_multisample(XDisplay, FXScreen)]]) + NL+
        Format('  GLX_ARB_create_context: %s', [CastleStringUtils.BoolToStr[GLX_ARB_create_context(XDisplay, FXScreen)]]) +NL+
        NL+
        'All extensions (according to glXQueryExtensionsString):' +NL+
        GlxExtensions);
    end;

    wmDeleteWindow := XInternAtom(XDisplay, 'WM_DELETE_WINDOW', XBool_True);

    InitializeXDisplayDone := true;
  end;
end;

procedure TGLApplication.CreateBackend;
const
  { Under Kylix, capital letters are XK_?  and lower are XK_l?.
    Under FPC,           ...     are XKc_?    ...    are XK_?. }
  XK_a_lower = {$ifdef DELPHI} XK_la {$endif} {$ifdef FPC} XK_a {$endif};
  XK_z_lower = {$ifdef DELPHI} XK_lz {$endif} {$ifdef FPC} XK_z {$endif};
  XK_a_upper = {$ifdef DELPHI} XK_a {$endif} {$ifdef FPC} XKc_a {$endif};
  XK_z_upper = {$ifdef DELPHI} XK_z {$endif} {$ifdef FPC} XKc_z {$endif};
var
  b: byte;
begin
  SetXDisplayName(''); { zainicjuj XDisplayName }

  { See ~/sources/fpc/trunk/packages/x11/src/keysym.pp }

  { initialize KeySym*ToKey }
  for b := Low(b) to High(b) do KeySym0ToKey[b] := K_None;
  for b := Low(b) to High(b) do KeySymFFToKey[b] := K_None;

  KeySymFFToKey[XK_Page_Up and $FF] := K_PageUp;
  KeySymFFToKey[XK_Page_Down and $FF] := K_PageDown;
  KeySymFFToKey[XK_Home and $FF] := K_Home;
  KeySymFFToKey[XK_End and $FF] := K_End;
  KeySymFFToKey[XK_Left and $FF] := K_Left;
  KeySymFFToKey[XK_Right and $FF] := K_Right;
  KeySymFFToKey[XK_Up and $FF] := K_Up;
  KeySymFFToKey[XK_Down and $FF] := K_Down;
  KeySymFFToKey[XK_Delete and $FF] := K_Delete;
  KeySymFFToKey[XK_Insert and $FF] := K_Insert;
  KeySymFFToKey[XK_BackSpace and $FF] := K_BackSpace;
  KeySymFFToKey[XK_Tab and $FF] := K_Tab;
  KeySymFFToKey[XK_KP_Add and $FF] := K_Numpad_Plus;
  KeySymFFToKey[XK_KP_Subtract and $FF] := K_Numpad_Minus;
  KeySymFFToKey[XK_Escape and $FF] := K_Escape;
  KeySymFFToKey[XK_Return and $FF] := K_Enter;
  KeySymFFToKey[XK_Print and $FF] := K_PrintScreen;
  KeySymFFToKey[XK_Caps_Lock and $FF] := K_CapsLock;
  KeySymFFToKey[XK_Scroll_Lock and $FF] := K_ScrollLock;
  KeySymFFToKey[XK_Num_Lock and $FF] := K_NumLock;
  KeySymFFToKey[XK_Pause and $FF] := K_Pause;

  KeySymFFToKey[XK_KP_0 and $FF] := K_Numpad_0;
  KeySymFFToKey[XK_KP_1 and $FF] := K_Numpad_1;
  KeySymFFToKey[XK_KP_2 and $FF] := K_Numpad_2;
  KeySymFFToKey[XK_KP_3 and $FF] := K_Numpad_3;
  KeySymFFToKey[XK_KP_4 and $FF] := K_Numpad_4;
  KeySymFFToKey[XK_KP_5 and $FF] := K_Numpad_5;
  KeySymFFToKey[XK_KP_6 and $FF] := K_Numpad_6;
  KeySymFFToKey[XK_KP_7 and $FF] := K_Numpad_7;
  KeySymFFToKey[XK_KP_8 and $FF] := K_Numpad_8;
  KeySymFFToKey[XK_KP_9 and $FF] := K_Numpad_9;
  KeySymFFToKey[XK_KP_End and $FF] := K_Numpad_End;
  KeySymFFToKey[XK_KP_Down and $FF] := K_Numpad_Down;
  KeySymFFToKey[XK_KP_Next and $FF] := K_Numpad_PageDown;
  KeySymFFToKey[XK_KP_Left and $FF] := K_Numpad_Left;
  KeySymFFToKey[XK_KP_Begin and $FF] := K_Numpad_Begin;
  KeySymFFToKey[XK_KP_Right and $FF] := K_Numpad_Right;
  KeySymFFToKey[XK_KP_Home and $FF] := K_Numpad_Home;
  KeySymFFToKey[XK_KP_Up and $FF] := K_Numpad_Up;
  KeySymFFToKey[XK_KP_Prior and $FF] := K_Numpad_PageUp;
  KeySymFFToKey[XK_KP_Insert and $FF] := K_Numpad_Insert;
  KeySymFFToKey[XK_KP_Delete and $FF] := K_Numpad_Delete;
  KeySymFFToKey[XK_KP_Enter and $FF] := K_Numpad_Enter;
  KeySymFFToKey[XK_KP_Multiply and $FF] := K_Numpad_Multiply;
  KeySymFFToKey[XK_KP_Divide and $FF] := K_Numpad_Divide;

  for b := 0 to 11 do KeySymFFToKey[(XK_F1+b) and $FF] := K_F1+b;

  for b := XK_0 to XK_9 do KeySym0ToKey[b] := ord(b);
  for b := XK_a_lower to XK_z_lower do KeySym0ToKey[b] := ord(UpCase(chr(b)));
  for b := XK_a_upper to XK_z_upper do KeySym0ToKey[b] := b;
  KeySym0ToKey[XK_Space] := K_Space;
  KeySym0ToKey[XK_comma] := K_comma;
  KeySym0ToKey[XK_period] := K_period;
  KeySym0ToKey[XK_bracketleft] := K_LeftBracket;
  KeySym0ToKey[XK_bracketright] := K_RightBracket;
  KeySym0ToKey[XK_apostrophe] := K_Apostrophe;
  KeySym0ToKey[XK_semicolon] := K_Semicolon;
  KeySym0ToKey[XK_slash] := K_Slash;
  KeySym0ToKey[XK_grave] := K_BackQuote;
  KeySym0ToKey[XK_minus] := K_Minus;
  KeySym0ToKey[XK_plus] := K_Plus;
  KeySym0ToKey[XK_equal] := K_Equal;
  KeySym0ToKey[XK_backslash] := K_BackSlash;
end;

procedure TGLApplication.DestroyBackend;
var
  C: TMouseCursor;
begin
  if XDisplay <> nil then
  begin
    for C := Low(C) to High(C) do
      if XCursors[C] <> X.None then
      begin
        XFreeCursor(XDisplay, XCursors[C]);
        XCursors[C] := X.None;
      end;

    XCloseDisplay(XDisplay);
  end;
end;

function TGLApplication.BackendName: string;
begin
  Result := 'Xlib';
end;

{$endif read_implementation}