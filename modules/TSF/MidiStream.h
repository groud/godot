#define TSF_IMPLEMENTATION
#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h" 


class MidiStream : public AudioStream {
	GDCLASS(MidiStream, AudioStream);
private:
	friend class MidiStreamPlayer;
	uint64_t pos;
	int sample_rate;
	int samples_count;
	
	
public:
	char* sf_filename;
	float gain;
	int note;
	int vel;
	int index;
	void set_position(uint64_t pos);
	virtual Ref<AudioStreamPlayback> instance_playback();
	static void AudioCallBack(void* data, short *stream, int len);
	short* buffer;
	
};
