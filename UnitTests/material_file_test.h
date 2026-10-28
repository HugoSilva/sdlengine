#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <core/material_file.h>

/*
A material is two shader files and a name. These tests pin the file's version
contract and the round trip: the manager can parse a material without a GL
context, but it still has to be able to read what Save wrote.
*/
namespace {

using namespace rse;

class MaterialFileTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_material_file_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);
    m_file = m_directory / "warm.material";
    m_scratch = std::filesystem::path{m_file}.concat(".tmp");
  }

  void TearDown() override { std::filesystem::remove_all(m_directory); }

  static std::string ReadFile(const std::filesystem::path& file) {
    std::ifstream stream{file, std::ios::binary};
    return std::string{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
  }

  void WriteFile(const std::string& contents) {
    std::ofstream stream{m_file, std::ios::binary};
    stream << contents;
  }

  std::filesystem::path m_directory;
  std::filesystem::path m_file;
  std::filesystem::path m_scratch;
};

TEST_F(MaterialFileTest, SavingWritesTheMaterialFile) {
  MaterialFile material;
  material.name = "Warm";
  material.vertex_shader = "shaders/warm.vert";
  material.fragment_shader = "shaders/warm.frag";

  std::string error;
  ASSERT_TRUE(MaterialFile::Save(m_file, material, &error)) << error;

  EXPECT_TRUE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));

  const std::string contents = ReadFile(m_file);
  EXPECT_NE(contents.find("<formatVersion>1</formatVersion>"), std::string::npos);
  EXPECT_NE(contents.find("<name>Warm</name>"), std::string::npos);
  EXPECT_NE(contents.find("<vertexShader>shaders/warm.vert</vertexShader>"), std::string::npos);
  EXPECT_NE(contents.find("<fragmentShader>shaders/warm.frag</fragmentShader>"), std::string::npos);
}

TEST_F(MaterialFileTest, AFileRoundTrips) {
  MaterialFile written;
  written.name = "Warm";
  written.vertex_shader = "shaders/warm.vert";
  written.fragment_shader = "shaders/warm.frag";

  std::string error;
  ASSERT_TRUE(MaterialFile::Save(m_file, written, &error)) << error;

  MaterialFile read;

  ASSERT_TRUE(MaterialFile::Load(m_file, &read, &error)) << error;
  EXPECT_EQ(read.format_version, MaterialFile::kFormatVersion);
  EXPECT_EQ(read.name, "Warm");
  EXPECT_EQ(read.vertex_shader, "shaders/warm.vert");
  EXPECT_EQ(read.fragment_shader, "shaders/warm.frag");
}

TEST_F(MaterialFileTest, SavingOverwritesThePreviousMaterial) {
  MaterialFile first;
  first.name = "First";

  std::string error;
  ASSERT_TRUE(MaterialFile::Save(m_file, first, &error)) << error;

  MaterialFile second;
  second.name = "Second";
  second.fragment_shader = "shaders/second.frag";
  ASSERT_TRUE(MaterialFile::Save(m_file, second, &error)) << error;

  MaterialFile read;
  ASSERT_TRUE(MaterialFile::Load(m_file, &read, &error)) << error;
  EXPECT_EQ(read.name, "Second");
  EXPECT_EQ(read.fragment_shader, "shaders/second.frag");
  EXPECT_FALSE(std::filesystem::exists(m_scratch));
}

TEST_F(MaterialFileTest, ANewerMaterialIsRefused) {
  WriteFile("<cereal><material><formatVersion>2</formatVersion><name>Future</name></material></cereal>");

  MaterialFile material;
  std::string error;
  EXPECT_FALSE(MaterialFile::Load(m_file, &material, &error));
  EXPECT_NE(error.find("newer"), std::string::npos);
}

TEST_F(MaterialFileTest, AVersionZeroFileIsNotAMaterial) {
  WriteFile("<cereal><material><formatVersion>0</formatVersion><name>Old</name></material></cereal>");

  MaterialFile material;
  std::string error;
  EXPECT_FALSE(MaterialFile::Load(m_file, &material, &error));
  EXPECT_NE(error.find("not a material"), std::string::npos);
}

TEST_F(MaterialFileTest, AMissingMaterialReportsThePath) {
  MaterialFile material;
  std::string error;
  EXPECT_FALSE(MaterialFile::Load(m_directory / "missing.material", &material, &error));
  EXPECT_NE(error.find("not found"), std::string::npos);
}

TEST_F(MaterialFileTest, AFailedSaveLeavesNoScratchFile) {
  MaterialFile material;

  // A directory cannot be opened as a file, so the write fails before the
  // rename. The scratch path must not survive the failure.
  const std::filesystem::path directory = m_directory / "directory.material";
  std::filesystem::create_directories(directory);

  std::string error;
  EXPECT_FALSE(MaterialFile::Save(directory, material, &error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path{directory}.concat(".tmp")));
}

}  // namespace
