FROM ubuntu:22.04
RUN apt update
RUN apt install -y build-essential bison flex
