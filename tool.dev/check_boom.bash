#!/bin/bash

SEARCH_DIR=${1:-.}

find "$SEARCH_DIR" -type f -name "*.php" -exec file {} + | grep "with BOM"

find "$SEARCH_DIR" -type f -name "*.php" -exec grep -lP "^\xef\xbb\xbf|^\xff\xfe|^\xfe\xff|^\x00\x00\xfe\xff|^\xff\xfe\x00\x00" {} +
