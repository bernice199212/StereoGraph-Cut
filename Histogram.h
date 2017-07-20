#pragma once
using namespace std;

class Histogram
{
public:

	Histogram();
	Histogram(float max, float min, int binCount);

	void AddItem(float value);
	int GetValue(float value);

	int GetTotalValue();

	float GetProbability(float value);

	virtual ~Histogram();


private:

	float max;
	float min;
	float binRange;

	int totalValue;
	int *binValues;

	int binCount;

	

};