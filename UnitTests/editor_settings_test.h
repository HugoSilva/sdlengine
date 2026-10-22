#pragma once

// The editor's settings live next to the editor, not in the engine, so these
// tests are only built when the editor is (see UnitTests/CMakeLists.txt).
#ifdef RSE_EDITOR_COMMANDS

#include <editor_settings.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <utils/file.h>
#include <utils/logger.h>

/*
editor.cfg is the one file the editor writes for itself. These tests pin what
it round-trips, that a hand-edited file cannot turn a setting into something
arbitrary, and which project a fresh run opens: the most recent one that is
still there.
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

  // A path that exists, which Load keeps: a file inside the fixture's own
  // directory, removed with the rest of it.
  std::filesystem::path ExistingProject(const std::string& name) const {
    const std::filesystem::path project = m_file.parent_path() / name / "project.xml";
    std::filesystem::create_directories(project.parent_path());
    std::ofstream(project) << "<cereal/>";
    return project;
  }

  // What RememberRecentProject stores: the path is absolute, because the next
  // run may not have the same working directory.
  static std::string Absolute(const std::string& path) { return std::filesystem::absolute(path).string(); }

  std::filesystem::path m_file;
};

TEST_F(EditorSettingsTest, SaveAndLoadRoundTrip) {
  const std::filesystem::path one = ExistingProject("one");
  const std::filesystem::path two = ExistingProject("two");

  EditorSettings saved;
  saved.grid.visible = false;
  saved.grid.base = 16.0f;
  saved.gizmo.mode = EditorSettings::GizmoMode::kRotate;
  saved.RememberRecentProject(one.string());
  saved.RememberRecentProject(two.string());
  saved.last_scene = "scenes/physics.xml";
  saved.layout_version = 1;
  saved.panels["Hierarchy"] = false;
  saved.panels["Viewport"] = true;
  saved.Save(m_file.string());

  EditorSettings loaded;
  loaded.Load(m_file.string());

  EXPECT_FALSE(loaded.grid.visible);
  EXPECT_FLOAT_EQ(loaded.grid.base, 16.0f);
  EXPECT_EQ(loaded.gizmo.mode, EditorSettings::GizmoMode::kRotate);
  ASSERT_EQ(loaded.recent_projects.size(), 2u);
  EXPECT_EQ(loaded.recent_projects.at(0), Absolute(two.string()));
  EXPECT_EQ(loaded.recent_projects.at(1), Absolute(one.string()));
  EXPECT_EQ(loaded.last_scene, saved.last_scene);
  EXPECT_EQ(loaded.layout_version, 1);
  ASSERT_EQ(loaded.panels.size(), 2u);
  EXPECT_FALSE(loaded.panels.at("Hierarchy"));
  EXPECT_TRUE(loaded.panels.at("Viewport"));
}

TEST_F(EditorSettingsTest, ACorruptValueKeepsTheDefault) {
  Write(
      "grid.visible=maybe\ngrid.base=not-a-number\ngizmo.mode=spin\npanel.Viewport=perhaps\n"
      "layout.version=soon\nrecent.x=somewhere/project.xml\nrecent.0=\nlast.project=\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  // A typo is ignored, not guessed at.
  EXPECT_TRUE(settings.grid.visible);
  EXPECT_FLOAT_EQ(settings.grid.base, 32.0f);
  EXPECT_EQ(settings.gizmo.mode, EditorSettings::GizmoMode::kTranslate);
  EXPECT_EQ(settings.layout_version, 0);
  EXPECT_TRUE(settings.panels.empty());
  EXPECT_TRUE(settings.recent_projects.empty());
}

TEST_F(EditorSettingsTest, TheRemovedDebugPanelIsDropped) {
  Write("panel.Debug=false\npanel.Hierarchy=true\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  // The panel no longer exists, so the key is neither read nor carried into
  // the next write.
  EXPECT_EQ(settings.panels.find("Debug"), settings.panels.end());
  ASSERT_EQ(settings.panels.size(), 1u);
  EXPECT_TRUE(settings.panels.at("Hierarchy"));

  settings.Save(m_file.string());
  const std::string written = File::Read(m_file.string().c_str());
  EXPECT_EQ(written.find("panel.Debug"), std::string::npos);
}

TEST_F(EditorSettingsTest, AMissingFileKeepsEveryDefault) {
  EditorSettings settings;
  settings.Load(m_file.string());

  EXPECT_TRUE(settings.grid.visible);
  EXPECT_FLOAT_EQ(settings.grid.base, 32.0f);
  EXPECT_EQ(settings.gizmo.mode, EditorSettings::GizmoMode::kTranslate);
  EXPECT_TRUE(settings.recent_projects.empty());
  EXPECT_TRUE(settings.last_scene.empty());
  EXPECT_TRUE(settings.panels.empty());

  // Reading a file that is not there is the first run, not an error.
  for (const Logger::Entry& entry : Logger::GetEntries()) {
    EXPECT_NE(Logger::Level::kError, entry.level);
  }
}

TEST_F(EditorSettingsTest, TheRecentListIsMostRecentFirst) {
  EditorSettings settings;
  settings.RememberRecentProject("one/project.xml");
  settings.RememberRecentProject("two/project.xml");
  settings.RememberRecentProject("three/project.xml");

  ASSERT_EQ(settings.recent_projects.size(), 3u);
  EXPECT_EQ(settings.recent_projects.at(0), Absolute("three/project.xml"));
  EXPECT_EQ(settings.recent_projects.at(1), Absolute("two/project.xml"));
  EXPECT_EQ(settings.recent_projects.at(2), Absolute("one/project.xml"));
}

TEST_F(EditorSettingsTest, RememberingAProjectAgainMovesItToTheFront) {
  EditorSettings settings;
  settings.RememberRecentProject("one/project.xml");
  settings.RememberRecentProject("two/project.xml");
  settings.RememberRecentProject("one/project.xml");

  // Opening the same project twice is one entry, not two, and the newest visit
  // is what the next run reopens.
  ASSERT_EQ(settings.recent_projects.size(), 2u);
  EXPECT_EQ(settings.recent_projects.at(0), Absolute("one/project.xml"));
  EXPECT_EQ(settings.recent_projects.at(1), Absolute("two/project.xml"));
}

TEST_F(EditorSettingsTest, TheRecentListKeepsFive) {
  EditorSettings settings;

  for (int index = 1; index <= 7; ++index) {
    settings.RememberRecentProject("project" + std::to_string(index) + "/project.xml");
  }

  ASSERT_EQ(settings.recent_projects.size(), EditorSettings::kMaxRecentProjects);
  EXPECT_EQ(settings.recent_projects.front(), Absolute("project7/project.xml"));
  EXPECT_EQ(settings.recent_projects.back(), Absolute("project3/project.xml"));
}

TEST_F(EditorSettingsTest, LoadDropsRecentProjectsThatAreGone) {
  const std::filesystem::path kept = ExistingProject("kept");
  const std::filesystem::path gone = m_file.parent_path() / "gone" / "project.xml";

  Write("recent.0=" + gone.string() + "\nrecent.1=" + kept.string() + "\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  // A deleted folder is not an entry: the next run would only fail on it.
  ASSERT_EQ(settings.recent_projects.size(), 1u);
  EXPECT_EQ(settings.recent_projects.front(), Absolute(kept.string()));
}

TEST_F(EditorSettingsTest, LoadOrdersByTheNumberNotTheLine) {
  const std::filesystem::path one = ExistingProject("one");
  const std::filesystem::path two = ExistingProject("two");

  // Hand-edited files are not always in order; recent.0 is the newest whatever
  // line it is on.
  Write("recent.1=" + two.string() + "\nrecent.0=" + one.string() + "\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  ASSERT_EQ(settings.recent_projects.size(), 2u);
  EXPECT_EQ(settings.recent_projects.at(0), Absolute(one.string()));
  EXPECT_EQ(settings.recent_projects.at(1), Absolute(two.string()));
}

TEST_F(EditorSettingsTest, LoadKeepsOneEntryPerProject) {
  const std::filesystem::path one = ExistingProject("one");
  const std::filesystem::path two = ExistingProject("two");

  Write("recent.0=" + one.string() + "\nrecent.1=" + two.string() + "\nrecent.2=" + one.string() + "\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  ASSERT_EQ(settings.recent_projects.size(), 2u);
  EXPECT_EQ(settings.recent_projects.at(0), Absolute(one.string()));
  EXPECT_EQ(settings.recent_projects.at(1), Absolute(two.string()));
}

TEST_F(EditorSettingsTest, LoadUsesTheOldLastProjectOnce) {
  const std::filesystem::path old = ExistingProject("old");

  // What the build before the list wrote: one project and no recent.N.
  Write("last.project=" + old.string() + "\n");

  EditorSettings settings;
  settings.Load(m_file.string());

  ASSERT_EQ(settings.recent_projects.size(), 1u);
  EXPECT_EQ(settings.recent_projects.front(), Absolute(old.string()));

  // Saving turns it into a list and drops the key: the fallback happens once.
  settings.Save(m_file.string());
  const std::string text = File::Read(m_file.string().c_str());

  EXPECT_NE(text.find("recent.0="), std::string::npos);
  EXPECT_EQ(text.find("last.project"), std::string::npos);
}

TEST_F(EditorSettingsTest, ACommandLineProjectWinsOverTheRecentList) {
  const std::filesystem::path remembered = m_file.parent_path() / "remembered" / "project.xml";

  EditorSettings settings;
  settings.recent_projects = {remembered.string()};

  // Even when the remembered project exists, what was typed is what opens.
  EXPECT_EQ(StartupProject(settings, "typed/project.xml"), std::filesystem::path{"typed/project.xml"});
}

TEST_F(EditorSettingsTest, TheMostRecentProjectIsReopenedWhenItIsStillThere) {
  const std::filesystem::path project = m_file.parent_path() / "project.xml";
  std::ofstream(project) << "<cereal/>";

  EditorSettings settings;
  settings.recent_projects = {project.string()};

  EXPECT_EQ(StartupProject(settings, ""), project);
}

TEST_F(EditorSettingsTest, AMissingMostRecentProjectIsAWarningAndTheDefault) {
  const std::filesystem::path gone = m_file.parent_path() / "moved" / "project.xml";

  EditorSettings settings;
  settings.recent_projects = {gone.string()};

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
