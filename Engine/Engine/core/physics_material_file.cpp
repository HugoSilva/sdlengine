#include "physics_material_file.h"

#include <exception>
#include <fstream>
#include <system_error>

#include <cereal/archives/xml.hpp>

namespace rse {
namespace {
// The version is read before the fields it governs. A newer physics material
// may have renamed or removed a field, so letting cereal read the whole struct
// first would report a parse error instead of the real "too new" message.
struct PhysicsMaterialHeader {
  int format_version{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("formatVersion", format_version));
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}
}  // namespace

bool PhysicsMaterialFile::Load(const std::filesystem::path& file, PhysicsMaterialFile* material, std::string* error) {
  if (material == nullptr) {
    SetError(error, "no physics material destination");
    return false;
  }

  int format_version{0};

  {
    std::ifstream stream{file};

    if (!stream.is_open()) {
      SetError(error, "not found: " + file.string());
      return false;
    }

    try {
      cereal::XMLInputArchive archive{stream};
      PhysicsMaterialHeader header;
      archive(cereal::make_nvp("physicsMaterial", header));
      format_version = header.format_version;
    } catch (const std::exception& exception) {
      SetError(error, std::string("not a physics material: ") + exception.what() + ": " + file.string());
      return false;
    }
  }

  if (format_version < 1) {
    SetError(error,
             "not a physics material file: format version " + std::to_string(format_version) + ": " + file.string());
    return false;
  }

  if (format_version < kFirstFormatVersion) {
    SetError(error, "physics material format version " + std::to_string(format_version) +
                        " is older than this build supports (" + std::to_string(kFirstFormatVersion) +
                        "): " + file.string());
    return false;
  }

  if (format_version > kFormatVersion) {
    SetError(error, "physics material format version " + std::to_string(format_version) +
                        " is newer than this build supports (" + std::to_string(kFormatVersion) +
                        "): " + file.string());
    return false;
  }

  std::ifstream stream{file};

  if (!stream.is_open()) {
    SetError(error, "not found: " + file.string());
    return false;
  }

  try {
    cereal::XMLInputArchive archive{stream};
    archive(cereal::make_nvp("physicsMaterial", *material));
  } catch (const std::exception& exception) {
    SetError(error, std::string("could not read physics material: ") + exception.what() + ": " + file.string());
    return false;
  }

  return true;
}

bool PhysicsMaterialFile::Save(const std::filesystem::path& file, const PhysicsMaterialFile& material,
                               std::string* error) {
  std::filesystem::path scratch{file};
  scratch += ".tmp";

  std::ofstream stream{scratch};

  if (!stream.is_open()) {
    SetError(error, "could not be opened for writing: " + scratch.string());
    return false;
  }

  bool succeeded = true;
  std::string message;

  try {
    cereal::XMLOutputArchive archive{stream};
    archive(cereal::make_nvp("physicsMaterial", material));
  } catch (const std::exception& exception) {
    message = exception.what();
    succeeded = false;
  } catch (...) {
    message = "unknown error";
    succeeded = false;
  }

  stream.close();

  std::error_code error_code;

  if (!succeeded) {
    std::filesystem::remove(scratch, error_code);
    SetError(error, message);
    return false;
  }

  std::filesystem::rename(scratch, file, error_code);

  if (error_code) {
    std::filesystem::remove(scratch, error_code);
    SetError(error, error_code.message());
    return false;
  }

  return true;
}
}  // namespace rse
