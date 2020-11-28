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
 *  @file    xml.c
 *  @author  Wolfgang Kampichler (DEC112)
 *  @date    06-2018
 *  @version 1.0
 *
 *  @brief this file holds xml function definitions
 */

/******************************************************************* INCLUDE */

#include "dec112lost.h"
#include "xml.h"

/***************************************************************** FUNCTIONS */

/**
 *  @brief  get xml attribute by name
 *
 *  @arg    xmlNodePtr, const char*
 *  @return xmlAttrPtr
 */

xmlAttrPtr xmlNodeGetAttrByName(xmlNodePtr node, const char *name) {
    xmlAttrPtr attr = node->properties;
    while (attr) {
        if (xmlStrcasecmp(attr->name, (unsigned char*)name) == 0)
            return attr;
        attr = attr->next;
    }
    return NULL;
}

/**
 *  @brief  get xml attribute content by name
 *
 *  @arg    xmlNodePtr, const char*
 *  @return char*
 */

char *xmlNodeGetAttrContentByName(xmlNodePtr node, const char *name) {
    xmlAttrPtr attr = xmlNodeGetAttrByName(node, name);
    if (attr)
        return (char*)xmlNodeGetContent(attr->children);
    else
        return NULL;
}

/**
 *  @brief  get xml child node by name
 *
 *  @arg    xmlNodePtr, const char*
 *  @return xmlNodePtr
 */

xmlNodePtr xmlNodeGetChildByName(xmlNodePtr node, const char *name) {
    xmlNodePtr cur = node->children;
    while (cur) {
        if (xmlStrcasecmp(cur->name, (unsigned char*)name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/**
 *  @brief  get xml node by name including namespace (ns)
 *
 *  @arg    xmlNodePtr, const char*, const char*
 *  @return xmlNodePtr
 */

xmlNodePtr xmlNodeGetNodeByName(xmlNodePtr node, const char *name, const char *ns) {
    xmlNodePtr cur = node;
    while (cur) {
        xmlNodePtr match = NULL;
        if (xmlStrcasecmp(cur->name, (unsigned char*)name) == 0) {
            if (!ns || (cur->ns && xmlStrcasecmp(cur->ns->prefix,
                                                 (unsigned char*)ns) == 0))
                return cur;
        }
        match = xmlNodeGetNodeByName(cur->children, name, ns);
        if (match)
            return match;
        cur = cur->next;
    }
    return NULL;
}

/**
 *  @brief  get xml node content by name including namespace (ns)
 *
 *  @arg    xmlNodePtr, const char*, const char*
 *  @return char*
 */

char *xmlNodeGetNodeContentByName(xmlNodePtr root, const char *name,
                                  const char *ns) {
    xmlNodePtr node = xmlNodeGetNodeByName(root, name, ns);
    if (node)
        return (char*)xmlNodeGetContent(node->children);
    else
        return NULL;
}

/**
 *  @brief  get first xml node of document by name including namespace
 *
 *  @arg    xmlDocPtr, const char*, const char*
 *  @return xmlNodePtr
 */

xmlNodePtr xmlDocGetNodeByName(xmlDocPtr doc, const char *name, const char *ns) {
    xmlNodePtr cur = doc->children;
    return xmlNodeGetNodeByName(cur, name, ns);
}

/**
 *  @brief  get first xml node content of document by name including namespace
 *
 *  @arg    xmlDocPtr, const char*, const char*
 *  @return char*
 */

char *xmlDocGetNodeContentByName(xmlDocPtr doc, const char *name,
                                 const char *ns) {
    xmlNodePtr node = xmlDocGetNodeByName(doc, name, ns);
    if (node)
        return (char*)xmlNodeGetContent(node->children);
    else
        return NULL;
}

/**
 *  @brief  parse node
 *
 *  @arg    char**, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parseNode(char** str, xmlNodePtr cur, xmlDocPtr doc) {
    xmlNodePtr node;

    node = cur;

    return node->next;
}

/**
 *  @brief  parse node with child
 *
 *  @arg    char**, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parseNodeWithChild(char** str, xmlNodePtr cur, xmlDocPtr doc) {
    xmlNodePtr node;

    node = cur;

    node = cur->xmlChildrenNode;

    return node->next;
}

/**
 *  @brief  parse content
 *
 *  @arg    char**, char*, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parseContent(char** str, char* attr, xmlNodePtr cur, xmlDocPtr doc) {
    char *buf;
    char *ptr;

    xmlNodePtr node;
    xmlChar *strContent = NULL;

    node = cur;

    if (node->type == XML_ELEMENT_NODE) {
        if ((!xmlStrcmp(node->name, BAD_CAST attr))) {
            strContent = xmlNodeGetContent(node);
        }
    }

    if (strContent) {
        buf = lost_stripSpaces(strContent);
        ptr = (char *)malloc(strlen((char *)buf) + 1);
        snprintf(ptr, strlen((char *)buf) + 1, "%s", (char *)buf);
        *str = ptr;
    }

    xmlFree(strContent);

    return node->next;
}

/**
 *  @brief  parse content with child
 *
 *  @arg    char**, char*, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parseContentWithChild(char** str, char* attr, xmlNodePtr cur,
                                 xmlDocPtr doc) {
    char *buf;
    char *ptr;
    xmlNodePtr node;
    xmlChar *strContent = NULL;

    node = cur;

    if (node->type == XML_ELEMENT_NODE) {
        if ((!xmlStrcmp(node->name, BAD_CAST attr))) {
            strContent = xmlNodeGetContent(node);
        }
    }

    if (strContent) {
        buf = lost_stripSpaces(strContent);
        ptr = (char *)malloc(strlen((char *)buf) + 1);
        snprintf(ptr, strlen((char *)buf) + 1, "%s", (char *)buf);
        *str = ptr;
    }

    xmlFree(strContent);

    node = cur->xmlChildrenNode;

    return node->next;
}

/**
 *  @brief  parse property
 *
 *  @arg    char**, char*, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parseProperty(char** str, char* prop, xmlNodePtr cur, xmlDocPtr doc) {
    char *ptr;
    xmlChar *strProperty = NULL;
    xmlNodePtr node;

    node = cur;

    strProperty = xmlGetProp(node, BAD_CAST prop);
    ptr = (char *)malloc(strlen((char *)strProperty) + 1);
    snprintf(ptr, strlen((char *)strProperty) + 1, "%s", (char *)strProperty);
    *str = ptr;
    xmlFree(strProperty);

    return node->next;
}

/**
 *  @brief  parse property with child
 *
 *  @arg    char**, char*, xmlNodePtr, xmlDocPtr
 *  @return xmlNodePtr
 */

xmlNodePtr parsePropertyWithChild(char** str, char* prop, xmlNodePtr cur,
                                  xmlDocPtr doc) {
    char *ptr;
    xmlChar *strProperty = NULL;
    xmlNodePtr node;

    node = cur;

    strProperty = xmlGetProp(node, BAD_CAST prop);
    ptr = (char *)malloc(strlen((char *)strProperty) + 1);
    snprintf(ptr, strlen((char *)strProperty) + 1, "%s", (char *)strProperty);
    *str = ptr;
    xmlFree(strProperty);

    node = cur->xmlChildrenNode;

    return node->next;
}



