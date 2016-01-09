/* 
 * File:   ConfigSettingString.h
 * Author: johannes
 *
 * Created on 26 December 2015, 01:20
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 * 
 */

#ifndef CONFIGSETTINGSTRING_H
#define	CONFIGSETTINGSTRING_H

#include <string>

class ConfigSettingString {
public:
    /// Registers an integer setting
    ConfigSettingString(const char* name, const char* synopsis, const std::string& initialValue);

    /// Assigns an integer value to the setting
    ConfigSettingString& operator=(const std::string& value);

    /// Returns the setting's value as integer
    operator char*(void) const;

    /// Tries to find a setting, returns a nullptr if no setting could be found
    static ConfigSettingString* FindSetting(const char* name);

private:
    void AddToList(void);

    ConfigSettingString* m_next;
    const char* m_name;
    const char* m_synopsis;
    std::string m_value;
};

namespace {

/*
 * No dynamic memory allocation, minimal pre-main work, no singletons are required.
 * This little trick allows us to make sure that we have a simple global linked list of 
 * ConfigSetting values where the memory is assured to be valid even if the actual
 * memory system isn't initialized yet. This is due the place these values are 
 * stored (either the .bss or .sdata segment )!!
 */
    static ConfigSettingString* head_s = nullptr;
    static ConfigSettingString* tail_s = nullptr;
}

#endif	/* CONFIGSETTINGSTRING_H */

