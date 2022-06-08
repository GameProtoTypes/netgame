#pragma once


#define SQRT_CELLS_PER_MAP_SECTOR  64
#define SQRT_SECTORS_PER_MAP  16
#define MAX_PEEPS 1024*64

struct Cell;
struct MapSector;
struct Peep {
	int valid;

	int map_xi;
	int map_yi;
	int xv;
	int yv;

	float redblue;

	struct MapSector* sector;
	struct Cell* cell;

} typedef Peep;


struct Cell {
	int localx;
	int localy;
	Peep* Peep;
} typedef Cell;

struct MapSector {
	Cell cells[SQRT_CELLS_PER_MAP_SECTOR][SQRT_CELLS_PER_MAP_SECTOR];
	int sectorX;
	int sectorY;
	struct MapSector* upSector;
	struct MapSector* downSector;
	struct MapSector* leftSector;
	struct MapSector* rightSector;
} typedef MapSector;

struct GameState {
	float screenWidth;
	float screenHeight;
	float mousex;
	float mousey;
	int clicked;

	int frameIdx;

	MapSector sectors[SQRT_SECTORS_PER_MAP][SQRT_SECTORS_PER_MAP];
	Peep peeps[MAX_PEEPS];
}typedef GameState;




void PrintPeep(Peep* p)
{
	printf("Peep (%d) at %d,%d\n", (unsigned int)(void*)p, p->map_xi, p->map_yi);
}




