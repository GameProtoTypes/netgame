#pragma once




#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <functional>

#include "assert.h"

#include "Game.CoreParam.Defines.h"

import GE.Basic;
import Game;
import GE.ImGui;

using namespace Game;
using namespace GE;



class GameGraphics;
class GameCompute
{
public:


	GameCompute();
	~GameCompute();

	void GameInit();
	void Stage1_Begin();
	void Stage1_End();

	bool stage1_Running = false;


	std::string GameStateString(int tickIdx);
	void SaveGameStateBase();
	void SaveGameStateDiff(std::vector<char>* data,bool deleteFile = true);
	void LoadGameStateFromDiff(int tickidx);
	void LoadGameStateFromDiff(std::string diffFileName, std::string resultGameStateFileName);
	void LoadGameStateFromDiff(std::vector<char>* data,  int id);



	std::shared_ptr<Game::GameState> gameState;
	std::shared_ptr<Game::GameStateActions> gameStateActions;
	GameGraphics* graphics = nullptr;

	//void RunGameKernel(std::function<void(ALL_CORE_PARAMS_TYPES)> kernelFunc);
	void RunGameKernel(std::function<void(ALL_CORE_PARAMS_TYPES)> kernelFunc, int numThreads = Game::GAME_UPDATE_WORKITEMS);




	ge_float peepVBOBuffer[Game::maxPeeps];
	ge_float particleVBOBuffer[Game::maxParticles];
	ge_ubyte mapTile1VBO[Game::mapDim*Game::mapDim];
	ge_ubyte mapTile1AttrVBO[Game::mapDim*Game::mapDim];
	ge_ubyte mapTile1OtherAttrVBO[Game::mapDim*Game::mapDim];
	ge_ubyte mapTile2VBO[Game::mapDim*Game::mapDim];
	ge_ubyte mapTile2AttrVBO[Game::mapDim*Game::mapDim];
	ge_ubyte mapTile2OtherAttrVBO[Game::mapDim*Game::mapDim];
	ge_float guiVBO[Game::maxGuiRects];
	ge_float linesVBO[Game::maxLines];


};

