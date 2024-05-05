# Libgssglue

Libgssglue provides a GSS-API library, but does not implement any
mechanisms itself; instead it calls routines in other libraries,
depending on the mechanism and system configuration.

Libgssglue requires C99 (stdint.h etc) and POSIX (sys/types.h,
unistd.h etc) and should work on all modern Unix-like platforms.

Libgssglue is licensed under a BSD-3-clause license, see
[COPYING](COPYING).

# Building

See [INSTALL](INSTALL) for building released tarball versions.

To build libgssglue from git you need several tools:

* C compiler, [gcc](https://www.gnu.org/software/gcc/) or [clang](https://clang.llvm.org/)
* [Make](https://www.gnu.org/software/make/)
* [Automake](https://www.gnu.org/software/automake/)
* [Autoconf](https://www.gnu.org/software/autoconf/)
* [Libtool](https://www.gnu.org/software/libtool/)
* [Tar](https://www.gnu.org/software/tar/) (for 'make dist')
* [Gzip](https://www.gnu.org/software/gzip/) (for 'make dist')
* [Git](https://git-scm.com/)

The required software is typically distributed with your operating
system, and the instructions for installing them differ.  Here are
some hints:

Debian/Ubuntu:
```
apt-get install -y autoconf automake libtool make indent git

```
Fedora/RHEL:
```
yum install -y make gcc autoconf automake libtool indent git
```

Build libgssglue as follows:

```
./bootstrap
./configure
make check
```

# Resources

* https://gsasl.gitlab.io/libgssglue/coverage/src/
* https://scan.coverity.com/projects/libgssglue
