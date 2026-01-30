#!/bin/bash
# Setup script for Geolocation-Sample
# Creates symlinks for Wt framework resources

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RESOURCES_DIR="$PROJECT_DIR/resources"

echo "Setting up Wt resources for Geolocation-Sample..."

# Find Wt resources directory
WT_RESOURCES=""

# Common locations to check
SEARCH_PATHS=(
    "/opt/homebrew/share/Wt/resources"
    "/usr/local/share/Wt/resources"
    "/usr/share/Wt/resources"
    "/usr/local/share/wt/resources"
)

for path in "${SEARCH_PATHS[@]}"; do
    if [ -d "$path" ]; then
        WT_RESOURCES="$path"
        break
    fi
done

# If not found, try to find it
if [ -z "$WT_RESOURCES" ]; then
    echo "Searching for Wt resources..."
    WT_CSS=$(find /usr /opt -name "wt.css" -path "*/themes/default/*" 2>/dev/null | head -1)
    if [ -n "$WT_CSS" ]; then
        WT_RESOURCES=$(dirname $(dirname $(dirname "$WT_CSS")))
    fi
fi

if [ -z "$WT_RESOURCES" ] || [ ! -d "$WT_RESOURCES" ]; then
    echo "ERROR: Could not find Wt resources directory."
    echo "Please install Wt or set WT_RESOURCES environment variable."
    exit 1
fi

echo "Found Wt resources at: $WT_RESOURCES"

# Create resources directory if needed
mkdir -p "$RESOURCES_DIR"

# Create symlinks
echo "Creating symlinks..."

# Themes directory
if [ -L "$RESOURCES_DIR/themes" ]; then
    rm "$RESOURCES_DIR/themes"
fi
if [ ! -d "$RESOURCES_DIR/themes" ]; then
    ln -s "$WT_RESOURCES/themes" "$RESOURCES_DIR/themes"
    echo "  Created: resources/themes -> $WT_RESOURCES/themes"
else
    echo "  Skipped: resources/themes (already exists as directory)"
fi

# webkit-transitions.css
if [ -L "$RESOURCES_DIR/webkit-transitions.css" ]; then
    rm "$RESOURCES_DIR/webkit-transitions.css"
fi
if [ ! -f "$RESOURCES_DIR/webkit-transitions.css" ]; then
    if [ -f "$WT_RESOURCES/webkit-transitions.css" ]; then
        ln -s "$WT_RESOURCES/webkit-transitions.css" "$RESOURCES_DIR/webkit-transitions.css"
        echo "  Created: resources/webkit-transitions.css"
    fi
else
    echo "  Skipped: resources/webkit-transitions.css (already exists)"
fi

echo ""
echo "Setup complete!"
echo ""
echo "Run the application with:"
echo "  ./franchise_ai_search --docroot ./resources --http-address 0.0.0.0 --http-port 8083"
