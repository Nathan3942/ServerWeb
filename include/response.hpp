/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:48:05 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/20 15:56:03 by njeanbou         ###   ########.fr       */
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
		
	public:
		Response(Request& req);
		~Response();

		void	setup_error_msg();
		void	setup_header(Request& req);
		void	setup_error(Request& req);
		
		std::vector<char> get_next_chunk();
		bool	has_more_data() const;
		void	close();

		std::string generate_error_page(int code, const std::string& msg);
		std::map<int, std::string> get_right_error_page(const Request& req);
		int	set_error_gestion(Request& req);
		std::string	setRedir(int code, const std::string& location);
		std::string get_content_type(const std::string& path);

		int	get_error_status() const;
		

		// const std::vector<char>& get_response() const;
};


