#include "smal_lresource.h"

#include <sstream>
#include <fstream>
#include <algorithm>
using namespace std;

SmallResource::SmallResource() {}

SmallResource::SmallResource(const std::string& path, const std::string& src) {
    Load(path, src);
}

void SmallResource::Load(const std::string &path, const std::string &data) {
    src_data = data;
    Read(path);
}

void SmallResource::Read(const std::string &path) {
    ifstream file(path);
    
    if(file) {
        std::string tmp;
        stringstream ss;
        
        while(getline(file, tmp)) {
            ss << tmp;
            _chunks.push_front( {} );
            ss 
               >> _chunks.front().user_data
               >> _chunks.front().begin
               >> _chunks.front().size;
            
            ss.clear();
        }
        
        file.close();
    }
}

vector<char> SmallResource::GetResource(const string& us_data) {
    //auto predicat = [&us_data] (Resource& rsrc ) -> bool {
    //    return (us_data == rsrc.user_data);
    //};
    auto& elem = *find(begin(_chunks), end(_chunks), us_data);
    
    ifstream src(src_data, ios_base::binary);
    vector<char> res(elem.size, '\0');
    
    if(src) {
        src.seekg(elem.begin);
        src.read(res.data(), elem.size);
        
        src.close();
    }
    
    return res;
}
