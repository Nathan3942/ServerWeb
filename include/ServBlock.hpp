/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 12:00:03 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/28 15:05:36 by njeanbou         ###   ########.fr       */
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
        std::string name; //server name 
        std::string root; //default server root
        int client_max_body_size; //value in octet
        std::vector<int> port; //each listening port
        std::vector <std::string> index; //index list in ascending order of importance
        std::map<int, std::string> error_page; // int is error code and string is associate file
        std::map<std::string, t_location> locations; //name of location + struct location

        void parse_HTTP(int &code, std::string &redir, std::string str);
        std::string root_compose(std::map<std::string, std::string> &module, std::string name);
        int parse_client_max_body_size(std::string &value);        
        
    public :
        class NoNameException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Server should have a name.";
            }
        };
        class confSyntaxException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Syntax error on config file.";
            }
        };
        class redirHTTPException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Bad syntax for redirHTTP.";
            }
        };
        class rootException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: alias and root define on the same bloc.";
            }
        };
        class tooMuchPageErrorException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Too much page error define on a line.";
            }
        };
        class invalidNumberException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Invalid number in client_max_body_size.";
            }
        };
        class conversionFailedException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Conversion failed for client_max_body_size.";
            }
        };
        class invalidSuffixException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: Invalid suffix in client_max_body_size.";
            }
        };
        class TooLargeExeption : public std::exception {
            virtual const char* what() const throw() {
                return "Conf: client_max_body_size too large.";
            }
        };

        ServBlock();
        ~ServBlock();

        int parse_ServBlock(std::ifstream &fd);

        //getters
        std::vector<int> get_port() const;
        std::string get_name() const;
        std::string get_root() const;
        int get_client_max_body_size() const;
        std::vector<std::string> get_index() const;
        const std::map<int, std::string> get_error_page() const;
        std::map<std::string, t_location> get_locations() const;


        // bool get_statu() const;
        void parse_location(std::ifstream &fd, t_location &loc, std::string name);
        void parse_error_page(const std::string &value, std::map<int, std::string> &error_pages);
};

std::ostream& operator<<(std::ostream& o, ServBlock& SBlock);
