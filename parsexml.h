extern void xmlnode_init();
extern void xmlnode_print(int n);
extern int xmlnode_parse(const char *xml);
extern char **xmlnode_gettext_byname(const char *name, int *n);
extern char *xmlnode_getattrval_byname(const char *name, const char *attr);