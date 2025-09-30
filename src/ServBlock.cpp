/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 12:01:03 by njeanbou          #+#    #+#             */
/*   Updated: 2025/09/30 12:20:53 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServBlock.hpp"

namespace {
    const char* default_name                = "localhost";
    const char* default_root                = "www";
    const int default_client_max_body_size  = 4;
    const int   default_port                = 8080;
    const char* default_index               = "index.html";
    

    std::string build_folder_format(std::string &str) {
        if (str[str.size() - 1] == '/')
            return str;
        str += '/';
        return str;
    }

    bool parse_bool(std::string value) {
       if (!value.empty() && value == "on")
           return true;
       return false;
    }

    void print_location(std::ostream& o, t_location loc) {
       o << "  allow_methods :" << loc.allow_methods << std::endl;
       o << "  redirHTTP :" << loc.redirHTTP << std::endl;
       o << "  root :" << loc.root << std::endl;
       o << "  upload_enable :" << loc.upload_enable << std::endl;
       o << "  directory_listing :" << loc.directory_listing << std::endl;
       o << "  upload_enable :" << loc.upload_enable << std::endl;
       o << "  cgi_extension :" << loc.cgi_extension << std::endl;
       for (std::vector<std::string>::iterator it = loc.index.begin(); it != loc.index.end(); it++)
           o<< "  index : " << *it << std::endl;
    }

    /* root_compose:
    *   si alias et root -> error statu = false;
    *   si alias et !root -> root = alias
    *   si !alias et root -> root = root + name
    *   si !alias et !root -> !root
    *   verifier si le / est a la fin sinon l'ajouter 
    */
    std::string root_compose(std::map<std::string, std::string> &module, std::string name, bool& statu)
    {
        if ((!module["alias"].empty() && !module["root"].empty())) {
            statu = false;
            return std::string();
        }
        if ((module["alias"].empty() && module["root"].empty()))
            return std::string();
        if (!module["alias"].empty() && module["root"].empty())
            return build_folder_format(module["alias"]);
        if (module["alias"].empty() && !module["root"].empty())
        {
            name.erase(0, 1);
            std::string tmp = build_folder_format(module["root"]) + name;
            return build_folder_format(tmp);
        }
        return "/";
    }

    int parse_client_max_body_size(std::string &value) {
        if (value.empty())
            return default_client_max_body_size;

        std::string numberPart = value;
        char suffix = '\0';

        if (std::isalpha(static_cast<unsigned char>(value[value.size() - 1]))) {
            suffix = value[value.size() - 1];
            numberPart = value.substr(0, value.size() - 1);
        } else if (std::isalpha(static_cast<unsigned char>(value[0]))) {
            suffix = value[0];
            numberPart = value.substr(1);
        }

        for (size_t i = 0; i < numberPart.size(); ++i) {
            char c = numberPart[i];
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                std::cout << "Error : Invalid number in client_max_body_size." << std::endl;
                return -1;
            }
        }

        int number;
        std::istringstream iss(numberPart);
        if (!(iss >> number)) {
            std::cout << "Error : Conversion failed for client_max_body_size." << std::endl;
            return -1;
        }

        int multiplier = 1;
        switch (suffix) {
            case 'k':
            case 'K':
                multiplier = 1024;
                break;
            case 'm':
            case 'M':
                multiplier = 1024 * 1024;
                break;
            case 'g':
            case 'G':
                multiplier = 1024 * 1024 * 1024;
                break;
            case '\0':
                multiplier = 1;
                break;
            default:
                std::cout << "Error : Invalid suffix in client_max_body_size." << std::endl;
                return -1;
        }
        if (number > std::numeric_limits<int>::max() / multiplier) {
            std::cout << "Error : client_max_body_size too large." << std::endl;
            return -1;
        }
        return number * multiplier;
    }
}

///////CONSTRUCTORS///////

ServBlock::ServBlock() : name(""), root("")
{
    statu = true;
}

ServBlock::~ServBlock()
{
}

///////PRIVATE///////

bool ServBlock::copy_file(const char* src, const char* dst) const {
    std::cout << "Copy : " << src << " -> " << dst;
    std::ifstream in(src, std::ios::binary);
    std::ofstream out(dst, std::ios::binary);
    if (!in || !out) {
        return false;
        std::cout << "  [FAIL]" << std::endl;
    }
    char buffer[4096];
    while (in.read(buffer, sizeof(buffer))) {
        out.write(buffer, in.gcount());
    }
    out.write(buffer, in.gcount());
    std::cout << "  [SUCCES]" << std::endl;
    return true;
}

void ServBlock::copy_all_files(const char* srcDir, const char* dstDir) const {
    DIR* dir = opendir(srcDir);
    if (!dir) {
        std::cerr << "Error : can't open " << srcDir << " folder." << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string srcPath = std::string(srcDir) + "/" + entry->d_name;
        std::string dstPath = std::string(dstDir) + "/" + entry->d_name;

        struct stat st;
        if (stat(srcPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            copy_file(srcPath.c_str(), dstPath.c_str());
        }
    }
    closedir(dir);
}

void ServBlock::root_checker(const char* srcDir, const char* dstDir) {
    if (access(dstDir, F_OK) == 0) {
        copy_all_files(srcDir, dstDir);
    } else {
        root = default_root;
    }
}

void ServBlock::set_default(int overwrite)
{
    if (overwrite) {
        port.push_back(default_port);
        name = default_name;
        root = default_root;
        index.push_back(default_index);
    } else {
        if (port.empty())
            port.push_back(default_port);
        if (name.empty())
            name = default_name;
        if (root.empty())
            root = default_root;
        if (index.empty())
            index.push_back(default_index);
    }
}

void ServBlock::parse_error_page(std::string value) {
    int tmp[100];
    int i = 0;
    std::istringstream iss(value);
    while(i < 100 && iss >> tmp[i])
        i++;
    iss.clear();
    std::string page;
    iss >> page;
    if (i >= 100) {
        statu = false;
        std::cout << "Too much page error define on a line." << std::endl;
        return;
    }
    for (int j = 0; j < i; j++)
        error_page[tmp[j]] = page;
}

///////METHODES///////

int ServBlock::parse_ServBlock(std::ifstream &fd) {
    std::vector<std::string> keys;
    keys.reserve(6);
    keys.push_back("listen"); //port
    keys.push_back("server_name"); //name
    keys.push_back("root"); //root
    keys.push_back("index"); //index
    keys.push_back("error_page"); //error_page
    keys.push_back("client_max_body_size"); //client_max_body_size

    std::string line;
    bool in_server_block = true;
    std::map<std::string, std::string> server;

    while (std::getline(fd, line) && in_server_block)
    {
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == '#') continue;
        if (line.find("}") != std::string::npos) {
            in_server_block = false;
            continue;
        }
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if ((line.find("location") != std::string::npos) && (line.find("{") != std::string::npos)) {
            std::string loc_name;
            iss >> loc_name;
            parse_location(fd, locations[loc_name], loc_name); 
        }
        else if (std::find(keys.begin(), keys.end(), key) != keys.end()) {
            std::string value;
            std::getline(iss, value, ';');
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\"") + 1);
            if (key == "error_page")
                parse_error_page(value);
            else
                server[key] = value;
        }
    }
    name                    = server["server_name"];
    root                    = server["root"];
    client_max_body_size    = parse_client_max_body_size(server["client_max_body_size"]);
    if (client_max_body_size == -1)
        statu = false;
    std::istringstream iss(server["listen"]);
    int tmp;
    while(iss >> tmp)
        port.push_back(tmp);
    iss.str(server["index"]);
    iss.clear();
    std::string word;
    while(iss >> word)
        index.push_back(word);
    // set_default(0);
    return 1;
}

///////GETTER///////

bool ServBlock::get_statu() const { return statu; }
std::vector<int> ServBlock::get_port() const { return port; } 
std::string ServBlock::get_name() const { return name; }
std::string ServBlock::get_root() const { return root; }
int ServBlock::get_client_max_body_size() const { return client_max_body_size; }
std::vector<std::string> ServBlock::get_index() const { return index; }
std::map<int, std::string> ServBlock::get_error_page() const { return error_page; }
std::map<std::string, t_location> ServBlock::get_locations() const { return locations; }

///////LOCATION///////

void ServBlock::parse_location(std::ifstream &fd, t_location &loc, std::string name) {
    std::vector<std::string> keys;
    keys.reserve(9);
    keys.push_back("alias");            //root
    keys.push_back("allow_methods");    //allow_methods
    keys.push_back("return");           //redirHTTP
    keys.push_back("root");             //root
    keys.push_back("upload_store");     //upload_store
    keys.push_back("autoindex");        //directory_listing
    keys.push_back("upload_enable");    //upload_enable
    keys.push_back("cgi_extension");    //cgi_extension
    keys.push_back("index");            //index

    loc.loc = name;

    std::string line;
    std::map<std::string, std::string> module;
    while (getline(fd, line)) {
        if (line.find("}") != std::string::npos) {
            break;
        }
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (std::find(keys.begin(), keys.end(), key) != keys.end()) {
            std::string value;
            std::getline(iss, value, ';');
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\"") + 1);
            module[key] = value;
        }
    }
    loc.allow_methods       = module["allow_methods"];
    loc.redirHTTP           = module["return"];
    loc.upload_store        = module["upload_store"];
    loc.allow_methods       = module["allow_methods"];
    loc.root                = root_compose(module, name, statu);
    loc.directory_listing   = parse_bool(module["autoindex"]);
    loc.upload_enable       = parse_bool(module["upload_enable"]);
    loc.cgi_extension       = (module["cgi_extension"].find(".php") != std::string::npos);

    std::istringstream iss(module["index"]);
    std::string word;
    while(iss >> word)
        loc.index.push_back(word);
}

std::ostream& operator<<(std::ostream& o, ServBlock& SBlock) {
    o << "statu :" << SBlock.get_statu() << std::endl;
    o << "name :" << SBlock.get_name() << std::endl;
    o << "root :" << SBlock.get_root() << std::endl;
    o << "client_max_body_size :" << SBlock.get_client_max_body_size() << std::endl;

    std::vector<int> port = SBlock.get_port();
    for (std::vector<int>::const_iterator it = port.begin(); it != port.end(); ++it)
        o << "port : " << *it << std::endl;
    std::vector<std::string> index = SBlock.get_index();
    for (std::vector<std::string>::const_iterator it = index.begin(); it != index.end(); ++it)
        o << "index : " << *it << std::endl;
    std::map<int, std::string> error_page = SBlock.get_error_page();
    for (std::map<int, std::string>::const_iterator it = error_page.begin(); it != error_page.end(); ++it)
        o << "error_page " << it->first << " " << it->second << std::endl;
    std::map<std::string, t_location> locations = SBlock.get_locations();
    for (std::map<std::string, t_location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        o << "LOCATIONS " << it->first << std::endl;
        print_location(o, it->second);
    }
    return o;
}