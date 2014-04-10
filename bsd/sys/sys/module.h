#ifndef _OSV_BSD_MODULE_H
#define _OSV_BSD_MODULE_H

typedef struct module *module_t;
typedef int (*modeventhand_t)(module_t, int /* modeventtype_t */, void *);

/*
 * Struct for registering modules statically via SYSINIT.
 */
typedef struct moduledata {
	const char	*name;		/* module name */
	modeventhand_t  evhand;		/* event handler */
	void		*priv;		/* extra data */
} moduledata_t;

#endif
