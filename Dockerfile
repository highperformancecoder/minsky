FROM highperformancecoder/travisciimage
COPY . /root
RUN useradd -m minsky
RUN chmod a+rx /root
RUN zypper install -y -l texlive-latex-bin-bin texlive-epstopdf-bin latex2html linkchecker
RUN cd /root && sh travisRun.sh

