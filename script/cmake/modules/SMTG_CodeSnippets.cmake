cmake_minimum_required(VERSION 3.14.0)

set(SMTG_EFFECT_AUDIO_BUSSES_CODE_SNIPPET
    "addAudioInput (STR16 (\"Stereo In\"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 (\"Stereo Out\"), Steinberg::Vst::SpeakerArr::kStereo);"
)

set(SMTG_EFFECT_EVENT_BUSSES_CODE_SNIPPET
    "/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 (\"Event In\"), 1);"
)

if(SMTG_ENABLE_VSTGUI_SUPPORT)
    set(SMTG_INCLUDE_VSTGUI_HEADER_CODE_SNIPPET
        "#include \"vstgui/plugin-bindings/vst3editor.h\""
    )
    set(SMTG_CREATE_VSTGUI_EDITOR_CODE_SNIPPET
        "// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, \"view\", \"${SMTG_PREFIX_FOR_FILENAMES}editor.uidesc\");
		return view;"
    )
else()
    set(SMTG_CREATE_VSTGUI_EDITOR_CODE_SNIPPET
        "// create your editor here and return a IPlugView ptr of it
        return nullptr;"
    )
endif()