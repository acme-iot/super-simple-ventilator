/* 
 * File:   SPIFFSAccess.cpp
* Author: Marc Schaefer <marc-schaefer.dev@highdynamics.org>
 * 
 * Created on 20. Januar 2018, 03:52
 */

#include "SPIFFSAccess.h"

SPIFFSAccess::SPIFFSAccess() {
}

SPIFFSAccess::SPIFFSAccess(const SPIFFSAccess& orig) {
}

SPIFFSAccess::~SPIFFSAccess() {
}

void SPIFFSAccess::begin() {
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted");
}

void SPIFFSAccess::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

String SPIFFSAccess::read_file(const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = SPIFFS.open(path, FILE_READ);
    if(!file || file.isDirectory()){
        Serial.println("Failed to open file for reading");
        return "";
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    Serial.print("\n");

    return file.readString();
}

String SPIFFSAccess::readWholeFile(fs::FS &fs, const char * path)
{

    File file = fs.open(path, FILE_READ);
    if(!file || file.isDirectory()){
        Serial.println("Failed to open file for reading");
        return String("");
    }

    return file.readString();
}

void SPIFFSAccess::writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void SPIFFSAccess::appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void SPIFFSAccess::renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void SPIFFSAccess::deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

int SPIFFSAccess::deleteFileWithPrefix(const char * _Prefix)
{
    int count = 0;
    File root = SPIFFS.open("/");
    if(!root){
        Serial.println("Failed to open directory");
        return -1;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return -1;
    }

    String prefix(_Prefix);
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
        } else {
            String name(file.name());
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
            if(name.startsWith(prefix))
            {
                SPIFFS.remove(name);
                Serial.print("REMOVED");
                ++count;
            }
        }
        file = root.openNextFile();
    }
    return count;
}
void SPIFFSAccess::testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

/* void setup(){
    Serial.begin(115200);
    if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    listDir(SPIFFS, "/", 0);
    writeFile(SPIFFS, "/hello.txt", "Hello ");
    appendFile(SPIFFS, "/hello.txt", "World!\n");
    readFile(SPIFFS, "/hello.txt");
    deleteFile(SPIFFS, "/foo.txt");
    renameFile(SPIFFS, "/hello.txt", "/foo.txt");
    readFile(SPIFFS, "/foo.txt");
    testFileIO(SPIFFS, "/test.txt");
     
} */