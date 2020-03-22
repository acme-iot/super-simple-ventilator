#include <FileSystem.h>

#include <stdexcept>
#include <ArduinoLog.h>

#define IS_INITED handleInitialized(isInitialized);

AQUABOTICS_BEGIN_NAMESPACE

void FileSystem::handleInitialized(bool initialized) {
  if (initialized) return;

  Log.fatal("must be initialized before calling");
  throw std::runtime_error("must be initialized before calling");
}

void FileSystem::write(const char *path, const char *content, bool append) {
  Log.trace("writing %s", path);

  IS_INITED

  if (SPIFFS.exists(path) && !append) {
    Log.error("%s does not exist", path);
    return;
  }

  File file = SPIFFS.open(path, append ? FILE_APPEND : FILE_WRITE);
  if (!file) {
    Log.error("%s not available for writing", path);
    return;
  }
  if (file.print(content)) {
    Log.trace("success writing to file");
  } else {
    Log.error("failed writing to file");
  }
}

FileSystem::FileSystem() {
}

FileSystem::FileSystem(const FileSystem &orig) {
}

FileSystem::~FileSystem() {
}

void FileSystem::begin() {
  if (isInitialized) {
    Log.warning("already initialized");
    return;
  }

  if (!SPIFFS.begin(true)) {
    Log.fatal("unable to mount SPIFFS");
    throw std::runtime_error("unable to mount SPIFFS");
  }
  isInitialized = true;
  Log.trace("file system mounted");
}

void FileSystem::deleteDir(const char *path) {
  Log.trace("deleting %s", path);

  IS_INITED

  if (!SPIFFS.rmdir(path)) {
    Log.error("unable to delete directory");
    return;
  }

  Log.trace("directory deleted");
}

void FileSystem::createDir(const char *path) {
  Log.trace("creating %s", path);

  IS_INITED

  if (!SPIFFS.mkdir(path)) {
    Log.error("unable to create directory");
    return;
  }

  Log.trace("directory created");
}

std::optional<String> FileSystem::readFile(const char *path) {
  Log.trace("attempting to read, %s", path);

  IS_INITED

  auto file = SPIFFS.open(path, FILE_READ);

  if (!file) {
    Log.error("file, %s does not exist or could not be read", path);
    return std::nullopt;
  } else if (file.isDirectory()) {
    Log.error("path, %s is a directory", path);
    return std::nullopt;
  }

  auto content = file.readString();
  Log.verbose("contents of %s", path);
  Log.verbose(content.c_str());

  return content;
}

void FileSystem::writeFile(const char *path, const char *content) {
  write(path, content, false);
}

bool FileSystem::existsFile(const char *path) {
  Log.trace("begin::existsFile");
  Log.trace("checking if %s exists", path);

  IS_INITED

  if (SPIFFS.exists(path)) {
    Log.trace("file does not exist");
    return false;
  }
  return true;
}

void FileSystem::renameFile(const char *original, const char *destination) {
  Log.trace("rename, %s > %s", original, destination);

  IS_INITED

  if (!SPIFFS.rename(original, destination)) {
    Log.error("rename failed");
    return;
  }
  Log.trace("rename successful");
}

void FileSystem::deleteFile(const char *path) {
  Log.trace("deleting %s", path);

  IS_INITED

  if (!SPIFFS.remove(path)) {
    Log.error("deletion failed");
    return;
  }
  Log.trace("deletion successful");
}

AQUABOTICS_END_NAMESPACE