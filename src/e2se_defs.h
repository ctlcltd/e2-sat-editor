/*!
 * e2-sat-editor/src/e2se_defs.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef e2se_defs_h
#define e2se_defs_h

#define E2SE_TARGET_DEBUG 0
#define E2SE_TARGET_RELEASE 1


// build target: debug, release
//#define E2SE_BUILD E2SE_TARGET_DEBUG

// portable application
//#define E2SE_PORTABLE

// demo mode
//#define E2SE_DEMO


#ifndef NOAUTODISCOVER

#ifndef E2SE_BUILD
#if defined(CMAKE_BUILD_TYPE)
#if CMAKE_BUILD_TYPE == "Debug"
#define E2SE_BUILD E2SE_TARGET_DEBUG
#else
#define E2SE_BUILD E2SE_TARGET_RELEASE
#endif
#elif defined(NDEBUG) || defined(_NDEBUG)
#define E2SE_BUILD E2SE_TARGET_RELEASE
#else
#define E2SE_BUILD E2SE_TARGET_DEBUG
#endif
#endif

#ifndef E2SE_PORTABLE
#if defined(WIN32) || defined(_WIN32)
#define E2SE_PORTABLE
#endif
#endif

#ifndef E2SE_DEMO
#if defined(__EMSCRIPTEN__)
#define E2SE_DEMO
#endif
#endif

#endif


#endif /* e2se_defs_h */
