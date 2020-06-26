#include "Parce_WoT_Blitz_txt_mode.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <functional>

static const Point ZERO { 0, 0 };
static const _tex EMPTY_PARAM { "noatlas", { 1, 1 } };

#define sc_int(x) static_cast<int>( (x) )

using namespace std;

#ifdef _DEBUG
static tm __fastcall TimeNow() {
   time_t _sec {0};
   tm _now;

   time(&_sec);
   localtime_s(&_now, &_sec);
   return _now;
}
#endif // _DEBUG
static inline void LogMessage(string message) noexcept {
#ifdef _DEBUG
   tm t = TimeNow();
   // dd.mm|hh:mm:ss >>> message
   cerr << t.tm_mday << "."
        << t.tm_mon + 1  << "|"
        << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << " >>> "
        << message << '\n';
#endif // _DEBUG
}

#pragma region >>> Обработка ввода вывода
namespace std {
   istream& operator>>(istream& is, Point& p) {
      is >> p.first >> p.second;
      return is;
   }
   istream& operator>>(istream& is, _layer& l) {
      auto& [pos, size, offset, img, index] = l;
      
      is >> pos >> size >> offset >> img >> index;
      return is;
   }

   ostream& operator<<(ostream& os, const Point& p) noexcept {
      os << p.first << ' ' << p.second;
      return os;
   }
   ostream& operator<<(ostream& os, const _tex& t) noexcept {
      const auto& [name, size] = t;
      os << "File: " << name << "; Size atlas: " << size << ";";
      return os;
   }
   ostream& operator<<(ostream& os, const _layer& l) noexcept {
      const auto& [pos, size, offset, img, index] = l;
      os << pos << ' ' << size << ' ' << offset << ' ' << img << ' ' << index;
      return os;
   }
}

ostream& operator<<(ostream& os, const DataManager& dm) noexcept {
   os << dm._data;
   return os;
}
ostream& operator<<(ostream& os, const Texs& texs) noexcept {
   Names names;
   Points sizes;

   Unpack_Texs(begin(texs), end(texs), back_inserter(names), back_inserter(sizes));

   copy(begin(names), end(names), ostream_iterator<string>(os, "\n"));
   copy(begin(sizes), end(sizes), ostream_iterator<Point>(os, "\n"));

   return os;
}
ostream& operator<<(ostream& os, const Layers& ls) noexcept {
   copy(begin(ls), end(ls), ostream_iterator<_layer>(os, "\n"));
   //for(auto& l: ls) os << l << '\n';
   return os;
}
ostream& operator<<(ostream& os, const Data& data) noexcept {
   const auto& [texs, layers] = data;

   os << texs.size() << '\n'
      << texs 
      << layers.size() << '\n'
      << layers;

   return os;
}

istream& operator>>(istream& is, Texs& t) {
   size_t count {0}; // кол-во файлов
   is >> count;

   Names  _n{count, ""};    // имена файлов
   Points _s{count, {0,0}}; // размеры

   copy_n(istream_iterator<string>{is}, count, begin(_n));
   copy_n(istream_iterator<Point>{is}, count, begin(_s));

   Pack_Texs(_n, _s, back_inserter(t));
   return is;
}
istream& operator>>(istream& is, Layers& l){
   size_t count {0};
   is >> count;
   l.resize(count);
   copy(istream_iterator<_layer>{is}, {}, begin(l));
   return is;
}

istream& operator>>(istream& is, Data& d) {
   auto& [texs, layers] = d;
   is >> texs;
   is >> layers;
   return is;
}
#pragma endregion

template<class _out_name, class _out_size>
void Unpack_Texs ( 
   Texs::const_iterator src_beg, 
   Texs::const_iterator src_end, 
   _out_name out_name,           
   _out_size out_size)           
{
   auto unpack = [&out_name, &out_size](auto& t) { 
      auto& [name, size] = t;
      *out_name++ = name;
      *out_size++ = size;
   };

   for_each(src_beg, src_end, unpack);
}

template<class _it_where>
void Pack_Texs (const Names& _names, const Points& _sizes, _it_where where_to ) {
   auto _n_beg = begin(_names);
   auto _s_beg = begin(_sizes);

   for(; _n_beg != end(_names) && _s_beg != end(_sizes); ++_n_beg, ++_s_beg, ++where_to) {
      *where_to = { *_n_beg, *_s_beg };
   }
}

TexModify::TexModify (Data& _data, size_t index) :
   file   ( get<sc_int(FieldTex::NameAtlas)>(get<sc_int(FieldData::Atlases)>(_data)[index])),
   width  ((get<sc_int(FieldTex::Size)     >(get<sc_int(FieldData::Atlases)>(_data)[index]).first)),
   height ((get<sc_int(FieldTex::Size)     >(get<sc_int(FieldData::Atlases)>(_data)[index]).second))
{}
TexModify::TexModify(_tex& t) :
   file   ( get<sc_int(FieldTex::NameAtlas)>(t)),
   width  ((get<sc_int(FieldTex::Size)     >(t).first)),
   height ((get<sc_int(FieldTex::Size)     >(t).second))
{}

LayerModify::LayerModify(Data& _data, size_t index) :
   position (get<sc_int(FieldLayer::PositionTile)>(get<sc_int(FieldData::Layers)>(_data)[index])),
   size     (get<sc_int(FieldLayer::TileSize)    >(get<sc_int(FieldData::Layers)>(_data)[index])),
   offset   (get<sc_int(FieldLayer::OffsetTile)  >(get<sc_int(FieldData::Layers)>(_data)[index])),
   atlas    (get<sc_int(FieldLayer::AtlasNumber) >(get<sc_int(FieldData::Layers)>(_data)[index])),
   name     (get<sc_int(FieldLayer::Name)        >(get<sc_int(FieldData::Layers)>(_data)[index]))
{}
LayerModify::LayerModify(_layer& l) :
   position (get<sc_int(FieldLayer::PositionTile)>(l)),
   size     (get<sc_int(FieldLayer::TileSize)    >(l)),
   offset   (get<sc_int(FieldLayer::OffsetTile)  >(l)),
   atlas    (get<sc_int(FieldLayer::AtlasNumber) >(l)),
   name     (get<sc_int(FieldLayer::Name)        >(l))
{ }

DataModify::DataModify(Data& data) :
   _texs  (get<sc_int(FieldData::Atlases)>(data)),
   _layers(get<sc_int(FieldData::Layers) >(data))
{}

PathAtlases FindAtlasImage(Data& _data, const Path& path) {
/// проверить на webp
  PathAtlases res;
  DataModify md(_data);
  QDir dir(path.path()); //.parent_path().u8string();
  
  QStringList maskFiles( {"", ""} );
  
  for(auto& [tex, size] : md._texs) {
    Path file(tex.data());
    maskFiles[0] = (file.baseName() + "*.png");
    maskFiles[1] = (file.baseName() + "*.webp");
    dir.setNameFilters(maskFiles);
    res.push_back(dir.entryInfoList()[0]);
  }
  //for(auto& [tex, size] : md._texs) {
  //    QString ptex = tex.data();
  //    ptex.resize(ptex.size() - 3);
  //    ptex += "png";
  //
  //    atlases.push_back( { dir + "\\" + ptex } );
  //}
  
  return res;
}


template<class T>
bool DataManager::TestIndex(T& _elem, size_t index) noexcept {
   return (index < _elem.size());
}

bool DataManager::Load(const Path& path) noexcept {
    _work_directory = path;
    _current_atlas = 0;
    _current_layer = 0;

    QString _work_dir = _work_directory.filePath();
    ifstream read_data(_work_dir.toLocal8Bit());
    if(read_data) {
        read_data >> _data;
        _atlas_images = FindAtlasImage(_data, _work_directory);
        
        _error_flag = false;
        read_data.close();
    }
    else {
        //LogMessage("Ошибка открытия файла \'" + string(_work_directory.absolutePath().toLocal8Bit()) + "\'");
        _error_flag = true;
    }
   
    return !_error_flag;
}

DataManager::DataManager(const Path& path) noexcept 
{
   Load(path);
}

void DataManager::MoveTile(int dx, int dy) noexcept {
   if(!_error_flag) {
      LayerModify _l(_data, _current_layer);

      auto& [x, y] = _l.offset;
      x += dx;
      y += dy;
   } else {
      LogMessage("Объект в состоянии ошибки. Операция MoveTile(int,int) невозможна");
   }
}

void DataManager::Center(int x, int y) noexcept {
   if(!_error_flag) {
      auto& [_x, _y] = LayerModify(_data, _current_layer).offset;
      _x = x;
      _y = y;
   } else {
      LogMessage("Объект в состоянии ошибки. Center(int,int) невозможна");
   }
}


const Point& DataManager::Center() const noexcept {
   return (!_error_flag) ? get<2>(get<1>(_data)[_current_layer]) : ZERO;
}

void DataManager::AddPosition(int ax, int ay) {
    TexModify tm(_data, _current_atlas);
    LayerModify lm(_data, _current_layer);
    
    int x = lm.position.first + ax;
    int y = lm.position.second + ay;
    
    if(x >= 0 
    //&& (x + lm.size.first  < tm.width )
    ) lm.position.first  = x;
    
    if(y >= 0 
    //&& (y + lm.size.second < tm.height)
    ) lm.position.second = y;
}
void DataManager::Position(int x, int y) noexcept {
   if(!_error_flag) {
      auto& [_x, _y] = LayerModify(_data, _current_layer).position;
      _x = x;
      _y = y;
   } else {
      LogMessage("Объект в состоянии ошибки. Операция Position(int,int) невозможна");
   }
}

const Point& DataManager::Position() const noexcept {
   return (!_error_flag) ? get<0>(get<1>(_data)[_current_layer]) : ZERO;
}

void DataManager::AddSize(int ax, int ay) {
    TexModify tm(_data, _current_atlas);
    LayerModify lm(_data, _current_layer);
    
    int w = lm.size.first + ax;
    int h = lm.size.second + ay;
    
    if(w > 0 
    //&& (w + lm.position.first  < tm.width )
    ) lm.size.first  = w;
    if(h > 0 
    //&& (h + lm.position.second < tm.height)
    ) lm.size.second = h;
}
void DataManager::Size(int width, int height) noexcept {
   if(!_error_flag) {
      auto& [_w, _h] = LayerModify(_data, _current_atlas).size;
      _w = width;
      _h = height;
   } else {
      LogMessage("Объект в состоянии ошибки. Операция Size(int,int) невозможна");
   }
}

const Point& DataManager::Size() const noexcept {
   return (!_error_flag) ? get<1>(get<1>(_data)[_current_layer]) : ZERO;
}

string DataManager::CurentLayer() const noexcept {
   string res;

   if(!_error_flag) {
      stringstream ss;
      ss << DataModify(_data)._layers.at(_current_layer);
      getline(ss, res);
   } else {
      LogMessage("Объект в состоянии ошибки. Операция CurentLayer() невозможна");
   }
   
   return res;
}

const _tex& DataManager::AtlasParam() const noexcept {
    return (!_error_flag) ? (get<0>(_data))[_current_atlas] : EMPTY_PARAM;
}

bool DataManager::ChangeTex(size_t index) noexcept {
   if(_error_flag) {
      LogMessage("Объект в состоянии ошибки. Операция ChangeTex(size_t) невозможна");
      return false;
   }
   else {
      if(TestIndex(DataModify(_data)._texs, index)) {
         _current_atlas = index;
         return true;
      } else {
         LogMessage("Выбор несуществующего атласа");
         return false;
      }
   }
}

bool DataManager::ChangeLayer(size_t index) noexcept {
   if(_error_flag) {
      LogMessage("Объект в состоянии ошибки. Операция ChangeLayer(size_t) невозможна");
      return false;
   }
   else {
      if(TestIndex(DataModify(_data)._layers, index)) {
         _current_layer = index;
         return true;
      } else {
         LogMessage("Выбор несуществующего слоя");
         return false;
      }
   }
}

Path DataManager::CurrentAtlas() const noexcept {
   return (!_error_flag) ? _atlas_images[_current_atlas] : QFileInfo{};
}

Names DataManager::AllLayers() const noexcept {
   Names res;
   if(!_error_flag) {
      string tmp;
      stringstream ss;

      ss << DataModify(_data)._layers;
      while(getline(ss, tmp)) {
         res.push_back(tmp);
      }
   } else {
      LogMessage("Объект в состоянии ошибки. Операция AllLayers() невозможна");
   }
   return res;
}

bool DataManager::SaveTo() noexcept {
    ofstream file(_work_directory.filePath().toLocal8Bit());
    bool result = false;
    
    if(!_error_flag && file) {
        Update();
    
        file << _data;
        file.close();
        result = true;
    }
    
    return result;
}

void DataManager::Update() noexcept {
    if(!_error_flag) {
        int 
            max_wt, // максимальная ширина тайла
            max_ht; // максимальная высота тайла
        
        DataModify m(_data);
        
        for(int i = 0; i < m._texs.size(); ++i) { // идем по атласам
            max_wt = 0;
            max_ht = 0;
            for(auto& layer : m._layers) { // идем по слоям
                int& atlas = get<3>(layer);
                auto& [w, h] = get<1>(layer);
                
                if (atlas == i) {
                    if(max_wt < w) max_wt = w;
                    if(max_ht < h) max_ht = h;
                }
            }
            
            auto& [AW, AH] = get<1>(m._texs[i]);
            AW = max_wt;
            AH = max_ht;
        }
    }
}