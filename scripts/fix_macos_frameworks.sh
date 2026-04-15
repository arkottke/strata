#!/bin/sh
# fix_macos_frameworks.sh — Fix Qt framework layouts for codesign --verify --strict --deep.
#
# macdeployqt / qt_generate_deploy_app_script may create flat layouts
# or partially-versioned layouts missing proper symlinks.  Restructure
# each framework into the Apple-required versioned directory structure.
#
# Usage: fix_macos_frameworks.sh <path-to.app>

set -e

APP_BUNDLE="${1:?Usage: $0 <path-to.app>}"

if [ ! -d "$APP_BUNDLE/Contents/Frameworks" ]; then
  echo "No Frameworks directory in $APP_BUNDLE — nothing to fix."
  exit 0
fi

for fw in "$APP_BUNDLE/Contents/Frameworks/"*.framework; do
  [ -d "$fw" ] || continue
  name=$(basename "$fw" .framework)

  # Skip if the framework already has a fully correct layout
  if [ -f "$fw/Versions/A/$name" ] && \
     [ "$(readlink "$fw/Versions/Current" 2>/dev/null)" = "A" ] && \
     [ -L "$fw/$name" ] && [ -L "$fw/Resources" ]; then
    continue
  fi

  # Locate the actual binary (may already be under Versions/A)
  real_bin=""
  if [ -f "$fw/Versions/A/$name" ]; then
    real_bin="$fw/Versions/A/$name"
  elif [ -f "$fw/$name" ] && [ ! -L "$fw/$name" ]; then
    real_bin="$fw/$name"
  fi
  [ -z "$real_bin" ] && continue

  mkdir -p "$fw/Versions/A/Resources"

  # Move binary into versioned location if needed
  if [ "$real_bin" != "$fw/Versions/A/$name" ]; then
    mv "$real_bin" "$fw/Versions/A/$name"
  fi

  # Move Resources into versioned location if it is a real directory
  if [ -d "$fw/Resources" ] && [ ! -L "$fw/Resources" ]; then
    cp -a "$fw/Resources/"* "$fw/Versions/A/Resources/" 2>/dev/null || true
    rm -rf "$fw/Resources"
  fi

  # (Re-)create symlinks
  rm -f "$fw/Versions/Current" "$fw/$name" "$fw/Resources" 2>/dev/null || true
  ln -sf A "$fw/Versions/Current"
  ln -sf "Versions/Current/$name" "$fw/$name"
  ln -sf "Versions/Current/Resources" "$fw/Resources"

  # Ensure Info.plist exists (codesign --strict requires one)
  if [ ! -f "$fw/Versions/A/Resources/Info.plist" ]; then
    cat > "$fw/Versions/A/Resources/Info.plist" << PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleIdentifier</key>
  <string>org.qt-project.${name}</string>
  <key>CFBundleExecutable</key>
  <string>${name}</string>
  <key>CFBundlePackageType</key>
  <string>FMWK</string>
</dict>
</plist>
PLIST
  fi
done

echo "Framework layouts fixed for: $APP_BUNDLE"
