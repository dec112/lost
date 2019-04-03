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
 * \brief Kamailio lost :: functions
 * \ingroup lost
 * Module: \ref lost
 */

#ifndef LOST_UTILITIES_H
#define LOST_UTILITIES_H

#define LOST_GEOLOC_HEADER    		"Geolocation: "
#define LOST_GEOLOC_HEADER_SIZE     strlen(LOST_GEOLOC_HEADER)


char *lost_trim_content(char*, int*);
void lost_rand_str(char*, size_t);
void lost_free_loc(p_loc_t);
int lost_get_location_object(p_loc_t, xmlDocPtr, xmlNodePtr);
char* lost_find_service_request(p_loc_t, int*);
char* lost_get_content(xmlNodePtr, const char*, int*);
char* lost_get_property(xmlNodePtr, const char*, int*);
char* lost_get_geolocation_header(struct sip_msg*, int*);
char* lost_get_childname(xmlNodePtr, const char*, int*);
char* lost_get_name(char*, int*);
p_loc_t lost_new_loc(str);


#endif






