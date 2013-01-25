#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

int xml_parse(const char * xml) {
    xmlDocPtr doc;
    xmlNodePtr curNode;
    xmlChar *szKey, *szAttr;
    xmlAttrPtr attrPtr;
    int count = 0;
    char key[256];

    doc = xmlParseMemory(xml, strlen(xml));
    if (NULL == doc) {
        fprintf(stderr, "open file failed!\n");
        return -1;
    }

    curNode = xmlDocGetRootElement(doc);
    if (NULL == curNode) {
        fprintf(stderr, "Document not parsed sucessfully!\n");
        xmlFreeDoc(doc);
        return -1;
    }

    printf("root name: %s\n", curNode->name);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        count++;
        szKey = xmlNodeGetContent(curNode);
        printf("%s\t", curNode->name);
        attrPtr = curNode->properties;
        while (attrPtr != NULL) {
            szAttr = xmlGetProp(curNode, BAD_CAST attrPtr->name);
            printf("%s:%s\t", attrPtr->name, szAttr);
            strcpy(key, szAttr);
            xmlFree(szAttr);
            attrPtr = attrPtr->next;
        }
        printf("%s\t", szKey);
        xmlFree(szKey);

        curNode = curNode->next;
    }

    xmlFreeDoc(doc);
    return 0;
}