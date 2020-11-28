FROM ubuntu:18.04

MAINTAINER Wolfgang Kampichler <wolfgang.kampichler@dec112.at>
ENV REFRESHED_AT 2020-04-19

RUN rm -rf /var/lib/apt/lists/* && apt-get update &&   apt-get install --assume-yes gnupg wget
# kamailio repo
RUN echo "deb http://deb.kamailio.org/kamailiodev-nightly bionic main" >   /etc/apt/sources.list.d/kamailio.list
RUN wget -O- http://deb.kamailio.org/kamailiodebkey.gpg | apt-key add -

RUN apt-get update && apt-get install --assume-yes kamailio kamailio-autheph-modules kamailio-berkeley-bin kamailio-berkeley-modules kamailio-dbg kamailio-extra-modules kamailio-geoip-modules kamailio-geoip2-modules kamailio-json-modules kamailio-ldap-modules kamailio-lua-modules kamailio-memcached-modules kamailio-nth kamailio-outbound-modules kamailio-perl-modules kamailio-postgres-modules kamailio-radius-modules kamailio-redis-modules kamailio-sctp-modules kamailio-snmpstats-modules kamailio-sqlite-modules kamailio-systemd-modules kamailio-tls-modules kamailio-utils-modules kamailio-websocket-modules kamailio-xml-modules kamailio-xmpp-modules

VOLUME /etc/kamailio

# clean
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["kamailio", "-DD", "-E"]

