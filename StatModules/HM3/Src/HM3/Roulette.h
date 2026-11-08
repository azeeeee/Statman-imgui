// Blood Money roulette rules adapted from TheKotti's Hitman Roulette:
// https://github.com/TheKotti/thekotti.github.io/
// Original concept and rule data by TheKotti. This port reproduces the BM mission/extras logic
// from the original roulette.js for use by the Statman overlay.

#pragma once

#include <string>
#include <vector>

namespace Roulette
{
	enum class Color
	{
		Normal,
		Highlight,
		Blue,
		Green
	};

	struct Segment
	{
		std::string Text;
		Color Color = Color::Normal;
	};

	struct Line
	{
		std::vector<Segment> Segments;
		bool BlankBefore = false;
	};

	struct Result
	{
		std::string Mission;
		std::vector<Line> Lines;
		std::vector<Segment> Extras;

		bool Empty() const
		{
			return Mission.empty() && Lines.empty() && Extras.empty();
		}
	};

	const char* const* GetMissionNames();
	int GetMissionCount();
	const char* const* GetExtraNames();
	int GetExtraCount();

	Result Generate(int p_MissionIndex, int p_ExtrasIndex);
	std::string ToPlainText(const Result& p_Result);
}
