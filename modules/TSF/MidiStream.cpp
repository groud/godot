#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h"
#include "MidiStream.h"
#define TSF_IMPLEMENTATION

tsf * TSFpointer;

MidiStream::MidiStream()
	:sample_rate(44100), buffer(), 

{

};

Ref<AudioStreamPlayback> MidiStream::instance_playback() {
	Ref<MidiStreamPlayer> talking_tree;
	talking_tree.instance();
	talking_tree->base=Ref<MidiStream>(this);
	return talking_tree;
}
	
void MidiStream::set_position(uint64_t p)
{
	pos = p;
}

float MidiStream::get_length();
{
	return 
}



 void MidiStream::AudioCallBack(void* data, short* buffer, int len) {
	

}

