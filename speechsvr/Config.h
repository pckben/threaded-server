#ifndef SPEECHSVR_CONFIG_H
#define SPEECHSVR_CONFIG_H
#include <string>
#include <map>
namespace speechsvr {
  class Config {
   public:
    Config(std::string filename);
    ~Config();

    int Size() const { return map_.size(); }
    std::string GetString(std::string key);
    int GetInt(std::string key);
    float GetFloat(std::string key);

   private:
    std::map<std::string, std::string> map_;
  };
}
#endif  // SPEECHSVR_CONFIG_H
