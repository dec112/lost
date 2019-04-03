/*
 * Copyright (C) 2018 DEC112, Wolfgang Kampichler
 *
 * This file is part of dec112lost
 *
 * dec112lost is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * dec112lost is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */

/**
 *  @file    helper.c
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief this file holds generic helper function definitions
 */

/******************************************************************* INCLUDE */

#include "helper.h"

/******************************************************************* GLOBALS */

const char crlf[] = {'\x0d', '\x0a', 0};

/***************************************************************** FUNCTIONS */


/**
 *  @brief  sends http error response
 *
 *  @arg    struct mg_connection*, int, const char*
 *  @return void
 */

void lost_httpErrorResponse(struct mg_connection *nc, int err_number,
                            const char *message) {
    LOG4ERROR(pL, "responding with: %i %s", err_number, message);
    mg_printf(nc,
              "HTTP/1.1 %i %s%s"
              SERVER_STR
              "Content-Length: 0%s%s",
              err_number, message, crlf,
              crlf, crlf);
    return;
}

/**
 *  @brief  sends http success response
 *
 *  @arg    struct mg_connection*, int, char*
 *  @return void
 */

void lost_httpSuccessResponse(struct mg_connection *nc, int iLength,
                              char *pchContent) {
    mg_printf(nc,
              "HTTP/1.1 200 OK%s"
              SERVER_STR
              "Content-Type: application/lost+xml%s"
              "Content-Length: %d%s%s"
              "%s",
              crlf,
              crlf,
              iLength, crlf, crlf,
              pchContent);
    return;
}

/**
 *  @brief  returns internal server error response
 *
 *  @arg    void
 *  @return char*
 */

char* lost_httpInternalServerError(void) {
    return  "HTTP/1.1 500 Internal Server Error\x0d\x0a"
            SERVER_STR
            "Content-Length: 0\x0d\x0a"
            "\x0d\x0a";
}

/**
 *  @brief  sets current time and expire time (+24h)
 *
 *  @arg    char**, char**
 *  @return void
 */

void lost_getTimeInformation(char **strTS, char **strEX) {
    time_t curtime;
    struct tm *loctime;

    char bufTT[64];
    char bufTS[128];
    char bufEX[128];

    curtime = time (NULL);
// convert to local time representation
    loctime = localtime (&curtime);
// print required format
    strftime (bufTT, 64, "%FT%T", loctime);

    if (loctime->tm_gmtoff > 0) {
        snprintf(bufTS, 128, "%s+%02i:%02i", bufTT, (int)loctime->tm_gmtoff/3600,
                 (int)(loctime->tm_gmtoff%3600) / 60);
    } else {
        snprintf(bufTS, 128, "%s-%02i:%02i", bufTT, (int)loctime->tm_gmtoff/3600,
                 (int)(loctime->tm_gmtoff%3600) / 60);
    }

    *strTS = (char *)malloc(strlen((char *)bufTS) + 1);
    snprintf(*strTS, strlen((char *)bufTS), "%s", (char *)bufTS);

// calculate expires +24h
    loctime->tm_hour += 24;
    mktime(loctime);
    strftime (bufTT, 64, "%FT%T", loctime);

    if (loctime->tm_gmtoff > 0) {
        snprintf(bufEX, 128, "%s+%02i:%02i", bufTT, (int)loctime->tm_gmtoff/3600,
                 (int)(loctime->tm_gmtoff%3600) / 60);
    } else {
        snprintf(bufEX, 128, "%s-%02i:%02i", bufTT, (int)loctime->tm_gmtoff/3600,
                 (int)(loctime->tm_gmtoff%3600) / 60);
    }

    *strEX = (char *)malloc(strlen((char *)bufEX) + 1);
    snprintf(*strEX, strlen((char *)bufEX), "%s", (char *)bufEX);

    return;
}

/**
 *  @brief  parses location element (point or circle) and sets lat / long / radius
 *
 *  @arg    p_req_t, xmlNodePtr
 *  @return int
 */

int lost_parseLocation(p_req_t ptr, xmlNodePtr node) {
    int iRadius = 0;
    int iReturn = -1;

    char chLat[BUFSIZE];
    char chLon[BUFSIZE];
    char *pchContent = NULL;

    xmlNodePtr cur = node;

    pchContent = xmlNodeGetNodeContentByName(cur, "pos", NULL);

    if (pchContent) {
        sscanf(pchContent, "%s %s", chLat, chLon);

        ptr->strLatitude = (char *)malloc(strlen((char *)chLat) + 1);
        snprintf(ptr->strLatitude, strlen((char *)chLat) + 1, "%s", (char *)chLat);

        ptr->strLongitude = (char *)malloc(strlen((char *)chLon) + 1);
        snprintf(ptr->strLongitude, strlen((char *)chLon) + 1, "%s", (char *)chLon);

        ptr->iRadius = iRadius;
        iReturn = 0;

        xmlFree(pchContent);
    }

    pchContent = xmlNodeGetNodeContentByName(cur, "radius", NULL);

    if (pchContent) {
        sscanf(pchContent, "%d", &iRadius);

        ptr->iRadius= iRadius;
        iReturn = 0;

        xmlFree(pchContent);
    }

    if (iReturn < 0) {
        LOG4ERROR(pL, "could not parse location information\n");
    }

    return iReturn;
}

/**
 *  @brief  allocates memory and initializes p_req_t object
 *
 *  @arg    void
 *  @return p_req_t
 */

p_req_t lost_malloc(void) {
    p_req_t ptr;

    ptr = (s_req_t *)malloc(sizeof(s_req_t));

    ptr->strIDType = NULL;
    ptr->strIDString = NULL;
    ptr->strIPAddr = NULL;
    ptr->strTS = NULL;
    ptr->strEX = NULL;
    ptr->strLocID = NULL;
    ptr->strLocProfile = NULL;
    ptr->strReqType = NULL;
    ptr->strURN = NULL;
    ptr->strURI = NULL;
    ptr->strNumber = NULL;
    ptr->strName = NULL;
    ptr->strCountry = NULL;
    ptr->strState = NULL;
    ptr->strToken = NULL;
    ptr->strLatitude = NULL;
    ptr->strLongitude = NULL;
    ptr->pchPolygon = NULL;
    ptr->iRadius = 0;
    ptr->bBoundary = 0;
    ptr->bExact = 0;
    ptr->iLocType = 0;
    ptr->bRecursion = 0;
    ptr->bSubstitution = 0;

    return ptr;
}

/**
 *  @brief  frees p_req_t object
 *
 *  @arg    p_req_t
 *  @return void
 */

void lost_free(p_req_t ptr) {
    if (ptr->strIDType)
        free(ptr->strIDType);
    if (ptr->strIDString)
        free(ptr->strIDString);
    if (ptr->strTS)
        free(ptr->strTS);
    if (ptr->strEX)
        free(ptr->strEX);
    if (ptr->strLatitude)
        free(ptr->strLatitude);
    if (ptr->strLongitude)
        free(ptr->strLongitude);
    if (ptr->pchPolygon)
        free(ptr->pchPolygon);
    if (ptr->strReqType)
        free(ptr->strReqType);
    if (ptr->strURI)
        free(ptr->strURI);
    if (ptr->strNumber)
        free(ptr->strNumber);
    if (ptr->strName)
        free(ptr->strName);
    if (ptr->strCountry)
        free(ptr->strCountry);
    if (ptr->strState)
        free(ptr->strState);
    if (ptr->strToken)
        free(ptr->strToken);

    if (ptr->strLocID)
        xmlFree(ptr->strLocID);
    if (ptr->strLocProfile)
        xmlFree(ptr->strLocProfile);
    if (ptr->strURN)
        xmlFree(ptr->strURN);

    if (ptr)
        free(ptr);
}

/**
 *  @brief  strips white space
 *
 *  @arg    char*
 *  @return char*
 */

char *lost_stripSpaces(char *str) {
    char *end;

    while(isspace(*str))
        str++;

    if(*str == 0)
        return NULL;

    end = str + strlen(str) - 1;

    while(end > str && isspace(*end))
        end--;

    *(end + 1) = '\0';

    return str;
}

/**
 *  @brief  generates a random string of specific length (characters)
 *
 *  @arg    char*
 *  @return void
 */

void lost_randomString(char *dest, size_t length) {
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

    return;
}
