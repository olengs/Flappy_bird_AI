#pragma once
#include <vector>

struct NNode
{
	std::vector<float> weights;
	float output;
	float z;
	float alpha;
	//float bias;

	NNode() : output(0.f) , z(0.f) , alpha(0.f)/*, bias(0.f)*/{}
	~NNode() {}
};
