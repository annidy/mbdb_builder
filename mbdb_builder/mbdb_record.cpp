#include "mbdb_record.h"

#include "mbdb_io.h"
#include "sha1.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <vector>

template<typename T>
void hash2str(const T hash, std::string& output)
{
    static const char* nibble2str = "0123456789abcdef";
    
    for (typename T::value_type tmp : hash) {
        output.push_back(nibble2str[tmp >> 4]);
        output.push_back(nibble2str[tmp & 0xf]);
    }
}

template<typename T>
void str2hash(T& hash, const std::string& input)
{
    for (int i = 0; i < input.size(); i+=2) {
        char n[3] = {0};
        n[0] = input[i];
        n[1] = input[i+1];
        typename T::value_type tmp;
        sscanf(n, "%02x", &tmp);
        hash.push_back(tmp);
    }
}

static std::string generate_storage_hash(const std::string& domain, const std::string& path)
{
    std::string   hash_input;
    unsigned char hash[20];
    std::string   hash_str;
    
    hash_input = domain + "-" + path;
    
    sha1::calc(hash_input.c_str(), hash_input.size(), hash);
    
    hash2str(std::vector<uint8_t>(hash, hash + 20), hash_str);
    
    return hash_str;
}

mbdb_record::mbdb_record(const char*& addr)
{
    std::vector<uint8_t>  tmp_hash;
    uint8_t               prop_count;
    std::string           unused_str;
    uint32_t              unused_u32;
    
    read<std::string>(addr, this->domain);
    read<std::string>(addr, this->path);
    read<std::string>(addr, this->link_target);
    
    read<std::vector<uint8_t>>(addr, tmp_hash);
    hash2str(tmp_hash, this->data_hash);
    
    read<std::string>(addr, unused_str);
    
    read<uint16_t>(addr, this->mode);
    read<uint32_t>(addr, unused_u32);
    read<uint32_t>(addr, unused_u32);
    read<uint32_t>(addr, this->uid);
    read<uint32_t>(addr, this->gid);
    read<uint32_t>(addr, this->mtime);
    read<uint32_t>(addr, this->atime);
    read<uint32_t>(addr, this->ctime);
    read<uint64_t>(addr, this->size);
    read<uint8_t>(addr, this->flag);
    
    read<uint8_t>(addr, prop_count);
    
    for (int i = 0; i < prop_count; ++i) {
        std::string name;
        std::string value;
        
        read<std::string>(addr, name);
        read<std::string>(addr, value);
        
        this->properties.push_back(std::make_pair(name, value));
    }
    
    this->storage_hash = generate_storage_hash(this->domain, this->path);
}

void mbdb_record::update(char*& addr) const
{
    std::vector<uint8_t>  tmp_hash;
    uint8_t               prop_count;
    
    write<std::string>(addr, this->domain);
    write<std::string>(addr, this->path);
    
    if (this->link_target.length() == 0) {
        skip<std::string>(addr);
    } else {
        write<std::string>(addr, this->link_target);
    }
    
    if (this->data_hash.length() == 0) {
        skip<std::string>(addr);
    } else {
        str2hash(tmp_hash, this->data_hash);
        write<std::vector<uint8_t>>(addr, tmp_hash);
    }
    skip<std::string>(addr);
    
    write<uint16_t>(addr, this->mode);
    skip<uint32_t>(addr);
    skip<uint32_t>(addr);
    
    write<uint32_t>(addr, this->uid);
    write<uint32_t>(addr, this->gid);
    write<uint32_t>(addr, this->mtime);
    write<uint32_t>(addr, this->atime);
    write<uint32_t>(addr, this->ctime);
    write<uint64_t>(addr, this->size);
    write<uint8_t>(addr, this->flag);
    
    prop_count = this->properties.size();
    write<uint8_t>(addr, prop_count);
    
    for(auto it = this->properties.begin(); it != this->properties.end(); it++) {
        write<std::string>(addr, it->first);
        write<std::string>(addr, it->second);
    }
}

std::string mbdb_record::get_path() const
{
    return this->domain + "/" + this->path;
}

static bool extract_file(const std::string& out, const std::string& in, bool empty)
{
    bool    res = false;
    int     in_fd;
    int     out_fd;
    ssize_t size;
    char    buf[4096];
    
    if ((out_fd = open(out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
        goto cleanup_out;
    
    if (empty) {
        res = true;
        goto cleanup_out;
    }
    
    if ((in_fd = open(in.c_str(), O_RDONLY)) == -1)
        goto cleanup_in;
    
    while ((size = read(in_fd, buf, sizeof buf)) > 0)
        write(out_fd, buf, size);
    
    res = true;
    
cleanup_in:
    close(in_fd);
cleanup_out:
    close(out_fd);
    
    return res;
}

static bool extract_dir(const std::string& out)
{
    return mkdir(out.c_str(), 0777) == 0;
}

void mbdb_record::extract(const char* mbdb_dir) const
{
    bool        res = false;
    std::string target_path;
    
    target_path.reserve(this->domain.size() + sizeof '/' + this->path.size());
    target_path.append(this->domain);
    if (this->path.size() > 0) {
        target_path.append("/");
        target_path.append(this->path);
    }
    
    if (this->mode & S_IFREG) {
        std::string in_path;
        
        in_path.reserve(strlen(mbdb_dir) + sizeof '/' + 2 * SHA_DIGEST_LENGTH);
        in_path.append(mbdb_dir);
        in_path.append("/");
        in_path.append(this->storage_hash);
        
        res = extract_file(target_path, in_path, this->size == 0);
    } else if (this->mode & S_IFDIR) {
        res = extract_dir(target_path);
    }
    
    if (res) {
        struct timeval times[2];
        
        times[0].tv_sec = this->atime;
        times[0].tv_usec = 0;
        times[1].tv_sec = this->mtime;
        times[1].tv_usec = 0;
        
        chmod(target_path.c_str(), this->mode & 0777);
        utimes(target_path.c_str(), times);
    }
}

void mbdb_record::cat(const char* mbdb_dir, const std::string& output) const
{
    std::string in_path;
    
    if (!(this->mode & S_IFREG)) {
        printf("%s: not a file", this->get_path().c_str());
        return;
    }
    
    in_path.reserve(strlen(mbdb_dir) + sizeof '/' + 2 * SHA_DIGEST_LENGTH);
    in_path.append(mbdb_dir);
    in_path.append("/");
    in_path.append(this->storage_hash);
    
    extract_file(output, in_path, this->size == 0);
}
