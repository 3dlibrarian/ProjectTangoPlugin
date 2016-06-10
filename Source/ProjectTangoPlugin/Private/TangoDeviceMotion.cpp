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

#include "ProjectTangoPluginPrivatePCH.h"
#include "TangoDeviceMotion.h"
#include "TangoFromToCObject.h"
#include "TangoCoordinateConversions.h"

#include "TangoDevice.h"


#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "AndroidApplication.h"
#endif

UTangoDeviceMotion::UTangoDeviceMotion() : UObject(), FTickableGameObject()
{
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::UTangoDeviceMotion: called"));
}

void UTangoDeviceMotion::ProperInitialize()
{
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::Initialize: called"));
#if PLATFORM_ANDROID
#endif
	CheckForChangeInRequests();
	bIsProperlyInitialized = true;
}

void UTangoDeviceMotion::ConnectCallback()
{
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::ConnectCallback: called"));
#if PLATFORM_ANDROID
	TangoCoordinateFramePair Pairs[RequestedPairs.Num()];
	int i = 0;
	for (auto& Elem : RequestedPairs)
	{
		Pairs[i].base = static_cast<TangoCoordinateFrameType>(static_cast<int32>(Elem.Key.BaseFrame));
		Pairs[i].target = static_cast<TangoCoordinateFrameType>(static_cast<int32>(Elem.Key.TargetFrame));
		i++;
	}
	if (TangoService_connectOnPoseAvailable(RequestedPairs.Num(), Pairs, [](void*, const TangoPoseData* Pose) {if (UTangoDevice::Get().getTangoDeviceMotionPointer() != nullptr)UTangoDevice::Get().getTangoDeviceMotionPointer()->OnPoseAvailable(Pose); }) != TANGO_SUCCESS)
	{
		UE_LOG(ProjectTangoPlugin, Error, TEXT("UTangoDeviceMotion::ConnectCallback: Was unsuccessfull"));
	}
#endif
	bCallbackIsConnected = true;
}

void UTangoDeviceMotion::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::UTangoDeviceMotion: Destructor called"));
}


/** Function called by the Tango Library with head pose data.
*/
#if PLATFORM_ANDROID
void UTangoDeviceMotion::OnPoseAvailable(const TangoPoseData* Pose)
{
	FTangoPoseData Data = FromCPointer(Pose);
	if (Data.FrameOfReference.BaseFrame == ETangoCoordinateFrameType::PREVIOUS_DEVICE_POSE && Data.FrameOfReference.TargetFrame == ETangoCoordinateFrameType::DEVICE)
	{
		PoseMutex.Lock();
		if (BroadcastTangoPoseData.Contains(Data.FrameOfReference))//We have to accumulate the previous device pose -> device pose frame!
		{
			FTangoPoseData OldData = BroadcastTangoPoseData[Data.FrameOfReference];
			Data.Position = OldData.QuatRotation * Data.Position + OldData.Position;
			Data.QuatRotation = OldData.QuatRotation * Data.QuatRotation;
			Data.Rotation = Data.QuatRotation.Rotator();
			BroadcastTangoPoseData[Data.FrameOfReference] = Data;
		}
		else
		{
			BroadcastTangoPoseData.FindOrAdd(Data.FrameOfReference) = Data;
		}
		PoseMutex.Unlock();
	}
	else
	{
		PoseMutex.Lock();
		BroadcastTangoPoseData.FindOrAdd(Data.FrameOfReference) = Data;
		PoseMutex.Unlock();
	}
}
#endif

//START - Tango Motion functions

FTangoPoseData UTangoDeviceMotion::GetPoseAtTime(FTangoCoordinateFramePair FrameOfReference, float Timestamp)
{
    //Prevent Tango calls before the system is ready, return null data instead
    if(!(UTangoDevice::Get().IsTangoServiceRunning()))
    {
        return FTangoPoseData();
    }
    
	//@TODO: See if there's a way to remove the need for this data structure here
	FTangoPoseData BlueprintFriendlyPoseData;
	TangoSpaceConversions::TangoSpaceConversionPair SpaceConverter;
	bool bIsValidQuery = TangoSpaceConversions::GetSpaceConversionPair(SpaceConverter, FrameOfReference);

	if (!bIsValidQuery)
	{
		UE_LOG(ProjectTangoPlugin, Warning, TEXT("UTangoDeviceMotion::GetPoseAtTime: Query not valid"));
		BlueprintFriendlyPoseData.StatusCode = ETangoPoseStatus::INVALID;
		return BlueprintFriendlyPoseData;
	}

	if (SpaceConverter.bIsStatic)//Just querying extrinsics
	{
		TangoSpaceConversions::ModifyPose(BlueprintFriendlyPoseData, SpaceConverter);
		BlueprintFriendlyPoseData.Timestamp = Timestamp;
		return BlueprintFriendlyPoseData;
	}
	else if (SpaceConverter.bNeedToBeQueriedFromDevice)
	{
		FrameOfReference.TargetFrame = ETangoCoordinateFrameType::DEVICE;
	}

#if PLATFORM_ANDROID
	TangoPoseData Result;

	////Remember to observe the Tango status in case the system isn't ready yet
	TangoErrorType ResultOfServiceCall;
	if (TangoService_getPoseAtTime(static_cast<double> (Timestamp), ToCObject(FrameOfReference), &Result) != TANGO_SUCCESS)
	{
		UE_LOG(ProjectTangoPlugin, Warning, TEXT("UTangoDeviceMotion::GetPoseAtTime: TangoService_getPoseAtTime not successful"));
		//return a generic object
		return FTangoPoseData();
	}
	BlueprintFriendlyPoseData = FromCPointer(&Result);
#endif
	TangoSpaceConversions::ModifyPose(BlueprintFriendlyPoseData, SpaceConverter);
	return BlueprintFriendlyPoseData;
}

bool UTangoDeviceMotion::IsTickable() const
{
	return bIsProperlyInitialized;
}

TStatId UTangoDeviceMotion::GetStatId() const
{
	return TStatId();
}


void UTangoDeviceMotion::Tick(float DeltaTime)
{
	PoseMutex.Lock();
	TMap<FTangoCoordinateFramePair, FTangoPoseData> BroadcastTangoPoseDataCopy = BroadcastTangoPoseData;
	BroadcastTangoPoseData.Empty(RequestedPairs.Num());
	PoseMutex.Unlock();
	
	for (auto& Elem : BroadcastTangoPoseDataCopy)
	{
		auto& BroadCastPairs = RequestedPairs[Elem.Key];
		for (auto& BroadCastPair : BroadCastPairs)
		{
			FTangoPoseData Pose = Elem.Value;
			TangoSpaceConversions::ModifyPose(Pose, BroadCastPair.Value.RequestedSpace);
			for (int32 ComponentID : BroadCastPair.Value.ComponentIDs)
			{
				if (UTangoDevice::Get().MotionComponents[ComponentID] != nullptr)
				{
					UTangoDevice::Get().MotionComponents[ComponentID]->OnTangoPoseAvailable.Broadcast(Pose, BroadCastPair.Key);
				}
			}
		}
	}
}

void UTangoDeviceMotion::ResetMotionTracking()
{
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::ResetMotionTracking: Called"));
#if PLATFORM_ANDROID
	TangoService_resetMotionTracking();
#endif
}

bool UTangoDeviceMotion::IsLocalized()
{
	//@TODO: See if there's a cleaner way to poll the service than getting entire pose value and checking the validity.
#if PLATFORM_ANDROID
	TangoPoseData Result;

	////Remember to observe the Tango status in case the system isn't ready yet
	TangoErrorType ResultOfServiceCall;
	TangoCoordinateFramePair ADFFramePair = { TANGO_COORDINATE_FRAME_AREA_DESCRIPTION, TANGO_COORDINATE_FRAME_DEVICE };

	ResultOfServiceCall = TangoService_getPoseAtTime(0.0, ADFFramePair, &Result);

	if (ResultOfServiceCall == TANGO_SUCCESS)
	{
		if (Result.status_code == TANGO_POSE_VALID)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		UE_LOG(ProjectTangoPlugin, Log, TEXT("TangoDeviceAreaLearning::IsLocalized: Could not successfully call GetPoseAtTime!"));
		return false;
	}

#endif
	//We're not localized if we're not on Android
	return false;
}

void UTangoDeviceMotion::CheckForChangeInRequests()
{
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::CheckForChangeInRequests: Called"));
	TMap<FTangoCoordinateFramePair, TMap<FTangoCoordinateFramePair,MotionEventRequestedFramePair>> NewRequestedPairs;
	for (int32 mc = 0; mc < UTangoDevice::Get().RequestedPairs.Num(); mc++)
	{
		for (int32 i = 0; i < UTangoDevice::Get().RequestedPairs[mc].Num(); ++i)
		{
			TangoSpaceConversions::TangoSpaceConversionPair RequestPairSpace; //We cannot request any pair so we have to look stuff up
			if (TangoSpaceConversions::GetSpaceConversionPair(RequestPairSpace, UTangoDevice::Get().RequestedPairs[mc][i]))
			{
				FTangoCoordinateFramePair TrueRequestPair;
				if (RequestPairSpace.bNeedToBeQueriedFromDevice)
				{
					TrueRequestPair = FTangoCoordinateFramePair(UTangoDevice::Get().RequestedPairs[mc][i].BaseFrame,ETangoCoordinateFrameType::DEVICE);
				}
				else if(RequestPairSpace.bIsStatic)//Ignore static ones
				{
					continue;
				}
				else
				{
					TrueRequestPair = UTangoDevice::Get().RequestedPairs[mc][i];
				}
				//Now add this to NewRequestedPairs in order to rebuild it.
				auto& RequestMap = NewRequestedPairs.FindOrAdd(TrueRequestPair);
				auto& Entry = RequestMap.FindOrAdd(RequestPairSpace.Pair);
				Entry.RequestedSpace = RequestPairSpace;
				Entry.ComponentIDs.AddUnique(mc);
			}
		}
	}
	if (NewRequestedPairs.Num() == RequestedPairs.Num())
	{
		for (auto& Elem : NewRequestedPairs)
		{
			if (!RequestedPairs.Contains(Elem.Key))
			{
				ConnectCallback();
				break;
			}
		}
	}
	else
	{
		ConnectCallback();
	}
	RequestedPairs = NewRequestedPairs;
	UE_LOG(ProjectTangoPlugin, Log, TEXT("UTangoDeviceMotion::CheckForChangeInRequests: Finished"));
}