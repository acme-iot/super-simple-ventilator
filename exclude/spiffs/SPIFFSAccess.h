/* 
 * File:   SPIFFSAccess.h
* Author: Marc Schaefer <marc-schaefer.dev@highdynamics.org>
 *
 * Created on 20. Januar 2018, 03:52
 */

#include "FS.h"
#include "SPIFFS.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef SPIFFSACCESS_H
#define SPIFFSACCESS_H


class SPIFFSAccess {
public:
    void begin();
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    String read_file(const char * path);
    String readWholeFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);
    void deleteFile(fs::FS &fs, const char * path);
    int deleteFileWithPrefix(const char * _Prefix);
    void testFileIO(fs::FS &fs, const char * path);
    SPIFFSAccess();
    SPIFFSAccess(const SPIFFSAccess& orig);
    virtual ~SPIFFSAccess();
private:

};

#endif /* SPIFFSACCESS_H */