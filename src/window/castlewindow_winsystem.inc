{
  Copyright 2001-2014 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

{ This is the common code for CASTLE_WINDOW_XLIB and CASTLE_WINDOW_WINAPI implementations. }

{$ifdef read_window_interface}
{$endif read_window_interface}

{$ifdef read_application_interface}
private
  LastDoTimerTime: TMilisecTime;

  { Same as ProcessMessage(WaitForMessage, WaitToLimitFPS),
    but additionally returns under WasAnyMessage was there any message
    processed.

    This ProcessMessage2 is useful to implement ProcessAllMessages.
    ActuallyProcessMessage is implemented using ProcessMessage2
    (and simply ignoring WasAnyMessage). }
  function ProcessMessage2(WaitForMessage, WaitToLimitFPS: boolean;
    var WasAnyMessage: boolean): boolean;
{$endif read_application_interface}

{$ifdef read_implementation}

{ common things for winsystem CastleWindow implementations.
  Uwagi :
   - zawsze musimy w jakis sposob sprawdzic czy okienko dla ktorego
     dostalismy message jest aktualnie na liscie Application.OpenWindows.
     Pamietajmy bowiem ze do naszej kolejki komunikatow mogly
     trafic message'y dla okienek ktore juz zamknelismy - typowym
     przykladem jest gdy zamykamy okienko w reakcji na jakies OnPress.
     Przeciez w kolejce komunikatow moze nam wtedy zostac OnRelease
     (wyslany przez WindowManagera kiedy mysmy robili cos innego.
     User szybko zrobil key down, potem up, dopiero potem my zauwazylismy
     ze dostalismy KeyDown, zrobilismy sobie Close (moze nawet
     zwolnilismy z pamieci obiekt TCastleWindowCustom !!) i potem wywolujemy
     ProcessMessage (bo np. sa jeszcze jakies inne okienka otwarte)
     i dostajemy messaga KeyUp dla okienka ktore juz jest Closed
     albo moze nawet ktorego juz nie ma w pamieci !!

     Dotyczy to zarowno WinAPI jak i Xlib.
}

{ TCastleApplication ---------------------------------------------------------- }

function TCastleApplication.ProcessAllMessages: boolean;
var
  WasAnyMessage: boolean;
  {$ifdef CASTLE_ENGINE_PLUGIN}
  I: Integer;
  {$endif}
begin
  repeat
    Result := ProcessMessage2(false, false, WasAnyMessage);
    if (not Result) or (not WasAnyMessage) then break;
  until false;

  {$ifdef CASTLE_ENGINE_PLUGIN}
  // TODO: why is this needed?
  for I := 0 to OpenWindowsCount - 1 do
    OpenWindows[I].Invalidate;
  {$endif CASTLE_ENGINE_PLUGIN}
end;

function TCastleApplication.ProcessMessage(WaitForMessage, WaitToLimitFPS: boolean): boolean;
var
  Dummy: boolean;
begin
  Result := ProcessMessage2(WaitForMessage, WaitToLimitFPS, Dummy);
end;

function TCastleApplication.ProcessMessage2(WaitForMessage, WaitToLimitFPS: boolean;
  var WasAnyMessage: boolean): boolean;

  { CheckMessage sprawdza czy jest jakis message w naszej queue,
      jesli tak - laduje go do msg/event i wywoluje HandleEvent i zwraca true,
      jesli nie - zwraca false.
    WaitMessage czeka az nasza kolejka bedzie niepusta i
      wtedy laduje message do msg/event i wywoluje HandleEvent.
      W rezultacie WaitMessage ZAWSZE przetworzy jakiegos messaga -
      dziala jak CheckMessage ktore uparlo sie zeby zwrocic true.
    NoMessageProcessed - wywolanie tej proc. oznacza ze w tym
      wywolaniu ProcessMessage CheckMessage zwrocilo false
      i z jakichs zewnetrznych powodow nie mozemy wywolac WaitMessage.
      Wobec tego nie przetworzylismy zadnego messaga - moze implementacja wymaga
      by cos zrobic w tej sytuacji ? (np. pod X-ami nalezy wowczas
      zrobic XFlush).

    Pod X-ami robimy XFlush w NoMessageProcessed i jest implicit XFlush
      w WaitMessage wiec zawsze w wywolaniu ProcessMessage zrobimy jakos
      XFlush - co jest wazne.
  }

  {$ifdef CASTLE_WINDOW_WINAPI}  var msg: TMsg;  {$endif}
  {$ifdef CASTLE_WINDOW_XLIB}    var event: TXEvent;  {$endif}

  procedure HandleEvent;
  begin
    WasAnyMessage := true;

    {$ifdef CASTLE_WINDOW_WINAPI}
    if msg.message = WM_QUIT then
      Terminate { set Terminated := true } else
    begin
      TranslateMessage(msg);
      DispatchMessage(msg);
      {$ifdef CASTLE_ENGINE_PLUGIN}
      { In case of plugin, we have to take care of calling TryHandlingMessage
        (which calls TCastleWindowCustom.WindowProc) ourselves.
        In other cases, it's called by WndProc which is automatically called
        by DispatchMessage because our window is connected with our window class.
        See DispatchMessage docs on:
        https://msdn.microsoft.com/en-us/library/windows/desktop/ms644934%28v=vs.85%29.aspx }
      TryHandlingMessage(msg.hWnd, msg.message, msg.wParam, msg.lParam);
      {$endif}
    end;
    {$endif}

    {$ifdef CASTLE_WINDOW_XLIB}
    HandleXEvent(event);
    {$endif}
  end;

  function CheckMessage: boolean;
  {$ifdef CASTLE_WINDOW_WINAPI}
  begin
    Result := true;
    {seek for message to any of windows of our thread}
    if PeekMessage(msg, 0, 0, 0, PM_REMOVE) then
      HandleEvent else
    (*{seek for messages to our thread with hWnd = 0 (send with PostThreadMessage)
      (NOT NEEDED NOW)}
    if PeekMessage(msg, HWND(-1), 0, 0, PM_REMOVE) then
      HandleEvent else *)
      Result := false;
  {$endif}

  {$ifdef CASTLE_WINDOW_XLIB}
  {$ifdef CASTLE_ENGINE_PLUGIN}
  var
    I: Integer;
  begin
    { When running as a plugin, be extra careful to eat only messages
      to our window, otherwise we could eat messages directed to other window. }
    for I := 0 to OpenWindowsCount - 1 do
      if XBool(XCheckWindowEvent(XDisplay,
        OpenWindows[I].WindowXID, AcceptedEventsMask, @event)) <> XBool_False then
      begin
        HandleEvent;
        Exit(true);
      end else
      if XBool(XCheckTypedWindowEvent(XDisplay,
        OpenWindows[I].WindowXID, ClientMessage, @event)) <> XBool_False then
      begin
        HandleEvent;
        Exit(true);
      end;
    Result := false;
  {$else}
  begin
    Result := true;
    if XBool(XCheckMaskEvent(XDisplay, AcceptedEventsMask, @event)) <> XBool_False then
      HandleEvent else
    if XBool(XCheckTypedEvent(XDisplay, ClientMessage, @event)) <> XBool_False then
      HandleEvent else
      Result := false;
  {$endif}
  {$endif}
  end;

  procedure WaitMessage;
  begin
    {$ifdef CASTLE_WINDOW_WINAPI}
    Check( LongInt(GetMessage(msg, 0, 0, 0)) <> -1, 'GetMessage failed with Result -1.');
    {$endif}
    {$ifdef CASTLE_WINDOW_XLIB}
    XNextEvent(XDisplay, @event); { implicit XFlush }
    {$endif}
    HandleEvent;
  end;

  procedure NoMessageProcessed;
  begin
    {$ifdef CASTLE_WINDOW_XLIB}
    XFlush(XDisplay)
    {$endif}
  end;

var
  WasAnyRendering: boolean;
  I: integer;
  Window: TCastleWindowCustom;
begin
  WasAnyMessage := false;

  if (OpenWindowsCount <> 0) and (not CheckMessage) then
  begin
    { no messages to us - call DoApplicationUpdate now.
      Wywolujemy OnUpdate teraz bo :
       - gbybysmy wywolywali OnUpdate w kazdym wywolaniu ProcessMessage
         to doprowadzalisbysmy do tego ze np. ruch mysza powoduje
         pozorne przyspieszenie rysowania - bo gdy user rusza mysza
         nasza petla jest zajeta przetwarzaniem messagy i w zwiazku z tym
         nie wykonuje zadnych OnRender ktore sa przeciez najbardziej
         opozniajaca czescia programu. W rezultacie wykonywanych jest
         mnostwo razy OnUpdate ktore, mimo ze zmieniaja jakas zmienna
         i wywoluja Invalidate to jednak czekaja sporo na
         spowodowane przez siebie redisplay (bo ciagle zamiast redisplay'a
         przetwarzamy ruchy myszka). Wiec aby OnUpdate byly wywolywane
         mniej wiecej z taka sama czestotliwoscia co OnRender'y
         musimy umiescic wywolanie OnUpdate tutaj - tuz przed wywolywaniem
         samych OnRender'ow.
       - gdybysmy wywolywali OnUpdate dopiero do ponizszych drawach
         jezeli not WasAnyRendering to z kolei zdarzenia OnRender
         (ktore np. same sobie juz wywoluja Invalidate) moglyby
         hamowac wywolywanie zdarzen OnUpdate.
         A tego nie chcemy - patrz komentarze w interfejsie,
         nadrzednym celem jest by OnUpdate bylo wykonywane mniej
         wiecej tak czesto co OnRender.  }
    DoApplicationUpdate;

    { redraw some windows; we do it only if our event queue is empty.
      Przy okazji robimy DoUpdate okien, majac nadzieje ze gromadzac
        blisko siebie callbacki tych samych okien zminimalizujemy koszty
        ciaglego zmieniania aktywnego kontekstu OpenGL'a.
      Ponizszy kod wydaje sie miec paranoje na punkcie zamykania okna - pamietajmy
        ze Closed moze zostac wywolane na oknie w kazdym callbacku, takze w OnUpdate
        i OnRender. A Close usuwa to okno z listy OpenWindows, dlatego kopiujemy sobie
        na poczatku OpenWindows[i] do Window. }
    WasAnyRendering := false;
    I := 0;
    while I < OpenWindowsCount do
    begin
      Window := OpenWindows[I];
      Window.DoUpdate;
      if Window.Closed then Continue {don't Inc(I)};
      if Window.Invalidated then
      begin
        Window.DoRender;
        WasAnyRendering := true;
      end;
      if Window.Closed then Continue {don't Inc(I)};
      Inc(I);
    end;

    { If we do not do anything in any Update/timer
      (this is checked by AllowSuspendForInput),
      and there's no reason to redisplay,
      and we're not during quit process (started by Application.Quit),
      => then we can wait, giving OS/CPU some rest (as opposed to doing
      "busy waiting"). }
    WaitForMessage := WaitForMessage and Application.AllowSuspendForInput and
      not (WasAnyRendering or Terminated);
    if WaitForMessage then
      WaitMessage else
      NoMessageProcessed;
  end;

  if Terminated then Exit(false);

  MaybeDoTimer(LastDoTimerTime);

  {$ifndef CASTLE_ENGINE_PLUGIN}
  { Plugin should never just sleep like this, to not block the browser
    from processing messages. Instead, the timer interval will prevent
    us from executing too often. }
  if (not WasAnyMessage) and
     (not Terminated) and
     (not WaitForMessage) and
     WaitToLimitFPS then
    DoLimitFPS;
  {$endif}

  Result := not Terminated;
end;

procedure TCastleApplication.Run;
begin
  if OpenWindowsCount = 0 then Exit;
  inherited Initialize; // set Terminated := false
  inherited Run; // calls DoRun, surrounded by HandleException, in a loop
end;

procedure TCastleApplication.QuitWhenNoOpenWindows;
begin
  Terminate; // set Terminated := true
end;

{$endif read_implementation}