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
#include <dirent.h>
#include <sys/stat.h>

class Config {
    private :
        const char *file_name;
        std::vector<int> port;
        std::string name;
        std::string root;
        std::string error;
        std::vector <std::string> index;
        // int client_max_body_size;
        // int keepalive_timeout;
    
        bool copy_file(const char* src, const char* dst) const;
        void copy_all_files(const char* srcDir, const char* dstDir) const;
        void root_checker(const char* srcDdir, const char* dstDir);
        void set_default(int overwrite);
        // std::vector<std::string> split_path(const std::string &path);
        
    public :
        Config(const char *str);
        ~Config();

        int parse_config();
        // int creat_root();
        // int creat_error_log();

        std::vector<int> get_port() const; 
        std::string get_name() const;
        std::string get_root() const;
        std::string get_error() const;
        std::vector<std::string> get_index() const;
};
