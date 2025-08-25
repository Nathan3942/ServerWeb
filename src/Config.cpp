#include "../include/Config.hpp"

// namespace
// {
//     const char* default_root = "www";
//     const int default_port = 8080;
//     const char* default_name = "localhost";
//     const char* default_index = "index.html";
//     const char* default_error = "404.html";
// }

///////CONSTRUCTORS///////

Config::Config(const char *str) {
    (void)str; // on ignore pour l'instant le nom de fichier

    // Nom du "fichier de config" (factice)
    this->file_name = "default.conf";

    // Ports d'écoute
    this->port.push_back(8080);
    this->port.push_back(8081);

    // Nom du serveur
    this->name = "MyTestServer";

    // Racine du serveur
    this->root = "./www";

    // Index par défaut
    this->index.push_back("index.html");
    this->index.push_back("index.php");

    // Taille max du body client (1 Mo ici)
    this->client_max_body_size = 1024 * 1024;

    // Pages d’erreur personnalisées
    this->error_page[404] = "./404.html";
    this->error_page[500] = "./500.html";

    // Exemple de location "/"
    t_location loc_root;
    loc_root.loc = "/";
    loc_root.allow_methods = "GET POST";
    loc_root.redirHTTP = ""; // pas de redirection
    loc_root.root = "./www";
    loc_root.upload_store = "./uploads";
    loc_root.directory_listing = true;
    loc_root.upload_enable = true;
    loc_root.cgi_extension = false;
    loc_root.index.push_back("index.html");
    loc_root.index.push_back("index.php");

    // Exemple de location "/upload"
    t_location loc_upload;
    loc_upload.loc = "/upload";
    loc_upload.allow_methods = "POST";
    loc_upload.redirHTTP = "";
    loc_upload.root = "./www/uploads";
    loc_upload.upload_store = "./uploads";
    loc_upload.directory_listing = false;
    loc_upload.upload_enable = true;
    loc_upload.cgi_extension = false;
    loc_upload.index.push_back("upload.html");

    // Ajout des locations dans la map
    this->r_path["/"] = loc_root;
    this->r_path["/upload"] = loc_upload;
}

// Config::Config(const char *str) : file_name(str)
// {
//     parse_config();
// }

Config::~Config()
{
}

///////PRIVATE///////

// bool Config::copy_file(const char* src, const char* dst) const
// {
//     std::cout << "Copy : " << src << " -> " << dst;
//     std::ifstream in(src, std::ios::binary);
//     std::ofstream out(dst, std::ios::binary);
//     if (!in || !out) {
//         return false;
//         std::cout << "  [FAIL]" << std::endl;
//     }
//     char buffer[4096];
//     while (in.read(buffer, sizeof(buffer))) {
//         out.write(buffer, in.gcount());
//     }
//     out.write(buffer, in.gcount());
//     std::cout << "  [SUCCES]" << std::endl;
//     return true;
// }

// void Config::copy_all_files(const char* srcDir, const char* dstDir) const
// {
//     DIR* dir = opendir(srcDir);
//     if (!dir)
//     {
//         std::cerr << "Error : can't open " << srcDir << " folder." << std::endl;
//         return;
//     }

//     struct dirent* entry;
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
//             continue;

//         std::string srcPath = std::string(srcDir) + "/" + entry->d_name;
//         std::string dstPath = std::string(dstDir) + "/" + entry->d_name;

//         struct stat st;
//         if (stat(srcPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
//         {
//             copy_file(srcPath.c_str(), dstPath.c_str());
//         }
//     }
//     closedir(dir);
// }

// void Config::root_checker(const char* srcDir, const char* dstDir)
// {
//     if (access(dstDir, F_OK) == 0)
//     {
//         copy_all_files(srcDir, dstDir);
//     } else
//     {
//         root = default_root;
//     }
// }

// void Config::set_default(int overwrite)
// {
//     if (overwrite)
//     {
//         port.push_back(default_port);
//         name = default_name;
//         root = default_root;
//         error = default_error;
//         index.push_back(default_index);
//     }
//     else
//     {
//         if (port.empty())
//             port.push_back(default_port);
//         if (name.empty())
//             name = default_name;
//         if (root.empty())
//             root = default_root;
//         if (error.empty())
//             error = default_error;
//         if (index.empty())
//             index.push_back(default_index);
//     }
// }

// ///////METHODES///////

// int Config::parse_config() {
//     std::vector<std::string> keys;
//     keys.push_back("listen");
//     keys.push_back("server_name");
//     keys.push_back("root");
//     keys.push_back("index");
//     keys.push_back("error_page 404");

//     std::ifstream fd(file_name);
//     if (!fd.is_open())
//     {
//         set_default(1);
//         std::cerr << "Error : " << file_name << " can't be open." << std::endl;
//         return 0;
//     }

//     std::string line;
//     bool in_server_block = false;
//     int server_block_index = 0;
//     std::map<std::string, std::string> server;

//     while (std::getline(fd, line) && server_block_index < 2)
//     {
//         line.erase(0, line.find_first_not_of(" \t"));
//         if (line.empty() || line[0] == '#') continue;
//         if (line.find("server {") != std::string::npos)
//         {
//             in_server_block = true;
//             server_block_index++;
//             continue;
//         }
//         if (in_server_block)
//         {
//             if (line.find("}") != std::string::npos)
//             {
//                 in_server_block = false;
//                 continue;
//             }

//             std::istringstream iss(line);
//             std::string key;
//             iss >> key;
//             if (std::find(keys.begin(), keys.end(), key) != keys.end())
//             {
//                 std::string value;
//                 std::getline(iss, value, ';');
//                 value.erase(0, value.find_first_not_of(" \t\""));
//                 value.erase(value.find_last_not_of(" \t\"") + 1);
//                 if (server.find(key) != server.end())
//                 {
//                     set_default(1);
//                     std::cerr << "Error : " << key << " is defined several times" << std::endl;
//                     return 0;
//                 }
//                 else 
//                     server[key] = value;
//             }
//         }
//     }
//     fd.close();
//     std::istringstream iss(server["listen"]);
//     int tmp;
//     while(iss >> tmp)
//         port.push_back(tmp);
//     name = server["server_name"];
//     root = server["root"];
//     error = server["error_page 404"];
//     iss.str(server["index"]);
//     iss.clear();
//     std::string word;
//     while(iss >> word)
//         index.push_back(word);
//     root_checker(default_root, root.c_str());
//     set_default(0);
//     return 1;
// }

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

std::string Config::get_error(int code) const
{
    std::map<int, std::string>::const_iterator it = error_page.find(code);
    if (it != error_page.end())
        return it->second;
    return "";
}

std::vector<std::string> Config::get_index() const
{
    return index;
}

int Config::get_client_max_body_size() const
{
    return (client_max_body_size);
}

std::map<std::string, t_location> Config::get_path_rules() const
{
    return (r_path);
}