cmake_minimum_required(VERSION 3.14.0)

set(SMTG_EFFECT_AUDIO_BUSSES_CODE_SNIPPET
    "addAudioInput (STR16 (\"Stereo In\"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 (\"Stereo Out\"), Steinberg::Vst::SpeakerArr::kStereo);"
)

set(SMTG_EFFECT_EVENT_BUSSES_CODE_SNIPPET
    "/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 (\"Event In\"), 1);"
)
