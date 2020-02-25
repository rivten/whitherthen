/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// NOTE(casey): Compilers
//

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#ifdef _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(casey): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SEE/NEON optimizations are not available for this compiler yet!!!!
#endif

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

typedef uintptr_t umm;
typedef intptr_t smm;

typedef b32 b32x;
typedef u32 u32x;

#define flag8(type) u8
#define flag16(type) u16
#define flag32(type) u32
#define flag64(type) u64

#define enum8(type) u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX

#define UMMFromPointer(Pointer) ((umm)(Pointer))
#define PointerFromUMM(type, Value) (type *)(Value)

#define U32FromPointer(Pointer) ((u32)(memory_index)(Pointer))
#define PointerFromU32(type, Value) (type *)((memory_index)Value)

#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)

#define FILE_AND_LINE__(A, B) A "|" #B
#define FILE_AND_LINE_(A, B) FILE_AND_LINE__(A, B)
#define FILE_AND_LINE FILE_AND_LINE_(__FILE__, __LINE__)

struct buffer
{
    umm Count;
    u8 *Data;
};
typedef buffer string;

union v2
{
    struct
    {
        real32 x, y;
    };
    struct
    {
        real32 u, v;
    };
    struct
    {
        real32 Width, Height;
    };
    real32 E[2];
};

union v2u
{
    struct
    {
        u32 x, y;
    };
    struct
    {
        u32 Width, Height;
    };
    u32 E[2];
};

union v3
{
    struct
    {
        real32 x, y, z;
    };
    struct
    {
        real32 u, v, __;
    };
    struct
    {
        real32 r, g, b;
    };
    struct
    {
        v2 xy;
        real32 Ignored0_;
    };
    struct
    {
        real32 Ignored1_;
        v2 yz;
    };
    struct
    {
        v2 uv;
        real32 Ignored2_;
    };
    struct
    {
        real32 Ignored3_;
        v2 v__;
    };
    f32 E[3];
};

union v3s
{
    struct
    {
        s32 x;
        s32 y;
        s32 z;
    };
    s32 E[3];
};

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                real32 x, y, z;
            };
        };
        
        real32 w;
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                real32 r, g, b;
            };
        };
        
        real32 a;
    };
    struct
    {
        v2 xy;
        real32 Ignored0_;
        real32 Ignored1_;
    };
    struct
    {
        real32 Ignored2_;
        v2 yz;
        real32 Ignored3_;
    };
    struct
    {
        real32 Ignored4_;
        real32 Ignored5_;
        v2 zw;
    };
    f32 E[4];
};

struct m4x4
{
    // NOTE(casey): These are stored ROW MAJOR - E[ROW][COLUMN]!!!
    f32 E[4][4];
};

struct m4x4_inv
{
    m4x4 Forward;
    m4x4 Inverse;
};

struct rectangle2i
{
    s32 MinX, MinY;
    s32 MaxX, MaxY;
};

struct rectangle2
{
    v2 Min;
    v2 Max;
};

struct rectangle3
{
    v3 Min;
    v3 Max;
};

union dev_id_value
{
    void *Void;
    u64 U64;
    u32 U32[2];
};
struct dev_id
{
    dev_id_value Value[2];
};

struct ticket_mutex
{
    u64 volatile Ticket;
    u64 volatile Serving;
};

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global static

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

#if COMPILE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#if 0
// NOTE(casey): This is a pre-C++11 way
#define CTAssert3(Expr, Line) struct CTAssert_____##Line {int Foo[(Expr) ? 1 : -1];};
#define CTAssert2(Expr, Line) CTAssert3(Expr, Line)
#define CTAssert(Expr) CTAssert2(Expr, __LINE__)
#else
#define CTAssert(Expr) static_assert(Expr, "Assertion failed: " #Expr)
#endif

#if COMPILE_INTERNAL
#define NotImplemented Assert(!"NotImplemented")
#else
#define NotImplemented NotImplemented!!!!!!!!!!!!!!
#endif

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(casey): swap, min, max ... macros???

// NOTE(casey): Added (Value-Value) here to force integral promotion to the size of Value
#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Value - Value) + (Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

inline b32x IsPow2(u32 Value)
{
    b32x Result = ((Value & ~(Value - 1)) == Value);
    return(Result);
}

#if COMPILER_MSVC
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = _InterlockedCompareExchange((long volatile *)Value, New, Expected);
    
    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = _InterlockedExchange64((__int64 volatile *)Value, New);
    
    return(Result);
}
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u64 Result = _InterlockedExchangeAdd64((__int64 volatile *)Value, Addend);
    
    return(Result);
}
inline u32 GetThreadID(void)
{
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    u32 ThreadID = *(u32 *)(ThreadLocalStorage + 0x48);
    
    return(ThreadID);
}

#elif COMPILER_LLVM
// TODO(casey): Does LLVM have real read-specific barriers yet?
#define CompletePreviousReadsBeforeFutureReads asm volatile("" ::: "memory")
#define CompletePreviousWritesBeforeFutureWrites asm volatile("" ::: "memory")
inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = __sync_val_compare_and_swap(Value, Expected, New);
    
    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = __sync_lock_test_and_set(Value, New);
    
    return(Result);
}
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u64 Result = __sync_fetch_and_add(Value, Addend);
    
    return(Result);
}
inline u32 GetThreadID(void)
{
    u32 ThreadID;
#if defined(__APPLE__) && defined(__x86_64__)
    asm("mov %%gs:0x00,%0" : "=r"(ThreadID));
#elif defined(__i386__)
    asm("mov %%gs:0x08,%0" : "=r"(ThreadID));
#elif defined(__x86_64__)
    asm("mov %%fs:0x10,%0" : "=r"(ThreadID));
#else
#error Unsupported architecture
#endif
    
    return(ThreadID);
}
#else
// TODO(casey): Other compilers/platforms??
#endif

inline u32
SafeTruncateToU32(u64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

inline u16
SafeTruncateToU16(u32 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFF);
    u16 Result = (u16)Value;
    return(Result);
}

inline u8
SafeTruncateToU8(u64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFF);
    u8 Result = (u8)Value;
    return(Result);
}

inline void
BeginTicketMutex(ticket_mutex *Mutex)
{
    u64 Ticket = AtomicAddU64(&Mutex->Ticket, 1);
    while(Ticket != Mutex->Serving) {_mm_pause();}
}

inline void
EndTicketMutex(ticket_mutex *Mutex)
{
    AtomicAddU64(&Mutex->Serving, 1);
}

inline u32
StringLength(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }
    
    return(Count);
}
