#include "pch.h"
#include "winmain.h"

#include <malloc.h>

#include "control.h"
#include "midi.h"
#include "options.h"
#include "pb.h"
#include "pinball.h"
#include "render.h"
#include "Sound.h"
#include "ndsfb_graphics.h"
#include "nds_input.h"
#include "dsi.h"
#include "mp3_shared.h"

#include "splash.h"

#include <fat.h>
#include <filesystem.h>

int winmain::bQuit = 0;
int winmain::activated;
int winmain::DispFrameRate = 0;
int winmain::DispGRhistory = 0;
int winmain::single_step = 0;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
int winmain::no_time_loss;

std::string winmain::DatFileName;
bool winmain::ShowSpriteViewer = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
std::string winmain::BasePath;
std::string winmain::FpsDetails;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;
optionsStruct &winmain::Options = options::Options;


int winmain::WinMain(LPCSTR lpCmdLine)
{
	std::set_new_handler(memalloc_failure);

	dsi::init();
	mp3_init();

	// Initialize graphics and input

	ndsfb_graphics::Initialize();

	// show splash image centered on screen, offset pos X,Y = 0,7
	for (int i = 0; i < splashBitmapLen/2; i++)
	{
		int start = (7) * 256 + 0;
		VRAM_A[start + i] = ((u16*)splashBitmap)[i];
	}

	if (!fatInitDefault())
		PrintFatalError("fatInitDefault() failed\nPlease check your SD card.\n");

	if (!nitroFSInit(0))
		PrintFatalError("nitroFSInit() failed\nMusic will not be available.");

	// Set the base path for PINBALL.DAT

	const char* dir = fatGetDefaultDrive();
	if (dir)
	{
		BasePath = dir;
		BasePath += "data/SpaceCadetPinball/";
	}
	else
		BasePath = "/data/SpaceCadetPinball/";

	pinball::quickFlag = 0; // strstr(lpCmdLine, "-quick") != nullptr;
	DatFileName = options::get_string("Pinball Data", pinball::get_rc_string(168, 0));

	// Check for full tilt .dat file and switch to it automatically

	auto cadetFilePath = pinball::make_path_name("CADET.DAT");
	auto cadetDat = fopen(cadetFilePath.c_str(), "r");
	if (cadetDat)
	{
		fclose(cadetDat);
		DatFileName = "CADET.DAT";
		pb::FullTiltMode = true;
	}

	// PB init from message handler

	{
		options::init();
		if (!Sound::Init(16, Options.Sounds))
			Options.Sounds = false;

		if (!pinball::quickFlag && !midi::music_init())
			Options.Music = false;

		if (pb::init())
		{
			PrintFatalError("Could not load game data:\n%s file is missing.\n", DatFileName.c_str());
		}
	}

	// Initialize game

	pb::reset_table();
	pb::firsttime_setup();

	ndsfb_graphics::AskRotationMode();
	ndsfb_graphics::SetSubScreenConsole(false);
	ndsfb_graphics::UpdateFull();

	nds_input::Initialize();
	nds_input::ScanPads();

	pb::replay_level(0);

	// Begin main loop

	bQuit = false;

	while (!bQuit)
	{
		// Input

		nds_input::ScanPads();

		if (nds_input::Exit())
			break;

		if (nds_input::Pause())
			pause();

		if (nds_input::NewGame())
			new_game();

		pb::keydown();
		pb::keyup();

		if (!single_step)
		{
			// Update game when not paused
			pb::frame(1000.0f / 60.0f);
		}

		// Copy game screen buffer to texture
		ndsfb_graphics::Update();

		ndsfb_graphics::SwapBuffers();

		mp3_fill_buffer();
	}

	ndsfb_graphics::SetSubScreenConsole(true);

	printf("Uninitializing...\n");

	end_pause();

	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	Sound::Close();

	printf("Finished uninitializing.\n");

	return 0;
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	char *caption = pinball::get_rc_string(170, 0);
	char *text = pinball::get_rc_string(179, 0);

	PrintFatalError("%s %s\n", caption, text);
}

void winmain::end_pause()
{
	if (single_step)
	{
		pb::pause_continue();
		no_time_loss = 1;
	}
}

void winmain::new_game()
{
	end_pause();
	pb::replay_level(0);
}

void winmain::pause()
{
	pb::pause_continue();
	no_time_loss = 1;
}

void winmain::UpdateFrameRate()
{
	// UPS >= FPS
	auto fps = Options.FramesPerSecond, ups = Options.UpdatesPerSecond;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}

void winmain::PrintFatalError(const char *message, ...)
{
	ndsfb_graphics::SetSubScreenConsole(true);

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);

	printf("\nPress A to exit.\n");

	while (true)
	{
		nds_input::ScanPads();

		if (nds_input::SkipError())
			break;

		ndsfb_graphics::SwapBuffers();
	}

	exit(EXIT_FAILURE);
}