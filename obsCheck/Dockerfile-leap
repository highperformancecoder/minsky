FROM opensuse/leap:15
ADD . /root
RUN zypper addrepo https://download.opensuse.org/repositories/home:hpcoder1/openSUSE_Leap_15.0/home:hpcoder1.repo
RUN zypper --gpg-auto-import-keys refresh
RUN zypper --non-interactive install minsky
RUN minsky /root/smoke.tcl
