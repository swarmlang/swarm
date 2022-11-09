FROM ubuntu:22.04
RUN apt update && \
    apt install -y build-essential bison flex

RUN mkdir /swarm
COPY . /swarm
RUN cd /swarm && \
    make -j$(nproc)

WORKDIR /swarm
