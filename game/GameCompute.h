#pragma once




#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <functional>

#include "assert.h"

import Game;

class GameGraphics;
class GameCompute
{
public:


	GameCompute();
	~GameCompute();

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
};

