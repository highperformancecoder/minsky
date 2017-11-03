# Minsky

Dynamic flow systems program to simulate economic flows (ie. complex system dynamics).
For a full overview, please see the [Manual](http://minsky.sf.net/manual/minsky.html)

- [File Structure](File Structure.md) for an overview of folders and files
- [Architecture](Architecture.md) for an overview of the app and DAG engine
- [Minsky Schema](githubdocs/schema.md) describes the schema of the Minsky .mky model files

## Getting Started

Minsky is an open source program with prebuilt binaries available for:
- [Windows](https://sourceforge.net/projects/minsky/files/Windows Binaries/)
- [Mac OS X](https://sourceforge.net/projects/minsky/files/Mac Binaries/)
- [various Linux distributions](https://build.opensuse.org/package/show/home:hpcoder1/minsky)

## Compiling Minsky from source code

Official releases are available from [SourceForge](https://sourceforge.net/projects/minsky/files/Sources/). You may also obtain later releases from this Github project, as release are tagged within git.

To compile Minsky, you will need a suitable posix compliant system, with a number of prerequisites installed. More information can be found on the [SourceForge wiki](https://sourceforge.net/p/minsky/wiki/Building Minsky/).

## Compiling release versions of Minsky

- [Windows](githubdocs/WindowsRelease.md)
- [MacOSX](githubdocs/MacRelease.md)
- Linux: Linux distributions are managed through the [OpenSUSE build service](https://build.opensuse.org/package/show/home:hpcoder1/minsky) which won't be documented here

## Roadmap

- Current focus is to bed down the refactored model, and release this as the first Quesnay iteration release (Minsky 2.1). This will include various features to assist pedagogy (marked "Kingston" in the SourceForge ticket tracker).

- Support for an online library of models and snippets, via forkable Github repositories.

