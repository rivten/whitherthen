#ifdef COMPILE_WINDOWS

struct sokol_loaded_code
{
    b32x IsValid;
    char *DLLFullPath;
    u32 TempDLLNumber;
    char* LockFullPath;
    char* TransientDLLName;
    HMODULE DLL;
    FILETIME DLLLastWriteTime;
    u32 FunctionCount;
    char **FunctionNames;
    void **Functions;
};

internal void SokolUnloadCode(sokol_loaded_code *Loaded)
{
    if(Loaded->DLL)
    {
        // TODO(casey): Currently, we never unload libraries, because
        // we may still be pointing to strings that are inside them
        // (despite our best efforts).  Should we just make "never unload"
        // be the policy?
        
        // FreeLibrary(GameCode->GameCodeDLL);
        Loaded->DLL = 0;
    }
    
    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
}

internal void SokolLoadCode(sokol_state* State, sokol_loaded_code *Loaded)
{
    char *SourceDLLName = Loaded->DLLFullPath;
    char *LockFileName = Loaded->LockFullPath;
    
    char TempDLLName[SOKOL_STATE_FILE_NAME_COUNT];
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Loaded->DLLLastWriteTime = SokolGetLastWriteTime(SourceDLLName);
        
        for(u32 AttemptIndex = 0;
            AttemptIndex < 128;
            ++AttemptIndex)
        {
            SokolBuildEXEPathFileName(State, Loaded->TransientDLLName, Loaded->TempDLLNumber,
                                      sizeof(TempDLLName), TempDLLName);
            if(++Loaded->TempDLLNumber >= 1024)
            {
                Loaded->TempDLLNumber = 0;
            }
            
            if(CopyFile(SourceDLLName, TempDLLName, FALSE))
            {
                break;
            }
        }
        
        Loaded->DLL = LoadLibraryA(TempDLLName);
        if(Loaded->DLL)
        {
            Loaded->IsValid = true;
            for(u32 FunctionIndex = 0;
                FunctionIndex < Loaded->FunctionCount;
                ++FunctionIndex)
            {
                void *Function = GetProcAddress(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if(Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    Loaded->IsValid = false;
                }
            }
        }
    }
    
    if(!Loaded->IsValid)
    {
        SokolUnloadCode(Loaded);
    }
}

#elif COMPILE_LINUX

struct sokol_loaded_code
{
    b32x IsValid;
    char *DLLFullPath;
    void *DLL;
    ino_t DLLFileID;
    
    u32 FunctionCount;
    char **FunctionNames;
    void **Functions;
};

internal inline ino_t
LinuxFileId(char *FileName)
{
    struct stat Attr = {};
    if (stat(FileName, &Attr))
    {
        Attr.st_ino = 0;
    }
    
    return Attr.st_ino;
}

internal void* SokolLoadFunction(void *DLLHandle, const char *Name)
{
    void *Symbol = dlsym(DLLHandle, Name);
    if (!Symbol)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
    }
    // TODO(michiel): Check if lib with underscore exists?!
    return Symbol;
}

internal void* SokolLoadLibrary(const char *Name)
{
    void *Handle = NULL;
    
    Handle = dlopen(Name, RTLD_NOW | RTLD_LOCAL);
    if (!Handle)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
    }
    return Handle;
}

internal void SokolUnloadLibrary(void *Handle)
{
    if (Handle != NULL)
    {
        dlclose(Handle);
        Handle = NULL;
    }
}

internal void SokolUnloadCode(sokol_loaded_code *Loaded)
{
    if (Loaded->DLL)
    {
        // TODO(casey): Currently, we never unload libraries, because
        // we may still be pointing to strings that are inside them
        // (despite our best efforts).  Should we just make "never unload"
        // be the policy?
        //LinuxUnloadLibrary(Loaded->Library);
        Loaded->DLL = 0;
    }
    Loaded->DLLFileID = 0;
    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
}

internal void SokolLoadCode(sokol_state* State, sokol_loaded_code *Loaded)
{
   ino_t FileID = LinuxFileId(Loaded->DLLFullPath);
    
    if (Loaded->DLLFileID != FileID)
    {
        // NOTE(michiel): Create temp file, copy the library and load.
        // dlopen uses a caching mechanism based on the library name.
        char TempFileName[SOKOL_STATE_FILE_NAME_COUNT];
        FormatString(SOKOL_STATE_FILE_NAME_COUNT, TempFileName, "%sXXXXXX",
                     Loaded->DLLFullPath);
        s32 FD = mkstemp(TempFileName);
        s32 OrigFile = open(Loaded->DLLFullPath, O_RDONLY);
        
        if ((FD >=0) && (OrigFile >= 0))
        {
            char ReadBuffer[4096];
            ssize_t ReadCount = read(OrigFile, ReadBuffer, sizeof(ReadBuffer));
            
            while (ReadCount > 0)
            {
                char *WriteBuffer = ReadBuffer;
                ssize_t WriteCount;
                do {
                    WriteCount = write(FD, WriteBuffer, ReadCount);
                    
                    if (WriteCount >= 0)
                    {
                        ReadCount -= WriteCount;
                        WriteBuffer += WriteCount;
                    }
                    else if (errno != EINTR)
                    {
                        Assert(!"Could not copy shared library while loading.");
                    }
                } while (ReadCount > 0);
                ReadCount = read(OrigFile, ReadBuffer, sizeof(ReadBuffer));
            }
            if (ReadCount == 0)
            {
                close(OrigFile);
                close(FD);
            }
            else
            {
                Assert(!"Could not copy whole shared library while loading.");
            }
        }
        else
        {
            Assert(!"Could not open shared library for copying.");
        }
        
        //LinuxUnloadLibrary(Loaded->Library);
        Loaded->DLLFileID = FileID;
        Loaded->IsValid = false;
        
        Loaded->DLL = SokolLoadLibrary(TempFileName);
        if (Loaded->DLL)
        {
            Loaded->IsValid = true;
            for (u32 FunctionIndex = 0;
                 FunctionIndex < Loaded->FunctionCount;
                 ++FunctionIndex)
            {
                void *Function = SokolLoadFunction(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if (Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    Loaded->IsValid = false;
                }
            }
        }
    }
    
    if(!Loaded->IsValid)
    {
        SokolUnloadCode(Loaded);
    }
}

#endif

