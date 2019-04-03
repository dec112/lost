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
 *  @file    sqlite.c
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief this file holds spatialite/sqlite function definitions
 */

/******************************************************************* INCLUDE */

#include "dec112lost.h"
#include "sqlite.h"
#include <spatialite.h>

/***************************************************************** FUNCTIONS */

/**
 *  @brief  DB query to get service mapping (input urn + location)
 *
 *  @arg    p_req_t, char*
 *  @return int
 */

int sqliteFindService(p_req_t ptr, char *pchNss) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    char strBuf[BUFSIZE];
    char strQuery[QUERYSIZE];
    int iRes = -1;
    void *cache;

// open database
    CALL_SQLITE (open (SQLITE_DB_LOST, &db));
// init database connection
    cache = spatialite_alloc_connection();
    spatialite_init_ex(db, cache, 0);

    if (ptr->iRadius > 0) {
        snprintf(strQuery, 1023,
                 "SELECT i.country country, i.country_code code, i.state state, m.uri uri, m.name name, m.dialstring string, "
                 "i.token token FROM boundaries b, "
                 "(SELECT Transform(Buffer(Transform(GeomFromText('POINT(%s %s)', 4326), 32632), %d), 4326) as area) a "
                 "INNER JOIN mapping as m"
                 "   ON b.id=m.boundary_id "
                 "INNER JOIN services as s"
                 "   ON m.service_id=s.id "
                 "INNER JOIN info as i"
                 "   ON b.id=i.boundary_id "
                 "WHERE Intersects(b.geom, a.area) "
                 "AND NOT IsEmpty(a.area) "
                 "AND s.nss='%s';", ptr->strLongitude, ptr->strLatitude, ptr->iRadius, pchNss);
    } else {
        snprintf(strQuery, 1023,
                 "SELECT i.country country, i.country_code code, i.state state, m.uri uri, m.name name, m.dialstring string, "
                 "i.token token FROM boundaries b, "
                 "(SELECT Transform(GeomFromText('POINT(%s %s)', 4326), 4326) as area) a "
                 "INNER JOIN mapping as m"
                 "   ON b.id=m.boundary_id "
                 "INNER JOIN services as s"
                 "   ON m.service_id=s.id "
                 "INNER JOIN info as i"
                 "   ON b.id=i.boundary_id "
                 "WHERE Contains(b.geom, a.area) "
                 "AND s.nss='%s';", ptr->strLongitude, ptr->strLatitude, pchNss);
    }

    LOG4DEBUG(pL, "query:\n%s\n", strQuery);

    CALL_SQLITE (prepare_v2 (db, strQuery, strlen(strQuery) + 1, &stmt, NULL));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        snprintf(strBuf, BUFSIZE, "%s [%s]", (char *)sqlite3_column_text(stmt, 0), (char *)sqlite3_column_text(stmt, 1));
        ptr->strCountry = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strCountry, strlen(strBuf)+1, "%s", strBuf);

        snprintf(strBuf, BUFSIZE, "%s", (char *)sqlite3_column_text(stmt, 2));
        ptr->strState = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strState, strlen(strBuf)+1, "%s", strBuf);

        snprintf(strBuf, BUFSIZE, "%s", (char *)sqlite3_column_text(stmt, 3));
        ptr->strURI = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strURI, strlen(strBuf)+1, "%s", strBuf);

        snprintf(strBuf, BUFSIZE, "%s", (char *)sqlite3_column_text(stmt, 4));
        ptr->strName = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strName, strlen(strBuf)+1, "%s", strBuf);

        snprintf(strBuf, BUFSIZE, "%s", (char *)sqlite3_column_text(stmt, 5));
        ptr->strNumber = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strNumber, strlen(strBuf)+1, "%s", strBuf);

        snprintf(strBuf, BUFSIZE, "%s", (char *)sqlite3_column_text(stmt, 6));
        ptr->strToken = (char *)malloc(strlen(strBuf)+1);
        snprintf(ptr->strToken, strlen(strBuf)+1, "%s", strBuf);
        iRes = 0;
    }

    CALL_SQLITE (finalize (stmt));
    CALL_SQLITE (db_release_memory (db));
    CALL_SQLITE (close (db));

    spatialite_cleanup_ex(cache);

    return iRes;
}


/**
 *  @brief  DB query to get service boundary (input token)
 *
 *  @arg    p_req_t
 *  @return char*
 */

int sqliteServiceBoundary(p_req_t ptr) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    char strQuery[QUERYSIZE];
//    char *pchPolygon = NULL;
    int iBlobsize;
    int iRes = -1;
    void *cache;

// open database
    CALL_SQLITE (open (SQLITE_DB_LOST, &db));
// init database connection
    cache = spatialite_alloc_connection();
    spatialite_init_ex(db, cache, 0);

    snprintf(strQuery, 1023,
             "SELECT length(AsText(b.geom)) as length, AsText(b.geom) "
             "FROM boundaries as b "
             "INNER JOIN info as i "
             "   ON b.id=i.boundary_id "
             "WHERE i.token='%s';", ptr->strToken);

    LOG4DEBUG(pL, "query:\n%s\n", strQuery);

    CALL_SQLITE (prepare_v2 (db, strQuery, strlen(strQuery) + 1, &stmt, NULL));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        iBlobsize = sqlite3_column_int(stmt, 0);
        ptr->pchPolygon = (char*)malloc(sizeof(char) * iBlobsize);
        if (ptr->pchPolygon) {
            snprintf(ptr->pchPolygon, iBlobsize, "%s", sqlite3_column_text(stmt, 1));
        }
        iRes = 0;
    }

    CALL_SQLITE (finalize (stmt));
    CALL_SQLITE (db_release_memory (db));
    CALL_SQLITE (close (db));

    spatialite_cleanup_ex(cache);

    return iRes;
}
