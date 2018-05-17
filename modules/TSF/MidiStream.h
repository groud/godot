#define TSF_IMPLEMENTATION
#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h" 


class MidiStream : public AudioStream {
	GDCLASS(MidiStream, AudioStream);
private:
	friend class MidiStreamPlayback;
	uint64_t pos;
	int sample_rate;
	
	
public:
	char* sf_filename;
	float gain;
	int note;
	int vel;

	virtual Ref<AudioStreamPlayback> instance_playback();
	void set_output(enum TSFOutputMode outputmode, int samplerate, float global_gain_db);
    float* buffer;
	void render_to_buffer(void* data, float* buffer, int len);
	
	
};
