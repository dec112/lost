FROM ubuntu:18.04 as prfbuild

MAINTAINER Wolfgang Kampichler <wolfgang.kampichler@dec112.at>
ENV REFRESHED_AT 2020-04-19

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
            build-essential \
            pkg-config \
            ca-certificates \
            curl \
            liblog4c-dev \
            libsqlite3-dev \
            libspatialite-dev \
            libssl-dev \
            libxml2-dev \
            libyaml-dev \
            && \
    apt-get purge -y --auto-remove && rm -rf /var/lib/apt/lists/*

WORKDIR /app/

COPY ./src/* ./

RUN cd ./ && \
    make && \
    ls -lat

FROM ubuntu:18.04

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
            libssl1.1 \
            libsqlite3-0 \
            libspatialite7 \
            ca-certificates \
            liblog4c3 \
            libxml2 \
            libyaml-0-2 \
            tzdata \
            && \
    apt-get purge -y --auto-remove && rm -rf /var/lib/apt/lists/*

RUN ln -fs /usr/share/zoneinfo/Europe/Vienna /etc/localtime
RUN dpkg-reconfigure --frontend noninteractive tzdata

COPY --from=prfbuild /app/dec112lost /bin/dec112lost


