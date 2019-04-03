/*
 * lost Module script functions
 *
 * Copyright (C) 2018 Wolfgang Kampichler, DEC112
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*!
 * \file
 * \brief Kamailio lost :: utilities
 * \ingroup lost
 * Module: \ref lost
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "../../core/parser/msg_parser.h"
#include "../../core/parser/parse_content.h"
#include "../../core/parser/parse_body.h"
#include "../../core/parser/parse_uri.h"
#include "../../core/dprint.h"
#include "../../core/mem/mem.h"
#include "../../core/mem/shm_mem.h"

#include "pidf.h"
#include "utilities.h"

#define BUFSIZE 	128

/*****************/
char *lost_trim_content(char *str, int *lgth) {
    char *end;

    while(isspace(*str))
        str++;

    if(*str == 0)
        return NULL;

    end = str + strlen(str) - 1;

    while(end > str && isspace(*end))
        end--;

    *(end + 1) = '\0';

    *lgth = (end + 1) - str;

    return str;
}


/*****************/
void lost_rand_str(char *dest, size_t length) {
    size_t index;
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    srand(time(NULL));
    while (length-- > 0) {
        index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void lost_free_loc(p_loc_t ptr) {
    pkg_free(ptr->identity);
    pkg_free(ptr->urn);
    pkg_free(ptr->longitude);
    pkg_free(ptr->latitude);
    pkg_free(ptr);
}

/*****************/
p_loc_t lost_new_loc(str rurn) {
    s_loc_t *ptr;
    char *id;
    char *urn;

    ptr = (s_loc_t *)pkg_malloc(sizeof(s_loc_t));
    if (ptr== NULL) {
        LM_ERR("No more private memory\n");
    }


    id = (char *)pkg_malloc(16 * sizeof(char) + 1);
    if (id== NULL) {
        LM_ERR("No more private memory\n");
    }

    urn = (char *)pkg_malloc(rurn.len + 1);
    if (urn== NULL) {
        LM_ERR("No more private memory\n");
    }

    memset(urn, 0, rurn.len + 1);
    memcpy(urn, rurn.s, rurn.len);
    urn[rurn.len] = '\0';

    lost_rand_str(id, 16);

    ptr->identity = id;
    ptr->urn = urn;
    ptr->longitude = NULL;
    ptr->latitude = NULL;
    ptr->radius = 0;
    ptr->recursive = 0;

    return ptr;
}

/*****************/
char* lost_find_service_request(p_loc_t loc, int *lgth) {
    int buffersize = 0;

    char buf[BUFSIZE];
    char *doc = NULL;

    xmlChar *xmlbuff = NULL;
    xmlDocPtr request = NULL;

    xmlNodePtr ptrFindService = NULL;
    xmlNodePtr ptrLocation = NULL;
    xmlNodePtr ptrPoint = NULL;
    xmlNodePtr ptrCircle = NULL;
    xmlNodePtr ptrRadius = NULL;

    xmlKeepBlanksDefault(1);
    *lgth = 0;

// create request
    request = xmlNewDoc(BAD_CAST "1.0");
// findService - element
    ptrFindService = xmlNewNode(NULL, BAD_CAST "findService");
    xmlDocSetRootElement(request, ptrFindService);
// set properties
    xmlNewProp(ptrFindService, BAD_CAST "xmlns", BAD_CAST "urn:ietf:params:xml:ns:lost1");
    xmlNewProp(ptrFindService, BAD_CAST "xmlns:p2", BAD_CAST "http://www.opengis.net/gml");
    xmlNewProp(ptrFindService, BAD_CAST "serviceBoundary", BAD_CAST "reference");
    xmlNewProp(ptrFindService, BAD_CAST "recursive", BAD_CAST "true");
// location - element
    ptrLocation = xmlNewChild(ptrFindService, NULL, BAD_CAST "location", NULL);
    xmlNewProp(ptrLocation, BAD_CAST "id", BAD_CAST loc->identity);
    xmlNewProp(ptrLocation, BAD_CAST "profile", BAD_CAST "geodetic-2d");
// set pos
    snprintf(buf, BUFSIZE, "%s %s", loc->latitude, loc->longitude);
// Point
    if (loc->radius == 0) {
        ptrPoint = xmlNewChild(ptrLocation, NULL, BAD_CAST "Point", NULL);
        xmlNewProp(ptrPoint, BAD_CAST "xmlns", BAD_CAST "http://www.opengis.net/gml");
        xmlNewProp(ptrPoint, BAD_CAST "srsName", BAD_CAST "urn:ogc:def:crs:EPSG::4326");
// pos
        xmlNewChild(ptrPoint, NULL, BAD_CAST "pos", BAD_CAST buf);
    } else {
// Circle - Point
        ptrCircle = xmlNewChild(ptrLocation, NULL, BAD_CAST "gs:Circle", NULL);
        xmlNewProp(ptrCircle, BAD_CAST "xmlns:gml", BAD_CAST "http://www.opengis.net/gml");
        xmlNewProp(ptrCircle, BAD_CAST "xmlns:gs", BAD_CAST "http://www.opengis.net/pidflo/1.0");
        xmlNewProp(ptrCircle, BAD_CAST "srsName", BAD_CAST "urn:ogc:def:crs:EPSG::4326");
// pos
        xmlNewChild(ptrCircle, NULL, BAD_CAST "gml:pos", BAD_CAST buf);
// Circle - radius
        snprintf(buf, BUFSIZE, "%d", loc->radius);
        ptrRadius = xmlNewChild(ptrCircle, NULL, BAD_CAST "gs:radius", BAD_CAST buf);
        xmlNewProp(ptrRadius, BAD_CAST "uom", BAD_CAST "urn:ogc:def:uom:EPSG::9001");
    }
// service - element
    snprintf(buf, BUFSIZE, "%s", loc->urn);
    xmlNewChild(ptrFindService, NULL, BAD_CAST "service", BAD_CAST buf);

    xmlDocDumpFormatMemory(request, &xmlbuff, &buffersize, 0);

    doc = (char*)pkg_malloc((buffersize + 1) * sizeof(char));
    if (doc == NULL) {
        LM_ERR("No more private memory\n");
    }

    memset(doc, 0, buffersize + 1);
    memcpy(doc, (char *)xmlbuff, buffersize);
    doc[buffersize] = '\0';

    *lgth = strlen(doc);

    xmlFree(xmlbuff);
    xmlFreeDoc(request);

    return doc;
}

char* lost_get_content(xmlNodePtr node, const char *name, int *lgth) {
    xmlNodePtr cur = node;
    char *content;
    char *cnt = NULL;

    int len;

    *lgth = 0;
    content = xmlNodeGetNodeContentByName(cur, name, NULL);
    len = strlen(content);

    cnt = (char*)pkg_malloc((len + 1) * sizeof(char));
    if (cnt == NULL) {
        LM_ERR("No more private memory\n");
    }

    memset(cnt, 0, len + 1);
    memcpy(cnt, content, len);
    cnt[len] = '\0';

    *lgth = strlen(cnt);

    xmlFree(content);

    return cnt;
}

char* lost_get_property(xmlNodePtr node, const char *name, int *lgth) {
    xmlNodePtr cur = node;
    char *content;
    char *cnt = NULL;

    int len;

    *lgth = 0;
    content = xmlNodeGetAttrContentByName(cur, name);
    len = strlen(content);

    cnt = (char*)pkg_malloc((len + 1) * sizeof(char));
    if (cnt == NULL) {
        LM_ERR("No more private memory\n");
    }

    memset(cnt, 0, len + 1);
    memcpy(cnt, content, len);
    cnt[len] = '\0';

    *lgth = strlen(cnt);

    xmlFree(content);

    return cnt;
}

char* lost_get_geolocation_header(struct sip_msg* msg, int *lgth) {
    struct hdr_field* hf;
    char *res = NULL;

    *lgth = 0;

    parse_headers(msg, HDR_EOH_F, 0);

    for (hf = msg->headers; hf; hf = hf->next) {
        if ((hf->type == HDR_OTHER_T) && (hf->name.len == LOST_GEOLOC_HEADER_SIZE - 2)) {
            // possible hit
            if (strncasecmp(hf->name.s, LOST_GEOLOC_HEADER, LOST_GEOLOC_HEADER_SIZE) == 0) {

                res = (char*)pkg_malloc((hf->body.len + 1) * sizeof(char));
                if (res == NULL) {
                    LM_ERR("No more private memory\n");
                } else {
                    memset(res, 0, hf->body.len + 1);
                    memcpy(res, hf->body.s, hf->body.len + 1);
                    res[hf->body.len] = '\0';

                    *lgth = strlen(res);
                }
            } else {
                LM_ERR("header '%.*s' length %d\n", hf->body.len, hf->body.s, hf->body.len);
            }
            break;
        }
    }

    return res;
}

char* lost_get_childname(xmlNodePtr node, const char *name, int *lgth) {
    xmlNodePtr cur = node;
    xmlNodePtr parent = NULL;
    xmlNodePtr child = NULL;

    char *cnt = NULL;
    int len;

    *lgth = 0;

    parent = xmlNodeGetNodeByName(cur, name, NULL);
    child = parent->children;

    if (child) {
        len = strlen((char *)child->name);

        cnt = (char*)pkg_malloc((len + 1) * sizeof(char));
        if (cnt == NULL) {
            LM_ERR("No more private memory\n");
        }

        memset(cnt, 0, len + 1);
        memcpy(cnt, child->name, len);
        cnt[len] = '\0';

        *lgth = strlen(cnt);
    }

    return cnt;
}

char* lost_get_name(char *name, int *lgth) {

    char *cnt = NULL;
    int len;

    *lgth = 0;

    if (name) {
        len = strlen(name);

        cnt = (char*)pkg_malloc((len + 1) * sizeof(char));
        if (cnt == NULL) {
            LM_ERR("No more private memory\n");
        }

        memset(cnt, 0, len + 1);
        memcpy(cnt, name, len);
        cnt[len] = '\0';

        *lgth = strlen(cnt);
    }

    return cnt;
}



