FROM ubuntu:latest

WORKDIR /app

RUN apt update &&\
    apt-get install -y\
    git >=2.41\
    gcc >=13.2\
    make >=4.4\
    linux-tools-common linux-tools-generic

COPY . /app

ENV LD_LIBRARY_PATH="cddlib/lib-src/.libs/:${LD_LIBRARY_PATH}"
RUN make all
RUN echo $LD_LIBRARY_PATH
