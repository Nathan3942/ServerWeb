/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 15:11:22 by njeanbou          #+#    #+#             */
/*   Updated: 2025/10/13 09:54:20 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServBlock.hpp"

namespace {
    const int default_client_max_body_size  = 4;
    

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

    bool isInteger(const std::string& s) {
        if (s.empty()) return false;
        char* end;
        const char* str = s.c_str();
        std::strtol(str, &end, 10);
        return (*end == '\0');
    }

    void print_location(std::ostream& o, t_location loc) {
        o << "  allow_methods :" << loc.allow_methods << std::endl;
        o << "  redirCode :" << loc.redirCode << std::endl;
        o << "  redirHTTP :" << loc.redirHTTP << std::endl;
        o << "  root :" << loc.root << std::endl;
        o << "  upload_enable :" << loc.upload_enable << std::endl;
        o << "  directory_listing :" << loc.directory_listing << std::endl;
        o << "  upload_enable :" << loc.upload_enable << std::endl;
        o << "  cgi_extension :" << loc.cgi_extension << std::endl;
        for (std::map<int, std::string>::const_iterator it = loc.error_page.begin(); it != loc.error_page.end(); ++it)
            o << "  error_page " << it->first << " " << it->second << std::endl;
        for (std::vector<std::string>::iterator it = loc.index.begin(); it != loc.index.end(); it++)
            o<< "  index : " << *it << std::endl;
    }
}

///////CONSTRUCTORS///////

ServBlock::ServBlock()
{
}

ServBlock::~ServBlock()
{
}

///////PRIVATE///////

void ServBlock::parse_HTTP(int &code, std::string &redir, std::string prompt) {
    if (prompt.empty()) {
        code = 0;
        redir = "";
        return;
    }
    std::istringstream iss(prompt);
    std::string word;
    iss >> word;
    if (!isInteger(word))
        throw redirHTTPException();
    char *end;
    const char* str = word.c_str();
    long nb = strtol(str, &end, 10);
    if (nb > 1000)
        throw redirHTTPException();
    code = nb;
    iss >> redir;
    if (iss >> word)
        throw redirHTTPException();
}


/* root_compose:
*   si alias et root -> exception;
*   si alias et !root -> root = alias
*   si !alias et root -> root = root + name
*   si !alias et !root -> !root
*   verifier si le / est a la fin sinon l'ajouter 
*/
std::string ServBlock::root_compose(std::map<std::string, std::string> &module, std::string name)
{
    if ((!module["alias"].empty() && !module["root"].empty())) {
        throw rootException();
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

int ServBlock::parse_client_max_body_size(std::string &value) {
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
        if (!std::isdigit(static_cast<unsigned char>(c)))
            throw invalidNumberException();
    }

    int number;
    std::istringstream iss(numberPart);
    if (!(iss >> number))
        throw conversionFailedException();
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
            throw invalidSuffixException();
    }
    if (number > std::numeric_limits<int>::max() / multiplier)
        throw TooLargeExeption();
    return number * multiplier;
}

void ServBlock::parse_error_page(const std::string &value, std::map<int, std::string> &error_pages)
{
    std::istringstream iss(value);
    std::vector<int> codes;
    std::string token;

    while (iss >> token)
    {
        if (!token.empty() && token[0] == '/')
        {
            for (size_t i = 0; i < codes.size(); ++i)
                error_pages[codes[i]] = token;
            return;
        }
        int code;
        std::istringstream(token) >> code;
        codes.push_back(code);
    }
    throw std::runtime_error("Config: Invalid error_page directive (missing path).");
}

///////METHODES///////

int ServBlock::parse_ServBlock(std::ifstream &fd) {
    std::vector<std::string> keys;
    keys.reserve(6);
    keys.push_back("listen"); //port
    keys.push_back("server_name"); //name
    keys.push_back("root"); //root
    keys.push_back("index"); //index
    keys.push_back("error_page");       //error_page
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
                parse_error_page(value, error_page);
            else
                server[key] = value;
        }
    }
    if (server["server_name"].empty())
        name = "default";
    name                    = server["server_name"];
    root                    = server["root"];
    client_max_body_size    = parse_client_max_body_size(server["client_max_body_size"]);
    std::istringstream iss(server["listen"]);
    int tmp;
    while(iss >> tmp)
        port.push_back(tmp);
    iss.str(server["index"]);
    iss.clear();
    std::string word;
    while(iss >> word)
        index.push_back(word);
    return 1;
}

///////GETTER///////

std::vector<int> ServBlock::get_port() const { return port; } 
std::string ServBlock::get_name() const { return name; }
std::string ServBlock::get_root() const { return root; }
int ServBlock::get_client_max_body_size() const { return client_max_body_size; }
std::vector<std::string> ServBlock::get_index() const { return index; }
const std::map<int, std::string> ServBlock::get_error_page() const { return error_page; }
std::map<std::string, t_location> ServBlock::get_locations() const { return locations; }

///////LOCATION///////

void ServBlock::parse_location(std::ifstream &fd, t_location &loc, std::string name) {
    std::vector<std::string> keys;
    keys.reserve(10);
    keys.push_back("alias");            //root
    keys.push_back("allow_methods");    //allow_methods
    keys.push_back("return");           //redirHTTP
    keys.push_back("root");             //root
    keys.push_back("upload_store");     //upload_store
    keys.push_back("autoindex");        //directory_listing
    keys.push_back("upload_enable");    //upload_enable
    keys.push_back("cgi_extension");    //cgi_extension
    keys.push_back("error_page");       //error_page
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
            if (key == "error_page")
                parse_error_page(value, loc.error_page);
            else
                module[key] = value;
        }
    }
    loc.allow_methods       = module["allow_methods"];
    parse_HTTP(loc.redirCode, loc.redirHTTP, module["return"]);
    loc.upload_store        = module["upload_store"];
    loc.allow_methods       = module["allow_methods"];
    loc.root                = root_compose(module, name);
    loc.directory_listing   = parse_bool(module["autoindex"]);
    loc.upload_enable       = parse_bool(module["upload_enable"]);
    loc.cgi_extension       = (module["cgi_extension"].find(".php") != std::string::npos);

    std::istringstream iss(module["index"]);
    std::string word;
    while(iss >> word)
        loc.index.push_back(word);
}

std::ostream& operator<<(std::ostream& o, ServBlock& SBlock) {
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
