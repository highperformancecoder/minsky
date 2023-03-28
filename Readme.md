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


## Roadmap

- See [planned features](https://sourceforge.net/p/minsky/features/)
- Emscripten classdesc descriptor to support Minsky in a browser.

