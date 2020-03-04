#ifdef __cplusplus
extern "C" {
#endif

typedef struct platform_file_handle
{
    b32 NoErrors;
    void *Platform;
} platform_file_handle;

typedef struct platform_file_info
{
    platform_file_info *Next;
    u64 FileDate; // NOTE(casey): This is a 64-bit number that _means_ the date to the platform, but doesn't have to be understood by the app as any particular date.
    u64 FileSize;
    char *BaseName; // NOTE(casey): Doesn't include a path or an extension
    void *Platform;
} platform_file_info;
typedef struct platform_file_group
{
    u32 FileCount;
    platform_file_info *FirstFileInfo;
    void *Platform;
} platform_file_group;

typedef enum platform_file_type
{
    PlatformFileType_Bitmap,

    PlatformFileType_Count,
} platform_file_type;
    
#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);
    
#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);
    
enum platform_open_file_mode_flags
{
    OpenFile_Read = 0x1,
    OpenFile_Write = 0x2,
};
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup, platform_file_info *Info, u32 ModeFlags)
typedef PLATFORM_OPEN_FILE(platform_open_file);
    
#define PLATFORM_GET_FILE_BY_PATH(name) platform_file_info *name(platform_file_group *FileGroup, char *Path, u32 ModeFlags)
typedef PLATFORM_GET_FILE_BY_PATH(platform_get_file_by_path);
    
#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Dest)
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);
    
#define PLATFORM_WRITE_DATA_TO_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Source)
typedef PLATFORM_WRITE_DATA_TO_FILE(platform_write_data_to_file);
    
#define PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(name) b32 name(platform_file_info *Info, u64 Size, void *Source)
typedef PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(platform_atomic_replace_file_contents);
    
#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
typedef PLATFORM_FILE_ERROR(platform_file_error);
    
#define PLATFORM_CLOSE_FILE(name) void name(platform_file_handle *Handle)
typedef PLATFORM_CLOSE_FILE(platform_close_file);
    
#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)
    
#ifdef __cplusplus
}
#endif

#ifdef FILE_IMPLEMENTATION
struct sokol_platform_file_group
{
    memory_arena Memory;
};

#if COMPILE_WINDOWS
internal platform_file_info *
SokolAllocateFileInfo(platform_file_group *FileGroup, WIN32_FILE_ATTRIBUTE_DATA *Data)
{
    sokol_platform_file_group *Win32FileGroup = (sokol_platform_file_group *)FileGroup->Platform;
    
    platform_file_info *Info = PushStruct(&Win32FileGroup->Memory, platform_file_info);
    Info->Next = FileGroup->FirstFileInfo;
    Info->FileDate = (((u64)Data->ftLastWriteTime.dwHighDateTime << (u64)32) | (u64)Data->ftLastWriteTime.dwLowDateTime);
    Info->FileSize = (((u64)Data->nFileSizeHigh << (u64)32) | (u64)Data->nFileSizeLow);
    FileGroup->FirstFileInfo = Info;
    ++FileGroup->FileCount;
    
    return(Info);
}

internal char *
UTF8FromUTF16(memory_arena *Arena, u32 NameSize, wchar_t *Name)
{
    u32 ResultStorage = (u32)(4*NameSize);
    char *Result = (char *)PushSize(Arena, ResultStorage + 1);
    u32 ResultSize = WideCharToMultiByte(CP_UTF8, 0, Name, NameSize,
                                         Result, ResultStorage, 0, 0);
    Result[ResultSize] = 0;
    return(Result);
}

internal wchar_t *
UTF16FromUTF8(memory_arena *Arena, u32 NameSize, char *Name)
{
    u32 ResultStorage = (u32)(2*NameSize);
    wchar_t *Result = (wchar_t *)PushSize(Arena, ResultStorage + 2);
    u32 ResultSize = MultiByteToWideChar(CP_UTF8, 0, Name, NameSize,
                                         Result, ResultStorage);
    Result[ResultSize] = 0;
    return(Result);
}

#endif

internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(SokolGetAllFilesOfTypeBegin)
{
#if COMPILE_WINDOWS
    platform_file_group Result = {};
    
    sokol_platform_file_group *Win32FileGroup = BootstrapPushStruct(sokol_platform_file_group, Memory);
    Result.Platform = Win32FileGroup;
    
    wchar_t *Stem = L"";
    wchar_t *WildCard = L"*.*";
    switch(Type)
    {
        case PlatformFileType_Bitmap:
            {
                Stem = L"..\\data\\";
                WildCard = L"..\\data\\*.png";
            } break;
        InvalidDefaultCase;
    }
    u32 StemSize = 0;
    for(wchar_t *Scan = Stem;
        *Scan;
        ++Scan)
    {
        ++StemSize;
    }
    
    WIN32_FIND_DATAW FindData;
    HANDLE FindHandle = FindFirstFileW(WildCard, &FindData);
    while(FindHandle != INVALID_HANDLE_VALUE)
    {
        platform_file_info *Info = SokolAllocateFileInfo(&Result, (WIN32_FILE_ATTRIBUTE_DATA *)&FindData);
        wchar_t *BaseNameBegin = FindData.cFileName;
        wchar_t *BaseNameEnd = 0;
        wchar_t *Scan = BaseNameBegin;
        while(*Scan)
        {
            if(Scan[0] == L'.')
            {
                BaseNameEnd = Scan;
            }
            
            ++Scan;
        }
        if(!BaseNameEnd)
        {
            BaseNameEnd = Scan;
        }
        
        u32 BaseNameSize = (u32)(BaseNameEnd - BaseNameBegin);
        Info->BaseName = UTF8FromUTF16(&Win32FileGroup->Memory, BaseNameSize, BaseNameBegin);
        
        // NOTE(casey): This will not be technically correct if you use Unicode filenames.
        for(char *Lower = Info->BaseName;
            *Lower;
            ++Lower)
        {
            *Lower = ToLowercase(*Lower);
        }
        
        u32 cFileNameSize = (u32)(((Scan - FindData.cFileName) + 1));
        Info->Platform = PushArray(&Win32FileGroup->Memory, StemSize + cFileNameSize, wchar_t);
        CopyArray(StemSize, Stem, Info->Platform);
        CopyArray(cFileNameSize, FindData.cFileName, (wchar_t *)Info->Platform + StemSize);
        
        if(!FindNextFileW(FindHandle, &FindData))
        {
            break;
        }
    }
    FindClose(FindHandle);
    
    return(Result);
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(SokolGetAllFilesOfTypeEnd)
{
#if COMPILE_WINDOWS
    sokol_platform_file_group *Win32FileGroup = (sokol_platform_file_group *)FileGroup->Platform;
    if(Win32FileGroup)
    {
        Clear(&Win32FileGroup->Memory);
    }
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

internal PLATFORM_GET_FILE_BY_PATH(SokolGetFileByPath)
{
#if COMPILE_WINDOWS
    sokol_platform_file_group *Win32FileGroup = (sokol_platform_file_group *)FileGroup->Platform;
    platform_file_info *Result = 0;
    
    wchar_t *Path16 = UTF16FromUTF8(&Win32FileGroup->Memory, StringLength(Path), Path);
    
    WIN32_FILE_ATTRIBUTE_DATA Data = {};
    if(GetFileAttributesExW(Path16, GetFileExInfoStandard, &Data) ||
       (ModeFlags & OpenFile_Write))
    {
        Result = SokolAllocateFileInfo(FileGroup, &Data);
        // TODO(casey): Should the base name be duplicated??
        Result->BaseName = Path;
        Result->Platform = Path16;
    }
    
    return(Result);
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

internal PLATFORM_FILE_ERROR(SokolFileError)
{
    // TODO(hugo): printing/logging
    Handle->NoErrors = false;
}

internal PLATFORM_OPEN_FILE(SokolOpenFile)
{
#if COMPILE_WINDOWS
    platform_file_handle Result = {};
    Assert(sizeof(HANDLE) <= sizeof(Result.Platform));
    
    DWORD HandlePermissions = 0;
    DWORD HandleCreation = 0;
    if(ModeFlags & OpenFile_Read)
    {
        HandlePermissions |= GENERIC_READ;
        HandleCreation = OPEN_EXISTING;
    }
    
    if(ModeFlags & OpenFile_Write)
    {
        HandlePermissions |= GENERIC_WRITE;
        HandleCreation = OPEN_ALWAYS;
    }
    
    wchar_t *FileName = (wchar_t *)Info->Platform;
    HANDLE Win32Handle = CreateFileW(FileName, HandlePermissions,
                                     FILE_SHARE_READ, 0, HandleCreation, 0, 0);
    Result.NoErrors = (Win32Handle != INVALID_HANDLE_VALUE);
    *(HANDLE *)&Result.Platform = Win32Handle;
    
    return(Result);
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

internal PLATFORM_CLOSE_FILE(SokolCloseFile)
{
#if COMPILE_WINDOWS
    HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
    if(Win32Handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Win32Handle);
    }
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

internal PLATFORM_READ_DATA_FROM_FILE(SokolReadDataFromFile)
{
#if COMPILE_WINDOWS
    if(PlatformNoFileErrors(Handle))
    {
        HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
        
        OVERLAPPED Overlapped = {};
        Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
        Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
        
        uint32 FileSize32 = SafeTruncateToU32(Size);
        
        DWORD BytesRead;
        if(ReadFile(Win32Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
           (FileSize32 == BytesRead))
        {
            // NOTE(casey): File read succeeded!
        }
        else
        {
            SokolFileError(Handle, "Read file failed.");
        }
    }
#elif COMPILE_LINUX
    NotImplemented;
#endif
}

#if COMPILE_WINDOWS
inline FILETIME SokolGetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}
#endif

#endif
