#include <wchar.h>

extern void xmlnode_init();
extern void xmlnode_print(int n);
extern int xmlnode_parse(const char *xml);
extern wchar_t **xmlnode_gettext_byname(const wchar_t *name, int *n);
extern wchar_t *xmlnode_getattrval_byname(const wchar_t *name, const wchar_t *attr);