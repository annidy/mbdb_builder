#include "mbdb_record.h"
#include "sha1.h"
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include <list>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <assert.h>

#ifdef _WIN32
#include "mingw-compat.h"
#include "mman.h"
#else
#include <sys/mman.h>
#endif
#define MBDB_SIG        "mbdb\5\0"
#define MBDB_SIG_LEN    ((sizeof MBDB_SIG) - 1)

static bool map(const char* path, char** begin_addr, char** end_addr)
{
    int     fd;
    struct  stat buf;
    void*   addr;
    
    fd = open(path, O_RDWR);
    stat(path, &buf);
    
    if ((addr = mmap(NULL, buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        printf("[error]mmap(%s)", path);
        return false;
    }
    
    close(fd);
    
    *begin_addr = (char*) addr;
    *end_addr = (char*) addr + buf.st_size;
    return true;
}

static void unmap(const char* begin_addr, const char* end_addr)
{
    munmap((void*) begin_addr, end_addr - begin_addr);
}

/*
 int main(int argc, char** argv)
 {
 const char*             begin_addr;
 const char*             end_addr;
 const char*             addr;
 std::list<mbdb_record>  contents;
 
 if (argc < 2)
 usage_die(argv[0]);
 
 map(argv[1], &begin_addr, &end_addr);
 
 addr = begin_addr;
 
 if (memcmp(addr, MBDB_SIG, MBDB_SIG_LEN) != 0)
 errx(EXIT_FAILURE, "%s: is not a valid MBDB file", argv[1]);
 
 addr += MBDB_SIG_LEN;
 
 while (addr < end_addr)
 contents.push_back(mbdb_record(addr));
 
 if (argc == 2 || strcmp(argv[2], "list") == 0) {
 for (const auto& e : contents)
 std::cout << e.get_path() << std::endl;
 } else if (strcmp(argv[2], "extract") == 0) {
 const char* dir = dirname(argv[1]);
 
 for (const auto& e : contents)
 e.extract(dir);
 } else if (strcmp(argv[2], "cat") == 0) {
 const char* dir = dirname(argv[1]);
 bool        did_it = false;
 
 if (argc != 4)
 usage_die(argv[0]);
 
 for (const auto& e : contents) {
 if (e.get_path() == argv[3]) {
 e.cat(dir, "/dev/stdout");
 did_it = true;
 break;
 }
 }
 
 if (!did_it) {
 errno = ENOENT;
 warn("%s", argv[3]);
 }
 } else {
 usage_die(argv[0]);
 }
 
 unmap(begin_addr, end_addr);
 
 return EXIT_SUCCESS;
 }
 */

int rebuild(const char *dir)
{
    char*             begin_addr;
    char*             end_addr;
    char*             addr;
    std::list<mbdb_record>  contents;
    std::string mbdb = std::string(dir)+"/Manifest.mbdb";
    if (!map(mbdb.c_str(), &begin_addr, &end_addr)) {
        return 1;
    }
    addr = begin_addr;
    if (memcmp(addr, MBDB_SIG, MBDB_SIG_LEN) != 0) {
        printf("%s: is not a valid MBDB file", mbdb.c_str());
        return 1;
    }
    
    addr += MBDB_SIG_LEN;
    const char *raddr = addr;
    while (raddr < end_addr)
        contents.push_back(mbdb_record(raddr));
    
    for (auto it = contents.begin(); it != contents.end(); ++it) {
        auto e = *it;
        std::string file = std::string(dir)+ "/" + e.storage_hash;
        if (e.data_hash.length()) {
            
            std::ifstream ifs(file);
            std::string str((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());
            
            unsigned char hash[20];
            sha1::calc(str.c_str(), (int)str.size(), hash);
            std::string hash_str;
            hash2str(std::vector<uint8_t>(hash, hash + 20), hash_str);
            assert(hash_str.length() == e.data_hash.length());
            
//            if (e.data_hash != hash_str) {
//                std::cout << e.storage_hash << std::endl;
//                if (e.size) {
//                    struct  stat buf;
//                    stat(file.c_str(), &buf);
//                    if (e.size != buf.st_size) {
//                        std::cout << "size" << std::endl;
//                    }
//                    e.size = buf.st_size;
//                }
//            }
            e.data_hash = hash_str;
        }
        e.update(addr);
    }
    unmap(begin_addr, end_addr);
    return 0;
}