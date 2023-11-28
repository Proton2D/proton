#pragma once

namespace proton
{
	class Random
	{
	public:
		static uint32_t Int(uint32_t min, uint32_t max);
		static float Float(float min, float max);
	};
}
