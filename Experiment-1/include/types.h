#pragma once

#include <cstdint>

#if UINTPTR_MAX == UINT64_MAX
#define ARCH_64BIT 1
#define ARCH_BITS 64
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef u32 uint;

typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;
#elif UINTPTR_MAX == UINT32_MAX
#define ARCH_32BIT 1
#define ARCH_BITS 32
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef u32 uint;

typedef int i32;
typedef short i16;
typedef char i8;
#endif