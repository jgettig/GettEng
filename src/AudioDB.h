#ifndef AUDIO_DB_H
#define AUDIO_DB_H

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <utility>

#include "SDL2/SDL.h"
#include "AudioHelper.h"

class AudioDB
{
public:

	static void init();
	static void deinit();

	//plays track with name - returns channel on success, negative error code on fail
	static void play_track(int channel, std::string name, bool does_loop);

	//stops track playing on channel - returns 0 on success, negative error code on fail
	static void stop_channel(int channel);

	static void set_volume(int channel, float volume);

	static bool is_init() { return initialized; }

private:
	static inline const size_t num_channels = 50;

	//tracks chunks by name - never deallocated
	static inline std::unordered_map<std::string, Mix_Chunk*> tracks;

	static inline bool initialized = false;

	static Mix_Chunk* get_chunk(std::string name);

	static void check_init();
};

#endif