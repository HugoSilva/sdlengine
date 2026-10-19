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
    m_Root = std::filesystem::current_path(error);
    if (error) {
      m_Root = std::filesystem::path{"."};
    }

    // Assets are where you usually want to start, so open there when it
    // exists and fall back to the root.
    m_Directory = m_Root;
    const std::filesystem::path resources = m_Root / "Resources";
    if (std::filesystem::is_directory(resources, error)) {
      m_Directory = resources;
    }

    Refresh();
  }

  const char* Id() const override { return "Content Browser"; }
  DockSlot Slot() const override { return DockSlot::Left; }

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
    m_Entries.clear();
    m_Selected = -1;

    std::error_code error;
    std::filesystem::directory_iterator iterator{m_Directory, error};
    if (error) {
      Logger::Error("Cannot list " + m_Directory.string() + ": " + error.message());
      return;
    }

    for (const std::filesystem::directory_entry& entry : iterator) {
      m_Entries.push_back(entry.path());
    }

    // Folders first, then names, case-insensitively: what a file browser is
    // expected to look like.
    std::sort(m_Entries.begin(), m_Entries.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
      std::error_code aError, bError;
      const bool aIsDirectory = std::filesystem::is_directory(a, aError);
      const bool bIsDirectory = std::filesystem::is_directory(b, bError);

      if (aIsDirectory != bIsDirectory) {
        return aIsDirectory;
      }

      return ToLower(a.filename().string()) < ToLower(b.filename().string());
    });
  }

  void Navigate(const std::filesystem::path& directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
      return;
    }

    m_Directory = directory;
    Refresh();
  }

  void DrawToolbar() {
    const bool atRoot = m_Directory == m_Root;

    ImGui::BeginDisabled(atRoot);
    if (ImGui::Button("Up")) {
      Navigate(m_Directory.parent_path());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
      Refresh();
    }

    // Relative to the root, so the line stays short enough to read in a
    // narrow column. The root itself has no name to show.
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(m_Directory, m_Root, error);
    const std::string relativeName = relative.generic_string();
    const std::string label =
        error || relativeName.empty() || relativeName == "." ? std::string("/") : "/" + relativeName;
    ImGui::TextDisabled("%s", label.c_str());
  }

  void DrawEntryList() {
    // The listing takes most of the column; the rest is for the preview.
    const float listHeight = ImGui::GetContentRegionAvail().y * 0.62f;

    if (!ImGui::BeginChild("##content_entries", ImVec2(0.0f, listHeight), ImGuiChildFlags_Borders,
                           ImGuiWindowFlags_HorizontalScrollbar)) {
      ImGui::EndChild();
      return;
    }

    for (int index = 0; index < static_cast<int>(m_Entries.size()); ++index) {
      const std::filesystem::path& path = m_Entries[index];

      std::error_code error;
      const bool isDirectory = std::filesystem::is_directory(path, error);
      const std::string name = path.filename().string();
      const std::string label = isDirectory ? name + "/" : name;

      if (isDirectory) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.62f, 0.80f, 0.98f, 1.0f));
      }

      const bool clicked = ImGui::Selectable(label.c_str(), index == m_Selected, ImGuiSelectableFlags_AllowDoubleClick);

      if (isDirectory) {
        ImGui::PopStyleColor();
      }

      if (clicked) {
        m_Selected = index;

        if (isDirectory && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          // Copied because navigate() refreshes the listing this
          // element lives in.
          const std::filesystem::path target = path;
          Navigate(target);
          ImGui::EndChild();
          return;
        }
      }
    }

    if (m_Entries.empty()) {
      ImGui::TextDisabled("(empty)");
    }

    ImGui::EndChild();
  }

  void DrawDetails() {
    ImGui::Separator();

    if (m_Selected < 0 || m_Selected >= static_cast<int>(m_Entries.size())) {
      ImGui::TextDisabled("Select a file to preview it.");
      return;
    }

    const std::filesystem::path& path = m_Entries[m_Selected];

    std::error_code error;
    const bool isDirectory = std::filesystem::is_directory(path, error);

    ImGui::TextUnformatted(path.filename().string().c_str());

    if (isDirectory) {
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

    const std::map<std::string, std::unique_ptr<rse::Texture>>::iterator found = m_Previews.find(key);
    if (found != m_Previews.end()) {
      return found->second.get();
    }

    std::unique_ptr<rse::Texture> preview = std::make_unique<rse::Texture>(key, key);
    rse::Texture* raw = preview.get();
    m_Previews.emplace(key, std::move(preview));

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

  std::filesystem::path m_Root;
  std::filesystem::path m_Directory;
  std::vector<std::filesystem::path> m_Entries;
  std::map<std::string, std::unique_ptr<rse::Texture>> m_Previews;
  int m_Selected{-1};
};
