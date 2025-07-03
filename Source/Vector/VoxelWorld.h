#pragma once

#include "CoreMinimal.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelBlockDataAsset.h"
#include "VoxelVoidDataAsset.h"
#include "VoxelBorderDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "VoxelWorld.generated.h"

class AVoxelChunk;
class UMaterialInterface;

constexpr int32 VOXEL_VOID_ID = 0;
constexpr int32 VOXEL_BORDER_ID = 1;
constexpr int32 VOXEL_DEFAULT_BLOCK_ID = 2;

USTRUCT(BlueprintType)
struct FSphericalRoom
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Center;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Radius;
};

UCLASS()
class VECTOR_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorld();

	UPROPERTY(EditAnywhere, Category = "Voxel | World")
	FIntVector WorldSizeInChunks = FIntVector(10, 10, 10);

	UPROPERTY(EditAnywhere, Category = "Voxel | Data")
	TArray<TObjectPtr<UVoxelBlockDataAsset>> VoxelBlockDataAssets;

	UPROPERTY(EditAnywhere, Category = "Voxel | Data")
	TObjectPtr<UVoxelVoidDataAsset>  VoxelVoidDataAsset;

	UPROPERTY(EditAnywhere, Category = "Voxel | Data")
	TObjectPtr<UVoxelBorderDataAsset>  VoxelBorderDataAsset;

	UPROPERTY(EditAnywhere, Category = "Voxel | Room Generation")
	float RoomRadius = 200.f;

	void Initialize(int32 NumberOfPlayers);

	UVoxelBaseDataAsset* GetVoxelData(int32 VoxelID) const;
	UMaterialInterface* GetVoxelMaterial(int32 VoxelID) const;

	int32 GetVoidID() const { return VOXEL_VOID_ID; }
	int32 GetBorderID() const { return VOXEL_BORDER_ID; }
	int32 GetDefaultBlockID() const { return VOXEL_DEFAULT_BLOCK_ID; }

	const TArray<APlayerStart*>& GetPlayerStartPoints() const { return PlayerStartPoints; }

	int32 GetVoxelID(const FIntVector& VoxelCoord) const;
	void SetVoxelID(const FIntVector& VoxelCoord, int32 NewVoxelID);

	float GetDurability(const FIntVector& VoxelCoord) const;
	void SetDurability(const FIntVector& VoxelCoord, float NewDurability);

	float CalculateDensity(const FIntVector& VoxelCoord) const;

	bool IsVoxelCoordValid(const FIntVector& VoxelCoord) const;

	int32 GetIndex(const FIntVector& VoxelCoord) const;

	void DamageVoxel(const FVector& HitPoint, float Radius, float DamageAmount);
	void ConstructVoxel(const FIntVector& TargetCoord, int32 NewVoxelID);

private:
	UPROPERTY()
	TArray<TObjectPtr<APlayerStart>> PlayerStartPoints;

	UPROPERTY()
	TMap<FIntVector, TObjectPtr<AVoxelChunk>> Chunks;

	UPROPERTY()
	TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataMap;

	UPROPERTY()
	TArray<FSphericalRoom> GeneratedRooms;

	UPROPERTY()
	TArray<int32> VoxelIDs;

	UPROPERTY()
	TArray<float> Durabilities;

	int32 WorldVolume = 0;
	FIntVector WorldSize;


	void GenerateRooms(int32 NumberOfRoomsToGenerate);
	void SpawnChunk(const FIntVector& ChunkCoord);
};
