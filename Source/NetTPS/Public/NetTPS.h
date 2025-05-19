// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(NetTPS, Log, All);

// 호출하는 함수와 줄번호 정보
#define APPINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define PRINT_APPINFO() UE_LOG(NetTPS, Warning, TEXT("%s"), *APPINFO)
#define PRINTLOG(fmt,...) \
	UE_LOG(NetTPS, Warning, TEXT("%s : %s"), *APPINFO, *FString::Printf(fmt, ##__VA_ARGS__))