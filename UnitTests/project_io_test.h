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
  EXPECT_TRUE(actual.IsAttached());
  EXPECT_EQ(actual.File(), expected.File());
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
  WriteManifest("<cereal><project><formatVersion>2</formatVersion></project></cereal>");

  Project project;
  std::string error;

  EXPECT_FALSE(Project::Load(m_file, project, &error));
  EXPECT_NE(error.find("newer"), std::string::npos);
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, AnOlderFormatVersionIsRefused) {
  WriteManifest("<cereal><project><formatVersion>0</formatVersion></project></cereal>");

  Project project;
  std::string error;

  EXPECT_FALSE(Project::Load(m_file, project, &error));
  EXPECT_NE(error.find("older"), std::string::npos);
  EXPECT_FALSE(project.IsAttached());
}

TEST_F(ProjectIOTest, AFailedLoadLeavesTheGivenProjectAlone) {
  std::string error;
  ASSERT_TRUE(MakeProject().Save(&error)) << error;

  Project project;
  ASSERT_TRUE(Project::Load(m_file, project, &error)) << error;
  ASSERT_TRUE(project.IsAttached());

  WriteManifest("<cereal><project><formatVersion>2</formatVersion></project></cereal>");
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
