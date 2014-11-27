#include "Common.h"
#include "StrategyManager.h"
#include "InformationManager.h"
#include "..\..\SparCraft\source\SparCraft.h"
#include "base\WorkerData.h"
#include <direct.h>
#include <ShlObj.h>
#include <iostream>
#include <cstring>

// constructor
StrategyManager::StrategyManager() 
	: firstAttackSent(false)
	, currentStrategy(0)
	, selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
{
	addStrategies();
	setStrategy();
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

void StrategyManager::addStrategies() 
{
	protossOpeningBook = std::vector<std::string>(NumProtossStrategies);
	terranOpeningBook  = std::vector<std::string>(NumTerranStrategies);
	zergOpeningBook    = std::vector<std::string>(NumZergStrategies);

	//protossOpeningBook[ProtossZealotRush]	= "0 0 0 0 1 0 0 3 0 0 3 0 1 3 0 4 4 4 4 4 1 0 4 4 4";
    protossOpeningBook[ProtossZealotRush]	= "0 0 0 0 1 0 3 3 0 0 4 1 4 4 0 4 4 0 1 4 3 0 1 0 4 0 4 4 4 4 1 0 4 4 4";
	//protossOpeningBook[ProtossZealotRush]	= "0";
	//protossOpeningBook[ProtossDarkTemplar] = "0 0 0 0 1 3 0 7 5 0 0 12 3 13 0 22 22 22 22 0 1 0";
    protossOpeningBook[ProtossDarkTemplar]	= "0 0 0 0 1 0 3 0 7 0 5 0 12 0 13 3 22 22 1 22 22 0 1 0";
	protossOpeningBook[ProtossDragoons]		= "0 0 0 0 1 0 0 3 0 7 0 0 5 0 0 3 8 6 1 6 6 0 3 1 0 6 6 6";

	zergOpeningBook[ZergZerglingRush]		= "0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 1 6";

	//Conner's opening build. builds marines between major buildings so we have some small defense
	//opening builds an academy, an engineering bay, a factory, marine shell upgrade, and missile turret to detect cloaked units
	//terranOpeningBook[TerranDefault]		= "0 0 0 0 0 1 0 0 3 0 0 3 0 1 0 4 0 0 5 5 5 5 5 6 5 5 5 5 5 20 5 5 5 5 5 9 17 5 5 5 5 5 8 8 21";
	terranOpeningBook[TerranDefault]		= "0 0 0 0 0 1 0 0 3 0 0 3 0 1 0 4 0 0 0 6";
	//This is WIP
	terranOpeningBook[TerranRampCamp] = "0 0 0 0 0 3 1 0 5 5 0 5 5 1";
	//Terran BBS http://wiki.teamliquid.net/starcraft2/BBS
	terranOpeningBook[TerranBBS] = "0 0 0 3 0 25 5 5 5 5";
	terranOpeningBook[TerranAntiFourPool] = "0 0 0 3 0 25 5 5 5 5";

	if (selfRace == BWAPI::Races::Terran)
	{
		results = std::vector<IntPair>(NumTerranStrategies);

		if (enemyRace == BWAPI::Races::Protoss)
		{
			usableStrategies.push_back(TerranDefault);
			usableStrategies.push_back(TerranRampCamp);
			usableStrategies.push_back(TerranBBS);
		}
		else if (enemyRace == BWAPI::Races::Terran)
		{
			usableStrategies.push_back(TerranDefault);
			usableStrategies.push_back(TerranBBS);
			usableStrategies.push_back(TerranAntiFourPool);
		}
		else if (enemyRace == BWAPI::Races::Zerg)
		{
			usableStrategies.push_back(TerranDefault);
			usableStrategies.push_back(TerranBBS);
			usableStrategies.push_back(TerranAntiFourPool);
		}
		else
		{
			BWAPI::Broodwar->printf("Enemy Race Unknown");
			usableStrategies.push_back(TerranDefault);
		}
	}
	else if (selfRace == BWAPI::Races::Protoss)
	{
		results = std::vector<IntPair>(NumProtossStrategies);

		if (enemyRace == BWAPI::Races::Protoss)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDarkTemplar);
			usableStrategies.push_back(ProtossDragoons);
		}
		else if (enemyRace == BWAPI::Races::Terran)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDarkTemplar);
			usableStrategies.push_back(ProtossDragoons);
		}
		else if (enemyRace == BWAPI::Races::Zerg)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDragoons);
		}
		else
		{
			BWAPI::Broodwar->printf("Enemy Race Unknown");
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDragoons);
		}
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		results = std::vector<IntPair>(NumZergStrategies);
		usableStrategies.push_back(ZergZerglingRush);
	}

	if (Options::Modules::USING_STRATEGY_IO)
	{
		readResults();
	}
}

void StrategyManager::readResults()
{
	/*FILE *stream;
	std::string filestr = "bwapi-data/AI/cattlepedigree/";
	std::string botname = BWAPI::Broodwar->enemy()->getName();
	filestr.append(botname);
	filestr.append(".cbdat");
	const char * file = filestr.c_str();
	struct stat buf;
	std::ifstream f_in(file);
	std::string line;
	getline(f_in, line);
	results[TerranDefault].first = atoi(line.c_str());
	getline(f_in, line);
	results[TerranDefault].second = atoi(line.c_str());
	getline(f_in, line);
	results[TerranBBS].first = atoi(line.c_str());
	getline(f_in, line);
	results[TerranBBS].second = atoi(line.c_str());
	getline(f_in, line);
	results[TerranAntiFourPool].first = atoi(line.c_str());
	getline(f_in, line);
	results[TerranAntiFourPool].second = atoi(line.c_str());
	getline(f_in, line);
	results[TerranRampCamp].first = atoi(line.c_str());
	getline(f_in, line);
	results[TerranRampCamp].second = atoi(line.c_str());
	f_in.close();*/

	// read in the name of the read and write directories from settings file
	struct stat buf;

	// if the file doesn't exist something is wrong so just set them to default settings
	if (stat(Options::FileIO::FILE_SETTINGS, &buf) == -1)
	{
		readDir = "bwapi-data/read/";
		writeDir = "bwapi-data/write/";
	}
	else
	{
		std::ifstream f_in(Options::FileIO::FILE_SETTINGS);
		getline(f_in, readDir);
		getline(f_in, writeDir);
		f_in.close();
	}

	// the file corresponding to the enemy's previous results
	std::string readFile = readDir + BWAPI::Broodwar->enemy()->getName() + ".txt";

	// if the file doesn't exist, set the results to zeros
	if (stat(readFile.c_str(), &buf) == -1)
	{
		std::fill(results.begin(), results.end(), IntPair(0,0));
	}
	// otherwise read in the results
	else
	{
		std::ifstream f_in(readFile.c_str());
		std::string line;
		getline(f_in, line);
		results[TerranDefault].first = atoi(line.c_str());
		getline(f_in, line);
		results[TerranDefault].second = atoi(line.c_str());
		getline(f_in, line);
		results[TerranBBS].first = atoi(line.c_str());
		getline(f_in, line);
		results[TerranBBS].second = atoi(line.c_str());
		getline(f_in, line);
		results[TerranAntiFourPool].first = atoi(line.c_str());
		getline(f_in, line);
		results[TerranAntiFourPool].second = atoi(line.c_str());
		getline(f_in, line);
		results[TerranRampCamp].first = atoi(line.c_str());
		getline(f_in, line);
		results[TerranRampCamp].second = atoi(line.c_str());
		f_in.close();
	}

	BWAPI::Broodwar->printf("Results (%s): (%d %d) (%d %d) (%d %d) (%d %d)", BWAPI::Broodwar->enemy()->getName().c_str(), 
		results[0].first, results[0].second, results[1].first, results[1].second, results[2].first, results[2].second, results[3].first, results[3].second);
}

void StrategyManager::writeResults()
{
	std::string writeFile = writeDir + BWAPI::Broodwar->enemy()->getName() + ".txt";
	std::ofstream f_out(writeFile.c_str());

	f_out << results[TerranDefault].first   << "\n";
	f_out << results[TerranDefault].second  << "\n";
	f_out << results[TerranBBS].first  << "\n";
	f_out << results[TerranBBS].second << "\n";
	f_out << results[TerranAntiFourPool].first     << "\n";
	f_out << results[TerranAntiFourPool].second    << "\n";
	f_out << results[TerranRampCamp].first     << "\n";
	f_out << results[TerranRampCamp].second    << "\n";

	f_out.close();
}

void StrategyManager::setStrategy()
{
	// if we are using file io to determine strategy, do so
	if (Options::Modules::USING_STRATEGY_IO)
	{
		double bestUCB = -1;
		int bestStrategyIndex = 0;

		// UCB requires us to try everything once before using the formula
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			int sum = results[usableStrategies[strategyIndex]].first + results[usableStrategies[strategyIndex]].second;

			if (sum == 0)
			{
				currentStrategy = usableStrategies[strategyIndex];
				if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
					switch(currentStrategy) {
					case TerranBBS: BWAPI::Broodwar->printf("Current Strategy: 1 TerranBBS"); return;
					case TerranRampCamp: 
						useCamping = true;
						BWAPI::Broodwar->printf("Current Strategy: 2 TerranRampCamp");
						return;
					case TerranAntiFourPool: BWAPI::Broodwar->printf("Current Strategy: 3 TerranAntiFourPool"); return;
					case TerranDefault: BWAPI::Broodwar->printf("Current Strategy: 0 TerranDefault"); return;
					default: BWAPI::Broodwar->printf("Current Strategy: Unknown"); return;
					}
				}
				return;
			}
		}

		// if we have tried everything once, set the maximizing ucb value
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			double ucb = getUCBValue(usableStrategies[strategyIndex]);

			if (ucb > bestUCB)
			{
				bestUCB = ucb;
				bestStrategyIndex = strategyIndex;
			}
		}
		
		currentStrategy = usableStrategies[bestStrategyIndex];

	}
	else
	{
		if (selfRace == BWAPI::Races::Protoss) {
			std::string enemyName(BWAPI::Broodwar->enemy()->getName());
        
			if (enemyName.compare("Skynet") == 0)
			{
				currentStrategy = ProtossDarkTemplar;
			}
			else
			{
				currentStrategy = ProtossZealotRush;
			}
		} else if (selfRace == BWAPI::Races::Terran){

			if (enemyRace == BWAPI::Races::Protoss)
			{
				//currentStrategy = TerranDefault;
				//currentStrategy = TerranAntiFourPool;
				currentStrategy = TerranBBS;
				//currentStrategy = TerranRampCamp;
			}
			else if (enemyRace == BWAPI::Races::Terran)
			{
				currentStrategy = TerranBBS;
			}
			else if (enemyRace == BWAPI::Races::Zerg)
			{
				currentStrategy = TerranBBS;
			}
			else
			{
				BWAPI::Broodwar->printf("Enemy Race Unknown");
				currentStrategy = TerranDefault;
			}
		}
	}

	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
		switch(currentStrategy) {
		case TerranBBS: BWAPI::Broodwar->printf("Current Strategy: 1 TerranBBS"); return;
		case TerranRampCamp: 
			useCamping = true;
			BWAPI::Broodwar->printf("Current Strategy: 2 TerranRampCamp");
			return;
		case TerranAntiFourPool: BWAPI::Broodwar->printf("Current Strategy: 3 TerranAntiFourPool"); return;
		case TerranDefault: BWAPI::Broodwar->printf("Current Strategy: 0 TerranDefault"); return;
		default: BWAPI::Broodwar->printf("Current Strategy: Unknown"); return;
		}
	}
}

void StrategyManager::onEnd(const bool isWinner)
{
	// write the win/loss data to file if we're using IO
	if (Options::Modules::USING_STRATEGY_IO)
	{
		// if the game ended before the tournament time limit
		if (BWAPI::Broodwar->getFrameCount() < Options::Tournament::GAME_END_FRAME)
		{
			if (isWinner)
			{
				results[getCurrentStrategy()].first = results[getCurrentStrategy()].first + 1;
			}
			else
			{
				results[getCurrentStrategy()].second = results[getCurrentStrategy()].second + 1;
			}
		}
		// otherwise game timed out so use in-game score
		else
		{
			if (getScore(BWAPI::Broodwar->self()) > getScore(BWAPI::Broodwar->enemy()))
			{
				results[getCurrentStrategy()].first = results[getCurrentStrategy()].first + 1;
			}
			else
			{
				results[getCurrentStrategy()].second = results[getCurrentStrategy()].second + 1;
			}
		}
		
		writeResults();
	}
}

const double StrategyManager::getUCBValue(const size_t & strategy) const
{
	double totalTrials(0);
	for (size_t s(0); s<usableStrategies.size(); ++s)
	{
		totalTrials += results[usableStrategies[s]].first + results[usableStrategies[s]].second;
	}

	double C		= 0.7;
	double wins		= results[strategy].first;
	double trials	= results[strategy].first + results[strategy].second;

	double ucb = (wins / trials) + C * sqrt(std::log(totalTrials) / trials);

	return ucb;
}

const int StrategyManager::getScore(BWAPI::Player * player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const std::string StrategyManager::getOpeningBook() const
{
	if (selfRace == BWAPI::Races::Protoss)
	{
		return protossOpeningBook[currentStrategy];
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		return terranOpeningBook[currentStrategy];
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		return zergOpeningBook[currentStrategy];
	} 

	// something wrong, return the protoss one
	return protossOpeningBook[currentStrategy];
}

// when do we want to defend with our workers?
// this function can only be called if we have no fighters to defend with
const int StrategyManager::defendWithWorkers()
{
	if (!Options::Micro::WORKER_DEFENSE)
	{
		return false;
	}

	// our home nexus position
	BWAPI::Position homePosition = BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition();;

	// enemy units near our workers
	int enemyUnitsNearWorkers = 0;

	// defense radius of nexus
	int defenseRadius = 300;

	// fill the set with the types of units we're concerned about
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if it's a zergling or a worker we want to defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			if (unit->getDistance(homePosition) < defenseRadius)
			{
				enemyUnitsNearWorkers++;
			}
		}
	}

	// if there are enemy units near our workers, we want to defend
	return enemyUnitsNearWorkers;
}

const int StrategyManager::getNumberUnitsNeededForAttack() const
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran) {
		switch(currentStrategy) {
		case TerranDefault:		return 1;
		case TerranBBS:			return 1;
		case TerranRampCamp:	return 1;
		case TerranAntiFourPool:return 10;
		default: return 1;
		}
	}
	else {
		return 1;
	}
}

// called by combat commander to determine whether or not to send an attack force
// freeUnits are the units available to do this attack
const bool StrategyManager::doAttack(const std::set<BWAPI::Unit *> & freeUnits)
{
	int ourForceSize = (int)freeUnits.size();

	int numUnitsNeededForAttack = getNumberUnitsNeededForAttack();

	bool doAttack  = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) >= 1
					|| ourForceSize >= numUnitsNeededForAttack;

	if (doAttack)
	{
		firstAttackSent = true;
	}

	return doAttack || firstAttackSent;
}

const bool StrategyManager::expandProtossZealotRush() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numZealots =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int frame =					BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

	// 2nd Nexus Conditions:
	//		We have 12 or more zealots
	//		It is past frame 7000
	if ((numNexus < 2) && (numZealots > 12 || frame > 9000))
	{
		return true;
	}

	// 3nd Nexus Conditions:
	//		We have 24 or more zealots
	//		It is past frame 12000
	if ((numNexus < 3) && (numZealots > 24 || frame > 15000))
	{
		return true;
	}

	if ((numNexus < 4) && (numZealots > 24 || frame > 21000))
	{
		return true;
	}

	if ((numNexus < 5) && (numZealots > 24 || frame > 26000))
	{
		return true;
	}

	if ((numNexus < 6) && (numZealots > 24 || frame > 30000))
	{
		return true;
	}

	return false;
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		switch (getCurrentStrategy()){
		// Insert strategies here for protoss strats
		case ProtossZealotRush:		return getProtossZealotRushBuildOrderGoal();
		case ProtossDarkTemplar:	return getProtossDarkTemplarBuildOrderGoal();
		case ProtossDragoons:		return getProtossDragoonsBuildOrderGoal();
		// Fallback for is something goes wrong
		default: return getProtossZealotRushBuildOrderGoal();
		}
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
	{
		switch (getCurrentStrategy()) {
		
		// Insert strategies here for terran strats
		case TerranBBS:				return getTerranBBSBuildOrderGoal();
		case TerranDefault:			return getTerranDefaultBuildOrderGoal();
		case TerranAntiFourPool:	return getTerranAntiFourPoolBuildOrderGoal();
		case TerranRampCamp:		return getTerranRampCampBuildOrderGoal();
		// Fallback for is something goes wrong
		default: return getTerranDefaultBuildOrderGoal();
		}
	}
	else
	{
		return getZergBuildOrderGoal();
	}
}

const MetaPairVector StrategyManager::getProtossDragoonsBuildOrderGoal() const
{
		// the goal to return
	MetaPairVector goal;

	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int dragoonsWanted = numDragoons > 0 ? numDragoons + 6 : 2;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}
	else
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}

		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || numDragoons > 6 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 2));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getProtossDarkTemplarBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDarkTeplar =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int darkTemplarWanted = 0;
	int dragoonsWanted = numDragoons + 6;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numDragoons > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, darkTemplarWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	
	return goal;
}

const MetaPairVector StrategyManager::getProtossZealotRushBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int zealotsWanted = numZealots + 8;
	int dragoonsWanted = numDragoons;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 4;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}

	if (numCyber > 0)
	{
		dragoonsWanted = numDragoons + 2;
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot,	zealotsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getTerranDefaultBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numFirebats = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Firebat);
	int numGhosts = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Ghost);
	int numVulture =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Vulture);
	int numWraith =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Wraith);
	int numTank = 				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	int numFactory = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Factory);
	int numMachineShop = 		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Machine_Shop);
	int numStarport = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Starport);
	int numScience = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Science_Facility);
	int numCovert = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Covert_Ops);
	int numTurret = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Missile_Turret);
	//int armor =					BWAPI::Broodwar->self()->allUnitCount(BWAPI::UpgradeTypes::Terran_Infantry_Armor);
	int numEngine =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay);

	int marinesWanted = numMarines + 12;
	int medicsWanted = numMedics + 2;
	int firebatsWanted = numFirebats + 12;
	//int vulturesWanted = numVulture + 4;
	int tanksWanted = numTank + 12;
	int wraithsWanted = numWraith + 1;
	int ghostsWanted = numGhosts + 12;
	
	//the following sequence of builds allows us to build ghosts and research personnel cloaking
	if(numFactory > 0)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Starport, 1));
		//goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Machine_Shop, 1));
	}
	if(numStarport > 0)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Science_Facility, 1));
	}
	if(numScience > 0 )
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Covert_Ops, 1));
	}
	if(numTurret < 2)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Missile_Turret, 1));
	}
	if(numCovert > 0)
	{
		goal.push_back(MetaPair(BWAPI::TechTypes::Personnel_Cloaking, 1));
	}
	if(numEngine > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 1));
	}
	
	
	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));
	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));
	//if we have a science lab with covert ops build ghosts
	if(numCovert > 0)
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Ghost,		ghostsWanted));
	}
	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const MetaPairVector StrategyManager::getTerranBBSBuildOrderGoal() const
{	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);	
	int numVultures = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Vulture);
	int hasBarracks = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Barracks);
	int hasFactory = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Factory);
	int hasRefinery = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Refinery);
	int numExtra = BWAPI::Broodwar->self()->minerals() / 100;

	//Marines --> Vultures
	int marinesWanted = numMarines + 4 + numExtra;
	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, marinesWanted));

	if( hasBarracks > 0 && hasRefinery <= 0) 
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Refinery, 1));
	}
	if( numMarines > 4 && hasFactory <= 0 && hasRefinery > 0)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Factory, 1));
	}
	if (hasFactory > 0) { goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Vulture, 1)); }

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const MetaPairVector StrategyManager::getTerranRampCampBuildOrderGoal() const
{

	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numEngine =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay);
	/*int numMedics =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numFirebats = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Firebat);
	int numGhosts = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Ghost);
	int numVulture =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Vulture);
	int numWraith =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Wraith);
	int numTank = 				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	int numFactory = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Factory);
	int numMachineShop = 		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Machine_Shop);
	int numStarport = 			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Starport);
	//int shellUpgrade =		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UpgradeTypes::U_238_Shells);*/

	//We want constant pumping out of Marines with this strategy.
	int marinesWanted = numMarines + 1;
	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));

	//We attempt to get the U-38 Shell Upgrade with this if-statement.
	if (numEngine == 0 && numMarines > 10) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Engineering_Bay, 1));
	}
	else if( numMarines > 10)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::U_238_Shells, 1));
	}
	
	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const MetaPairVector StrategyManager::getTerranAntiFourPoolBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);	
	int numMedics = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	//int numBunkers = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Bunker);
	int hasBarracks = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Barracks);
	int hasAcademy = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Academy);
	int hasRefinery = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Refinery);
	bool hasStims = BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Stim_Packs);
	int numExtra = BWAPI::Broodwar->self()->minerals() / 100;

	//We want constant pumping out of Marines with this strategy.
	int marinesWanted = numMarines + 4 + numExtra;
	int medicsWanted = numMedics + 1;
	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, marinesWanted));

	/** SEGMENT NOT WORKING
	if (numBunkers > 0 && numBunkers < 3 && hasBarracks) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Bunker, 1));
	}
	**/
	if( numMarines > 9)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Academy, 1));
	}
	if( hasAcademy && !hasRefinery) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Refinery, 1));
	}
	if (hasRefinery && hasStims == false) {
		goal.push_back(MetaPair(BWAPI::TechTypes::Stim_Packs, 1));
	}
	if (hasStims == true && numMarines > 10) {
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic, medicsWanted));
	}

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}


const bool StrategyManager::expandTerran() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numCommandCenter =		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Command_Center);
	int numMarines =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int frame =					BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}
	
	if ((numCommandCenter < 2) && (numMarines > 12 && frame > 9000))
	{
		return true;
	}
	
	if ((numCommandCenter < 3) && (numMarines > 24 && frame > 15000))
	{
		return true;
	}

	if ((numCommandCenter < 4) && (numMarines > 24 && frame > 24000))
	{
		return true;
	}

	if ((numCommandCenter < 5) && (numMarines > 24 && frame > 30000))
	{
		return true;
	}

	if ((numCommandCenter < 6) && (numMarines > 24 && frame > 35000))
	{
		return true;
	}

	return false;
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;
	
	int numMutas  =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
	int numHydras  =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);

	int mutasWanted = numMutas + 6;
	int hydrasWanted = numHydras + 6;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, 4));
	//goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Stim_Packs,	1));

	//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const int StrategyManager::getCurrentStrategy()
 {
	 return currentStrategy;
 }

const bool StrategyManager::isCampingActive() {
	return useCamping;
}

void StrategyManager::isCampingActive(bool state) {
	useCamping = state;
}
