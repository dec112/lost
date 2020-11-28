/*
 * Copyright (C) 2018  <Wolfgang Kampichler>
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
 * requires: libxml2-dev libspatialite-dev, liblog4c-dev, sqlite3
 */

/**
 *  @file    dec112lost.c
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief this file holds the main function definition
 */

/******************************************************************* INCLUDE */

#include "helper.h"
#include "lost.h"
#include "xml.h"

/******************************************************************* GLOBALS */

static sig_atomic_t s_signal_received = 0;

/******************************************************************* SIGNALS */

static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    s_signal_received = sig_num;
}

/********************************************************************** MAIN */

int main(int argc, char *argv[]) {
    struct mg_mgr mgr;
    struct mg_connection *nc;

    char *fn_lost = LOST_RNG;
    static const char *s_http_port = LOCAL_PORT;
    const char *sLogCat;

    char s_ip_port[256];

    int opt = 0;

    strIPAddr = LOCAL_IP_ADDR;

    pValidrngctxt = NULL;
    pValidctxt = NULL;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

// get arguments
    if (argc < 5) {
        printf("usage: dec112lost -i <ip/domain str> -p <listening port>\n");
        exit(0);
    }

    sLogCat = LOGCAT; //LOGCATDBG;

    while ((opt = getopt(argc, argv, "i:p:v")) != -1) {
        switch(opt) {
        case 'v':
            sLogCat = LOGCATDBG;
            break;
        case 'i':
            strIPAddr = optarg;
            break;
        case 'p':
            s_http_port = optarg;
            break;
        case '?':
            if (optopt == 'i') {
                ERROR_PRINT("missing ip address string\n");
            } else if (optopt == 'p') {
                ERROR_PRINT("missing listening port\n");
            } else {
                ERROR_PRINT("invalid option received\n");
            }
            exit(0);
            break;
        }
    }

    if (log4c_init()) {
        ERROR_PRINT("can't initialize logging\n");
        exit(0);
    }

    pL = log4c_category_get(sLogCat);

    if (pL == NULL) {
        ERROR_PRINT("can't get log category\n");
        exit(0);
    }

    LOG4INFO(pL, "started");

    LOG4DEBUG(pL, "ip/domain string: %s", strIPAddr);
    LOG4DEBUG(pL, "listening port: %s", s_http_port);

    snprintf(s_ip_port, 255, "%s", s_http_port);
//    snprintf(s_ip_port, 255, "%s:%s", strIPAddr, s_http_port);

// inititate SQLite database and SpatiaLite and show versions
    spatialite_init(0);

    LOG4DEBUG(pL, "SQLite version: %s", sqlite3_libversion ());
    LOG4DEBUG(pL, "SpatiaLite version: %s", spatialite_version ());

    spatialite_cleanup();

// load LOST schema
    xmlRelaxNGParserCtxtPtr pRNGparser = NULL;
    xmlRelaxNGPtr pXmlRNGschema = NULL;

    pRNGparser = xmlRelaxNGNewParserCtxt(fn_lost);
    pXmlRNGschema = xmlRelaxNGParse(pRNGparser);
    pValidrngctxt = xmlRelaxNGNewValidCtxt(pXmlRNGschema);

// initiate mongoose
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_ip_port, lost_handleHttpEvent);

    if (!nc) {
        ERROR_PRINT("could not bind port: %s\n", s_ip_port);
        exit(0);
    }

// set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    //s_http_server_opts.document_root = ".";  // Serve current directory
    //s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav
    //s_http_server_opts.enable_directory_listing = "yes";

// start server
    while (s_signal_received == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

// stop and cleanup
    printf("\n");
    LOG4INFO(pL, "stopped");

    if (pValidrngctxt) {
        xmlRelaxNGFreeValidCtxt(pValidrngctxt);
    }

    if (pXmlRNGschema) {
        xmlRelaxNGFree(pXmlRNGschema);
    }

    if (pRNGparser) {
    	xmlRelaxNGFreeParserCtxt(pRNGparser);
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    mg_mgr_free(&mgr);

    log4c_fini();

    return 0;
}


