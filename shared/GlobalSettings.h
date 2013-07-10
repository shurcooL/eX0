#pragma once
#ifndef __GlobalSettings_H__
#define __GlobalSettings_H__

#define EX0_DEBUG
#define EX0_VERSION "0.002"

#ifdef EX0_CLIENT
#	define EX0_PRODUCT_STRING "eX0"
#else
#	define EX0_PRODUCT_STRING "eX0ds"
#endif // EX0_CLIENT

#ifdef EX0_DEBUG
#	define EX0_DEBUG_OPTIONS_STRING " with Debug options"
#	define ED(x) /*x*/
#else
#	define EX0_DEBUG_OPTIONS_STRING ""
#	define ED(x) x
#endif // EX0_DEBUG

#if defined NDEBUG
#	define EX0_BUILD_STRING EX0_PRODUCT_STRING" v"EX0_VERSION" (Release Build"EX0_DEBUG_OPTIONS_STRING", built on "__DATE__" at "__TIME__")"
#elif defined _DEBUG
#	define EX0_BUILD_STRING EX0_PRODUCT_STRING" v"EX0_VERSION" (Debug Build"EX0_DEBUG_OPTIONS_STRING", built on "__DATE__" at "__TIME__")"
#else
#	error "Neither NDEBUG nor _DEBUG are defined."
#endif // DEBUG

#define NOMINMAX

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;
typedef unsigned long long	u_int64;

typedef char				int8;
typedef unsigned char		uint8;
typedef short				int16;
typedef unsigned short		uint16;
typedef int					int32;
typedef unsigned int		uint32;
typedef long long			int64;
typedef unsigned long long	uint64;

#define nullptr 0

#endif // __GlobalSettings_H__
