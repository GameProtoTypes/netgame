#pragma once



#define MAX_PEEPS (1024*128)

#define WORKGROUPSIZE (32)
#define TOTALWORKITEMS MAX_PEEPS

#define SQRT_MAXSECTORS (32)
#define SECTOR_SIZE (64)


struct Cell;
struct MapSector;
struct Peep {
	//int valid;
	int map_x_Q15_16;
	int map_y_Q15_16;

	int xv_Q15_16;
	int yv_Q15_16;

	cl_long netForcex_Q16;
	cl_long netForcey_Q16;

	int target_x;
	int target_y;

	int faction;
	struct MapSector* mapSector;
	int mapSectorListIdx;

	struct Peep* nextSectorPeep;
	struct Peep* prevSectorPeep;
} typedef Peep;

struct MapSector {
	int nextPeepIdx;
	Peep* lastPeep;
	int xidx;
	int yidx;
} typedef MapSector;

struct GameState {
	Peep peeps[MAX_PEEPS];
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	cl_int mapWidth;
	cl_int mapHeight;


	cl_int mousex;
	cl_int mousey;

	cl_int clicked;

	int frameIdx;
}typedef GameState;


void AssignPeepToSector(GameState* gameState, Peep* peep)
{
	cl_int x = ((peep->map_x_Q15_16 >> 16) / (SECTOR_SIZE)) ;
	cl_int y = ((peep->map_y_Q15_16 >> 16) / (SECTOR_SIZE)) ;

	//clamp
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= SQRT_MAXSECTORS)
		x = SQRT_MAXSECTORS - 1;
	if (y >= SQRT_MAXSECTORS)
		y = SQRT_MAXSECTORS - 1;

	MapSector* newSector = &gameState->sectors[x][y];
	if ((peep->mapSector != newSector))
	{
		if (peep->mapSector != NULL)
		{
			//update old sector
			if ((peep->prevSectorPeep != NULL) && (peep->nextSectorPeep == NULL))
				peep->prevSectorPeep->nextSectorPeep = NULL;
			else if ((peep->prevSectorPeep == NULL) && (peep->nextSectorPeep != NULL))
				peep->nextSectorPeep->prevSectorPeep = NULL;
			else if((peep->prevSectorPeep != NULL) && (peep->nextSectorPeep != NULL))
				peep->prevSectorPeep->nextSectorPeep = peep->nextSectorPeep;//remove link

			if (peep->mapSector->lastPeep == peep)
				peep->mapSector->lastPeep = NULL;

		}

		//assign new sector
		peep->mapSector = newSector;

		//put peep in the sector.  extend list
		if(peep->mapSector->lastPeep != NULL)
			peep->mapSector->lastPeep->nextSectorPeep = peep;

		peep->mapSector->lastPeep = peep;
	}
}

void PrintPeep(Peep* p)
{
	printf("Peep (%d) at %d,%d\n", (unsigned int)(void*)p, p->map_x_Q15_16, p->map_y_Q15_16);
}




