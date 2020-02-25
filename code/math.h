/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define CUBE(a) ((a)*(a)*(a))
#define SQUARE(a) ((a)*(a))

inline v2u
V2U(u32 X, u32 Y)
{
    v2u Result = {X, Y};
    
    return(Result);
}

inline v2
V2i(int32 X, int32 Y)
{
    v2 Result = {(real32)X, (real32)Y};
    
    return(Result);
}

inline v2
V2i(uint32 X, uint32 Y)
{
    v2 Result = {(real32)X, (real32)Y};
    
    return(Result);
}

inline v2
V2(real32 X, real32 Y)
{
    v2 Result;
    
    Result.x = X;
    Result.y = Y;
    
    return(Result);
}

inline v2
V2From(v2u Source)
{
    v2 Result = {(f32)Source.x, (f32)Source.y};
    return(Result);
}

inline v3
V3(real32 X, real32 Y, real32 Z)
{
    v3 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return(Result);
}

inline v3
V3(v2 XY, real32 Z)
{
    v3 Result;
    
    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;
    
    return(Result);
}

inline v4
V4(real32 X, real32 Y, real32 Z, real32 W)
{
    v4 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    
    return(Result);
}

inline v4
V4(v3 XYZ, real32 W)
{
    v4 Result;
    
    Result.xyz = XYZ;
    Result.w = W;
    
    return(Result);
}

//
// NOTE(casey): Scalar operations
//

inline real32
Square(real32 A)
{
    real32 Result = A*A;
    
    return(Result);
}

inline r32
Sin01(r32 t)
{
    r32 Result = Sin(Pi32*t);
    
    return(Result);
}

inline r32
Triangle01(r32 t)
{
    r32 Result = 2.0f*t;
    if(Result > 1.0f)
    {
        Result = 2.0f - Result;
    }
    
    return(Result);
}

inline real32
Lerp(real32 A, real32 t, real32 B)
{
    real32 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

inline s32
S32BinormalLerp(s32 A, f32 tBinormal, s32 B)
{
    f32 t = 0.5f + (0.5f*tBinormal);
    f32 fResult = Lerp((f32)A, t, (f32)B);
    s32 Result = RoundReal32ToInt32(fResult);
    
    return(Result);
}

inline s32
Clamp(s32 Min, s32 Value, s32 Max)
{
    s32 Result = Value;
    
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }
    
    return(Result);
}

inline real32
Clamp(real32 Min, real32 Value, real32 Max)
{
    real32 Result = Value;
    
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }
    
    return(Result);
}

inline real32
Clamp01(real32 Value)
{
    real32 Result = Clamp(0.0f, Value, 1.0f);
    
    return(Result);
}

inline real32
Clamp01MapToRange(real32 Min, real32 t, real32 Max)
{
    real32 Result = 0.0f;
    
    real32 Range = Max - Min;
    if(Range != 0.0f)
    {
        Result = Clamp01((t - Min) / Range);
    }
    
    return(Result);
}

inline real32
ClampBinormalMapToRange(real32 Min, real32 t, real32 Max)
{
    real32 Result = -1.0f + 2.0f*Clamp01MapToRange(Min, t, Max);
    return(Result);
}

inline r32
ClampAboveZero(r32 Value)
{
    r32 Result = (Value < 0) ? 0.0f : Value;
    return(Result);
}

inline real32
SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
    real32 Result = N;
    
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }
    
    return(Result);
}

inline real32
SafeRatio0(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
    
    return(Result);
}

inline real32
SafeRatio1(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    
    return(Result);
}

inline f64
SafeRatioN(f64 Numerator, f64 Divisor, f64 N)
{
    f64 Result = N;
    
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }
    
    return(Result);
}

inline f64
SafeRatio0(f64 Numerator, f64 Divisor)
{
    f64 Result = SafeRatioN(Numerator, Divisor, 0.0);
    
    return(Result);
}

//
// NOTE(casey): v2 operations
//

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}

inline v2
operator*(real32 A, v2 B)
{
    v2 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2
operator*(v2 B, real32 A)
{
    v2 Result = A*B;
    
    return(Result);
}

inline v2 &
operator*=(v2 &B, real32 A)
{
    B = A * B;
    
    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;
    
    return(A);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};
    
    return(Result);
}

inline real32
Inner(v2 A, v2 B)
{
    real32 Result = A.x*B.x + A.y*B.y;
    
    return(Result);
}

inline real32
LengthSq(v2 A)
{
    real32 Result = Inner(A, A);
    
    return(Result);
}

inline real32
Length(v2 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    
    return(Result);
}

inline v2
Arm2(r32 Angle)
{
    v2 Result = {Cos(Angle), Sin(Angle)};
    
    return(Result);
}

//
// NOTE(casey): v3 operations
//

inline v3
operator*(real32 A, v3 B)
{
    v3 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    
    return(Result);
}

inline v3
operator*(v3 B, real32 A)
{
    v3 Result = A*B;
    
    return(Result);
}

inline v3 &
operator*=(v3 &B, real32 A)
{
    B = A * B;
    
    return(B);
}

inline v3
operator/(v3 B, real32 A)
{
    v3 Result = (1.0f/A)*B;
    
    return(Result);
}

inline v3
operator/(f32 B, v3 A)
{
    v3 Result =
    {
        B / A.x,
        B / A.y,
        B / A.z,
    };
    
    return(Result);
}

inline v3 &
operator/=(v3 &B, real32 A)
{
    B = B / A;
    
    return(B);
}

inline v3
operator-(v3 A)
{
    v3 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return(Result);
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;
    
    return(A);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}

inline v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;
    
    return(A);
}

inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    
    return(Result);
}

inline v3s
Hadamard(v3s A, v3s B)
{
    v3s Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    
    return(Result);
}

inline real32
Inner(v3 A, v3 B)
{
    real32 Result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return(Result);
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;
    
    return(Result);
}

inline real32
LengthSq(v3 A)
{
    real32 Result = Inner(A, A);
    
    return(Result);
}

inline real32
Length(v3 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v3
Normalize(v3 A)
{
    v3 Result = A * (1.0f / Length(A));
    
    return(Result);
}

inline v3
NOZ(v3 A)
{
    v3 Result = {};
    
    r32 LenSq = LengthSq(A);
    if(LenSq > Square(0.0001f))
    {
        Result = A * (1.0f / SquareRoot(LenSq));
    }
    
    return(Result);
}

inline v3
Clamp01(v3 Value)
{
    v3 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    
    return(Result);
}

inline v3
Lerp(v3 A, real32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

inline v3
Min(v3 A, v3 B)
{
    v3 Result =
    {
        Minimum(A.x, B.x),
        Minimum(A.y, B.y),
        Minimum(A.z, B.z),
    };
    
    return(Result);
}

inline v3
Max(v3 A, v3 B)
{
    v3 Result =
    {
        Maximum(A.x, B.x),
        Maximum(A.y, B.y),
        Maximum(A.z, B.z),
    };
    
    return(Result);
}

//
// NOTE(casey): v4 operations
//

inline v4
operator*(real32 A, v4 B)
{
    v4 Result;
    
    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;
    
    return(Result);
}

inline v4
operator*(v4 B, real32 A)
{
    v4 Result = A*B;
    
    return(Result);
}

inline v4 &
operator*=(v4 &B, real32 A)
{
    B = A * B;
    
    return(B);
}

inline v4
operator-(v4 A)
{
    v4 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;
    
    return(Result);
}

inline v4
operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

inline v4 &
operator+=(v4 &A, v4 B)
{
    A = A + B;
    
    return(A);
}

inline v4
operator-(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return(Result);
}

inline v4 &
operator-=(v4 &A, v4 B)
{
    A = A - B;
    
    return(A);
}

inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};
    
    return(Result);
}

inline real32
Inner(v4 A, v4 B)
{
    real32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    
    return(Result);
}

inline real32
LengthSq(v4 A)
{
    real32 Result = Inner(A, A);
    
    return(Result);
}

inline real32
Length(v4 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v4
Clamp01(v4 Value)
{
    v4 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);
    
    return(Result);
}

inline v4
Lerp(v4 A, real32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}

//
// NOTE(casey): Rectangle2
//

inline rectangle2
InvertedInfinityRectangle2(void)
{
    rectangle2 Result;
    
    Result.Min.x = Result.Min.y = F32Max;
    Result.Max.x = Result.Max.y = -F32Max;
    
    return(Result);
}

inline rectangle2
Union(rectangle2 A, rectangle2 B)
{
    rectangle2 Result;
    
    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;
    
    return(Result);
}

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2
GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline rectangle2
Rectangle2From(rectangle2i Source)
{
    rectangle2 Result =
    {
        (f32)Source.MinX,
        (f32)Source.MinY,
        (f32)Source.MaxX,
        (f32)Source.MaxY,
    };
    
    return(Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle2
Offset(rectangle2 A, v2 Offset)
{
    rectangle2 Result;
    
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;
    
    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    
    return(Result);
}

inline bool32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y));
    
    return(Result);
}

inline b32
RectanglesIntersect(rectangle2 A, rectangle2 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) ||
                   (B.Min.x >= A.Max.x) ||
                   (B.Max.y <= A.Min.y) ||
                   (B.Min.y >= A.Max.y));
    return(Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    
    return(Result);
}

inline r32
GetArea(rectangle2 A)
{
    v2 Dim = GetDim(A);
    r32 Result = Dim.x*Dim.y;
    return(Result);
}

//
// NOTE(casey): Rectangle3
//

inline rectangle3
InvertedInfinityRectangle3(void)
{
    rectangle3 Result;
    
    Result.Min.x = Result.Min.y = Result.Min.z = F32Max;
    Result.Max.x = Result.Max.y = Result.Max.z = -F32Max;
    
    return(Result);
}

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v3
GetRadius(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Max - Rect.Min);
    return(Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;
    
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;
    
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;
    
    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    
    return(Result);
}

inline bool32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.z >= Rectangle.Min.z) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y) &&
                     (Test.z < Rectangle.Max.z));
    
    return(Result);
}

inline b32x
IsInRectangleCenterHalfDim(v3 P, v3 Radius, v3 Test)
{
    v3 Rel = Test - P;
    b32x Result = ((AbsoluteValue(Rel.x) <= Radius.x) &&
                   (AbsoluteValue(Rel.y) <= Radius.y) &&
                   (AbsoluteValue(Rel.z) <= Radius.z));
    
    return(Result);
}

inline bool32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    bool32 Result = !((B.Max.x <= A.Min.x) ||
                      (B.Min.x >= A.Max.x) ||
                      (B.Max.y <= A.Min.y) ||
                      (B.Min.y >= A.Max.y) ||
                      (B.Max.z <= A.Min.z) ||
                      (B.Min.z >= A.Max.z));
    return(Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);
    
    return(Result);
}

inline v3
PointFromUVW(rectangle3 A, v3 UVW)
{
    v3 Result;
    
    Result.x = Lerp(A.Min.x, UVW.x, A.Max.x);
    Result.y = Lerp(A.Min.y, UVW.y, A.Max.y);
    Result.z = Lerp(A.Min.z, UVW.z, A.Max.z);
    
    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;
    
    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;
    
    return(Result);
}

inline rectangle3
Union(rectangle3 A, rectangle3 B)
{
    rectangle3 Result;
    
    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Min.z = (A.Min.z < B.Min.z) ? A.Min.z : B.Min.z;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;
    Result.Max.z = (A.Max.z > B.Max.z) ? A.Max.z : B.Max.z;
    
    return(Result);
}

internal v3
GetMinZCenterP(rectangle3 R)
{
    v3 Result = GetCenter(R);
    Result.z = GetMinCorner(R).z;
    return(Result);
}

internal v3
GetMaxZCenterP(rectangle3 R)
{
    v3 Result = GetCenter(R);
    Result.z = GetMaxCorner(R).z;
    return(Result);
}

internal rectangle3
MakeRelative(rectangle3 R, v3 P)
{
    rectangle3 Result = Offset(R, -P);
    return(Result);
}

//
//
//

inline s32
GetWidth(rectangle2i A)
{
    s32 Result = A.MaxX - A.MinX;
    return(Result);
}

inline s32
GetHeight(rectangle2i A)
{
    s32 Result = A.MaxY - A.MinY;
    return(Result);
}

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;
    
    Result.MinX = (A.MinX < B.MinX) ? B.MinX : A.MinX;
    Result.MinY = (A.MinY < B.MinY) ? B.MinY : A.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? B.MaxX : A.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? B.MaxY : A.MaxY;
    
    return(Result);
}

inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;
    
    Result.MinX = (A.MinX < B.MinX) ? A.MinX : B.MinX;
    Result.MinY = (A.MinY < B.MinY) ? A.MinY : B.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? A.MaxX : B.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? A.MaxY : B.MaxY;
    
    return(Result);
}

inline int32
GetClampedRectArea(rectangle2i A)
{
    int32 Width = (A.MaxX - A.MinX);
    int32 Height = (A.MaxY - A.MinY);
    int32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width*Height;
    }
    
    return(Result);
}

inline bool32
HasArea(rectangle2i A)
{
    bool32 Result = ((A.MinX < A.MaxX) && (A.MinY < A.MaxY));
    
    return(Result);
}

inline b32x
HasArea(rectangle3 A)
{
    b32x Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y) && (A.Min.z < A.Max.z));
    
    return(Result);
}

inline rectangle2i
InvertedInfinityRectangle2i(void)
{
    rectangle2i Result;
    
    Result.MinX = Result.MinY = INT_MAX;
    Result.MaxX = Result.MaxY = -INT_MAX;
    
    return(Result);
}

inline rectangle2i
Offset(rectangle2i A, s32 X, s32 Y)
{
    rectangle2i Result = A;
    
    Result.MinX += X;
    Result.MaxX += X;
    Result.MinY += Y;
    Result.MaxY += Y;
    
    return(Result);
}

inline rectangle2i
RectMinMax(s32 MinX, s32 MinY, s32 MaxX, s32 MaxY)
{
    rectangle2i Result;
    
    Result.MinX = MinX;
    Result.MinY = MinY;
    Result.MaxX = MaxX;
    Result.MaxY = MaxY;
    
    return(Result);
}

inline rectangle2i
RectMinDim(s32 MinX, s32 MinY, s32 DimX, s32 DimY)
{
    rectangle2i Result;
    
    Result.MinX = MinX;
    Result.MinY = MinY;
    Result.MaxX = MinX + DimX;
    Result.MaxY = MinY + DimY;
    
    return(Result);
}

inline v4
sRGBLinearize(v4 C)
{
    v4 Result;
    
    Result.r = Square(C.r);
    Result.g = Square(C.g);
    Result.b = Square(C.b);
    Result.a = C.a;
    
    return(Result);
}

inline v4
LinearTosRGB(v4 C)
{
    v4 Result;
    
    Result.r = SquareRoot(C.r);
    Result.g = SquareRoot(C.g);
    Result.b = SquareRoot(C.b);
    Result.a = C.a;
    
    return(Result);
}

inline v4
sRGBLinearize(f32 R, f32 G, f32 B, f32 A)
{
    v4 Input = {R, G, B, A};
    v4 Result = sRGBLinearize(Input);
    return(Result);
}

inline v4
SRGB255ToLinear1(v4 C)
{
    v4 Result;
    
    real32 Inv255 = 1.0f / 255.0f;
    
    Result.r = Square(Inv255*C.r);
    Result.g = Square(Inv255*C.g);
    Result.b = Square(Inv255*C.b);
    Result.a = Inv255*C.a;
    
    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;
    
    real32 One255 = 255.0f;
    
    Result.r = One255*SquareRoot(C.r);
    Result.g = One255*SquareRoot(C.g);
    Result.b = One255*SquareRoot(C.b);
    Result.a = One255*C.a;
    
    return(Result);
}

internal m4x4
operator*(m4x4 A, m4x4 B)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    
    m4x4 R = {};
    
    for(int r = 0; r <= 3; ++r) // NOTE(casey): Rows (of A)
    {
        for(int c = 0; c <= 3; ++c) // NOTE(casey): Column (of B)
        {
            for(int i = 0; i <= 3; ++i) // NOTE(casey): Columns of A, rows of B!
            {
                R.E[r][c] += A.E[r][i]*B.E[i][c];
            }
        }
    }
    
    return(R);
}

internal v4
Transform(m4x4 A, v4 P)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    
    v4 R;
    
    R.x = P.x*A.E[0][0] + P.y*A.E[0][1] + P.z*A.E[0][2] + P.w*A.E[0][3];
    R.y = P.x*A.E[1][0] + P.y*A.E[1][1] + P.z*A.E[1][2] + P.w*A.E[1][3];
    R.z = P.x*A.E[2][0] + P.y*A.E[2][1] + P.z*A.E[2][2] + P.w*A.E[2][3];
    R.w = P.x*A.E[3][0] + P.y*A.E[3][1] + P.z*A.E[3][2] + P.w*A.E[3][3];
    
    return(R);
}

inline v3
operator*(m4x4 A, v3 P)
{
    v3 R = Transform(A, V4(P, 1.0f)).xyz;
    return(R);
}

inline v4
operator*(m4x4 A, v4 P)
{
    v4 R = Transform(A, P);
    return(R);
}

inline m4x4
Identity(void)
{
    m4x4 R =
    {
        {{1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline m4x4
XRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);
    
    m4x4 R =
    {
        {{1, 0, 0, 0},
            {0, c,-s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline m4x4
YRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);
    
    m4x4 R =
    {
        {{ c, 0, s, 0},
            { 0, 1, 0, 0},
            {-s, 0, c, 0},
            { 0, 0, 0, 1}},
    };
    
    return(R);
}

inline m4x4
ZRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);
    
    m4x4 R =
    {
        {{c,-s, 0, 0},
            {s, c, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline m4x4
Translation(v3 T)
{
    m4x4 R =
    {
        {{1, 0, 0, T.x},
            {0, 1, 0, T.y},
            {0, 0, 1, T.z},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline m4x4
Transpose(m4x4 A)
{
    m4x4 R;
    
    for(int j = 0; j <= 3; ++j)
    {
        for(int i = 0; i <= 3; ++i)
        {
            R.E[j][i] = A.E[i][j];
        }
    }
    
    return(R);
}

inline m4x4_inv
PerspectiveProjection(f32 AspectWidthOverHeight, f32 FocalLength, f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight;
    f32 c = FocalLength; // NOTE(casey): This should really be called "film back distance"
    
    f32 n = NearClipPlane; // NOTE(casey): Near clip plane _distance_
    f32 f = FarClipPlane; // NOTE(casey): Far clip plane _distance_
    
    // NOTE(casey): These are the perspective correct terms, for when you divide by -z
    f32 d = (n+f) / (n-f);
    f32 e = (2*f*n) / (n-f);
    
    m4x4_inv Result =
    {
        // NOTE(casey): Forward
        {{{a*c,    0,  0,  0},
                {  0,  b*c,  0,  0},
                {  0,    0,  d,  e},
                {  0,    0, -1,  0}}},
        
        // NOTE(casey): Inverse
        {{{1/(a*c),       0,   0,   0},
                {      0, 1/(b*c),   0,   0},
                {      0,       0,   0,  -1},
                {      0,       0, 1/e, d/e}}},
    };
    
#if COMPILE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v4 Test0 = Result.Forward*V4(0, 0, -n, 1);
    Test0.xyz /= Test0.w;
    v4 Test1 = Result.Forward*V4(0, 0, -f, 1);
    Test1.xyz /= Test1.w;
#endif
    
    return(Result);
}

inline m4x4_inv
OrthographicProjection(f32 AspectWidthOverHeight, f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight;
    
    f32 n = NearClipPlane; // NOTE(casey): Near clip plane _distance_
    f32 f = FarClipPlane; // NOTE(casey): Far clip plane _distance_
    
    // NOTE(casey): These are the non-perspective corrected terms, for orthographic
    f32 d = 2.0f / (n - f);
    f32 e = (n + f) / (n - f);
    
    m4x4_inv Result =
    {
        {{{a,  0,  0,  0},
                {0,  b,  0,  0},
                {0,  0,  d,  e},
                {0,  0,  0,  1}}},
        
        {{{1/a,   0,   0,    0},
                {  0, 1/b,   0,    0},
                {  0,   0, 1/d, -e/d},
                {  0,   0,   0,    1}}},
    };
    
#if COMPILE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v3 Test0 = Result.Forward*V3(0, 0, -n);
    v3 Test1 = Result.Forward*V3(0, 0, -f);
#endif
    
    return(Result);
}

internal m4x4
Columns3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 R =
    {
        {{X.x, Y.x, Z.x, 0},
            {X.y, Y.y, Z.y, 0},
            {X.z, Y.z, Z.z, 0},
            {  0,   0,   0, 1}}
    };
    
    return(R);
}

internal m4x4
Rows3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 R =
    {
        {{X.x, X.y, X.z, 0},
            {Y.x, Y.y, Y.z, 0},
            {Z.x, Z.y, Z.z, 0},
            {  0,   0,   0, 1}}
    };
    
    return(R);
}

internal m4x4
Translate(m4x4 A, v3 T)
{
    m4x4 R = A;
    
    R.E[0][3] += T.x;
    R.E[1][3] += T.y;
    R.E[2][3] += T.z;
    
    return(R);
}

inline v3
GetColumn(m4x4 A, u32 C)
{
    v3 R = {A.E[0][C], A.E[1][C], A.E[2][C]};
    return(R);
}

inline v3
GetRow(m4x4 A, u32 R)
{
    v3 Result = {A.E[R][0], A.E[R][1], A.E[R][2]};
    return(Result);
}

internal m4x4_inv
CameraTransform(v3 X, v3 Y, v3 Z, v3 P)
{
    m4x4_inv Result;
    
    // TODO(casey): It seems really suspicious that unary negation binds first
    // to the m4x4... is that actually the C++ grammar?  I guess it is :(
    m4x4 A = Rows3x3(X, Y, Z);
    v3 AP = -(A*P);
    A = Translate(A, AP);
    Result.Forward = A;
    
    v3 iX = X/LengthSq(X);
    v3 iY = Y/LengthSq(Y);
    v3 iZ = Z/LengthSq(Z);
    v3 iP = {AP.x*iX.x + AP.y*iY.x + AP.z*iZ.x,
        AP.x*iX.y + AP.y*iY.y + AP.z*iZ.y,
        AP.x*iX.z + AP.y*iY.z + AP.z*iZ.z};
    
    m4x4 B = Columns3x3(iX, iY, iZ);
    B = Translate(B, -iP);
    Result.Inverse = B;
    
#if COMPILE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
#endif
    
    return(Result);
}

internal v2
RayIntersect2(v2 Pa, v2 ra, v2 Pb, v2 rb)
{
    v2 Result = {};
    
    /* NOTE(casey):
    
       Pa.x + ta*ra.x = Pb.x + tb*rb.x
       Pa.y + ta*ra.y = Pb.y + tb*rb.y
    */
    
    f32 d = (rb.x*ra.y - rb.y*ra.x);
    if(d != 0.0f)
    {
        f32 ta = ((Pa.x - Pb.x)*rb.y + (Pb.y - Pa.y)*rb.x) / d;
        f32 tb = ((Pa.x - Pb.x)*ra.y + (Pb.y - Pa.y)*ra.x) / d;
        
        Result = V2(ta, tb);
    }
    
    return(Result);
}

internal u32
SwapRAndB(u32 C)
{
    u32 Result = ((C & 0xFF00FF00) |
                  ((C >> 16) & 0xFF) |
                  ((C & 0xFF) << 16));
    
    return(Result);
}

internal u32
ReplAlpha(u32 C)
{
    u32 Alpha = (C >> 24);
    u32 Result = ((Alpha << 24) |
                  (Alpha << 16) |
                  (Alpha <<  8) |
                  (Alpha <<  0));
    
    return(Result);
}

internal u32
MulAlpha(u32 C)
{
    u32 C0 = ((C >> 0) & 0xFF);
    u32 C1 = ((C >> 8) & 0xFF);
    u32 C2 = ((C >> 16) & 0xFF);
    u32 Alpha = (C >> 24);
    
    // NOTE(casey): This is a quick-and-dirty lossy multiply, where you lose one bit
    C0 = ((C0*Alpha) >> 8);
    C1 = ((C1*Alpha) >> 8);
    C2 = ((C2*Alpha) >> 8);
    
    u32 Result = ((Alpha << 24) |
                  (C2 << 16) |
                  (C1 <<  8) |
                  (C0 <<  0));
    
    return(Result);
}

inline v4
BGRAUnpack4x8(u32 Packed)
{
    v4 Result = {(real32)((Packed >> 16) & 0xFF),
        (real32)((Packed >> 8) & 0xFF),
        (real32)((Packed >> 0) & 0xFF),
        (real32)((Packed >> 24) & 0xFF)};
    
    return(Result);
}

inline u32
BGRAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundReal32ToUInt32(Unpacked.a) << 24) |
                  (RoundReal32ToUInt32(Unpacked.r) << 16) |
                  (RoundReal32ToUInt32(Unpacked.g) << 8) |
                  (RoundReal32ToUInt32(Unpacked.b) << 0));
    
    return(Result);
}

inline u64
BGRAPack4x16(v4 Unpacked)
{
    u64 Result = (((u64)RoundReal32ToUInt32(Unpacked.a) << 48) |
                  ((u64)RoundReal32ToUInt32(Unpacked.r) << 32) |
                  ((u64)RoundReal32ToUInt32(Unpacked.g) << 16) |
                  ((u64)RoundReal32ToUInt32(Unpacked.b) << 0));
    
    return(Result);
}

inline v4
RGBAUnpack4x8(u32 Packed)
{
    v4 Result = {(real32)((Packed >> 0) & 0xFF),
        (real32)((Packed >> 8) & 0xFF),
        (real32)((Packed >> 16) & 0xFF),
        (real32)((Packed >> 24) & 0xFF)};
    
    return(Result);
}

inline u32
RGBAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundReal32ToUInt32(Unpacked.a) << 24) |
                  (RoundReal32ToUInt32(Unpacked.b) << 16) |
                  (RoundReal32ToUInt32(Unpacked.g) << 8) |
                  (RoundReal32ToUInt32(Unpacked.r) << 0));
    
    return(Result);
}

inline b32x
IsInRange(f32 Min, f32 Value, f32 Max)
{
    b32x Result = ((Min <= Value) &&
                   (Value <= Max));
    
    return(Result);
}

internal rectangle2i
AspectRatioFit(u32 RenderWidth, u32 RenderHeight,
               u32 WindowWidth, u32 WindowHeight)
{
    rectangle2i Result = {};
    
    if((RenderWidth > 0) &&
       (RenderHeight > 0) &&
       (WindowWidth > 0) &&
       (WindowHeight > 0))
    {
        r32 OptimalWindowWidth = (r32)WindowHeight * ((r32)RenderWidth / (r32)RenderHeight);
        r32 OptimalWindowHeight = (r32)WindowWidth * ((r32)RenderHeight / (r32)RenderWidth);
        
        if(OptimalWindowWidth > (r32)WindowWidth)
        {
            // NOTE(casey): Width-constrained display - top and bottom black bars
            Result.MinX = 0;
            Result.MaxX = WindowWidth;
            
            r32 Empty = (r32)WindowHeight - OptimalWindowHeight;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);
            s32 UseHeight = RoundReal32ToInt32(OptimalWindowHeight);
            
            Result.MinY = HalfEmpty;
            Result.MaxY = Result.MinY + UseHeight;
        }
        else
        {
            // NOTE(casey): Height-constrained display - left and right black bars
            Result.MinY = 0;
            Result.MaxY = WindowHeight;
            
            r32 Empty = (r32)WindowWidth - OptimalWindowWidth;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);
            s32 UseWidth = RoundReal32ToInt32(OptimalWindowWidth);
            
            Result.MinX = HalfEmpty;
            Result.MaxX = Result.MinX + UseWidth;
        }
    }
    
    return(Result);
}

internal r32
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, f32 HalfDimInMeters)
{
    f32 Result = (FocalLength*HalfDimInMeters) / MonitorHalfDimInMeters;
    return(Result);
}

internal v2
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, v2 HalfDimInMeters)
{
    v2 Result =
    {
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.x),
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.y),
    };
    
    return(Result);
}

internal v3s
FloorToV3S(v3 A)
{
    v3s Result =
    {
        FloorReal32ToInt32(A.x),
        FloorReal32ToInt32(A.y),
        FloorReal32ToInt32(A.z),
    };
    
    return(Result);
}

internal v3s
V3S(s32 X, s32 Y, s32 Z)
{
    v3s Result = {X, Y, Z};
    
    return(Result);
}

internal v3
V3(v3s A)
{
    v3 Result =
    {
        (f32)A.x,
        (f32)A.y,
        (f32)A.z,
    };
    
    return(Result);
}

internal v3s
operator-(v3s A, v3s B)
{
    v3s Result =
    {
        A.x - B.x,
        A.y - B.y,
        A.z - B.z,
    };
    
    return(Result);
}

internal v3s
operator+(v3s A, v3s B)
{
    v3s Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
    };
    
    return(Result);
}

internal v3
GetClosestPointInBox(rectangle3 Box, v3 P)
{
    v3 Result = Min(Box.Max, Max(Box.Min, P));
    return(Result);
}

internal f32
GetDistanceToBoxSq(rectangle3 Box, v3 P)
{
    v3 ClosestP = GetClosestPointInBox(Box, P);
    f32 DistanceSq = LengthSq(P - ClosestP);
    return(DistanceSq);
}

internal v2
OctahedralFromUnitVector(v3 V)
{
    f32 OneNorm = AbsoluteValue(V.x) + AbsoluteValue(V.y) + AbsoluteValue(V.z);
    v2 Result = (1.0f / OneNorm)*V.xy;
    if(V.z < 0)
    {
        f32 Ox = SignOf(Result.x) * (1.0f - AbsoluteValue(Result.y));
        f32 Oy = SignOf(Result.y) * (1.0f - AbsoluteValue(Result.x));
        Result.x = Ox;
        Result.y = Oy;
    }
    
    return(Result);
}

internal v3
UnitVectorFromOctahedral(v2 O)
{
    f32 Ox = O.x;
    f32 Oy = O.y;
    f32 SumXY = (AbsoluteValue(Ox) + AbsoluteValue(Oy));
    f32 Oz = 1.0f - SumXY;
    if(SumXY > 1)
    {
        Ox = SignOf(O.x) * (1.0f - AbsoluteValue(O.y));
        Oy = SignOf(O.y) * (1.0f - AbsoluteValue(O.x));
    }
    
    v3 Result = V3(Ox, Oy, Oz);
    Result = Normalize(Result);
    
    return(Result);
}
