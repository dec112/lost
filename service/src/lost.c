/*
 * Copyright (C) 2018  <Wolfgang Kampichler>
 *
 * This file is part of dec112lost
 *
 * dec112lost is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dec112lost is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *  @file    lost.c
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief this file holds lost function definitions
 */

/******************************************************************* INCLUDE */

#include "dec112lost.h"
#include "lost.h"

/***************************************************************** FUNCTIONS */

/**
 *  @brief  lost server main http event handling (mongoose by cesanta)
 *
 *  @arg    struct mg_connection, int, void
 *  @return void
 */

void lost_handleHttpEvent(struct mg_connection *nc, int ev, void *p) {
    int iLength = 0;

    char *pchContent = NULL;

    struct http_message *hm;
    struct sockaddr_in server;
    struct mbuf *io = &nc->recv_mbuf;

    socklen_t c;

    p_req_t ptr = NULL;

    hm = (struct http_message *) p;
    c = sizeof(server);

    switch(ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/lost") == 0) {
// alloc request object
            ptr = lost_malloc();
            ptr->strIPAddr = strIPAddr;
// get the current time
            lost_getTimeInformation(&ptr->strTS, &ptr->strEX);
// get remote IP
            getpeername(nc->sock, (struct sockaddr*) &server, &c);

            LOG4DEBUG(pL, "http request from %s:%i", inet_ntoa(server.sin_addr),
                      ntohs(server.sin_port));

            ptr->strIDString = (char *)malloc(strlen(inet_ntoa(server.sin_addr)) + 1);
            snprintf(ptr->strIDString, strlen(inet_ntoa(server.sin_addr)) + 1,
                     "%s", inet_ntoa(server.sin_addr));

            if (ptr->strIDString) {
                ptr->strIDType = (char *)malloc(strlen(DEFAULT_TYPE) + 1);
                snprintf(ptr->strIDType, strlen(DEFAULT_TYPE) + 1, "%s", DEFAULT_TYPE);
            }
// dispatch 200 response
            pchContent = lost_dispatchResponse(ptr, &iLength, hm);

            if (pchContent) {
                lost_httpSuccessResponse(nc, iLength, pchContent);
            } else {
// 500 error response
                LOG4ERROR(pL, "internal error");

                lost_httpErrorResponse(nc, 500, (const char*)"Internal Server Error");
            }

// free resources
            if (pchContent) {
                free(pchContent);
            }
            lost_free(ptr);
            mbuf_remove(io, io->len);
        } else {
            LOG4ERROR(pL, "unknown path");

            lost_httpErrorResponse(nc, 500, (const char*)"Internal Server Error");
            mbuf_remove(io, io->len);
        }
        break;
    default:
        break;
    }
}

/**
 *  @brief  dispatches correct response body based on request received
 *
 *  @arg    p_req_t, int*, struct http_message
 *  @return char *
 */

char *lost_dispatchResponse(p_req_t ptr, int *lgth, struct http_message *hm) {
    int iRet;

    char *pchContent = NULL;

    xmlDocPtr doc = NULL;
    xmlNode *cur = NULL;

    xmlKeepBlanksDefault(1);

    *lgth = 0;

    xmlSetGenericErrorFunc(NULL, lost_xml_error);

// if exists, read xml body
    if (hm->body.len > 0) {
        doc = xmlReadMemory((char*)hm->body.p, hm->body.len, "request.xml", NULL, 0);
    }
    if (doc == NULL) {
// create error response

        LOG4ERROR(pL, "could not parse xml body");

    } else {
// validate lost request
        iRet = xmlRelaxNGValidateDoc(pValidrngctxt, doc);

        if (iRet > 0) {
// validation error

            LOG4ERROR(pL, "request fails to validate");

            pchContent = lost_xmlErrorResponse(ptr, &(*lgth),
                                          (const char*)"Badly formed or invalid XML",
                                          (const char*)"badRequest");

        } else if (iRet == 0) {
// request is ok

            LOG4DEBUG(pL, "request validates");

// get LoST request elements
            cur = xmlDocGetRootElement(doc);

            if (cur->type == XML_ELEMENT_NODE) {
// parse request elements
                if ((!xmlStrcmp(cur->name, BAD_CAST "findService"))) {
                    pchContent = lost_findServiceResponse(ptr, &(*lgth), doc);
                } else if ((!xmlStrcmp(cur->name, BAD_CAST "getServiceBoundary"))) {
                    pchContent = lost_getServiceBoundaryResponse(ptr, &(*lgth), doc);
                } else if ((!xmlStrcmp(cur->name, BAD_CAST "listServices"))) {
                    LOG4INFO(pL, "### listServices request");
                    pchContent = NULL;
                    //pchContent = lost_listServicesResponse(ptr, &(*lgth), doc);
                } else if ((!xmlStrcmp(cur->name, BAD_CAST "listServicesByLocation"))) {
                    LOG4INFO(pL, "### listServicesByLocation request");
                    pchContent = NULL;
                    //pchContent = lost_listServicesByLocationResponse(ptr, &(*lgth), doc);
                } else {
// nothing proper found ... create error response
                    pchContent = NULL;
                }
            }
        }
    }

    xmlFreeDoc(doc);

    return pchContent;
}

/**
 *  @brief  parses findService request and creates response
 *
 *  @arg    p_req_t, int*, xmlDocPtr
 *  @return char *
 */

char *lost_findServiceResponse(p_req_t ptr, int *lgth, xmlDocPtr doc) {
    int iRes;

    char *pchContent;
    char *pchNss;

    sqlite3 *db;
    sqlite3_stmt *stmt;

    xmlChar *strXml = NULL;
    xmlNode *cur = NULL;

    xmlKeepBlanksDefault(1);

    iRes = 0;
    *lgth = 0;
    pchContent = NULL;

// get LoST request name
    cur = xmlDocGetRootElement(doc);
    if (cur->type == XML_ELEMENT_NODE) {
// parse findService request elements
        if ((!xmlStrcmp(cur->name, BAD_CAST "findService"))) {
            strXml = xmlGetProp(cur, BAD_CAST "recursive");
            if ((!xmlStrcmp(strXml, BAD_CAST "true"))) {
                ptr->bRecursion = 1;
            }
            xmlFree(strXml);

            strXml = xmlGetProp(cur, BAD_CAST "serviceBoundary");
            if ((!xmlStrcmp(strXml, BAD_CAST "value"))) {
                ptr->bBoundary = 1;
            }
            if ((!xmlStrcmp(strXml, BAD_CAST "reference"))) {
                ptr->bBoundary = 0;
            }
            xmlFree(strXml);

            LOG4INFO(pL, "### findService request <%s%s>", (ptr->bBoundary == 1) ? "value" : "reference",
                     (ptr->bRecursion == 1) ? ", recursive" : "");

            cur = cur->xmlChildrenNode;
// return the first location element
            while (cur != NULL) {
                if (cur->type == XML_ELEMENT_NODE) {
                    if ((!xmlStrcmp(cur->name, BAD_CAST "location"))) {
                        ptr->strLocID = (char*)xmlGetProp(cur, BAD_CAST "id");
                        ptr->strLocProfile = (char*)xmlGetProp(cur, BAD_CAST "profile");

                        LOG4DEBUG(pL, "location element found");

                        if (lost_parseLocation(ptr, cur) < 0) {
                            pchContent = lost_xmlErrorResponse(ptr, &(*lgth),
                                                          (const char*)"The geodetic "
                                                          "location in the request "
                                                          "was invalid",
                                                          (const char*)"locationInvalid");
                            return pchContent;
                        }

                        if ((xmlStrcmp(BAD_CAST ptr->strLocProfile, BAD_CAST "geodetic-2d")) != 0) {
                            pchContent = lost_xmlErrorResponse(ptr, &(*lgth),
                                                          (const char*)"The location profile "
                                                          "in the request was not recognized",
                                                          (const char*)"locationProfileUnrecognized");
                            return pchContent;
                        }
                    }
// return the service URN
                    if ((!xmlStrcmp(cur->name, BAD_CAST "service"))) {
                        ptr->strURN = (char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                    }
                }
                cur = cur->next;
            }
        }
    }

    LOG4DEBUG(pL, "locID=%s", ptr->strLocID);
    LOG4DEBUG(pL, "locProfile=%s", ptr->strLocProfile);
    LOG4DEBUG(pL, "latitude=%s", ptr->strLatitude);
    LOG4DEBUG(pL, "longitude=%s", ptr->strLongitude);
    LOG4DEBUG(pL, "radius=%d", ptr->iRadius);
    LOG4DEBUG(pL, "URN=%s", ptr->strURN);

    if ((ptr->strLatitude) && (ptr->strLongitude) && (ptr->strLocID) && (ptr->strURN)) {
// get URN's name specific string
        pchNss = strrchr(ptr->strURN, ':');
        pchNss++;
// open database
        if (0 == sqliteFindService(ptr, pchNss)) {
            if (1 == ptr->bBoundary) {
                if (0 != sqliteServiceBoundary(ptr)) {
                    ptr->bBoundary = 0;
                }
            }
// create response
            pchContent = lost_xmlFindServiceResponse(ptr, &(*lgth));

            LOG4INFO(pL, "urn: %s", ptr->strURN);
            LOG4INFO(pL, "loc: %s,%s", ptr->strCountry, ptr->strState);
            LOG4INFO(pL, "geo: %.*s/%.*s/%d", 5, ptr->strLatitude, 5, ptr->strLongitude, ptr->iRadius);
            LOG4INFO(pL, "ecc: %s", ptr->strName);
            LOG4INFO(pL, "uri: %s", ptr->strURI);
            LOG4INFO(pL, "emn: %s", ptr->strNumber);
            LOG4INFO(pL, "tok: %s", ptr->strToken);
            LOG4INFO(pL, "### findServiceResponse <%d>", *lgth);
            //LOG4DEBUG(pL, "%s", pchContent);

        } else {
            if ((strstr(pchNss, "sos")) && (0 == sqliteFindService(ptr, "sos"))) {
                ptr->bSubstitution = 1;
                if (1 == ptr->bBoundary) {
                    if (0 != sqliteServiceBoundary(ptr)) {
                        ptr->bBoundary = 0;
                    }
                }
                pchContent = lost_xmlFindServiceResponse(ptr, &(*lgth));

                LOG4WARN(pL, "### service substitution: %s", ptr->strURN);
                LOG4INFO(pL, "urn: urn:service:sos");
                LOG4INFO(pL, "loc: %s,%s", ptr->strCountry, ptr->strState);
                LOG4INFO(pL, "geo: %.*s/%.*s/%d", 5, ptr->strLatitude, 5, ptr->strLongitude, ptr->iRadius);
                LOG4INFO(pL, "ecc: %s", ptr->strName);
                LOG4INFO(pL, "uri: %s", ptr->strURI);
                LOG4INFO(pL, "emn: %s", ptr->strNumber);
                LOG4INFO(pL, "tok: %s", ptr->strToken);
                LOG4INFO(pL, "### findServiceResponse <%d>", *lgth);
                //LOG4DEBUG(pL, "%s", pchContent);

            } else {
                // create error response
                pchContent = lost_xmlErrorResponse(ptr, &(*lgth),
                                              (const char*)"Service does not exist for "
                                              "the location indicated",
                                              (const char*)"serviceNotImplemented");

                LOG4INFO(pL, "no result, responding with error");
            }
        }
    }

    return pchContent;
}

/**
 *  @brief  parses getServiceBoundary request and creates response
 *
 *  @arg    p_req_t, int*, xmlDocPtr
 *  @return char *
 */

char *lost_getServiceBoundaryResponse(p_req_t ptr, int *lgth, xmlDocPtr doc) {
    char strQuery[1024];
    char *pchPolygon = NULL;
    char *pchContent = NULL;

    xmlNode *cur = NULL;

    xmlKeepBlanksDefault(1);

    *lgth = 0;
    pchContent = NULL;

// get LoST request
    cur = xmlDocGetRootElement(doc);
    if (cur->type == XML_ELEMENT_NODE) {
// parse findService Elements
        if ((!xmlStrcmp(cur->name, BAD_CAST "getServiceBoundary"))) {

            LOG4INFO(pL, "### getServiceBoundary request");

            ptr->strToken = (char*)xmlGetProp(cur, BAD_CAST "key");
        }
    }

    LOG4DEBUG(pL, "key=%s", ptr->strToken);

    if (ptr->strToken) {
// open database
        if (0 == sqliteServiceBoundary(ptr)) {
// create response
            pchContent = lost_xmlGetServiceBoundaryResponse(ptr, &(*lgth));

            LOG4INFO(pL, "tok: %s", ptr->strToken);

        } else {
// create error response
            pchContent = lost_xmlErrorResponse(ptr, &(*lgth),
                                          (const char*)"The location key in the "
                                          "request was invalid",
                                          (const char*)"locationInvalid");

            LOG4ERROR(pL, "no result, responding with error");

        }

        LOG4INFO(pL, "### getServiceBoundaryResponse <%d>", *lgth);
        //LOG4DEBUG(pL, "%s", pchContent);
    }

    return pchContent;
}

/**
 *  @brief  error response
 *
 *  @arg    p_req_t, int*, const char*, const char*
 *  @return char*
 */

char* lost_xmlErrorResponse(p_req_t ptr, int *lgth, const char *message,
                         const char *message_type) {
    int iSize = 0;
    char strBuf[BUFSIZE];
    char *pchContent;

    xmlChar *xmlBuf;
    xmlDocPtr xmlDoc = NULL;
    xmlNodePtr root = NULL;
    xmlNodePtr node_0 = NULL;

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");

    root = xmlNewNode(NULL, BAD_CAST "errors");
    xmlDocSetRootElement(xmlDoc, root);
    xmlNewProp(root, BAD_CAST "xmlns", BAD_CAST "urn:ietf:params:xml:ns:lost1");
    snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
    xmlNewProp(root, BAD_CAST "source", BAD_CAST strBuf);

    node_0 = xmlNewChild(root, NULL, BAD_CAST message_type, NULL);
    xmlNewProp(node_0, BAD_CAST "message", BAD_CAST message);
    xmlNewProp(node_0, BAD_CAST "xml:lang", BAD_CAST "en");
    xmlDocDumpFormatMemory(xmlDoc, &xmlBuf, &iSize, 1);

    pchContent = (char*)malloc(iSize+1);
    *lgth = snprintf(pchContent, iSize+1, "%s", (char *)xmlBuf);

    xmlFree(xmlBuf);
    xmlFreeDoc(xmlDoc);

    return pchContent;
}

/**
 *  @brief  find service response body (RFC5222 Section 8)
 *
 *  @arg    p_req_t, int*
 *  @return char*
 */

char* lost_xmlFindServiceResponse(p_req_t ptr, int *lgth) {
    int iSize = 0;
    char strBuf[BUFSIZE];
    char *pchContent;
    char *pPolygonSplit;
    char *pt;

    char delimiter[] = ",)";

    char latitude[32];
    char longitude[32];

    *lgth = 0;

    xmlChar *xmlBuf;
    xmlDocPtr xmlDoc = NULL;
    xmlNodePtr root = NULL;
    xmlNodePtr node_0 = NULL;
    xmlNodePtr node_1 = NULL;
    xmlNodePtr node_2 = NULL;
    xmlNodePtr node_3 = NULL;
    xmlNodePtr node_4 = NULL;
    xmlNodePtr node_5 = NULL;
    xmlNodePtr node_6 = NULL;
    xmlNodePtr node_7 = NULL;
    xmlNodePtr node_8 = NULL;
    xmlNodePtr node_9 = NULL;
    xmlNodePtr node_10 = NULL;

// create response
    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
// findServiceResponse
    root = xmlNewNode(NULL, BAD_CAST "findServiceResponse");
    xmlDocSetRootElement(xmlDoc, root);
    xmlNewProp(root, BAD_CAST "xmlns", BAD_CAST "urn:ietf:params:xml:ns:lost1");
    xmlNewProp(root, BAD_CAST "xmlns:p2", BAD_CAST "http://www.opengis.net/gml");
// mapping
    node_0 = xmlNewChild(root, NULL, BAD_CAST "mapping", NULL);
    xmlNewProp(node_0, BAD_CAST "expires", BAD_CAST ptr->strEX);
    xmlNewProp(node_0, BAD_CAST "lastUpdated", BAD_CAST ptr->strTS);
    snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
    xmlNewProp(node_0, BAD_CAST "source", BAD_CAST strBuf);
    snprintf(strBuf, BUFSIZE, "wklostv0120190120");
    xmlNewProp(node_0, BAD_CAST "sourceId", BAD_CAST strBuf);
// displayName
    snprintf(strBuf, BUFSIZE, "%s", ptr->strName);
    xmlNewChild(node_0, NULL, BAD_CAST "displayName", BAD_CAST strBuf);
// service
    if (0 == ptr->bSubstitution) {
        snprintf(strBuf, BUFSIZE, "%s", ptr->strURN);
        xmlNewChild(node_0, NULL, BAD_CAST "service", BAD_CAST strBuf);
    } else {
        snprintf(strBuf, BUFSIZE, "urn:service:sos");
        xmlNewChild(node_0, NULL, BAD_CAST "service", BAD_CAST strBuf);
    }
    if (0 == ptr->bBoundary) {
        // serviceBoundaryReference
        node_1 = xmlNewChild(node_0, NULL, BAD_CAST "serviceBoundaryReference", NULL);
        snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
        xmlNewProp(node_1, BAD_CAST "source", BAD_CAST strBuf);
        snprintf(strBuf, BUFSIZE, "%s", ptr->strToken);
        xmlNewProp(node_1, BAD_CAST "key", BAD_CAST strBuf);
    } else {
        // serviceBoundary
        node_7 = xmlNewChild(node_0, NULL, BAD_CAST "serviceBoundary", NULL);
        xmlNewProp(node_7, BAD_CAST "profile", BAD_CAST "geodetic-2d");
        // p2:Polygon
        node_8 = xmlNewChild(node_7, NULL, BAD_CAST "p2:polygon", NULL);
        xmlNewProp(node_8, BAD_CAST "srsName", BAD_CAST "urn:ogc:def::crs:EPSG::4326");
        // p2:exterior
        node_9 = xmlNewChild(node_8, NULL, BAD_CAST "p2:exterior", NULL);
        // p2:LinearRing
        node_10 = xmlNewChild(node_9, NULL, BAD_CAST "p2:LinearRing", NULL);
        // p2:pos
        pPolygonSplit = strrchr(ptr->pchPolygon, '(') + 1; //remove leading part of response
        pt = strtok(pPolygonSplit, delimiter);
        while(pt != NULL) {
            sscanf(pt,"%s %s", longitude, latitude);
            snprintf(strBuf, BUFSIZE, "%s %s", latitude, longitude);
            xmlNewChild(node_10, NULL, BAD_CAST "p2:pos", BAD_CAST strBuf);
            pt = strtok(NULL, delimiter);
        }
    }
// uri
    snprintf(strBuf, BUFSIZE, "%s", ptr->strURI);
    xmlNewChild(node_0, NULL, BAD_CAST "uri", BAD_CAST strBuf);
// serviceNumber
    snprintf(strBuf, BUFSIZE, "%s", ptr->strNumber);
    xmlNewChild(node_0, NULL, BAD_CAST "serviceNumber", BAD_CAST strBuf);
// warnings
    if (1 == ptr->bSubstitution) {
        node_5 = xmlNewChild(root, NULL, BAD_CAST "warnings", NULL);
        snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
        xmlNewProp(node_5, BAD_CAST "source", BAD_CAST strBuf);
        node_6 = xmlNewChild(node_5, NULL, BAD_CAST "serviceSubstitution", NULL);
        snprintf(strBuf, BUFSIZE, "Unable to determine PSAP for %s; using urn:service:sos", ptr->strURN);
        xmlNewProp(node_6, BAD_CAST "message", BAD_CAST strBuf);
        xmlNewProp(node_6, BAD_CAST "xml:lang", BAD_CAST "en");
    }
// path
    node_2 = xmlNewChild(root, NULL, BAD_CAST "path", NULL);
// via
    node_3 = xmlNewChild(node_2, NULL, BAD_CAST "via", NULL);
    snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
    xmlNewProp(node_3, BAD_CAST "source", BAD_CAST strBuf);
// locationUsed
    node_4 = xmlNewChild(root, NULL, BAD_CAST "locationUsed", NULL);
    snprintf(strBuf, BUFSIZE, "%s", ptr->strLocID);
    xmlNewProp(node_4, BAD_CAST "id", BAD_CAST strBuf);
// write document
    xmlDocDumpFormatMemoryEnc(xmlDoc, &xmlBuf, &iSize, "UTF-8", 1);

    pchContent = (char*)malloc(iSize+1);
    *lgth = snprintf(pchContent, iSize+1, "%s", (char *)xmlBuf);

    xmlFree(xmlBuf);
    xmlFreeDoc(xmlDoc);

    return pchContent;
}

/**
 *  @brief  lost get service boundary response body (RFC5222 Section 9)
 *
 *  @arg    p_req_t, int*
 *  @return char*
 */

char* lost_xmlGetServiceBoundaryResponse(p_req_t ptr, int *lgth) {
    int iSize = 0;
    char strBuf[BUFSIZE];
    char *pchContent;
    char *pPolygonSplit;
    char *pt;

    char delimiter[] = ",)";

    char latitude[32];
    char longitude[32];

    *lgth = 0;

    xmlChar *xmlBuf;
    xmlDocPtr xmlDoc = NULL;
    xmlNodePtr root = NULL;
    xmlNodePtr node_0 = NULL;
    xmlNodePtr node_1 = NULL;
    xmlNodePtr node_2 = NULL;
    xmlNodePtr node_3 = NULL;
    xmlNodePtr node_4 = NULL;
    xmlNodePtr node_5 = NULL;

// create response
    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
// findServiceResponse
    root = xmlNewNode(NULL, BAD_CAST "getServiceBoundaryResponse");
    xmlDocSetRootElement(xmlDoc, root);
    xmlNewProp(root, BAD_CAST "xmlns", BAD_CAST "urn:ietf:params:xml:ns:lost1");
    xmlNewProp(root, BAD_CAST "xmlns:p2", BAD_CAST "http://www.opengis.net/gml");
// serviceBoundary
    node_0 = xmlNewChild(root, NULL, BAD_CAST "serviceBoundary", NULL);
    xmlNewProp(node_0, BAD_CAST "profile", BAD_CAST "geodetic-2d");
// p2:Polygon
    node_1 = xmlNewChild(node_0, NULL, BAD_CAST "p2:polygon", NULL);
    xmlNewProp(node_1, BAD_CAST "srsName", BAD_CAST "urn:ogc:def::crs:EPSG::4326");
// p2:exterior
    node_2 = xmlNewChild(node_1, NULL, BAD_CAST "p2:exterior", NULL);
// p2:LinearRing
    node_3 = xmlNewChild(node_2, NULL, BAD_CAST "p2:LinearRing", NULL);
// p2:pos
    pPolygonSplit = strrchr(ptr->pchPolygon, '(') + 1; //remove leading part of response
    pt = strtok(pPolygonSplit, delimiter);
    while(pt != NULL) {
        sscanf(pt,"%s %s", longitude, latitude);
        snprintf(strBuf, BUFSIZE, "%s %s", latitude, longitude);
        xmlNewChild(node_3, NULL, BAD_CAST "p2:pos", BAD_CAST strBuf);
        pt = strtok(NULL, delimiter);
    }
// path
    node_4 = xmlNewChild(root, NULL, BAD_CAST "path", NULL);
// via
    node_5 = xmlNewChild(node_4, NULL, BAD_CAST "via", NULL);
    snprintf(strBuf, BUFSIZE, "%s", ptr->strIPAddr);
    xmlNewProp(node_5, BAD_CAST "source", BAD_CAST strBuf);
// write document
    xmlDocDumpFormatMemoryEnc(xmlDoc, &xmlBuf, &iSize, "UTF-8", 1);

    pchContent = (char*)malloc(iSize+1);
    *lgth = snprintf(pchContent, iSize+1, "%s", (char *)xmlBuf);

    xmlFree(xmlBuf);
    xmlFreeDoc(xmlDoc);

    return pchContent;
}

static void lost_xml_error(void *user_data, const char *msg, ...) {
    va_list args;

    va_start(args, msg);
    LOG4ERROR(pL, msg, args);
    va_end(args);
}
