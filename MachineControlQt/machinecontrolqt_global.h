#pragma once

#include <qobject.h>
#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MACHINECONTROLQT_LIB)
#  define MACHINECONTROLQT_EXPORT Q_DECL_EXPORT
# else
#  define MACHINECONTROLQT_EXPORT Q_DECL_IMPORT
# endif
#else
# define MACHINECONTROLQT_EXPORT
#endif
