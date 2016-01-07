/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
==============================================================================

    PhaseDetectJPEditor.h
    Created: 23 Nov 2015 11:51:39am
    Author:  AsaadLab

  ==============================================================================
*/

#ifndef PHASEDETECTJPEDITOR_H_INCLUDED
#define PHASEDETECTJPEDITOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../Editors/GenericEditor.h"
#include "../Editors/ElectrodeButtons.h"

class DetectorInterfaceJP;
class PhaseDetectJP;
class ElectrodeButton;

/**

User interface for the PhaseDetect processor.

@see PhaseDetectJP

*/

class PhaseDetectJPEditor : public GenericEditor,
	public ComboBox::Listener
{
public:
	PhaseDetectJPEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);

	virtual ~PhaseDetectJPEditor();

	void buttonEvent(Button* button);

	void comboBoxChanged(ComboBox* c);

	void updateSettings();

	void saveCustomParameters(XmlElement* xml);
	void loadCustomParameters(XmlElement* xml);

private:

	ScopedPointer<ComboBox> detectorSelector;

	ScopedPointer<UtilityButton> plusButton;

	void addDetector();

	// ScopedPointer<ComboBox> inputChannelSelectionBox;
	// ScopedPointer<ComboBox> outputChannelSelectionBox;

	// ScopedPointer<Label> intputChannelLabel;
	// ScopedPointer<Label> outputChannelLabel;

	OwnedArray<DetectorInterfaceJP> interfaces;

	int previousChannelCount;

	Array<Colour> backgroundColours;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseDetectJPEditor);

};

class DetectorInterfaceJP : public Component,
	public ComboBox::Listener,
	public Button::Listener
{
public:
	DetectorInterfaceJP(PhaseDetectJP*, Colour, int);
	~DetectorInterfaceJP();

	void paint(Graphics& g);

	void comboBoxChanged(ComboBox*);
	void buttonClicked(Button*);

	void updateChannels(int);

	void setPhase(int);
	void setInputChan(int);
	void setOutputChan(int);
	void setGateChan(int);

	int getPhase();
	int getInputChan();
	int getOutputChan();
	int getGateChan();

private:

	Colour backgroundColour;

	Path sineWave;
	Font font;

	int idNum;

	PhaseDetectJP* processor;

	OwnedArray<ElectrodeButton> phaseButtons;

	ScopedPointer<ComboBox> inputSelector;
	ScopedPointer<ComboBox> gateSelector;
	ScopedPointer<ComboBox> outputSelector;

};



#endif  // PHASEDETECTJPEDITOR_H_INCLUDED
