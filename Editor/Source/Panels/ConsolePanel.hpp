#pragma once

#include <cstdint>
#include <vector>

#include <imgui.h>

#include "../Layout/EditorPanel.hpp"
#include "Utils/Logger.hpp"

// ---------------------------------------------------------------------------
// Console for the engine log.
//
// Everything Logger::log/warning/error prints also lands in a bounded history
// (see Utils/Logger.hpp). The panel copies that history only when its revision
// moves, so an idle console costs nothing.
// ---------------------------------------------------------------------------
class ConsolePanel : public EditorPanel
{
public:
	const char* id() const override { return "Console"; }
	DockSlot slot() const override { return DockSlot::Bottom; }

	void draw() override
	{
		if (!ImGui::Begin(id(), &visible))
		{
			ImGui::End();
			return;
		}

		refreshEntries();
		drawToolbar();
		ImGui::Separator();
		drawEntries();

		ImGui::End();
	}

private:
	void refreshEntries()
	{
		const uint64_t revision = Logger::getRevision();
		if (revision == m_Revision)
		{
			return;
		}

		m_Revision = revision;
		m_Entries = Logger::getEntries();
		m_ScrollToBottom = m_Follow;
	}

	void drawToolbar()
	{
		ImGui::SetNextItemWidth(80.0f);
		ImGui::Checkbox("Log", &m_ShowLog);
		ImGui::SameLine();
		ImGui::Checkbox("Warning", &m_ShowWarning);
		ImGui::SameLine();
		ImGui::Checkbox("Error", &m_ShowError);
		ImGui::SameLine();
		ImGui::Checkbox("Follow", &m_Follow);
		ImGui::SameLine();

		if (ImGui::Button("Clear"))
		{
			Logger::clear();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("(%zu)", m_Entries.size());
	}

	void drawEntries()
	{
		// The horizontal scrollbar keeps long messages readable rather than
		// wrapping them into an unreadable block.
		ImGui::BeginChild("##console_entries", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

		for (const Logger::Entry& entry : m_Entries)
		{
			if (!isVisible(entry.level))
			{
				continue;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, colorFor(entry.level));
			ImGui::TextUnformatted(entry.message.c_str());
			ImGui::PopStyleColor();
		}

		if (m_ScrollToBottom)
		{
			ImGui::SetScrollHereY(1.0f);
			m_ScrollToBottom = false;
		}

		ImGui::EndChild();
	}

	bool isVisible(Logger::Level level) const
	{
		switch (level)
		{
			case Logger::Level::Warning: return m_ShowWarning;
			case Logger::Level::Error:   return m_ShowError;
			default:                     return m_ShowLog;
		}
	}

	static ImVec4 colorFor(Logger::Level level)
	{
		switch (level)
		{
			case Logger::Level::Warning: return ImVec4(0.95f, 0.78f, 0.28f, 1.0f);
			case Logger::Level::Error:   return ImVec4(0.94f, 0.40f, 0.40f, 1.0f);
			default:                     return ImGui::GetStyleColorVec4(ImGuiCol_Text);
		}
	}

	std::vector<Logger::Entry> m_Entries;
	uint64_t m_Revision{ 0 };
	bool m_ScrollToBottom{ false };

	bool m_ShowLog{ true };
	bool m_ShowWarning{ true };
	bool m_ShowError{ true };
	bool m_Follow{ true };
};
