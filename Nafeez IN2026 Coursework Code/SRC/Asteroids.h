#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include <vector>

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////
	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	uint mLevel;
	uint mAsteroidCount;

	//scorething
	int mCurrentScore = 0;

	//power-Up Variables
	bool enhancedThrusterEnabled = false;
	bool InvulnerabilitySelected = false;
	int addedLives = 0;
	float mouseX;
	float mouseY;
		
	std::vector<shared_ptr<GUILabel>> mLeaderboardRows;

	int mSelectedIndex = 0;
	int mScrollOffset = 0;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void saveScore(const std::string& name, int score);
	void loadLeaderboard();
	void updateLeaderboard();
	void showLeaderboard();
	void CreateGUI();
	void destoryMenu();
	void setMenuVis();
	void setMenuInvis();
	void setGuideVis();
	void setGuideInvis();
	void setPowerVis();
	void setPowerInvis();
	shared_ptr<GUILabel> mPowerTitle;
	shared_ptr<GUILabel> mPowerTxt1;
	shared_ptr<GUILabel> mPowerTxt2;
	shared_ptr<GUILabel> mPowerTxt3;
	shared_ptr<GUILabel> mLeaderboardLabel;
	shared_ptr<GUILabel> mUPandDOWN;
	shared_ptr<GUILabel> mGuideTxt1;
	shared_ptr<GUILabel> mGuideTxt2;
	shared_ptr<GUILabel> mGuideTxt3;
	shared_ptr<GUILabel> mGuideTxt4;
	shared_ptr<GUILabel> mGuideTxt5;
	shared_ptr<GUILabel> mGuideTxt6;
	shared_ptr<GUILabel> mGuideTxt7;
	shared_ptr<GUILabel> mGuideTxt8;
	shared_ptr<GUILabel> mTitleLabel;
	shared_ptr<GUILabel> mStartLabel;
	shared_ptr<GUILabel> mGuideLabel;
	shared_ptr<GUILabel> mLBLabel;
	shared_ptr<GUILabel> mPULabel;
	shared_ptr<GUILabel> mBackLabel;
	shared_ptr<GUILabel> mQuitLabel;
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();

	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	const static uint END_INVULNERABLE = 3;//timer for Invulenrability

	//State Machine to determine the state of the game.
	enum class GameState {
		MENU,
		PLAYING,
		GMENU,
		LBMENU,
		PUMENU,
		GAME_OVER,//This is the state for entering name for game over screen
	};

	GameState mState = GameState::MENU;

	//Player input name upon death
	std::string mPlayerNameInput;
	shared_ptr<GUILabel> mNameInputLabel;

	struct ScoreEntry {
		std::string name;
		int score;
	};

	std::vector<ScoreEntry> mLeaderBoard;
};

#endif