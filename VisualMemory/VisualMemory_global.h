#ifndef VISUALMEMORY_GLOBAL_H
#define VISUALMEMORY_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef VISUALMEMORY_LIB
# define VISUALMEMORY_EXPORT Q_DECL_EXPORT
#else
# define VISUALMEMORY_EXPORT Q_DECL_IMPORT
#endif

#endif // VISUALMEMORY_GLOBAL_H
