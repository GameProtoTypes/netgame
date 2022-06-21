#pragma once



#define MAX_PEEPS (1024*64)

#define WORKGROUPSIZE (32)
#define TOTALWORKITEMS MAX_PEEPS

#define SQRT_MAXSECTORS (128)
#define SECTOR_SIZE (32)


struct Cell;
struct MapSector;
struct Peep {

	//State:
	int map_x_Q15_16;
	int map_y_Q15_16;
	int xv_Q15_16;
	int yv_Q15_16;

	int target_x_Q16;
	int target_y_Q16;

	int faction;




	cl_long netForcex_Q16;
	cl_long netForcey_Q16;

	cl_int minDistPeep_Q16;
	struct Peep* minDistPeep;

	
	struct MapSector* mapSector_pending;
	struct MapSector* mapSector;

	struct Peep* nextSectorPeep;
	struct Peep* prevSectorPeep;

	struct Peep* nextSelectionPeep;
	struct Peep* prevSelectionPeep;


	//noncore
	cl_int selectedByClient;





} typedef Peep;

struct MapSector {
	Peep* lastPeep;
	int xidx;
	int yidx;
	cl_uint lock;
} typedef MapSector;

struct GameState {
	Peep peeps[MAX_PEEPS];
	MapSector sectors[SQRT_MAXSECTORS][SQRT_MAXSECTORS];
	cl_int mapWidth;
	cl_int mapHeight;


	cl_int mousex;
	cl_int mousey;
	cl_int mouse_dragBeginx;
	cl_int mouse_dragBeginy;
	cl_int mousescroll;
	cl_int clicked;

	cl_int mousePrimaryDown;
	cl_int mousePrimaryPressed;
	cl_int mousePrimaryReleased;

	cl_int mouseSecondaryDown;
	cl_int mouseSecondaryPressed;
	cl_int mouseSecondaryReleased;

	cl_float viewX;
	cl_float viewY;
	cl_float view_beginX;
	cl_float view_beginY;
	cl_float viewScale;


	cl_int action_DoSelect;
	cl_int params_DoSelect_StartX_Q16;
	cl_int params_DoSelect_StartY_Q16;
	cl_int params_DoSelect_EndX_Q16;
	cl_int params_DoSelect_EndY_Q16;

	cl_int action_CommandToLocation;
	cl_int params_CommandToLocation_X_Q16;
	cl_int params_CommandToLocation_Y_Q16;

	Peep* selectedPeepsLast;

	cl_uint frameIdx;
}typedef GameState;



void AssignPeepToSector_Detach(GameState* gameState, Peep* peep)
{
	cl_int x = ((peep->map_x_Q15_16 >> 16) / (SECTOR_SIZE));
	cl_int y = ((peep->map_y_Q15_16 >> 16) / (SECTOR_SIZE));

	//clamp
	if (x < -SQRT_MAXSECTORS / 2)
		x = -SQRT_MAXSECTORS / 2;
	if (y < -SQRT_MAXSECTORS / 2)
		y = -SQRT_MAXSECTORS / 2;
	if (x >= SQRT_MAXSECTORS / 2)
		x = SQRT_MAXSECTORS / 2 - 1;
	if (y >= SQRT_MAXSECTORS / 2)
		y = SQRT_MAXSECTORS / 2 - 1;

	MapSector* newSector = &gameState->sectors[x + SQRT_MAXSECTORS / 2][y + SQRT_MAXSECTORS / 2];
	if ((peep->mapSector != newSector))
	{
		if (peep->mapSector != NULL)
		{
			//remove peep from old sector
			if ((peep->prevSectorPeep != NULL))
			{
				peep->prevSectorPeep->nextSectorPeep = peep->nextSectorPeep;
			}
			if ((peep->nextSectorPeep != NULL))
			{
				peep->nextSectorPeep->prevSectorPeep = peep->prevSectorPeep;
			}


			if (peep->mapSector->lastPeep == peep) {
				peep->mapSector->lastPeep = peep->prevSectorPeep;
				if (peep->mapSector->lastPeep != NULL)
					peep->mapSector->lastPeep->nextSectorPeep = NULL;
			}

			//completely detach
			peep->nextSectorPeep = NULL;
			peep->prevSectorPeep = NULL;

		}

		//assign new sector for next stage
		peep->mapSector_pending = newSector;
	}
}
void AssignPeepToSector_Insert(GameState* gameState, Peep* peep)
{
	//assign new sector
	if (peep->mapSector != peep->mapSector_pending)
	{
		peep->mapSector = peep->mapSector_pending;

		//put peep in the sector.  extend list
		if (peep->mapSector->lastPeep != NULL)
		{
			peep->mapSector->lastPeep->nextSectorPeep = peep;
			peep->prevSectorPeep = peep->mapSector->lastPeep;
		}
		peep->mapSector->lastPeep = peep;
	}
}


void AssignPeepToSector(GameState* gameState, Peep* peep)
{
	cl_int x = ((peep->map_x_Q15_16 >> 16) / (SECTOR_SIZE)) ;
	cl_int y = ((peep->map_y_Q15_16 >> 16) / (SECTOR_SIZE)) ;

	//clamp
	if (x < -SQRT_MAXSECTORS / 2)
		x = -SQRT_MAXSECTORS / 2;
	if (y < -SQRT_MAXSECTORS / 2)
		y = -SQRT_MAXSECTORS / 2;
	if (x >= SQRT_MAXSECTORS / 2)
		x = SQRT_MAXSECTORS / 2 - 1;
	if (y >= SQRT_MAXSECTORS / 2)
		y = SQRT_MAXSECTORS / 2 - 1;

	MapSector* newSector = &gameState->sectors[x+SQRT_MAXSECTORS/2][y+SQRT_MAXSECTORS/2];
	if ((peep->mapSector != newSector))
	{
		if (peep->mapSector != NULL)
		{
			//remove peep from old sector
			if ((peep->prevSectorPeep != NULL))
			{
				peep->prevSectorPeep->nextSectorPeep = peep->nextSectorPeep;
			}
			if ((peep->nextSectorPeep != NULL))
			{
				peep->nextSectorPeep->prevSectorPeep = peep->prevSectorPeep;
			}


			if (peep->mapSector->lastPeep == peep) {
				peep->mapSector->lastPeep = peep->prevSectorPeep;
				if(peep->mapSector->lastPeep != NULL)
					peep->mapSector->lastPeep->nextSectorPeep = NULL;
			}

			//completely detach
			peep->nextSectorPeep = NULL;
			peep->prevSectorPeep = NULL;

		}

		//assign new sector
		peep->mapSector = newSector;

		//put peep in the sector.  extend list
		if (peep->mapSector->lastPeep != NULL)
		{
			peep->mapSector->lastPeep->nextSectorPeep = peep;
			peep->prevSectorPeep = peep->mapSector->lastPeep;
		}
		peep->mapSector->lastPeep = peep;
	}
}

void PrintPeep(Peep* p)
{
	printf("Peep (%d) at %d,%d\n", (unsigned int)(void*)p, p->map_x_Q15_16, p->map_y_Q15_16);
}




