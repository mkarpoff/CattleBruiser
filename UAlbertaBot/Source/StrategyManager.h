#pragma once

#include "Common.h"
#include "BWTA.h"
#include "base/BuildOrderQueue.h"
#include "InformationManager.h"
#include "base/WorkerManager.h"
#include "base/StarcraftBuildOrderSearchManager.h"
#include <sys/stat.h>
#include <cstdlib>

#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"

typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;

class StrategyManager 
{
	StrategyManager();
	~StrategyManager() {}

	std::vector<std::string>	protossOpeningBook;
	std::vector<std::string>	terranOpeningBook;
	std::vector<std::string>	zergOpeningBook;

	std::string					readDir;
	std::string					writeDir;
	std::vector<IntPair>		results;
	std::vector<int>			usableStrategies;
	int							currentStrategy;

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	bool						firstAttackSent;
	bool						useCamping;

	void	addStrategies();
	void	setStrategy();
	void	readResults();
	void	writeResults();
	const	int					getNumberUnitsNeededForAttack() const;
	const	int					getScore(BWAPI::Player * player) const;
	const	double				getUCBValue(const size_t & strategy) const;
	
	// protoss strategy
	const	bool				expandProtossZealotRush() const;
	const	std::string			getProtossZealotRushOpeningBook() const;
	const	MetaPairVector		getProtossZealotRushBuildOrderGoal() const;

	const	bool				expandProtossDarkTemplar() const;
	const	std::string			getProtossDarkTemplarOpeningBook() const;
	const	MetaPairVector		getProtossDarkTemplarBuildOrderGoal() const;

	const	bool				expandProtossDragoons() const;
	const	std::string			getProtossDragoonsOpeningBook() const;
	const	MetaPairVector		getProtossDragoonsBuildOrderGoal() const;

	const	bool				expandTerran() const;
	const	MetaPairVector		getTerranDefaultBuildOrderGoal() const;
	const   MetaPairVector      getTerranRampCampBuildOrderGoal() const;
	const	MetaPairVector		getTerranBBSBuildOrderGoal() const;
	const	MetaPairVector		getTerranAntiFourPoolBuildOrderGoal() const;
	const	MetaPairVector		getZergBuildOrderGoal() const;

	const	std::string			getProtossOpeningBook() const;
	const	std::string			getTerranOpeningBook() const;
	const	std::string			getZergOpeningBook() const;

public:

	enum { ProtossZealotRush=0, ProtossDarkTemplar=1, ProtossDragoons=2, NumProtossStrategies=3 };
	enum { TerranDefault=0, TerranRampCamp=1, TerranBBS=2, TerranAntiFourPool=3, NumTerranStrategies=4 };
	enum { ZergZerglingRush=0, NumZergStrategies=1 };

	static	StrategyManager &	Instance();

			void				onEnd(const bool isWinner);
	
	const	bool				regroup(int numInRadius);
	const	bool				doAttack(const std::set<BWAPI::Unit *> & freeUnits);
	const	int				    defendWithWorkers();
	const	bool				rushDetected();

	const	int					getCurrentStrategy();
	const	bool				isCampingActive();
			void				isCampingActive(bool state);

	const	MetaPairVector		getBuildOrderGoal();
	const	std::string			getOpeningBook() const;
};
