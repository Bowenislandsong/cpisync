FROM x86-64/ubuntu:latest
RUN apt-get -y update && apt-get install -y cmake g++ ntl gmp 
ADD . /cpisync
CMD ["/bin/bash"]

