#!/bin/sh
# Copyright 2011 Chris Andrews. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice, this list of
#       conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright notice, this list
#       of conditions and the following disclaimer in the documentation and/or other materials
#       provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY CHRIS ANDREWS ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CHRIS ANDREWS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# GYP's MAKEFLAGS confuses libusdt's Makefile
#
unset MAKEFLAGS

# Ask node what arch it's been built for, and build libusdt to match.
#
# We use node from the path; npm will have adjusted PATH for us if
# necessary, otherwise we assume the user did so when building by
# hand.
#
# (this will need to change at the point that GYP is able to build
# node extensions universal on the Mac - for now we'll go with x86_64
# on a 64 bit Mac, because that's the default architecture in that
# situation).
#
ARCH=`node libusdt-arch.js`
echo "Building libusdt for ${ARCH}"
export ARCH

# Respect a MAKE variable if set
if [ -z $MAKE ]; then
  # Default to `gmake` first if available, because we require GNU make
  # and `make` isn't GNU make on some plats.
  MAKE=`which gmake`
  if [ -z $MAKE ]; then
    MAKE=make
  fi
fi

# Build.
#
$MAKE -C libusdt clean all
