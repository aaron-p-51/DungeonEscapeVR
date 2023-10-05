// Fill out your copyright notice in the Description page of Project Settings.


#include "DSettingsSaveGame.h"


// Min and Max volume settings values
const float MIN_VOLUME_SETTING = 0.f;
const float MAX_VOLUME_SETTING = 1.f;

// Min and Max texture resolution values
const int32 MIN_RESOLUTION_SETTING = 0;
const int32 MAX_RESOLUTION_SETTING = 100;


const TMap<FString, FString> UDSettingsSaveGame::AntiAliasingMap =
{
	{"Low", "0"},
	{"Medium", "2"},
	{"High", "4"},
};


const TMap<FString, FString> UDSettingsSaveGame::ShadowsMap =
{
	{"Off", "0"},
	{"Low", "1"},
	{"High", "2"},
};


const TMap<FString, FString> UDSettingsSaveGame::TexturesMap =
{
	{"Low", "0"},
	{"Medium", "1"},
	{"High", "2"},
	{"Epic", "3"}
};


UDSettingsSaveGame::UDSettingsSaveGame()
{
	SettingsData.MasterVolume = 0.5f;
	SettingsData.AmbientVolume = 0.5f;
	SettingsData.EffectsVolume = 0.5f;
	SettingsData.UIVolume = 0.5f;
	SettingsData.ResolutionScale = 50;

	TArray<FString> ValueStrings;
	AntiAliasingMap.GenerateValueArray(ValueStrings);
	if (ValueStrings.Num() >= 0)
	{
		SettingsData.AntiAliasing = ValueStrings[0];
	}

	ValueStrings.Empty();
	ShadowsMap.GenerateValueArray(ValueStrings);
	if (ValueStrings.Num() >= 1)
	{
		SettingsData.Shadows = ValueStrings[1];
	}

	ValueStrings.Empty();
	TexturesMap.GenerateValueArray(ValueStrings);
	if (ValueStrings.Num() >= 1)
	{
		SettingsData.Textures = ValueStrings[0];
	}
}


void UDSettingsSaveGame::SetMasterVolume(float Value)
{
	Value = ClampAudioRange(Value);
	SettingsData.MasterVolume = Value;
}


void UDSettingsSaveGame::SetAmbientVolume(float Value)
{
	Value = ClampAudioRange(Value);
	SettingsData.AmbientVolume = Value;
}


void UDSettingsSaveGame::SetEffectsVolume(float Value)
{
	Value = ClampAudioRange(Value);
	SettingsData.EffectsVolume = Value;
}


void UDSettingsSaveGame::SetUIVolume(float Value)
{
	Value = ClampAudioRange(Value);
	SettingsData.UIVolume = Value;
}


void UDSettingsSaveGame::ResolutionScale(int32 Value)
{
	Value = FMath::Clamp<int32>(Value, MIN_RESOLUTION_SETTING, MAX_RESOLUTION_SETTING);
	SettingsData.ResolutionScale = Value;
}


void UDSettingsSaveGame::SetAntiAliasing(const FString& AntiAliasingSetting)
{
	if (const FString* AntiAliasingValue = UDSettingsSaveGame::AntiAliasingMap.Find(AntiAliasingSetting))
	{
		SettingsData.AntiAliasing = *AntiAliasingValue;
	}
}


void UDSettingsSaveGame::SetShadows(const FString& ShadowsSetting)
{
	if (const FString* ShadowsValue = UDSettingsSaveGame::ShadowsMap.Find(ShadowsSetting))
	{
		SettingsData.Shadows = *ShadowsValue;
	}
}


void UDSettingsSaveGame::SetTextures(const FString& TexturesSetting)
{
	if (const FString* TexturesValue = UDSettingsSaveGame::TexturesMap.Find(TexturesSetting))
	{
		SettingsData.Textures = *TexturesValue;
	}
}


float UDSettingsSaveGame::ClampAudioRange(float Value) const
{
	return FMath::Clamp<float>(Value, MIN_VOLUME_SETTING, MAX_VOLUME_SETTING);
}
