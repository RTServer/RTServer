#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define MAX_XMLTEXT_LENG 1024 //内容最大长度
#define MAX_XMLNODE_NUM 128 //最大节点数
#define MAX_XMLATTR_KEY_LENG 32 //最大节点属性名长度
#define MAX_XMLATTR_VALUE_LENG 256 //最大节点属性值长度
#define MAX_XMLNAME_LENG 32 //最大节点名称长度
#define MAX_XMLSAME_NUM 8 //可能相同节点名称的个数
#define MAX_XMLATTR_NUM 16 //属性最大个数

//定义属性数据结构
typedef struct _XMLATTR {
    wchar_t key[MAX_XMLATTR_KEY_LENG + 1]; //属性字段
    wchar_t value[MAX_XMLATTR_VALUE_LENG + 1]; //属性值
    struct _XMLATTR *next; //下一个属性指针
}_XMLATTR;

//定义xml数据结构
typedef struct _XMLNODE {
    int n; //第几个节点，作为唯一值，初始化时置为-1
    int deep; //深度，0为根节点
    int parent; //父节点的值
    wchar_t name[MAX_XMLNAME_LENG + 1]; //名称
    wchar_t text[MAX_XMLTEXT_LENG + 1]; //内容
    struct _XMLATTR *attr;
}_XMLNODE;


//定义全局变量
static _XMLNODE _xmlnode[MAX_XMLNODE_NUM]; //模块化全局变量_xmlnode
static int _n = -1, _deep = -1;

/**
 * 函数声明
 */
void xmlnode_init();
void xmlnode_print(int n);
int xmlnode_parse(const char *xml);
wchar_t **xmlnode_gettext_byname(const wchar_t *name, int *n);
wchar_t *xmlnode_getattrval_byname(const wchar_t *name, const wchar_t *attr);
static void element_parse(xmlNode *aNode);


/**
 * 函数体
 */

/**
 * 初始化
 */
void xmlnode_init() {
    int i;
    _n = _deep = -1;
    for(i = 0; i < MAX_XMLNODE_NUM; i++) {
        _xmlnode[i].n = -1;
        wmemset(_xmlnode[i].name, 0, sizeof(_xmlnode[i].name)/sizeof(wchar_t));
        wmemset(_xmlnode[i].text, 0, sizeof(_xmlnode[i].text)/sizeof(wchar_t));
        free(_xmlnode[i].attr);
        _xmlnode[i].attr = NULL;
    }
}

/**
 * 打印xml节点数据
 * @param int n
 */
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
        printf("name:%ls\n", _xmlnode[i].name);
        printf("n:%d deep:%d parent:%d\n", _xmlnode[i].n, _xmlnode[i].deep, _xmlnode[i].parent);
        printf("attr:");
        _XMLATTR *_xa = _xmlnode[i].attr, __xa;
        while(_xa != NULL) {
            __xa = *_xa;
            printf("%ls->%ls\t", __xa.key, __xa.value);
            _xa = __xa.next;
        }
        printf("\n");
        printf("text:%ls\n", _xmlnode[i].text);
        printf("\n");
    }
}

/**
 * 根据节点名称获取内容
 * @param const wchar_t *name
 * @param int n
 */
wchar_t **xmlnode_gettext_byname(const wchar_t *name, int *n) {
    int i, j = 0;
    wchar_t **str = (wchar_t **)malloc(MAX_XMLSAME_NUM * sizeof(wchar_t *)); //暂时认为相同节点个数不会超过8
    for(i = 0; i < MAX_XMLNODE_NUM && _xmlnode[i].n != -1; i++) {
        if(!wcscmp(name, _xmlnode[i].name)) {
            if(j == MAX_XMLSAME_NUM) break;
            str[j] = (wchar_t *)malloc((MAX_XMLTEXT_LENG + 1) * sizeof(wchar_t));
            wcsncpy(str[j], _xmlnode[i].text, MAX_XMLTEXT_LENG);
            j++;
        }
    }
    *n = j;
    return str;
}

/**
 * 根据节点名称和属性获取属性值
 * @param const wchar_t *name
 * @param const wchar_t *attr
 */
wchar_t *xmlnode_getattrval_byname(const wchar_t *name, const wchar_t *attr) {
    int i;
    wchar_t *value = NULL;
    for(i = 0; i < MAX_XMLNODE_NUM && _xmlnode[i].n != -1; i++) {
        if(!wcscmp(name, _xmlnode[i].name)) {
            _XMLATTR *_xa = _xmlnode[i].attr, __xa;
            while(_xa != NULL) {
                __xa = *_xa;
                if(!wcscmp(attr, __xa.key)) {
                    value = (wchar_t *)malloc((MAX_XMLATTR_VALUE_LENG + 1) * sizeof(wchar_t));
                    wcsncpy(value, __xa.value, MAX_XMLATTR_VALUE_LENG);
                    break;
                }
                _xa = __xa.next;
            }
            break;
        }
    }
    return value;
}

/**
 * 解析xml数据
 * @param const char *xml
 */
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
    static int _pd[MAX_XMLNODE_NUM];

    for(curNode = aNode; curNode; curNode = curNode->next) {
        switch(curNode->type) {
            case XML_ELEMENT_NODE:
                _n++;
                oneTimeRecurNodeNum++;
                //节点名称
                mbstowcs(_xmlnode[_n].name, curNode->name, sizeof(_xmlnode[_n].name) - 1);

                //属性
                attrPtr = curNode->properties;
                //_XMLATTR _xmlattr[MAX_XMLATTR_NUM]; //认为节点数不超过16个 (这种方式内存地址会重复)
                _XMLATTR *_xmlattr = (_XMLATTR *)malloc(MAX_XMLATTR_NUM * sizeof(_XMLATTR)); //需要申请内存
                int i = 0;
                while(attrPtr != NULL) {
                    szAttr = xmlGetProp(curNode, BAD_CAST attrPtr->name);
                    mbstowcs(_xmlattr[i].key, attrPtr->name, sizeof(_xmlattr[i].key) - 1);
                    mbstowcs(_xmlattr[i].value, szAttr, sizeof(_xmlattr[i].value) - 1);
                    if(i == 0) _xmlattr[i].next = _xmlattr;
                    else _xmlattr[i - 1].next = &_xmlattr[i]; //上一个next指向当前
                    i++;

                    xmlFree(szAttr);
                    attrPtr = attrPtr->next;
                }
                if(i) {
                    if(i == 1) _xmlattr[0].next = NULL; //如果一个属性，需要将next指向NULL
                    _xmlnode[_n].attr = _xmlattr;
                }
                break;
            case XML_TEXT_NODE:
                //内容
                szKey = xmlNodeGetContent(curNode);
                mbstowcs(_xmlnode[_n].text, szKey, sizeof(_xmlnode[_n].text) - 1);
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