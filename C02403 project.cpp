// C02403 project.cpp: A program using the TL-Engine
#pragma once
#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h> // General console window includes
#include <conio.h>
#include <ctype.h>
#include <memory>
#include "ModelCreation.h"
#include "Collisions.h"
#include "wtypes.h" 
#include "Bullets.h"
#include "Weapons.h"
#include "Targets.h"
#include <time.h>
#include "Weapons.h"
#include "Engine.h"
#include <SFML/Audio.hpp>

using namespace tle;
using namespace std;

//enum fireModes { Single, Burst, Auto };
enum standingState { Standing, Crouching, Prone };
enum menuState {MainMenu, PauseMenu, GameRunning};
I3DEngine* myEngine = New3DEngine(kTLX);


const float upperCamYMax = -50.0f;
const float lowerCamYMax = 50.0f;

const int numGuns = 6;

float Time = 0;
float countDownTime = 1.0f;

float WeaponTime = 0;
bool canShoot = true;

int bulletsFired = 0;


void Fire(IModel* &cameraDummy, float& frametime, float& shoottimer);

void movement(I3DEngine* myEngine, IModel* camDummy, float& currentCamRotation, float& currentCamY, float& camYCounter, standingState& currPlayerStandState, float& movementSpeed, float& currentMoveSpeed);
//void gunSwapAndDrop(I3DEngine* myEngine, float& interactionZspeed, float& currentInteractionDistance, IModel*& interactionDummy, bool& canCollide, Weapon* WeaponArray[], int whichGunEquipped, IModel*& cameraDummy, float& oldPlayerX, float& oldPlayerZ);
void desktopResolution(int& horizontal, int& vertical);

vector <sBullet*> vBullets;
vector <sBullet*> vMagazine;
vector <sTarget*> vTargets;
deque <unique_ptr<sWeapon>> vGuns;
sWeapon* currentGun =nullptr;



int whichGunEquipped = numGuns;
fireModes CurrentFireMode = Auto;


void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it

	int horizontal = 0; int vertical = 0;
	float reloadTimer = 0;
	desktopResolution(horizontal, vertical);
	myEngine->StartFullscreen(horizontal, vertical);
	myEngine->StartMouseCapture();


	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\Media");
	ICamera* myCam = myEngine->CreateCamera(kManual, 0, 0, 0);

	{

		ISprite* Crosshair = myEngine->CreateSprite("crosshair.png", (horizontal / 2) - 6000, (vertical / 2) - 60);
		ISprite* ammoUI = myEngine->CreateSprite("ammoUIPNG.png", -1000, vertical - 150);

		ISprite* fireModeSemi = myEngine->CreateSprite("SemiAutoUI.png", -20, vertical - 105);
		ISprite* fireModeBurstUI = myEngine->CreateSprite("burstFireUI.png", -29, vertical - 105);
		ISprite* fireModeFullUI = myEngine->CreateSprite("FullAutoUI.png", -43, vertical - 105);

		IFont* MainFont = myEngine->LoadFont("Stencil STD", 60);

		IMesh* dummyMesh = myEngine->LoadMesh("Dummy.x");
		IMesh* bulletMesh = myEngine->LoadMesh("Bullet.x");


		IModel* fence[80];
		IModel* cameraDummy = dummyMesh->CreateModel(5, 15, 80);
		IModel* interactionDummy = dummyMesh->CreateModel(0, 0, 0);
		IModel* gunFireTest = dummyMesh->CreateModel(0, 0, 0);

		gunFireTest->AttachToParent(cameraDummy);

		spawnTargets(vTargets);
		spawnGuns(vGuns);
		spawnBullets(500, bulletMesh, vBullets);
		refillNewWeapon(100, vMagazine, vBullets);

		interactionDummy->Scale(7);
		interactionDummy->AttachToParent(myCam);

		myCam->AttachToParent(cameraDummy);
		myCam->SetMovementSpeed(0.0f);
		cameraDummy->RotateY(180);

		float frameTime = myEngine->Timer();
		float movementSpeed = frameTime;
		float shoottimer = 0.04f;
		float currentMoveSpeed = 50.0f;

		float mouseMoveX = 0.0f;
		float mouseMoveY = 0.0f;
		float camYCounter = 0.0f;

		float interactionZspeed = 0.0f;
		float currentInteractionDistance = 0.0f;
		bool canCollide = false;

		float oldPlayerX = 0;
		float oldPlayerZ = 0;

		standingState currPlayerStandState = Standing;
		bool crouched = false;
		bool prone = false;

		stringstream ammoText;

		menuState currentGameState = MainMenu;
		bool spritesInPosition = false;

		/**** Set up your scene here ****/
		CreateFences(myEngine, fence); CreateScene(myEngine); CreateWalls(myEngine);

		// The main game loop, repeat until engine is stopped
		while (myEngine->IsRunning())
		{
			WeaponTime = WeaponTime + frameTime;
			frameTime = myEngine->Timer();
			myEngine->DrawScene();

			if (CurrentFireMode == Auto)
			{
				fireModeBurstUI->SetPosition(29, vertical - 105);
				fireModeFullUI->SetPosition(43, vertical - 105);
			}
			else if (CurrentFireMode == Burst)
			{
				fireModeFullUI->SetPosition(-28, vertical - 105);
			}
			else
			{
				fireModeFullUI->SetPosition(-28, vertical - 105);
				fireModeBurstUI->SetPosition(-28, vertical - 105);
			}

			if (currentGun != nullptr)
			{
				ammoText << currentGun->magAmount << " / " << currentGun->magCapacity;
				MainFont->Draw(ammoText.str(), 100, vertical - 90, kWhite, kCentre);
				MainFont->Draw(currentGun->name, 180, vertical - 115, kWhite, kCentre);
				ammoText.str("");
			}

			oldPlayerX = cameraDummy->GetX();
			oldPlayerZ = cameraDummy->GetZ();

			movementSpeed = currentMoveSpeed * frameTime;

			/**** Update your scene each frame here ****/
			if (currentGameState == MainMenu)
			{
				if (myEngine->KeyHit(Key_R))
				{
					currentGameState = GameRunning;
				}
			}
			else if (currentGameState == GameRunning)
			{
				if (spritesInPosition == false)
				{
					Crosshair->SetPosition((horizontal / 2) - 60, (vertical / 2) - 60);
					ammoUI->SetPosition(10, vertical - 150);
					spritesInPosition = true;
				}

				if (CurrentFireMode == Auto)
				{
					fireModeBurstUI->SetPosition(29, vertical - 105);
					fireModeFullUI->SetPosition(43, vertical - 105);
					fireModeSemi->SetPosition(13, vertical - 105);
				}
				else if (CurrentFireMode == Burst)
				{
					fireModeFullUI->SetPosition(-28, vertical - 105);
					fireModeSemi->SetPosition(13, vertical - 105);
				}
				else
				{
					fireModeSemi->SetPosition(13, vertical - 105);
					fireModeFullUI->SetPosition(-28, vertical - 105);
					fireModeBurstUI->SetPosition(-28, vertical - 105);
				}

				mouseMoveX = myEngine->GetMouseMovementX();
				mouseMoveY = myEngine->GetMouseMovementY();

				if (camYCounter < upperCamYMax && mouseMoveY < 0) { mouseMoveY = 0; }
				if (camYCounter > lowerCamYMax && mouseMoveY > 0) { mouseMoveY = 0; }

				camYCounter += mouseMoveY * 0.1f;

				movement(myEngine, cameraDummy, mouseMoveX, mouseMoveY, camYCounter, currPlayerStandState, movementSpeed, currentMoveSpeed);

				for (auto& Guns : vGuns)
				{
					if (canCollide == true && gunInteraction(interactionDummy, Guns->weaponModel) && currentGun == nullptr)
					{
						Guns->weaponModel->ResetOrientation();
						Guns->weaponModel->AttachToParent(cameraDummy);
						Guns->weaponModel->SetLocalPosition(2.0f, -2.0f, 7.0f);
						Guns->weaponModel->RotateY(-0.2f);
						currentGun = (Guns.get());
						reloadMagazine(currentGun->magCapacity, vMagazine);
						currentGun->magAmount = currentGun->magCapacity;

					}
				}

				if (!FenceCollision(cameraDummy))
				{
					cameraDummy->SetPosition(oldPlayerX, 15, oldPlayerZ);
				}

				if (myEngine->KeyHit(Key_E))
				{
					interactionZspeed = 0.0f;
					currentInteractionDistance = 0.0f;
					interactionDummy->SetLocalPosition(0, 0, 0);
					interactionZspeed = 0.01f;
					canCollide = true;
				}

				if (myEngine->KeyHit(Key_Q) && currentGun != nullptr)
				{
					currentGun->weaponModel->DetachFromParent();
					currentGun->weaponModel->SetPosition(oldPlayerX, 0.2, oldPlayerZ);
					currentGun->weaponModel->RotateLocalZ(90.0f);
					currentGun->weaponModel->RotateY(rand());
					currentGun = nullptr;
				}

				if (currentInteractionDistance >= 2.0f)
				{
					canCollide = false;
					interactionZspeed = 0.0f;
					currentInteractionDistance = 0.0f;
					interactionDummy->SetLocalPosition(0, 0, 0);
				}

				interactionDummy->MoveLocalZ(interactionZspeed);
				currentInteractionDistance += interactionZspeed;


				if (myEngine->KeyHit(Key_X))
				{
					if (CurrentFireMode == Auto)
					{
						CurrentFireMode = Burst;
					}
					else if (CurrentFireMode == Burst)
					{
						CurrentFireMode = Single;
					}
					else if (CurrentFireMode == Single)
					{
						CurrentFireMode = Auto;
					}
				}

				if (myEngine->KeyHeld(Key_R))
				{
					reloadTimer += frameTime;
					if (reloadTimer > 1.2f)
					{
						reloadMagazine(currentGun->magCapacity, vMagazine);
						currentGun->magAmount = currentGun->magCapacity;
						reloadTimer = 0;
					}
				}

				if (myEngine->KeyHit(Key_N))
				{
					for (auto& i : vTargets)
					{
						i->model->SetY(12);
						i->state = Ready;

					}
				}
				if (currentGun != nullptr)
				{
					Fire(cameraDummy, frameTime, shoottimer);
				}

				//if (shoottimer <= 0 &&currentGun->magAmount > 0)
				//{
				// currentGun->shootingsound.play();
				//	shoottimer =currentGun->fireRate * 3;
				//}

				moveBullets(100, vMagazine, frameTime);
				moveTargets(vTargets, frameTime);
				bulletToTarget(vTargets, vMagazine);
				//END
			}
		}
	myEngine->Delete();	// Delete the 3D engine now we are finished with it
		

	}
}

void movement(I3DEngine* myEngine, IModel* camDummy, float& currentCamX, float &mouseMoveY, float& camYCounter, standingState& currPlayerStandState, float& movementSpeed, float& currentMoveSpeed)
{
	if (camYCounter > upperCamYMax && mouseMoveY < 0)
	{
		camDummy->RotateLocalX(mouseMoveY * 0.1f);
	}

	if (camYCounter < lowerCamYMax && mouseMoveY > 0)
	{
		camDummy->RotateLocalX(mouseMoveY * 0.1f);
	}

	camDummy->RotateY(currentCamX * 0.1f);

	if (myEngine->KeyHeld(Key_W))
	{
		camDummy->MoveLocalZ(movementSpeed);
	}

	if (myEngine->KeyHeld(Key_S))
	{
		camDummy->MoveLocalZ(-movementSpeed);
	}

	if (myEngine->KeyHeld(Key_A))
	{
		camDummy->MoveLocalX(-movementSpeed);
	}

	if (myEngine->KeyHeld(Key_D))
	{
		camDummy->MoveLocalX(movementSpeed);
	}

	if (myEngine->KeyHeld(Key_Escape))
	{
		myEngine->Stop();
	}

	if (myEngine->KeyHit(Key_C))
	{
		if (currPlayerStandState == Standing)
		{
			currPlayerStandState = Crouching;
			currentMoveSpeed = 25.0f;
		}
		else if (currPlayerStandState == Crouching)
		{
			currPlayerStandState = Prone;
			currentMoveSpeed = 10.0f;
		}
		else if (currPlayerStandState == Prone)
		{
			currPlayerStandState = Standing;
			currentMoveSpeed = 50.0f;
		}
	}

	if (currPlayerStandState == Crouching)
	{
		camDummy->SetY(9);
	}
	else if (currPlayerStandState == Prone)
	{
		camDummy->SetY(3);
	}
	else if (currPlayerStandState == Standing)
	{
		camDummy->SetY(15);
	}
}

//void gunSwapAndDrop(I3DEngine* myEngine, float& interactionZspeed, float& currentInteractionDistance, IModel*& interactionDummy, bool& canCollide, Weapon* WeaponArray[], int whichGunEquipped, IModel*& cameraDummy, float& oldPlayerX, float& oldPlayerZ)
//{
//	if (myEngine->KeyHit(Key_E))
//	{
//		interactionZspeed = 0.0f;
//		currentInteractionDistance = 0.0f;
//		interactionDummy->SetLocalPosition(0, 0, 0);
//		interactionZspeed = 0.01f;
//		canCollide = true;
//	}
//
//	for (int i = 0; i < numGuns; i++)
//	{
//		if (canCollide == true && gunInteraction(interactionDummy, WeaponArray[i]->weaponModel) && whichGunEquipped == numGuns)
//		{
//			whichGunEquipped = i;
//			WeaponArray[i]->weaponModel->ResetOrientation();
//			WeaponArray[i]->weaponModel->AttachToParent(cameraDummy);
//			WeaponArray[i]->weaponModel->SetLocalPosition(2.0f, -2.0f, 7.0f);
//			WeaponArray[i]->weaponModel->RotateY(-0.2f);
//		}
//	}
//
//	if (currentInteractionDistance >= 2.0f)
//	{
//		canCollide = false;
//		interactionZspeed = 0.0f;
//		currentInteractionDistance = 0.0f;
//		interactionDummy->SetLocalPosition(0, 0, 0);
//	}
//
//	if (myEngine->KeyHit(Key_R) && whichGunEquipped < numGuns)
//	{
//		WeaponArray[whichGunEquipped]->weaponModel->DetachFromParent();
//		WeaponArray[whichGunEquipped]->weaponModel->SetPosition(oldPlayerX, 0.2, oldPlayerZ);
//		WeaponArray[whichGunEquipped]->weaponModel->RotateLocalZ(90.0f);
//		whichGunEquipped = numGuns;
//	}
//}


void desktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;						      //Gets a handle to the current window
	const HWND hDesktop = GetDesktopWindow(); //Gets the size and places it to a variable
	GetWindowRect(hDesktop, &desktop);        //Gets the coordinates for the corner of the screen

	horizontal = desktop.right;               //Holds the values for the screen resolution.
	vertical = desktop.bottom;				  //Holds the values for the screen resolution.
}

void Fire(IModel* &cameraDummy, float& frameTime, float& shoottimer)
{
	switch (CurrentFireMode)
	{
	case Auto:
		if (myEngine->KeyHeld(Mouse_LButton))
		{
			shoottimer -= frameTime;
			/*if (shoottimer <= 0 && currentGun->magAmount > 0)
			{
				currentGun->shootingSound.play();
				shoottimer = currentGun->fireRate * 3;
			}*/

			for (int i = 0; i <currentGun->magCapacity; i++)
			{
				if (WeaponTime >currentGun->fireRate)
				{
					if (vMagazine[i]->status == Reloaded)
					{
						float matrix[4][4];
						cameraDummy->GetMatrix(&matrix[0][0]);
						vMagazine[i]->model->SetMatrix(&matrix[0][0]);
						vMagazine[i]->model->MoveLocalZ(10.0f);
						vMagazine[i]->model->RotateLocalX(90.0f);
						vMagazine[i]->model->Scale(0.004f);
						vMagazine[i]->status = Fired;
						currentGun->magAmount--;
						WeaponTime = 0.0f;

					}
				}

			}
		}
		break;
	case Burst:
		
		if (myEngine->KeyHit(Mouse_LButton))
		{
			canShoot = false;
		}
		
		if (!canShoot)
		{
			for (int i = 0; i <currentGun->magCapacity; i++)
			{
				if (WeaponTime > (currentGun->fireRate / 2))
				{
					if (vMagazine[i]->status == Reloaded)
					{
						currentGun->shootingSound.play();
						float matrix[4][4];
						cameraDummy->GetMatrix(&matrix[0][0]);
						vMagazine[i]->model->SetMatrix(&matrix[0][0]);
						vMagazine[i]->model->MoveLocalZ(10.0f);
						vMagazine[i]->model->RotateLocalX(90.0f);
						vMagazine[i]->model->Scale(0.004f);
						vMagazine[i]->status = Fired;
						currentGun->magAmount--;
						WeaponTime = 0.0f;
						bulletsFired++;
					}

				}
			}
		}
		if (bulletsFired == 3)
		{
			canShoot = true;
			bulletsFired = 0;
		}



		break;
	case Single:
		if (myEngine->KeyHit(Mouse_LButton))
		{
			for (int i = 0; i < currentGun->magCapacity; i++)
			{

				if (vMagazine[i]->status == Reloaded)
				{
					currentGun->shootingSound.play();
					float matrix[4][4];
					cameraDummy->GetMatrix(&matrix[0][0]);
					vMagazine[i]->model->SetMatrix(&matrix[0][0]);
					vMagazine[i]->model->MoveLocalZ(10.0f);
					vMagazine[i]->model->RotateLocalX(90.0f);
					vMagazine[i]->model->Scale(0.004f);
					vMagazine[i]->status = Fired;
					currentGun->magAmount--;
					break;
				}

			}



			break;

		}
		

	}
}

