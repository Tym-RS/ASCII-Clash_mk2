#ifndef ASCII_CLASH_ERRHELPER_H
#define ASCII_CLASH_ERRHELPER_H

#define ERR_PARAM std::string *err = nullptr
#define SET_ERR(text) if(err) *err = text

#endif
