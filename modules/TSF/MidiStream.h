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
	void reset();
	void set_position(uint64_t pos);
	tsf * TSFpointer;
	char* sf_filename;
	void set_filename(char* filename);
	float gain;
	int note;
	int vel;
	void note_on(int n,int v);
	virtual Ref<AudioStreamPlayback> instance_playback();
	void set_output(enum TSFOutputMode outputmode, int samplerate, float global_gain_db);
    float* buffer;
	void buffer_function(float* b);
	MidiStream();
};
