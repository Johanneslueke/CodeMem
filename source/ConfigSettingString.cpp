#include "../include/ConfigSettingString.h"

ConfigSettingString::ConfigSettingString(const char* name, const char* synopsis, const std::string& initialValue) {
    this->AddToList();
    m_name = name;
    m_synopsis = synopsis;
    m_value = initialValue;

}

void ConfigSettingString::AddToList() {
    if (head_s) {
        tail_s->m_next = this;
        tail_s = this;
    } else {
        head_s = this;
        tail_s = this;
    }

}

ConfigSettingString* ConfigSettingString::FindSetting(const char* name) {

    auto cursor = head_s;

    do {
        if (cursor != nullptr) {
            //TODO(hannes): if name is not found it still tries to access the next element. This is wrong
            if (cursor->m_name == name)
                return cursor;
            else
                cursor = cursor->m_next;
        }
    }
    while (cursor != tail_s);

    return nullptr;
}

ConfigSettingString& ConfigSettingString::operator=(const std::string& value) {
    m_value = value;

    return *this;
}

ConfigSettingString::operator char*() const {
    return (char*)m_value.c_str();
}
