#pragma once

constexpr uint32_t SwapByteOrderInt(uint32_t val)
{
	return (val >> 24) |
		((val << 8) & 0x00FF0000) |
		((val >> 8) & 0x0000FF00) |
		(val << 24);
}

constexpr uint16_t SwapByteOrderShort(uint16_t val)
{
	return static_cast<uint16_t>((val >> 8) | (val << 8));
}

#pragma pack(push)
#pragma pack(1)
struct riff_block
{
	uint32_t TkStart;
	uint32_t CbBuffer;
	char AData[4];
};

struct riff_data
{
	uint32_t Data;
	uint32_t DataSize;
	uint32_t BlocksPerChunk;
	riff_block Blocks[1];
};

struct riff_header
{
	uint32_t Riff;
	uint32_t FileSize;
	uint32_t Mids;
	uint32_t Fmt;
	uint32_t FmtSize;
	uint32_t dwTimeFormat;
	uint32_t cbMaxBuffer;
	uint32_t dwFlags;
	riff_data Data;
};

struct midi_event
{
	uint32_t iTicks;
	uint32_t iEvent;
};

struct midi_header
{
	explicit midi_header(uint16_t tickdiv)
		: tickdiv(tickdiv)
	{
	}

	const char MThd[4]{'M', 'T', 'h', 'd'};
	const uint32_t chunklen = SwapByteOrderInt(6);
	const int16_t format = SwapByteOrderShort(0);
	const uint16_t ntracks = SwapByteOrderShort(1);
	uint16_t tickdiv;
};

struct midi_track
{
	explicit midi_track(uint32_t chunklen)
		: chunklen(chunklen)
	{
	}

	const char MTrk[4]{'M', 'T', 'r', 'k'};
	uint32_t chunklen;
};

static_assert(sizeof(riff_block) == 0xC, "Wrong size of riff_block");
static_assert(sizeof(riff_data) == 0x18, "Wrong size of riff_data");
static_assert(sizeof(riff_header) == 0x38, "Wrong size of riff_header");
static_assert(sizeof(midi_event) == 8, "Wrong size of midi_event2");
static_assert(sizeof(midi_header) == 14, "Wrong size of midi_header");
static_assert(sizeof(midi_track) == 8, "Wrong size of midi_track");

#pragma pack(pop)

class midi
{
public:
	static int play_pb_theme();
	static int music_stop();
	static int music_init();
	static void music_shutdown();
private:
	static std::vector<void*> LoadedTracks;
	static void *track1, *track2, *track3, *active_track, *NextTrack;
	static bool SetNextTrackFlag;
	static void* load_track(std::string fileName);
	static bool play_track(void* midi);
	static std::vector<uint8_t>* MdsToMidi(std::string file);
};
