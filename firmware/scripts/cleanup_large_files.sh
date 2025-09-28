#!/bin/bash
# cleanup-large-files.sh

TARGET_DIR="/root/Bela/samples"

# Find and delete files larger than 100MB
find "$TARGET_DIR" -type f -size +100M -print -delete
