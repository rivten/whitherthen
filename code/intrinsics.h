/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// TODO(casey): Convert all of these to platform-efficient versions
// and remove math.h
//
// cosf
// sinf
// atan2f
// ceilf
// floorf
//
//

#include <math.h>

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline int32
SignOf(int32 Value)
{
    int32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline real32
SignOf(real32 Value)
{
    real32 Result = (Value >= 0) ? 1.0f : -1.0f;
    return(Result);
}

inline f32
SquareRoot(f32 Real32)
{
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Real32)));
    return(Result);
}

inline f32
ReciprocalSquareRoot(f32 Real32)
{
    f32 Result = (1.0f / SquareRoot(Real32));
    return(Result);
}

inline real32
AbsoluteValue(real32 Real32)
{
    real32 Result = fabsf(Real32);
    return(Result);
}

inline uint32
RotateLeft(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotl(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(Result);
}

inline u64
RotateLeft(u64 Value, s32 Amount)
{
#if COMPILER_MSVC
    u64 Result = _rotl64(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 63;
    u64 Result = ((Value << Amount) | (Value >> (64 - Amount)));
#endif
    
    return(Result);
}

inline uint32
RotateRight(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}

inline s32
RoundReal32ToInt32(f32 Real32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline u32
RoundReal32ToUInt32(real32 Real32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline int32
FloorReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline int32
CeilReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return(Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = cosf(Angle);
    return(Result);
}

inline real32
ATan2(real32 Y, real32 X)
{
    real32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    uint32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}

inline bit_scan_result
FindMostSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};
    
#if COMPILER_MSVC
    Result.Found = _BitScanReverse((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 32;
        Test > 0;
        --Test)
    {
        if(Value & (1 << (Test - 1)))
        {
            Result.Index = Test - 1;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}
