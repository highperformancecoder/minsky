FROM debian
ADD . /root
RUN echo 'deb http://download.opensuse.org/repositories/home:/hpcoder1/Debian_9.0/ /' > /etc/apt/sources.list.d/home:hpcoder1.list
RUN apt-get update
RUN apt-get install -y --allow-unauthenticated minsky
RUN minsky /root/smoke.tcl
