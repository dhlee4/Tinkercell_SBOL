<<<<<<< HEAD
#ifndef LIB_LA_LIB_UTIL_H
#define LIB_LA_LIB_UTIL_H

#ifdef WIN32
#if LIB_EXPORTS
#  define LIB_EXTERN __declspec(dllexport)
#else
#  define LIB_EXTERN __declspec(dllimport)
#endif
#else
#  define LIB_EXTERN
#endif

#if __cplusplus
#  define BEGIN_C_DECLS extern "C" { //
#  define END_C_DECLS   } //
#else
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif

#endif


=======
#ifndef LIB_LA_LIB_UTIL_H
#define LIB_LA_LIB_UTIL_H

#ifdef WIN32
#if LIB_EXPORTS
#  define LIB_EXTERN __declspec(dllexport)
#else
#  define LIB_EXTERN __declspec(dllimport)
#endif
#else
#  define LIB_EXTERN
#endif

#if __cplusplus
#  define BEGIN_C_DECLS extern "C" { //
#  define END_C_DECLS   } //
#else
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif

#endif


>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
