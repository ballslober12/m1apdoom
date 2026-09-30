#!/bin/sh
set -eu

SRC=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
BUILD=${BUILD:-/tmp/m1ap_doom_build}
TOOLCHAIN=${TOOLCHAIN:-$HOME/m1ap_toolchain/gp2-s7-mipsel-lg-gcc-4.3.2-glibc-2.9-nptl}
CC=${CC:-$TOOLCHAIN/bin/mipsel-lg-linux-gnu-gcc}
LIBS=${LIBS:-$HOME/m1ap_toolchain/_libs}

if [ ! -x "$CC" ]; then
    echo "ERROR: M1AP compiler not found: $CC" >&2
    echo "Set TOOLCHAIN=/path/to/gp2-s7-mipsel-lg-gcc-4.3.2-glibc-2.9-nptl" >&2
    exit 1
fi

rm -rf "$BUILD"
mkdir -p "$BUILD"
cp -a "$SRC/linuxdoom-1.10/." "$BUILD/"
cp "$SRC/include/lgnc.h" "$BUILD/"
cp -a "$SRC/include/directfb" "$BUILD/"
cd "$BUILD"

# Profile:
# -O2: better instruction/cache balance than size-heavy -Os on this CPU
# -mips32 -mabi=32 -EL: original M1AP MIPS32 little-endian ABI
# -fomit-frame-pointer: one more register for the software renderer
# -ffast-math: only the platform layer uses floating point; Doom itself is fixed point
# -fno-strict-aliasing: matches the old Doom code's pointer tricks safely
M1AP_TICRATE=${M1AP_TICRATE:-50}
CFLAGS="-std=gnu99 -O2 -mips32 -mabi=32 -EL -fomit-frame-pointer -ffast-math -fno-math-errno -fno-strict-aliasing -ffunction-sections -fdata-sections -DM1AP_TICRATE=$M1AP_TICRATE -DNORMALUNIX -DNO_NETWORK -DNO_SOUND -I. -I$SRC/include -I$SRC/include/directfb"

# Compile each translation unit separately so a bad reference is traceable,
# then link strictly (-z defs: no silently unsatisfied symbols).
OBJS=""
for f in *.c; do
    o="${f%.c}.o"
    echo "CC  $f"
    "$CC" $CFLAGS -c "$f" -o "$o"
    OBJS="$OBJS $o"
done

echo "LINK m1ap_doom"
"$CC" $CFLAGS $OBJS -L"$LIBS" -Wl,-rpath-link,"$LIBS"     -Wl,-z,defs -Wl,-rpath,/mnt/usb1/Drive1/LG_GAMES/build     -o m1ap_doom -ldirectfb -ldirect -llgncopenapi -lm -lrt

cp m1ap_doom "$SRC/"
echo "Built: $SRC/m1ap_doom"
file "$SRC/m1ap_doom" 2>/dev/null || true

# Library shipping reminder: the runtime crash 'undefined symbol: info' means
# the TV loaded different .so versions than these. Copy them next to the exe:
echo ""
echo "Copy these to /mnt/usb1/Drive1/LG_GAMES/build/ alongside the binary:"
for l in libdirectfb libdirect liblgncopenapi; do
    ls "$LIBS"/$l.so* 2>/dev/null || true
done
