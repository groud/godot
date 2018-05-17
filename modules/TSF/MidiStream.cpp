#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h"
#include "MidiStream.h"
#define TSF_IMPLEMENTATION

tsf * TSFpointer;

MidiStream::MidiStream()
	:sample_rate(44100), gain(-10)

{

};

Ref<AudioStreamPlayback> MidiStream::instance_playback() {
	Ref<MidiStreamPlayback> talking_tree;
	talking_tree.instance();
	talking_tree->base=Ref<MidiStream>(this);
	return talking_tree;
}
void MidiStream::set_output(enum TSFOutputMode outputmode, int samplerate, float global_gain_db) {
	samplerate = sample_rate;
	global_gain_db = gain;
	tsf_set_output(TSFpointer, TSF_STEREO_INTERLEAVED, sample_rate, gain);
}


 void MidiStream::render_to_buffer(void* data, float* stream, int len){
	 
	 stream = buffer;
	 len = MidiStream::get_length();

	 tsf_render_float(TSFpointer, buffer, len, 0); //i swear this got errors when i tried it yesterday xD i am still not sure it would work though
}

