/*
  ==============================================================================

    PhaseDetectJP.cpp
    Created: 20 Nov 2015 1:25:59pm
    Author:  Janos Perge, AsaadLab

  ==============================================================================

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

*/


#include <stdio.h>
#include "PhaseDetectJP.h"
#include "PhaseDetectJPEditor.h"

PhaseDetectJP::PhaseDetectJP()
	: GenericProcessor("Detect Phase JP"), activeModule(-1),
	risingPos(false), risingNeg(false), fallingPos(false), fallingNeg(false)

{

}


PhaseDetectJP::~PhaseDetectJP()
{

}

AudioProcessorEditor* PhaseDetectJP::createEditor()
{
	editor = new PhaseDetectJPEditor(this, true);

	std::cout << "Creating editor." << std::endl;

	return editor;
}

void PhaseDetectJP::addModule()
{
	DetectorModule m = DetectorModule();
	m.inputChan = -1;
	m.outputChan = -1;
	m.gateChan = -1;
	m.isActive = true;
	m.lastSample = 0.0f;
	m.type = NONE;
	m.samplesSinceTrigger = 5000;
	m.wasTriggered = false;
	m.phase = NO_PHASE;

	modules.add(m);
}

void PhaseDetectJP::setActiveModule(int i)
{
	activeModule = i;

}


void PhaseDetectJP::setParameter(int parameterIndex, float newValue)
{

	DetectorModule& module = modules.getReference(activeModule);

	if (parameterIndex == 1) // module type
	{

		int val = (int)newValue;

		switch (val)
		{
		case 0:
			module.type = NONE;
			break;
		case 1:
			module.type = PEAK;
			break;
		case 2:
			module.type = FALLING_ZERO;
			break;
		case 3:
			module.type = TROUGH;
			break;
		case 4:
			module.type = RISING_ZERO;
			break;
		default:
			module.type = NONE;
		}
	}
	else if (parameterIndex == 2)   // inputChan
	{
		module.inputChan = (int)newValue;
	}
	else if (parameterIndex == 3)   // outputChan
	{
		module.outputChan = (int)newValue;
	}
	else if (parameterIndex == 4)   // gateChan
	{
		module.gateChan = (int)newValue;
		if (module.gateChan < 0)
		{
			module.isActive = true;
		}
		else
		{
			module.isActive = false;
		}
	}

}

void PhaseDetectJP::updateSettings()
{

}

bool PhaseDetectJP::enable()
{
	return true;
}

void PhaseDetectJP::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
	// MOVED GATING TO PULSE PAL OUTPUT!
	// now use to randomize phase for next trial

	//std::cout << "GOT EVENT." << std::endl;

	if (eventType == TTL)
	{
		const uint8* dataptr = event.getRawData();

		// int eventNodeId = *(dataptr+1);
		int eventId = *(dataptr + 2);
		int eventChannel = *(dataptr + 3);

		for (int i = 0; i < modules.size(); i++)
		{
			DetectorModule& module = modules.getReference(i);

			if (module.gateChan == eventChannel)
			{
				if (eventId)
					module.isActive = true;
				else
					module.isActive = false;
			}
		}

	}

}

void PhaseDetectJP::process(AudioSampleBuffer& buffer,
	MidiBuffer& events)
{
	int pulseDurationSamp;
	int stimThresholduV;

	pulseDurationSamp = 1000; //for a 30kHz sampling rate, divide this by 3 to get it in ms
	stimThresholduV = -2000;   //stimulation threshold in microVolts

	checkForEvents(events);

	// loop through the modules
	for (int i = 0; i < modules.size(); i++)
	{
		DetectorModule& module = modules.getReference(i);

		// check to see if it's active and has a channel
		if (module.isActive && module.outputChan >= 0 &&
			module.inputChan >= 0 &&
			module.inputChan < buffer.getNumChannels())
		{
			for (int i = 0; i < getNumSamples(module.inputChan); i++)
			{
				const float sample = *buffer.getReadPointer(module.inputChan, i);

				if (sample < module.lastSample && sample > 0 && module.phase != FALLING_POS)
				{

					if (module.type == PEAK)
					{
						addEvent(events, TTL, i, 1, module.outputChan);
						module.samplesSinceTrigger = 0;
						module.wasTriggered = true;
					}

					module.phase = FALLING_POS;

				}
				else if (sample < stimThresholduV && module.lastSample >= stimThresholduV && module.phase != FALLING_NEG)
				{

					if (module.type == FALLING_ZERO)
					{
						addEvent(events, TTL, i, 1, module.outputChan);
						module.samplesSinceTrigger = 0;
						module.wasTriggered = true;
					}

					module.phase = FALLING_NEG;

				}
				else if (sample > module.lastSample && sample < 0 && module.phase != RISING_NEG)
				{

					if (module.type == TROUGH)
					{
						addEvent(events, TTL, i, 1, module.outputChan);
						module.samplesSinceTrigger = 0;
						module.wasTriggered = true;
					}

					module.phase = RISING_NEG;

				}
				else if (sample > 0 && module.lastSample <= 0 && module.phase != RISING_POS)
				{

					if (module.type == RISING_ZERO)
					{
						addEvent(events, TTL, i, 1, module.outputChan);
						module.samplesSinceTrigger = 0;
						module.wasTriggered = true;
					}

					module.phase = RISING_POS;

				}

				module.lastSample = sample;

				if (module.wasTriggered)
				{
					// turn off stimulation pulse X samples after the trigger
					if (module.samplesSinceTrigger > pulseDurationSamp)
					{
						addEvent(events, TTL, i, 0, module.outputChan);
						module.wasTriggered = false;
					}
					else
					{
						module.samplesSinceTrigger++;
					}
				}

			}


		}

	}

}

void PhaseDetectJP::estimateFrequency()
{

	// int N = (numPeakIntervals < NUM_INTERVALS) ? numPeakIntervals
	//         : NUM_INTERVALS;

	// int sum = 0;

	// for (int i = 0; i < N; i++)
	// {
	//     sum += peakIntervals[i];
	// }

	// estimatedFrequency = getSampleRate()/(float(sum)/float(N));


}
