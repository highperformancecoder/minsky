FROM ubuntu
ADD . /root
RUN cat /etc/os-release
RUN apt-get update -qq -y
RUN apt-get install -y wget gnupg2
RUN wget http://download.opensuse.org/repositories/home:hpcoder1/xUbuntu_18.04/Release.key
RUN apt-key add - < Release.key
RUN echo 'deb http://download.opensuse.org/repositories/home:/hpcoder1/xUbuntu_18.04/ /' >/etc/apt/sources.list.d/hpcoders.list
RUN apt-get update -qq -y
RUN apt-get install -y --allow-unauthenticated minsky
RUN minsky /root/smoke.tcl
