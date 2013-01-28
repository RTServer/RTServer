#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define MAX_CONTENT_LENG 1024
#define MAX_XMLNODE_LENG 100

//定义xml数据结构
typedef struct _XMLNODE {
    int n; //第几个节点，作为唯一值，初始化时置为-1
    int deep; //深度，0为根节点
    int nextDeep; //下一个节点的深度
    char name; //名称
    char content; //内容
    char **attr; //属性
}_XMLNODE;

//定义全局变量
static _XMLNODE _xmlnode[MAX_XMLNODE_LENG]; //模块化全局变量_xmlnode

/**
* 函数声明
*/
void xmlnode_init();
int xmlnode_add(int deep, int nextDeep, char name, char content, char **attr);
static void element_parse(xmlNode *aNode);
int xml_parse(const char *xml);

/**
* 函数体
*/

void xmlnode_init() {
    int i;
    for(i = 0; i < MAX_XMLNODE_LENG; i++) {
        _xmlnode[i].n = -1;
    }
}

int xmlnode_add(int deep, int nextDeep, char name, char content, char **attr) {
    int i;
    for(i = 0; i < MAX_XMLNODE_LENG; i++) {
        if(_xmlnode[i].n == -1) {
            _xmlnode[i].deep = deep;
            _xmlnode[i].nextDeep = nextDeep;
            _xmlnode[i].name = name;
            _xmlnode[i].content = content;
            _xmlnode[i].attr = attr;
            break;
        }
    }
    if(i == MAX_XMLNODE_LENG) {
        return -1;
    }
    return i;
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
    xmlnode_init(); //初始化xml存储
    element_parse(rootElement);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

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
                printf("\n");
                //打印节点名称
                printf("NodeName:\t%s\n", curNode->name);

                //打印属性
                attrPtr = curNode->properties;
                bzero(key, 256);
                //strcpy(key, "AttrName:\t");
                //sprintf(key, "AttrName:\t");
                strncat(key, "AttrName:\t", 10);
                while(attrPtr != NULL) {
                    szAttr = xmlGetProp(curNode, BAD_CAST attrPtr->name);
                    //sprintf(key, "%s:%s\t", attrPtr->name, szAttr);
                    //printf("%s:%s\t", attrPtr->name, szAttr);
                    strncat(key, attrPtr->name, strlen(attrPtr->name));
                    strncat(key, ":", 1);
                    strncat(key, szAttr, strlen(szAttr));
                    strncat(key, "\t", 1);
                    xmlFree(szAttr);
                    attrPtr = attrPtr->next;
                }
                if(strlen(key) > 10) printf("%s\n", key);
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