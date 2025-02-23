#include "MyGameState.h"
#include "MyGameInstance.h"
#include "MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

AMyGameState::AMyGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 30.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
	CurrentWave = 0;
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&AMyGameState::UpdateHUD,
		0.1f,
		true
	);

	GetWorldTimerManager().SetTimer(
		LevelCountdownTimerHandle,
		this,
		&AMyGameState::DecreaseLevelTime,
		1.0f,
		true
	);
}

void AMyGameState::DecreaseLevelTime()
{
	if (LevelDuration > 0)
	{
		LevelDuration--;
	}

	UpdateHUD();

	if (LevelDuration <= 0)
	{
		GetWorldTimerManager().ClearTimer(LevelCountdownTimerHandle);
		EndLevel();
	}
}

int32 AMyGameState::GetScore() const
{
	return Score;
}

void AMyGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* SpartaGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

void AMyGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			MyPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* SpartaGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	bWavesCompleted = false;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	for (AActor* VolumeActor : FoundVolumes)
	{
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(VolumeActor);
		if (SpawnVolume)
		{
			SpawnVolume->StartWave();
		}
	}
	UpdateHUD();
	UE_LOG(LogTemp, Warning, TEXT("Level %d Start!"), CurrentLevelIndex+1);
}

void AMyGameState::OnLevelTimeUp()
{
	EndLevel();
}

void AMyGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d/%d"),
		CollectedCoinCount,
		SpawnedCoinCount);

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		if (CurrentLevelIndex == MaxLevels - 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("finished"));
			OnGameOver();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("next level"));
			EndLevel();
		}
	}
}

void AMyGameState::EndLevel()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMyGameInstance* SpartaGameInstance = Cast<UMyGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}

	if (CurrentLevelIndex > MaxLevels)
	{
		OnGameOver();
		return;
	}
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver();
	}
}

void AMyGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			MyPlayerController->ShowMainMenu(true);
		}
	}
}

void AMyGameState::OnWaveCompleted()
{
	bWavesCompleted = true;
	if (CurrentWave >= 3) {
		if (CurrentLevelIndex == MaxLevels - 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("last level, last wave start! last 15second!"));
			GetWorldTimerManager().SetTimer(LevelEndTimerHandle, this, &AMyGameState::OnGameOver, 15.0f, false);
		}
		else if (CurrentWave >= 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("last wave finished! after 15second play next level!"));
			GetWorldTimerManager().SetTimer(LevelEndTimerHandle, this, &AMyGameState::EndLevel, 15.0f, false);
		}
	}
}

void AMyGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = MyPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), LevelDuration)));
				}

				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance);
						if (MyGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), MyGameInstance->TotalScore)));
						}
					}
				}

				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}
			}
		}
	}
}