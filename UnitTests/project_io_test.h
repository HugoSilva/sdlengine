#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <core/project.h>

/*
The manifest is the file that turns a folder into a project. These tests pin
what it stores, that it survives a round trip, and that a manifest this build
cannot read is refused *before* anything is assigned to the caller's project --
a failed open must not leave a project attached to a file it could not parse,
or the next save would overwrite it.
*/
namespace {

using namespace rse;

class ProjectIOTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_project_io_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);

    m_file = m_directory / "project.xml";
    m_scratch = std::filesystem::path{m_file}.concat(".tmp");
  }

  void TearDown() override { std::filesystem::remove_all(m_directory); }

  void WriteManifest(const std::string& contents) const {
    std::ofstream file{m_file};
    file << contents;
  }

  static std::string ReadFile(const std::filesystem::path& file) {
    std::ifstream stream{file};
    return std::string{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
  }

  Project MakeProject() const {
    Project project;
    project.SetFile(m_file);
    project.name = "Sandbox";
    project.engine_version = "0.13";
    project.asset_root = "assets";
    project.window.title = "Sandbox";
    project.window.width = 1600;
    project.window.height = 900;
    project.startup_scene = "scenes/start.xml";
    project.scenes = {"scenes/start.xml", "scenes/level_02.xml"};
    project.audio.master = 100;
    project.audio.music = 80;
    project.audio.sfx = 60;
    project.input = {{"editor.save", "F5"}, {"game.up", "W"}};
    return project;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_file;
  std::filesystem::path m_scratch;
};

TEST_F(ProjectIOTest, DefaultsDescribeAFreshProject) {
  const Project project;

  EXPECT_EQ(project.format_version, Project::kFormatVersion);
  EXPECT_EQ(project.name, "Untitled");
  EXPECT_EQ(project.asset_root, "assets");
  EXPECT_EQ(project.window.width, 1280);
  EXPECT_EQ(project.window.height, 720);
  EXPECT_EQ(project.startup_scene, "scenes/start.xml");
  ASSERT_EQ(project.scenes.size(), 1u);
  EXPECT_EQ(project.scenes.front(), "scenes/start.xml");
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, SaveWritesTheManifestAndLeavesNoScratchFile) {
  const Project project = MakeProject();

  std::string error;
  ASSERT_TRUE(project.Save(&error)) << error;

  EXPECT_TRUE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));
}

TEST_F(ProjectIOTest, RoundTripPreservesEveryField) {
  const Project expected = MakeProject();

  std::string error;
  ASSERT_TRUE(expected.Save(&error)) << error;

  Project actual;
  ASSERT_TRUE(Project::Load(m_file, actual, &error)) << error;

  EXPECT_EQ(actual.format_version, expected.format_version);
  EXPECT_EQ(actual.name, expected.name);
  EXPECT_EQ(actual.engine_version, expected.engine_version);
  EXPECT_EQ(actual.asset_root, expected.asset_root);
  EXPECT_EQ(actual.window.title, expected.window.title);
  EXPECT_EQ(actual.window.width, expected.window.width);
  EXPECT_EQ(actual.window.height, expected.window.height);
  EXPECT_EQ(actual.startup_scene, expected.startup_scene);
  ASSERT_EQ(actual.scenes.size(), expected.scenes.size());
  EXPECT_EQ(actual.scenes.at(0), expected.scenes.at(0));
  EXPECT_EQ(actual.scenes.at(1), expected.scenes.at(1));
  EXPECT_EQ(actual.audio.master, expected.audio.master);
  EXPECT_EQ(actual.audio.music, expected.audio.music);
  EXPECT_EQ(actual.audio.sfx, expected.audio.sfx);
  ASSERT_EQ(actual.input.size(), expected.input.size());
  EXPECT_EQ(actual.input.at(0).action, expected.input.at(0).action);
  EXPECT_EQ(actual.input.at(0).key, expected.input.at(0).key);
  EXPECT_EQ(actual.input.at(1).action, expected.input.at(1).action);
  EXPECT_EQ(actual.input.at(1).key, expected.input.at(1).key);
  EXPECT_TRUE(actual.IsAttached());
  EXPECT_EQ(actual.File(), expected.File());
}

TEST_F(ProjectIOTest, AudioLevelsOutsideZeroToFullAreClampedOnLoad) {
  Project project = MakeProject();
  project.audio.master = 300;
  project.audio.music = -5;
  project.audio.sfx = 200;

  std::string error;
  ASSERT_TRUE(project.Save(&error)) << error;

  Project loaded;
  ASSERT_TRUE(Project::Load(m_file, loaded, &error)) << error;

  // The scale is 0-128; a hand-edited manifest cannot ask for a gain above one
  // or below silence.
  EXPECT_EQ(loaded.audio.master, 128);
  EXPECT_EQ(loaded.audio.music, 0);
  EXPECT_EQ(loaded.audio.sfx, 128);
}

TEST_F(ProjectIOTest, SavedPathsUseForwardSlashes) {
  Project project = MakeProject();
  project.asset_root = "assets\\shaders";
  project.startup_scene = "scenes\\start.xml";
  project.scenes = {"scenes\\start.xml", "scenes\\level_02.xml"};

  std::string error;
  ASSERT_TRUE(project.Save(&error)) << error;

  const std::string contents = ReadFile(m_file);
  EXPECT_EQ(contents.find('\\'), std::string::npos);
  EXPECT_NE(contents.find("assets/shaders"), std::string::npos);
  EXPECT_NE(contents.find("scenes/start.xml"), std::string::npos);
}

TEST_F(ProjectIOTest, LoadWithoutAFileReportsFailure) {
  Project project;
  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(Project::Load(m_file, project, &error)); });
  EXPECT_NE(error.find("not found"), std::string::npos);
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, LoadOfAnEmptyFileReportsFailure) {
  WriteManifest("");

  Project project;
  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(Project::Load(m_file, project, &error)); });
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, LoadOfAMalformedFileReportsFailure) {
  WriteManifest("<cereal><project><name>no format version");

  Project project;
  std::string error;

  EXPECT_NO_THROW({ EXPECT_FALSE(Project::Load(m_file, project, &error)); });
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, ANewerFormatVersionIsRefused) {
  WriteManifest("<cereal><project><formatVersion>99</formatVersion></project></cereal>");

  Project project = MakeProject();
  std::string error;

  EXPECT_FALSE(Project::Load(m_file, project, &error));
  EXPECT_NE(error.find("99"), std::string::npos);
  EXPECT_NE(error.find("newer"), std::string::npos);

  // A refused file leaves the caller's project exactly as it was.
  EXPECT_TRUE(project.IsAttached());
  EXPECT_EQ(project.name, "Sandbox");
}

TEST_F(ProjectIOTest, AVersionZeroManifestIsNotAProjectFile) {
  WriteManifest("<cereal><project><formatVersion>0</formatVersion></project></cereal>");

  Project project;
  std::string error;

  EXPECT_FALSE(Project::Load(m_file, project, &error));
  EXPECT_NE(error.find("not a project file"), std::string::npos);
  EXPECT_FALSE(project.IsAttached());

  // A negative version is the same kind of nonsense, not an "older" manifest.
  WriteManifest("<cereal><project><formatVersion>-1</formatVersion></project></cereal>");
  EXPECT_FALSE(Project::Load(m_file, project, &error));
  EXPECT_NE(error.find("not a project file"), std::string::npos);
}

// The first published manifest stays loadable. The fixture is never rewritten,
// so a change that stops reading it fails here instead of in a user's project.
TEST_F(ProjectIOTest, AV1ManifestLoadsFromTheFixture) {
  const std::filesystem::path fixture = std::filesystem::path{RSE_FIXTURES_DIR} / "project_v1.xml";

  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(fixture, project, &error)) << error;

  EXPECT_EQ(project.format_version, 1);
  EXPECT_EQ(project.name, "Fixture");
  EXPECT_EQ(project.engine_version, "0.13");
  EXPECT_EQ(project.asset_root, "assets");
  EXPECT_EQ(project.window.title, "Fixture Project");
  EXPECT_EQ(project.window.width, 1024);
  EXPECT_EQ(project.window.height, 576);
  EXPECT_EQ(project.startup_scene, "scenes/start.xml");
  ASSERT_EQ(project.scenes.size(), 1u);
  EXPECT_EQ(project.scenes.front(), "scenes/start.xml");

  // A version 1 manifest predates the audio section; it opens at the full
  // volume it played at, not at silence.
  EXPECT_EQ(project.audio.master, 128);
  EXPECT_EQ(project.audio.music, 128);
  EXPECT_EQ(project.audio.sfx, 128);

  // Version 1 also predates the input section: the compiled defaults are the
  // bindings, and there is nothing for the project to override.
  EXPECT_TRUE(project.input.empty());

  EXPECT_EQ(project.File(), std::filesystem::absolute(fixture));
}

// Version 2 added the audio section; version 3 the input bindings. A version 2
// manifest therefore has one and not the other, which is what additive means.
TEST_F(ProjectIOTest, AV2ManifestHasAudioButNoInputOverrides) {
  WriteManifest(
      "<cereal><project>"
      "<formatVersion>2</formatVersion>"
      "<name>Two</name>"
      "<engineVersion>0.13</engineVersion>"
      "<assetRoot>assets</assetRoot>"
      "<window><title>Two</title><width>800</width><height>600</height></window>"
      "<startupScene>scenes/start.xml</startupScene>"
      "<scene size=\"dynamic\"><value0>scenes/start.xml</value0></scene>"
      "<audio><master>64</master><music>32</music><sfx>16</sfx></audio>"
      "</project></cereal>");

  Project project;
  std::string error;

  ASSERT_TRUE(Project::Load(m_file, project, &error)) << error;

  EXPECT_EQ(project.format_version, 2);
  EXPECT_EQ(project.audio.master, 64);
  EXPECT_EQ(project.audio.music, 32);
  EXPECT_EQ(project.audio.sfx, 16);
  EXPECT_TRUE(project.input.empty());
}

TEST_F(ProjectIOTest, SaveWritesTheCurrentFormatVersionNotTheLoadedOne) {
  Project project = MakeProject();
  project.format_version = 0;  // as if it had been read from an older file
  project.engine_version = "0.1";

  std::string error;
  ASSERT_TRUE(project.Save(&error)) << error;

  const std::string contents = ReadFile(m_file);

  EXPECT_NE(contents.find("<formatVersion>" + std::to_string(Project::kFormatVersion) + "</formatVersion>"),
            std::string::npos);
  EXPECT_EQ(contents.find("<formatVersion>0</formatVersion>"), std::string::npos);

  // The engine version is informational, but it would otherwise keep naming the
  // build that wrote the file first.
  EXPECT_NE(contents.find(std::string{"<engineVersion>"} + kEngineVersion + "</engineVersion>"), std::string::npos);
}

TEST_F(ProjectIOTest, AFailedLoadLeavesTheGivenProjectAlone) {
  std::string error;
  ASSERT_TRUE(MakeProject().Save(&error)) << error;

  Project project;
  ASSERT_TRUE(Project::Load(m_file, project, &error)) << error;
  ASSERT_TRUE(project.IsAttached());

  WriteManifest("<cereal><project><formatVersion>" + std::to_string(Project::kFormatVersion + 1) +
                "</formatVersion></project></cereal>");
  EXPECT_FALSE(Project::Load(m_file, project, &error));

  // Still the project that was open, not a half-read replacement of it.
  EXPECT_TRUE(project.IsAttached());
  EXPECT_EQ(project.name, "Sandbox");
}

TEST_F(ProjectIOTest, ResolveUsesTheProjectDirectory) {
  Project project = MakeProject();

  EXPECT_EQ(project.Resolve("scenes/start.xml"), m_directory / "scenes/start.xml");
  EXPECT_EQ(project.Resolve("scenes\\start.xml"), m_directory / "scenes/start.xml");
  EXPECT_EQ(project.Resolve("../shared/tex3.png"), m_directory / ".." / "shared" / "tex3.png");
  EXPECT_EQ(project.Resolve("C:/assets/tex3.png"), std::filesystem::path{"C:/assets/tex3.png"});
}

TEST_F(ProjectIOTest, ResolveLeavesPathsAloneWithoutAProject) {
  const Project project;

  EXPECT_EQ(project.Resolve("Resources/tex3.png"), std::filesystem::path{"Resources/tex3.png"});
}

TEST_F(ProjectIOTest, AnUnwritablePathReportsFailure) {
  Project project;
  project.SetFile(m_directory / "missing" / "project.xml");

  std::string error;

  EXPECT_FALSE(project.Save(&error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path{project.File()}.concat(".tmp")));
}
}  // namespace
