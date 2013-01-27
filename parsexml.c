#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

static void element_parse(xmlNode *aNode) {
    xmlNode *curNode = NULL;
    xmlChar *szKey, *szAttr;
    xmlAttrPtr attrPtr;
    int count = 0;
    char key[256];

    for (curNode = aNode; curNode; curNode = curNode->next) {
        //printf("node type:%d---", curNode->type);
        switch(curNode->type) {
            case XML_ELEMENT_NODE:
                //打印节点名称
                printf("NodeName:\t%s\n", curNode->name);

                //打印属性
                attrPtr = curNode->properties;
                //bzero(key, 256);
                //strcpy(key, "AttrName:\t");
                while(attrPtr != NULL) {
                    szAttr = xmlGetProp(curNode, BAD_CAST attrPtr->name);
                    //sprintf(key, "%s:%s\t", attrPtr->name, szAttr);
                    printf("%s:%s\t", attrPtr->name, szAttr);
                    xmlFree(szAttr);
                    attrPtr = attrPtr->next;
                }
                //if(strlen(key) > 10) printf("%s\n", key);
                break;
            case XML_TEXT_NODE:
                //打印内容
                szKey = xmlNodeGetContent(curNode);
                printf("Content:\t%s\n", szKey);
                xmlFree(szKey);
                break;
        }
        
        element_parse(curNode->children);
    }
}

int xml_parse(const char *xml) {
    xmlDocPtr doc;    
    xmlNode *rootElement = NULL;

    LIBXML_TEST_VERSION
    doc = xmlParseMemory(xml, strlen(xml));
    if (NULL == doc) {
        fprintf(stderr, "open file failed!\n");
        return -1;
    }

    rootElement = xmlDocGetRootElement(doc);
    element_parse(rootElement);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}