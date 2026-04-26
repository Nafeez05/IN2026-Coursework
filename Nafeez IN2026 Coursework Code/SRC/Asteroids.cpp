#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include <fstream>
#include <algorithm>

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	//Create the GUI
	CreateGUI();
	//CreateMenuGUI();
	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	glutFullScreen();
	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (mState == GameState::MENU) {
		if (key == '1') {
			//prevents you from playing game without being on main menu, similar concept used for rest of Menu
			destoryMenu();
		}

		if (key == '2') {
			mState = GameState::GMENU;
			mBackLabel->SetVisible(true);
			setGuideVis();
			setMenuInvis();
			return;//temp: REMOVE PLEASE
		}

		if (key == '3') {
			mState = GameState::LBMENU;
			mBackLabel->SetVisible(true);
			setMenuInvis();
			mLeaderboardLabel->SetVisible(true);
			mSelectedIndex = 0;
			mScrollOffset = 0;
			showLeaderboard();
			return;//temp: REMOVE PLEASE
		}

		if (key == '4') {
			mState = GameState::PUMENU;
			mBackLabel->SetVisible(true);
			setMenuInvis();
			setPowerVis();
			return;//temp: REMOVE PLEASE
		}

		if (key == '5') {
			exit(0);
		}

		return;
	}

	if (mState == GameState::GMENU) {
		if (key == '0') {
			mState = GameState::MENU;
			setGuideInvis();
			setMenuVis();
			mBackLabel->SetVisible(false);
			return;
		}
	}

	if (mState == GameState::LBMENU) {
		if (key == '0') {
			mState = GameState::MENU;
			mLeaderboardLabel->SetVisible(false);
			setMenuVis();
			mBackLabel->SetVisible(false);
			for (int i = 0; i < 10; i++) {
				mLeaderboardRows[i]->SetVisible(false);
			}
			return;
		}
	}

	if (mState == GameState::PUMENU) {
		if (key == '0') {
			mState = GameState::MENU;
			setMenuVis();
			setPowerInvis();
			mBackLabel->SetVisible(false);
			return;
		}
	}

	if (mState == GameState::PLAYING) {
		switch (key)
		{
		case ' ':
			mSpaceship->Shoot();
			break;
		default:
			break;
		}
	}

	if (mState == GameState::GAME_OVER) {
		if (key == 13) {
			saveScore(mPlayerNameInput, mCurrentScore);
			mNameInputLabel->SetVisible(false);
			return;
		}

		if (key == 8) {
			if (!mPlayerNameInput.empty()) {
				mPlayerNameInput.pop_back();
			}
		}
		else if (isalnum(key)) {
			mPlayerNameInput += key;
		}

		mNameInputLabel->SetText("Enter Name: " + mPlayerNameInput);	
		return;
	}

	if (key == '5') {
		exit(0);
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (mState == GameState::PLAYING) {
		switch (key)
		{
			// If up arrow key is pressed start applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If left arrow key is pressed start rotating anti-clockwise
		case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
			// If right arrow key is pressed start rotating clockwise
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
			// Default case - do nothing
		default: break;
		}
	}

	if (mState == GameState::LBMENU) {
		if (key == GLUT_KEY_DOWN) {
			if (mSelectedIndex < mLeaderBoard.size() - 1) {
				mSelectedIndex++;
			}

			if (mSelectedIndex >= mScrollOffset + 10) {
				mScrollOffset++;
			}
			updateLeaderboard();
		}

		if (key == GLUT_KEY_UP) {
			if (mSelectedIndex > 0) {
				mSelectedIndex--;
			}

			if (mSelectedIndex < mScrollOffset) {
				mScrollOffset--;
			}
			updateLeaderboard();
		}
	return;
	}
	return;
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (mState != GameState::PLAYING) {
		return;
	}

	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
		mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
		mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
		// Set the visibility of the label to false (hidden)
		mGameOverLabel->SetVisible(true);
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mLivesLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.1f, 0.5f));

	//Makes menu options
	auto container = mGameDisplay->GetContainer();
	mBackLabel = make_shared <GUILabel>("<< Press 0 to go Back");
	mBackLabel->SetVisible(false);
	mTitleLabel = make_shared <GUILabel>("Asteroids");
	mStartLabel = make_shared <GUILabel>("Press 1 to Start Playing");
	mGuideLabel = make_shared <GUILabel>("Press 2 to check Game Guide");
	mLBLabel = make_shared <GUILabel>("Press 3 to view Leader Boards");
	mPULabel = make_shared <GUILabel>("Press 4 to gain Power-Ups");
	mQuitLabel = make_shared <GUILabel>("Press 5 to Quit");

	mLeaderboardLabel = make_shared <GUILabel>("Leader Board:");
	mLeaderboardLabel->SetVisible(false);

	for (int i = 0; i < 10; i++) {
		auto row = make_shared<GUILabel>("");
		row->SetVisible(false);
		container->AddComponent(static_pointer_cast<GUIComponent>(row), GLVector2f(0.035f, 0.8f - (i*0.05f)));
		mLeaderboardRows.push_back(row);
	}

	//This is for the power-up Menu
	mPowerTitle = make_shared <GUILabel>("Please pick a Power-up!");
	mPowerTxt1 = make_shared <GUILabel>("1) Extra +1 Life");
	mPowerTxt2 = make_shared <GUILabel>("2) Temporary Invulnerability");
	mPowerTxt3 = make_shared <GUILabel>("3) Singular Teleportation");
	setPowerInvis();

	//This is for the How to Play Segment
	mGuideTxt1 = make_shared <GUILabel>("How to Play the Asteroids Game...");
	mGuideTxt2 = make_shared <GUILabel>("You can Press the UP arrow to activate yoor thrusters, moving your spaceship forwards");
	mGuideTxt3 = make_shared <GUILabel>("and can angle the bow of your ship using the RIGHT & LEFT arrows. The SPACE Key, when ");
	mGuideTxt4 = make_shared <GUILabel>("pressed can fire a bullet from the bow of your ship capable of destoying an asteroids.");
	mGuideTxt5 = make_shared <GUILabel>("Power-Ups...");
	mGuideTxt6 = make_shared <GUILabel>("Extra Life = Does what it says on the tin: gives you a +1 to your livese to score points");
	mGuideTxt7 = make_shared <GUILabel>("Invulnerability = gives you 10 seconds of invulnerability to collision with Asteroids");
	mGuideTxt8 = make_shared <GUILabel>("Teleportation = warp to any location hovered over by your mouse.");
	setGuideInvis();

	//This is for the game_over stuff:
	mNameInputLabel = make_shared<GUILabel>("Enter Name: ");
	mNameInputLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mNameInputLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mNameInputLabel->SetVisible(false);

	container->AddComponent(static_pointer_cast<GUIComponent>(mPowerTitle), GLVector2f(0.02f, 0.9f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mPowerTxt1), GLVector2f(0.02f, 0.85f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mPowerTxt2), GLVector2f(0.02f, 0.8f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mPowerTxt3), GLVector2f(0.02f, 0.75f));

	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt1), GLVector2f(0.02f, 0.9f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt2), GLVector2f(0.02f, 0.85f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt3), GLVector2f(0.02f, 0.8f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt4), GLVector2f(0.02f, 0.75f));

	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt5), GLVector2f(0.02f, 0.55f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt6), GLVector2f(0.02f, 0.5f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt7), GLVector2f(0.02f, 0.45f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideTxt8), GLVector2f(0.02f, 0.4f));

	container->AddComponent(static_pointer_cast<GUIComponent>(mLeaderboardLabel), GLVector2f(0.035f, 0.85f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mNameInputLabel), GLVector2f(0.5f, 0.6f));

	container->AddComponent(static_pointer_cast<GUIComponent>(mBackLabel), GLVector2f(0.0001f, 0.99999999f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.9f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mStartLabel), GLVector2f(0.5f, 0.7f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mGuideLabel), GLVector2f(0.5f, 0.6f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mLBLabel), GLVector2f(0.5f, 0.5f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mPULabel), GLVector2f(0.5f, 0.4f));
	container->AddComponent(static_pointer_cast<GUIComponent>(mQuitLabel), GLVector2f(0.5f, 0.3f));


}

void Asteroids::loadLeaderboard() {
	mLeaderBoard.clear();
	std::ifstream file("leaderboard.txt");
	std::string name;
	int score;

	while (file >> name >> score) {
		mLeaderBoard.push_back({ name, score });
	}

	std::sort(mLeaderBoard.begin(), mLeaderBoard.end(), [](const ScoreEntry& a, const ScoreEntry& b)
		{
			return a.score > b.score;
		});
}

void Asteroids::updateLeaderboard() {
	for (int i = 0; i < 10; i++) {
		int dataIndex = mScrollOffset + i;

		if (dataIndex < mLeaderBoard.size()) {
			std::string prefix = (dataIndex == mSelectedIndex) ? "> " : " ";

			std::string text = prefix + mLeaderBoard[dataIndex].name + " - " + std::to_string(mLeaderBoard[dataIndex].score);

			mLeaderboardRows[i]->SetText(text);
			mLeaderboardRows[i]->SetVisible(true);
		}
		else {
			mLeaderboardRows[i]->SetVisible(false);
		}
	}
}

void Asteroids::saveScore(const std::string& name, int score) {
	std::ofstream file("leaderboard.txt", std::ios::app);
	file << name << " " << score << std::endl;
}

void Asteroids::showLeaderboard() {
	loadLeaderboard();
	updateLeaderboard();

	for (int i = 0; i < 10; i++) {
		mLeaderboardRows[i]->SetVisible(true);
	}
}

void Asteroids::setPowerInvis() {
	mPowerTitle->SetVisible(false);
	mPowerTxt1->SetVisible(false);
	mPowerTxt2->SetVisible(false);
	mPowerTxt3->SetVisible(false);
}

void Asteroids::setPowerVis() {
	mPowerTitle->SetVisible(true);
	mPowerTxt1->SetVisible(true);
	mPowerTxt2->SetVisible(true);
	mPowerTxt3->SetVisible(true);
}

void Asteroids::setGuideInvis() {
	mGuideTxt1->SetVisible(false);
	mGuideTxt2->SetVisible(false);
	mGuideTxt3->SetVisible(false);
	mGuideTxt4->SetVisible(false);
	mGuideTxt5->SetVisible(false);
	mGuideTxt6->SetVisible(false);
	mGuideTxt7->SetVisible(false);
	mGuideTxt8->SetVisible(false);
}

void Asteroids::setGuideVis() {
	mGuideTxt1->SetVisible(true);
	mGuideTxt2->SetVisible(true);
	mGuideTxt3->SetVisible(true);
	mGuideTxt4->SetVisible(true);
	mGuideTxt5->SetVisible(true);
	mGuideTxt6->SetVisible(true);
	mGuideTxt7->SetVisible(true);
	mGuideTxt8->SetVisible(true);
}

void Asteroids::setMenuInvis() {
	mTitleLabel->SetVisible(false);
	mStartLabel->SetVisible(false);
	mQuitLabel->SetVisible(false);
	mGuideLabel->SetVisible(false);
	mPULabel->SetVisible(false);
	mLBLabel->SetVisible(false);
}

void Asteroids::setMenuVis() {
	mGuideLabel->SetVisible(true);
	mTitleLabel->SetVisible(true);
	mStartLabel->SetVisible(true);
	mLBLabel->SetVisible(true);
	mPULabel->SetVisible(true);
	mQuitLabel->SetVisible(true);
}


void Asteroids::destoryMenu() {
	setMenuInvis();
	mState = GameState::PLAYING;

	mTitleLabel->SetVisible(false);
	mStartLabel->SetVisible(false);
	mQuitLabel->SetVisible(false);
	mGuideLabel->SetVisible(false);
	mPULabel->SetVisible(false);
	mLBLabel->SetVisible(false);

	//set up the game stuff like the score and the lives
	mLivesLabel->SetVisible(true);
	mScoreLabel->SetVisible(true);

	// Create a spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship());
	// Create some asteroids and add them to the world
	CreateAsteroids(10);
}

void Asteroids::OnScoreChanged(int score)
{
	mCurrentScore = score;
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else
	{
		mState = GameState::GAME_OVER;
		SetTimer(500, SHOW_GAME_OVER);
		mPlayerNameInput = "";
		mNameInputLabel->SetText("Enter Name: ");
		mNameInputLabel->SetVisible(true);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}





