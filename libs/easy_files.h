typedef struct {
    bool valid;
    size_t fileSize;
    unsigned char *memory;
} FileContents;

size_t GetFileSize(FILE *FileHandle) {
    long Result = fseek(FileHandle, 0, SEEK_END);
    if(Result != 0) {
        assert(!"Seek Error");
    }
    Result = ftell(FileHandle);
    if(fseek(FileHandle, 0, SEEK_SET) != 0) {
        assert(!"Seek Error");
    }
    return (size_t)Result;
}

FileContents platformReadEntireFile(char *FileName, bool nullTerminate) {
    FileContents Result = {};
    assert(FileName);
    FILE* FileHandle = fopen(FileName, "rb+");
    
    if(FileHandle)
    {
        size_t allocSize = Result.fileSize = GetFileSize(FileHandle);
        assert(Result.fileSize > 0);

        if(nullTerminate) { allocSize += 1; }

        Result.memory = (unsigned char *)calloc(allocSize, 1);
        size_t ReturnSize = fread(Result.memory, 1, Result.fileSize, FileHandle);
        if(ReturnSize == Result.fileSize)
        {
            if(nullTerminate) {
                Result.memory[Result.fileSize] = '\0'; // put at the end of the file
                Result.fileSize += 1;
            }
            Result.valid = true;
            //NOTE(Oliver): Successfully read
        } else {
            assert(!"Couldn't read file");
            Result.valid = false;
            free(Result.memory);
        }
        fclose(FileHandle);
    } else {
        Result.valid = false;
        printf("NO FILE\n");
        assert(!"Couldn't open file");
    }
    return Result;
}