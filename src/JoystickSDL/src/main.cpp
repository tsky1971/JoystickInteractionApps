///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 
// 
// 
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>     //for using the function sleep
#include <stdio.h>
#include <time.h>
#include <map>
#include <list>
#include <array>
#include <future>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
using namespace std::literals;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// SDL2 Headers
#if defined(_WINDOWS)
	#define SDL_MAIN_HANDLED	
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_main.h>
#else
	#include <SDL3/SDL.h>
#endif

#ifdef _WIN32
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#else
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_BORDERLESS);
#endif

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_GLContext gl_context;

int g_NumJoysticks = 0;
SDL_JoystickID* g_pJoystickIDArray = nullptr;
char g_pszGUID[33];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"

ImGuiIO io;
bool show_demo_window = false;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char* HIDAPI_ConvertString(const wchar_t* wide_string)
{
	char* string = NULL;

	if (wide_string) {
		string = SDL_iconv_string("UTF-8", "WCHAR_T", (char*)wide_string, (SDL_wcslen(wide_string) + 1) * sizeof(wchar_t));
		if (string == NULL) {
			switch (sizeof(wchar_t)) {
			case 2:
				string = SDL_iconv_string("UTF-8", "UCS-2-INTERNAL", (char*)wide_string, (SDL_wcslen(wide_string) + 1) * sizeof(wchar_t));
				break;
			case 4:
				string = SDL_iconv_string("UTF-8", "UCS-4-INTERNAL", (char*)wide_string, (SDL_wcslen(wide_string) + 1) * sizeof(wchar_t));
				break;
			}
		}
	}
	return string;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EJoystickInputType : Uint8
{
	INPUTTYPE_UNKNOWN,
	INPUTTYPE_JOYSTICK,
	INPUTTYPE_GAMECONTROLLER,
};

enum class EJoystickPOVDirection : Uint8
{
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_UP_RIGHT,
	DIRECTION_RIGHT,
	DIRECTION_DOWN_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_DOWN_LEFT,
	DIRECTION_LEFT,
	DIRECTION_UP_LEFT,
};

struct SJoystickState
{	
	Sint16 Axes[16];
	Uint8 Buttons[64];
	EJoystickPOVDirection Hats[8];
};

struct SInputDevice_SDL
{
	Uint32 sdl_InstanceID;

	SDL_Joystick* sdl_pJoystickDevice;

	std::string DeviceName;

	Uint32 Axes;
	Uint32 Buttons;
	Uint32 Hats;

	SJoystickState CurrentState;
	SJoystickState StatePrevious;
};

std::map<Uint32, SInputDevice_SDL> InputDeviceMap;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Init()
{

	Uint32 result = 0;

	// Setup SDL
	// 
	// 
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	
	// all devices away SDL_SetHint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT, "0x06A3/0x0763");

	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif


	// Create window with SDL_Renderer graphics context
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	window = SDL_CreateWindow("windows", 1280, 720, window_flags);
	if (window == nullptr)
	{
		SDL_Log("Error: SDL_CreateWindow(): %s", SDL_GetError());
		return -1;
	}
	renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		SDL_Log("Error: SDL_CreateRenderer(): %s", SDL_GetError());
		return -1;
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	SDL_Log("Current SDL_Renderer: %s", info.name);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);


	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state	
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//{745a17a0 - 74d3 - 11d0 - b6fe - 00a0c90f57da}
	//"03000000341a00003608000000000000,PS3 Controller,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7",
	//char* mappingString = "745a17a074d311d0b6fe00a0c90f57da, Saitek Rudder Pro,a.x,a.y";
	//result = SDL_GameControllerAddMapping(mappingString);


	//std::cout << "search for HID devices... " << std::endl;
	//SDL_Log("search for HID devices... ");

	//// "0x06A3/0x0763" Saitek Pro Flight Rudder Pedals (USB)
	//SDL_hid_device_info* hidDeviceInfo = SDL_hid_enumerate(0x06A3, 0x0763);
	//if (hidDeviceInfo != nullptr) {		
	//	SDL_Log("found HID device: ");
	//	std::cout << "product_id " << hidDeviceInfo->product_id << std::endl;
	//	std::cout << "vendor_id " << hidDeviceInfo->vendor_id << std::endl;
	//	std::cout << "product_string " << hidDeviceInfo->product_string << std::endl;		
	//	std::cout << "manufacturer_string " << hidDeviceInfo->manufacturer_string << std::endl;
	//	
	//	SDL_hid_device* pHidDevice = SDL_hid_open(0x06A3, 0x0763, NULL);
	//	if (pHidDevice != nullptr) {
	//		SDL_Log("HID Device is open!");
	//		wchar_t manu[1024];
	//		SDL_hid_get_manufacturer_string(pHidDevice, manu, 256);
	//		
	//		
	//		SDL_Log("Manufactor: %s", HIDAPI_ConvertString(manu));
	//		std::cout << "manufacturer_string " << HIDAPI_ConvertString(manu) << std::endl;
	//	}
	//	else {
	//		SDL_Log("HID Device is NOT open!");
	//	}
	//}

	result = SDL_WasInit(SDL_INIT_JOYSTICK);

	g_pJoystickIDArray = SDL_GetJoysticks(&g_NumJoysticks);
	
	for (int deviceIndex = 0; deviceIndex < g_NumJoysticks; deviceIndex++) {

		SDL_JoystickID joystickInstanceID = g_pJoystickIDArray[deviceIndex];
		SDL_Log("Joystick deviceIndex=%d <-> JoystickId=%d", deviceIndex, joystickInstanceID);	

		std::cout << "JoystickInstanceName: " << SDL_GetJoystickInstanceName(joystickInstanceID) << std::endl;

		if (SDL_IsGamepad(joystickInstanceID)) {
			std::cout << "IsGamepad " << g_pszGUID << std::endl;
		}
		else
			if (SDL_IsJoystickVirtual(joystickInstanceID)) {
				std::cout << "SDL_IsJoystickVirtual " << g_pszGUID << std::endl;
			}
			else {
				std::cout << "unknown " << g_pszGUID << std::endl;
			}

		SDL_Joystick* pJoystickDevice = SDL_OpenJoystick(joystickInstanceID);		
		if (pJoystickDevice != nullptr) {
					
			SDL_JoystickGUID guid = SDL_GetJoystickGUID(pJoystickDevice);			
			SDL_GetJoystickGUIDString(guid, g_pszGUID, 33);
		
			std::cout << "Joystick: " << SDL_GetJoystickName(pJoystickDevice) << " GUID=" << g_pszGUID << std::endl;
			std::cout << "Joystick Name " << SDL_GetJoystickInstanceName(joystickInstanceID) << std::endl;
			std::cout << "--- Number of Axis " << SDL_GetNumJoystickAxes(pJoystickDevice) << std::endl;			
			std::cout << "--- Number of Buttons " << SDL_GetNumJoystickButtons(pJoystickDevice) << std::endl;
			std::cout << "--- Number of Hats " << SDL_GetNumJoystickHats(pJoystickDevice) << std::endl;

			SInputDevice_SDL &device = InputDeviceMap[joystickInstanceID];
			device.sdl_InstanceID = joystickInstanceID;
			device.sdl_pJoystickDevice = pJoystickDevice;
			device.DeviceName = SDL_GetJoystickName(pJoystickDevice);
			device.Axes = SDL_GetNumJoystickAxes(pJoystickDevice);			
			device.Buttons = SDL_GetNumJoystickButtons(pJoystickDevice);
			device.Hats = SDL_GetNumJoystickHats(pJoystickDevice);
		}

		//SDL_Gamepad* pGameDevice = SDL_OpenGamepad(deviceIndex);
		//if (pGameDevice != nullptr) {
		//	//SDL_SensorType sensorType;

		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_INVALID)) {
		//		std::cout << "SDL_SENSOR_INVALID" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_UNKNOWN)) {
		//		std::cout << "SDL_SENSOR_UNKNOWN" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_ACCEL)) {
		//		std::cout << "Accelerometer" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_GYRO)) {
		//		std::cout << "Gyroscope" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_ACCEL_L)) {
		//		std::cout << "Accelerometer for left Joy-Con controller and Wii nunchuk " << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_GYRO_L)) {
		//		std::cout << "Gyroscope for left Joy-Con controller" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_ACCEL_R)) {
		//		std::cout << " Accelerometer for right Joy-Con controller" << std::endl;
		//	}
		//	if (SDL_GamepadHasSensor(pGameDevice, SDL_SENSOR_GYRO_R)) {
		//		std::cout << "SDL_SENSOR_UNKNOWN" << std::endl;
		//	}
		//	
		//}
		std::cout << "--- deviceIndex " << deviceIndex << " END ---" << std::endl << std::endl;
	}

	return 0;
}

void Done()
{
	// Cleanup

	SDL_free(g_pJoystickIDArray);

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

}


void AppLoop()
{
	static float f = 0.0f;
	static int counter = 0;

	// Main loop
	bool done = false;
	while (!done)
	{
		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			SDL_Event event;

			SDL_PumpEvents();			

			while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) == 1)
			{
				ImGui_ImplSDL3_ProcessEvent(&event);

				if (event.type == SDL_EVENT_QUIT) {
					done = true;
				}
				switch (event.type)
				{
				case SDL_EVENT_JOYSTICK_ADDED:
					SDL_Log("Event ADD Joystick Device=%d", event.jdevice.which);
					break;
				case SDL_EVENT_GAMEPAD_ADDED:
				{
					SDL_Log("Event ADD Joystick/GameController Device=%d will be added. TESTING PHASE", event.gdevice.which);

					break;
				}
				case SDL_EVENT_JOYSTICK_REMOVED:
				{
					SDL_Log("Event REMOVE Joystick Device=%d will be removed", event.jdevice.which);
					break;
				}
				case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
				case SDL_EVENT_JOYSTICK_BUTTON_UP:
					SDL_Log("Event JoystickButton Device=%d Button=%d State=%d", event.jdevice.which, event.jbutton.button, event.jbutton.state);
					
					if (auto &search = InputDeviceMap.find(event.jdevice.which); search != InputDeviceMap.end()) {
						SInputDevice_SDL* inputDevice = &(search->second);
						Uint8 value = event.jbutton.state;
						inputDevice->CurrentState.Buttons[event.jbutton.button] = value;
					}

					break;
				case SDL_EVENT_JOYSTICK_AXIS_MOTION:
					//SDL_Log("Event JoystickAxis Device=%d Axis=%d Value=%d", event.jdevice.which, event.jaxis.axis, event.jaxis.value / (event.jaxis.value < 0 ? 32768.0f : 32767.0f));
					//SDL_Log("Event JoystickAxis Device=%d Axis=%d Value=%d", event.jdevice.which, event.jaxis.axis, event.jaxis.value);						
					
					if (auto& search = InputDeviceMap.find(event.jdevice.which); search != InputDeviceMap.end()) {
						SInputDevice_SDL *inputDevice = &(search->second);
						Sint16 value = event.jaxis.value;
						inputDevice->CurrentState.Axes[event.jaxis.axis] = value;
					}
					break;
				case SDL_EVENT_JOYSTICK_HAT_MOTION:
					//SDL_Log("Event JoystickHat Device=%d Hat=%d Value=%d", event.jdevice.which, event.jhat.hat, event.jhat.value);
					break;
				}
			}

			std::map<Uint32, SInputDevice_SDL>::iterator deviceIter = InputDeviceMap.begin();
			for (auto& [_instanceId, _inputDevice] : InputDeviceMap) {

				SInputDevice_SDL device = _inputDevice;
				ImGui::Begin(device.DeviceName.c_str(), &show_another_window);
				ImGui::Text("%s", device.DeviceName.c_str());
				ImGui::Text("InstanceID %d", device.sdl_InstanceID);				

				SDL_JoystickGUID guid = SDL_GetJoystickGUID(device.sdl_pJoystickDevice);
				SDL_GetJoystickGUIDString(guid, g_pszGUID, 33);
				ImGui::Text("GUID %s", g_pszGUID);

				ImGui::Text("Number of Axis %i", device.Axes);
				ImGui::Text("Number of Buttons %i", device.Buttons);
				ImGui::Text("Number of Hats %i", device.Hats);
				
				char axeName[16];
				int ivalue = 0;
				Uint8 bvalue = false;

				for (int idx = 0; idx < device.Axes; idx++) {
					ivalue = device.CurrentState.Axes[idx];

					//SDL_Log("JoystickAxis Device=%d Axis=%d Value=%i", device.sdl_InstanceID, idx, ivalue);

					sprintf(axeName, "Axis %f", idx);
					ImGui::SliderInt(axeName, &ivalue, -32768, 32768);
				}

				int x = 1;
				for (int idx = 0; idx < device.Buttons; idx++) {
					bvalue = device.CurrentState.Buttons[idx];

					//SDL_Log("JoystickAxis Device=%d Button=%d Value=%i", device.sdl_InstanceID, idx, bvalue);

					sprintf(axeName, "B %d", idx);
					if (bvalue == 1) {		
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.3f, 0.4f, 1.0f });
						if (ImGui::Button(axeName, ImVec2(50, 20))) {

						}
						
						if (x == 5) {
							x = 0;
						}
						else {
							ImGui::SameLine();
						}

						ImGui::PopStyleColor(1);
					}
					else {
						
						if (ImGui::Button(axeName, ImVec2(50, 30))) {

						}
						if (x == 5) {
							x = 0;
						}
						else {
							ImGui::SameLine();
						}
					}
					x++;
				}
				

				/*SDL_JoystickID joystickInstanceID = g_pJoystickIDArray[deviceIndex];

				SDL_Joystick* pJoystickDevice = SDL_OpenJoystick(joystickInstanceID);
				if (pJoystickDevice != nullptr) {
					std::string deviceName = SDL_GetJoystickName(pJoystickDevice);

					ImGui::Begin(deviceName.c_str(), &show_another_window);
					ImGui::Text("%s", SDL_GetJoystickName(pJoystickDevice));
					ImGui::Text("DeviceIndex %d", deviceIndex);
					ImGui::Text("InstanceID %d", joystickInstanceID);

					SDL_JoystickGUID guid = SDL_GetJoystickGUID(pJoystickDevice);
					SDL_GetJoystickGUIDString(guid, g_pszGUID, 33);

					int NumJoystickAxes = SDL_GetNumJoystickAxes(pJoystickDevice);
					int NumJoystickButtons = SDL_GetNumJoystickButtons(pJoystickDevice);
					int NumJoystickHats = SDL_GetNumJoystickHats(pJoystickDevice);

					ImGui::Text("GUID %s", g_pszGUID);
					ImGui::Text("Number of Axis %i", NumJoystickAxes);
					ImGui::Text("Number of Buttons %i", NumJoystickButtons);
					ImGui::Text("Number of Hats %i", NumJoystickHats);

					for (int na = 0; na < NumJoystickButtons; ++na) {
					}

					char axeName[16];

					for (int na = 0; na < NumJoystickAxes; ++na) {
						float axis = event.jaxis.axis;
						float value = event.jaxis.value;
						sprintf(axeName, "Axis %f", axis);
						ImGui::SliderFloat(axeName, &value, -32768, 32768);
					}			
				}
					*/

				ImGui::End();
				
			} // for
	
						
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
		
	}
}

/**
   *
   */
int main(void)
{
    Init();

	AppLoop();

    Done();

	return 0;
}
