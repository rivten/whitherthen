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
//#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup, platform_file_info *Info, u32 ModeFlags)
//typedef PLATFORM_OPEN_FILE(platform_open_file);
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(char* FileName, platform_file_info *Info)
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
internal PLATFORM_FILE_ERROR(SokolFileError)
{
    Handle->NoErrors = false;
}

internal PLATFORM_OPEN_FILE(SokolOpenFile)
{
    platform_file_handle Result = {};
    FILE* fp = fopen(FileName, "rb");
    Result.NoErrors = (fp != 0);
    Result.Platform = fp;

    // NOTE(hugo): Here I diverge from HandmadeHero's implementation.
    // They get the FileInfo from previous call.
    // Here, the FileInfo is part of the result.
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        Info->FileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }

    return(Result);
}

internal PLATFORM_CLOSE_FILE(SokolCloseFile)
{
    FILE* SokolHandle = (FILE *)Handle->Platform;
    if(SokolHandle != 0)
    {
        fclose(SokolHandle);
    }
}

internal PLATFORM_READ_DATA_FROM_FILE(SokolReadDataFromFile)
{
    if(PlatformNoFileErrors(Handle))
    {
        FILE* SokolHandle = (FILE *)Handle->Platform;
        uint32 FileSize32 = SafeTruncateToU32(Size);
        size_t BytesRead = fread(Dest, 1, Size, SokolHandle);
        if(BytesRead == FileSize32)
        {
        }
        else
        {
            SokolFileError(Handle, "Read file failed.");
        }
    }
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
