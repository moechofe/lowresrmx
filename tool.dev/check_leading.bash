#!/bin/bash

SEARCH_DIR=${1:-.}

grep -rPzl "(?s)^\s+<\?php" $SEARCH_DIR
