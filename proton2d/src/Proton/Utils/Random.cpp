#include "ptpch.h"

#include "Proton/Utils/Random.h"

#include <random>

namespace proton {

	uint32_t Random::Int(uint32_t min, uint32_t max)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(min, max);
		return dis(gen);
	}

	float Random::Float(float min, float max)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis((float)min, (float)max);
		return (float)dis(gen);
	}
}
