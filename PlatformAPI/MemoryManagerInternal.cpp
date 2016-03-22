#include <vector>
#include <map>

//TODO(jojo): probably should manage the memory i am using here too
//			  so that i can say i really do everything on my own here XD
std::map<int, MemoryArena*> ArenaRegister;
uint64 ArenaCounter = 0;

void RegisterNewArena(MemoryArena* arena)
{
	arena->ID=ArenaCounter++;
	ArenaRegister.insert( std::pair<int,MemoryArena*>(arena->ID,arena));
}

void DeleteArenaFromRegister(uint64 ID)
{
	auto iter = ArenaRegister.find(ID);
	if( iter != ArenaRegister.end())
		ArenaRegister.erase(iter);
}
