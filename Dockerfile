FROM highperformancecoder/builttravisciimage
COPY . /root
RUN useradd -m minsky
RUN chmod a+rx /root
#RUN zypper addrepo https://download.opensuse.org/repositories/home:hpcoder1/openSUSE_Tumbleweed/home:hpcoder1.repo
#RUN zypper --gpg-auto-import-keys refresh
#RUN zypper install -y -l unuran-devel
#RUN cd /root && sh travisRun.sh

