# DEC112/DEC112 2.0 Lost Service

__Guide to build the LosT Service from sources.__

For more about the DEC112 Project visit: [dec112.at](https://dec112.at)

```
Main Author: Wolfgang Kampichler

Support: <info@dec112.at>

Licence: GPLv2
```

## Overview

This is a step by step tutorial about how to build and install the DEC112 LoST Service using the sources downloaded from the repository.

## Prerequisites

To be able to compile the DEC112 LoST Service, make sure that you've installed or dowdowloaded the following:
* libxml2-dev libspatialite-dev, liblog4c-dev, sqlite3
* mongoose.c and mongoose.h ([mongoose](https://github.com/cesanta/mongoose))
* curl for testing (optional)

## SQLite Database

To create a GIS database, you may want to follow steps as explained in [Working with spatialite databases in QGIS](https://docs.qgis.org/2.8/de/docs/training_manual/databases/spatialite.html). Make sure that the table containing the geometry has the following structure and name. 

```
CREATE TABLE "boundaries" (
	`id`	INTEGER PRIMARY KEY AUTOINCREMENT,
	`name`	TEXT,
	`geom`	POLYGON
)
```
In addition DEC112 LoST requires tables `services`, `info`, and `mapping`. Table `services` lists the `urn:service` Namespace Specific String as defined in [RFC2142](https://tools.ietf.org/html/rfc2141) and [RFC5031](https://tools.ietf.org/html/rfc5031): 
```        
CREATE TABLE "services" (
	`id`	INTEGER NOT NULL,
	`nss`	TEXT,
	PRIMARY KEY(`id`)
)
```
Table `info` extends `boundaries` with some metainformation:
```
CREATE TABLE "info" (
	`boundary_id`	INTEGER NOT NULL,
	`country`	TEXT,
	`country_code`	TEXT,
	`region`	TEXT,
	`state`	TEXT,
	`county`	TEXT,
	`city`	TEXT,
	`district`	TEXT,
	`token`	TEXT
)
```
Table `mapping` provides the mapping of a boundary to a specific PSAP URI per Namespace Specific String (e.g. `urn:service:sos`, `urn:service:sos.police`, `urn:service:sos.ambulance`, ...):
```
CREATE TABLE "mapping" (
	`boundary_id`	INTEGER,
	`service_id`	INTEGER,
	`uri`	TEXT,
	`name`	TEXT,
	`dialstring`	TEXT
)
```
## Compiling the DEC112 LoST Service

1. Have a look at [Clone or download the repository](https://help.github.com/en/articles/cloning-a-repository)
2. `cd srs/`
3. `make` and `cp dec112lost ../bin`
4. `cd ../bin` and `./dec112lost -i 127.0.0.1 -p 8448` (usage: dec112lost -i <ip/domain str> -p <listening port>)
5. The folder `schema` contains HELD and LoST schema definitions for validation
6. Note: log4crc may require changes (refer to the example below):

```
<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE log4c SYSTEM "">
<log4c>
    <config>
        <bufsize>0</bufsize>
        <debug level="0"/>
        <nocleanup>0</nocleanup>
    </config>
        <category name="root" priority="trace"/>
	<appender name="syslog" type="syslog" layout="basic"/>
	<appender name="stdout" type="stream" layout="basic"/>
        <layout name="basic" type="basic"/>
        <layout name="dated" type="dated"/>
        <category name="lost" priority="info" appender="syslog" />
	<category name="lost.dgb" priority="debug" appender="stdout" />
</log4c>
```

## Using and testing the DEC112 LoST Service

The default SQLite databases is dec112-db-example.sqlite (located in `data`) and may be changed via #define SQLITE_DB_LOST "../data/dec112-db-example.sqlite" (sqlite.h). The folder `test` of the repository contains predefined LoST requests (refer to LoST `<findService>` according to [RFC5222 Section 8.3](https://tools.ietf.org/html/rfc5222#section-8.3)). The following command sends a request using the `test-findservice-plugtests.xml` request example
```
curl -H "Content-Type: application/lost+xml;charset=utf-8" -d @test-findservice-plugtests.xml -X POST http://localhost:8448/lost
```
with the following response returned by the server:
```
<?xml version="1.0"?>
<findServiceResponse xmlns="urn:ietf:params:xml:ns:lost1" xmlns:p2="http://www.opengis.net/gml">
  <mapping expires="2019-04-04T12:17:39+02:0" lastUpdated="2019-04-03T12:17:39+02:0" source="localhost" sourceId="wokllv0120181229">
    <displayName>police-02</displayName>
    <service>urn:service:sos.police</service>
    <serviceBoundaryReference source="localhost" key="215A1F9D4E1D4B66AD73C6BE1C5791A8"/>
    <uri>sip:psap-02-police@plugtests.net</uri>
    <serviceNumber>17</serviceNumber>
  </mapping>
  <path>
    <via source="localhost"/>
  </path>
  <locationUsed id="6020688f1ce1896d"/>
</findServiceResponse>
```

## Docker

__Guide to build a LoST Server Docker image.__

An image is simply created using the Dockerfile example [Dockerfile] (https://github.com/dec112/lost/blob/master/service/docker/Dockerfile) with the following commands.

```
cd service
cp docker/Dockerfile .
docker build --tag ecrf:1.0 .
```

