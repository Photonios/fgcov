# fgcov (fixed-gcov)
fgcov is a hack for a bug/flaw in GCC's gcov, a tool for code coverage reporting. The flaw occurs when one runs gcov with the `-p` and `-l` flag:

	gcov -p -l -o [dir] [gdca file]

This _can_ can cause extremely long filenames. It can actually create filenames that exceed 255 characters. This is problematic for most systems, which do not allow file names that exceed 255 characters. At the moment, there is no decent solution for this. A patch has been merged recently that adds a command line option to overcome this problem. This patch however will only be part of the 7.1 release of GCC in March 2017. Even then, it will take a while before that version becomes available in distribution's their repositories.

fgcov aims to solve this problem without requiring you to compile the bleeding-edge version of gcov from source.

## What?
`fgcov` is a slim wrapper over standard `gcov` that hashes filenames (using MD5) at runtime to avoid hitting the 255 maximum file name length. It can be used as a drop-in replacement for `gcov` by simply invoking `fgcov`.

## How?
`fgcov` relies on a couple of rather `hackish` techniques:

1. The `LD_PRELOAD` trick. A special shared library that overrides the `fopen` function is injected into gcov. If gcov attempts to write a `*.gcov` file, it will intercept this, hash the filename and then invoke the real `fopen` function.
2. When the modified `fopen` function is called, it will also override the specified string buffer containing the filename so that also gcov is "informed" of this change.
3. The special shared library (`libgcov.so`) is injected into the `fgcov` binary and is extracted to `/tmp` when the program is run. `fgcov` then runs `gcov` by specifying `libfgcov` in the `LD_PRELOAD` environment variable.

See:

- https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/
- https://sourceware.org/binutils/docs/binutils/objcopy.html

Read `lib.cpp` for more information.

### Why MD5?
It is well-known that MD5 can no longer be considered "secure". But secure is not what we want, we want to hash filenames so they have a fixed length and stay unique, MD5 is still useful for that and its hashes are a little shorted then for example SHA256.

This program uses a MD5 implementation that was shamefully ripped from a random web page :-)

## Building

	cmake ../; make -j8

## Using
Invoke `fgcov` instead of `gcov`.
