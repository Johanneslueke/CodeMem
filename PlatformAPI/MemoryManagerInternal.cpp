#include <vector>
#include <map>
#include <string>
#include <iostream>

//TODO(jojo): probably should manage the memory i am using here too
//			  so that i can say i really do everything on my own here XD
std::map<std::string, MemoryArena*> ArenaRegister;
uint64 ArenaCounter = 0;
uint64 ChunkCounter = 0;

void RegisterNewArena(MemoryArena* arena)
{
	std::cerr<<"--- RegisterNewArena\n";
	arena->MemoryChunks;
	arena->ID=ArenaCounter++;
	ArenaRegister.insert( std::pair<std::string,MemoryArena*>(std::to_string(arena->ID),arena));
}

void DeleteArenaFromRegister(uint64 ID)
{
	auto iter = ArenaRegister.find(std::to_string(ID));
	if( iter != ArenaRegister.end())
		ArenaRegister.erase(iter);
}

void AddNewMemoryChunk(MemoryArena* arena, MemoryAdress adress, memory_index size)
{
	std::cerr<<"--- AddNewMemoryChunk\n";
	MemoryChunk chunk(adress,size);
	std::pair<std::string,MemoryChunk*> pa(
		"0",&chunk
		);
	std::cerr<<",,,,,,,";
	arena->MemoryChunks
	.insert(
		pa
		);
}

void RemoveMemoryChunk(MemoryArena* arena, int ID)
{
	auto iter = arena->MemoryChunks.find(std::to_string(ID));
	if(iter != arena->MemoryChunks.end())
		arena->MemoryChunks.erase(iter);
}