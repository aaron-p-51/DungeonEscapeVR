// Fill out your copyright notice in the Description page of Project Settings.


#include "DSettingsSaveGame.h"

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
	Value = FMath::Clamp<float>(Value, 0.f, 1.f);
	SettingsData.MasterVolume = Value;
}


void UDSettingsSaveGame::SetAmbientVolume(float Value)
{
	Value = FMath::Clamp<float>(Value, 0.f, 1.f);
	SettingsData.AmbientVolume = Value;
}


void UDSettingsSaveGame::SetEffectsVolume(float Value)
{
	Value = FMath::Clamp<float>(Value, 0.f, 1.f);
	SettingsData.EffectsVolume = Value;
}


void UDSettingsSaveGame::SetUIVolume(float Value)
{
	Value = FMath::Clamp<float>(Value, 0.f, 1.f);
	SettingsData.UIVolume = Value;
}


void UDSettingsSaveGame::ResolutionScale(int32 Value)
{
	Value = FMath::Clamp<int32>(Value, 0, 100);
	SettingsData.ResolutionScale = Value;
}


void UDSettingsSaveGame::SetAntiAliasing(const FString& AntiAliasingSetting)
{
	const FString* AntiAliasingValue = UDSettingsSaveGame::AntiAliasingMap.Find(AntiAliasingSetting);
	if (AntiAliasingValue)
	{
		SettingsData.AntiAliasing = *AntiAliasingValue;
	}
}


void UDSettingsSaveGame::SetShadows(const FString& ShadowsSetting)
{
	const FString* ShadowsValue = UDSettingsSaveGame::ShadowsMap.Find(ShadowsSetting);
	if (ShadowsValue)
	{
		SettingsData.Shadows = *ShadowsValue;
	}
}


void UDSettingsSaveGame::SetTextures(const FString& TexturesSetting)
{
	const FString* TexturesValue = UDSettingsSaveGame::TexturesMap.Find(TexturesSetting);
	if (TexturesValue)
	{
		SettingsData.Textures = *TexturesValue;
	}
}
