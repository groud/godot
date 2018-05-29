#define TSF_IMPLEMENTATION
#include "servers/audio/audio_stream.h"
#include "MidiStream.h"
#include "MidiStreamPlayback.h"



MidiStream::MidiStream(){
	sample_rate=44100;
	note = 50;
	vel = 50;
	sf_filename = "florestan-subset.sf2";
	TSFpointer;
}


Ref<AudioStreamPlayback> MidiStream::instance_playback() {
	Ref<MidiStreamPlayback> talking_tree;
	talking_tree.instance();
	talking_tree->base = Ref<MidiStream>(this);
	return talking_tree;
}

void MidiStream::set_position(uint64_t p) {
	pos = p;
}

void MidiStream::set_filename(char* filename) {
	filename = sf_filename;
	
}

void MidiStream::buffer_function(float* b){
	 b = buffer;
	 tsf_render_float(TSFpointer, b, sample_rate, 0);
}

void MidiStream::set_output(enum TSFOutputMode outputmode, int samplerate, float global_gain_db) {
	samplerate = sample_rate;
	global_gain_db = gain;
	tsf_set_output(TSFpointer, TSF_STEREO_INTERLEAVED, sample_rate, gain);
}

void MidiStream::note_on(int n, int v)
{
	n = note;
	v = vel;
	tsf_note_on(TSFpointer, 0, n, v);
}

void MidiStream::reset() {
	set_position(0);
}


void MidiStream::_bind_methods() {
	ClassDB::bind_method(D_METHOD("reset"), &MidiStream::reset);
	ClassDB::bind_method(D_METHOD("get_stream_name"), &MidiStream::get_stream_name);
	ClassDB::bind_method(D_METHOD("note_on"), &MidiStream::note_on);
}


String MidiStream::get_stream_name() const {
	return "MidiStream";
}

