#!/bin/bash
set -eu

# Allow calling this script from it's own dir.
if [ -f window_events.lpr ]; then
  cd ../../
fi

# Call this from ../../ (or just use `make examples').
fpc -dRELEASE @kambi.cfg examples/window/window_events.lpr