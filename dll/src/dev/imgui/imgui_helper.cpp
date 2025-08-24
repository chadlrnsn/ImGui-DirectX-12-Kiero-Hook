#include "imgui_helper.h"

#include <sstream>


void ImGuiHelper::Draw3DBox(SDK::FVector& center, SDK::FVector& Bounds, SDK::APlayerController* Controller, ImColor color)
{
	const float wX = (center.X + Bounds.X) - (center.X - Bounds.X);
	const float wY = (center.Y + Bounds.Y) - (center.Y - Bounds.Y);

	SDK::FVector Top{ center.X, center.Y, center.Z + Bounds.Z };
	SDK::FVector Bottom{ center.X, center.Y, center.Z - Bounds.Z };

	SDK::FVector a1 = { center.X - wX / 2, center.Y + wY / 2, Bottom.Z };
	SDK::FVector a2 = { center.X + wX / 2, center.Y + wY / 2, Bottom.Z };
	SDK::FVector a3 = { center.X - wX / 2, center.Y - wY / 2, Bottom.Z };
	SDK::FVector a4 = { center.X + wX / 2, center.Y - wY / 2, Bottom.Z };

	SDK::FVector b1 = { center.X - wX / 2, center.Y + wY / 2, Top.Z };
	SDK::FVector b2 = { center.X + wX / 2, center.Y + wY / 2, Top.Z };
	SDK::FVector b3 = { center.X - wX / 2, center.Y - wY / 2, Top.Z };
	SDK::FVector b4 = { center.X + wX / 2, center.Y - wY / 2, Top.Z };

	SDK::FVector2D a1w2sUE{};
	SDK::FVector2D a2w2sUE{};
	SDK::FVector2D a3w2sUE{};
	SDK::FVector2D a4w2sUE{};

	SDK::FVector2D b1w2sUE{};
	SDK::FVector2D b2w2sUE{};
	SDK::FVector2D b3w2sUE{};
	SDK::FVector2D b4w2sUE{};

	if (Controller->ProjectWorldLocationToScreen(a1, &a1w2sUE, false) && Controller->ProjectWorldLocationToScreen(a2, &a2w2sUE, false)
		&& Controller->ProjectWorldLocationToScreen(a3, &a3w2sUE, false) && Controller->ProjectWorldLocationToScreen(a4, &a4w2sUE, false)
		&& Controller->ProjectWorldLocationToScreen(b1, &b1w2sUE, false) && Controller->ProjectWorldLocationToScreen(b2, &b2w2sUE, false)
		&& Controller->ProjectWorldLocationToScreen(b3, &b3w2sUE, false) && Controller->ProjectWorldLocationToScreen(b4, &b4w2sUE, false))
	{

		ImVec2 a1w2s = ImVec2(a1w2sUE.X, a1w2sUE.Y);
		ImVec2 a2w2s = ImVec2(a2w2sUE.X, a2w2sUE.Y);
		ImVec2 a3w2s = ImVec2(a3w2sUE.X, a3w2sUE.Y);
		ImVec2 a4w2s = ImVec2(a4w2sUE.X, a4w2sUE.Y);

		ImVec2 b1w2s = ImVec2(b1w2sUE.X, b1w2sUE.Y);
		ImVec2 b2w2s = ImVec2(b2w2sUE.X, b2w2sUE.Y);
		ImVec2 b3w2s = ImVec2(b3w2sUE.X, b3w2sUE.Y);
		ImVec2 b4w2s = ImVec2(b4w2sUE.X, b4w2sUE.Y);

		ImGui::GetBackgroundDrawList()->AddLine(a1w2s, a2w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a2w2s, a4w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a4w2s, a3w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a3w2s, a1w2s, color, 1.f);
															  
		ImGui::GetBackgroundDrawList()->AddLine(b1w2s, b2w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(b2w2s, b4w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(b4w2s, b3w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(b3w2s, b1w2s, color, 1.f);
															  
		ImGui::GetBackgroundDrawList()->AddLine(a1w2s, b1w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a2w2s, b2w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a3w2s, b3w2s, color, 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(a4w2s, b4w2s, color, 1.f);
	}
}

void ImGuiHelper::HelpMarker(const char* desc)
{
	ImGui::TextDisabled("[?]");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ImGuiHelper::AddText(ImVec2 pos, char* text, ImColor color)
{
	auto DrawList = ImGui::GetForegroundDrawList();
	auto wText = text;

	auto Size = ImGui::CalcTextSize(wText);
	pos.x -= Size.x / 2.f;
	pos.y -= Size.y / 2.f;

	//	ImGui::PushFont(m_font);

	DrawList->AddText(ImVec2(pos.x + 1, pos.y + 1), ImColor(0, 0, 0, 255), wText);
	DrawList->AddText(ImVec2(pos.x, pos.y), color, wText);

	//	ImGui::PopFont();
}

float ImGuiHelper::DrawOutlinedText(ImFont* pFont, const ImVec2& pos, float size, ImU32 color, bool center, const char* text, ...)
{
	va_list(args);
	va_start(args, text);

	CHAR wbuffer[256] = { };
	vsprintf_s(wbuffer, text, args);

	va_end(args);

	auto DrawList = ImGui::GetBackgroundDrawList();

	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, wbuffer);

		if (center)
		{
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}
		else
		{
			DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

float ImGuiHelper::DrawOutlinedTextForeground(ImFont* pFont, const ImVec2& pos, float size, ImU32 color, bool center, const char* text, ...)
{
	va_list(args);
	va_start(args, text);

	CHAR wbuffer[256] = { };
	vsprintf_s(wbuffer, text, args);

	va_end(args);

	auto DrawList = ImGui::GetForegroundDrawList();

	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, wbuffer);

		if (center)
		{
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}
		else
		{
			DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);
			DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), wbuffer);

			DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), wbuffer);
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

void ImGuiHelper::RectFilled(float x0, float y0, float x1, float y1, ImColor color, float rounding, int rounding_corners_flags)
{
	auto vList = ImGui::GetBackgroundDrawList();
	vList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
}

void ImGuiHelper::HealthBar(float x, float y, float w, float h, int phealth, ImColor col)
{
	auto vList = ImGui::GetBackgroundDrawList();

	int healthValue = max(0, min(phealth, 100));

	int barColor = ImColor(min(510 * (100 - healthValue) / 100, 255), min(510 * healthValue / 100, 255), 25, 255);
	vList->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), col);
	RectFilled(x, y, x + w, y + (((float)h / 100.0f) * (float)phealth), barColor, 0.0f, 0);
}