#!/bin/bash
set -eu

# Hack to allow calling this script from it's dir.
if [ -f simple_video_editor.lpr ]; then
  cd ../../
fi

# Call this from ../../ (or just use `make examples').

fpc -dRELEASE @kambi.cfg examples/images_videos/simple_video_editor.lpr