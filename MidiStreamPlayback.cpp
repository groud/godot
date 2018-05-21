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

void MidiStreamPlayback::seek(float p_time) {
	float max = get_length();
	if (p_time < 0) {
		p_time = 0;
	}
	base->set_position(uint64_t(p_time * base->mix_rate) << MIX_FRAC_BITS);
}

void MidiStreamPlayback::mix(AudioFrame *p_buffer, float p_rate, int p_frames) {
	ERR_FAIL_COND(!active);
	if (!active) {
		return;
	}
	zeromem(pcm_buffer, PCM_BUFFER_SIZE);
	float * buf = (float *)pcm_buffer;
	base->buffer_function(pcm_buffer);

	for (int i = 0; i < p_frames; i++) {
		float sample = float(buf[i]) / 32767.0;
		p_buffer[i] = AudioFrame(sample, sample);
	}
}

int MidiStreamPlayback::get_loop_count() const {
	return 0;
}

float MidiStreamPlayback::get_playback_position() const {
	return 0.0;
}

float MidiStreamPlayback::get_length() const {
	return 0.0;
}

bool MidiStreamPlayback::is_playing() const {
	return active;
}
