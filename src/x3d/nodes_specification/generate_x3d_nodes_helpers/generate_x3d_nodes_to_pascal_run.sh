#!/bin/bash
set -eu

./generate_x3d_nodes_to_pascal ../components/*.txt ../../../../../www/htdocs/x3d_extensions.txt > ../../x3dnodes_node_helpers.inc
