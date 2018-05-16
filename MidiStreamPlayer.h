#include "tsf.h"
#include "reference.h"
#include "resource.h"
#include "servers/audio/audio_stream.h"

class MidiStreamPlayback : public AudioStreamPlayback {
	GDCLASS(MidiStreamPlayback, AudioStreamPlayback);
	friend class MidiStream;

private :

	enum {
		PCM_BUFFER_SIZE = 4096
	};
	enum {
		MIX_FRAC_BITS = 13,
		MIX_FRAC_LEN = (1 << MIX_FRAC_BITS),
		MIX_FRAC_MASK = MIX_FRAC_LEN - 1,
	};
	void * pcm_buffer;
	Ref<MidiStream> base;
	bool active;

public:

};