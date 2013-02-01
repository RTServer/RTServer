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
static int _n = -1, _deep = -1;

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
    _n = _deep = -1;
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
        printf("name:%s\n", _xmlnode[i].name);
        printf("n:%d deep:%d parent:%d\n", _xmlnode[i].n, _xmlnode[i].deep, _xmlnode[i].parent);
        printf("attr:%s\n", _xmlnode[i].attr);
        printf("text:%s\n", _xmlnode[i].text);
        printf("\n");
    }
}

int xmlnode_parse(const char *xml) {
    xmlDocPtr doc;    
    xmlNode *rootElement = NULL;

    LIBXML_TEST_VERSION
    doc = xmlParseMemory(xml, strlen(xml));
    if (NULL == doc) {
        fprintf(stderr, "xml invalid!\n");
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
    int oneTimeRecurNodeNum = 0; //一次递归节点数
    static int _pd[MAX_XMLNODE_LENG];
    char key[256];

    for(curNode = aNode; curNode; curNode = curNode->next) {
        switch(curNode->type) {
            case XML_ELEMENT_NODE:
                _n++;
                oneTimeRecurNodeNum++;
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
        //设置深度
        if(oneTimeRecurNodeNum == 1) { //oneTimeRecurNodeNum为1时深度加1
            _deep++;
        }else if(oneTimeRecurNodeNum > 1) { //oneTimeRecurNodeNum大于1时，深度等于对照表中保存的上一个深度
            _deep = _pd[oneTimeRecurNodeNum - 1];
        }
        _pd[oneTimeRecurNodeNum] = _deep; //保存对照表中的oneTimeRecurNodeNum和当前深度的关系
        _xmlnode[_n].deep = _deep; //为全局节点数组赋值深度

        //设置父节点
        if(_n == 0) {
            _xmlnode[_n].parent = -1;
        }else {
            int _deepCur = _xmlnode[_n].deep, _deepPre = _xmlnode[_n - 1].deep;
            if(_deepCur == _deepPre) { //当前深度和前一个深度一样，则当前父亲也和上一个一样
                _xmlnode[_n].parent = _xmlnode[_n - 1].parent;
            }else if(_deepCur > _deepPre) { //当前深度大于前一个深度，说明向内进了一层，则当前父亲就是上一个节点
                _xmlnode[_n].parent = _n - 1;
            }else { //当前深度小于前一个深度，说明在回退，需判断前一个节点到其父节点中和其相同深度的节点个数(_deepSame)
                //则当前父节点=[当前节点值-(上一个节点深度-当前节点深度+_deepSame)].parent
                int _deepSame = _n - 1 - _xmlnode[_n - 1].parent;
                _xmlnode[_n].parent = _xmlnode[_n - (_deepPre - _deepCur + _deepSame)].parent;
            }
        }

        //设置节点标示
        _xmlnode[_n].n = _n;

        /**
        if(oneTimeRecurNodeNum) { //oneTimeRecurNodeNum为0即为text(内容)
            if(_n == 0 && oneTimeRecurNodeNum == 1) printf("_oTRNN\t_n\tdeep\tparent\tcodename\n");
            printf("%d\t%d\t%d\t%d\t%s\t\n", oneTimeRecurNodeNum, _n, _deep, _xmlnode[_n].parent, curNode->name);
        }*/
        
        element_parse(curNode->children);
    }
}