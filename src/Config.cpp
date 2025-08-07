#include "../include/Config.hpp"

namespace
{
    const char* default_root = "www";
    const int default_port = 8080;
    const char* default_name = "localhost";
    const char* default_index = "index.html";
    const char* default_error = "404.html";
}

///////CONSTRUCTORS///////

Config::Config(const char *str) : file_name(str)
{
    parse_config();
}

Config::~Config()
{
}

///////PRIVATE///////

bool Config::copy_file(const char* src, const char* dst) const
{
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

void Config::copy_all_files(const char* srcDir, const char* dstDir) const
{
    DIR* dir = opendir(srcDir);
    if (!dir)
    {
        std::cerr << "Error : can't open " << srcDir << " folder." << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string srcPath = std::string(srcDir) + "/" + entry->d_name;
        std::string dstPath = std::string(dstDir) + "/" + entry->d_name;

        struct stat st;
        if (stat(srcPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        {
            copy_file(srcPath.c_str(), dstPath.c_str());
        }
    }
    closedir(dir);
}

void Config::root_checker(const char* srcDir, const char* dstDir)
{
    if (access(dstDir, F_OK) == 0)
    {
        copy_all_files(srcDir, dstDir);
    } else
    {
        root = default_root;
    }
}

void Config::set_default(int overwrite)
{
    if (overwrite)
    {
        port.push_back(default_port);
        name = default_name;
        root = default_root;
        error = default_error;
        index.push_back(default_index);
    }
    else
    {
        if (port.empty())
            port.push_back(default_port);
        if (name.empty())
            name = default_name;
        if (root.empty())
            root = default_root;
        if (error.empty())
            error = default_error;
        if (index.empty())
            index.push_back(default_index);
    }
}

///////METHODES///////

int Config::parse_config() {
    std::vector<std::string> keys;
    keys.push_back("listen");
    keys.push_back("server_name");
    keys.push_back("root");
    keys.push_back("index");
    keys.push_back("error_page 404");

    std::ifstream fd(file_name);
    if (!fd.is_open())
    {
        set_default(1);
        std::cerr << "Error : " << file_name << " can't be open." << std::endl;
        return 0;
    }

    std::string line;
    bool in_server_block = false;
    int server_block_index = 0;
    std::map<std::string, std::string> server;

    while (std::getline(fd, line) && server_block_index < 2)
    {
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == '#') continue;
        if (line.find("server {") != std::string::npos)
        {
            in_server_block = true;
            server_block_index++;
            continue;
        }
        if (in_server_block)
        {
            if (line.find("}") != std::string::npos)
            {
                in_server_block = false;
                continue;
            }

            std::istringstream iss(line);
            std::string key;
            iss >> key;
            if (std::find(keys.begin(), keys.end(), key) != keys.end())
            {
                std::string value;
                std::getline(iss, value, ';');
                value.erase(0, value.find_first_not_of(" \t\""));
                value.erase(value.find_last_not_of(" \t\"") + 1);
                if (server.find(key) != server.end())
                {
                    set_default(1);
                    std::cerr << "Error : " << key << " is defined several times" << std::endl;
                    return 0;
                }
                else 
                    server[key] = value;
            }
        }
    }
    fd.close();
    std::istringstream iss(server["listen"]);
    int tmp;
    while(iss >> tmp)
        port.push_back(tmp);
    name = server["server_name"];
    root = server["root"];
    error = server["error_page 404"];
    iss.str(server["index"]);
    iss.clear();
    std::string word;
    while(iss >> word)
        index.push_back(word);
    root_checker(default_root, root.c_str());
    set_default(0);
    return 1;
}

///////GETTER///////

std::vector<int> Config::get_port() const
{
    return port;
} 

std::string Config::get_name() const
{
    return name;
}

std::string Config::get_root() const
{
    return root;
}

std::string Config::get_error() const
{
    return error;
}

std::vector<std::string> Config::get_index() const
{
    return index;
}

