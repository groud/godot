
#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h" 


class MidiStream : public AudioStream {
	GDCLASS(MidiStream, AudioStream)
	OBJ_SAVE_TYPE(AudioStream)

	friend class MidiStreamPlayback;
	uint64_t pos;
	int sample_rate;
	float gain;
	int note;
	int vel;
	char* sf_filename;
public:
	MidiStream();
	void reset();
	void set_position(uint64_t pos);
	tsf * TSFpointer;
	void set_filename(char* filename);
	void note_on(int n,int v);
	virtual Ref<AudioStreamPlayback> instance_playback();
	void set_output(enum TSFOutputMode outputmode, int samplerate, float global_gain_db);
    float* buffer;
	void buffer_function(float* b);
	virtual String get_stream_name() const;
	virtual float get_length() const { return 0; }

protected:
	static void _bind_methods();
};
