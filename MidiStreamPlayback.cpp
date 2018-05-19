#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h"
#include "MidiStreamPlayback.h"
#define TSF_IMPLEMENTATION
#include "math/math_funcs.h"
#include "print_string.h"


MidiStreamPlayback::MidiStreamPlayback()
		: active(false) {
		AudioServer::get_singleton()->lock();
		pcm_buffer = AudioServer::get_singleton()->audio_data_alloc(PCM_BUFFER_SIZE);
		zeromem(pcm_buffer, PCM_BUFFER_SIZE);
		AudioServer::get_singleton()->unlock();
	}

MidiStreamPlayback::~MidiStreamPlayback() {
	if (pcm_buffer) {
		AudioServer::get_singleton()->audio_data_free(pcm_buffer);
		pcm_buffer = NULL;
	}
}

void MidiStreamPlayback::stop() {
	active = false;
	base->reset();
}


