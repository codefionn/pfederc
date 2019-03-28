FROM ubuntu
COPY ./src            /usr/src/pfederc/src
COPY ./include        /usr/src/pfederc/include
COPY ./CMakeLists.txt /usr/src/pfederc
COPY ./test           /usr/src/pfederc/test
WORKDIR /usr/src/pfederc/build

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get -y update  && apt-get -y upgrade \
 && apt-get -y install g++ cmake libllvm7 \
 && rm -rf /var/lib/apt/lists/*
RUN cmake .. 
RUN cmake --build .

CMD [ "./pfederc" ]
