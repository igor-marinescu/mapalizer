//******************************************************************************
// Configuration File
//
// Text file with one "category:name=value" entry per line.
//
// Igor Marinescu
//******************************************************************************
#ifndef CONFFILE_H
#define CONFFILE_H

//******************************************************************************
// Includes
//******************************************************************************
#include <map>
#include <string>
#include <QString>

//******************************************************************************
//******************************************************************************
class ConfFile{

public:

    ConfFile();
    explicit ConfFile(const QString & filename);

    // Load the file (previous content is cleared). The file name is
    // remembered for save(). Returns false if the file could not be read.
    bool load(const QString & filename);

    // Save to the remembered file name. Returns false on error.
    bool save() const;

    const QString & fileName() const { return filename; }
    void setFileName(const QString & name) { filename = name; }

    bool exists(const std::string & cat) const;
    bool exists(const std::string & cat, const std::string & name) const;

    void set(const std::string & cat, const std::string & name, int val);
    void set_str(const std::string & cat, const std::string & name, const std::string & val);
    void set_qstr(const std::string & cat, const std::string & name, const QString & qstr);

    // Returns true if the value exists and is a valid integer
    bool get(const std::string & cat, const std::string & name, int * val) const;
    // Returns the value or 'default_value' if missing/invalid
    int  get_or(const std::string & cat, const std::string & name, int default_value) const;

    bool get_str(const std::string & cat, const std::string & name, std::string * val = nullptr) const;
    QString get_qstr(const std::string & cat, const std::string & name,
                     const QString & def = QString()) const;

    void clear();
    void clear(const std::string & cat);
    void clear(const std::string & cat, const std::string & name);

private:

    typedef std::map<std::string, std::string> MapNameValue;
    typedef std::map<std::string, MapNameValue> MapCatName;

    // Returns the stored value or nullptr if not found
    const std::string * find(const std::string & cat, const std::string & name) const;

    MapCatName varMap;
    QString filename;
};

//******************************************************************************
#endif // CONFFILE_H
