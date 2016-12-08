# eXtensible ARchiver

A fork/clone of the subversion xar repository that includes several enhancements
and bug fixes including very basic command line signature support.

The project page with extensive documentation on signing, pre-bootstrapped
downloads, etc. can be found at:

  <https://github.com/KubaKaszycki/xar/>

## Enhancements

See the [xar/NEWS](xar/NEWS) file for release notes and the
[xar/ChangeLog](xar/ChangeLog) file for a more detailed list.

Notable enhancements are command line signature support, `--strip-components` and
`--to-stdout` support, cygwin build support, support for arbitrary checksum
message digests (e.g. sha256, sha512), improved liblzma compatibility, many
new command synonyms, and lots and lots of bug fixes and warning eliminations.

See the xar/NEWS and/or xar/ChangeLog file for the full list.

## Source Directories

<table>
<tr>
  <th>xar</th>
  <td>Primary source directory, contains libxar sources and xar sources.</td>
</tr>
<tr>
  <th>XarCMPlugin</th>
  <td>OS X xar contextual menu plugin with Xcode project. Not integrated into
      Autotools build system. May be out of date.</td>
</tr>
<tr>
  <th>XarKit</th>
  <td>OS X xar framework Xcode project. May be incomplete.</td>
</tr>
<tr>
  <th>xarmdimport</th>
  <td>OS X xar SpotLight plugin with Xcode project. Not integrated into
      Autotools build system. May not be release quality.</td>
</tr>
<tr>
  <th>xarql</th>
  <td>OS X xar QuickLook plugin with Xcode project. Not integrated into
      Autotools build system. May not be release quality.</td>
</tr>
<tr>
  <th>tools</th>
  <td>Some xar archive tool sources and man pages. They are not very useful.
      Some may be very out of date or even uncompilable.</td>
</tr>
<tr>
  <th>python</th>
  <td>The pyxar Python bindings for xar. May be incomplete.</td>
</tr>
</table>

## License

See the xar/LICENSE file.  This is a "New BSD License" aka a
"Modified BSD license" or a "3-clause BSD license".
See the xar/LICENSE file for details.

## Building

First, if bootstraping from freshly cloned repository, generate the build
system.

```shell
git submodule update --init
./autogen.sh
```

Then, configure it:

```shell
./configure
```

Run it with `--help` argument to view possible arguments for it.

Now, build it:

```shell
make
```

It tries to be as portable as possible, but GNU Make is recommended.

You should check whether you didn't compile faulty release. Run:

```shell
make check
```

to run the test-suites.

It's time for installation. Type:

```shell
sudo make install
```

To put it in system directories (it even install macOS/GNUStep frameworks).

Pre-bootstrapped xar release tarballs (i.e. with configure already generated)
and pre-built binaries can be found in the downloads section of the project
page at:

<https://github.com/KubaKaszycki/xar/releases/>
