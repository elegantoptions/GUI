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

*/

#include <stdio.h>
#include "FilterNode.h"
#include "Editors/FilterEditor.h"

FilterNode::FilterNode()
    : GenericProcessor("Bandpass Filter"), defaultLowCut(300.0f), defaultHighCut(6000.0f)

{

    // // Deprecated "parameters" class // //
    // Array<var> lowCutValues;
    // lowCutValues.add(1.0f);
    // lowCutValues.add(4.0f);
    // lowCutValues.add(100.0f);
    // lowCutValues.add(600.0f);

    // parameters.add(Parameter("low cut",lowCutValues, 3, 0));

    // Array<var> highCutValues;
    // highCutValues.add(12.0f);
    // highCutValues.add(3000.0f);
    // highCutValues.add(6000.0f);
    // highCutValues.add(9000.0f);

    // parameters.add(Parameter("high cut",highCutValues, 2, 1));

}

FilterNode::~FilterNode()
{

}

AudioProcessorEditor* FilterNode::createEditor()
{
    editor = new FilterEditor(this, true);

    FilterEditor* ed = (FilterEditor*) getEditor();
    ed->setDefaults(defaultLowCut, defaultHighCut);

    std::cout << "Creating editor." << std::endl;

    return editor;
}

// ----------------------------------------------------
// From the filter library documentation:
// ----------------------------------------------------
//
// each family of filters is given its own namespace
// RBJ: filters from the RBJ cookbook
// Butterworth
// ChebyshevI: ripple in the passband
// ChebyshevII: ripple in the stop band
// Elliptic: ripple in both the passband and stopband
// Bessel: theoretically with linear phase
// Legendre: "Optimum-L" filters with steepest transition and monotonic passband
// Custom: Simple filters that allow poles and zeros to be specified directly

// within each namespace exists a set of "raw filters"
// Butterworth::LowPass
//				HighPass
// 				BandPass
//				BandStop
//				LowShelf
// 				HighShelf
//				BandShelf
//
//	class templates (such as SimpleFilter) which require FilterClasses
//    expect an identifier of a raw filter
//  raw filters do not support introspection, or the Params style of changing
//    filter settings; they only offer a setup() function for updating the IIR
//    coefficients to a given set of parameters
//

// each filter family namespace also has the nested namespace "Design"
// here we have all of the raw filter names repeated, except these classes
//  also provide the Design interface, which adds introspection, polymorphism,
//  the Params style of changing filter settings, and in general all fo the features
//  necessary to interoperate with the Filter virtual base class and its derived classes

// available methods:
//
// filter->getKind()
// filter->getName()
// filter->getNumParams()
// filter->getParamInfo()
// filter->getDefaultParams()
// filter->getParams()
// filter->getParam()

// filter->setParam()
// filter->findParamId()
// filter->setParamById()
// filter->setParams()
// filter->copyParamsFrom()

// filter->getPoleZeros()
// filter->response()
// filter->getNumChannels()
// filter->reset()
// filter->process()

void FilterNode::updateSettings()
{

    if (getNumInputs() < 1024 && getNumInputs() != filters.size())
    {

        filters.clear();
        lowCuts.clear();
        highCuts.clear();

        for (int n = 0; n < getNumInputs(); n++)
        {

            // std::cout << "Creating filter number " << n << std::endl;

            filters.add(new Dsp::SmoothedFilterDesign
                        <Dsp::Butterworth::Design::BandPass 	// design type
                        <3>,								 	// order
                        1,										// number of channels (must be const)
                        Dsp::DirectFormII>						// realization
                        (1));


            //Parameter& p1 =  parameters.getReference(0);
            //p1.setValue(600.0f, n);
            //Parameter& p2 =  parameters.getReference(1);
            //p2.setValue(6000.0f, n);

            // restore defaults

            float lc, hc;

            if (lowCuts.size() > n)
            {
                lc = lowCuts[n];
                hc = highCuts[n];
            } else {
                lc = defaultLowCut;
                hc = defaultHighCut;
            }

            lowCuts.add(lc);
            highCuts.add(hc);

            setFilterParameters(lc, hc, n);
        }

    }

}

double FilterNode::getLowCutValueForChannel(int chan)
{
    return lowCuts[chan];
}

double FilterNode::getHighCutValueForChannel(int chan)
{
    return highCuts[chan];
}

void FilterNode::setFilterParameters(double lowCut, double highCut, int chan)
{

    Dsp::Params params;
    params[0] = getSampleRate(); // sample rate
    params[1] = 2; // order
    params[2] = (highCut + lowCut)/2; // center frequency
    params[3] = highCut - lowCut; // bandwidth

    if (filters.size() > chan)
        filters[chan]->setParams(params);

}

void FilterNode::setParameter(int parameterIndex, float newValue)
{

    if (newValue <= 0.01 || newValue >= 10000.0f)
        return;

    //std::cout << "Setting channel " << currentChannel;// << std::endl;

    if (parameterIndex == 0)
    {
       // std::cout << " low cut to " << newValue << std::endl;
        lowCuts.set(currentChannel,newValue);
    }
    else
    {
        //std::cout << " high cut to " << newValue << std::endl;
        highCuts.set(currentChannel,newValue);
    }

    //std::cout << newValue << std::endl;

    setFilterParameters(lowCuts[currentChannel],
                        highCuts[currentChannel],
                        currentChannel);


    // Deprecated code:
    //if (parameterIndex)
    //
    // Parameter& p =  parameters.getReference(parameterIndex);

    // p.setValue(newValue, currentChannel);

    // Parameter& p1 =  parameters.getReference(0);
    // Parameter& p2 =  parameters.getReference(1);

    // std::cout << float(p1[currentChannel]) << " ";
    // std::cout << float(p2[currentChannel]) << std::endl;

    // if (parameterIndex == 0) {
    // 	parameters[0].setValue(newValue, currentChannel);
    // 	setFilterParameters(newValue, parameters[0][currentChannel], currentChannel);
    // } else {
    // 	parameters[1].setValue(newValue, currentChannel);
    // 	setFilterParameters(lowCuts[currentChannel], newValue, currentChannel);
    // }

    editor->updateParameterButtons(parameterIndex);

}

void FilterNode::process(AudioSampleBuffer& buffer,
                         MidiBuffer& midiMessages,
                         int& nSamples)
{

    for (int n = 0; n < getNumOutputs(); n++)
    {
        float* ptr = buffer.getSampleData(n);
        filters[n]->process(nSamples, &ptr);
    }

}

void FilterNode::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, bool isEventChannel)
{

    //std::cout << "CHANNEL: " << channelNumber << std::endl;

    if (!isEventChannel && channelNumber > -1 && channelNumber < highCuts.size())
    {
        //std::cout << "Saving custom parameters for filter node." << std::endl;

        XmlElement* channelParams = channelInfo->createNewChildElement("PARAMETERS");
        channelParams->setAttribute("highcut",highCuts[channelNumber]);
        channelParams->setAttribute("lowcut",lowCuts[channelNumber]);
    }

}

void FilterNode::loadCustomChannelParametersFromXml(XmlElement* channelInfo, bool isEventChannel)
{

    int channelNum = channelInfo->getIntAttribute("number");

    if (!isEventChannel)
    {
        forEachXmlChildElement(*channelInfo, subNode)
        {
            if (subNode->hasTagName("PARAMETERS"))
            {
                highCuts.set(channelNum, subNode->getDoubleAttribute("highcut",defaultHighCut));
                lowCuts.set(channelNum, subNode->getDoubleAttribute("lowcut",defaultLowCut));

                setFilterParameters(lowCuts[channelNum],
                                    highCuts[channelNum],
                                    channelNum);

            }
        }
    }


}
