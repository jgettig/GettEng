#include "AudioDB.h"

#include <iostream>
#include <filesystem>

#include "Consts.h"

using std::cout;
using std::endl;
using std::string;
using std::array;
using std::unordered_map;

void AudioDB::init()
{
	if (initialized) {
		cout << "error: double AudioDB init call";
		exit(0);
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		cout << "Failed to initialize audio";
		exit(0);
	}

	if (AudioHelper::Mix_OpenAudio498(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
		MIX_DEFAULT_CHANNELS, 2048) < 0) {

		cout << "Failed to open audio";
		exit(0);
	}

	if (AudioHelper::Mix_AllocateChannels498(num_channels) < 0) {
		cout << "Failed to allocate audio channels";
		exit(0);
	}

	initialized = true;
}

void AudioDB::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized AudioDB";
		exit(0);
	}

	//would free chunks but won't bc of AG
	initialized = false;
}

void AudioDB::play_track(int channel, std::string name, bool does_loop)
{
	check_init();

	Mix_Chunk* chunk = get_chunk(name);

	int loops = 0;
	if (does_loop) loops = -1;

	if (AudioHelper::Mix_PlayChannel498(channel, chunk, loops) < 0) {
		cout << "Failed to play requested audio";
		exit(0);
	}
}

void AudioDB::stop_channel(int channel)
{
	check_init();

	if (channel < 0 || channel >= num_channels) return;

	AudioHelper::Mix_HaltChannel498(channel);
}

void AudioDB::set_volume(int channel, float _volume)
{
	int volume = static_cast<int>(_volume);
	AudioHelper::Mix_Volume498(channel, volume);
}

/* ------------------------ private ------------------------ */

Mix_Chunk* AudioDB::get_chunk(std::string name)
{
	if (tracks.count(name) != 0)
		return tracks[name];

	string fname = AUDIO_FOLDER_PATH + name;
	if (std::filesystem::exists(fname + ".wav"))
		fname += ".wav";
	else if (std::filesystem::exists(fname + ".ogg"))
		fname += ".ogg";
	else {
		cout << "error: failed to play audio clip " << name;
		exit(0);
	}

	Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV498(fname.c_str());
	
	tracks.emplace(name, chunk);

	return chunk;
}

void AudioDB::check_init()
{
	if (!initialized) {
		cout << "error: called AudioDB function before initializing";
		exit(0);
	}
}
