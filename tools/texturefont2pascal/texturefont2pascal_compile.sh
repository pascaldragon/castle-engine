#!/bin/bash
set -eu

# Call this script from this directory,
# or from base castle_game_engine directory.
# Or just do "make examples" in base castle_game_engine directory.

# Allow calling this script from it's dir.
if [ -f texturefont2pascal.lpr ]; then cd ../../; fi

fpc -dRELEASE @castle-fpc.cfg tools/texturefont2pascal/texturefont2pascal.lpr
