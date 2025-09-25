/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:48:05 by ichpakov          #+#    #+#             */
/*   Updated: 2025/09/25 13:19:10 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//REPONSE HTTP

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <vector>
#include <fstream>

#include "server.hpp"

#define WEBROOT "./www"

class Request;

class Response
{
	private:
		std::ifstream file;
		std::string	header;
		std::string path;
		std::string	content_type;
		std::string body_cgi;
		std::string _root;
		int	error_status;
		int error_code;
		bool	header_sent;
		bool	error_sent;
		bool	autoindex_sent;
		bool	redir;
		
		std::map<int, std::string> error_msg;

		// std::string	content;
		// std::vector<char> http_response;
		
	public:
		Response(Request& req);
		~Response();

		bool	has_more_data() const;
		std::vector<char> get_next_chunk();
		int	get_error_status() const;
		void	close();
		std::string generate_error_page(int code, const std::string& msg);

		std::string	read_file(const std::string& path);
		std::string get_content_type(const std::string& path);
		std::vector<char> build_reponse(const std::string& body);

		int	set_error_gestion(Request& req);
		std::string	setRedir(int code, const std::string& location);

		// const std::vector<char>& get_response() const;
};


