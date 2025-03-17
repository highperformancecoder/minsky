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

## Using Minsky in "batch mode" from Python
A little known secret is that Minsky can be used in a headless batch mode, a capability it has had from the beginning in the form of TCL scripting. Since March 2024, Python scripting has been available in Linux builds, and is coming soon to Windows and Mac builds (version 3.12). TCL scripting is deprecated and will soon be removed from the codebase.

[Introduction to scripting Minsky in Python]

## Compiling Minsky from source code

Official releases are available from [SourceForge](https://sourceforge.net/projects/minsky/files/Sources/). You may also obtain later releases from this Github project, as release are tagged within git.

To compile Minsky, you will need a suitable posix compliant system, [with a number of prerequisites installed](Compiling.md).

Type `make -j4` in the top level directory of the repository.

## Running Minsky from source code

Minsky can be run directly by running `gui-js/dist/executables/linux-unpacked/minsky`.

You can also run it by running `npm start` from the gui-js directory. At the time of writing, npm start will not work on certain Linux systems. I suspect it has to do with poor IPv6 support in electron, but the cause is as yet unknown.

## Compiling release versions of Minsky

- [Windows](githubdocs/WindowsRelease.md)
- [MacOSX](githubdocs/MacRelease.md)
- [Linux](githubdocs/LinuxRelease.md)

## Using the REST Service

Notes on using the [REST Service](RESTService.md).

## JS frontend debugging notes

- `console.log` works on minsky-electron code
- for minsky-web code, use `electronService.log`, which takes a single string argument. Best trick is to use typescript template literal types, which allows embedding variable values simply within a string, eg
~~~
    this.electronService.log(`Var=${some javascript expression}`);
~~~
NB since the upgrade of electron, process is no longer available in the renderer (minsky-web) environment.
- use `green(`*some string*`)` or `red(`*some string*`)` to colorise log output to make it easier to spot your messages. With console.log, you only need to colourise the first argument.
- enable front end debugging tools by setting `OPEN_DEV_TOOLS_IN_DEV_BUILD = true` in `libs/shared/src/lib/constants/constants.ts`. This can be very useful in tracking down runtime errors, even syntax errors, as the renderer process fails silently.
- When building for production (make in toplevel directory, or npm run export:package:*), then you also need to set the environment variable `ELECTRON_IS_DEV=1`

## Setting up emacs

For those using emacs (comme moi-meme), I have configured the project so that typing tsc on the gui-js directory runs the typescript compiler. A convenience Makefile is in that directory, allowing the default "make -k" command to work.

- You will need to install tsc somewhere in your path, eg "zypper install typescript"
- You should install the [tide package](https://github.com/ananthakumaran/tide/). Note that `M-x package-install` did not work for me, I had to download the tarfiles directly from [Melpa](https://melpa.org/), and run `M-x package-install-file` to install the packages manually.

## Debugging with gdb

- You can debug an `npm start` session by using `ps -ef|grep main.js` to find the minsky process, then use the pid to attach to in gdb.
- Alternatively, you can create an executable with `npm run export:package:linux` and open it with `gdb gui-js/dist/executables/linux-unpacked/minsky`.
- For unit tests, cd to the directory containing `jest.config.js` (eg gui-js/libs/shared), then run (eg)
~~~~
   gdb `which node`
   r /usr/local/bin/jest src/lib/backend/minsky.test.ts
~~~~
Regular jest CLI arguments can be provided. You will need to install the jest package (npm install jest perhaps?)

## Profiling

Unfortunately gperftools do not work with the JS frontend. Indeed, most of the time minsky will crash under profiling. For now, profiling requires the use of the old Tk version of minsky.

However, if you build the C++ source (actually only addon.cc necessary) with the TIMERS=1 make flag, this enables profiling of the backend commands, which is printed as a summary table on application exit.

## Roadmap

- See [planned features](https://sourceforge.net/p/minsky/features/)
- Emscripten classdesc descriptor to support Minsky in a browser.

