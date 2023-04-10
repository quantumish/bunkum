
FROM ubuntu:latest
RUN apt-get update
# RUN apt-get install -y zlib1g
# RUN apt-get install -y zlib1g-dev

WORKDIR /root
COPY public public

# # for compilation
# COPY utils utils
# COPY http http
# COPY Makefile .
# COPY http.c .
# RUN make

COPY serv .
RUN chmod +x ./serv
ENTRYPOINT ["./serv"]
