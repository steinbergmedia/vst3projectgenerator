cmake_minimum_required(VERSION 3.14.0)

set(SMTG_EFFECT_AUDIO_BUSSES_CODE_SNIPPET
    "addAudioInput (STR16 (\"Stereo In\"), Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput (STR16 (\"Stereo Out\"), Steinberg::Vst::SpeakerArr::kStereo);"
)

set(SMTG_EFFECT_EVENT_BUSSES_CODE_SNIPPET
    "addEventInput (STR16 (\"Event In\"), 1);"
)