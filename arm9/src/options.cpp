#include "pch.h"
#include "options.h"

#include "midi.h"
#include "Sound.h"
#include "winmain.h"

constexpr int options::MaxUps, options::MaxFps, options::MinUps, options::MinFps, options::DefUps, options::DefFps;
constexpr int options::MaxSoundChannels, options::MinSoundChannels, options::DefSoundChannels;

optionsStruct options::Options{};
std::map<std::string, std::string> options::settings{};
ControlsStruct options::RebindControls{};
const ControlRef* options::ControlWaitingForKey = nullptr;
const ControlRef options::Controls[6]
{
	{"Left Flipper", RebindControls.LeftFlipper},
	{"Right Flipper", RebindControls.RightFlipper},
	{"Left Table Bump", RebindControls.LeftTableBump},
	{"Right Table Bump", RebindControls.RightTableBump},
	{"Bottom Table Bump", RebindControls.BottomTableBump},
	{"Plunger", RebindControls.Plunger},
};


void options::init()
{
	Options.KeyDft.LeftFlipper = 0;
	Options.KeyDft.RightFlipper = 0;
	Options.KeyDft.Plunger = 0;
	Options.KeyDft.LeftTableBump = 0;
	Options.KeyDft.RightTableBump = 0;
	Options.KeyDft.BottomTableBump = 0;
	Options.Key = Options.KeyDft;

	Options.Sounds = get_int("Sounds", true);
	Options.Music = get_int("Music", true);
	Options.Players = get_int("Players", 1);
	Options.Key.LeftFlipper = get_int("Left Flipper key", Options.Key.LeftFlipper);
	Options.Key.RightFlipper = get_int("Right Flipper key", Options.Key.RightFlipper);
	Options.Key.Plunger = get_int("Plunger key", Options.Key.Plunger);
	Options.Key.LeftTableBump = get_int("Left Table Bump key", Options.Key.LeftTableBump);
	Options.Key.RightTableBump = get_int("Right Table Bump key", Options.Key.RightTableBump);
	Options.Key.BottomTableBump = get_int("Bottom Table Bump key", Options.Key.BottomTableBump);
	Options.UniformScaling = get_int("Uniform scaling", true);
	Options.LinearFiltering = get_int("Linear Filtering", true);
	Options.FramesPerSecond = std::min(MaxFps, std::max(MinUps, get_int("Frames Per Second", DefFps)));
	Options.UpdatesPerSecond = std::min(MaxUps, std::max(MinUps, get_int("Updates Per Second", DefUps)));
	Options.UpdatesPerSecond = std::max(Options.UpdatesPerSecond, Options.FramesPerSecond);
	Options.ShowMenu = get_int("ShowMenu", true);
	Options.UncappedUpdatesPerSecond = get_int("Uncapped Updates Per Second", false);
	Options.SoundChannels = get_int("Sound Channels", DefSoundChannels);
	Options.SoundChannels = std::min(MaxSoundChannels, std::max(MinSoundChannels, Options.SoundChannels));

	winmain::UpdateFrameRate();
}

void options::uninit()
{
	set_int("Sounds", Options.Sounds);
	set_int("Music", Options.Music);
	set_int("Players", Options.Players);
	set_int("Left Flipper key", Options.Key.LeftFlipper);
	set_int("Right Flipper key", Options.Key.RightFlipper);
	set_int("Plunger key", Options.Key.Plunger);
	set_int("Left Table Bump key", Options.Key.LeftTableBump);
	set_int("Right Table Bump key", Options.Key.RightTableBump);
	set_int("Bottom Table Bump key", Options.Key.BottomTableBump);
	set_int("Uniform scaling", Options.UniformScaling);
	set_int("Linear Filtering", Options.LinearFiltering);
	set_int("Frames Per Second", Options.FramesPerSecond);
	set_int("Updates Per Second", Options.UpdatesPerSecond);
	set_int("ShowMenu", Options.ShowMenu);
	set_int("Uncapped Updates Per Second", Options.UncappedUpdatesPerSecond);
	set_int("Sound Channels", Options.SoundChannels);
}


int options::get_int(LPCSTR lpValueName, int defaultValue)
{
	auto value = GetSetting(lpValueName, std::to_string(defaultValue));
	return std::stoi(value);
}

void options::set_int(LPCSTR lpValueName, int data)
{
	SetSetting(lpValueName, std::to_string(data));
}

std::string options::get_string(LPCSTR lpValueName, LPCSTR defaultValue)
{
	return GetSetting(lpValueName, defaultValue);
}

void options::set_string(LPCSTR lpValueName, LPCSTR value)
{
	SetSetting(lpValueName, value);
}

float options::get_float(LPCSTR lpValueName, float defaultValue)
{
	auto value = GetSetting(lpValueName, std::to_string(defaultValue));
	return std::stof(value);
}

void options::set_float(LPCSTR lpValueName, float data)
{
	SetSetting(lpValueName, std::to_string(data));
}


void options::toggle(Menu1 uIDCheckItem)
{
	switch (uIDCheckItem)
	{
	case Menu1::Sounds:
		Options.Sounds ^= true;
		Sound::Enable(Options.Sounds);
		return;
	case Menu1::Music:
		Options.Music ^= true;
		if (!Options.Music)
			midi::music_stop();
		else
			midi::play_pb_theme();
		return;
	case Menu1::Show_Menu:
		Options.ShowMenu = Options.ShowMenu == 0;
		return;
	case Menu1::OnePlayer:
	case Menu1::TwoPlayers:
	case Menu1::ThreePlayers:
	case Menu1::FourPlayers:
		Options.Players = static_cast<int>(uIDCheckItem) - static_cast<int>(Menu1::OnePlayer) + 1;
		break;
	case Menu1::WindowLinearFilter:
		Options.LinearFiltering ^= true;
		break;
	default:
		break;
	}
}

void options::KeyDown(int key)
{
	// if (ControlWaitingForKey)
	// {
	// 	// Skip function keys, just in case.
	// 	if (key < SDLK_F1 || key > SDLK_F12)
	// 	{
	// 		ControlWaitingForKey->Option = key;
	// 		ControlWaitingForKey = nullptr;
	// 	}
	// }
}

const std::string& options::GetSetting(const std::string& key, const std::string& value)
{
	auto setting = settings.find(key);
	if (setting == settings.end())
	{
		settings[key] = value;
		return value;
	}
	return setting->second;
}

void options::SetSetting(const std::string& key, const std::string& value)
{
	settings[key] = value;
}
