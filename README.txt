===================
eXtensible ARchiver
===================
A fork/clone of the subversion xar repository that includes several enhancements
and bug fixes including very basic command line signature support.

The project page with extensive documentation on signing, pre-bootstrapped
downloads, etc. can be found at:

  http://mackyle.github.io/xar

------------
Enhancements
------------
See the xar/NEWS file for release notes and the xar/ChangeLog file for a more
detailed list.

Notable enhancements are command line signature support, --strip-components and
--to-stdout support, cygwin build support, support for arbitrary checksum
message digests (e.g. sha256, sha512), improved liblzma compatibility, many
new command synonyms, and lots and lots of bug fixes and warning eliminations.

See the xar/NEWS and/or xar/ChangeLog file for the full list.

------------------
Source Directories
------------------
xar -
        Primary source directory, contains libxar sources and xar sources.

XarCMPlugin -
        OS X xar contextual menu plugin with Xcode project.  May be out of date.

XarKit -
        OS X xar framework Xcode project.  May be out of date.

xarmdimport -
        OS X xar SpotLight plugin with Xcode project.  May not be release quality.

xarql -
        OS X xar QuickLook plugin with Xcode project.  May not be release quality.

tools -
        Some xar archive tool sources and man pages.  May be out of date.

python -
        The pyxar Python bindings for xar.  May be very out of date.

-------
License
-------
See the xar/LICENSE file.  This is a "New BSD License" aka a
"Modified BSD license" or a "3-clause BSD license".
See the xar/LICENSE file for details.

--------
Building
--------
A standard configure and make will suffice.  If building directly from the git
sources the xar/autogen.sh script must be run to generate the configure script.
The autogen.sh script takes a single, optional "--noconfigure" argument to
suppress running configure.  If building from a release tarball, the configure
script has already been generated.

After a successful configure simply do a standard make, but it must be GNU make
(which is typically installed as either make or gmake).

After a successful build, the standard "sudo make install" (again GNU make must
be used) will install.

See the xar/INSTALL file for additional information.

Pre-bootstrapped xar release tarballs (i.e. with configure already generated)
can be found in the downloads section of the project page at:

  http://mackyle.github.io/xar/#downloads
