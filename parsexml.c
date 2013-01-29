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
    int parent; //父节点的值
    char name[10]; //名称
    char text[MAX_CONTENT_LENG]; //内容
    char attr[256]; //属性
}_XMLNODE;

//定义全局变量
static _XMLNODE _xmlnode[MAX_XMLNODE_LENG]; //模块化全局变量_xmlnode
static _n = -1;

/**
* 函数声明
*/
void xmlnode_init();
void xmlnode_print(int n);
int xmlnode_parse(const char *xml);
static void element_parse(xmlNode *aNode);


/**
* 函数体
*/

void xmlnode_init() {
    int i;
    _n = -1;
    for(i = 0; i < MAX_XMLNODE_LENG; i++) {
        _xmlnode[i].n = -1;
    }
}

void xmlnode_print(int n) {
    int i, j, k;
    if(n == -1) {
        j = 0;
        k = _n;
    }else {
        if(n > _n) {
            printf("%s\n", "超范围了");
            return;
        }
        j = k = n;
    }
    for(i = j; i <= k; i++) {
        printf("%s\n", _xmlnode[i].name);
        if(strlen(_xmlnode[i].attr)) printf("%s\n", _xmlnode[i].attr);
        if(strlen(_xmlnode[i].text))printf("%s\n", _xmlnode[i].text);
        printf("\n");
    }
}

int xmlnode_parse(const char *xml) {
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

static void element_parse(xmlNode *aNode) {
    xmlNode *curNode = NULL;
    xmlChar *szKey, *szAttr;
    xmlAttrPtr attrPtr;
    int parent = 0, deep = 0;
    char key[256];

    for(curNode = aNode; curNode; curNode = curNode->next) {
        switch(curNode->type) {
            case XML_ELEMENT_NODE:
                _n++;parent++;
                //节点名称
                bzero(_xmlnode[_n].name, 10);
                strncat(_xmlnode[_n].name, curNode->name, strlen(curNode->name));

                //属性
                attrPtr = curNode->properties;
                bzero(key, 256);
                while(attrPtr != NULL) {
                    szAttr = xmlGetProp(curNode, BAD_CAST attrPtr->name);
                    strncat(key, attrPtr->name, strlen(attrPtr->name));
                    strncat(key, ":", 1);
                    strncat(key, szAttr, strlen(szAttr));
                    strncat(key, "\t", 1);
                    xmlFree(szAttr);
                    attrPtr = attrPtr->next;
                }
                bzero(_xmlnode[_n].attr, 256);
                strncat(_xmlnode[_n].attr, key, strlen(key));
                break;
            case XML_TEXT_NODE:
                //内容
                szKey = xmlNodeGetContent(curNode);
                bzero(_xmlnode[_n].text, MAX_CONTENT_LENG);
                strncat(_xmlnode[_n].text, szKey, strlen(szKey));
                xmlFree(szKey);
                break;
        } 
        int _p = _n - parent;
        printf("%d\t%d\t%d\t%d\t%s\t\n", parent, _n, _p, 1,curNode->name);
        element_parse(curNode->children);
    }
}