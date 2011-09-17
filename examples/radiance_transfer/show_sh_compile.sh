#!/bin/bash
set -eu

# Hack to allow calling this script from it's dir.
if [ -f show_sh.lpr ]; then
  cd ../../
fi

# Call this from ../../ (or just use `make examples').

fpc -dRELEASE @kambi.cfg examples/radiance_transfer/show_sh.lpr