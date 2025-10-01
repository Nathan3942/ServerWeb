/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:33 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/01 15:05:07 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//PARSING REQUETE HTTP

#include <iostream>
#include <string>

#pragma once

#include "CGI.hpp"
// #include "server.hpp"
#include "location.hpp"
#include "ServBlock.hpp"

#define MAX_BODY_SIZE (1 * 1024 * 1024)
#define MAX_URI_LENGTH 4096

class CGI;
class Config;
class ServBlock;
// struct t_location;

class Request
{
    private :
        CGI *cgi;
        std::string raw_request;
        std::string path;
        std::string method;
        std::string body;
        int error_code;
        ServBlock   *s_block;
        t_location p_rules;
        bool    dir_lst;

        void	error_check();
        void    rules_error(t_location rules);

    public :
        Request();
        Request(int client_fd, Config& conf);
        Request(const Request& copy);
        ~Request();
        std::string receive_request(int client_fd);
        ServBlock   *extract_block(Config& conf);
        int extract_port();
        std::string extract_path(const std::string& raw);
        t_location    extract_location();
        void    setup_full_path();



        std::string get_path() const;
        std::string get_raw_request() const;
        std::string get_method() const;
        std::string get_body() const;
		int	get_error_code() const;
        CGI *get_cgi() const;
        t_location get_path_rules() const;
        ServBlock   get_serv_block() const;
        bool    get_dir_lst() const;



		void	set_error_code(int error);
        void    set_dir_lst(bool set);
};
