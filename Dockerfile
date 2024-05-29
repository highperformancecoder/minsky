FROM highperformancecoder/builttravisciimage
ARG debug="DEBUG=1"
COPY . /root
RUN useradd -m minsky
RUN chmod a+rx /root
RUN cd /root && env $debug sh travisRun.sh

