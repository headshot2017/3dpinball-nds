#pragma once

enum class Menu1:int
{
	New_Game = 101,
	About_Pinball = 102,
	High_Scores = 103,
	Exit = 105,
	Sounds = 201,
	Music = 202,
	Help_Topics = 301,
	Launch_Ball = 401,
	Pause_Resume_Game = 402,
	Full_Screen = 403,
	Demo = 404,
	Select_Table = 405,
	Player_Controls = 406,
	OnePlayer = 408,
	TwoPlayers = 409,
	ThreePlayers = 410,
	FourPlayers = 411,
	Show_Menu = 412,
	MaximumResolution = 500,
	R640x480 = 501,
	R800x600 = 502,
	R1024x768 = 503,
	WindowUniformScale = 600,
	WindowLinearFilter = 601,
};

struct ControlsStruct
{
	uint16_t LeftFlipper;
	uint16_t RightFlipper;
	uint16_t Plunger;
	uint16_t LeftTableBump;
	uint16_t RightTableBump;
	uint16_t BottomTableBump;
};

struct optionsStruct
{
	ControlsStruct Key;
	ControlsStruct KeyDft;
	bool Sounds;
	bool Music;
	int Players;
	bool UniformScaling;
	bool LinearFiltering;
	int FramesPerSecond;
	int UpdatesPerSecond;
	bool ShowMenu;
	bool UncappedUpdatesPerSecond;
	int SoundChannels;
};

struct ControlRef
{
	const char* Name;
	uint16_t& Option;
};


class options
{
public:
	// Original does ~120 updates per second.
	static constexpr int MaxUps = 360, MaxFps = MaxUps, MinUps = 60, MinFps = MinUps,
	                     DefUps = 120, DefFps = 60;
	// Original uses 8 sound channels
	static constexpr int MaxSoundChannels = 32, MinSoundChannels = 1, DefSoundChannels = 8;
	static optionsStruct Options;

	static void init();
	static void uninit();
	static int get_int(LPCSTR lpValueName, int defaultValue);
	static void set_int(LPCSTR lpValueName, int data);
	static std::string get_string(LPCSTR lpValueName, LPCSTR defaultValue);
	static void set_string(LPCSTR lpValueName, LPCSTR value);
	static float get_float(LPCSTR lpValueName, float defaultValue);
	static void set_float(LPCSTR lpValueName, float data);
	static void toggle(Menu1 uIDCheckItem);
	static void KeyDown(int key);
private:
	static std::map<std::string, std::string> settings;
	static ControlsStruct RebindControls;
	static const ControlRef Controls[6];
	static const ControlRef* ControlWaitingForKey;

	static const std::string& GetSetting(const std::string& key, const std::string& value);
	static void SetSetting(const std::string& key, const std::string& value);
};
