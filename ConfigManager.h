#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>

class ConfigManager
{
public:
	ConfigManager();
	~ConfigManager();

	// Singleton interface
	static ConfigManager* Instance();


	std::string work_path(){ return work_path_; };
	std::string temp_path(){ return temp_path_; };
	void set_work_path(std::string work_path);
	void set_temp_path(std::string temp_path);

private:
	
	std::string work_path_;
	std::string temp_path_;

	static ConfigManager* instance_;
};


#endif