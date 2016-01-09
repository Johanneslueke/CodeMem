/* 
 * File:   ConfigSettingInt.h
 * Author: johannes
 *
 * Created on 23 December 2015, 17:29
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 * 
 */

#ifndef CONFIGSETTINGINT_H
#define	CONFIGSETTINGINT_H

class ConfigSettingInt {
public:
    /// Registers an integer setting
    ConfigSettingInt(const char* name, const char* synopsis, int initialValue);

    /// Registers an integer setting, constraining it to the range [minValue, maxValue]
    ConfigSettingInt(const char* name, const char* synopsis, int initialValue, int minValue, int maxValue);

    /// Assigns an integer value to the setting
    ConfigSettingInt& operator=(int value);

    /// Returns the setting's value as integer
    operator int(void) const;

    /// Tries to find a setting, returns a nullptr if no setting could be found
    static ConfigSettingInt* FindSetting(const char* name);

private:
    void AddToList(void);

    ConfigSettingInt* m_next;
    const char* m_name;
    const char* m_synopsis;
    int m_value;
    int m_min;
    int m_max;
};

namespace {

/*
 * No dynamic memory allocation, minimal pre-main work, no singletons are required.
 * This little trick allows us to make sure that we have a simple global linked list of 
 * ConfigSetting values where the memory is assured to be valid even if the actual
 * memory system isn't initialized yet. This is due the place these values are 
 * stored (either the .bss or .sdata segment )!!
 */
    static ConfigSettingInt* head = nullptr;
    static ConfigSettingInt* tail = nullptr;
}

#endif	/* CONFIGSETTINGINT_H */

