/*
 * DEC112 LoST Module
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
 * \brief Kamailio lost ::
 * \ingroup lost
 * Module: \ref lost
 */

#include "../../core/mod_fix.h"
#include "../../core/sr_module.h"
#include "../../core/ut.h"
#include "../../core/locking.h"

#include "../../core/pvar.h"
#include "../../core/mem/mem.h"
#include "../../core/dprint.h"

#include "../../core/script_cb.h"

#include "functions.h"

MODULE_VERSION

/* Module parameter variables */

/* Module management function prototypes */
static int mod_init(void);
static void destroy(void);

/* Fixup functions to be defined later */
static int fixup_lost_request(void** param, int param_no);
static int fixup_free_lost_request(void** param, int param_no);
static int fixup_lost_request_urn(void** param, int param_no);
static int fixup_free_lost_request_urn(void** param, int param_no);
static int fixup_lost_response(void** param, int param_no);
static int fixup_free_lost_response(void** param, int param_no);

/* Wrappers for http_query to be defined later */
static int w_lost_request(struct sip_msg* _m, char* _result);
static int w_lost_request_urn(struct sip_msg* _m, char* _urn, char* _result);
static int w_lost_response(struct sip_msg* _m, char* _body, char* _result, char* _name);

/* Exported functions */
static cmd_export_t cmds[] = {
    {
        "lost_query", (cmd_function)w_lost_request, 1,
        fixup_lost_request, fixup_free_lost_request,
        REQUEST_ROUTE|FAILURE_ROUTE|BRANCH_ROUTE
    },
    {
        "lost_query_urn", (cmd_function)w_lost_request_urn, 2,
        fixup_lost_request_urn, fixup_free_lost_request_urn,
        REQUEST_ROUTE|FAILURE_ROUTE|BRANCH_ROUTE
    },
    {
        "lost_response", (cmd_function)w_lost_response, 3,
        fixup_lost_response, fixup_free_lost_response,
        REQUEST_ROUTE|FAILURE_ROUTE|BRANCH_ROUTE
    },
    {0, 0, 0, 0, 0, 0}
};

/* Exported parameters
static param_export_t params[] = {
    {0, 0, 0}
};
*/

/* Module interface */
struct module_exports exports= {
    "lost",  	/* module name*/
    DEFAULT_DLFLAGS, /* dlopen flags */
    cmds,       /* exported functions */
    0,          /* module parameters */
    0,          /* exported statistics */
    0,          /* exported MI functions */
    0,          /* exported pseudo-variables */
    0,          /* extra processes */
    mod_init,   /* module initialization function */
    0,          /* response function */
    destroy,    /* destroy function */
    0,          /* per-child init function */
};


static int mod_init(void) {
    return 0;
}

static void destroy(void) {
}

/* Fixup functions */
static int fixup_lost_request(void** param, int param_no) {
    if (param_no == 1) {
        if (fixup_pvar_null(param, 1) != 0) {
            LM_ERR("failed to fixup result pvar\n");
            return -1;
        }
        if (((pv_spec_t *)(*param))->setf == NULL) {
            LM_ERR("result pvar is not writeble\n");
            return -1;
        }
        return 0;
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int fixup_free_lost_request(void** param, int param_no) {
    if (param_no == 1){
        return fixup_free_pvar_null(param, 1);
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int fixup_lost_request_urn(void** param, int param_no) {
    if (param_no == 1) {
        return fixup_spve_null(param, 1);
    }

    if (param_no == 2) {
        if (fixup_pvar_null(param, 1) != 0) {
            LM_ERR("failed to fixup result pvar\n");
            return -1;
        }
        if (((pv_spec_t *)(*param))->setf == NULL) {
            LM_ERR("result pvar is not writeble\n");
            return -1;
        }
        return 0;
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int fixup_free_lost_request_urn(void** param, int param_no) {
    if (param_no == 1) {
        return fixup_free_spve_null(param, 1);
    }

    if (param_no == 2) {
        return fixup_free_pvar_null(param, 1);
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int fixup_lost_response(void** param, int param_no) {
    if (param_no == 1) {
        return fixup_spve_null(param, 1);
    }

    if ((param_no == 2) || (param_no == 3)) {
        if (fixup_pvar_null(param, 1) != 0) {
            LM_ERR("failed to fixup result pvar\n");
            return -1;
        }
        if (((pv_spec_t *)(*param))->setf == NULL) {
            LM_ERR("result pvar is not writeble\n");
            return -1;
        }
        return 0;
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int fixup_free_lost_response(void** param, int param_no) {
    if (param_no == 1) {
        return fixup_free_spve_null(param, 1);
    }

    if ((param_no == 2) || (param_no == 3)) {
        return fixup_free_pvar_null(param, 1);
    }

    LM_ERR("invalid parameter number <%d>\n", param_no);
    return -1;
}

static int w_lost_request(struct sip_msg* _m, char* _result) {
    return lost_function(_m, _result, NULL, NULL);
}

static int w_lost_request_urn(struct sip_msg* _m, char* _urn, char* _result) {
    return lost_function_urn(_m, _result, _urn);
}

static int w_lost_response(struct sip_msg* _m, char* _body, char* _result, char* _name) {
    return lost_function(_m, _result, _body, _name);
}
