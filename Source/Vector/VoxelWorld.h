// VoxelWorld.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "VoxelWorld.generated.h"

class AVoxelChunk;
class UVoxelBaseDataAsset;
class UVoxelBlockDataAsset;
class UVoxelSubstanceDataAsset;
class UMaterialInterface;

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

	UPROPERTY(EditAnywhere, Category = "Voxel | Data", meta = (DisplayThumbnail = "false"))
	TArray<TObjectPtr<UVoxelBaseDataAsset>> VoxelDataAssets;

	UPROPERTY(EditAnywhere, Category = "Voxel | Generation")
	TObjectPtr<UVoxelBaseDataAsset> VoidVoxelData;

	UPROPERTY(EditAnywhere, Category = "Voxel | Generation")
	TObjectPtr<UVoxelBaseDataAsset> DefaultVoxelData;

	UPROPERTY(EditAnywhere, Category = "Voxel | Room Generation")
	float RoomRadius = 200.f;

	void InitializeWorld(int32 NumberOfPlayers);

	UVoxelBaseDataAsset* GetVoxelData(int32 VoxelID) const;
	UMaterialInterface* GetVoxelMaterial(int32 VoxelID) const;
	int32 GetVoidID() const { return VoidID; }
	int32 GetDefaultID() const { return DefaultID; }

	const TArray<APlayerStart*>& GetPlayerStartPoints() const { return PlayerStartPoints; }

	int32 GetGlobalVoxelID(const FIntVector& GlobalCoord) const;
	void SetGlobalVoxelID(const FIntVector& GlobalCoord, int32 NewVoxelID);

	float GetGlobalDurability(const FIntVector& GlobalCoord) const;
	void SetGlobalDurability(const FIntVector& GlobalCoord, float NewDurability);

	float CalculateGlobalDensity(const FIntVector& GlobalCoord) const;

	bool IsGlobalCoordValid(const FIntVector& GlobalCoord) const;

	int32 GetGlobalIndex(const FIntVector& GlobalCoord) const;

	FVector GetSmoothNormalAtGlobalCoord(const FIntVector& GlobalCoord) const;

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
	int32 VoidID = 0;
	UPROPERTY()
	int32 DefaultID = 1;

	UPROPERTY()
	TArray<int32> GlobalVoxelIDMap;

	UPROPERTY()
	TArray<float> GlobalDurabilityMap;

	int32 WorldVoxelVolume = 0;
	FIntVector WorldVoxelDimensions;


	void GenerateRooms(int32 NumberOfRoomsToGenerate);
	void SpawnChunk(const FIntVector& ChunkCoord);
};
