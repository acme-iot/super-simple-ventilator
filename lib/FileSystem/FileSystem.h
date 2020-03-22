#ifndef FILE_H_
#define FILE_H_ 1

#include <aquabotics_defines.h>

#include "FS.h"
#include "SPIFFS.h"

#include <optional>
#include <stdio.h>
#include <stdlib.h>

AQUABOTICS_BEGIN_NAMESPACE

class FileSystem {
 private:
  bool isInitialized;
  void handleInitialized(bool initialized);
  void write(const char *path, const char *contents, bool append = false);

 public:
  void begin();
  void deleteDir(const char *path);
  void createDir(const char *path);
  std::optional<String> readFile(const char *path);
  void writeFile(const char *path, const char *content);
  bool existsFile(const char *path);
  void renameFile(const char *original, const char *destination);
  void deleteFile(const char *path);
  FileSystem();
  FileSystem(const FileSystem &orig);
  virtual ~FileSystem();
};

AQUABOTICS_END_NAMESPACE

#endif