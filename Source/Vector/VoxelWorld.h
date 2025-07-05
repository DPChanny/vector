#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelBaseDataAsset.h"
#include "VoxelWorld.generated.h"

class AVoxelChunk;
class AVoxelDebugActor;
class APlayerStart;
class UMaterialInterface;
class UVoxelBlockDataAsset;
class UVoxelVoidDataAsset;
class UVoxelBorderDataAsset;

USTRUCT(BlueprintType)
struct FNexus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Center;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Radius;
};

UCLASS()
class VECTOR_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

protected:
	AVoxelWorld();

	UPROPERTY(EditAnywhere, Category = "World | Meta")
	int32 ChunkSize = 10;

	UPROPERTY(EditAnywhere, Category = "World | Meta")
	int32 VoxelSize = 50;

	UPROPERTY(EditAnywhere, Category = "World | Meta")
	FIntVector WorldSizeInChunks = FIntVector(20, 20, 20);

	UPROPERTY(EditAnywhere, Category = "World | Voxel")
	TArray<TObjectPtr<UVoxelBlockDataAsset>> VoxelBlockDataAssets;

	UPROPERTY(EditAnywhere, Category = "World | Voxel")
	TObjectPtr<UVoxelVoidDataAsset>  VoxelVoidDataAsset;

	UPROPERTY(EditAnywhere, Category = "World | Voxel")
	TObjectPtr<UVoxelBorderDataAsset>  VoxelBorderDataAsset;

	UPROPERTY(EditAnywhere, Category = "Voxel | Nexus")
	float NexusRadius = 200.f;

	void AddDebugVoxel(const FIntVector& VoxelCoord);
	void RemoveDebugVoxel(const FIntVector& VoxelCoord);

	UPROPERTY(EditDefaultsOnly, Category = "Voxel Debug")
	TSubclassOf<AVoxelDebugActor> DebugActorClass;

	TMap<FIntVector, TObjectPtr<AVoxelDebugActor>> DebugVoxels;

	TSet<FIntVector> DebugVoxelsBuffer;

public:
	void Initialize(int32 NumberOfPlayers);

	UVoxelBaseDataAsset* GetVoxelData(int32 VoxelID) const;

	inline const int32 GetVoidID() const { return 0; }
	inline const int32 GetBorderID() const { return 1; }
	inline const int32 GetDefaultBlockID() const { return 2; }

	inline const float GetSurfaceLevel() const { return 0.f; }

	inline const int32 GetChunkSize() const { return ChunkSize; }
	inline const int32 GetVoxelSize() const { return VoxelSize; }

	const TArray<APlayerStart*>& GetPlayerStarts() const { return PlayerStarts; }

	const int32 GetVoxelID(const TObjectPtr<UVoxelBaseDataAsset>& VoxelDataAsset);
	const int32 GetVoxelID(const FIntVector& VoxelCoord) const;
	void SetVoxelID(const FIntVector& VoxelCoord, int32 NewVoxelID);

	float GetDurability(const FIntVector& VoxelCoord) const;
	void SetDurability(const FIntVector& VoxelCoord, float NewDurability);

	float GetDensity(const FIntVector& VoxelCoord) const;

	bool IsVoxelCoordValid(const FIntVector& VoxelCoord) const;

	int32 GetIndex(const FIntVector& VoxelCoord) const;

	void DamageVoxel(const FVector& HitPoint, float Radius, float DamageAmount);
	void ConstructVoxel(const FVector& Center, float Radius, float ConstructionAmount, int32 VoxelIDToConstruct);

	FIntVector WorldPosToVoxelCoord(const FVector& WorldPos) const;
	FIntVector VoxelCoordToChunkCoord(const FIntVector& VoxelCoord);

	void GetVoxelCoordsInRadius(const FVector& Center, float Radius, TSet<FIntVector>& FoundVoxelCoords);

	void InitializeNexuses(int32 NexusCount);
	void InitializeChunk(const FIntVector& ChunkCoord);

	void SetDebugVoxels(const TSet<FIntVector>& NewDebugVoxels);
	void SetDebugVoxel(const FIntVector& NewDebugVoxel);
	void FlushDebugVoxelBuffer();

private:
	void ProcessVoxel(const FVector& Center, float Radius, TFunction<void(const FIntVector&, TSet<FIntVector>&)> VoxelModifier);
	void AddDirtyChunk(const FIntVector& VoxelCoord, TSet<FIntVector>& DirtyChunks);
	void UpdateDirtyChunk(const TSet<FIntVector>& DirtyChunks);
	bool IsSurfaceVoxel(const FIntVector& VoxelCoord) const;

	UPROPERTY()
	TArray<TObjectPtr<APlayerStart>> PlayerStarts;

	UPROPERTY()
	TMap<FIntVector, TObjectPtr<AVoxelChunk>> Chunks;

	UPROPERTY()
	TMap<int32, TObjectPtr<UVoxelBaseDataAsset>> VoxelDataMap;

	UPROPERTY()
	TArray<FNexus> Nexuses;

	UPROPERTY()
	TArray<int32> VoxelIDs;

	UPROPERTY()
	TArray<float> Durabilities;

	int32 WorldVolume = 0;
	FIntVector WorldSize;
};
