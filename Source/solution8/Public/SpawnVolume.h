// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSpawnRow.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

class UBoxComponent;

UCLASS()
class SOLUTION8_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

	UPROPERTY(Visibleanywhere, BlueprintReadOnly, Category = "Spawning")
	USceneComponent* Scene;
	UPROPERTY(Visibleanywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawningBox;
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* ItemDataTable;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AActor* SpawnRandomItem();

	FItemSpawnRow* GetRandomItem() const;
	AActor* SpawnItem(TSubclassOf<AActor> ItemClass);
	FVector GetRandomPointInVolume() const;

	// ���̺� �ý��� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave System")
	int32 CurrentWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave System")
	int32 TotalWaves = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave System")
	int32 ItemsPerWave = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave System")
	float WaveDuration = 5.0f;

	// ���̺� ���� �Լ�
	void StartWave();
	void EndWave();
	void SpawnWaveItems();

private:
	FTimerHandle WaveTimerHandle;
};
