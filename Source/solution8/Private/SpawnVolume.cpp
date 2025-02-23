// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "MyGameState.h"
#include "Components/BoxComponent.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);

	ItemDataTable = nullptr;
}

AActor* ASpawnVolume::SpawnRandomItem()
{
	if (FItemSpawnRow* SelectedRow = GetRandomItem())
	{
		if (UClass* ActualClass = SelectedRow->ItemClass.Get())
		{
			return SpawnItem(ActualClass);
		}
	}

	return nullptr;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
	if (!ItemDataTable) return nullptr;

	TArray<FItemSpawnRow*> AllRows;
	static const FString ContextString(TEXT("ItemSpawnContext"));
	ItemDataTable->GetAllRows(ContextString, AllRows);

	if (AllRows.IsEmpty()) return nullptr;

	float TotalChance = 0.0f;
	for (const FItemSpawnRow* Row : AllRows)
	{
		if (Row)
		{
			TotalChance += Row->Spawnchance;
		}
	}

	const float RandValue = FMath::FRandRange(0.0f, TotalChance);

	float AccumulateChance = 0.0f;

	for (FItemSpawnRow* Row : AllRows)
	{
		AccumulateChance += Row->Spawnchance;
		if (RandValue <= AccumulateChance)
		{
			return Row;
		}
	}

	return nullptr;
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	FVector BoxOrigin = SpawningBox->GetComponentLocation();

	return BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
	if (!ItemClass) return nullptr;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		ItemClass,
		GetRandomPointInVolume(),
		FRotator::ZeroRotator
	);

	return SpawnedActor;
}

void ASpawnVolume::StartWave()
{
	if (CurrentWave >= TotalWaves)
	{
		UE_LOG(LogTemp, Warning, TEXT("All Waves Finished"));
		return;
	}

	CurrentWave++;
	UE_LOG(LogTemp, Warning, TEXT("Wave %d Start"), CurrentWave);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Wave %d Start"), CurrentWave));
	}

	// ���� ���̺꿡 �´� ������ ���� ����
	SpawnWaveItems();

	// ���� �ð��� ������ ���� ���̺� ����
	GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &ASpawnVolume::EndWave, 10.0f, false);
}

void ASpawnVolume::EndWave()
{
	UE_LOG(LogTemp, Warning, TEXT("Wave %d Finished"), CurrentWave);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Wave %d Finished!"), CurrentWave));
	}

	AMyGameState* GameState = GetWorld()->GetGameState<AMyGameState>();
	if (GameState)
	{
		GameState->CurrentWave = CurrentWave;
		GameState->OnWaveCompleted();
	}

	if (CurrentWave < TotalWaves)
	{
		UE_LOG(LogTemp, Warning, TEXT("15second later next wave start"));
		GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &ASpawnVolume::StartWave, 10.0f, false);
		StartWave();
	}
}

void ASpawnVolume::SpawnWaveItems()
{
	// ���̺긶�� �����ϴ� ������ ����
	int32 SpawnCount = ItemsPerWave * CurrentWave;

	for (int32 i = 0; i < SpawnCount; i++)
	{
		SpawnRandomItem();
	}
}

