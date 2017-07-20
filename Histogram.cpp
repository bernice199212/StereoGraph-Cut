#include <iostream>
#include "StdAfx.h"
#include "Histogram.h"

Histogram::Histogram()
{
	min = 0; max = 0; binCount = 0; totalValue = 0; binValues = 0;
}

Histogram::Histogram(float _max, float _min, int _binCount)
{
	min = _min;
	max = _max;
	binCount = _binCount;
	binValues = new int[_binCount];

	memset(binValues, 0, sizeof(int)*_binCount);
	totalValue = 0;
	binRange = (max - min) / (float)_binCount;

}

Histogram::~Histogram()
{
	//free(binValues);
	binValues = NULL;

	min = 0; max = 0; binCount = 0; totalValue = 0; binValues = 0;
}

void Histogram::AddItem(float value)
{
	if (value < min || value > max)
	{
		return;
	}

	int binIndex = floorf(value / binRange);
	this->binValues[binIndex]++;
	totalValue++;
}

int Histogram::GetValue(float value)
{
	if (value < min || value > max)
	{
		return 0;
	}

	int binIndex = floorf(value / binRange);
	return this->binValues[binIndex];
}

int Histogram::GetTotalValue()
{
	return totalValue;
}

float Histogram::GetProbability(float value)
{
	if (value < min || value > max)
	{
		return 0;
	}

	int binIndex = floorf(value / binRange);
	int Value = this->binValues[binIndex];

	float prob = (float)Value / (float)totalValue;

	return prob;
}

