// Fill out your copyright notice in the Description page of Project Settings.


#include "game_GameInstance.h"

void Ugame_GameInstance::IncrementScoreHumanPlayer()
{
	ScoreHumanPlayer += 1;
}

void Ugame_GameInstance::IncrementScoreAiPlayer()
{
	ScoreAiPlayer += 1;
}

int32 Ugame_GameInstance::GetScoreHumanPlayer()
{
	return ScoreHumanPlayer;
}

int32 Ugame_GameInstance::GetScoreAiPlayer()
{
	return ScoreAiPlayer;
}

FString Ugame_GameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

void Ugame_GameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}