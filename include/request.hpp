/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:33 by ichpakov          #+#    #+#             */
/*   Updated: 2025/08/08 16:08:15 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//PARSING REQUETE HTTP

#include <iostream>
#include <string>

#pragma once

#include "CGI.hpp"
#include "server.hpp"

#define MAX_BODY_SIZE (1 * 1024 * 1024)
#define MAX_URI_LENGTH 4096

class CGI;

class Request
{
    private :
        CGI *cgi;
        std::string raw_request;
        std::string path;
        std::string method;
        std::string body;
        int error_code;

        void	error_check(std::string root);

    public :
        Request();
        Request(int client_fd, const std::string index, const std::string root);
        Request(const Request& copy);
        ~Request();
        std::string receive_request(int client_fd);
        std::string extract_path(const std::string& raw, const std::string index);

        std::string get_path() const;
        std::string get_raw_request() const;
        std::string get_method() const;
        std::string get_body() const;
		int	get_error_code() const;
        CGI *get_cgi() const;

		void	set_error_code(int error);
};
