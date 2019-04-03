# DEC112 Kamailio LoST Module

__Guide to build the LoST Module from sources.__

For more about the DEC112 Project visit: [dec112.at](https://dec112.at)

```
Main Author: Wolfgang Kampichler

Support: <info@dec112.at>

Licence: GNUv2
```

## Overview

This is a step by step tutorial about how to build and install the DEC112 LoST module using the sources downloaded from the repository.

## Prerequisites

To be able to install the DEC112 LoST module, make sure that you've dowloaded and installed latest kamailio sources - you may want to follow steps as explained in the
[Guide to install Kamailio SIP Server v5.2 (devel) from Git repository](https://kamailio.org/docs/tutorials/5.3.x/kamailio-install-guide-git/). Please note that the DEC112 LoST module requires __libxml2__.

## Compiling the DEC112 LoST module

1. Have a look at [Clone or download the repository](https://help.github.com/en/articles/cloning-a-repository)
2. copy the lost folder to /[kamailio-src-root]/src/modules/
3. `make modules modules=src/modules/lost` (in the [kamailio-src-root] folder)
4. copy /[kamailio-src-root]/src/modules/lost/lost.so to your kamailio lib folder (see the guide to install kamailio)

## Using the DEC112 LoST module

The DEC112 LoST module exports three functions, lost_query, lost_query_urn and lost_response.

### *lost_query(result)*
lost_query returns a LoST `<findService>` Request XML object according to [RFC5222 Section 8.3](https://tools.ietf.org/html/rfc5222#section-8.3) stored in the writeable variable (result). The service URN is taken from the request line and the location from the multipart MIME body (PIDF-LO)

### *lost_query_urn(urn, result)*
lost_query_urn returns a LoST `<findService>` Request XML object according to [RFC5222 Section 8.3](https://tools.ietf.org/html/rfc5222#section-8.3) stored in the writeable variable (result). The service URN is taken from the urn variable and the location from the multipart MIME body (PIDF-LO). This function may be useful in case the request does not contain a service urn, or a different urn shall be used. 

### *lost_response(result, target, display-name)*
lost_response parses the a LoST `<findService>` Response message (XML object) according to [RFC5222 Section 8.4](https://tools.ietf.org/html/rfc5222#section-8.4) stored in the read/writeable variable (result) and returns the target sip uri (`<uri>`) stored in the writeable variable (target) and the display name (`<displayName>`) stored in the writeable variable (display-name).

## Kamailio configuration examples
The DEC112 LoST module utilizes the http_client module, which has to be loaded together with the DEC112 lost module

```
loadmodule "http_client.so"
loadmodule "lost.so"
```
The following initializes the http_client module assuming a LoST Service conform to [RFC5222](https://tools.ietf.org/html/rfc5222) runs at localhost:8448, with a connection timeout set to 1s.
```
modparam("http_client", "httpcon", "lostserver=>http://127.0.0.1:8448");
modparam("http_client", "connection_timeout", 1);
```
The snippet below shows how DEC112 lost module functions are used in the main routing logic, with the first part checking if a service urn is part of the request line (see pvar $rz) and the second part assuming a specific 3-digit emergency number (e.g. 112).

```
        if($rz=~"^urn$") {
                lost_query("$var(fsrequest)");
        } else if($rU=~"^112$") {
                $var(myurn) = "urn:service:sos";
                lost_query_urn("$var(myurn)", "$var(fsrequest)");
        }
```
The follwing takes the LoST `<findService>` Response message (XML object returned as $var(fsrequest)) as http POST message body input (refer to http_client module examples).

```        
       $var(res) = http_connect("lostserver", "/lost", "application/lost+xml", "$var(fsrequest)", "$var(fsresponse)");
       
       lost_response("$var(fsresponse)", "$var(target)", "$var(display)");
```
Finally, $var(target) may be used for request routing or relaying. 
