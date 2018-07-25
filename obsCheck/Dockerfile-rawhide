FROM fedora:rawhide
ADD . /root
RUN dnf install -y --allowerasing 'dnf-command(config-manager)'
RUN dnf config-manager --add-repo https://download.opensuse.org/repositories/home:hpcoder1/Fedora_Rawhide/home:hpcoder1.repo
RUN dnf install -y minsky
RUN minsky /root/smoke.tcl
