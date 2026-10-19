#pragma once

#include <Core.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <imgui.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "../Layout/EditorPanel.hpp"
#include "Utils/Logger.hpp"

// ---------------------------------------------------------------------------
// File browser for the project directory.
//
// Navigation is clamped to the working directory the editor was started in:
// "Up" stops at the root and folders are only entered from the listing, so the
// browser cannot wander off into the rest of the machine. The project files
// saved by File > Save live at the root, one step up from Resources.
// ---------------------------------------------------------------------------
class ContentBrowserPanel : public EditorPanel {
 public:
  ContentBrowserPanel() {
    std::error_code error;
    m_root = std::filesystem::current_path(error);
    if (error) {
      m_root = std::filesystem::path{"."};
    }

    // Assets are where you usually want to start, so open there when it
    // exists and fall back to the root.
    m_directory = m_root;
    const std::filesystem::path resources = m_root / "Resources";
    if (std::filesystem::is_directory(resources, error)) {
      m_directory = resources;
    }

    Refresh();
  }

  const char* Id() const override { return "Content Browser"; }
  DockSlot Slot() const override { return DockSlot::kLeft; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    DrawToolbar();
    ImGui::Separator();
    DrawEntryList();
    DrawDetails();

    ImGui::End();
  }

 private:
  void Refresh() {
    m_entries.clear();
    m_selected = -1;

    std::error_code error;
    std::filesystem::directory_iterator iterator{m_directory, error};
    if (error) {
      Logger::Error("Cannot list " + m_directory.string() + ": " + error.message());
      return;
    }

    for (const std::filesystem::directory_entry& entry : iterator) {
      m_entries.push_back(entry.path());
    }

    // Folders first, then names, case-insensitively: what a file browser is
    // expected to look like.
    std::sort(m_entries.begin(), m_entries.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
      std::error_code a_error, b_error;
      const bool a_is_directory = std::filesystem::is_directory(a, a_error);
      const bool b_is_directory = std::filesystem::is_directory(b, b_error);

      if (a_is_directory != b_is_directory) {
        return a_is_directory;
      }

      return ToLower(a.filename().string()) < ToLower(b.filename().string());
    });
  }

  void Navigate(const std::filesystem::path& directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
      return;
    }

    m_directory = directory;
    Refresh();
  }

  void DrawToolbar() {
    const bool at_root = m_directory == m_root;

    ImGui::BeginDisabled(at_root);
    if (ImGui::Button("Up")) {
      Navigate(m_directory.parent_path());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
      Refresh();
    }

    // Relative to the root, so the line stays short enough to read in a
    // narrow column. The root itself has no name to show.
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(m_directory, m_root, error);
    const std::string relative_name = relative.generic_string();
    const std::string label =
        error || relative_name.empty() || relative_name == "." ? std::string("/") : "/" + relative_name;
    ImGui::TextDisabled("%s", label.c_str());
  }

  void DrawEntryList() {
    // The listing takes most of the column; the rest is for the preview.
    const float list_height = ImGui::GetContentRegionAvail().y * 0.62f;

    if (!ImGui::BeginChild("##content_entries", ImVec2(0.0f, list_height), ImGuiChildFlags_Borders,
                           ImGuiWindowFlags_HorizontalScrollbar)) {
      ImGui::EndChild();
      return;
    }

    for (int index = 0; index < static_cast<int>(m_entries.size()); ++index) {
      const std::filesystem::path& path = m_entries[index];

      std::error_code error;
      const bool is_directory = std::filesystem::is_directory(path, error);
      const std::string name = path.filename().string();
      const std::string label = is_directory ? name + "/" : name;

      if (is_directory) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.62f, 0.80f, 0.98f, 1.0f));
      }

      const bool clicked = ImGui::Selectable(label.c_str(), index == m_selected, ImGuiSelectableFlags_AllowDoubleClick);

      if (is_directory) {
        ImGui::PopStyleColor();
      }

      if (clicked) {
        m_selected = index;

        if (is_directory && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          // Copied because navigate() refreshes the listing this
          // element lives in.
          const std::filesystem::path target = path;
          Navigate(target);
          ImGui::EndChild();
          return;
        }
      }
    }

    if (m_entries.empty()) {
      ImGui::TextDisabled("(empty)");
    }

    ImGui::EndChild();
  }

  void DrawDetails() {
    ImGui::Separator();

    if (m_selected < 0 || m_selected >= static_cast<int>(m_entries.size())) {
      ImGui::TextDisabled("Select a file to preview it.");
      return;
    }

    const std::filesystem::path& path = m_entries[m_selected];

    std::error_code error;
    const bool is_directory = std::filesystem::is_directory(path, error);

    ImGui::TextUnformatted(path.filename().string().c_str());

    if (is_directory) {
      ImGui::TextDisabled("folder");
      return;
    }

    ImGui::TextDisabled("%s", DescribeSize(path).c_str());

    if (!IsImage(path.extension().string())) {
      return;
    }

    rse::Texture* preview = PreviewFor(path);
    if (preview == nullptr || preview->GetWidth() == 0 || preview->GetHeight() == 0) {
      ImGui::TextDisabled("(no preview)");
      return;
    }

    const ImVec2 available = ImGui::GetContentRegionAvail();
    if (available.x <= 0.0f || available.y <= 0.0f) {
      return;
    }

    // Fit inside whatever room is left without distorting the image.
    const float aspect = static_cast<float>(preview->GetHeight()) / static_cast<float>(preview->GetWidth());
    float width = available.x;
    float height = width * aspect;
    if (height > available.y) {
      height = available.y;
      width = height / aspect;
    }

    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(preview->GetId())), ImVec2(width, height));
  }

  // Loads an image the first time it is selected and keeps it, so clicking
  // through a folder does not re-upload the same file every time.
  rse::Texture* PreviewFor(const std::filesystem::path& path) {
    const std::string key = path.string();

    const std::map<std::string, std::unique_ptr<rse::Texture>>::iterator found = m_previews.find(key);
    if (found != m_previews.end()) {
      return found->second.get();
    }

    std::unique_ptr<rse::Texture> preview = std::make_unique<rse::Texture>(key, key);
    rse::Texture* raw = preview.get();
    m_previews.emplace(key, std::move(preview));

    if (raw->GetWidth() == 0 || raw->GetHeight() == 0) {
      Logger::Warning("Could not preview " + key);
    }

    return raw;
  }

  static bool IsImage(const std::string& extension) {
    const std::string lowered = ToLower(extension);
    return lowered == ".png" || lowered == ".jpg" || lowered == ".jpeg" || lowered == ".bmp" || lowered == ".gif" ||
           lowered == ".tga" || lowered == ".webp" || lowered == ".avif" || lowered == ".qoi";
  }

  static std::string DescribeSize(const std::filesystem::path& path) {
    std::error_code error;
    const std::uintmax_t bytes = std::filesystem::file_size(path, error);
    if (error) {
      return "size unknown";
    }

    std::ostringstream out;
    if (bytes < 1024) {
      out << bytes << " B";
    } else if (bytes < 1024 * 1024) {
      out << (bytes / 1024) << " KB";
    } else {
      out.setf(std::ios::fixed);
      out.precision(1);
      out << (static_cast<double>(bytes) / (1024 * 1024)) << " MB";
    }

    return out.str();
  }

  static std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
  }

  std::filesystem::path m_root;
  std::filesystem::path m_directory;
  std::vector<std::filesystem::path> m_entries;
  std::map<std::string, std::unique_ptr<rse::Texture>> m_previews;
  int m_selected{-1};
};
