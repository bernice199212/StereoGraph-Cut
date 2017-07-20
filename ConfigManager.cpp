#include "stdafx.h"
#include "ConfigManager.h"

ConfigManager* ConfigManager::instance_ = NULL;

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

ConfigManager* ConfigManager::Instance()
{
	if (instance_ == NULL)
		instance_ = new ConfigManager();
	return instance_;
}

void ConfigManager::set_work_path(std::string work_path)
{
	// Find the last index of '//'
	size_t last_slash_index = work_path.rfind("\\");
	this->work_path_ = work_path.substr(0, last_slash_index+1);
}

void ConfigManager::set_temp_path(std::string temp_path)
{
	size_t last_slash_index = temp_path.rfind("\\");
	this->temp_path_ = temp_path.substr(0, last_slash_index+1);
}