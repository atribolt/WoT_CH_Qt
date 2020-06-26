#ifndef TEST_H
#define TEST_H

#include "webp/decode.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <QPixmap>
#include <QDir>

struct Decode {
    Decode(const QFileInfo& path) { 
        this->Load_RGBA_WebP (path);
    }

    std::vector<uint8_t> data;
    WebPBitstreamFeatures _img_prop;
    
    void Load_RGBA_WebP (const QFileInfo& fileName) {
        std::ifstream wimg(fileName.filePath().toLocal8Bit(), std::ios_base::binary);
        
        if(wimg){
            std::copy(std::istream_iterator<uint8_t>(wimg), {}, std::back_inserter(data));
            wimg.close();
            
            if(WebPGetFeatures(data.data(), data.size(), &_img_prop) == VP8_STATUS_OK) {
                uint8_t* decode_buf = WebPDecodeRGBA(data.data(), data.size(), &_img_prop.width, &_img_prop.height);
                
                data.resize(_img_prop.width * _img_prop.height * 4); // 4 - (r, g, b, a)
                std::copy(
                    std::make_move_iterator(std::begin(data)),
                    std::make_move_iterator(std::end(data)),
                    decode_buf
                );
                
                if(decode_buf) delete[] decode_buf;
            }
        }
    }
    
    QPixmap Get() {
        QPixmap res;
        res.loadFromData(data.data(), data.size(), "RGBA");
        return res;
    }
};

#endif // TEST_H
