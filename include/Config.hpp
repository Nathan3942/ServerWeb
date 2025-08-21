/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichpakov <ichpakov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:33 by ichpakov          #+#    #+#             */
/*   Updated: 2025/06/20 12:00:34 by ichpakov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//PARSING REQUETE HTTP

#pragma once

#include "server.hpp"
#include <vector>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring> 

typedef struct {
    std::string allow_methods;
    std::string redirHTTP; // status_code + taget_url
    std::string root;
    std::string upload_store;
    bool directory_listing;
    bool upload_enable;
    bool cgi_extension;
    std::vector<std::string> index;
}   location;

class Config {
    private :
        const char *file_name; //name of config file
        std::vector<int> port; //each listening port
        std::string name; //server name 
        std::string root; //dedfault server root
        std::vector <std::string> index; //index list in ascending order of importance
        int client_max_body_size;
        std::map<int, std::string> error_page; // int is error code and string is associate file
        std::map<std::string, location> r_path; //name of location + struct location

        bool copy_file(const char* src, const char* dst) const;
        void copy_all_files(const char* srcDir, const char* dstDir) const;
        void root_checker(const char* srcDdir, const char* dstDir);
        void set_default(int overwrite);
        
    public :
        Config(const char *str);
        ~Config();

        int parse_config();

        std::vector<int> get_port() const; 
        std::string get_name() const;
        std::string get_root() const;
        std::string get_error() const;
        std::vector<std::string> get_index() const;
        int	get_client_max_body_size() const;
		std::map<std::string, location> get_path_rules() const;
};

/* CONSIGNES :
In the configuration file, you should be able to:
    Define all the interface:port pairs on which your server will listen to
    (defining multiple websites served by your program).
    Set up default error pages.
    Set the maximum allowed size for client request bodies.
Specify rules or configurations on a URL/route (no regex required here),
for a website, among the following:
    List of accepted HTTP methods for the route.
    HTTP redirection.
    Directory where the requested file should be located (e.g., if URL /kapouet
    is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for
    /tmp/www/pouic/toto/pouet).
    Enabling or disabling directory listing.
    Default file to serve when the requested resource is a directory.
    Uploading files from the clients to the server is authorized,
    and storage location is provided.
    Execution of CGI, based on file extension (for example .php).
*/

/* valeurs :
*   multi index
*   port et leurs address
*   server name
*   page d'erreur
*   root par defaut si les locations ne sont pas set
*   location definit les regles a appliquer sur les urls qui contiennent la location.
*
*   les regles possibles sont :
*       -allow_methoddes GET POST;                          string          ----|
*       -redir HTTP : return 301 http://exemple.com;        string              |
*       -racine des fichiers : root /sddfsfs;               string              |
*       -fichier defaut index pour cette localisation       vector              |   struct des
*       -upload_anable on;                                  bool                |   localisations
*       -Dossier stockage upload : upload_store /uploads;   string              |
*       ? cgi_extension .php + cgi_path /php-cgi            bool | string   ----|
*/
