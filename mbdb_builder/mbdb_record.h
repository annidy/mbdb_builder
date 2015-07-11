#ifndef MBDB_RECORD_H_
#define MBDB_RECORD_H_

#include <cstdint>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <utility>

struct mbdb_record {
    typedef std::list<std::pair<std::string, std::string>> prop_table;
    
    mbdb_record(const char*& addr);
    void update(char*& addr) const;
    std::string get_path() const;
    void extract(const char* mbdb_dir) const;
    void cat(const char* mbdb_dir, const std::string& output) const;
    
    std::string domain;
    std::string path;
    std::string link_target;
    std::string data_hash;
    uint16_t    mode;
    uint32_t    uid;
    uint32_t    gid;
    uint32_t    mtime;
    uint32_t    atime;
    uint32_t    ctime;
    uint64_t    size;
    uint8_t     flag;
    prop_table  properties;
    
    std::string storage_hash;
};

void str2hash(std::vector<uint8_t> &hash, const std::string input);
void hash2str(std::vector<uint8_t> hash, std::string& output);
#endif /* !MBDB_RECORD_H_ */
