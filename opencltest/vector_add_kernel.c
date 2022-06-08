
#include "peep.h"
#include "random.h"

int MapX2SectorX(int map_x) {
    return map_x / SQRT_CELLS_PER_MAP_SECTOR;
}
int MapY2SectorY(int map_y) {
    return map_y / SQRT_CELLS_PER_MAP_SECTOR;
}
Cell* Map2Cell(__global const GameState* gameState, int map_x, int map_y)
{
    MapSector* sector = &gameState->sectors[MapX2SectorX(map_x)][MapY2SectorY(map_y)];
    return &sector->cells[map_x% SQRT_CELLS_PER_MAP_SECTOR][map_y%SQRT_CELLS_PER_MAP_SECTOR];
}


void MovePeep(__global const GameState* gameState, Peep* peep, int2 dir)
{
    //check dest
    int newX = peep->map_xi + dir.x;
    int newY = peep->map_yi + dir.y;

    //printf("attempting movingpeep\n");
    
    if (newX < 0 || newX >= SQRT_CELLS_PER_MAP_SECTOR * SQRT_SECTORS_PER_MAP)
    {
        peep->xv = -peep->xv;
        return;
    }
    if (newY < 0 || newY >= SQRT_CELLS_PER_MAP_SECTOR * SQRT_SECTORS_PER_MAP)
    {
        peep->yv = -peep->yv;
        return;
    }

    Cell* newCell = Map2Cell(gameState, newX, newY);
    MapSector* newSector = &gameState->sectors[MapX2SectorX(newX)][MapY2SectorY(newY)];
    if (newCell->Peep != NULL)
    {
        peep->xv = -peep->xv;
        peep->yv = -peep->yv;
        return;
    }
    //MOVE
    peep->cell->Peep = NULL;

    peep->cell = newCell;
    peep->cell->Peep = peep;

    peep->sector = newSector;
    peep->map_xi = newX;
    peep->map_yi = newY;
    //printf("moved peep:%d\n", peep);
   
}

void UpdatePeep(__global const GameState* gameState, Peep* peep)
{
    MovePeep(gameState, peep, (int2)(peep->xv, peep->yv));
}


void SectorUpdate(__global const GameState* gameState, MapSector* sector)
{
    Peep* PeepsToUpdate[SQRT_CELLS_PER_MAP_SECTOR * SQRT_CELLS_PER_MAP_SECTOR];
    int i = 0;
    //printf("sectorx: %d, sectory: %d\n", sector->sectorX, sector->sectorY);
    for (int x = 0; x < SQRT_CELLS_PER_MAP_SECTOR; x++)
    {
        for (int y = 0; y < SQRT_CELLS_PER_MAP_SECTOR; y++)
        {
            //printf("secx: %d, secy: %d\n", x, y);
            Cell* cell = &(sector->cells[x][y]);
            //printf("cell localx:%d, localy:%d\n", cell->localx, cell->localy);
            if (cell->Peep != NULL)
            {
                
                PeepsToUpdate[i] = cell->Peep;
                i++;
            }
        }
    }
    for (int p = 0; p < i; p++) {
        UpdatePeep(gameState, PeepsToUpdate[p]);
    }
}


void GameInit(__global const GameState* gameState)
{
    //setup map
    for (int secX = 0; secX < SQRT_SECTORS_PER_MAP; secX++)
    {
        for (int secY = 0; secY < SQRT_SECTORS_PER_MAP; secY++)
        {
            MapSector* sector = &gameState->sectors[secX][secY];
            sector->sectorX = secX;
            sector->sectorY = secY;
            for (int cellX = 0; cellX < SQRT_CELLS_PER_MAP_SECTOR; cellX++)
            {
                for (int cellY = 0; cellY < SQRT_CELLS_PER_MAP_SECTOR; cellY++)
                {
                    Cell* cell = &(sector->cells[cellX][cellY]);
                    cell->localx = cellX;
                    cell->localy = cellY;
                    cell->Peep = NULL;
                }
            }
        }
    }

    //setup peeps
    for (int i = 0; i < MAX_PEEPS; i++)
    {
        Peep* p = &(gameState->peeps[i]);
        p->xv = RandomRange(i, -4, 4);
        p->yv = RandomRange(i+1, -4, 4);
        p->valid = 1;
        p->map_xi =  RandomRange(i, 0, SQRT_CELLS_PER_MAP_SECTOR*SQRT_SECTORS_PER_MAP);
        p->map_yi =  RandomRange(i+1, 0, SQRT_CELLS_PER_MAP_SECTOR * SQRT_SECTORS_PER_MAP);

        //printf("%d\n", p->map_xi);
        int sectorX = p->map_xi / SQRT_CELLS_PER_MAP_SECTOR;
        int sectorY = p->map_yi / SQRT_CELLS_PER_MAP_SECTOR;

        int cellX = p->map_xi % SQRT_CELLS_PER_MAP_SECTOR;
        int cellY = p->map_yi % SQRT_CELLS_PER_MAP_SECTOR;
        p->sector = &gameState->sectors[sectorX][sectorY];
        p->cell = &(p->sector->cells[cellX][cellY]);
        p->cell->Peep = p;
    }
}






__kernel void game_update(__global const GameState* gameState) {
    // Get the index of the current element to be processed
    int globalRow = get_global_id(0);
    int globalCol = get_global_id(1);

    int localRow = get_local_id(0);
    int localCol = get_local_id(1);

    if (gameState->frameIdx == 0)
    {
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
        if (globalCol == 0 && globalRow == 0)
        {
            GameInit(gameState);
            printf("Game Initialized.\n");
        }
    }
    else
    {

//printf("frameIdx:%d\n", gameState->frameIdx);


        MapSector* sector = &(gameState->sectors[globalRow][globalCol]);
        

        SectorUpdate(gameState, sector);



    }


}