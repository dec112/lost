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
/*****************/


#include "../../core/mod_fix.h"
#include "../../core/pvar.h"
#include "../../core/route_struct.h"
#include "../../core/ut.h"
#include "../../core/trim.h"
#include "../../core/mem/mem.h"
#include "../../core/parser/msg_parser.h"
#include "../../core/parser/parse_content.h"
#include "../../core/parser/parse_body.h"
#include "../../core/parser/parse_uri.h"
#include "../../core/lvalue.h"

#include "pidf.h"
#include "utilities.h"


/*****************/
int lost_function(struct sip_msg* _m, char* _dst, char* _post, char* _name) {

    pv_spec_t *dst;
    pv_spec_t *name;
    pv_value_t vald;
    pv_value_t valn;
    struct msg_start *fl;
    p_loc_t loc = NULL;
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;

    char *search;

    str lfsr = {NULL, 0};
    str lres = {NULL, 0};
    str lrdn = {NULL, 0};
    str pidb = {NULL, 0};
    str rurn = {NULL, 0};
    str geoh = {NULL, 0};

    if (_post) {
        if (fixup_get_svalue(_m, (gparam_p)_post, &lfsr) != 0) {
            LM_ERR("cannot get response data\n");
            return -1;
        }
        if (lfsr.len > 0) {
            doc = xmlReadMemory(lfsr.s, lfsr.len, 0, 0,
                                XML_PARSE_NOBLANKS |
                                XML_PARSE_NONET |
                                XML_PARSE_NOCDATA);

            if (!doc) {
                LM_ERR("invalid xml document: \n[%.*s]\n", lfsr.len, lfsr.s);
                goto err;
            }

            root = xmlDocGetRootElement(doc);
            if (!root) {
                LM_ERR("empty xml document: \n[%.*s]\n", lfsr.len, lfsr.s);
                goto err;
            }

            LM_DBG("findServiceResponse: \n[%.*s]\n", lfsr.len, lfsr.s);

            if ((!xmlStrcmp(root->name, (const xmlChar*)"findServiceResponse"))) {
                lres.s = lost_get_content(root, (char*)"uri", &lres.len);
                if (!lres.s) {
                    LM_ERR("uri element not found: \n[%.*s]\n", lfsr.len, lfsr.s);
                    goto err;
                }
                LM_DBG("lost_response - uri: \n[%.*s]\n", lres.len, lres.s);

                lrdn.s = lost_get_content(root, (char*)"displayName", &lrdn.len);
                if (!lrdn.s) {
                    LM_ERR("displayName element not found: \n[%.*s]\n", lfsr.len, lfsr.s);
                    goto err;
                }

                LM_DBG("lost_response - displayName: \n[%.*s]\n", lrdn.len, lrdn.s);
            } else if ((!xmlStrcmp(root->name, (const xmlChar*)"errors"))) {
                //LM_ERR("lost error response received\n");
                LM_DBG("lost error response received\n");

                lres.s = lost_get_name((char*)root->name, &lres.len);
                lrdn.s = lost_get_childname(root, (char*)"errors", &lrdn.len);

                if (!lrdn.s) {
                    LM_ERR("error pattern element not found: \n[%.*s]\n", lfsr.len, lfsr.s);
                    goto err;
                }

                LM_DBG("lost_response - uri: \n[%.*s]\n", lres.len, lres.s);
                LM_DBG("lost_response - displayName: \n[%.*s]\n", lrdn.len, lrdn.s);

            } else {
                LM_ERR("root element is not valid: \n[%.*s]\n", lfsr.len, lfsr.s);
                goto err;
            }
        } else {
            LM_ERR("empty xml document\n");
            goto err;
        }

        vald.rs = lres;
        vald.rs.s = lres.s;
        vald.rs.len = lres.len;

        valn.rs = lrdn;
        valn.rs.s = lrdn.s;
        valn.rs.len = lrdn.len;

        valn.flags = PV_VAL_STR;
        name = (pv_spec_t *)_name;
        name->setf(_m, &name->pvp, (int)EQ_T, &valn);

        xmlFreeDoc(doc);
    } else {
        // get urn
        fl=&(_m->first_line);
        rurn.len = fl->u.request.uri.len;
        rurn.s = fl->u.request.uri.s;

        LM_DBG("r_urn: \n[%.*s]\n", rurn.len, rurn.s);

        //get geolocation header field
        geoh.s = lost_get_geolocation_header(_m, &geoh.len);

        if (geoh.len == 0) {
            LM_ERR("geolocation header not found\n");
            return -1;
        } else {
            LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);

            search = geoh.s;
            // look for cid
            if ( (*(search + 0) == '<')
                    && ((*(search + 1) == 'c') || (*(search + 1) == 'C'))
                    && ((*(search + 2) == 'i') || (*(search + 2) == 'I'))
                    && ((*(search + 3) == 'd') || (*(search + 3) == 'D'))
                    && (*(search + 4) == ':') ) {
                search += 4;
                *search = '<';
                geoh.s = search;
                geoh.len = geoh.len - 4;

                LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);
            }
            // look for http url
            if ( (*(search + 0) == '<')
                    && ((*(search + 1) == 'h') || (*(search + 1) == 'H'))
                    && ((*(search + 2) == 't') || (*(search + 2) == 'T'))
                    && ((*(search + 3) == 't') || (*(search + 3) == 'T'))
                    && ((*(search + 4) == 'p') || (*(search + 3) == 'P'))
                    && (*(search + 5) == ':') ) {
                // held request goes here ...
                LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);
                goto err; //TBD
            }

        }

        // get multipart body
        pidb.s = get_body_part_by_filter(_m, 0, 0, geoh.s, NULL, &pidb.len);

        if (!pidb.s) {
            LM_ERR("no multipart body found\n");
            return -1;
        } else {
            LM_INFO("multipart pidf+xml body: \n[%.*s]\n", pidb.len, pidb.s);  //LM_DBG

            doc = xmlReadMemory(pidb.s, pidb.len, 0, 0,
                                XML_PARSE_NOBLANKS |
                                XML_PARSE_NONET |
                                XML_PARSE_NOCDATA);

            if (!doc) {
                LM_DBG("invalid xml document: \n[%.*s]\n", pidb.len, pidb.s);
                goto err;
            }

            root = xmlDocGetRootElement(doc);
            if (!root) {
                LM_ERR("empty xml document\n");
                goto err;
            }

            if ((!xmlStrcmp(root->name, (const xmlChar*)"presence")) ||
                    (!xmlStrcmp(root->name, (const xmlChar*)"locationResponse"))) {

                loc = lost_new_loc(rurn);

                if (xmlParseLocationInfo(root, loc) < 0) {
                    LM_ERR("location element not found\n");
                    goto err;
                }
                lres.s = lost_find_service_request(loc, &lres.len);

                LM_DBG("lost_response - findService request: \n[%.*s]\n", lres.len, lres.s);

                lost_free_loc(loc);
            } else {
                LM_DBG("root element is not valid: \n[%.*s]\n", pidb.len, pidb.s);
                goto err;
            }
        }

        vald.rs = lres;
        vald.rs.s = lres.s;
        vald.rs.len = lres.len;
    }

    vald.flags = PV_VAL_STR;
    dst = (pv_spec_t *)_dst;
    dst->setf(_m, &dst->pvp, (int)EQ_T, &vald);

    return vald.rs.len;

err:
    if (loc)
        lost_free_loc(loc);
    if (doc)
        xmlFreeDoc(doc);
    return -1;
}

int lost_function_urn(struct sip_msg* _m, char* _dst, char* _urn) {

    pv_spec_t *dst;
    pv_value_t val;
    p_loc_t loc = NULL;
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;

    char *search;

    str lfsr = {NULL, 0};
    str lres = {NULL, 0};
    str pidb = {NULL, 0};
    str rurn = {NULL, 0};
    str geoh = {NULL, 0};

    // get urn
    if (_urn) {
        if (fixup_get_svalue(_m, (gparam_p)_urn, &lfsr) != 0) {
            ERR("cannot get response data\n");
            return -1;
        }
        if (lfsr.len > 0) {
            rurn.len = lfsr.len;
            rurn.s = lfsr.s;
        } else {
            return -1;
        }

        LM_DBG("r_urn: \n[%.*s]\n", rurn.len, rurn.s);

        //get geolocation header field
        geoh.s = lost_get_geolocation_header(_m, &geoh.len);

        if (geoh.len == 0) {
            LM_ERR("geolocation header not found\n");
            return -1;
        } else {
            LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);

            search = geoh.s;
            // look for cid
            if ( (*(search + 0) == '<')
                    && ((*(search + 1) == 'c') || (*(search + 1) == 'C'))
                    && ((*(search + 2) == 'i') || (*(search + 2) == 'I'))
                    && ((*(search + 3) == 'd') || (*(search + 3) == 'D'))
                    && (*(search + 4) == ':') ) {
                search += 4;
                *search = '<';
                geoh.s = search;
                geoh.len = geoh.len - 4;

                LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);
            }
            // look for http url
            if ( (*(search + 0) == '<')
                    && ((*(search + 1) == 'h') || (*(search + 1) == 'H'))
                    && ((*(search + 2) == 't') || (*(search + 2) == 'T'))
                    && ((*(search + 3) == 't') || (*(search + 3) == 'T'))
                    && ((*(search + 4) == 'p') || (*(search + 3) == 'P'))
                    && (*(search + 5) == ':') ) {
                // held request goes here ...
                LM_DBG("geo hdr: \n[%.*s]\n", geoh.len, geoh.s);
                goto err; //TBD
            }

        }

        // get multipart body
        pidb.s = get_body_part_by_filter(_m, 0, 0, geoh.s, NULL, &pidb.len);

        if (!pidb.s) {
            LM_ERR("no multipart body found\n");
            return -1;
        } else {
            LM_DBG("multipart pidf+xml body: \n[%.*s]\n", pidb.len, pidb.s);

            doc = xmlReadMemory(pidb.s, pidb.len, 0, 0,
                                XML_PARSE_NOBLANKS |
                                XML_PARSE_NONET |
                                XML_PARSE_NOCDATA);

            if (!doc) {
                LM_DBG("invalid xml document: \n[%.*s]\n", pidb.len, pidb.s);
                goto err;
            }

            root = xmlDocGetRootElement(doc);
            if (!root) {
                LM_ERR("empty xml document\n");
                goto err;
            }

            if ((!xmlStrcmp(root->name, (const xmlChar*)"presence")) ||
                    (!xmlStrcmp(root->name, (const xmlChar*)"locationResponse"))) {

                loc = lost_new_loc(rurn);

                if (xmlParseLocationInfo(root, loc) < 0) {
                    LM_ERR("location element not found\n");
                    goto err;
                }
                lres.s = lost_find_service_request(loc, &lres.len);

                LM_DBG("lost_response - findService request: \n[%.*s]\n", lres.len, lres.s);

                lost_free_loc(loc);
            } else {
                LM_DBG("root element is not valid: \n[%.*s]\n", pidb.len, pidb.s);
                goto err;
            }
        }

        val.rs = lres;
        val.rs.s = lres.s;
        val.rs.len = lres.len;
    }

    val.flags = PV_VAL_STR;
    dst = (pv_spec_t *)_dst;
    dst->setf(_m, &dst->pvp, (int)EQ_T, &val);

    return val.rs.len;

err:
    if (loc)
        lost_free_loc(loc);
    if (doc)
        xmlFreeDoc(doc);
    return -1;
}
