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

#include "Components/ActorComponent.h"
#include "TangoDataTypes.h"
#include "TangoEventComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTangoConnect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTangoDisconnect);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTangoServiceException,FTangoEvent,Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFisheyeOverExposed, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFisheyeUnderExposed, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorOverExposed, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorUnderExposed, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooFewFeaturesTracked, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAreaDescriptionSaveProgress, FTangoEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnknownEvent, FTangoEvent, Event);

UCLASS(ClassGroup = Tango, meta = (BlueprintSpawnableComponent))
class PROJECTTANGOPLUGIN_API UTangoEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTangoEventComponent(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tango has connected"))
		FOnTangoConnect OnTangoConnect;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tango has disconnected"))
		FOnTangoDisconnect OnTangoDisconnect;

	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tango has a service exception"))
		FOnTangoServiceException OnTangoServiceException;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tangos fisheye camera is overexposed"))
		FOnFisheyeOverExposed OnFisheyeOverExposed;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tangos fisheye camera is underexposed"))
		FOnFisheyeUnderExposed OnFisheyeUnderExposed;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tagnos color camera is overexposed"))
		FOnColorOverExposed OnColorOverExposed;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tangos color camera is underexposed"))
		FOnColorUnderExposed OnColorUnderExposed;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when Tango was unable to track enough features to keep up with movement"))
		FOnTooFewFeaturesTracked OnTooFewFeaturesTracked;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires when the progess on saving an ADF file changed"))
		FOnAreaDescriptionSaveProgress OnAreaDescriptionSaveProgress;
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Fires whenever something unkown happens"))
		FOnUnknownEvent OnUnknownEvent;
protected:

	virtual void BeginPlay() override;

};
