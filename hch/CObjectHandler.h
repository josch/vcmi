#ifndef COBJECTHANDLER_H
#define COBJECTHANDLER_H
#include "../global.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include "CCreatureHandler.h"

using boost::logic::tribool;
class CCPPObjectScript;
class CGObjectInstance;
class CScript;
class CObjectScript;
class CGHeroInstance;
class CTown;
class CHero;
class CBuilding;
class CSpell;
class CGTownInstance;
class CArtifact;
class CGDefInfo;
class CSpecObjInfo;

class DLL_EXPORT CCastleEvent
{
public:
	std::string name, message;
	int wood, mercury, ore, sulfur, crystal, gems, gold; //gain / loss of resources
	unsigned char players; //players for whom this event can be applied
	bool forHuman, forComputer;
	int firstShow; //postpone of first encounter time in days
	int forEvery; //every n days this event will occure

	unsigned char bytes[6]; //build specific buildings (raw format, similar to town's)

	int gen[7]; //additional creatures in i-th level dwelling

	bool operator<(const CCastleEvent &drugie) const
	{
		return firstShow<drugie.firstShow;
	}
};

class DLL_EXPORT CObject //typical object that can be encountered on a map
{
public:
	std::string name; //object's name
};

class DLL_EXPORT CGObjectInstance
{
public:
	int3 pos; //h3m pos
	int ID, subID; //normal ID (this one from OH3 maps ;]) - eg. town=98; hero=34
	si32 id;//number of object in CObjectHandler's vector		
	CGDefInfo * defInfo;
	CCPPObjectScript * state;
	CSpecObjInfo * info;
	unsigned char animPhaseShift;

	int tempOwner; //uzywane dla szybkosci, skrypt ma obowiazek aktualizowac te zmienna
	bool blockVisit;
	
	virtual bool isHero() const;
	int getOwner() const; 
	void setOwner(int ow); 
	int getWidth() const; //returns width of object graphic in tiles
	int getHeight() const; //returns height of object graphic in tiles
	bool visitableAt(int x, int y) const; //returns true if ibject is visitable at location (x, y) form left top tile of image (x, y in tiles)
	bool operator<(const CGObjectInstance & cmp) const;  //screen printing priority comparing
	CGObjectInstance();
	virtual ~CGObjectInstance();
	CGObjectInstance(const CGObjectInstance & right);
	CGObjectInstance& operator=(const CGObjectInstance & right);
};

class  DLL_EXPORT CArmedInstance: public CGObjectInstance
{
public:
	CCreatureSet army; //army
};

class DLL_EXPORT CGHeroInstance : public CArmedInstance
{
public:
	int moveDir; //format:	123
					//		8 4
					//		765
	bool isStanding;
	CHero * type;
	int exp; //experience point
	int level; //current level of hero
	std::string name; //may be custom
	std::string biography; //may be custom
	int portrait; //may be custom
	int mana; // remaining spell points
	std::vector<int> primSkills; //0-attack, 1-defence, 2-spell power, 3-knowledge
	std::vector<std::pair<int,int> > secSkills; //first - ID of skill, second - level of skill (0 - basic, 1 - adv., 2 - expert)
	int movement; //remaining movement points
	int identifier; //from the map file
	bool sex;

	struct DLL_EXPORT Patrol
	{
		Patrol(){patrolling=false;patrolRadious=-1;};
		bool patrolling;
		int patrolRadious;
	} patrol;


	bool inTownGarrison; // if hero is in town garrison 
	CGTownInstance * visitedTown; //set if hero is visiting town or in the town garrison

	std::vector<int> artifacts; //hero's artifacts from bag
	std::map<int,int> artifWorn; //map<position,artifact_id>; positions: 0 - head; 1 - shoulders; 2 - neck; 3 - right hand; 4 - left hand; 5 - torso; 6 - right ring; 7 - left ring; 8 - feet; 9 - misc1; 10 - misc2; 11 - misc3; 12 - misc4; 13 - mach1; 14 - mach2; 15 - mach3; 16 - mach4; 17 - spellbook; 18 - misc5
	std::set<int> spells; //known spells (spell IDs)

	virtual bool isHero() const;
	unsigned int getTileCost(const EterrainType & ttype, const Eroad & rdtype, const Eriver & rvtype) const;
	unsigned int getLowestCreatureSpeed();
	unsigned int getAdditiveMoveBonus() const;
	float getMultiplicativeMoveBonus() const;
	static int3 convertPosition(int3 src, bool toh3m); //toh3m=true: manifest->h3m; toh3m=false: h3m->manifest
	int3 getPosition(bool h3m) const; //h3m=true - returns position of hero object; h3m=false - returns position of hero 'manifestation'
	int getSightDistance() const; //returns sight distance of this hero
	void setPosition(int3 Pos, bool h3m); //as above, but sets position

	bool canWalkOnSea() const;
	int getCurrentLuck() const;
	int getCurrentMorale() const;
	int getSecSkillLevel(const int & ID) const; //-1 - no skill
	const CArtifact * getArt(int pos);
	CGHeroInstance();
	virtual ~CGHeroInstance();
};

class DLL_EXPORT CGTownInstance : public CArmedInstance
{
public:
	CTown * town;
	std::string name; // name of town
	int builded; //how many buildings has been built this turn
	int destroyed; //how many buildings has been destroyed this turn
	const CGHeroInstance * garrisonHero, *visitingHero;
	int identifier; //special identifier from h3m (only > RoE maps)
	int alignment;
	std::set<int> forbiddenBuildings, builtBuildings;
	std::vector<int> possibleSpells, obligatorySpells, availableSpells;

	struct StrInfo
	{
		std::map<int,int> creatures; //level - available amount
	} strInfo;
	std::set<CCastleEvent> events;


	int getSightDistance() const; //returns sight distance
	int fortLevel() const; //0 - none, 1 - fort, 2 - citadel, 3 - castle
	int hallLevel() const; // -1 - none, 0 - village, 1 - town, 2 - city, 3 - capitol
	bool creatureDwelling(const int & level, bool upgraded=false) const;
	int getHordeLevel(const int & HID) const; //HID - 0 or 1; returns creature level or -1 if that horde structure is not present
	int creatureGrowth(const int & level) const;
	bool hasFort() const;
	bool hasCapitol() const;
	int dailyIncome() const;

	CGTownInstance();
	virtual ~CGTownInstance();
};

class DLL_EXPORT CObjectHandler
{
public:
	std::vector<CObject> objects; //vector of objects; i-th object in vector has subnumber i
	std::vector<int> cregens; //type 17. dwelling subid -> creature ID
	void loadObjects();

	std::vector<std::string> creGens; //names of creatures' generators
	std::vector<std::string> advobtxt;
	std::vector<std::string> xtrainfo;
	std::vector<std::string> restypes;
	std::vector<std::pair<std::string,std::string> > mines; //first - name; second - event description
};


#endif //COBJECTHANDLER_H