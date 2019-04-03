/*
 * $Id: pidf.h 1401 2006-12-14 11:12:42Z anca_vamanu $
 *
 * presence module - presence server implementation
 *
 * Copyright (C) 2006 Voice Sistem S.R.L.
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
 * History:
 * --------
 *  2006-08-15  initial version (anca)
 */

/*! \file
 * \brief Kamailio Presence_XML :: PIDF handling
 * \ref pidf.c
 * \ingroup presence_xml
 */


#ifndef PIDF_H
#define PIDF_H

#include "../../core/str.h"
#include <libxml/parser.h>

#define BUFSIZE 	128

typedef struct {
    char* identity;
	char* urn;
    char* longitude;
    char* latitude;
    int radius;
    int recursive;
} s_loc_t, *p_loc_t;

xmlNodePtr xmlNodeGetNodeByName(xmlNodePtr node, const char *name,
															const char *ns);
xmlNodePtr xmlDocGetNodeByName(xmlDocPtr doc, const char *name, const char *ns);
xmlNodePtr xmlNodeGetChildByName(xmlNodePtr node, const char *name);

char *xmlNodeGetNodeContentByName(xmlNodePtr root, const char *name,
		const char *ns);
char *xmlNodeGetAttrContentByName(xmlNodePtr node, const char *name);

char *xmlParseFindServiceResponse(xmlNodePtr node, const char *name, int *len);
int xmlParseLocationInfo(xmlNodePtr node, p_loc_t loc);

#endif
