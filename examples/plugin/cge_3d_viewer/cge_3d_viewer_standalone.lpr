{$mode objfpc}{$H+}
{$apptype GUI}
program cge_3d_viewer_standalone;
{$ifdef MSWINDOWS}
  {$R automatic-windows-resources.res}
{$endif MSWINDOWS}
uses CastleWindow, Game;
begin
  { explicitly initialize MainWindow with useful NamedParameters,
    to make it similar to what happens when you run a plugin. }
  Application.MainWindow := Application.DefaultWindowClass.Create(Application);
  Application.MainWindow.NamedParameters.Values['cge_scene'] :=
    'http://svn.code.sf.net/p/castle-engine/code/trunk/demo_models/cube_environment_mapping/cubemap_generated_in_dynamic_world.x3dv';
  Application.MainWindow.OpenAndRun;
end.
