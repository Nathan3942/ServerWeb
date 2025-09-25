/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 12:00:03 by njeanbou          #+#    #+#             */
/*   Updated: 2025/09/25 13:22:04 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "server.hpp"
#include "location.hpp"

#include <map>

/*location definit les regles a appliquer sur les urls qui contiennent la location.
*   les regles possibles sont :
*       -allow_methoddes GET POST;                          string          ----|
*       -redir HTTP : return 301 http://exemple.com;        string              |
*       -racine des fichiers : root /sddfsfs;               string              |
*       -fichier defaut index pour cette localisation       vector              |   struct des
*       -upload_anable on;                                  bool                |   localisations
*       -Dossier stockage upload : upload_store /uploads;   string              |
*       ? cgi_extension .php + cgi_path /php-cgi            bool | string   ----|
*/

class ServBlock {
    private :
        bool statu; //true for valide and false for invalide configuration file
        std::string name; //server name 
        std::string root; //default server root
        int client_max_body_size; //value in octet
        std::vector<int> port; //each listening port
        std::vector <std::string> index; //index list in ascending order of importance
        std::map<int, std::string> error_page; // int is error code and string is associate file
        std::map<std::string, t_location> locations; //name of location + struct location

        bool copy_file(const char* src, const char* dst) const;
        void copy_all_files(const char* srcDir, const char* dstDir) const;
        void root_checker(const char* srcDdir, const char* dstDir);
        void set_default(int overwrite);

        
        
    public :
        ServBlock();
        ServBlock(std::string name);
        ~ServBlock();

        int parse_ServBlock(std::ifstream &fd);

        //getters
        bool get_statu() const;
        std::vector<int> get_port() const; 
        std::string get_name() const;
        std::string get_root() const;
        int get_client_max_body_size() const;
        std::vector<std::string> get_index() const;
        std::map<int, std::string> get_error_page() const;
        std::map<std::string, t_location> get_locations() const;


        // bool get_statu() const;
        void parse_location(std::ifstream &fd, t_location &loc, std::string name);
        void parse_error_page(std::string value);

};

std::ostream& operator<<(std::ostream& o, ServBlock& SBlock);
