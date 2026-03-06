#!/usr/bin/env bash

usage="Usage: $0 [-h|--help] [--wipe]"

diag ()
{
  echo "$*" 1>&2
}

wipe=
release=

while test $# -ne 0; do
  case "$1" in
    -h|--help)
      diag "$usage"
      diag
      diag "Initialize MinGW-w64 build configurations using bdep."
      diag
      diag "This script sets up the build system to cross-compile for Windows"
      diag "and install the resulting binaries directly into your IW4x game"
      diag "directory."
      diag
      diag "Options:"
      diag "  --wipe       Remove existing bdep state and host configurations"
      diag "               before initializing (clean slate)."
      diag
      diag "  --release    Create the optimized Release configuration (@mingw32-release)"
      diag "               in addition to the default Debug configuration."
      diag
      exit 0
      ;;
    --wipe)
      wipe=yes
      shift
      ;;
    --release)
      release=yes
      shift
      ;;
    *)
      diag "error: unknown option '$1'"
      diag "$usage"
      exit 1
      ;;
  esac
done

if ! command -v bdep >/dev/null 2>&1; then
  diag "error: bdep not found in PATH"
  exit 1
fi

# Derive project directory basename (e.g., 'libiw4x').
#
project="$(basename "$(pwd)")"

# Cache path for the game root.
#
# We cache this to avoid pestering the user for the path every time they
# re-run the bootstrap.
#
cache="${XDG_CACHE_HOME:-$HOME/.cache}/iw4x/bootstrap-root"

# Wipe.
#
# If requested, wipe the local bdep state and the associate host directory.
# This is useful if the project structure has changed significantly or if
# the configurations have drifted.
#
if test -n "$wipe"; then
  diag "Wiping configurations: removing existing state..."

  if test -d .bdep; then
    rm -rf .bdep >/dev/null 2>&1 || true
  fi

  # Assuming standard bdep layout where host config is ../<project>-host.
  #
  if test -d "../${project}-host"; then
    rm -rf "../${project}-host" >/dev/null 2>&1 || true
  fi
fi

# Locate IW4x.
#
iw4x=

if test -f "$cache"; then
  cached="$(cat "$cache")"
  if test -d "$cached"; then
    iw4x="$cached"
  fi
fi

if test -z "$iw4x"; then
  diag "Specify the absolute path to IW4x installation root:"
  printf "> "
  read -er iw4x

  if test -z "$iw4x"; then
    diag "error: empty path is not valid"
    exit 1
  fi

  if test ! -d "$iw4x"; then
    diag "error: directory '$iw4x' does not exist"
    exit 1
  fi

  # Normalize (strip trailing slash).
  #
  iw4x="${iw4x%/}"

  # Update cache.
  #
  mkdir -p "$(dirname "$cache")" >/dev/null 2>&1 && printf "%s\n" "$iw4x" > "$cache"
fi

diag
diag "Using IW4x installation root: $iw4x"
diag "Creating build configurations..."
diag

# Debug Configuration.
#
# We enable frame pointers and full GDB info to make stack traces actually
# readable in generic debuggers. We also dump `compile_commands.json` into the
# project root for LSP support.
#
# Note: We set `config.install.root` to the game directory so `b install` (and
# by extension `bdep update`) deploys the DLL right where the game exe lives.
#
bdep init -C @mingw32-debug                                          \
  config.cxx=x86_64-w64-mingw32-g++                                  \
  config.cc.coptions="-ggdb                                          \
                      -grecord-gcc-switches                          \
                      -pipe                                          \
                      -mtune=generic                                 \
                      -fasynchronous-unwind-tables                   \
                      -fno-omit-frame-pointer                        \
                      -mno-omit-leaf-frame-pointer                   \
                      -Wa,-mbig-obj"                                 \
  config.cc.loptions="-static-libgcc -static-libstdc++"              \
  config.cc.compiledb=./                                             \
  cc                                                                 \
  config.import.libiw4x=libiw4x                                      \
  config.install.filter='include/@false lib/@false share/@false'     \
  config.install.root="$iw4x"                                        \
  config.install.bin="$iw4x"                                         \
  --wipe

# Release Configuration.
#
# Only created if requested via --release.
#
if test -n "$release"; then
  bdep init -C @mingw32-release                                        \
    config.cxx=x86_64-w64-mingw32-g++                                  \
    config.cc.coptions="-O2                                            \
                        -ggdb                                          \
                        -grecord-gcc-switches                          \
                        -pipe                                          \
                        -mtune=generic                                 \
                        -fasynchronous-unwind-tables                   \
                        -fno-omit-frame-pointer                        \
                        -mno-omit-leaf-frame-pointer                   \
                        -Wa,-mbig-obj"                                 \
    config.cc.loptions="-static-libgcc -static-libstdc++"              \
    config.cc.compiledb=./                                             \
    cc                                                                 \
    config.import.libiw4x=libiw4x                                      \
    config.install.filter='include/@false lib/@false share/@false'     \
    config.install.root="$iw4x"                                        \
    config.install.bin="$iw4x"                                         \
    --wipe
fi

# HACK:
#
# Work around a bootstrap phase ordering issue during nasm target evaluation.
#
# The problem is an implicit dependency cycle: the build system attempts to
# resolve rules requiring nasm before nasm executable is built and registered.
# This causes the initial execution phase to fail.
#
# To mitigate this, force a secondary build invocation. On the second pass, the
# nasm is correctly identified, allowing downstream targets to build.
#
# Note that on CI we cannot have a secondary invocation. We can, however, pass
# the -s flag to build synchronously which bypasses the underlying problem.
#
b || true
b
