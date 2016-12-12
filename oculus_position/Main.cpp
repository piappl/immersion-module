#include <cstdlib>
#include <iostream>
#include <string>

#include <winsock2.h>
#include <windows.h>

#include <OVR_CAPI.h> 
#include <Extras/OVR_Math.h>

void sendMsg(char *buf, int len)
{
	SOCKET sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5005);
	addr.sin_addr.s_addr = inet_addr("192.168.230.70"); // Destination IP address

	sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));

	closesocket(sock);
}

void printHMDInfo(ovrHmd hmd)
{
	std::cout << std::string('-', 32) << std::endl;
	std::cout << "Opened HMD: " << hmd->ProductName << " " << hmd->ProductId << std::endl;
	std::cout << "Serial number: " << hmd->SerialNumber << std::endl;
	std::cout << "Resolution: " << hmd->Resolution.w << "x" << hmd->Resolution.h << std::endl;
	std::cout << std::string('-', 32) << std::endl;
}

int main(int argc, char **argv)
{
	// Initialize OVR
	if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
	{
		std::cout << "ERROR: Could not initialize OVR." << std::endl;
		return -1;
	}

	std::cout << "STATUS: OVR initialized." << std::endl;

	ovrHmd ovr_hmd = nullptr;
	if (ovrHmd_Create(0, &ovr_hmd) != ovrSuccess)
	{
		std::cout << "ERROR: Could not open any HMD." << std::endl;
		return -1;
	}

	printHMDInfo(ovr_hmd);

	// Start the sensor which provides the Rift’s pose and motion
	ovrHmd_ConfigureTracking(ovr_hmd, ovrTrackingCap_Orientation | 
		                              ovrTrackingCap_MagYawCorrection | 
									  ovrTrackingCap_Position, 0);
	
	bool initialized = false;
	float pitch0 = 0;
	float yaw0 = 0;
	float roll0 = 0;
	
	while(true)
	{
		// Query the HMD for the current tracking state 
		ovrTrackingState ts = ovrHmd_GetTrackingState(ovr_hmd, ovr_GetTimeInSeconds());

		float yaw = 0;
		float eye_pitch = 0;
		float eye_roll = 0;

		OVR::Posef pose = ts.HeadPose.ThePose;
		pose.Rotation.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &eye_pitch, &eye_roll);

		if (!initialized)
		{
			yaw0 = yaw;
			pitch0 = eye_pitch;
			roll0 = eye_roll;

			initialized = true;
		}

		char buf[1024];
		int len = sprintf((char*)buf, "{\"roll\": %f, \"pitch\": %f, \"yaw\":%f}", eye_roll - roll0, eye_pitch - pitch0, yaw - yaw0);
		sendMsg((char*)buf, len);
        
		std::cout << yaw << ", " << eye_pitch << ", " << eye_roll << std::endl;

		Sleep(40);
	}

	ovrHmd_Destroy(ovr_hmd);
	ovr_Shutdown();

	std::system("pause");
	return 0;
}