# Minsky

Dynamic flow systems program to simulate economic flows (ie. complex system dynamics).
For a full overview, please see the [Manual](http://minsky.sf.net/manual/minsky.html)

- [File Structure](File%20structure.md) for an overview of folders and files
- [Architecture](Architecture.md) for an overview of the app and DAG engine
- [Minsky Schema](githubdocs/schema.md) describes the schema of the Minsky .mky model files

## Getting Started

Minsky is an open source program with prebuilt binaries available for:
- [Windows](https://sourceforge.net/projects/minsky/files/Windows%20Binaries/)
- [Mac OS X](https://sourceforge.net/projects/minsky/files/Mac%20Binaries/)
- [various Linux distributions](https://build.opensuse.org/package/show/home:hpcoder1/minsky)

## Compiling Minsky from source code

Official releases are available from [SourceForge](https://sourceforge.net/projects/minsky/files/Sources/). You may also obtain later releases from this Github project, as release are tagged within git.

To compile Minsky, you will need a suitable posix compliant system, [with a number of prerequisites installed](Compiling.md).

## Compiling release versions of Minsky

- [Windows](githubdocs/WindowsRelease.md)
- [MacOSX](githubdocs/MacRelease.md)
- Linux: Linux distributions are managed through the [OpenSUSE build service](https://build.opensuse.org/package/show/home:hpcoder1/minsky) which won't be documented here.

## Using the REST Service

Notes on using the [REST Service](RESTService.md).

## Roadmap

- Finalising the Malthus iteration, due end of August.
- Starting the Say iteration.
- Javascript implementation of Minsky in electron.
- Emscripten classdesc descriptor to support Minsky in a browser.

