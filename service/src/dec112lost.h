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
 *  @file    dec112lost.h
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief dec112lost main header file
 */

#ifndef DEC112LOST_H_INCLUDED
#define DEC112LOST_H_INCLUDED

/******************************************************************* INCLUDE */

#include <libxml/xmlschemastypes.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/relaxng.h>

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <log4c.h>

#include "mongoose.h"

/******************************************************************** DEFINE */

#define BUFSIZE 128

#define LIBXML_SCHEMAS_ENABLED
#define LOST_RNG "../schema/lost-rng.xml"
#define LOCAL_IP_ADDR "127.0.0.1"
#define LOCAL_PORT "8448"
#define DEFAULT_TYPE "ip"
#define SERVER_STR "Server: DEC112-LOST v2.0\x0d\x0a"
#define ID_STRING "dec112-lost-v20-20190120"

#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s():%d: " \
                                          fmt, __func__, __LINE__, ##args)
#define ERROR_PRINT(fmt, args...) fprintf(stderr, "ERROR: %s():%d: " \
                                          fmt, __func__, __LINE__, ##args)

/************************************************************** DEFINE LOG4C */

#define LOGCATDBG	"lost.dbg"
#define LOGCAT		"lost"

#define __SHORT_FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : \
    __FILE__)

#define __LOG4C__(category, format, loglevel, ...) log4c_category_log(category, \
    loglevel, format, ## __VA_ARGS__)
#define __LOGDB__(category, format, loglevel, ...) log4c_category_log(category, \
    loglevel, "[%s] [%s:%d] " format, __func__, __SHORT_FILE__, __LINE__, ## \
    __VA_ARGS__)

#define LOG4FATAL(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_FATAL, ## __VA_ARGS__)
#define LOG4ALERT(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_ALERT, ## __VA_ARGS__)
#define LOG4CRIT(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_CRIT, ## __VA_ARGS__)
#define LOG4ERROR(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_ERROR, ## __VA_ARGS__)
#define LOG4WARN(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_WARN, ## __VA_ARGS__)
#define LOG4NOTICE(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_NOTICE, ## __VA_ARGS__)
#define LOG4INFO(category, format, ...) __LOG4C__(category, format, \
    LOG4C_PRIORITY_INFO, ## __VA_ARGS__)

#define LOG4DEBUG(category, format, ...) __LOGDB__(category, format, \
    LOG4C_PRIORITY_DEBUG, ## __VA_ARGS__)
#define LOG4TRACE(category, format, ...) __LOGDB__(category, format, \
    LOG4C_PRIORITY_TRACE, ## __VA_ARGS__)

/********************************************************************* TYPES */

typedef struct {

    char *strIDType;
    char *strIDString;
    char *strIPAddr;
    char *strTS;
    char *strEX;
    char *strLocID;
    char *strLocProfile;
    char *strReqType;
    char *strURN;
    char *strURI;
    char *strNumber;
    char *strName;
    char *strCountry;
    char *strState;
    char *strToken;
    char *strLatitude;
    char *strLongitude;
    char *pchPolygon;
    int iRadius;
    int bBoundary;
    int bExact;
    int iLocType;
    int bRecursion;
    int bSubstitution;
} s_req_t, *p_req_t;

/******************************************************************* GLOBALS */

xmlRelaxNGValidCtxtPtr pValidrngctxt;
xmlSchemaValidCtxtPtr pValidctxt;
log4c_category_t *pL;

char *strIPAddr;

/**************************************************************** PROTOTYPES */

int lost_parseLocation(p_req_t, xmlNodePtr);
int lost_parseLocation(p_req_t, xmlNodePtr);

int sqliteFindService(p_req_t ptr, char*);
int sqliteServiceBoundary(p_req_t ptr);

char *lost_httpInternalServerError(void);
char *lost_findServiceResponse(p_req_t, int*, xmlDocPtr);
char *lost_getServiceBoundaryResponse(p_req_t, int*, xmlDocPtr);
char *lost_listServicesResponse(p_req_t, int*, xmlDocPtr);
char *lost_listServicesByLocationResponse(p_req_t, int*, xmlDocPtr);
char *lost_dispatchResponse(p_req_t, int*, struct http_message*);
char *lost_stripSpaces(char*);
char *lost_xmlErrorResponse(p_req_t, int*, const char*, const char*);
char *lost_xmlFindServiceResponse(p_req_t ptr, int*);
char *lost_xmlGetServiceBoundaryResponse(p_req_t ptr, int*);

char *xmlNodeGetAttrContentByName(xmlNodePtr, const char*);
char *xmlNodeGetNodeContentByName(xmlNodePtr, const char*, const char*);
char *xmlDocGetNodeContentByName(xmlDocPtr, const char*, const char*);
char *xmlErrorResponse(p_req_t, int*, const char*, const char*);
char *xmlFindServiceResponse(p_req_t ptr, int*);
char *xmlGetServiceBoundaryResponse(p_req_t ptr, int*);
xmlNodePtr xmlDocGetNodeByName(xmlDocPtr, const char*, const char*);
xmlNodePtr xmlNodeGetChildByName(xmlNodePtr, const char*);
xmlNodePtr xmlNodeGetNodeByName(xmlNodePtr, const char*, const char*);
xmlAttrPtr xmlNodeGetAttrByName(xmlNodePtr, const char*);

xmlNodePtr parseNode(char**, xmlNodePtr, xmlDocPtr);
xmlNodePtr parseNodeWithChild(char**, xmlNodePtr, xmlDocPtr);
xmlNodePtr parseContent(char**, char*, xmlNodePtr, xmlDocPtr);
xmlNodePtr parseContentWithChild(char**, char*, xmlNodePtr, xmlDocPtr);
xmlNodePtr parseProperty(char**, char*, xmlNodePtr, xmlDocPtr);
xmlNodePtr parsePropertyWithChild(char**, char*, xmlNodePtr, xmlDocPtr);

p_req_t lost_malloc(void);

void lost_free(p_req_t);
void lost_getTimeInformation(char**, char**);
void lost_httpSuccessResponse(struct mg_connection*, int, char*);
void lost_httpErrorResponse(struct mg_connection*, int, const char*);
void lost_randomString(char*, size_t);
void lost_handleHttpEvent(struct mg_connection*, int, void*);

void sig_handler(int);

static void lost_xml_error(void*, const char*, ...);

#endif // DEC112LOST_H_INCLUDED
