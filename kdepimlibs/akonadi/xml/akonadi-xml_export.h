
#ifndef AKONADI_XML_EXPORT_H
#define AKONADI_XML_EXPORT_H

#ifdef AKONADI_XML_STATIC_DEFINE
#  define AKONADI_XML_EXPORT
#  define AKONADI_XML_NO_EXPORT
#else
#  ifndef AKONADI_XML_EXPORT
#    ifdef MAKE_AKONADI_XML_LIB
        /* We are building this library */
#      define AKONADI_XML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define AKONADI_XML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef AKONADI_XML_NO_EXPORT
#    define AKONADI_XML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef AKONADI_XML_DEPRECATED
#  define AKONADI_XML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef AKONADI_XML_DEPRECATED_EXPORT
#  define AKONADI_XML_DEPRECATED_EXPORT AKONADI_XML_EXPORT AKONADI_XML_DEPRECATED
#endif

#ifndef AKONADI_XML_DEPRECATED_NO_EXPORT
#  define AKONADI_XML_DEPRECATED_NO_EXPORT AKONADI_XML_NO_EXPORT AKONADI_XML_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define AKONADI_XML_NO_DEPRECATED
#endif

#endif
