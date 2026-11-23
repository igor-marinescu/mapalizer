//******************************************************************************
// Configuration File
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "ConfFile.h"

#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <QFile>
#include <QSaveFile>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
// Replace line breaks, they would break the "one entry per line" format
//******************************************************************************
static std::string singleLine(const std::string & str){
    std::string out(str);
    for(std::string::size_type i = 0; i < out.size(); ++i)
        if((out[i] == '\r') || (out[i] == '\n'))
            out[i] = ' ';
    return out;
}

//******************************************************************************
ConfFile::ConfFile(){
}

//******************************************************************************
ConfFile::ConfFile(const QString & filename){
    load(filename);
}

//******************************************************************************
bool ConfFile::load(const QString & filename){

    varMap.clear();
    this->filename = filename;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray content = file.readAll();
    std::istringstream istr(std::string(content.constData(), content.size()));

    std::string line;
    std::string::size_type pos1, pos2;

    // Extract all variables/lines from file
    while(std::getline(istr, line)){

        trimLineEnd(line);

        // Get category
        pos1 = line.find(':');
        if(pos1 == std::string::npos)
            continue;   // No category in this line, ignore it
        std::string category = trim(line.substr(0, pos1));

        // Get name
        pos2 = line.find('=', pos1 + 1);
        if(pos2 == std::string::npos)
            continue;   // No name in this line, ignore it
        std::string name = trim(line.substr(pos1 + 1, pos2 - pos1 - 1));

        // Get value (kept as is, may contain blanks)
        varMap[category][name] = line.substr(pos2 + 1);
    }

    return true;
}

//******************************************************************************
bool ConfFile::save() const{

    if(filename.isEmpty())
        return false;

    std::ostringstream ostr;
    for(MapCatName::const_iterator cat = varMap.begin(); cat != varMap.end(); ++cat){
        for(MapNameValue::const_iterator var = cat->second.begin(); var != cat->second.end(); ++var){
            ostr << cat->first << ":" << var->first << "=" << var->second << "\n";
        }
    }
    std::string content = ostr.str();

    // QSaveFile writes to a temporary file and replaces the target only on
    // success (no corrupt configuration if writing fails)
    QSaveFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        DBG_ERROR("ConfFile::save cannot open " << filename.toStdString());
        return false;
    }
    if(file.write(content.c_str(), (qint64)content.size()) != (qint64)content.size()){
        DBG_ERROR("ConfFile::save write error " << filename.toStdString());
        file.cancelWriting();
        return false;
    }
    return file.commit();
}

//******************************************************************************
const std::string * ConfFile::find(const std::string & cat, const std::string & name) const{

    MapCatName::const_iterator it = varMap.find(cat);
    if(it == varMap.end())
        return nullptr;

    MapNameValue::const_iterator it2 = it->second.find(name);
    if(it2 == it->second.end())
        return nullptr;

    return &it2->second;
}

//******************************************************************************
bool ConfFile::exists(const std::string & cat) const{
    return (varMap.find(cat) != varMap.end());
}

//******************************************************************************
bool ConfFile::exists(const std::string & cat, const std::string & name) const{
    return find(cat, name) != nullptr;
}

//******************************************************************************
void ConfFile::set(const std::string & cat, const std::string & name, int val){
    std::ostringstream ss;
    ss << val;
    varMap[cat][name] = ss.str();
}

//******************************************************************************
void ConfFile::set_str(const std::string & cat, const std::string & name, const std::string & val){
    varMap[cat][name] = singleLine(val);
}

//******************************************************************************
void ConfFile::set_qstr(const std::string & cat, const std::string & name, const QString & qstr){
    set_str(cat, name, qstr.toStdString());
}

//******************************************************************************
bool ConfFile::get(const std::string & cat, const std::string & name, int * val) const{

    const std::string * str = find(cat, name);
    if(str == nullptr)
        return false;

    std::string txt = trim(*str);
    if(txt.empty())
        return false;

    char * end = nullptr;
    errno = 0;
    long lval = strtol(txt.c_str(), &end, 10);
    if((errno != 0) || (end == nullptr) || (*end != '\0')
    || (lval < INT_MIN) || (lval > INT_MAX))
        return false;   // not a (valid) number

    if(val)
        *val = (int)lval;

    return true;
}

//******************************************************************************
int ConfFile::get_or(const std::string & cat, const std::string & name, int default_value) const{

    int val;
    if(get(cat, name, &val))
        return val;
    return default_value;
}

//******************************************************************************
bool ConfFile::get_str(const std::string & cat, const std::string & name, std::string * val) const{

    const std::string * str = find(cat, name);
    if(str == nullptr)
        return false;

    if(val)
        *val = *str;

    return true;
}

//******************************************************************************
QString ConfFile::get_qstr(const std::string & cat, const std::string & name, const QString & def) const{

    const std::string * str = find(cat, name);
    if(str == nullptr)
        return def;

    return QString::fromStdString(*str);
}

//******************************************************************************
void ConfFile::clear(){
    varMap.clear();
}

//******************************************************************************
void ConfFile::clear(const std::string & cat){
    varMap.erase(cat);
}

//******************************************************************************
void ConfFile::clear(const std::string & cat, const std::string & name){

    MapCatName::iterator it = varMap.find(cat);
    if(it == varMap.end())
        return;

    it->second.erase(name);
}

//******************************************************************************
