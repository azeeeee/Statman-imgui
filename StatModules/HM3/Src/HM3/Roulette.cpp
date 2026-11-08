// Blood Money roulette rules adapted from TheKotti's Hitman Roulette:
// https://github.com/TheKotti/thekotti.github.io/
// Original concept and rule data by TheKotti. Ported from the original roulette.js.

#include <HM3/Roulette.h>

#include <algorithm>
#include <iterator>
#include <random>
#include <utility>

namespace
{
	const char* s_MissionNames[] = {
		"Random Mission",
		"A Vintage Year",
		"Curtains Down",
		"Flatline",
		"A New Life",
		"The Murder of Crows",
		"You Better Watch Out...",
		"Death on the Mississippi",
		"Till Death Do Us Part",
		"A House of Cards",
		"A Dance with the Devil",
		"Amendment XXV"
	};

	const char* s_ExtraNames[] = {
		"Random extras",
		"No extras",
		"1 extra",
		"2 extras",
		"3 extras",
		"4 extras"
	};

	std::mt19937& GetRng()
	{
		static std::mt19937 s_Rng(std::random_device{}());
		return s_Rng;
	}

	int RandomIndex(int p_Count)
	{
		if (p_Count <= 1)
			return 0;

		std::uniform_int_distribution<int> s_Distribution(0, p_Count - 1);
		return s_Distribution(GetRng());
	}

	bool Roll(double p_Chance)
	{
		std::uniform_real_distribution<double> s_Distribution(0.0, 1.0);
		return s_Distribution(GetRng()) <= p_Chance;
	}

	template <typename T>
	const T& Pick(const std::vector<T>& p_Values)
	{
		return p_Values[RandomIndex(static_cast<int>(p_Values.size()))];
	}

	template <typename T>
	void Shuffle(std::vector<T>& p_Values)
	{
		std::shuffle(p_Values.begin(), p_Values.end(), GetRng());
	}

	void AddText(std::vector<Roulette::Segment>& p_Segments, std::string p_Text, Roulette::Color p_Color = Roulette::Color::Normal)
	{
		if (p_Text.empty())
			return;

		p_Segments.push_back({ std::move(p_Text), p_Color });
	}

	struct Draft
	{
		std::string Mission;
		std::vector<Roulette::Line> Lines;
		std::vector<Roulette::Segment> Extras;
	};

	void AddLine(Draft& p_Draft, std::vector<Roulette::Segment> p_Segments, bool p_BlankBefore = false)
	{
		Roulette::Line s_Line;
		s_Line.Segments = std::move(p_Segments);
		s_Line.BlankBefore = p_BlankBefore;
		p_Draft.Lines.push_back(std::move(s_Line));
	}

	void AddExtra(Draft& p_Draft, const char* p_Text, Roulette::Color p_Color)
	{
		p_Draft.Extras.push_back({ p_Text, p_Color });
	}

	void AddGenericExtras(Draft& p_Draft)
	{
		if (Roll(0.15))
			AddExtra(p_Draft, "Do not get shot at by the guards.", Roulette::Color::Blue);

		if (Roll(0.15))
			AddExtra(p_Draft, "Do not use gunshots or explosives as distractions.", Roulette::Color::Blue);

		if (Roll(0.15))
			AddExtra(p_Draft, "Do not use any unnecessary knockouts or accidents.", Roulette::Color::Blue);

		if (Roll(0.15))
			AddExtra(p_Draft, "Do not use any disguises.", Roulette::Color::Blue);
	}

	Draft MakeAVY()
	{
		static const std::vector<std::string> s_Entries = {
			"left from the beginning",
			"right from the beginning",
			"through the party to the cellar",
			"through the party to the main building"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity",
			"Falling object",
			"Water/Wine",
			"Kitchen knife",
			"Hammer",
			"Fire extinguisher",
			"Snub nosed (revolver)"
		};

		static const std::vector<std::string> s_Exits = {
			"cliffside path",
			"elevator"
		};

		Draft s_Draft;
		s_Draft.Mission = "A Vintage Year";

		std::vector<std::string> s_Targets = { "Manuel" };

		if (Roll(0.49))
			s_Targets.push_back("Don Fernando");
		else
			s_Targets.insert(s_Targets.begin(), "Don Fernando");

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Start by heading ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, " and kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Line));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Targets[i] + ": ");
			AddText(s_TargetLine, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine), i == 0);
		}

		std::vector<Roulette::Segment> s_ExitLine;
		AddText(s_ExitLine, "Exit using the ");
		AddText(s_ExitLine, Pick(s_Exits), Roulette::Color::Highlight);
		AddText(s_ExitLine, ".");
		AddLine(s_Draft, std::move(s_ExitLine), true);

		return s_Draft;
	}

	Draft MakeCD()
	{
		static const std::vector<std::string> s_Entries = {
			"left and down the stairs",
			"through the opera hall",
			"right of the hall and down the stairs"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP Mine",
			"Gun",
			"Gravity",
			"Falling object",
			"Hammer",
			"Screwdriver",
			"Real WWI pistol"
		};

		Draft s_Draft;
		s_Draft.Mission = "Curtains Down";

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Start by heading ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, " and kill the targets using the following methods.");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_DAlvade;
		AddText(s_DAlvade, "D'Alvade: ");
		AddText(s_DAlvade, Pick(s_Kills), Roulette::Color::Highlight);
		AddLine(s_Draft, std::move(s_DAlvade), true);

		std::vector<Roulette::Segment> s_Delahunt;
		AddText(s_Delahunt, "Delahunt: ");
		AddText(s_Delahunt, Pick(s_Kills), Roulette::Color::Highlight);
		AddLine(s_Draft, std::move(s_Delahunt));

		if (Roll(0.20))
			AddExtra(s_Draft, "Kill Delahunt first.", Roulette::Color::Highlight);

		if (Roll(0.20))
			AddExtra(s_Draft, "Retrieve the prop WWI pistol.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeFL()
	{
		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Pool",
			"Gravity",
			"Accidents"
		};

		static const std::vector<std::string> s_Entries = {
			"second floor",
			"front door",
			"security station",
			"gym"
		};

		struct Target
		{
			const char* Name;
			Roulette::Color Color;
		};

		std::vector<Target> s_Targets = {
			{ "Red target", Roulette::Color::Highlight },
			{ "Blue target", Roulette::Color::Blue },
			{ "Green target", Roulette::Color::Green }
		};

		Shuffle(s_Targets);

		Draft s_Draft;
		s_Draft.Mission = "Flatline";

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Enter through the ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, ".");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Header), true);

		for (const auto& s_Target : s_Targets)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Target.Name + std::string(": "), s_Target.Color);
			AddText(s_TargetLine, Pick(s_Kills));
			AddLine(s_Draft, std::move(s_TargetLine));
		}

		return s_Draft;
	}

	Draft MakeANL()
	{
		static const std::vector<std::string> s_Entries = {
			"garage",
			"basement",
			"front door",
			"backyard"
		};

		static const std::vector<std::string> s_Kills = {
			"kitchen knife",
			"fiber wire",
			"nailer",
			"RU-AP mine",
			"baseball bat",
			"gun",
			"glass ceiling",
			"car",
			"gravity",
			"swimming pool",
			"hedge cutter",
			"Bull .480"
		};

		static const std::vector<std::string> s_Exits = {
			"garage",
			"front door"
		};

		Draft s_Draft;
		s_Draft.Mission = "A New Life";

		std::vector<std::string> s_KillList = s_Kills;

		if (Roll(0.04))
			s_KillList.push_back("lethal dose of boredom that is already killing him from the inside");

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Enter through the ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, ".");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_KillLine;
		AddText(s_KillLine, "Kill Vinnie using the ");
		AddText(s_KillLine, Pick(s_KillList), Roulette::Color::Highlight);
		AddText(s_KillLine, ".");
		AddLine(s_Draft, std::move(s_KillLine), true);

		std::vector<Roulette::Segment> s_ExitLine;
		AddText(s_ExitLine, "Exit through the ");
		AddText(s_ExitLine, Pick(s_Exits), Roulette::Color::Highlight);
		AddText(s_ExitLine, ".");
		AddLine(s_Draft, std::move(s_ExitLine), true);

		if (Roll(0.14))
			AddExtra(s_Draft, "Vinnie's wife must survive.", Roulette::Color::Highlight);

		if (Roll(0.19))
			AddExtra(s_Draft, "The dog must survive.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeTMOC()
	{
		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity",
			"Kitchen knife",
			"Kazo TRG"
		};

		std::vector<std::string> s_Targets = {
			"Angelina",
			"Mark Puryah II",
			"Raymond"
		};

		// The original site never lets Raymond appear as the first target.
		for (int i = 0; i < 64 && s_Targets.front() == "Raymond"; ++i)
			Shuffle(s_Targets);

		if (s_Targets.front() == "Raymond")
			std::rotate(s_Targets.begin(), s_Targets.begin() + 1, s_Targets.end());

		Draft s_Draft;
		s_Draft.Mission = "The Murder of Crows";

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Header));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_Line;
			AddText(s_Line, s_Targets[i] + ": ");
			AddText(s_Line, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_Line), i == 0);
		}

		if (Roll(0.05))
			AddExtra(s_Draft, "Retrieve the SG552.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeYBWO()
	{
		static const std::vector<std::string> s_Entries = {
			"guest elevator",
			"staff elevator"
		};

		static const std::vector<std::string> s_TopFloor = {
			"elevator",
			"staircase"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity",
			"Jacuzzi/Water",
			"Kitchen knife"
		};

		Draft s_Draft;
		s_Draft.Mission = "You Better Watch Out...";

		std::vector<std::string> s_KillList = s_Kills;

		if (Roll(0.04))
			s_KillList.push_back("Old age");

		std::vector<std::string> s_Targets = { "Chad" };

		if (Roll(0.69))
			s_Targets.push_back("Lorne");
		else
			s_Targets.insert(s_Targets.begin(), "Lorne");

		const std::string s_Exit = Roll(0.69) ? "helicopter." : "boat.";

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "When going up, use the ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, " to reach the main floor and the ");
		AddText(s_Line, Pick(s_TopFloor), Roulette::Color::Highlight);
		AddText(s_Line, " to reach the top floor.");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Header));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Targets[i] + ": ");
			AddText(s_TargetLine, Pick(s_KillList), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine), i == 0);
		}

		std::vector<Roulette::Segment> s_ExitLine;
		AddText(s_ExitLine, "Exit using the ");
		AddText(s_ExitLine, s_Exit, Roulette::Color::Highlight);
		AddLine(s_Draft, std::move(s_ExitLine), true);

		if (Roll(0.19))
			AddExtra(s_Draft, "Kill the female assassin.", Roulette::Color::Highlight);

		if (Roll(0.19))
			AddExtra(s_Draft, "The dog must survive.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeDOTM()
	{
		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Kitchen knife",
			"Fire extinguisher",
			"Shovel",
			"Accidents",
			"Desert Eagle"
		};

		static const std::vector<std::string> s_Starts = {
			"to the engine room",
			"to the cabins"
		};

		Draft s_Draft;
		s_Draft.Mission = "Death on the Mississippi";

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Start by heading ");
		AddText(s_Line, Pick(s_Starts), Roulette::Color::Highlight);
		AddText(s_Line, ".");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following weapons:");
		AddLine(s_Draft, std::move(s_Header), true);

		std::vector<Roulette::Segment> s_Skip;
		AddText(s_Skip, "Skip: ");
		AddText(s_Skip, Pick(s_Kills), Roulette::Color::Highlight);
		AddLine(s_Draft, std::move(s_Skip), true);

		for (int i = 0; i < 6; ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, "Gang member: ");
			AddText(s_TargetLine, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine));
		}

		if (Roll(0.10))
			AddExtra(s_Draft, "Retrieve the FN-2000.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeTDDUP()
	{
		static const std::vector<std::string> s_Entries = {
			"left door",
			"front door",
			"right door"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity",
			"Swamp",
			"Shovel",
			"Chandelier",
			"Elephant Rifle"
		};

		static const std::vector<std::string> s_Exits = {
			"your own boat",
			"the priest's boat"
		};

		Draft s_Draft;
		s_Draft.Mission = "Till Death Do Us Part";

		std::vector<std::string> s_Targets = { "Buddy" };

		if (Roll(0.49))
			s_Targets.push_back("Pappy");
		else
			s_Targets.insert(s_Targets.begin(), "Pappy");

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Enter through the ");
		AddText(s_Line, Pick(s_Entries), Roulette::Color::Highlight);
		AddText(s_Line, " and kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Line));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Targets[i] + ": ");
			AddText(s_TargetLine, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine), i == 0);
		}

		std::vector<Roulette::Segment> s_ExitLine;
		AddText(s_ExitLine, "Exit using ");
		AddText(s_ExitLine, Pick(s_Exits), Roulette::Color::Highlight);
		AddText(s_ExitLine, ".");
		AddLine(s_Draft, std::move(s_ExitLine), true);

		if (Roll(0.30))
			AddExtra(s_Draft, "Never go to the second floor.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeAHOC()
	{
		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity"
		};

		std::vector<std::string> s_Targets = {
			"Schmutz",
			"The sheikh",
			"The scientist"
		};

		Shuffle(s_Targets);

		Draft s_Draft;
		s_Draft.Mission = "A House of Cards";

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Header));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_Line;
			AddText(s_Line, s_Targets[i] + ": ");
			AddText(s_Line, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_Line), i == 0);
		}

		if (Roll(0.15))
			AddExtra(s_Draft, "Never step into 7th or 8th floors.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeADWTD()
	{
		static const std::vector<std::string> s_Starts = {
			"Heaven party",
			"Hell party"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Accidents",
			"Meat Cleaver",
			"Stiletto",
			"Cane sword",
			"Dragunov Sniper"
		};

		static const std::vector<std::string> s_Targets = {
			"Martinez",
			"Vaana",
			"Maynard John",
			"Eve"
		};

		Draft s_Draft;
		s_Draft.Mission = "A Dance with the Devil";

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "Visit the ");
		AddText(s_Line, Pick(s_Starts), Roulette::Color::Highlight);
		AddText(s_Line, " first and kill the targets using the following weapons.");
		AddLine(s_Draft, std::move(s_Line));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Targets[i] + ": ");
			AddText(s_TargetLine, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine), i == 0);
		}

		if (Roll(0.10))
			AddExtra(s_Draft, "Steal the video tape.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeAXXV()
	{
		static const std::vector<std::string> s_FirstHalf = {
			"the roof",
			"the hallway"
		};

		static const std::vector<std::string> s_SecondHalf = {
			"the roof",
			"the courtyard"
		};

		static const std::vector<std::string> s_Kills = {
			"Fiber wire",
			"RU-AP mine",
			"Gun",
			"Gravity",
			"Kitchen knife",
			"Nailer",
			"Custom 1911"
		};

		Draft s_Draft;
		s_Draft.Mission = "Amendment XXV";

		std::vector<std::string> s_Targets = { "The Vice President" };

		if (Roll(0.79))
			s_Targets.push_back("Mark Parchezzi III");
		else
			s_Targets.insert(s_Targets.begin(), "Mark Parchezzi III");

		std::vector<Roulette::Segment> s_Line;
		AddText(s_Line, "When going to the main building, use ");
		AddText(s_Line, Pick(s_FirstHalf), Roulette::Color::Highlight);
		AddText(s_Line, " and when going to the West Wing, use ");
		AddText(s_Line, Pick(s_SecondHalf), Roulette::Color::Highlight);
		AddText(s_Line, ".");
		AddLine(s_Draft, std::move(s_Line));

		std::vector<Roulette::Segment> s_Header;
		AddText(s_Header, "Kill the targets using the following methods:");
		AddLine(s_Draft, std::move(s_Header));

		for (size_t i = 0; i < s_Targets.size(); ++i)
		{
			std::vector<Roulette::Segment> s_TargetLine;
			AddText(s_TargetLine, s_Targets[i] + ": ");
			AddText(s_TargetLine, Pick(s_Kills), Roulette::Color::Highlight);
			AddLine(s_Draft, std::move(s_TargetLine), i == 0);
		}

		std::vector<Roulette::Segment> s_ExitLine;
		AddText(s_ExitLine, "When returning to the main building, use ");
		AddText(s_ExitLine, Pick(s_SecondHalf), Roulette::Color::Highlight);
		AddText(s_ExitLine, " and when returning to the entrance, use ");
		AddText(s_ExitLine, Pick(s_FirstHalf), Roulette::Color::Highlight);
		AddText(s_ExitLine, ".");
		AddLine(s_Draft, std::move(s_ExitLine), true);

		if (Roll(0.05))
			AddExtra(s_Draft, "The fire alarm must not be triggered.", Roulette::Color::Highlight);

		if (Roll(0.10))
			AddExtra(s_Draft, "The dog must survive.", Roulette::Color::Highlight);

		return s_Draft;
	}

	Draft MakeMission(int p_MissionIndex)
	{
		switch (p_MissionIndex)
		{
		case 1: return MakeAVY();
		case 2: return MakeCD();
		case 3: return MakeFL();
		case 4: return MakeANL();
		case 5: return MakeTMOC();
		case 6: return MakeYBWO();
		case 7: return MakeDOTM();
		case 8: return MakeTDDUP();
		case 9: return MakeAHOC();
		case 10: return MakeADWTD();
		case 11: return MakeAXXV();
		default: return MakeAVY();
		}
	}

	Draft GenerateOnce(int p_MissionIndex)
	{
		if (p_MissionIndex <= 0)
			p_MissionIndex = 1 + RandomIndex(Roulette::GetMissionCount() - 1);

		Draft s_Draft = MakeMission(p_MissionIndex);

		// Generic extras are rolled after the mission-specific ones, matching the site's display order.
		AddGenericExtras(s_Draft);

		return s_Draft;
	}
}

const char* const* Roulette::GetMissionNames()
{
	return s_MissionNames;
}

int Roulette::GetMissionCount()
{
	return static_cast<int>(std::size(s_MissionNames));
}

const char* const* Roulette::GetExtraNames()
{
	return s_ExtraNames;
}

int Roulette::GetExtraCount()
{
	return static_cast<int>(std::size(s_ExtraNames));
}

Roulette::Result Roulette::Generate(int p_MissionIndex, int p_ExtrasIndex)
{
	if (p_MissionIndex < 0 || p_MissionIndex >= GetMissionCount())
		p_MissionIndex = 0;

	if (p_ExtrasIndex < 0 || p_ExtrasIndex >= GetExtraCount())
		p_ExtrasIndex = 0;

	// Extra index 0 means "random", 1 means "no extras", 2 means "1 extra", and so on.
	const int s_TargetExtras = p_ExtrasIndex <= 0 ? -1 : p_ExtrasIndex - 1;

	Draft s_Draft;
	const int s_Attempts = s_TargetExtras < 0 ? 1 : 20000;

	for (int i = 0; i < s_Attempts; ++i)
	{
		s_Draft = GenerateOnce(p_MissionIndex);

		if (s_TargetExtras < 0 || static_cast<int>(s_Draft.Extras.size()) == s_TargetExtras)
			break;
	}

	Result s_Result;
	s_Result.Mission = s_Draft.Mission;
	s_Result.Lines = std::move(s_Draft.Lines);
	s_Result.Extras = std::move(s_Draft.Extras);

	return s_Result;
}

std::string Roulette::ToPlainText(const Result& p_Result)
{
	std::string s_Text = p_Result.Mission;

	if (s_Text.empty())
		return s_Text;

	s_Text += "\n\n";

	for (const auto& s_Line : p_Result.Lines)
	{
		if (s_Line.BlankBefore)
			s_Text += "\n";

		for (const auto& s_Segment : s_Line.Segments)
			s_Text += s_Segment.Text;

		s_Text += "\n";
	}

	if (!p_Result.Extras.empty())
	{
		s_Text += "\nExtras:\n";

		for (const auto& s_Extra : p_Result.Extras)
			s_Text += "- " + s_Extra.Text + "\n";
	}

	return s_Text;
}
