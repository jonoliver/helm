/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "modulation_slider.h"
#include "full_interface.h"
#include "synth_gui_interface.h"
#include "twytch_common.h"
//[/Headers]

#include "modulation_manager.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

#define FRAMES_PER_SECOND 30

//[/MiscUserDefs]

//==============================================================================
ModulationManager::ModulationManager (mopo::output_map modulation_sources, std::map<std::string, TwytchSlider*> sliders, mopo::output_map mono_modulations, mopo::output_map poly_modulations)
{

    //[UserPreSize]
    modulation_sources_ = modulation_sources;
    setInterceptsMouseClicks(false, true);
    startTimerHz(FRAMES_PER_SECOND);

    current_modulator_ = "";

    polyphonic_destinations_ = new Component();
    polyphonic_destinations_->setInterceptsMouseClicks(false, true);

    monophonic_destinations_ = new Component();
    monophonic_destinations_->setInterceptsMouseClicks(false, true);

    slider_model_lookup_ = sliders;
    for (auto slider : slider_model_lookup_) {
        std::string name = slider.first;
        const mopo::Processor::Output* mono_total = mono_modulations[name];
        const mopo::Processor::Output* poly_total = poly_modulations[name];

        // Create modulation meter.
        if (mono_total) {
            std::string name = slider.second->getName().toStdString();
            ModulationMeter* meter = new ModulationMeter(mono_total, poly_total, slider.second);
            addAndMakeVisible(meter);
            meter_lookup_[name] = meter;
            meter->setName(name);
            Rectangle<int> local_bounds = slider.second->getBoundsInParent();
            meter->setBounds(slider.second->getParentComponent()->localAreaToGlobal(local_bounds));
        }

        // Create modulation slider.
        ModulationSlider* mod_slider = new ModulationSlider(slider.second);
        mod_slider->setLookAndFeel(&look_and_feel_);
        mod_slider->addListener(this);
        if (poly_total)
            polyphonic_destinations_->addAndMakeVisible(mod_slider);
        else
            monophonic_destinations_->addAndMakeVisible(mod_slider);

        slider_lookup_[name] = mod_slider;
        owned_sliders_.push_back(mod_slider);
    }

    addAndMakeVisible(polyphonic_destinations_);
    addAndMakeVisible(monophonic_destinations_);
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    forgetModulator();
    //[/Constructor]
}

ModulationManager::~ModulationManager()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    for (auto meter : meter_lookup_)
        delete meter.second;
    for (Slider* slider : owned_sliders_)
        delete slider;
    //[/Destructor]
}

//==============================================================================
void ModulationManager::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ModulationManager::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    polyphonic_destinations_->setBounds(getBounds());
    monophonic_destinations_->setBounds(getBounds());
    for (auto slider : slider_lookup_) {
        TwytchSlider* model = slider_model_lookup_[slider.first];
        Point<int> global_top_left = model->localPointToGlobal(Point<int>(0, 0));
        slider.second->setVisible(model->isVisible());
        slider.second->setBounds(global_top_left.x, global_top_left.y,
                                 model->getWidth(), model->getHeight());
    }

    for (auto meter : meter_lookup_) {
        Slider* model = slider_model_lookup_[meter.first];
        meter.second->setBounds(model->getParentComponent()->localAreaToGlobal(model->getBounds()));
        meter.second->setVisible(model->isVisible());
    }
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void ModulationManager::timerCallback() {
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    if (parent == nullptr)
        return;

    int num_voices = parent->getNumActiveVoices();

    for (auto slider : slider_lookup_) {
        TwytchSlider* model = slider_model_lookup_[slider.first];
        slider.second->setVisible(model->isVisible());
    }

    for (auto meter : meter_lookup_) {
        meter.second->setVisible(slider_model_lookup_[meter.first]->isVisible());
        meter.second->update(num_voices);
    }
}

void ModulationManager::sliderValueChanged(Slider *slider) {
    std::string destination_name = slider->getName().toStdString();
    FullInterface* parent = findParentComponentOfClass<FullInterface>();
    if (parent)
        parent->modulationChanged(current_modulator_);

    setModulationAmount(current_modulator_, destination_name, slider->getValue());
}

void ModulationManager::setModulationAmount(std::string source, std::string destination,
                                            mopo::mopo_float amount) {
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    parent->changeModulationAmount(source, destination, amount);
}

void ModulationManager::forgetModulator() {
    polyphonic_destinations_->setVisible(false);
    monophonic_destinations_->setVisible(false);
    current_modulator_ = "";
}

void ModulationManager::changeModulator(std::string new_modulator) {
    current_modulator_ = new_modulator;
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    std::vector<mopo::ModulationConnection*> connections =
        parent->getSourceConnections(new_modulator);
    for (auto slider : slider_lookup_) {
        std::string destination_name = slider.second->getName().toStdString();
        float value = 0.0f;

        for (mopo::ModulationConnection* connection : connections) {
            if (connection->destination == destination_name) {
                value = connection->amount.value();
                break;
            }
        }
        slider.second->setValue(value);
    }

    polyphonic_destinations_->setVisible(true);
    bool source_is_poly = modulation_sources_[current_modulator_]->owner->isPolyphonic();
    monophonic_destinations_->setVisible(!source_is_poly);
}

void ModulationManager::showMeter(std::string name, bool show) {
    meter_lookup_[name]->setVisible(show);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ModulationManager" componentName=""
                 parentClasses="public Component, public SliderListener, public Timer"
                 constructorParams="mopo::output_map modulation_sources, std::map&lt;std::string, TwytchSlider*&gt; sliders, mopo::output_map mono_modulations, mopo::output_map poly_modulations"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
