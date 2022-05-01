# OBS packaging strategy
Linux releases are distributed via the  [OpenSUSE build service](https://build.opensuse.org/project/show/home:hpcoder1) (aka OBS). The C++ and Javascript portions are distributed as separate packages. 

# C++ backend portion (minskyRESTService)
This is handled via the minsky-restservice (or minsky-resetservice-beta) package. Making a new release involved copying the source distribution (created by `make dist`) into the OBS repository, and updating minsky.spec and minsky.dsc respectivey.

# Javascript front end
This is handled in the minsky (or minsky-beta) package. Making a new release involves running `make js-dist`, and copying the resultant `minskyjs-<version>.tar.gz` file into the minsky OBS package and updating the spec and dsc file approriately.
This file does contain some x86-64 architecture files (minsky, dynamic libs), so it is likely that only x86-64 architectures can be supported.
