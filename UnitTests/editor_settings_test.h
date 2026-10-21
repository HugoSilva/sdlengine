#pragma once

// The editor's settings live next to the editor, not in the engine, so these
// tests are only built when the editor is (see UnitTests/CMakeLists.txt).
#ifdef RSE_EDITOR_COMMANDS

#include <editor_settings.h>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <utils/logger.h>

/*
editor.cfg is the one file the editor writes for itself. These tests pin what
it round-trips, that a hand-edited file cannot turn a setting into something
arbitrary, and which project a fresh run opens.
*/
namespace {

class EditorSettingsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_file = std::filesystem::temp_directory_path() / "rse_editor_settings_test" / "editor.cfg";
    std::filesystem::create_directories(m_file.parent_path());
    std::filesystem::remove(m_file);
    Logger::Clear();
  }

  void TearDown() override {
    std::filesystem::remove_all(m_file.parent_path());
    Logger::Clear();
  }

  void Write(const std::string& contents) const {
    std::ofstream file{m_file};
    file << contents;
  }

  std::filesystem::path m_file;
};

TEST_F(EditorSettingsTest, SaveAndLoadRoundTrip) {
  EditorSettings saved;
  saved.grid.visible = false;
  saved.grid.base = 16.0f;
  saved.gizmo.mode = EditorSettings::GizmoMode::kRotate;
  saved.last_project = "C:/Projects/RSE/SampleProject/project.xml";
  saved.last_scene = "scenes/physics.xml";
  saved.panels["Hierarchy"] = false;
  saved.panels["Viewport"] = true;
  saved.Save(m_file.string());

  EditorSettings loaded;
  loaded.Load(m_file.string());

  EXPECT_FALSE(loaded.grid.visible);
  EXPECT_FLOAT_EQ(loaded.grid.base, 16.0f);
  EXPECT_EQ(loaded.gizmo.mode, EditorSettings::GizmoMode::kRotate);
  EXPECT_EQ(loaded.last_project, saved.last_project);
  EXPECT_EQ(loaded.last_scene, saved.last_scene);
  ASSERT_EQ(loaded.panels.size(), 2u);
  EXPECT_FALSE(loaded.panels.at("Hierarchy"));
  EXPECT_TRUE(loaded.panels.at("Viewport"));
}

TEST_F(EditorSettingsTest, ACorruptValueKeepsTheDefault) {
  Write("grid.visible=maybe\ngrid.base=not-a-number\ngizmo.mode=spin\npanel.Viewport=perhaps\nlast.project=\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  // A typo is ignored, not guessed at.
  EXPECT_TRUE(settings.grid.visible);
  EXPECT_FLOAT_EQ(settings.grid.base, 32.0f);
  EXPECT_EQ(settings.gizmo.mode, EditorSettings::GizmoMode::kTranslate);
  EXPECT_TRUE(settings.panels.empty());
}

TEST_F(EditorSettingsTest, AMissingFileKeepsEveryDefault) {
  EditorSettings settings;
  settings.Load(m_file.string());

  EXPECT_TRUE(settings.grid.visible);
  EXPECT_FLOAT_EQ(settings.grid.base, 32.0f);
  EXPECT_EQ(settings.gizmo.mode, EditorSettings::GizmoMode::kTranslate);
  EXPECT_TRUE(settings.last_project.empty());
  EXPECT_TRUE(settings.last_scene.empty());
  EXPECT_TRUE(settings.panels.empty());

  // Reading a file that is not there is the first run, not an error.
  for (const Logger::Entry& entry : Logger::GetEntries()) {
    EXPECT_NE(Logger::Level::kError, entry.level);
  }
}

TEST_F(EditorSettingsTest, ACommandLineProjectWinsOverTheLastOne) {
  const std::filesystem::path remembered = m_file.parent_path() / "remembered" / "project.xml";

  EditorSettings settings;
  settings.last_project = remembered.string();

  // Even when the remembered project exists, what was typed is what opens.
  EXPECT_EQ(StartupProject(settings, "typed/project.xml"), std::filesystem::path{"typed/project.xml"});
}

TEST_F(EditorSettingsTest, TheLastProjectIsReopenedWhenItIsStillThere) {
  const std::filesystem::path project = m_file.parent_path() / "project.xml";
  std::ofstream(project) << "<cereal/>";

  EditorSettings settings;
  settings.last_project = project.string();

  EXPECT_EQ(StartupProject(settings, ""), project);
}

TEST_F(EditorSettingsTest, AMissingLastProjectIsAWarningAndTheDefault) {
  const std::filesystem::path gone = m_file.parent_path() / "moved" / "project.xml";

  EditorSettings settings;
  settings.last_project = gone.string();

  EXPECT_EQ(StartupProject(settings, ""), std::filesystem::path{"project.xml"});

  // A moved folder must not stop the editor from starting, so it is said once
  // and the default takes over.
  bool warned = false;

  for (const Logger::Entry& entry : Logger::GetEntries()) {
    if (entry.level == Logger::Level::kWarning) {
      warned = true;
    }
  }

  EXPECT_TRUE(warned);
}

TEST_F(EditorSettingsTest, NothingRememberedStillOpensTheDefault) {
  const EditorSettings settings;
  EXPECT_EQ(StartupProject(settings, ""), std::filesystem::path{"project.xml"});
}
}  // namespace

#endif  // RSE_EDITOR_COMMANDS
