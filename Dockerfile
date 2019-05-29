FROM amd64/ubuntu:bionic
RUN apt-get update && apt-get install -y cmake g++ make libgmp-dev libntl-dev libcppunit-dev gperf
WORKDIR /cpisync
EXPOSE 8001
CMD ["/bin/bash"]

