#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <ecs/ecs_manager.h>

/*
Regression tests for the project save/load path. Both functions used to let the
cereal archive throw straight out of the game loop: loading with no project.xml
present (or with an unreadable one) took the editor down, because the only
handler is the catch-all in Core::start(). They must now report the problem and
return false instead.
*/
namespace {

using namespace rse;

// save() and load() work on "project.xml" in the current working directory,
// so the fixtures run inside their own folder and restore the old one.
class ECSManagerProjectIo : public ::testing::Test {
 protected:
  void SetUp() override {
    m_previous_directory = std::filesystem::current_path();
    m_directory = std::filesystem::temp_directory_path() / "rse_project_io_test";

    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);
    std::filesystem::current_path(m_directory);
  }

  void TearDown() override {
    std::filesystem::current_path(m_previous_directory);
    std::filesystem::remove_all(m_directory);
  }

  void WriteProjectFile(const std::string& contents) const {
    std::ofstream project_file{"project.xml"};
    project_file << contents;
  }

  std::filesystem::path m_previous_directory;
  std::filesystem::path m_directory;
};

TEST_F(ECSManagerProjectIo, LoadWithoutAProjectFileReportsFailure) {
  ASSERT_FALSE(std::filesystem::exists("project.xml"));

  EXPECT_NO_THROW({ EXPECT_FALSE(ECSManager::Load()); });
}

TEST_F(ECSManagerProjectIo, LoadOfAnEmptyProjectFileReportsFailure) {
  WriteProjectFile("");
  ASSERT_TRUE(std::filesystem::exists("project.xml"));

  EXPECT_NO_THROW({ EXPECT_FALSE(ECSManager::Load()); });
}

TEST_F(ECSManagerProjectIo, LoadOfAMalformedProjectFileReportsFailure) {
  WriteProjectFile("<cereal>this is not a project file");

  EXPECT_NO_THROW({ EXPECT_FALSE(ECSManager::Load()); });
}

TEST_F(ECSManagerProjectIo, SaveWritesTheProjectFile) {
  EXPECT_TRUE(ECSManager::Save());

  EXPECT_TRUE(std::filesystem::exists("project.xml"));
  EXPECT_FALSE(std::filesystem::exists("project.xml.tmp"));
}

TEST_F(ECSManagerProjectIo, SavedProjectCanBeLoadedAgain) {
  ASSERT_TRUE(ECSManager::Save());

  EXPECT_NO_THROW({ EXPECT_TRUE(ECSManager::Load()); });
}
}  // namespace
