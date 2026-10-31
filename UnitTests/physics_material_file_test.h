#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <core/physics_material_file.h>

/*
A physics material is three fixture numbers and a name. These tests pin the
file's version contract and the round trip: the manager can parse one without a
physics world, but it still has to be able to read what Save wrote.
*/
namespace {

using namespace rse;

class PhysicsMaterialFileTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_directory = std::filesystem::temp_directory_path() / "rse_physics_material_file_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);
    m_file = m_directory / "ice.physicsmaterial";
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

TEST_F(PhysicsMaterialFileTest, SavingWritesThePhysicsMaterialFile) {
  PhysicsMaterialFile material;
  material.name = "Ice";
  material.material.density = 0.9f;
  material.material.friction = 0.05f;
  material.material.restitution = 0.1f;

  std::string error;
  ASSERT_TRUE(PhysicsMaterialFile::Save(m_file, material, &error)) << error;

  EXPECT_TRUE(std::filesystem::exists(m_file));
  EXPECT_FALSE(std::filesystem::exists(m_scratch));

  const std::string contents = ReadFile(m_file);
  EXPECT_NE(contents.find("<formatVersion>1</formatVersion>"), std::string::npos);
  EXPECT_NE(contents.find("<name>Ice</name>"), std::string::npos);
  EXPECT_NE(contents.find("<density>0.89999997615814209</density>"), std::string::npos);
  EXPECT_NE(contents.find("<friction>0.05000000074505806</friction>"), std::string::npos);
  EXPECT_NE(contents.find("<restitution>0.10000000149011612</restitution>"), std::string::npos);
}

TEST_F(PhysicsMaterialFileTest, AFileRoundTrips) {
  PhysicsMaterialFile written;
  written.name = "Ice";
  written.material.density = 0.9f;
  written.material.friction = 0.05f;
  written.material.restitution = 0.1f;

  std::string error;
  ASSERT_TRUE(PhysicsMaterialFile::Save(m_file, written, &error)) << error;

  PhysicsMaterialFile read;

  ASSERT_TRUE(PhysicsMaterialFile::Load(m_file, &read, &error)) << error;
  EXPECT_EQ(read.format_version, PhysicsMaterialFile::kFormatVersion);
  EXPECT_EQ(read.name, "Ice");
  EXPECT_FLOAT_EQ(read.material.density, 0.9f);
  EXPECT_FLOAT_EQ(read.material.friction, 0.05f);
  EXPECT_FLOAT_EQ(read.material.restitution, 0.1f);
}

TEST_F(PhysicsMaterialFileTest, AnEmptyNameIsAllowed) {
  PhysicsMaterialFile written;
  written.material.restitution = 0.5f;

  std::string error;
  ASSERT_TRUE(PhysicsMaterialFile::Save(m_file, written, &error)) << error;

  PhysicsMaterialFile read;
  ASSERT_TRUE(PhysicsMaterialFile::Load(m_file, &read, &error)) << error;
  EXPECT_TRUE(read.name.empty());
  EXPECT_FLOAT_EQ(read.material.restitution, 0.5f);
}

TEST_F(PhysicsMaterialFileTest, SavingOverwritesThePreviousMaterial) {
  PhysicsMaterialFile first;
  first.name = "First";
  first.material.friction = 0.1f;

  std::string error;
  ASSERT_TRUE(PhysicsMaterialFile::Save(m_file, first, &error)) << error;

  PhysicsMaterialFile second;
  second.name = "Second";
  second.material.friction = 0.9f;
  ASSERT_TRUE(PhysicsMaterialFile::Save(m_file, second, &error)) << error;

  PhysicsMaterialFile read;
  ASSERT_TRUE(PhysicsMaterialFile::Load(m_file, &read, &error)) << error;
  EXPECT_EQ(read.name, "Second");
  EXPECT_FLOAT_EQ(read.material.friction, 0.9f);
  EXPECT_FALSE(std::filesystem::exists(m_scratch));
}

TEST_F(PhysicsMaterialFileTest, ANewerMaterialIsRefused) {
  WriteFile("<cereal><physicsMaterial><formatVersion>2</formatVersion><name>Future</name></physicsMaterial></cereal>");

  PhysicsMaterialFile material;
  std::string error;
  EXPECT_FALSE(PhysicsMaterialFile::Load(m_file, &material, &error));
  EXPECT_NE(error.find("newer"), std::string::npos);
}

TEST_F(PhysicsMaterialFileTest, AVersionZeroFileIsNotAPhysicsMaterial) {
  WriteFile("<cereal><physicsMaterial><formatVersion>0</formatVersion><name>Old</name></physicsMaterial></cereal>");

  PhysicsMaterialFile material;
  std::string error;
  EXPECT_FALSE(PhysicsMaterialFile::Load(m_file, &material, &error));
  EXPECT_NE(error.find("not a physics material"), std::string::npos);
}

TEST_F(PhysicsMaterialFileTest, AMissingMaterialReportsThePath) {
  PhysicsMaterialFile material;
  std::string error;
  EXPECT_FALSE(PhysicsMaterialFile::Load(m_directory / "missing.physicsmaterial", &material, &error));
  EXPECT_NE(error.find("not found"), std::string::npos);
}

TEST_F(PhysicsMaterialFileTest, AFailedSaveLeavesNoScratchFile) {
  PhysicsMaterialFile material;

  // A directory cannot be opened as a file, so the write fails before the
  // rename. The scratch path must not survive the failure.
  const std::filesystem::path directory = m_directory / "directory.physicsmaterial";
  std::filesystem::create_directories(directory);

  std::string error;
  EXPECT_FALSE(PhysicsMaterialFile::Save(directory, material, &error));
  EXPECT_FALSE(error.empty());
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path{directory}.concat(".tmp")));
}

}  // namespace
