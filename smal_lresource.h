#ifndef SMAL_LRESOURCE_H
#define SMAL_LRESOURCE_H

#include <list>
#include <vector>
#include <memory>
#include <string>

struct Resource{
    std::string user_data;
    int 
        begin { -1 },
        size  { -1 };
};

namespace std {
    inline bool operator==(const Resource& a, const Resource& b) {
        return a.user_data == b.user_data;
    }
    inline bool operator!=(const Resource& a, const Resource& b) {
        return !(a == b);
    }
    inline bool operator==(const Resource& a, const std::string& b) {
        return a.user_data == b;
    }
    inline bool operator!=(const Resource& a, const std::string& b) {
        return !(a == b);
    }
}

class SmallResource
{
    std::string src_data;
    
    void Read(const std::string& path);
public:
    std::list<Resource> _chunks;
    
    SmallResource();
    SmallResource(const std::string& path, const std::string& data);
    
    void Load(const std::string& path, const std::string& data);
    
    std::vector<char> GetResource(const std::string& us_data);
};

#endif // SMAL_LRESOURCE_H
