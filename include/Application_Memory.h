/* 
 * File:   CSkript_Memory.h
 * Author: johannes
 *
 * Created on 20 December 2015, 13:57
 * 
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 */

#ifndef CSKRIPT_MEMORY_H
#define	CSKRIPT_MEMORY_H

#include "Platform.h"

struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;

    int32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline b32
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

inline b32
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

//
//
//

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize(Count*sizeof((Pointer)[0]), Pointer)

inline void
ZeroSize(memory_index Size, void *Ptr)
{
    // 
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
    memory_index AlignmentOffset = 0;
    
    memory_index ResultPointer = (memory_index)Arena->Base + Arena->Used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return(AlignmentOffset);
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = 0x1,
};

struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params
DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline arena_push_params
AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
Align(u32 Alignment, b32 Clear)
{
    arena_push_params Params = DefaultArenaParams();
    if(Clear)
    {
        Params.Flags |= ArenaFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~ArenaFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return(Params);
}

inline memory_index
GetArenaSizeRemaining(memory_arena *Arena, arena_push_params Params = DefaultArenaParams())
{
    memory_index Result = Arena->Size - (Arena->Used + GetAlignmentOffset(Arena, Params.Alignment));

    return(Result);
}


#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ## __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ## __VA_ARGS__))

inline memory_index
GetEffectiveSizeFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = SizeInit;
        
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;

    return(Size);
}

inline b32
ArenaHasRoomFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    b32 Result = ((Arena->Used + Size) <= Arena->Size);
    return(Result);
}

inline void *
PushSize_(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    std::cerr<<"PushSize_ START\n";
    memory_index Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    
    Assert((Arena->Used + Size) <= Arena->Size);
    std::cerr<<"    Arena->Used: "<<Arena->Used/1024/1024<<" MB\n";
    std::cerr<<"    Requested Size: "<<Size/1024/1024<<" MB\n";
    std::cerr<<"    Arena->Used+Size: "<<(Arena->Used + Size)/1024/1024<<" MB\n";
    std::cerr<<"    Arena->Size: "<<Arena->Size/1024/1024<<" MB\n";

    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;

    Assert(Size >= SizeInit);

    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }
    
    std::cerr<<"PushSize_ END\n";
    return(Result);
}

// NOTE(jojo): This is generally not for production use, this is probably
// only really something we need during testing, but who knows
inline char *
PushString(memory_arena *Arena, char *Source)
{
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }
    
    char *Dest = (char *)PushSize_(Arena, Size, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }

    return(Dest);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
Clear(memory_arena *Arena)
{
    InitializeArena(Arena, Arena->Size, Arena->Base);
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size, arena_push_params Params = DefaultArenaParams())
{
    Result->Size = Size;
    Result->Base = (uint8 *)PushSize_(Arena, Size, Params);
    Result->Used = 0;
    Result->TempCount = 0;
}

inline void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}

    return(DestInit);
}

#endif	/* CSKRIPT_MEMORY_H */

