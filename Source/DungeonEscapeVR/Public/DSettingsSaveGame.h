// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DSettingsSaveGame.generated.h"


/** Struct to hold all currently set graphics and audio settings */
USTRUCT(BlueprintType)
struct FSettingsData
{
	GENERATED_BODY()

	UPROPERTY()
	float MasterVolume;

	UPROPERTY()
	float AmbientVolume;

	UPROPERTY()
	float EffectsVolume;

	UPROPERTY()
	float UIVolume;

	UPROPERTY()
	int32 ResolutionScale;

	UPROPERTY()
	FString AntiAliasing;

	UPROPERTY()
	FString Shadows;

	UPROPERTY()
	FString Textures;

};

/**
 * 
 */
UCLASS()
class DUNGEONESCAPEVR_API UDSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

/**
 * Members
 */

private:

	UPROPERTY()
	FSettingsData SettingsData;
	

public:

	/** Maps used to map user displayed options to engine scalability settings */
	static const TMap<FString, FString> AntiAliasingMap;
	static const TMap<FString, FString> ShadowsMap;
	static const TMap<FString, FString> TexturesMap;


/**
 * Methods
 */

public:

	/** Set default settings. Lowest possible graphics settings, and 50 percent for all volume settings */
	UDSettingsSaveGame();

	/** Set settings in SettingsData */
	void SetMasterVolume(float Value);
	void SetAmbientVolume(float Value);
	void SetEffectsVolume(float Value);
	void SetUIVolume(float Value);
	void ResolutionScale(int32 Value);
	void SetAntiAliasing(const FString& AntiAliasingSetting);
	void SetShadows(const FString& ShadowsSetting);
	void SetTextures(const FString& TexturesSetting);


	/** Get settings from SettingsData  */
	float GetMasterVolume() const { return SettingsData.MasterVolume; }
	float GetAmbientVolume() const { return SettingsData.AmbientVolume; }
	float GetEffectsVolume() const { return SettingsData.EffectsVolume; }
	float GetUIVolume() const { return SettingsData.UIVolume; }
	int32 ResolutionScale() const { return SettingsData.ResolutionScale; }
	FString GetAntiAliasing() const { return SettingsData.AntiAliasing; }
	FString GetShadows() const { return SettingsData.Shadows; }
	FString GetTextures() const { return SettingsData.Textures; }


};
