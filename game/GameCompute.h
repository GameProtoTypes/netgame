#pragma once




#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

#include "assert.h"

#include "Game.CoreParam.Defines.h"

import GE.Basic;
import Game;
import GE.ImGui;

using namespace Game;
using namespace GE;



class GameGraphics;
class GameNetworking;
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

	void ImGuiProfiler();

	std::shared_ptr<Game::GameState> gameState;
	std::shared_ptr<Game::GameStateActions> gameStateActions;
	GameGraphics* graphics = nullptr;
	GameNetworking* networking = nullptr;

	//void RunGameKernel(std::function<void(ALL_CORE_PARAMS_TYPES)> kernelFunc);
	void RunGameKernel(std::function<void(ALL_CORE_PARAMS_TYPES)> kernelFunc, int funcId, int numThreads = Game::GAME_UPDATE_WORKITEMS);


	static const int numProfileFuncs = 12;
	float profileTimes[numProfileFuncs][Game::GAME_UPDATE_WORKITEMS][500];
	float frameTimes[500] = { 0.0f };
	std::chrono::steady_clock::time_point lastFrameTimePoint;

	ge_float peepVBOBuffer[Game::maxPeeps*6] = { 0.0f };
	ge_float particleVBOBuffer[Game::maxParticles] = { 0.0f };
	ge_ubyte mapTile1VBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_uint mapTile1AttrVBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_uint mapTile1OtherAttrVBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_ubyte mapTile2VBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_uint mapTile2AttrVBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_uint mapTile2OtherAttrVBO[Game::mapDim*Game::mapDim] = { 0 };
	ge_float guiVBO[Game::maxGuiRects*7] = { 0.0f };
	ge_float linesVBO[Game::maxLines*5] = { 0.0f };


};

