/*Copyright 2016 Opaque Media Group

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#pragma once

#if PLATFORM_ANDROID
#include "tango_client_api.h"
#endif

#include "TangoDeviceImage.generated.h"


static bool bTexturesHaveDataInThem;
static double ImageBufferTimestamp;

UCLASS(NotBlueprintable, NotPlaceable, Transient)
class UTangoDeviceImage : public UObject
{
	GENERATED_BODY()
public:
	virtual void BeginDestroy() override;

	void Init(
#if PLATFORM_ANDROID
		TangoConfig config_
#endif
		);

	bool CreateYUVTextures(
#if PLATFORM_ANDROID
		TangoConfig config_
#endif
	);
	void ConnectCallback();
	bool DisconnectCallback();

	void TickByDevice();

	UTexture* GetYTexture();

	UTexture * GetCrTexture();

	UTexture * GetCbTexture();

	//Tango Image functions
	bool bIsImageBufferSet;
	float GetImageBufferTimestamp();

	bool setRuntimeConfig(FTangoRuntimeConfig& runtimeConfig);

private:

	bool WantToConnectCallbacks;
	bool CallbackConnected;

	bool TexturesReady();
	void OnNewDataAvailable();
	void CheckConnectCallback();

	bool NewDataAvailable;

};
