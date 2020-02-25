/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdarg.h>

struct adler_32
{
    u32 S1;
    u32 S2;
};

#define ConstZ(Z) {sizeof(Z) - 1, (u8 *)(Z)}
#define BundleZ(Z) BundleString(sizeof(Z) - 1, (Z))

#define CopyArray(Count, Source, Dest) Copy((Count)*sizeof(*(Source)), (Source), (Dest))
internal void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}
    
    return(DestInit);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize(Count*sizeof((Pointer)[0]), Pointer)
internal void
ZeroSize(memory_index Size, void *Ptr)
{
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

internal b32x
IsValid(buffer Buffer)
{
    b32x Result = (Buffer.Count > 0);
    return(Result);
}

internal string
WrapZ(char *Z)
{
    string Result;
    
    Result.Count = StringLength(Z);
    Result.Data = (u8 *)Z;
    
    return(Result);
}

internal string
BundleString(umm Count, char *Z)
{
    string Result;
    
    Result.Count = Count;
    Result.Data = (u8 *)Z;
    
    return(Result);
}

internal string
RemoveExtension(string FileNameInit)
{
    string FileName = FileNameInit;
    
    umm NewCount = FileName.Count;
    for(umm Index = 0;
        Index < FileName.Count;
        ++Index)
    {
        char C = FileName.Data[Index];
        if(C == '.')
        {
            NewCount = Index;
        }
        else if((C == '/') || (C == '\\'))
        {
            NewCount = FileName.Count;
        }
    }
    
    FileName.Count = NewCount;
    
    return(FileName);
}

internal string
RemovePath(string FileNameInit)
{
    string FileName = FileNameInit;
    
    umm NewStart = 0;
    for(umm Index = 0;
        Index < FileName.Count;
        ++Index)
    {
        char C = FileName.Data[Index];
        if((C == '/') || (C == '\\'))
        {
            NewStart = Index + 1;
        }
    }
    
    FileName.Data += NewStart;
    FileName.Count -= NewStart;
    
    return(FileName);
}

internal u8 *
Advance(buffer *Buffer, umm Count)
{
    u8 *Result = 0;
    
    if(Buffer->Count >= Count)
    {
        Result = Buffer->Data;
        Buffer->Data += Count;
        Buffer->Count -= Count;
    }
    else
    {
        Buffer->Data += Buffer->Count;
        Buffer->Count = 0;
    }
    
    return(Result);
}

internal char
ToLowercase(char Char)
{
    char Result = Char;
    
    if((Result >= 'A') && (Result <= 'Z'))
    {
        Result += 'a' - 'A';
    }
    
    return(Result);
}

internal void
UpdateStringHash(u32 *HashValue, char Value)
{
    // TODO(casey): Better hash function
    *HashValue = 65599*(*HashValue) + Value;
}

internal u32
StringHashOf(char *Z)
{
    u32 HashValue = 0;
    
    while(*Z)
    {
        UpdateStringHash(&HashValue, *Z++);
    }
    
    return(HashValue);
}

internal u32
StringHashOf(string S)
{
    u32 HashValue = 0;
    
    for(umm Index = 0;
        Index < S.Count;
        ++Index)
    {
        UpdateStringHash(&HashValue, S.Data[Index]);
    }
    
    return(HashValue);
}

internal adler_32
BeginAdler32(void)
{
    adler_32 Result = {};
    Result.S1 = 1;
    Result.S2 = 0;
    return(Result);
}

internal void
Adler32Append(adler_32 *Adler, umm Size, void *Data)
{
    u32 S1 = Adler->S1;
    u32 S2 = Adler->S2;
    
    for(umm Index = 0; Index < Size; ++Index) 
    {
        S1 = (S1 + ((u8 *)Data)[Index]) % 65521;
        S2 = (S2 + S1) % 65521;
    }
    
    Adler->S1 = S1;
    Adler->S2 = S2;
}

internal u32 
EndAdler32(adler_32 *Adler)
{
    u32 Result = (Adler->S2*65536 + Adler->S1);
    return(Result);
}

internal b32
IsEndOfLine(char C)
{
    b32 Result = ((C == '\n') ||
                  (C == '\r'));
    
    return(Result);
}

internal b32
IsSpacing(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\v') ||
                  (C == '\f'));
    
    return(Result);
}

internal b32
IsWhitespace(char C)
{
    b32 Result = (IsSpacing(C) || IsEndOfLine(C));
    
    return(Result);
}

internal b32x
IsAlpha(char C)
{
    b32x Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));
    
    return(Result);
}

internal b32x
IsNumber(char C)
{
    b32x Result = ((C >= '0') && (C <= '9'));
    
    return(Result);
}

internal b32
IsHex(char Char)
{
    b32 Result = (((Char >= '0') && (Char <= '9')) ||
                  ((Char >= 'A') && (Char <= 'F')) ||
                  ((Char >= 'a') && (Char <= 'f')));
    
    return(Result);
}

internal u32
GetHex(char Char)
{
    u32 Result = 0;
    
    if((Char >= '0') && (Char <= '9'))
    {
        Result = Char - '0';
    }
    else if((Char >= 'A') && (Char <= 'F'))
    {
        Result = 0xA + (Char - 'A');
    }
    else if((Char >= 'a') && (Char <= 'f'))
    {
        Result = 0xA + (Char - 'a');
    }
    
    return(Result);
}

internal b32x
MemoryIsEqual(umm Count, void *AInit, void *BInit)
{
    u8 *A = (u8 *)AInit;
    u8 *B = (u8 *)BInit;
    while(Count--)
    {
        if(*A++ != *B++)
        {
            return(false);
        }
    }
    
    return(true);
}

internal b32x
MemoryIsEqual(buffer FileBuffer, buffer HHTContents)
{
    b32x Result = ((FileBuffer.Count == HHTContents.Count) &&
                   MemoryIsEqual(FileBuffer.Count, FileBuffer.Data, HHTContents.Data));
    return(Result);
}

internal b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);
    
    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        
        Result = ((*A == 0) && (*B == 0));
    }
    
    return(Result);
}

internal b32
StringsAreEqual(umm ALength, char *A, char *B)
{
    b32 Result = false;
    
    if(B)
    {
        char *At = B;
        for(umm Index = 0;
            Index < ALength;
            ++Index, ++At)
        {
            if((*At == 0) ||
               (A[Index] != *At))
            {
                return(false);
            }
        }
        
        Result = (*At == 0);
    }
    else
    {
        Result = (ALength == 0);
    }
    
    return(Result);
}

internal b32
StringsAreEqual(memory_index ALength, char *A, memory_index BLength, char *B)
{
    b32 Result = (ALength == BLength);
    
    if(Result)
    {
        Result = true;
        for(u32 Index = 0;
            Index < ALength;
            ++Index)
        {
            if(A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }
    
    return(Result);
}

internal b32x
StringsAreEqual(string A, char *B)
{
    b32x Result = StringsAreEqual(A.Count, (char *)A.Data, B);
    return(Result);
}

internal b32x
StringsAreEqual(string A, string B)
{
    b32x Result = StringsAreEqual(A.Count, (char *)A.Data, B.Count, (char *)B.Data);
    return(Result);
}

internal b32
StringsAreEqualLowercase(memory_index ALength, char *A, memory_index BLength, char *B)
{
    b32 Result = (ALength == BLength);
    
    if(Result)
    {
        Result = true;
        for(u32 Index = 0;
            Index < ALength;
            ++Index)
        {
            if(ToLowercase(A[Index]) != ToLowercase(B[Index]))
            {
                Result = false;
                break;
            }
        }
    }
    
    return(Result);
}

internal b32x
StringsAreEqualLowercase(string A, string B)
{
    b32x Result = StringsAreEqualLowercase(A.Count, (char *)A.Data, B.Count, (char *)B.Data);
    return(Result);
}

internal s32
S32FromZInternal(char **AtInit)
{
    s32 Result = 0;
    
    char *At = *AtInit;
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }
    
    *AtInit = At;
    
    return(Result);
}

internal s32
S32FromZ(char *At)
{
    char *Ignored = At;
    s32 Result = S32FromZInternal(&Ignored);
    return(Result);
}

struct format_dest
{
    umm Size;
    char *At;
};

internal void
OutChar(format_dest *Dest, char Value)
{
    if(Dest->Size)
    {
        --Dest->Size;
        *Dest->At++ = Value;
    }
}

internal void
OutChars(format_dest *Dest, char *Value)
{
    // NOTE(casey): Not particularly speedy, are we?  :P
    while(*Value)
    {
        OutChar(Dest, *Value++);
    }
}

#define ReadVarArgUnsignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, u64) : (u64)va_arg(ArgList, u32)
#define ReadVarArgSignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, s64) : (s64)va_arg(ArgList, s32)
#define ReadVarArgFloat(Length, ArgList) va_arg(ArgList, f64)

char DecChars[] = "0123456789";
char LowerHexChars[] = "0123456789abcdef";
char UpperHexChars[] = "0123456789ABCDEF";
internal void
U64ToASCII(format_dest *Dest, u64 Value, u32 Base, char *Digits)
{
    Assert(Base != 0);
    
    char *Start = Dest->At;
    do
    {
        u64 DigitIndex = (Value % Base);
        char Digit = Digits[DigitIndex];
        OutChar(Dest, Digit);
        
        Value /= Base;
    } while(Value != 0);
    char *End = Dest->At;
    
    while(Start < End)
    {
        --End;
        char Temp = *End;
        *End = *Start;
        *Start = Temp;
        ++Start;
    }
}

internal void
F64ToASCII(format_dest *Dest, f64 Value, u32 Precision)
{
    if(Value < 0)
    {
        OutChar(Dest, '-');
        Value = -Value;
    }
    
    u64 IntegerPart = (u64)Value;
    Value -= (f64)IntegerPart;
    U64ToASCII(Dest, IntegerPart, 10, DecChars);
    
    OutChar(Dest, '.');
    
    // TODO(casey): Note that this is NOT an accurate way to do this!
    for(u32 PrecisionIndex = 0;
        PrecisionIndex < Precision;
        ++PrecisionIndex)
    {
        Value *= 10.0f;
        u32 Integer = (u32)Value;
        Value -= (f32)Integer;
        OutChar(Dest, DecChars[Integer]);
    }
}

// NOTE(casey): Size returned __DOES NOT__ include the null terminator.
internal umm
FormatStringList(umm DestSize, char *DestInit, char *Format, va_list ArgList)
{
    format_dest Dest = {DestSize, DestInit};
    if(Dest.Size)
    {
        char *At = Format;
        while(At[0])
        {
            if(*At == '%')
            {
                ++At;
                
                b32 ForceSign = false;
                b32 PadWithZeros = false;
                b32 LeftJustify = false;
                b32 PostiveSignIsBlank = false;
                b32 AnnotateIfNotZero = false;
                
                b32 Parsing = true;
                
                //
                // NOTE(casey): Handle the flags
                //
                Parsing = true;
                while(Parsing)
                {
                    switch(*At)
                    {
                        case '+': {ForceSign = true;} break;
                        case '0': {PadWithZeros = true;} break;
                        case '-': {LeftJustify = true;} break;
                        case ' ': {PostiveSignIsBlank = true;} break;
                        case '#': {AnnotateIfNotZero = true;} break;
                        default: {Parsing = false;} break;
                    }
                    
                    if(Parsing)
                    {
                        ++At;
                    }
                }
                
                //
                // NOTE(casey): Handle the width
                //
                b32 WidthSpecified = false;
                s32 Width = 0;
                if(*At == '*')
                {
                    Width = va_arg(ArgList, int);
                    WidthSpecified = true;
                    ++At;
                }
                else if((*At >= '0') && (*At <= '9'))
                {
                    Width = S32FromZInternal(&At);
                    WidthSpecified = true;
                }
                
                //
                // NOTE(casey): Handle the precision
                //
                b32 PrecisionSpecified = false;
                s32 Precision = 0;
                if(*At == '.')
                {
                    ++At;
                    
                    if(*At == '*')
                    {
                        Precision = va_arg(ArgList, int);
                        PrecisionSpecified = true;
                        ++At;
                    }
                    else if((*At >= '0') && (*At <= '9'))
                    {
                        Precision = S32FromZInternal(&At);
                        PrecisionSpecified = true;
                    }
                    else
                    {
                        Assert(!"Malformed precision specifier!");
                    }
                }
                
                // TODO(casey): Right now our routine doesn't allow non-specified
                // precisions, so we just set non-specified precisions to a specified value
                if(!PrecisionSpecified)
                {
                    Precision = 6;
                }
                
                //
                // NOTE(casey): Handle the length
                //
                u32 IntegerLength = 4;
                u32 FloatLength = 8;
                // TODO(casey): Actually set different values here!
                if((At[0] == 'h') && (At[1] == 'h'))
                {
                    At += 2;
                }
                else if((At[0] == 'l') && (At[1] == 'l'))
                {
                    At += 2;
                }
                else if(*At == 'h')
                {
                    ++At;
                }
                else if(*At == 'l')
                {
                    IntegerLength = 8;
                    ++At;
                }
                else if(*At == 'j')
                {
                    ++At;
                }
                else if(*At == 'z')
                {
                    ++At;
                }
                else if(*At == 't')
                {
                    ++At;
                }
                else if(*At == 'L')
                {
                    ++At;
                }
                
                char TempBuffer[64];
                char *Temp = TempBuffer;
                format_dest TempDest = {ArrayCount(TempBuffer), Temp};
                char *Prefix = "";
                b32 IsFloat = false;
                
                switch(*At)
                {
                    case 'd':
                    case 'i':
                    {
                        s64 Value = ReadVarArgSignedInteger(IntegerLength, ArgList);
                        b32 WasNegative = (Value < 0);
                        if(WasNegative)
                        {
                            Value = -Value;
                        }
                        U64ToASCII(&TempDest, (u64)Value, 10, DecChars);
                        
                        // TODO(casey): Make this a common routine once floating
                        // point is available.
                        if(WasNegative)
                        {
                            Prefix = "-";
                        }
                        else if(ForceSign)
                        {
                            Assert(!PostiveSignIsBlank); // NOTE(casey): Not a problem here, but probably shouldn't be specified?
                            Prefix = "+";
                        }
                        else if(PostiveSignIsBlank)
                        {
                            Prefix = " ";
                        }
                    } break;
                    
                    case 'u':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 10, DecChars);
                    } break;
                    
                    case 'm':
                    {
                        // TODO(casey): Put in a fractional thing here...
                        umm Value = va_arg(ArgList, umm);
                        char *Suffix = "b ";
                        if(Value >= Gigabytes(1))
                        {
                            Suffix = "gb";
                            Value = (Value + Gigabytes(1) - 1) / Gigabytes(1);
                        }
                        else if(Value >= Megabytes(1))
                        {
                            Suffix = "mb";
                            Value = (Value + Megabytes(1) - 1) / Megabytes(1);
                        }
                        else if(Value >= Kilobytes(1))
                        {
                            Suffix = "kb";
                            Value = (Value + Kilobytes(1) - 1) / Kilobytes(1);
                        }
                        U64ToASCII(&TempDest, Value, 10, DecChars);
                        OutChars(&TempDest, Suffix);
                    } break;
                    
                    case 'o':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 8, DecChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0";
                        }
                    } break;
                    
                    case 'x':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, LowerHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0x";
                        }
                    } break;
                    
                    case 'X':
                    {
                        u64 Value = ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, UpperHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0X";
                        }
                    } break;
                    
                    // TODO(casey): Support other kinds of floating point prints
                    // (right now we only do basic decimal output)
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                    {
                        f64 Value = ReadVarArgFloat(FloatLength, ArgList);
                        F64ToASCII(&TempDest, Value, Precision);
                        IsFloat = true;
                    } break;
                    
                    case 'c':
                    {
                        int Value = va_arg(ArgList, int);
                        OutChar(&TempDest, (char)Value);
                    } break;
                    
                    case 's':
                    {
                        char *String = va_arg(ArgList, char *);
                        
                        // TODO(casey): Obey precision, width, etc.
                        
                        Temp = String;
                        if(PrecisionSpecified)
                        {
                            TempDest.Size = 0;
                            for(char *Scan = String;
                                *Scan && (TempDest.Size < Precision);
                                ++Scan)
                            {
                                ++TempDest.Size;
                            }
                        }
                        else
                        {
                            TempDest.Size = StringLength(String);
                        }
                        TempDest.At = String + TempDest.Size;
                    } break;
                    
                    case 'S':
                    {
                        string String = va_arg(ArgList, string);
                        
                        // TODO(casey): Obey precision, width, etc.
                        
                        Temp = (char *)String.Data;
                        TempDest.Size = String.Count;
                        if(PrecisionSpecified && (TempDest.Size > Precision))
                        {
                            TempDest.Size = Precision;
                        }
                        TempDest.At = Temp + TempDest.Size;
                    } break;
                    
                    case 'p':
                    {
                        void *Value = va_arg(ArgList, void *);
                        U64ToASCII(&TempDest, *(umm *)&Value, 16, LowerHexChars);
                    } break;
                    
                    case 'n':
                    {
                        int *TabDest = va_arg(ArgList, int *);
                        *TabDest = (int)(Dest.At - DestInit);
                    } break;
                    
                    case '%':
                    {
                        OutChar(&Dest, '%');
                    } break;
                    
                    default:
                    {
                        Assert(!"Unrecognized format specifier");
                    } break;
                }
                
                if(TempDest.At - Temp)
                {
                    smm UsePrecision = Precision;
                    if(IsFloat || !PrecisionSpecified)
                    {
                        UsePrecision = (TempDest.At - Temp);
                    }
                    
                    smm PrefixLength = StringLength(Prefix);
                    smm UseWidth = Width;
                    smm ComputedWidth = UsePrecision + PrefixLength;
                    if(UseWidth < ComputedWidth)
                    {
                        UseWidth = ComputedWidth;
                    }
                    
                    if(PadWithZeros)
                    {
                        Assert(!LeftJustify); // NOTE(casey): Not a problem, but no way to do it?
                        LeftJustify = false;
                    }
                    
                    if(!LeftJustify)
                    {
                        while(UseWidth > (UsePrecision + PrefixLength))
                        {
                            OutChar(&Dest, PadWithZeros ? '0' : ' ');
                            --UseWidth;
                        }
                    }
                    
                    for(char *Pre = Prefix;
                        *Pre && UseWidth;
                        ++Pre)
                    {
                        OutChar(&Dest, *Pre);
                        --UseWidth;
                    }
                    
                    if(UsePrecision > UseWidth)
                    {
                        UsePrecision = UseWidth;
                    }
                    while(UsePrecision > (TempDest.At - Temp))
                    {
                        OutChar(&Dest, '0');
                        --UsePrecision;
                        --UseWidth;
                    }
                    while(UsePrecision && (TempDest.At != Temp))
                    {
                        OutChar(&Dest, *Temp++);
                        --UsePrecision;
                        --UseWidth;
                    }
                    
                    if(LeftJustify)
                    {
                        while(UseWidth)
                        {
                            OutChar(&Dest, ' ');
                            --UseWidth;
                        }
                    }
                }
                
                if(*At)
                {
                    ++At;
                }
            }
            else
            {
                OutChar(&Dest, *At++);
            }
        }
        
        if(Dest.Size)
        {
            Dest.At[0] = 0;
        }
        else
        {
            Dest.At[-1] = 0;
        }
    }
    
    umm Result = Dest.At - DestInit;
    return(Result);
}

// TODO(casey): Eventually, make this return a string struct
internal umm
FormatString(umm DestSize, char *Dest, char *Format, ...)
{
    va_list ArgList;
    
    va_start(ArgList, Format);
    umm Result = FormatStringList(DestSize, Dest, Format, ArgList);
    va_end(ArgList);
    
    return(Result);
}

internal u64
MurmurHashUpdate(u64 h, u64 k)
{
    // NOTE(casey): This is based on the 128-bit MurmurHash from MurmurHash3
    
    u64 c1 = 0x87c37b91114253d5ULL;
    u64 c2 = 0x4cf5ad432745937fULL;
    u64 m1 = 5;
    u64 n1 = 0x52dce729ULL;
    
    k *= c1;
    k = RotateLeft(k, 31);
    k *= c2;
    
    h ^= k;
    
    h = RotateLeft(h, 27);
    h = h*m1+n1;
    
    return(h);
}

internal u64
MurmurHashFinalize(u64 h)
{
    // NOTE(casey): This is based on the 128-bit MurmurHash from MurmurHash3
    
    h ^= h >> 33ULL;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33ULL;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33ULL;
    
    return(h);
}

internal u64
CheckSumOf(buffer Buffer, u64 Seed = 1234)
{
    // TODO(casey): We would have to special-case the MurmurHash on big-endian machines
    // to make sure we matched, but I HATE BIG ENDIAN MACHINES AND THEY SHOULD
    // ALL GO AWAY DIE DIE DIE so we are not going to do that.
    
    u64 Result = Seed;
    
    u64 Count64 = (Buffer.Count / sizeof(u64));
    u64 Count8 = Buffer.Count - (Count64 * sizeof(u64));
    
    // TODO(casey): This may be unaligned... if we find speed problems on
    // an align-required platform, we may need to move to an aligned location
    // first.
    u64 *At = (u64 *)Buffer.Data;
    for(u64 Index = 0;
        Index < Count64;
        ++Index)
    {
        Result = MurmurHashUpdate(Result, *At++);
    }
    
    if(Count8)
    {
        u64 Residual = 0;
        Copy(Count8, At, &Residual);
        Result = MurmurHashUpdate(Result, Residual);
    }
    
    Result = MurmurHashFinalize(Result);
    
    return(Result);
}

global v3 DebugColorTable[] =
{
    /* 00 */ {1, 0, 0},
    /* 01 */ {0, 1, 0},
    /* 02 */ {0, 0, 1},
    /* 03 */ {1, 1, 0},
    /* 04 */ {0, 1, 1},
    /* 05 */ {1, 0, 1},
    /* 06 */ {1, 0.5f, 0},
    /* 07 */ {1, 0, 0.5f},
    /* 08 */ {0.5f, 1, 0},
    /* 09 */ {0, 1, 0.5f},
    /* 10 */ {0.5f, 0, 1},
    /* 11 */ {1, 0.75f, 0.5f},
    /* 12 */ {1, 0.5f, 0.75f},
    /* 13 */ {0.75f, 1, 0.5f},
    /* 14 */ {0.5f, 1, 0.75f},
    /* 15 */ {0.5f, 0.75f, 1},
    
    /* 16 */ {1, 0.25f, 0.25f},
    /* 17 */ {0.25f, 1, 0.25f},
    /* 18 */ {0.25f, 0.25f, 1},
    /* 19 */ {1, 1, 0.25f},
    /* 20 */ {0.25f, 1, 1},
    /* 21 */ {1, 0.25f, 1},
    /* 22 */ {1, 0.5f, 0.25f},
    /* 23 */ {1, 0.25f, 0.5f},
    /* 24 */ {0.5f, 1, 0.25f},
    /* 25 */ {0.25f, 1, 0.5f},
    /* 26 */ {0.5f, 0.25f, 1},
    /* 27 */ {1, 0.25f, 0.5f},
    /* 28 */ {1, 0.5f, 0.25f},
    /* 29 */ {0.25f, 1, 0.5f},
    /* 30 */ {0.5f, 1, 0.25f},
    /* 31 */ {0.5f, 0.25f, 1},
};

internal v3
GetDebugColor3(u32 Value)
{
    v3 Result = DebugColorTable[Value % ArrayCount(DebugColorTable)];
    return(Result);
}

internal v4
GetDebugColor4(u32 Value, f32 Alpha = 1.0f)
{
    v4 Result = V4(GetDebugColor3(Value), Alpha);
    return(Result);
}
