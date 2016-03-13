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
#include "TangoDataTypes.h"
//#include "TangoFromToCObject.generated.h"

#if PLATFORM_ANDROID
#include "tango_client_api.h"

static FTangoCoordinateFramePair FromCObject(TangoCoordinateFramePair ToConvert)
{
	return FTangoCoordinateFramePair((ETangoCoordinateFrameType::Type) ToConvert.base, (ETangoCoordinateFrameType::Type) ToConvert.target);
}

static TangoCoordinateFramePair ToCObject(FTangoCoordinateFramePair ToConvert)
{
	TangoCoordinateFramePair Result;
	Result.base = (TangoCoordinateFrameType)(int8)ToConvert.BaseFrame;
	Result.target = (TangoCoordinateFrameType)(int8)ToConvert.TargetFrame;
	return Result;
}
//@TODO: Update this code to include the proper transformation to Unreal coordinates
static FTangoPoseData FromCPointer(const TangoPoseData* ToConvert)
{
	FTangoPoseData Result;
	Result.Position = FVector(ToConvert->translation[0], ToConvert->translation[1], ToConvert->translation[2]);
	Result.Rotation = FRotator(FQuat(ToConvert->orientation[0], ToConvert->orientation[1], ToConvert->orientation[2], ToConvert->orientation[3]));
	Result.QuatRotation = FQuat(ToConvert->orientation[0], ToConvert->orientation[1], ToConvert->orientation[2], ToConvert->orientation[3]);
	Result.FrameOfReference = FromCObject(ToConvert->frame);
	Result.Timestamp = ToConvert->timestamp;
	Result.StatusCode = (ETangoPoseStatus::Type)ToConvert->status_code;
	return Result;
}


//@NOTE: This requires a WorldToMetersValue to be passed in as it may change during runtime.
static FTangoPoseData FromCObject(const TangoPoseData ToConvert, float WorldToMetersValue)
{
	FQuat TangoToUnrealRotationQuat(ToConvert.orientation[2],
		-ToConvert.orientation[0],
		-ToConvert.orientation[1],
		ToConvert.orientation[3]);

	FQuat ConvertedQuat = (FQuat(0.0, 0.7071, 0.0, 0.7071) * TangoToUnrealRotationQuat);

	FVector ConvertedFVector = WorldToMetersValue * FVector(ToConvert.translation[1], ToConvert.translation[0], ToConvert.translation[2]);

	FTangoPoseData Result;
	Result.Position = ConvertedFVector;
	Result.Rotation = FRotator(ConvertedQuat);
	Result.QuatRotation = ConvertedQuat;
	Result.Timestamp = ToConvert.timestamp;
	Result.FrameOfReference = FromCObject(ToConvert.frame);
	Result.StatusCode = (ETangoPoseStatus::Type)ToConvert.status_code;
	return Result;
}

static FTangoXYZijData FromCObject(TangoXYZij)
{
	//@TODO: This function
	return FTangoXYZijData();
}

static FTangoCameraIntrinsics FromCObject(TangoCameraIntrinsics ToConvert)
{

	FTangoCameraIntrinsics Result;

	//if(ToConvert != NULL)
	//{
	Result.CameraID = static_cast<TEnumAsByte<ETangoCameraType::Type>>((int)ToConvert.camera_id);

	Result.Width = static_cast<int32>(ToConvert.width);
	Result.Height = static_cast<int32> (ToConvert.height);

	Result.Fx = (float)ToConvert.fx;
	Result.Fy = (float)ToConvert.fy;
	Result.Cx = (float)ToConvert.cx;
	Result.Cy = (float)ToConvert.cy;

	//Initialize 5 elements to a value of '0'.
	Result.Distortion.Init(0, 5);
	//Check that these values definitely exist- try/catch block?
	if (ToConvert.distortion != nullptr)
	{
		Result.Distortion[0] = ToConvert.distortion[0];
		Result.Distortion[1] = ToConvert.distortion[1];
		Result.Distortion[2] = ToConvert.distortion[2];
		Result.Distortion[3] = ToConvert.distortion[3];
		Result.Distortion[4] = ToConvert.distortion[4];
	}
	else
	{
		UE_LOG(ProjectTangoPlugin, Warning, TEXT("FTangoCameraIntrinsics:FromCObject: 'result' array variable on C object was a null pointer "));
	}
	//}
	//else
	//{
	//	UE_LOG(ProjectTangoPlugin, Warning, TEXT("FTangoCameraIntrinsics::FromCObject: C object was null."));
	//}
	return Result;

}

static FTangoEvent FromCObject(const TangoEvent* Event)
{
	FTangoEvent UEvent;

	UEvent.Type = static_cast<ETangoEventType::Type>(Event->type);

	if (FString(Event->event_key).Equals(TEXT("TangoServiceException"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::KEY_SERVICE_EXCEPTION;
	}
	else if (FString(Event->event_key).Equals(TEXT("FisheyeOverExposed"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::DESCRIPTION_FISHEYE_OVER_EXPOSED;
	}
	else if (FString(Event->event_key).Equals(TEXT("FisheyeUnderExposed"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::DESCRIPTION_FISHEYE_UNDER_EXPOSED;
	}
	else if (FString(Event->event_key).Equals(TEXT("ColorOverExposed"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::DESCRIPTION_COLOR_OVER_EXPOSED;
	}
	else if (FString(Event->event_key).Equals(TEXT("ColorUnderExposed"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::DESCRIPTION_COLOR_UNDER_EXPOSED;
	}
	else if (FString(Event->event_key).Equals(TEXT("TooFewFeaturesTracked"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::DESCRIPTION_TOO_FEW_FEATURES;
	}
	else if (FString(Event->event_key).Equals(TEXT("AreaDescriptionSaveProgress"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::KEY_AREA_DESCRIPTION_SAVE_PROGRESS;
	}
	else if (FString(Event->event_key).Equals(TEXT("Unknown"), ESearchCase::IgnoreCase))
	{
		UEvent.Key = ETangoEventKeyType::UNKOWN;
	}
	else
	{
		UE_LOG(ProjectTangoPlugin, Warning, TEXT("UTangoDevice::OnTangoEvent: Unknown TangoEvent: %s %s"), *FString(Event->event_key), *FString(Event->event_value));
	}

	UEvent.Message = FString(Event->event_value);
	UEvent.TimeStamp = static_cast<float>(Event->timestamp);

	return UEvent;
}
#endif