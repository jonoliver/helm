/* Copyright 2013-2015 Matt Tytel
 *
 * twytch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * twytch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with twytch.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "twytch.h"
#include "twytch_editor.h"

TwytchEditor::TwytchEditor(Twytch& gb) : AudioProcessorEditor(&gb), glitch_bitch_(gb) {
  controls_ = glitch_bitch_.getSynth()->getControls();
  setLookAndFeel(&look_and_feel_);

  mopo::control_map::iterator iter = controls_.begin();
  for (; iter != controls_.end(); ++iter) {
    mopo::Control* control = iter->second;
    Slider* next_slider = new Slider(iter->first);
    next_slider->setRange(control->min(), control->max(), control->getIncrementSize());
    next_slider->setSliderStyle(Slider::LinearHorizontal);
    next_slider->setTextBoxStyle(Slider::TextBoxLeft, false, 80, 20);
    next_slider->setValue(control->current_value(), NotificationType::dontSendNotification);
    next_slider->addListener(this);
    // addAndMakeVisible(next_slider);

    sliders_[iter->first] = next_slider;
  }

  synth_editor_ = new SynthesisEditor();
  synth_editor_->addControls(controls_);
  addAndMakeVisible(synth_editor_);
  setSize(1000, 700);
}

TwytchEditor::~TwytchEditor() {
  std::map<std::string, Slider*>::iterator iter = sliders_.begin();
  for (; iter != sliders_.end(); ++iter)
    delete iter->second;
}

void TwytchEditor::paint(Graphics& g) {
  setSize(1000, 700);

  g.fillAll(Colours::white);
  g.setColour(Colours::black);
  g.setFont(15.0f);

  int x = 20;
  int y = 20;
  std::map<std::string, Slider*>::iterator iter = sliders_.begin();
  for (; iter != sliders_.end(); ++iter) {
    // g.drawText(iter->first, x, y, 100, 18, Justification::centredLeft);
    y+= 20;

    if (y > 600) {
      y = 20;
      x = 520;
    }
  }
}

void TwytchEditor::resized() {
  int x = 120;
  int y = 20;
  std::map<std::string, Slider*>::iterator iter = sliders_.begin();
  for (; iter != sliders_.end(); ++iter) {
    iter->second->setBounds(x, y, 360, 18);
    y+= 20;

    if (y > 600) {
      y = 20;
      x = 620;
    }
  }

  synth_editor_->setBounds(0, 0, getWidth(), getHeight());
}

void TwytchEditor::sliderValueChanged(Slider* edited_slider) {
  std::string name = edited_slider->getName().toStdString();
  controls_[name]->set(edited_slider->getValue());
}