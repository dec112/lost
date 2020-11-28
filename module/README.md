# DEC112 Kamailio LoST Module

__Guide to build the LosT Service from sources.__

For more about the DEC112 Project visit: [dec112.at](https://dec112.at)

```
Main Author: Wolfgang Kampichler

Support: <info@dec112.at>

Licence: GPLv2
```

## Note

__Guide to build the LoST Module from sources.__

The LoST Module, which was developed in the course of DEC112 and DEC112 2.0, is now part of Kamailio (version 5.4 and higher). Please visit [Kamailio](https://github.com/kamailio/kamailio/tree/5.4/src/modules/lost) for further details.

## Docker

__Guide to build a Kamailio Docker image including the LoST Module.__

An image is simply created using the Dockerfile example [Dockerfile] (https://github.com/dec112/lost/blob/master/module/docker/Dockerfile) with the following commands.

```
cd docker
docker build --tag esrp:1.0 .
```

