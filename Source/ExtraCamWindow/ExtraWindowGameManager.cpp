#include "IExtraWindowPlugin.h"
#include "ExtraWindowGameManager.h"
#include "EngineModule.h"
#include "ExtraWindowUI.h"

AExtraWindowGameManager::AExtraWindowGameManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bGameWindowShown = true;
}

void AExtraWindowGameManager::BeginPlay()
{
	Super::BeginPlay();

	UGameEngine* gameEngine = Cast<UGameEngine>(GEngine);
	if (gameEngine != NULL)
	{
		GameWindow = gameEngine->GameViewportWindow;
		GameWindow.Pin()->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &AExtraWindowGameManager::OnGameWindowClosed));
		GEngine->GetFirstLocalPlayerController(GetWorld())->bShowMouseCursor = true;
		//gameEngine->GameViewport->SetCaptureMouseOnClick( EMouseCaptureMode::NoCapture );
		//gameEngine->GameViewport->GetWindow()->GetNativeWindow()->AdjustCachedSize( FVector2D( 100, 100 ) );
	}
}

void AExtraWindowGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRunning)
	{
		ShowGameWindow();
	}
	else
	{
		HideGameWindow();
	}

}

void AExtraWindowGameManager::ShowGameWindow()
{
	if (GameWindow.IsValid())
	{
		UExtraWindowUI::GetResolutionMonitor(true, GameWindowLocation, GameWindowSize);

		if (!bGameWindowShown)
		{
			GameWindowHiddenWidget->RemoveFromViewport();

			SetGraphicsQuality(GraphicsQuality);

			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
			FSystemResolution::RequestResolutionChange(GameWindowSize.X, GameWindowSize.Y, EWindowMode::WindowedFullscreen);

			GEngine->GetFirstLocalPlayerController(GetWorld())->SetPause(false);

			bGameWindowShown = true;
		}

		//do the reshape if neccesary
		if (GameWindow.Pin()->GetSizeInScreen().X != GameWindowSize.X)
		{
			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
		}
	}
}

void AExtraWindowGameManager::HideGameWindow()
{

	if (GameWindow.IsValid())
	{
		UExtraWindowUI::GetResolutionMonitor(true, GameWindowLocation, GameWindowSize);

		if (bGameWindowShown)
		{
			GameWindowHiddenWidget->AddToViewport(0);

			SetGraphicsQuality(0);

			FSystemResolution::RequestResolutionChange(GameWindowSize.X, GameWindowSize.Y, EWindowMode::WindowedFullscreen);

			GEngine->GetFirstLocalPlayerController(GetWorld())->SetPause(true);

			bGameWindowShown = false;
		}

		//set reshape of the game window if size not proper to hidden-mode
		bool bReshapeWindow = false;
		if (GameWindow.Pin()->GetSizeInScreen().X != GameWindowSize.X)
		{
			bReshapeWindow = true;
		}

		//set relocation of the game window if dragged
		if (bGameWindowDragged)
		{
			if (UExtraWindowUI::GetWINLeftMouseButtonDown())
			{
				GameWindowLocation = UExtraWindowUI::GetWINCursorPosition() - 75;
			}
			else
			{
				bGameWindowDragged = false;
			}
			bReshapeWindow = true;
		}

		//do the reshape (and relocation) if neccesary
		if (bReshapeWindow)
		{
			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
		}

	}
}

void AExtraWindowGameManager::OnGameWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	StopGame();
}

void AExtraWindowGameManager::SetGraphicsQuality(const int32 newGraphicsQuality)
{
	GEngine->GetGameUserSettings()->ScalabilityQuality.SetFromSingleQualityLevel(newGraphicsQuality);
	GEngine->GetGameUserSettings()->ApplyNonResolutionSettings();
}