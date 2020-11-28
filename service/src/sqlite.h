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
 *  @file    sqlite.h
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief spatialite/sqlite header file
 */

#ifndef SQLITE_H_INCLUDED
#define SQLITE_H_INCLUDED

/******************************************************************* INCLUDE */

#include <sqlite3.h>
#include <spatialite.h>
#include "dec112lost.h"

/******************************************************************** DEFINE */

#define QUERYSIZE 1024
#define SQLITE_DB_LOST "data/dects-db-at.sqlite"

#define CALL_SQLITE(f)                                          \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_OK) {                                   \
            LOG4ERROR(pL, "%s failed with status %d: %s",       \
                     #f, i, sqlite3_errmsg (db));               \
        }                                                       \
    }                                                           \

#define CALL_SQLITE_EXPECT(f,x)                                 \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_ ## x) {                                \
            LOG4ERROR(pL, "%s failed with status %d: %s",       \
                     #f, i, sqlite3_errmsg (db));               \
        }                                                       \
    }


#endif // SQLITE_H_INCLUDED
